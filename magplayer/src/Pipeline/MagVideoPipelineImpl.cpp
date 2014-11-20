#include "MagVideoPipelineImpl.h"
#include "MagPlayer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "VideoPipelineImpl"


#define VIDEO_ES_DUMP_FILE_NAME "/lmp/magplayer_video.es"

MagVideoPipelineImpl::MagVideoPipelineImpl():
                                      mLooper(NULL),
                                      mMsgHandler(NULL),
                                      mMagPlayerNotifier(NULL),
                                      mbPostFillBufEvt(false){ 
    AGILE_LOGV("Constructor!");
    mState = ST_INIT;
    mIsFlushed = false;
    mEmptyThisBufferMsg = createMessage(MagVideoPipeline_EmptyThisBuffer);
    mDestroyMsg = createMessage(MagVideoPipeline_Destroy);
}

MagVideoPipelineImpl::~MagVideoPipelineImpl(){
    AGILE_LOGV("enter!");
    mLooper->clear(mLooper);
    mDestroyMsg->postMessage(mDestroyMsg, 0);
    mLooper->waitOnAllDone(mLooper);
    destroyHandler(&mMsgHandler);
    destroyLooper(&mLooper);
    AGILE_LOGV("exit!");
}

void MagVideoPipelineImpl::setMagPlayerNotifier(MagMessageHandle notifyMsg){
    mMagPlayerNotifier = notifyMsg;
}

MagMessageHandle MagVideoPipelineImpl::getMagPlayerNotifier(){
    return mMagPlayerNotifier;
}

_status_t MagVideoPipelineImpl::setup(i32 trackID, TrackInfo_t *sInfo){
    if (mState != ST_INIT){
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return MAG_INVALID_OPERATION;
    }
    mState = ST_STOP;
    mTrackID = trackID;

#ifdef DUMP_VIDEO_ES_FILE
    mDumpFile = fopen(VIDEO_ES_DUMP_FILE_NAME, "wb+");
    if (NULL == mDumpFile){
        AGILE_LOGE("failed to open the file: %s", VIDEO_ES_DUMP_FILE_NAME);
    }else{
        AGILE_LOGD("Create the file: %s -- OK!", VIDEO_ES_DUMP_FILE_NAME);
    }
#endif
    return MAG_NO_ERROR;
}

_status_t MagVideoPipelineImpl::start(){
    AGILE_LOGV("enter!");
    if ((mState == ST_STOP) ||
        (mState == ST_PAUSE)){
        mState = ST_PLAY;
        postFillThisBuffer();
    }else{
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return MAG_INVALID_OPERATION;
    }
    AGILE_LOGV("exit!");
    return MAG_NO_ERROR;
}

_status_t MagVideoPipelineImpl::stop(){
    _status_t ret = MAG_NO_ERROR;

    if ((mState == ST_PLAY) ||
        (mState == ST_PAUSE)){
        mLooper->clear(mLooper);

#ifdef DUMP_VIDEO_ES_FILE
        if (mDumpFile){
            AGILE_LOGD("Close the file -- OK!");
            fclose(mDumpFile);
            mDumpFile = NULL;
        }
#endif
    }else{
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        ret = MAG_INVALID_OPERATION;
    }
    mState = ST_STOP;
    return ret;
}

_status_t MagVideoPipelineImpl::pause(){
    if (mState == ST_PLAY){
        AGILE_LOGD("do pause!");
        mState = ST_PAUSE;
    }else{
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return MAG_INVALID_OPERATION;
    }
    return MAG_NO_ERROR;
}

_status_t MagVideoPipelineImpl::resume(){
    if (mIsFlushed){
        mIsFlushed = false;
        if (mState == ST_PLAY){
            postFillThisBuffer();
        }else{
            AGILE_LOGD("[flush to resume] in state: %s, keep it!", state2String(mState));
        }
    }else{
        if ((mState == ST_PAUSE) || (mState == ST_PLAY)){
            mState = ST_PLAY;
            postFillThisBuffer();
        }else{
            AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
            return MAG_INVALID_OPERATION;
        }
    }
    return MAG_NO_ERROR;
}

_status_t MagVideoPipelineImpl::flush(){
    if ((mState == ST_PLAY) ||
        (mState == ST_PAUSE)){
        mIsFlushed = true;
        mLooper->clear(mLooper);
        mLooper->waitOnAllDone(mLooper);
    }
    return MAG_NO_ERROR;
}

