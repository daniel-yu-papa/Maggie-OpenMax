#include "MagPlayer.h"

MagPlayer::MagPlayer(){
    initialize();
}

MagPlayer::~MagPlayer(){

}

void MagPlayer::onSourceNotify(MagMessageHandle msg){
    void *value;
    boolean ret;

    ret = msg->findPointer(msg, "source", &value);
    if (!ret){
        AGILE_LOGE("failed to find the source object!");
        return;
    }
    
    if (mSource == NULL){
        mSource = static_cast<MagPlayer_Component_CP *>value;
        mDemuxer = new MagPlayer_Demuxer_Base();
        mState = ST_INITIALIZED;
    }else{
        AGILE_LOGE("Failed! There is existed source(0x%x) and is still running...", mSource);
        delete static_cast<MagPlayer_Component_CP *>value;
    }
}

_status_t MagPlayer::setDataSource(const char *url, const MagMiniDBHandle settings){
    getLooper();

    MagMessageHandle msg = createMessage(MagMsg_SourceNotify);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

_status_t MagPlayer::setDataSource(i32 fd, i64 offset, i64 length){
    getLooper();

    MagMessageHandle msg = createMessage(MagMsg_SourceNotify);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

_status_t MagPlayer::setDataSource(StreamBufferUser *buffer){
    getLooper();
    MagPlayer_Component_CP *cp;

    cp = new MagPlayer_Component_CP();
    if (cp != NULL){
        cp->Create(buffer);
    }else{
       AGILE_LOGE("failed to allocate the object: MagPlayer_Component_CP");
       return MAG_NO_MEMORY;
    }
    MagMessageHandle msg = createMessage(MagMsg_SourceNotify);
    if (msg != NULL){
        msg->setPointer(msg, "source", static_cast<void *>(cp));
        postMessage(msg, 0);
    }
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
    if (mpDriver != NULL)
        mpDriver->ErrorEvtListener(what, extra);
}

void MagPlayer::onPrepared(MagMessageHandle msg){
    _status_t ret;
    
    if (ST_INITIALIZED != mState){
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }else{
        mState = ST_PREPARING;
        
        ret = mDemuxer->start(mSource, 0);

        if (ret == MAG_NO_ERROR){
            Mag_SetEvent(mCompletePrepareEvt);
        }else{
            MagMessageHandle msg = createMessage(MagMsg_ErrorNotify);
            if (msg != NULL){
                msg->setInt32(msg, "what", E_WHAT_DEMUXER);
                msg->setInt32(msg, "extra", ret);
                postMessage(msg, 0);
            }
        }
    }      
}

void MagPlayer::prepareAsync(){
    MagMessageHandle msg = createMessage(MagMsg_Prepare);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::onStart(MagMessageHandle msg){
    if ((ST_PREPARED == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
        mTrackTable = mDemuxer->getTrackInfoList();
        if (mTrackTable->videoTrackNum > 0){
            mDemuxer->setPlayingTrackID(0);
        }
        if (mTrackTable->audioTrackNum > 0){
            mDemuxer->setPlayingTrackID(mTrackTable->videoTrackNum);
        }
        if (mTrackTable->subtitleTrackNum > 0){
            mDemuxer->setPlayingTrackID(mTrackTable->videoTrackNum + mTrackTable->audioTrackNum);
        }
        mState = ST_RUNNING;
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

void MagPlayer::start(){
    MagMessageHandle msg = createMessage(MagMsg_Start);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::onStop(MagMessageHandle msg){
    if ((ST_RUNNING == mState) ||
        (ST_PAUSED == mState)  ||
        (ST_PLAYBACK_COMPLETED == mState)){
        /*todo*/
        mState = ST_STOPPED;
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

void MagPlayer::stop(){
    MagMessageHandle msg = createMessage(MagMsg_Stop);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::onPause(MagMessageHandle msg){
    if (ST_RUNNING == mState){
        /*todo*/
        mState = ST_PAUSED;
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

void MagPlayer::pause(){
    MagMessageHandle msg = createMessage(MagMsg_Pause);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::onResume(MagMessageHandle msg){
    if (ST_PAUSED == mState){
        /*todo*/
        mState = ST_RUNNING;
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

void MagPlayer::resume(){
    MagMessageHandle msg = createMessage(MagMsg_Resume);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::onFlush(MagMessageHandle msg){
    if (isValidFSState(mState)){
        mFlushBackState = mState;
        mState          = ST_FLUSHING;
        
        /*todo*/
        
        Mag_SetEvent(mCompleteFlushEvt);
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

void MagPlayer::flush(){
    MagMessageHandle msg = createMessage(MagMsg_Flush);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::onSeek(MagMessageHandle msg){
    if (isValidFSState(mState)){
        mSeekBackState = mState;
        mState         = ST_SEEKING;
        
        /*todo*/
        
        Mag_SetEvent(mCompleteSeekEvt);
    }else if (ST_FLUSHING == mState){
        mFlushCompleteMQ->put(mFlushCompleteMQ, msg);
    }else if (ST_SEEKING == mState){
        mSeekCompleteMQ->put(mSeekCompleteMQ, msg);
    }else{
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }
}

void MagPlayer::seekTo(ui32 msec){
    MagMessageHandle msg = createMessage(MagMsg_SeekTo);
    if (msg != NULL){
        postMessage(msg, 0);
    }
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

void MagPlayer::fast(i32 speed){
    MagMessageHandle msg = createMessage(MagMsg_Fast);
    if (msg != NULL){
        msg->setInt32(msg, "speed", speed);
        postMessage(msg, 0);
    }
}

void MagPlayer::onReset(MagMessageHandle msg){
    /*todo*/
    mState = ST_IDLE;
    if (NULL != mNotifyResetCompleteFn)
        mNotifyResetCompleteFn(mResetCompletePriv);
}

void MagPlayer::resetAsync(){
    MagMessageHandle msg = createMessage(MagMsg_Reset);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

void MagPlayer::setDriver(MagPlayerDriver *driver) {
    mpDriver = driver;
}

void MagPlayer::setResetCompleteListener(fnNotifyResetComplete fn, void *priv){
    if (NULL != fn){
        mNotifyResetCompleteFn = fn;
        mResetCompletePriv = priv;
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

void MagPlayer::setFlushCompleteListener(fnNotifyPrepareComplete fn, void *priv){
    if (NULL != fn){
        mNotifyFlushCompleteFn = fn;
        mFlushCompletePriv = priv;
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

void MagPlayer::setParameters(const char *name, MagParamType_t type, void *value){
    if (NULL != mParametersDB){
        switch (type){
            case MagParamTypeInt32:
                i32 v = static_cast<i32>(*value);
                mParametersDB->setInt32(mParametersDB, name, v);
                break;

            case MagParamTypeInt64:
                i64 v = static_cast<i64>(*value);
                mParametersDB->setInt64(mParametersDB, name, v);
                break;

            case MagParamTypeUInt32:
                ui32 v = static_cast<_size_t>(*value);
                mParametersDB->setSize(mParametersDB, name, v);
                break;

            case MagParamTypeFloat:
                fp32 v = static_cast<fp32>(*value);
                mParametersDB->setFloat(mParametersDB, name, v);
                break;

            case MagParamTypeDouble:
                fp64 v = static_cast<fp64>(*value);
                mParametersDB->setDouble(mParametersDB, name, v);
                break;

            case MagParamTypePointer:
                mParametersDB->setPointer(mParametersDB, name, value);
                break;

            case MagParamTypeString:
                char *v = static_cast<char *>(value);
                mParametersDB->setString(mParametersDB, name, v);
                break;

            default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                break;
        }
    }else{
        AGILE_LOGE("the parameter db is NOT initialized!");
    }
}

_status_t MagPlayer::getParameters(const char *name, MagParamType_t type, void **value){
    boolean result;
    _status_t ret = MAG_NO_ERROR;
    
    if (NULL != mParametersDB){
        switch (type){
            case MagParamTypeInt32:
                i32 v;
                result = mParametersDB->findInt32(mParametersDB, name, &v);
                if (result)
                    *value = v;
                else
                    ret = MAG_BAD_VALUE;
                break;

            case MagParamTypeInt64:
                i64 v;;
                result = mParametersDB->findInt64(mParametersDB, name, &v);
                if (result)
                    *value = v;
                else
                    ret = MAG_BAD_VALUE;
                break;

            case MagParamTypeUInt32:
                ui32 v;
                result = mParametersDB->findSize(mParametersDB, name, &v);
                if (result)
                    *value = v;
                else
                    ret = MAG_BAD_VALUE;
                break;

            case MagParamTypeFloat:
                fp32 v;
                result = mParametersDB->findFloat(mParametersDB, name, &v);
                if (result)
                    *value = v;
                else
                    ret = MAG_BAD_VALUE;
                break;

            case MagParamTypeDouble:
                fp64 v;;
                result = mParametersDB->findDouble(mParametersDB, name, &v);
                if (result)
                    *value = v;
                else
                    ret = MAG_BAD_VALUE;
                break;

            case MagParamTypePointer:
                result = mParametersDB->findPointer(mParametersDB, name, value);
                if (!result)
                    ret = MAG_BAD_VALUE;
                break;

            case MagParamTypeString:
                char **v;
                result = mParametersDB->findString(mParametersDB, name, v);
                if (result)
                    *value = (void *)*v;
                else
                    ret = MAG_BAD_VALUE;
                break;

            default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                break;
        }
    }else{
        AGILE_LOGE("the parameter db is NOT initialized!");
    }
}

bool MagPlayer::isValidFSState(State_t st){
    if ((ST_PREPARED == mState) ||
        (ST_RUNNING  == mState) ||
        (ST_FASTING  == mState) ||
        (ST_PAUSED   == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
        return true;
    }else{
        return false;
    }
}

void MagPlayer::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer thiz = (MagPlayer *)priv;
    
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

        case MagMsg_Resume:
            thiz->onResume(msg);
            break;

        case MagMsg_Flush:
            thiz->onFlush(msg);
            break;

        case MagMsg_SeekTo:
            thiz->onSeek(msg);
            break;
            
        case MagMsg_Error:
            thiz->onError(msg);
            break;
        default:
            break;
    }
}

void MagPlayer::onCompletePrepareEvtCB(void *priv){
    MagPlayer thiz = (MagPlayer *)priv;
    MagMessageHandle msg = NULL;

    do {
        mPrepareCompleteMQ->get(mPrepareCompleteMQ, &msg);
        if (NULL != msg){
            postMessage(msg, 0);
        }
    }while(msg);
    thiz->mState = ST_PREPARED;

    if (mpDriver != NULL)
        mpDriver->PrepareCompleteEvtListener();
}

void MagPlayer::onCompleteSeekEvtCB(void *priv){
    MagPlayer thiz = (MagPlayer *)priv;
    MagMessageHandle msg = NULL;

    do {
        mSeekCompleteMQ->get(mSeekCompleteMQ, &msg);
        if (NULL != msg){
            postMessage(msg, 0);
        }
    }while(msg);

    if (isValidFSState(thiz->mSeekBackState)){
        thiz->mState = thiz->mSeekBackState;
    }else{
        AGILE_LOGE("backstate:%d is not valid for seekTo operation. QUIT!", thiz->mSeekBackState);
    }
}

void MagPlayer::onCompleteFlushEvtCB(void *priv){
    MagPlayer thiz = (MagPlayer *)priv;
    MagMessageHandle msg = NULL;

    do {
        mFlushCompleteMQ->get(mFlushCompleteMQ, &msg);
        if (NULL != msg){
            postMessage(msg, 0);
        }
    }while(msg);

    if (isValidFSState(thiz->mFlushBackState)){
        thiz->mState = thiz->mFlushBackState;
        if (ST_RUNNING == thiz->mState){
            start();
        }
        if (mpDriver != NULL)
            mpDriver->FlushCompleteEvtListener();
    }else{
        AGILE_LOGE("backstate:%d is not valid for Flush operation. QUIT!", thiz->mFlushBackState);
    }
}

void MagPlayer::initialize(){
    mState          = ST_IDLE;
    mFlushBackState = ST_IDLE;
    mSeekBackState  = ST_IDLE;
    mLooper         = NULL;
    mMsgHandler     = NULL;   
    mTrackTable     = NULL;
    
    mNotifyResetCompleteFn = NULL;

    mParametersDB = createMagMiniDB(PARAMETERS_DB_SIZE);
        
    mFlushCompleteMQ   = Mag_CreateMsgQueue();
    mSeekCompleteMQ    = Mag_CreateMsgQueue();
    mPrepareCompleteMQ = Mag_CreateMsgQueue();

    Mag_CreateMutex(&mStateLock);

    Mag_CreateEventGroup(&mEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mCompletePrepareEvt, 0)){}
        Mag_AddEventGroup(mEventGroup, mCompletePrepareEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mCompleteSeekEvt, 0)){}
        Mag_AddEventGroup(mEventGroup, mCompleteSeekEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mCompleteFlushEvt, 0)){}
        Mag_AddEventGroup(mEventGroup, mCompleteFlushEvt);
    
    Mag_CreateEventScheduler(&mEventScheduler, MAG_EVT_SCHED_NORMAL);

    Mag_RegisterEventCallback(mEventScheduler, mCompletePrepareEvt, onCompletePrepareEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mCompleteSeekEvt, onCompleteSeekEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mCompleteFlushEvt, onCompleteFlushEvtCB, (void *)this);
}

MagMessageHandle MagPlayer::createMessage(ui32 what) {
    MagMessageHandle msg = mag_mallocz(sizeof(MagMessage_t));
    if (msg != NULL){
        msg->mWhat   = what;
        msg->mTarget = mMsgHandler->id(mMsgHandler);
    }
    return msg;
}

_status_t MagPlayer::postMessage(MagMessageHandle msg, ui64 delay){
    if ((NULL == mLooper) || (NULL == mMsgHandler)){
        AGILE_LOGE("Looper is not running correctly(looper=0x%p, handler=0x%p)", mLooper, mMsgHandler);
        return MAG_NO_INIT;
    }

    mLooper->postMessage(msg, delay);
    return MAG_NO_ERROR;
}

_status_t MagPlayer::getLooper(){
    if (NULL == mLooper){
        mLooper = createLooper(LOOPER_NAME);
    }
    
    if (NULL != mLooper){
        if (NULL == mMsgHandler){
            mMsgHandler = createHandler(mLooper, onMessageReceived, (void *)this);

            if (NULL != mMsgHandler){
                mLooper->registerHandler(mLooper, mMsgHandler);
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