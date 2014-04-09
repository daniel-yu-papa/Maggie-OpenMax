#include "MagPlayer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magPlayer"


#define LOOPER_NAME "MagPlayerLooper"

MagPlayer::MagPlayer(){
    initialize();
}

MagPlayer::~MagPlayer(){
    cleanup();
}

void MagPlayer::onSourceNotify(MagMessageHandle msg){
    void *value;
    boolean ret;

    AGILE_LOGV("enter!");
    
    ret = msg->findPointer(msg, "source", &value);
    if (!ret){
        AGILE_LOGE("failed to find the source object!");
        return;
    }
    
    if (mSource == NULL){
        mSource = static_cast<MagPlayer_Component_CP *>(value);
        mDemuxer = new MagPlayer_Demuxer_FFMPEG();
        mState = ST_INITIALIZED;
    }else{
        AGILE_LOGE("Failed! There is existed source(0x%x) and still running...", mSource);
        delete static_cast<MagPlayer_Component_CP *>(value);
    }
}

_status_t MagPlayer::setDataSource(const char *url){
    getLooper();

    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::setDataSource(i32 fd, i64 offset, i64 length){
    getLooper();

    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::setDataSource(StreamBufferUser *buffer){
    MagPlayer_Component_CP *cp;

    getLooper();
    
    cp = new MagPlayer_Component_CP();
    if (cp != NULL){
        cp->Create(buffer);
    }else{
       AGILE_LOGE("failed to allocate the object: MagPlayer_Component_CP");
       return MAG_NO_MEMORY;
    }

    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->setPointer(mSourceNotifyMsg, "source", static_cast<void *>(cp));
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        delete cp;
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onErrorNotify(MagMessageHandle msg){
    i32 what;
    i32 extra;
    boolean ret;
    
    ret = msg->findInt32(msg, "what", &what);
    if (!ret){
        AGILE_LOGE("failed to find the what object!");
        what = E_WHAT_UNKNOWN;
    }

    ret = msg->findInt32(msg, "extra", &extra);
    if (!ret){
        AGILE_LOGE("failed to find the extra object!");
        extra = MAG_UNKNOWN_ERROR;
    }
    
    mState = ST_ERROR;

    Mag_SetEvent(mErrorEvt);
    
    if ((mErrorFn != NULL) && (mErrorPriv != NULL))
        mErrorFn(mErrorPriv, what, extra);
}

void MagPlayer::onPrepared(MagMessageHandle msg){
    _status_t ret;
    
    if (ST_INITIALIZED != mState){
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }else{
        mState = ST_PREPARING;

        void *value;

        if (MAG_NO_ERROR == getParameters(kDemuxer_Container_Type, MagParamTypeString, &value)){
            AGILE_LOGV("container type: %s", static_cast<char *>(value));
            mDemuxer->setParameters(kDemuxer_Container_Type, MagParamTypeString, value);
        }
        if (MAG_NO_ERROR == getParameters(kDemuxer_Probe_Stream, MagParamTypeString, &value)){
            AGILE_LOGV("probe stream: %s", static_cast<char *>(value));
            mDemuxer->setParameters(kDemuxer_Probe_Stream, MagParamTypeString, value);
        }
        ret = mDemuxer->start(mSource, mParametersDB);

        if (ret == MAG_NO_ERROR){
            MagMessageHandle msg;

            mDemuxerNotify = mDemuxer->createNotifyMsg();
            
            mTrackTable = mDemuxer->getTrackInfoList();
            if (mTrackTable->videoTrackNum > 0){
                mDemuxer->setPlayingTrackID(0);
                
                msg = createMessage(MagMsg_ComponentNotify);
                msg->setInt32(msg, "track-idx", 0);
                /*construct the video pipeline*/
#ifdef MOCK_OMX_IL
                mVOmxComponent = new MagPlayer_Mock_OMX("video");
                mVOmxComponent->setMagPlayerNotifier(msg);
#endif
            }
            if (mTrackTable->audioTrackNum > 0){
                mDemuxer->setPlayingTrackID(mTrackTable->videoTrackNum);

                msg = createMessage(MagMsg_ComponentNotify);
                msg->setInt32(msg, "track-idx", mTrackTable->videoTrackNum);
                /*construct the video pipeline*/
#ifdef MOCK_OMX_IL
                mAOmxComponent = new MagPlayer_Mock_OMX("audio");
                mAOmxComponent->setMagPlayerNotifier(msg);
#endif
            }
            if (mTrackTable->subtitleTrackNum > 0){
                mDemuxer->setPlayingTrackID(mTrackTable->videoTrackNum + mTrackTable->audioTrackNum);
            }
            
            Mag_SetEvent(mCompletePrepareEvt);
        }else{
            if (mErrorNotifyMsg != NULL){
                mErrorNotifyMsg->setInt32(mErrorNotifyMsg, "what", E_WHAT_DEMUXER);
                mErrorNotifyMsg->setInt32(mErrorNotifyMsg, "extra", ret);
                mErrorNotifyMsg->postMessage(mErrorNotifyMsg, 0);
            }else{
                AGILE_LOGE("the mErrorNotifyMsg is NULL");
            }
        }
    }      
}

_status_t MagPlayer::prepareAsync(){
    if (mPrepareMsg != NULL){
        mPrepareMsg->postMessage(mPrepareMsg, 0);
    }else{
        AGILE_LOGE("the mPrepareMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::prepare(){
    prepareAsync();
    AGILE_LOGD("before prepare waiting!");
    Mag_WaitForEventGroup(mEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("after prepare waiting!");
    
    return MAG_NO_ERROR;
}

void MagPlayer::onStart(MagMessageHandle msg){
    if ((ST_PREPARED == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
#ifdef MOCK_OMX_IL
        if (NULL != mVOmxComponent)
            mVOmxComponent->start();

        if (NULL != mAOmxComponent)
            mAOmxComponent->start();
#endif

        mState = ST_RUNNING;
    }else if (ST_PREPARING == mState){
        mPrepareCompleteMQ->put(mPrepareCompleteMQ, msg);
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

_status_t MagPlayer::start(){
    AGILE_LOGV("enter!");
    if (mStartMsg != NULL){
        mStartMsg->postMessage(mStartMsg, 0);
    }else{
        AGILE_LOGE("the mStartMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onStop(MagMessageHandle msg){
    if ((ST_RUNNING == mState) ||
        (ST_PAUSED == mState)  ||
        (ST_PLAYBACK_COMPLETED == mState)){
        /*todo*/
        mState = ST_STOPPED;
    }else if (ST_PREPARING == mState){
        mPrepareCompleteMQ->put(mPrepareCompleteMQ, msg);
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else if (ST_FASTING == mState){
        mState = mFastBackState;
        flush();
        start();
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

_status_t MagPlayer::stop(){
    if (mStopMsg != NULL){
        mStopMsg->postMessage(mStopMsg, 0);
    }else{
        AGILE_LOGE("the mStopMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onPause(MagMessageHandle msg){
    if (ST_RUNNING == mState){
        /*todo*/
        mState = ST_PAUSED;
    }else if (ST_PREPARING == mState){
        mPrepareCompleteMQ->put(mPrepareCompleteMQ, msg);
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

_status_t MagPlayer::pause(){
    if (mPauseMsg != NULL){
        mPauseMsg->postMessage(mPauseMsg, 0);
    }else{
        AGILE_LOGE("the mPauseMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onFlush(MagMessageHandle msg){
    if (isValidFSState(mState)){
        mFlushBackState = mState;
        mState          = ST_FLUSHING;
        
        /*todo*/
        
        Mag_SetEvent(mCompleteFlushEvt);
    }else if (ST_PREPARING == mState){
        mPrepareCompleteMQ->put(mPrepareCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

bool MagPlayer::isPlaying(){
    return true;
}

_status_t MagPlayer::getCurrentPosition(int* msec){
    return MAG_NO_ERROR;
}

_status_t MagPlayer::getDuration(int* msec){
    return MAG_NO_ERROR;
}

_status_t MagPlayer::flush(){
    if (mFlushMsg != NULL){
        mFlushMsg->postMessage(mFlushMsg, 0);
    }else{
        AGILE_LOGE("the mFlushMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onSeek(MagMessageHandle msg){
    if (isValidFSState(mState)){
        mSeekBackState = mState;
        mState         = ST_SEEKING;
        
        /*todo*/
        
        Mag_SetEvent(mCompleteSeekEvt);
    }else if (ST_PREPARING == mState){
        mPrepareCompleteMQ->put(mPrepareCompleteMQ, msg);
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

_status_t MagPlayer::seekTo(i32 msec){
    if (mSeekToMsg != NULL){
        mSeekToMsg->setInt32(mFastMsg, "seek_to", msec);
        mSeekToMsg->postMessage(mSeekToMsg, 0);
    }else{
        AGILE_LOGE("the mFlushMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onFast(MagMessageHandle msg){
    if ((ST_PREPARED == mState) ||
        (ST_RUNNING == mState)){
        mFastBackState = mState;
        /*todo*/
        mState = ST_FASTING;
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
    }
}

_status_t MagPlayer::fast(i32 speed){
    if (mFastMsg != NULL){
        mFastMsg->setInt32(mFastMsg, "speed", speed);
        mFastMsg->postMessage(mFastMsg, 0);
    }else{
        AGILE_LOGE("the mFastMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onReset(MagMessageHandle msg){
    /*todo*/
    mState = ST_IDLE;
}

_status_t MagPlayer::reset(){
    if (mResetMsg != NULL){
        mResetMsg->postMessage(mResetMsg, 0);
    }else{
        AGILE_LOGE("the mResetMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onSetVolume(MagMessageHandle msg){

}

_status_t MagPlayer::setVolume(float leftVolume, float rightVolume){
    if (mSetVolumeMsg != NULL){
        mSetVolumeMsg->setFloat(mSetVolumeMsg, "left", leftVolume);
        mSetVolumeMsg->setFloat(mSetVolumeMsg, "right", rightVolume);
        mSetVolumeMsg->postMessage(mSetVolumeMsg, 0);
    }else{
        AGILE_LOGE("the mResetMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::setSeekCompleteListener(fnNotifySeekComplete fn, void *priv){
    if (NULL != fn){
        mNotifySeekCompleteFn = fn;
        mSeekCompletePriv     = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setPrepareCompleteListener(fnNotifyPrepareComplete fn, void *priv){
    if (NULL != fn){
        mNotifyPrepareCompleteFn = fn;
        mPrepareCompletePriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setFlushCompleteListener(fnNotifyFlushComplete fn, void *priv){
    if (NULL != fn){
        mNotifyFlushCompleteFn = fn;
        mFlushCompletePriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setInfoListener(fnNotifyInfo fn, void *priv){
    if (NULL != fn){
        mInfoFn   = fn;
        mInfoPriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setErrorListener(fnNotifyError fn, void *priv){
    if (NULL != fn){
        mErrorFn   = fn;
        mErrorPriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::onSetParameters(MagMessageHandle msg){
    if (ST_INITIALIZED != mState){
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }else{
        char *name;
        i32 t;
        MagParamType_t type;
        
        msg->findString(msg, "name", &name);
        msg->findInt32(msg, "type", &t);
        type = static_cast<MagParamType_t>(t);

        if (NULL != mParametersDB){
            switch (type){
                case MagParamTypeInt32:
                    {
                    i32 v;
                    msg->findInt32(msg, "value", &v);
                    AGILE_LOGV("name = %s, value = %d", name, v);
                    mParametersDB->setInt32(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeInt64:
                    {
                    i64 v;
                    msg->findInt64(msg, "value", &v);
                    mParametersDB->setInt64(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeUInt32:
                    {
                    ui32 v;
                    msg->findUInt32(msg, "value", &v);
                    mParametersDB->setUInt32(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeFloat:
                    {
                    fp32 v;
                    msg->findFloat(msg, "value", &v);
                    mParametersDB->setFloat(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeDouble:
                    {
                    fp64 v;
                    msg->findDouble(msg, "value", &v);
                    mParametersDB->setDouble(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypePointer:
                {
                    void *value;
                    msg->findPointer(msg, "value", &value);
                    mParametersDB->setPointer(mParametersDB, name, value);
                }
                    break;

                case MagParamTypeString:
                {
                    char *v;
                    msg->findString(msg, "value", &v);
                    AGILE_LOGV("name = %s, value = %s", name, v);
                    mParametersDB->setString(mParametersDB, name, v);
                }
                    break;

                default:
                    AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                    break;
            }
        }else{
            AGILE_LOGE("the parameter db is NOT initialized!");
        }
    }
}

_status_t MagPlayer::setParameters(const char *name, MagParamType_t type, void *value){
    if (NULL != mSetParametersMsg){
        mSetParametersMsg->setString(mSetParametersMsg, "name", name);
        mSetParametersMsg->setInt32(mSetParametersMsg, "type", static_cast<i32>(type));
        switch (type){
            case MagParamTypeInt32:
            {
                i32 v = *(static_cast<i32 *>(value));
                mSetParametersMsg->setInt32(mSetParametersMsg, "value", v);
            }
                break;
                
            case MagParamTypeInt64:
            {
                i64 v = *(static_cast<i64 *>(value));
                mSetParametersMsg->setInt64(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypeUInt32:
            {
                ui32 v = *(static_cast<ui32 *>(value));
                mSetParametersMsg->setUInt32(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypeFloat:
            {
                fp32 v = *(static_cast<fp32 *>(value));
                mSetParametersMsg->setFloat(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypeDouble:
            {
                fp64 v = *(static_cast<fp64 *>(value));
                mSetParametersMsg->setDouble(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypePointer:
            {
                mSetParametersMsg->setPointer(mSetParametersMsg, "value", value);
            }
                break;

            case MagParamTypeString:
            {
                char *v = static_cast<char *>(value);
                mSetParametersMsg->setString(mSetParametersMsg, "value", v);
            }
                break;

             default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                break;
        }
        mSetParametersMsg->postMessage(mSetParametersMsg, 0);
    }else{
        AGILE_LOGE("the mSetParametersMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::getParameters(const char *name, MagParamType_t type, void **value){
    boolean result;
    _status_t ret = MAG_NO_ERROR;
    
   if (NULL != mParametersDB){
        switch (type){
            case MagParamTypeInt32:
            {
                i32 *v = NULL;
                result = mParametersDB->findInt32(mParametersDB, name, v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeInt64:
            {
                i64 *v = NULL;
                result = mParametersDB->findInt64(mParametersDB, name, v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeUInt32:
            {
                ui32 *v = NULL;
                result = mParametersDB->findUInt32(mParametersDB, name, v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeFloat:
            {
                fp32 *v = NULL;
                result = mParametersDB->findFloat(mParametersDB, name, v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeDouble:
            {
                fp64 *v = NULL;
                result = mParametersDB->findDouble(mParametersDB, name, v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypePointer:
            {
                result = mParametersDB->findPointer(mParametersDB, name, value);
                if (!result)
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeString:
            {
                char *v = NULL;
                result = mParametersDB->findString(mParametersDB, name, &v);
                AGILE_LOGD("string: %s", v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                ret = MAG_BAD_VALUE;
                break;
        }
    }else{
        AGILE_LOGE("the parameter db is NOT initialized!");
    }
    return ret;
}

bool MagPlayer::isValidFSState(State_t st){
    if ((ST_PREPARED == mState) ||
        (ST_RUNNING  == mState) ||
        (ST_PAUSED   == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
        return true;
    }else{
        return false;
    }
}

void MagPlayer::onComponentNotify(MagMessageHandle msg){
    boolean ret;
    i32 ivalue;
    MagMessageHandle mvalue;
    i32 what;

    ret = msg->findInt32(msg, "what", &what);
    if (!ret){
        AGILE_LOGE("failed to find the what int32!");
        return;
    }

    if (what == MagPlayer::kWhatFillThisBuffer){
        ret = msg->findInt32(msg, "track-idx", &ivalue);
        if (!ret){
            AGILE_LOGE("failed to find the track-idx int32!");
            return;
        }

        ret = msg->findMessage(msg, "reply", &mvalue);
        if (!ret){
            AGILE_LOGE("failed to find the reply message!");
            return;
        }
        
        if (NULL != mDemuxerNotify){
            AGILE_LOGV("what=kWhatFillThisBuffer, track-idx=%d", ivalue);
            mDemuxerNotify->setInt32(mDemuxerNotify, "what", MagPlayer_Demuxer_Base::kWhatReadFrame);
            mDemuxerNotify->setInt32(mDemuxerNotify, "track-idx", ivalue);
            mDemuxerNotify->setMessage(mDemuxerNotify, "reply", mvalue);
            mDemuxerNotify->postMessage(mDemuxerNotify, 0);
        }      
    }
}

void MagPlayer::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    
    switch (msg->what(msg)) {
        case MagMsg_SourceNotify:
            thiz->onSourceNotify(msg);
            break;
            
        case MagMsg_Prepare:
            thiz->onPrepared(msg);
            break;

        case MagMsg_Start:
            thiz->onStart(msg);
            break;

        case MagMsg_Stop:
            thiz->onStop(msg);
            break;

        case MagMsg_Pause:
            thiz->onPause(msg);
            break;

        case MagMsg_Flush:
            thiz->onFlush(msg);
            break;

        case MagMsg_SeekTo:
            thiz->onSeek(msg);
            break;
            
        case MagMsg_Fast:
            thiz->onFast(msg);
            break;
            
        case MagMsg_SetVolume:
            thiz->onSetVolume(msg);
            break;

        case MagMsg_Reset:
            thiz->onReset(msg);
            break;
            
        case MagMsg_ErrorNotify:
            thiz->onErrorNotify(msg);
            break;

        case MagMsg_ComponentNotify:
            thiz->onComponentNotify(msg);
            break;
            
        case MagMsg_SetParameters:
            thiz->onSetParameters(msg);
            break;
            
        default:
            break;
    }
}

void MagPlayer::onCompletePrepareEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    MagMessageHandle msg = NULL;

    if (NULL == thiz)
        return;

    /*must set the state firstly and then proceed the messages in the waiting queue. Otherwise it might cause dead lock*/
    thiz->mState = ST_PREPARED;
    do {
        thiz->mPrepareCompleteMQ->get(thiz->mPrepareCompleteMQ, &msg);
        if (NULL != msg){
            msg->postMessage(msg, 0);
        }
    }while(msg);
    
    if ((thiz->mNotifyPrepareCompleteFn != NULL) && (thiz->mPrepareCompletePriv != NULL))
        thiz->mNotifyPrepareCompleteFn(thiz->mPrepareCompletePriv);
}

void MagPlayer::onCompleteSeekEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    MagMessageHandle msg = NULL;

    if (NULL == thiz)
        return;
    
    if (thiz->isValidFSState(thiz->mSeekBackState)){
        thiz->mState = thiz->mSeekBackState;
        if ((thiz->mNotifySeekCompleteFn != NULL) && (thiz->mSeekCompletePriv != NULL))
            thiz->mNotifySeekCompleteFn(thiz->mSeekCompletePriv);
    }else{
        AGILE_LOGE("backstate: state %d is not valid for seekTo operation. QUIT!", thiz->mSeekBackState);
    }
    
    do {
        thiz->mSeekCompleteMQ->get(thiz->mSeekCompleteMQ, &msg);
        if (NULL != msg){
            msg->postMessage(msg, 0);
        }
    }while(msg);
}

void MagPlayer::onCompleteFlushEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    MagMessageHandle msg = NULL;

    if (NULL == thiz)
        return;

    if (thiz->isValidFSState(thiz->mFlushBackState)){
        thiz->mState = thiz->mFlushBackState;
        if (ST_RUNNING == thiz->mState){
            thiz->start();
        }
        
        if ((thiz->mNotifyFlushCompleteFn != NULL) && (thiz->mFlushCompletePriv != NULL))
            thiz->mNotifyFlushCompleteFn(thiz->mFlushCompletePriv);
    }else{
        AGILE_LOGE("backstate:%d is not valid for Flush operation. QUIT!", thiz->mFlushBackState);
    }

    do {
        thiz->mFlushCompleteMQ->get(thiz->mFlushCompleteMQ, &msg);
        if (NULL != msg){
            msg->postMessage(msg, 0);
        }
    }while(msg);
}

void MagPlayer::onErrorEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;

    if (NULL == thiz)
        return;
    
    thiz->mState = ST_ERROR;
}

void MagPlayer::initialize(){
    AGILE_LOGV("Enter!");
    
    mState          = ST_IDLE;
    mFlushBackState = ST_IDLE;
    mSeekBackState  = ST_IDLE;
    
    mLooper         = NULL;
    mMsgHandler     = NULL;

    mSource         = NULL;
    
    mParametersDB = createMagMiniDB(PARAMETERS_DB_SIZE);
        
    mFlushCompleteMQ   = Mag_CreateMsgQueue();
    mSeekCompleteMQ    = Mag_CreateMsgQueue();
    mPrepareCompleteMQ = Mag_CreateMsgQueue();

    Mag_CreateMutex(&mGetParamLock);
    Mag_CreateMutex(&mStateLock);

    Mag_CreateEventGroup(&mEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mCompletePrepareEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mCompletePrepareEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mCompleteSeekEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mCompleteSeekEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mCompleteFlushEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mCompleteFlushEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mErrorEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mErrorEvt);
    
    Mag_CreateEventScheduler(&mEventScheduler, MAG_EVT_SCHED_NORMAL);
    
    Mag_RegisterEventCallback(mEventScheduler, mCompletePrepareEvt, onCompletePrepareEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mCompleteSeekEvt, onCompleteSeekEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mCompleteFlushEvt, onCompleteFlushEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mErrorEvt, onErrorEvtCB, (void *)this);

    mSourceNotifyMsg    = createMessage(MagMsg_SourceNotify);
    mPrepareMsg         = createMessage(MagMsg_Prepare);
    mStartMsg           = createMessage(MagMsg_Start);
    mStopMsg            = createMessage(MagMsg_Stop);
    mPauseMsg           = createMessage(MagMsg_Pause);
    mFlushMsg           = createMessage(MagMsg_Flush);
    mSeekToMsg          = createMessage(MagMsg_SeekTo);
    mFastMsg            = createMessage(MagMsg_Fast);
    mResetMsg           = createMessage(MagMsg_Reset);
    mSetVolumeMsg       = createMessage(MagMsg_SetVolume);
    mErrorNotifyMsg     = createMessage(MagMsg_ErrorNotify);
    mComponentNotifyMsg = createMessage(MagMsg_ComponentNotify);
    mSetParametersMsg   = createMessage(MagMsg_SetParameters);
}

MagMessageHandle MagPlayer::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

void MagPlayer::cleanup(){
    mState          = ST_IDLE;
    mFlushBackState = ST_IDLE;
    mSeekBackState  = ST_IDLE;

    destroyMagMiniDB(mParametersDB);
        
    Mag_DestroyMsgQueue(mFlushCompleteMQ);
    Mag_DestroyMsgQueue(mSeekCompleteMQ);
    Mag_DestroyMsgQueue(mPrepareCompleteMQ);

    Mag_DestroyMutex(mStateLock);

    Mag_DestroyEvent(mCompletePrepareEvt);
    Mag_DestroyEvent(mCompleteSeekEvt);
    Mag_DestroyEvent(mCompleteFlushEvt);
    Mag_DestroyEvent(mErrorEvt);

    Mag_DestroyEventGroup(mEventGroup);
    Mag_DestroyEventScheduler(mEventScheduler);

    destroyMagMessage(mSourceNotifyMsg);
    destroyMagMessage(mPrepareMsg);
    destroyMagMessage(mStartMsg);
    destroyMagMessage(mStopMsg);
    destroyMagMessage(mPauseMsg);
    destroyMagMessage(mFlushMsg);
    destroyMagMessage(mSeekToMsg);
    destroyMagMessage(mFastMsg);
    destroyMagMessage(mResetMsg);
    destroyMagMessage(mSetVolumeMsg);
    destroyMagMessage(mErrorNotifyMsg);
    destroyMagMessage(mComponentNotifyMsg);
}

_status_t MagPlayer::getLooper(){
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

void MagPlayer::setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h){
    AGILE_LOGD("x = %d, y = %d, w = %d, h = %d", x, y, w, h);
}

_status_t MagPlayer::buildAudioPipeline(){
    return MAG_NO_ERROR;
}
_status_t MagPlayer::buildVideoPipeline(){
    return MAG_NO_ERROR;
}

#undef LOOPER_NAME


