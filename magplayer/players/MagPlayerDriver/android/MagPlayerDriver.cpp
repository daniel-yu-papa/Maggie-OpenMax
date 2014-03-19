#include "MagPlayerDriver.h"

static const MagParametersTable_t sParamTable[] = {
    /*key: 1  */{kMediaType,        MagParamTypeInt32},
    /*key: 2  */{kVideo_Codec,      MagParamTypeInt32},
    /*key: 3  */{kAudio_Codec,      MagParamTypeInt32},
    /*key: 4  */{kVideo_TS_pidlist, MagParamTypePointer},
    /*key: 5  */{kAudio_TS_pidlist, MagParamTypePointer},
};

MagPlayerDriver::MagPlayerDriver(void *client, notify_client_callback_f cb):
    mState(MPD_ST_IDLE){
    mpClient = client;
    mClientNotifyFn = cb;
    
    mpPlayer = new MagPlayer();
    if (NULL != mpPlayer){
        mpPlayer->setPrepareCompleteListener(PrepareCompleteEvtListener, static_cast<void *>(this));
        mpPlayer->setFlushCompleteListener(FlushCompleteEvtListener, static_cast<void *>(this));
        mpPlayer->setErrorListener(ErrorEvtListener, static_cast<void *>(this));
        mpPlayer->setInfoListener(InfoEvtListener, static_cast<void *>(this));
        mpPlayer->setSeekCompleteListener(SeekCompleteEvtListener, static_cast<void *>(this));
    }
}

MagPlayerDriver::~MagPlayerDriver(){
    delete mpPlayer;
}

_status_t MagPlayerDriver::setDataSource(const char *url){
    if (mpPlayer != NULL){
        mState = MPD_ST_INITIALIZED;
        return mpPlayer->setDataSource(url);
    }else{
        return MAG_NO_INIT;
    }
}

_status_t MagPlayerDriver::setDataSource(i32 fd, i64 offset, i64 length){
    if (mpPlayer != NULL){
        mState = MPD_ST_INITIALIZED;
        return mpPlayer->setDataSource(fd, offset, length);
    }else{
        return MAG_NO_INIT;
    }
}

_status_t MagPlayerDriver::setDataSource(const sp<IStreamBuffer> &source){
    Parcel param;
    _status_t ret;

    if (mpPlayer ==  NULL){
        return MAG_NO_INIT;
    }
    
    ret = getParameter(idsMediaType, &param);
    if (ret == MAG_NO_ERROR){
        i32 mt = param.readInt32();
        if (mt == MediaTypeTS){
            mpStreamBufUser = new StreamBufferUser(source, 4*1024*1024, 1);
        }else if (mt == MediaTypeES){

        }else{
           AGILE_LOGE("unsupported source type: %d", mt);
           return MAG_BAD_TYPE;
        }
    }else{
        AGILE_LOGE("need to set the media type before setSource");
        return MAG_INVALID_OPERATION;
    }
    
    mState = MPD_ST_INITIALIZED;
    return mpPlayer->setDataSource(mpStreamBufUser);
}

_status_t MagPlayerDriver::setVideoSurfaceTexture(const sp<ISurfaceTexture> &surfaceTexture){
    return MAG_NO_ERROR;
}

