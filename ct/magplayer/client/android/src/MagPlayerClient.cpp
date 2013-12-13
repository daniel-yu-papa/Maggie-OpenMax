#include "MagPlayerClient.h"

namespace android {

MagPlayerClient::MagPlayerClient()
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

// always call with lock held
void MagPlayerClient::clear_l()
{
    mCurrentPosition = -1;
    mSeekPosition = -1;
    mVideoWidth = mVideoHeight = 0;
}

status_t MagPlayerClient::setListener(const sp<MagPlayerListener>& listener)
{
    AGILE_LOGV("setListener");
    Mutex::Autolock _l(mLock);
    mListener = listener;
    return NO_ERROR;
}

status_t MagPlayerClient::attachNewPlayer(const sp<IMagPlayerClient>& player)
{
    status_t err = UNKNOWN_ERROR;
    sp<IMagPlayerClient> p;
    { // scope for the lock
        Mutex::Autolock _l(mLock);

        if ( !( (mCurrentState & MAG_PLAYER_IDLE) ||
                (mCurrentState == MAG_PLAYER_STATE_ERROR ) ) ) {
            AGILE_LOGE("attachNewPlayer called in state %d", mCurrentState);
            return INVALID_OPERATION;
        }

        clear_l();
        p = mPlayer;
        mPlayer = player;
        if (player != 0) {
            mCurrentState = MAG_PLAYER_INITIALIZED;
            err = NO_ERROR;
        } else {
            AGILE_LOGE("Unable to to create media player");
        }
    }

    if (p != 0) {
        p->disconnect();
    }

    return err;
}

status_t MagPlayerClient::setDataSource(
        const char *url, const KeyedVector<String8, String8> *headers)
{
    AGILE_LOGV("setDataSource(%s)", url);
    status_t err = BAD_VALUE;
    if (url != NULL) {
        const sp<IMagPlayerService>& service(getMagPlayerService());
        if (service != 0) {
            sp<IMagPlayerClient> player(service->create(getpid(), this));
            if (NO_ERROR != player->setDataSource(url, headers)) {
                player.clear();
            }
            err = attachNewPlayer(player);
        }
    }
    return err;
}

status_t MagPlayerClient::setDataSource(int fd, int64_t offset, int64_t length)
{
    AGILE_LOGV("setDataSource(fd:%d, offset:%lld, length:%lld)", fd, offset, length);
    status_t err = BAD_VALUE;
    if (url != NULL) {
        const sp<IMagPlayerService>& service(getMagPlayerService());
        if (service != 0) {
            sp<IMagPlayerClient> player(service->create(getpid(), this));
            if (NO_ERROR != player->setDataSource(fd, offset, length)) {
                player.clear();
            }
            err = attachNewPlayer(player);
        }
    }
    return err;
}

status_t MagPlayerClient::setDataSource(const sp<IStreamSource> &source){
    AGILE_LOGV("setDataSource");
    status_t err = BAD_VALUE;
    if (url != NULL) {
        const sp<IMagPlayerService>& service(getMagPlayerService());
        if (service != 0) {
            sp<IMagPlayerClient> player(service->create(getpid(), this));
            if (NO_ERROR != player->setDataSource(source)) {
                player.clear();
            }
            err = attachNewPlayer(player);
        }
    }
    return err;
}

status_t MagPlayerClient::invoke(const Parcel& request, Parcel *reply)
{
    Mutex::Autolock _l(mLock);
    const bool hasBeenInitialized =
            (mCurrentState != MAG_PLAYER_STATE_ERROR) &&
            ((mCurrentState & MAG_PLAYER_IDLE) != MAG_PLAYER_IDLE);
    if ((mPlayer != NULL) && hasBeenInitialized) {
        AGILE_LOGV("invoke %d", request.dataSize());
        return  mPlayer->invoke(request, reply);
    }
    AGILE_LOGE("invoke failed: wrong state %X", mCurrentState);
    return INVALID_OPERATION;
}

status_t MagPlayerClient::setVideoSurfaceTexture(
        const sp<ISurfaceTexture>& surfaceTexture)
{
    AGILE_LOGV("setVideoSurfaceTexture");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return NO_INIT;
    return mPlayer->setVideoSurfaceTexture(surfaceTexture);
}

// must call with lock held
status_t MagPlayerClient::prepareAsync_l()
{
    if ( (mPlayer != 0) && ( mCurrentState & ( MAG_PLAYER_INITIALIZED | MAG_PLAYER_STOPPED) ) ) {
        mCurrentState = MAG_PLAYER_PREPARING;
        return mPlayer->prepareAsync();
    }
    AGILE_LOGE("prepareAsync called in state %d", mCurrentState);
    return INVALID_OPERATION;
}

// TODO: In case of error, prepareAsync provides the caller with 2 error codes,
// one defined in the Android framework and one provided by the implementation
// that generated the error. The sync version of prepare returns only 1 error
// code.
status_t MagPlayerClient::prepare()
{
    AGILE_LOGV("prepare");
    Mutex::Autolock _l(mLock);
    mLockThreadId = getThreadId();
    if (mPrepareSync) {
        mLockThreadId = 0;
        return -EALREADY;
    }
    mPrepareSync = true;
    status_t ret = prepareAsync_l();
    if (ret != NO_ERROR) {
        mLockThreadId = 0;
        return ret;
    }

    if (mPrepareSync) {
        mSignal.wait(mLock);  // wait for prepare done
        mPrepareSync = false;
    }
    AGILE_LOGV("prepare complete - status=%d", mPrepareStatus);
    mLockThreadId = 0;
    return mPrepareStatus;
}

status_t MagPlayerClient::prepareAsync()
{
    AGILE_LOGV("prepareAsync");
    Mutex::Autolock _l(mLock);
    return prepareAsync_l();
}

status_t MagPlayerClient::start()
{
    AGILE_LOGV("start");
    Mutex::Autolock _l(mLock);
    if (mCurrentState & MAG_PLAYER_STARTED)
        return NO_ERROR;
    
    if ( (mPlayer != 0) && ( mCurrentState & ( MAG_PLAYER_PREPARED |
                    MAG_PLAYER_PLAYBACK_COMPLETE | MAG_PLAYER_PAUSED ) ) ) {
        mPlayer->setLooping(mLoop);
        mPlayer->setVolume(mLeftVolume, mRightVolume);
        mCurrentState = MAG_PLAYER_STARTED;
        status_t ret = mPlayer->start();
        if (ret != NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        } else {
            if (mCurrentState == MAG_PLAYER_PLAYBACK_COMPLETE) {
                AGILE_LOGV("playback completed immediately following start()");
            }
        }
        return ret;
    }
    AGILE_LOGE("start called in state %d", mCurrentState);
    return INVALID_OPERATION;
}

status_t MagPlayerClient::stop()
{
    AGILE_LOGV("stop");
    Mutex::Autolock _l(mLock);
    if (mCurrentState & MAG_PLAYER_STOPPED) return NO_ERROR;
    if ( (mPlayer != 0) && ( mCurrentState & ( MAG_PLAYER_STARTED | MAG_PLAYER_PREPARED |
                    MAG_PLAYER_PAUSED | MAG_PLAYER_PLAYBACK_COMPLETE ) ) ) {
        status_t ret = mPlayer->stop();
        if (ret != NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        } else {
            mCurrentState = MAG_PLAYER_STOPPED;
        }
        return ret;
    }
    ALOGE("stop called in state %d", mCurrentState);
    return INVALID_OPERATION;
}

status_t MagPlayerClient::pause()
{
    AGILE_LOGV("pause");
    Mutex::Autolock _l(mLock);
    if (mCurrentState & (MAG_PLAYER_PAUSED | MAG_PLAYER_PLAYBACK_COMPLETE))
        return NO_ERROR;
    if ((mPlayer != 0) && (mCurrentState & MAG_PLAYER_STARTED)) {
        status_t ret = mPlayer->pause();
        if (ret != NO_ERROR) {
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        } else {
            mCurrentState = MAG_PLAYER_PAUSED;
        }
        return ret;
    }
    AGILE_LOGE("pause called in state %d", mCurrentState);
    return INVALID_OPERATION;
}

bool MagPlayerClient::isPlaying()
{
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        bool temp = false;
        mPlayer->isPlaying(&temp);
        AGILE_LOGV("isPlaying: %d", temp);
        if ((mCurrentState & MAG_PLAYER_STARTED) && ! temp) {
            AGILE_LOGE("internal/external state mismatch corrected");
            mCurrentState = MAG_PLAYER_PAUSED;
        }
        return temp;
    }
    AGILE_LOGV("isPlaying: no active player");
    return false;
}

