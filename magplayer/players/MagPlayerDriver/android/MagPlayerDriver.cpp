#include "MagPlayerDriver.h"

MagPlayerDriver::MagPlayerDriver(void *client, notify_client_callback_f cb):
    mState(MPD_IDLE){

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

_status_t MagPlayerDriver::setDataSource(const sp<IStreamSource> &source){
    mState = MPD_INITIALIZED;
    return mpPlayer->setDataSource(source);
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

}

_status_t MagPlayerDriver::getParameter(int key, Parcel *reply){

}

_status_t MagPlayerDriver::invoke(const Parcel &request, Parcel *reply){

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