_status_t MagPlayerDriver::prepare(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->prepare();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::prepareAsync(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->prepareAsync();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::start(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->start();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::stop(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->stop();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::pause(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->pause();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

bool MagPlayerDriver::isPlaying(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->isPlaying();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

_status_t MagPlayerDriver::seekTo(int msec){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->seekTo(msec);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::flush(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->flush();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::getCurrentPosition(int *msec){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->getCurrentPosition(msec);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::getDuration(int *msec){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->getDuration(msec);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::reset(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->reset();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriver::setParameter(int key, const Parcel &request){
    _status_t ret = MAG_NO_ERROR;

    if (MPD_ST_INITIALIZED != mState)
        return MAG_INVALID_OPERATION;
    
    switch(key){
        case idsMediaType:
        case idsVideo_Codec:
        case idsAudio_Codec:
            i32 value = request.readInt32();
            mpPlayer->setParameters(sParamTable[key].name, sParamTable[key].type, static_cast<void *>(&value));
            break;

        case idsVideo_TS_pidlist:
        case idsAudio_TS_pidlist:
            /*the parcel format: number pid#1 codec#1 pid#2 codec#2 pid#3 codec#3 ....*/
            TrackInfo_t *track;
            char keyValue[64];
            i32 i;
            i32 number = request.readInt32();

            if (key == idsVideo_TS_pidlist)
                mpPlayer->setParameters(kDemuxer_Video_Track_Number, MagParamTypeInt32, static_cast<void *>(&number));
            else
                mpPlayer->setParameters(kDemuxer_Audio_Track_Number, MagParamTypeInt32, static_cast<void *>(&number));
            
            for (i = mSetTrackIndex; i < number + mSetTrackIndex; i++){
                track = (TrackInfo_t *)mag_malloc(sizeof(TrackInfo_t));

                if (NULL != track){
                    if (key == idsVideo_TS_pidlist)
                        track->type  = TRACK_VIDEO;
                    else
                        track->type = TRACK_AUDIO;
                    
                    sprintf(keyValue, kDemuxer_Track_Info, i);
                    track->pid   = request.readInt32();
                    track->codec = request.readInt32();
                    mpPlayer->setParameters(keyValue, MagParamTypePointer, static_cast<void *>(track));
                    if (key == idsVideo_TS_pidlist)
                        sprintf(keyValue, "video.%s", keyValue);
                    else
                        sprintf(keyValue, "audio.%s", keyValue);
                    track->name  = mag_strdup(keyValue);
                }else{
                    AGILE_LOGE("No memory to create the TrackInfo_t");
                }
            }  
            mSetTrackIndex = i;
            break;
        
        default:
            ret = MAG_BAD_VALUE;
            AGILE_LOGE("unsupported parameter key: %d", key);
            break;
    }
    return ret;
}

_status_t MagPlayerDriver::getParameter(int key, Parcel *reply){
    _status_t ret = MAG_NO_ERROR;
    void *value;

    if (MPD_ST_INITIALIZED != mState)
        return MAG_INVALID_OPERATION;
    
    switch(key){
        case idsMediaType:
        case idsVideo_Codec:
        case idsAudio_Codec:
            ret = mpPlayer->getParameters(sParamTable[key].name, sParamTable[key].type, &value);
            if (ret == MAG_NO_ERROR)
                reply->writeInt32(static_cast<i32>(*value));
            break;

        case idsVideo_TS_pidlist:
        case idsAudio_TS_pidlist:
        default:
            ret = MAG_BAD_VALUE;
            AGILE_LOGE("unsupported parameter key: %d", key);
            break;
    }
    return ret;
}

_status_t MagPlayerDriver::invoke(const Parcel &request, Parcel *reply){
    i32 methodID;
    
    if (MPD_ST_INITIALIZED != mState)
        return MAG_INVALID_OPERATION;

    methodID = request.readInt32();
    
    switch(methodID){
        case MAG_INVOKE_ID_SET_WINDOW_SIZE:
            /*parcel format: method id, x, y, w, h*/
            i32 x, y, w, h;
            
            x = request.readInt32();
            y = request.readInt32();
            w = request.readInt32();
            h = request.readInt32();
            mpPlayer->setDisplayWindow(x, y, w, h);
            break;
    };
}

void MagPlayerDriver::PrepareCompleteEvtListener(void *priv){
    if (priv != NULL){
        MagPlayerDriver *obj = static_cast<MagPlayerDriver *>(priv);
        obj->sendEvent(MEDIA_PREPARED);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriver::SeekCompleteEvtListener(void *priv){
    if (priv != NULL){
        MagPlayerDriver *obj = static_cast<MagPlayerDriver *>(priv);
        obj->sendEvent(MEDIA_SEEK_COMPLETE);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriver::ErrorEvtListener(void *priv, i32 what, i32 extra){
    mState = MPD_ST_ERROR;
    if (priv != NULL){
        MagPlayerDriver *obj = static_cast<MagPlayerDriver *>(priv);
        obj->sendEvent(MEDIA_ERROR, what, extra);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriver::InfoEvtListener(void *priv, i32 what, i32 extra){
    if (priv != NULL){
        MagPlayerDriver *obj = static_cast<MagPlayerDriver *>(priv);
        obj->sendEvent(MEDIA_INFO, what, extra);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriver::FlushCompleteEvtListener(void *priv){
    if (priv != NULL){
        MagPlayerDriver *obj = static_cast<MagPlayerDriver *>(priv);
        obj->sendEvent(MEDIA_FLUSH_COMPLETE);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriver::sendEvent(int msg, int ext1, int ext2) {
    if ((NULL != mClientNotifyFn) && (NULL != mpClient)){
        mClientNotifyFn(mpClient, msg, ext1, ext2);
    }else{
        AGILE_LOGE("failed to do sendEvent! (mClientNotifyFn=0x%p, mpClient=0x%p)", mClientNotifyFn, mpClient);
    }
}