status_t MagPlayerClient::getVideoWidth(int *w)
{
    AGILE_LOGV("getVideoWidth");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    *w = mVideoWidth;
    return NO_ERROR;
}

status_t MagPlayerClient::getVideoHeight(int *h)
{
    AGILE_LOGV("getVideoHeight");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    *h = mVideoHeight;
    return NO_ERROR;
}

status_t MagPlayerClient::getCurrentPosition(int *msec)
{
    AGILE_LOGV("getCurrentPosition");
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        if (mCurrentPosition >= 0) {
            AGILE_LOGV("Using cached seek position: %d", mCurrentPosition);
            *msec = mCurrentPosition;
            return NO_ERROR;
        }
        return mPlayer->getCurrentPosition(msec);
    }
    return INVALID_OPERATION;
}

status_t MagPlayerClient::getDuration_l(int *msec)
{
    AGILE_LOGV("getDuration_l");
    bool isValidState = (mCurrentState & (MAG_PLAYER_PREPARED | MAG_PLAYER_STARTED | MAG_PLAYER_PAUSED | MAG_PLAYER_STOPPED | MAG_PLAYER_PLAYBACK_COMPLETE));
    if (mPlayer != 0 && isValidState) {
        int durationMs;
        status_t ret = mPlayer->getDuration(&durationMs);
        if (msec) {
            *msec = durationMs;
        }
        return ret;
    }
    AGILE_LOGE("Attempt to call getDuration without a valid mediaplayer");
    return INVALID_OPERATION;
}