_status_t MagVideoPipelineImpl::reset(){
    mState = ST_INIT;
    return MAG_NO_ERROR;
}

void *MagVideoPipelineImpl::getClkConnectedComp(i32 *port){
    return NULL;
}

void MagVideoPipelineImpl::proceedMediaBuffer(MediaBuffer_t *buf){
#ifdef DUMP_VIDEO_ES_FILE
    if (NULL != mDumpFile){
        fwrite(buf->buffer, 1, buf->buffer_size, mDumpFile);
    }
#endif
    pushEsPackets(buf);
    buf->release(buf);
}

void MagVideoPipelineImpl::notifyPlaybackComplete(){
    /*simulate the video playback complete and then notify the APP*/
    mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "what", MagPlayer::kWhatPlayComplete);
    mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "track-id", mTrackID);
    mMagPlayerNotifier->setMessage(mMagPlayerNotifier, "reply", mEmptyThisBufferMsg, MAG_FALSE);
    mMagPlayerNotifier->postMessage(mMagPlayerNotifier, 0);
}

void MagVideoPipelineImpl::onEmptyThisBuffer(MagMessageHandle msg){
    boolean ret;
    void *value;
    MediaBuffer_t *buf = NULL;
    char *eos = false;
    
    if ((mState == ST_STOP) ||
        (mState == ST_INIT)) {
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return;
    }

    ret = msg->findPointer(msg, "media-buffer", &value);
    if (!ret){
        AGILE_LOGE("failed to find the media-buffer pointer!");
        return;
    }  

    buf = static_cast<MediaBuffer_t *>(value);

    if (NULL != buf){
        proceedMediaBuffer(buf);
        postFillThisBuffer();
    }else{
        ret = msg->findString(msg, "eos", &eos);
        if (!ret){
            AGILE_LOGE("failed to find the eos string!");
        }  

        if (!strcmp(eos, "yes")){
            AGILE_LOGD("get EOS. finished!"); 
        }else{
            postFillThisBuffer();
        }
    }
}

void MagVideoPipelineImpl::onDestroy(MagMessageHandle msg){
    AGILE_LOGV("enter!");
    if (mEmptyThisBufferMsg)
        destroyMagMessage(&mEmptyThisBufferMsg);
    if (mMagPlayerNotifier)
        destroyMagMessage(&mMagPlayerNotifier);
    mEmptyThisBufferMsg = NULL;
    mMagPlayerNotifier  = NULL;
}

void MagVideoPipelineImpl::setFillBufferFlag(bool flag){
    mbPostFillBufEvt = flag;
}

bool MagVideoPipelineImpl::getFillBufferFlag(){
    return mbPostFillBufEvt;
}

void MagVideoPipelineImpl::postFillThisBuffer(){
    if (mMagPlayerNotifier != NULL){
        if ((mState == ST_PLAY) && (!mIsFlushed)){
            if (needData()){
                mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "what", MagPlayer::kWhatFillThisBuffer);
                mMagPlayerNotifier->setMessage(mMagPlayerNotifier, "reply", mEmptyThisBufferMsg, MAG_FALSE);
                mMagPlayerNotifier->postMessage(mMagPlayerNotifier, 0);
            }else{
                setFillBufferFlag(true);
                AGILE_LOGD("driver buffer is full!");
            }
        }else{
            AGILE_LOGV("don't send out the fillThisBuffer event in %s state: (%s)", 
                        mIsFlushed ? "flushed" : "none-Play",
                        state2String(mState));
        }
    }else{
        AGILE_LOGE("mMagPlayerNotifier is not setting!");
    }
}

void MagVideoPipelineImpl::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagVideoPipelineImpl *thiz = (MagVideoPipelineImpl *)priv;
    
    switch (msg->what(msg)) {
        case MagVideoPipeline_EmptyThisBuffer:
            thiz->onEmptyThisBuffer(msg);
            break;
        
        case MagVideoPipeline_Destroy:
            thiz->onDestroy(msg);
            break;

        default:
            break;
    }
}

MagMessageHandle MagVideoPipelineImpl::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagVideoPipelineImpl::getLooper(){
    if ((NULL != mLooper) && (NULL != mMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == mLooper){
        mLooper = createLooper(LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", mLooper);
    }
    
    if (NULL != mLooper){
        if (NULL == mMsgHandler){
            mMsgHandler = createHandler(mLooper, onMessageReceived, (void *)this);

            if (NULL != mMsgHandler){
                mLooper->registerHandler(mLooper, mMsgHandler);
                mLooper->start(mLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}


