#include "MagPlayerClient.h"

MagPlayerClient::MagPlayerClient():
                         mCurrentState(MAG_PLAYER_IDLE),
                         mpListener(NULL)
{
}

MagPlayerClient::~MagPlayerClient()
{

}

void MagPlayerClient::disconnect()
{
    AGILE_LOGV("disconnect")

    sp<IMagPlayerClient> p;
    {
        Mutex::Autolock _l(mLock);
        p = mPlayer;
        mPlayer.clear();
    }

    if (p != 0) {
        p->disconnect();
    }
}

_status_t MagPlayerClient::setListener(const MagPlayerListener* listener)
{
    AGILE_LOGV("setListener");
    Mutex::Autolock _l(mLock);
    mpListener = listener;
    return MAG_NO_ERROR;
}

_status_t MagPlayerClient::attachNewPlayer(const sp<IMagPlayerClient>& player)
{
    _status_t err = MAG_UNKNOWN_ERROR;
    sp<IMagPlayerClient> p;
    { // scope for the lock
        Mutex::Autolock _l(mLock);

        if ( !( (mCurrentState & MAG_PLAYER_IDLE) ||
                (mCurrentState == MAG_PLAYER_STATE_ERROR ) ) ) {
            AGILE_LOGE("attachNewPlayer called in state %d", mCurrentState);
            return MAG_INVALID_OPERATION;
        }

        p = mPlayer;
        mPlayer = player;
        if (player != 0) {
            mCurrentState = MAG_PLAYER_INITIALIZED;
            err = MAG_NO_ERROR;
        } else {
            AGILE_LOGE("Unable to to create media player");
        }
    }

    if (p != 0) {
        p->disconnect();
    }

    return err;
}

_status_t MagPlayerClient::setDataSource(const char *url)
{
    AGILE_LOGV("setDataSource(%s)", url);
    _status_t err = MAG_BAD_VALUE;
    
    if (url != NULL) {
        const sp<IMagPlayerService>& service(getMagPlayerService());
        if (service != 0) {
            sp<IMagPlayerClient> player(service->create(getpid(), this));
            if (MAG_NO_ERROR != player->setDataSource(url)) {
                player.clear();
            }
            err = attachNewPlayer(player);
        }
    }
    return err;
}

_status_t MagPlayerClient::setDataSource(i32 fd, i64 offset, i64 length)
{
    AGILE_LOGV("setDataSource(fd:%d, offset:%lld, length:%lld)", fd, offset, length);
    _status_t err = MAG_BAD_VALUE;
    
    const sp<IMagPlayerService>& service(getMagPlayerService());
    if (service != 0) {
        sp<IMagPlayerClient> player(service->create(getpid(), this));
        if (MAG_NO_ERROR != player->setDataSource(fd, offset, length)) {
            player.clear();
        }
        err = attachNewPlayer(player);
    }
    return err;
}

_status_t MagPlayerClient::setDataSource(const sp<IStreamBuffer> &source){
    AGILE_LOGV("setDataSource");
    _status_t err = MAG_BAD_VALUE;

    const sp<IMagPlayerService>& service(getMagPlayerService());
    if (service != 0) {
        sp<IMagPlayerClient> player(service->create(getpid(), this));
        if (MAG_NO_ERROR != player->setDataSource(source)) {
            player.clear();
        }
        err = attachNewPlayer(player);
    }
    return err;
}

_status_t MagPlayerClient::invoke(const Parcel& request, Parcel *reply)
{
    Mutex::Autolock _l(mLock);
    const bool hasBeenInitialized = (mCurrentState == MAG_PLAYER_RUNNING);
    if ((mPlayer != NULL) && hasBeenInitialized) {
        AGILE_LOGV("invoke %d", request.dataSize());
        return  mPlayer->invoke(request, reply);
    }
    AGILE_LOGE("invoke failed: wrong state %X", mCurrentState);
    return MAG_INVALID_OPERATION;
}

_status_t MagPlayerClient::setVideoSurfaceTexture(
        const sp<ISurfaceTexture>& surfaceTexture)
{
    AGILE_LOGV("setVideoSurfaceTexture");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return MAG_NO_INIT;
    return mPlayer->setVideoSurfaceTexture(surfaceTexture);
}

