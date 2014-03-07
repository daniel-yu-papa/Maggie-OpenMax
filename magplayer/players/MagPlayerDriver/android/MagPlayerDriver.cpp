#include "MagPlayerDriver.h"

static const MagParametersTable_t sParamTable[] = {
    /*key: 1  */{kMediaType,        MagParamTypeInt32},
    /*key: 2  */{kVideo_Codec,      MagParamTypeInt32},
    /*key: 3  */{kAudio_Codec,      MagParamTypeInt32},
    /*key: 4  */{kVideo_TS_pidlist, MagParamTypePointer},
    /*key: 5  */{kAudio_TS_pidlist, MagParamTypePointer},
};

MagPlayerDriver::MagPlayerDriver(void *client, notify_client_callback_f cb):
    mState(MPD_IDLE),
    mSetTrackIndex(0){

    mpClient = client;
    mClientNotifyFn = cb;
    
    mpPlayer = new MagPlayer();

    if (NULL != mpPlayer){
        Mag_CreateEventGroup(&mPrepareEvtGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&mPrepareDoneEvt, 0))
            Mag_AddEventGroup(mPrepareEvtGroup, mPrepareDoneEvt);
        if (MAG_ErrNone == Mag_CreateEvent(&mPrepareErrorEvt, 0))
            Mag_AddEventGroup(mPrepareEvtGroup, mPrepareErrorEvt);
    }

    Mag_CreateMutex(&mNotifyLock);
}

MagPlayerDriver::~MagPlayerDriver(){
    Mag_DestroyEvent(mPrepareDoneEvt);
    Mag_DestroyEvent(mPrepareErrorEvt);
    Mag_DestroyEventGroup(mPrepareEvtGroup);
    delete mpPlayer;
}

_status_t MagPlayerDriver::setDataSource(
             const char *url, const MagMiniDBHandle settings){
    mState = MPD_INITIALIZED;
    return mpPlayer->setDataSource(url, settings);
}

_status_t MagPlayerDriver::setDataSource(i32 fd, i64 offset, i64 length){
    mState = MPD_INITIALIZED;
    return mpPlayer->setDataSource(fd, offset, length);
}

_status_t MagPlayerDriver::setDataSource(const sp<IStreamBuffer> &source){
    Parcel param;
    _status_t ret;
    
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
    
    mState = MPD_INITIALIZED;
    return mpPlayer->setDataSource(mpStreamBufUser);
}

_status_t MagPlayerDriver::setVideoSurface(const sp<Surface> &surface){

}

_status_t MagPlayerDriver::setVideoSurfaceTexture(const sp<ISurfaceTexture> &surfaceTexture){

}

_status_t MagPlayerDriver::prepare(){
    MagErr_t ret;
    
    mpPlayer->prepareAsync();
    mState = MPD_PREPARING;
    Mag_WaitForEventGroup(mPrepareEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");
}

_status_t MagPlayerDriver::prepareAsync(){
    mpPlayer->prepareAsync();
    return MAG_NO_ERROR;
}

_status_t MagPlayerDriver::start(){
    if (MPD_PREPARED == mState){
        mState = MPD_RUNNING;
        mpPlayer->start();
        return MAG_NO_ERROR;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_NO_INIT;
    }
}

_status_t MagPlayerDriver::stop(){
    if (MPD_RUNNING == mState){
        mState = MPD_STOPPED;
        mSetTrackIndex = 0;
        
        mpPlayer->stop();
        return MAG_NO_ERROR;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_UNKNOWN_ERROR;
    }
}

_status_t MagPlayerDriver::pause(){
    if (MPD_RUNNING == mState){
        mState = MPD_PAUSED;
        mpPlayer->pause();
        return MAG_NO_ERROR;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_UNKNOWN_ERROR;
    }
}

bool MagPlayerDriver::isPlaying(){
    if (MPD_RUNNING == mState)
        return true;
    else
        return false;
}

_status_t MagPlayerDriver::seekTo(int msec){
    if ((MPD_PREPARED == mState) ||
        (MPD_RUNNING  == mState) ||
        (MPD_PAUSED   == mState) ||
        (MPD_PLAYBACK_COMPLETED == mState)){
        mSeekBackState = mState;
        mState = MPD_SEEKING;
        mpPlayer->seekTo(msec);
        return MAG_NO_ERROR;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_UNKNOWN_ERROR;
    }
}

_status_t MagPlayerDriver::getCurrentPosition(int *msec){

}

_status_t MagPlayerDriver::getDuration(int *msec){

}

_status_t MagPlayerDriver::reset(){

}

_status_t MagPlayerDriver::setParameter(int key, const Parcel &request){
    _status_t ret = MAG_NO_ERROR;
    
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
    i32 methodID = request.readInt32();
    
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

void MagPlayerDriver::PrepareCompleteEvtListener(){
    mState = MPD_PREPARED;
    Mag_SetEvent(mPrepareDoneEvt);
}

void MagPlayerDriver::SeekCompleteEvtListener(){
    mState = mSeekBackState;
    Mag_SetEvent(mPrepareDoneEvt);
}

void MagPlayerDriver::ErrorEvtListener(i32 what, i32 extra){
    if (MPD_PREPARING == mState){
        mState = MPD_ERROR;
        Mag_SetEvent(mPrepareErrorEvt);
    }
    sendEvent()
}

void MagPlayerDriver::FlushCompleteEvtListener(){
    mState = mFlushBackState;
}

void MagPlayerDriver::setNotifyCallback(void* client, notify_client_callback_f notifyFunc){
    Mag_AcquireMutex(mNotifyLock);

    mpClient        = client;
    mClientNotifyFn = notifyFunc;
    
    Mag_ReleaseMutex(mNotifyLock);
}

void MagPlayerDriver::sendEvent(int msg, int ext1, int ext2, const Parcel *obj) {
    if ((NULL != mClientNotifyFn) && (NULL != mpClient)){
        mClientNotifyFn(mpClient, msg, ext1, ext2, obj);
    }else{
        AGILE_LOGE("failed to do sendEvent! (mClientNotifyFn=0x%p, mpClient=0x%p)", mClientNotifyFn, mpClient);
    }
}

