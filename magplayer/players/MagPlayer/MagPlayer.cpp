#include "MagPlayer.h"

MagPlayer::MagPlayer(){
    initialize();
}

MagPlayer::~MagPlayer(){

}

void MagPlayer::onSourceNotify(MagMessageHandle msg){

    /*todo*/
    mState = ST_INITIALIZED;
}

_status_t MagPlayer::setDataSource(const char *url, const void* parameters){
    getLooper();

    MagMessageHandle msg = createMessage(MagMsg_SourceNotify);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

_status_t MagPlayer::setDataSource(int fd, i64 offset, i64 length){
    getLooper();

    MagMessageHandle msg = createMessage(MagMsg_SourceNotify);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

_status_t MagPlayer::setDataSource(MagBufferHandle buffer){
    getLooper();

    MagMessageHandle msg = createMessage(MagMsg_SourceNotify);
    if (msg != NULL){
        postMessage(msg, 0);
    }
}

_status_t MagPlayer::setDataSource(MAG_EXTERNAL_SOURCE source){
    mExtSource = source;
}

void MagPlayer::onPrepared(MagMessageHandle msg){
    if (ST_INITIALIZED != mState){
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }else{
        mState = ST_PREPARING;
        
        /*todo*/

        Mag_SetEvent(mCompletePrepareEvt);
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

void MagPlayer::getParameters(const char *name, MagParamType_t type, void **value){
    if (NULL != mParametersDB){
        switch (type){
            case MagParamTypeInt32:
                i32 v;
                mParametersDB->findInt32(mParametersDB, name, &v);
                *value = v;
                break;

            case MagParamTypeInt64:
                i64 v;;
                mParametersDB->findInt64(mParametersDB, name, &v);
                *value = v;
                break;

            case MagParamTypeUInt32:
                ui32 v;
                mParametersDB->findSize(mParametersDB, name, &v);
                *value = v;
                break;

            case MagParamTypeFloat:
                fp32 v;
                mParametersDB->findFloat(mParametersDB, name, &v);
                *value = v;
                break;

            case MagParamTypeDouble:
                fp64 v;;
                mParametersDB->findDouble(mParametersDB, name, &v);
                *value = v;
                break;

            case MagParamTypePointer:
                mParametersDB->findPointer(mParametersDB, name, value);
                break;

            case MagParamTypeString:
                char **v;
                mParametersDB->findString(mParametersDB, name, v);
                *value = (void *)*v;
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

    if (mNotifyPrepareCompleteFn != NULL)
        mNotifyPrepareCompleteFn(mPrepareCompletePriv);
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
        if (NULL != mNotifyFlushCompleteFn)
            mNotifyFlushCompleteFn(mFlushCompletePriv);
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
    mExtSource      = MAG_EXTERNAL_SOURCE_Unused;
    
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