_status_t MagPlayerClient::prepare()
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("prepare");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_INITIALIZED ) ) {
        ret = mPlayer->prepare();
        if (ret == MAG_NO_ERROR)
            mCurrentState = MAG_PLAYER_RUNNING;
    }
    return ret;
}

_status_t MagPlayerClient::prepareAsync()
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("prepareAsync");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_INITIALIZED ) ) {
        ret = mPlayer->prepareAsync();
        if (ret == MAG_NO_ERROR)
            mCurrentState = MAG_PLAYER_RUNNING;
    }
    return ret;
}

_status_t MagPlayerClient::start()
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("start");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->start();
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::stop()
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("stop");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->stop();
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::pause()
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("pause");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->pause();
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        } 
    }
    return ret;
}

_status_t MagPlayerClient::isPlaying(bool* state)
{   
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("isPlaying");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->isPlaying(state);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::getCurrentPosition(int *msec)
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("getCurrentPosition");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->getCurrentPosition(msec);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::getDuration(int *msec)
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("getDuration");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->getDuration(msec);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::seekTo(int msec)
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("seekTo");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->seekTo(msec);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::fast(int multiple){
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("fast(%dX)", multiple);
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->fast(multiple);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::flush(){
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("flush()");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->flush();
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::reset()
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("reset");
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->reset();
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }else{
            mCurrentState = MAG_PLAYER_IDLE;
        }
    }
    return ret;
}

_status_t MagPlayerClient::setVolume(float leftVolume, float rightVolume)
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("setVolume(l:%f, r:%f)", leftVolume, rightVolume);
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->setVolume(leftVolume, rightVolume);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::setParameter(int key, const Parcel& request)
{
   _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("setParameter(key:%d)", key);
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->setParameter(key, request);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

_status_t MagPlayerClient::getParameter(int key, Parcel *reply)
{
    _status_t ret = MAG_UNKNOWN_ERROR;
    
    AGILE_LOGV("getParameter(key:%d)", key);
    Mutex::Autolock _l(mLock);
    if ( (mPlayer != 0) && ( mCurrentState & MAG_PLAYER_RUNNING ) ) {
        _status_t ret = mPlayer->getParameter(key, reply);
        if (ret != MAG_NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        }
    }
    return ret;
}

void MagPlayerClient::notify(int msg, int ext1, int ext2)
{
    AGILE_LOGV("message received msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);

    // Allows calls from JNI in idle state to notify errors
    if (!(msg == MEDIA_ERROR && mCurrentState == MAG_PLAYER_IDLE) && mPlayer == 0) {
        AGILE_LOGV("notify(%d, %d, %d) callback on disconnected mediaplayer", msg, ext1, ext2);
        return;
    }

    switch (msg) {
    case MEDIA_NOP: // interface test message
        break;
    
    case MEDIA_PLAYBACK_COMPLETE:
        AGILE_LOGV("playback complete");
        break;
        
    case MEDIA_ERROR:
        // Always log errors.
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        AGILE_LOGE("error (%d, %d)", ext1, ext2);
        mCurrentState = MAG_PLAYER_STATE_ERROR;
        break;
        
    case MEDIA_INFO:
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        if (ext1 != MEDIA_INFO_VIDEO_TRACK_LAGGING) {
            AGILE_LOGW("info/warning (%d, %d)", ext1, ext2);
        }
        break;
    
    case MEDIA_BUFFERING_UPDATE:
        AGILE_LOGV("buffering %d", ext1);
        break;
    
    default:
        AGILE_LOGV("unrecognized message: (%d, %d, %d)", msg, ext1, ext2);
        break;
    }

    MagPlayerListener *listener = mpListener;

    // this prevents re-entrant calls into client code
    if (listener != 0) {
        Mutex::Autolock _l(mNotifyLock);
        AGILE_LOGV("callback application");
        listener->notify(msg, ext1, ext2);
        AGILE_LOGV("back from callback");
    }
}

void MagPlayerClient::died()
{
    AGILE_LOGV("died");
    notify(MEDIA_ERROR, MEDIA_ERROR_SERVER_DIED, 0);
}