status_t MagPlayerClient::getDuration(int *msec)
{
    Mutex::Autolock _l(mLock);
    return getDuration_l(msec);
}

status_t MagPlayerClient::seekTo_l(int msec)
{
    AGILE_LOGV("seekTo %d", msec);
    if ((mPlayer != 0) && ( mCurrentState & ( MAG_PLAYER_STARTED | MAG_PLAYER_PREPARED | MAG_PLAYER_PAUSED |  MAG_PLAYER_PLAYBACK_COMPLETE) ) ) {
        if ( msec < 0 ) {
            AGILE_LOGW("Attempt to seek to invalid position: %d", msec);
            msec = 0;
        }

        int durationMs;
        status_t err = mPlayer->getDuration(&durationMs);

        if (err != OK) {
            AGILE_LOGW("Stream has no duration and is therefore not seekable.");
            return err;
        }

        if (msec > durationMs) {
            AGILE_LOGW("Attempt to seek to past end of file: request = %d, "
                  "durationMs = %d",
                  msec,
                  durationMs);

            msec = durationMs;
        }

        // cache duration
        mCurrentPosition = msec;
        if (mSeekPosition < 0) {
            mSeekPosition = msec;
            return mPlayer->seekTo(msec);
        }
        else {
            AGILE_LOGV("Seek in progress - queue up seekTo[%d]", msec);
            return NO_ERROR;
        }
    }
    AGILE_LOGE("Attempt to perform seekTo in wrong state: mPlayer=%p, mCurrentState=%u", mPlayer.get(), mCurrentState);
    return INVALID_OPERATION;
}


status_t MagPlayerClient::seekTo(int msec)
{
    mLockThreadId = getThreadId();
    Mutex::Autolock _l(mLock);
    status_t result = seekTo_l(msec);
    mLockThreadId = 0;

    return result;
}

status_t MagPlayerClient::reset_l()
{
    mLoop = false;
    if (mCurrentState == MAG_PLAYER_IDLE) return NO_ERROR;
    mPrepareSync = false;
    if (mPlayer != 0) {
        status_t ret = mPlayer->reset();
        if (ret != NO_ERROR) {
            AGILE_LOGE("reset() failed with return code (%d)", ret);
            mCurrentState = MAG_PLAYER_STATE_ERROR;
        } else {
            mCurrentState = MAG_PLAYER_IDLE;
        }
        // setDataSource has to be called again to create a
        // new mediaplayer.
        mPlayer = 0;
        return ret;
    }
    clear_l();
    return NO_ERROR;
}

status_t MagPlayerClient::reset()
{
    AGILE_LOGV("reset");
    Mutex::Autolock _l(mLock);
    return reset_l();
}

status_t MagPlayerClient::setLooping(int loop)
{
    AGILE_LOGV("MagPlayer::setLooping");
    Mutex::Autolock _l(mLock);
    mLoop = (loop != 0);
    if (mPlayer != 0) {
        return mPlayer->setLooping(loop);
    }
    return OK;
}

bool MagPlayerClient::isLooping() {
    AGILE_LOGV("isLooping");
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        return mLoop;
    }
    AGILE_LOGV("isLooping: no active player");
    return false;
}

status_t MagPlayerClient::setVolume(float leftVolume, float rightVolume)
{
    AGILE_LOGV("MagPlayer::setVolume(%f, %f)", leftVolume, rightVolume);
    Mutex::Autolock _l(mLock);
    mLeftVolume = leftVolume;
    mRightVolume = rightVolume;
    if (mPlayer != 0) {
        return mPlayer->setVolume(leftVolume, rightVolume);
    }
    return OK;
}

status_t MagPlayerClient::setParameter(int key, const Parcel& request)
{
    AGILE_LOGV("MediaPlayer::setParameter(%d)", key);
    Mutex::Autolock _l(mLock);
    if (mPlayer != NULL) {
        return  mPlayer->setParameter(key, request);
    }
    AGILE_LOGV("setParameter: no active player");
    return INVALID_OPERATION;
}

status_t MagPlayerClient::getParameter(int key, Parcel *reply)
{
    AGILE_LOGV("MediaPlayer::getParameter(%d)", key);
    Mutex::Autolock _l(mLock);
    if (mPlayer != NULL) {
        return  mPlayer->getParameter(key, reply);
    }
    AGILE_LOGV("getParameter: no active player");
    return INVALID_OPERATION;
}

void MagPlayerClient::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    AGILE_LOGV("message received msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
    bool send = true;
    bool locked = false;

    // TODO: In the future, we might be on the same thread if the app is
    // running in the same process as the media server. In that case,
    // this will deadlock.
    //
    // The threadId hack below works around this for the care of prepare
    // and seekTo within the same process.
    // FIXME: Remember, this is a hack, it's not even a hack that is applied
    // consistently for all use-cases, this needs to be revisited.
    if (mLockThreadId != getThreadId()) {
        mLock.lock();
        locked = true;
    }

    // Allows calls from JNI in idle state to notify errors
    if (!(msg == MEDIA_ERROR && mCurrentState == MAG_PLAYER_IDLE) && mPlayer == 0) {
        AGILE_LOGV("notify(%d, %d, %d) callback on disconnected mediaplayer", msg, ext1, ext2);
        if (locked) mLock.unlock();   // release the lock when done.
        return;
    }

    switch (msg) {
    case MEDIA_NOP: // interface test message
        break;
    case MEDIA_PREPARED:
        AGILE_LOGV("prepared");
        mCurrentState = MAG_PLAYER_PREPARED;
        if (mPrepareSync) {
            AGILE_LOGV("signal application thread");
            mPrepareSync = false;
            mPrepareStatus = NO_ERROR;
            mSignal.signal();
        }
        break;
    case MEDIA_PLAYBACK_COMPLETE:
        AGILE_LOGV("playback complete");
        if (mCurrentState == MAG_PLAYER_IDLE) {
            AGILE_LOGE("playback complete in idle state");
        }
        if (!mLoop) {
            mCurrentState = MAG_PLAYER_PLAYBACK_COMPLETE;
        }
        break;
    case MEDIA_ERROR:
        // Always log errors.
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        AGILE_LOGE("error (%d, %d)", ext1, ext2);
        mCurrentState = MAG_PLAYER_STATE_ERROR;
        if (mPrepareSync)
        {
            AGILE_LOGV("signal application thread");
            mPrepareSync = false;
            mPrepareStatus = ext1;
            mSignal.signal();
            send = false;
        }
        break;
    case MEDIA_INFO:
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        if (ext1 != MEDIA_INFO_VIDEO_TRACK_LAGGING) {
            AGILE_LOGW("info/warning (%d, %d)", ext1, ext2);
        }
        break;
    case MEDIA_SEEK_COMPLETE:
        AGILE_LOGV("Received seek complete");
        if (mSeekPosition != mCurrentPosition) {
            AGILE_LOGV("Executing queued seekTo(%d)", mSeekPosition);
            mSeekPosition = -1;
            seekTo_l(mCurrentPosition);
        }
        else {
            AGILE_LOGV("All seeks complete - return to regularly scheduled program");
            mCurrentPosition = mSeekPosition = -1;
        }
        break;
    case MEDIA_BUFFERING_UPDATE:
        AGILE_LOGV("buffering %d", ext1);
        break;
    case MEDIA_SET_VIDEO_SIZE:
        AGILE_LOGV("New video size %d x %d", ext1, ext2);
        mVideoWidth = ext1;
        mVideoHeight = ext2;
        break;
    case MEDIA_TIMED_TEXT:
        AGILE_LOGV("Received timed text message");
        break;
    default:
        AGILE_LOGV("unrecognized message: (%d, %d, %d)", msg, ext1, ext2);
        break;
    }

    sp<MediaPlayerListener> listener = mListener;
    if (locked) mLock.unlock();

    // this prevents re-entrant calls into client code
    if ((listener != 0) && send) {
        Mutex::Autolock _l(mNotifyLock);
        AGILE_LOGV("callback application");
        listener->notify(msg, ext1, ext2, obj);
        AGILE_LOGV("back from callback");
    }
}

void MagPlayerClient::died()
{
    AGILE_LOGV("died");
    notify(MEDIA_ERROR, MEDIA_ERROR_SERVER_DIED, 0);
}



};
