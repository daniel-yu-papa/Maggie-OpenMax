#ifndef __MAGPLAYER_BASE_H__
#define __MAGPLAYER_BASE_H__


#include <binder/IMemory.h>
#include <media/IMediaPlayerClient.h>
#include <media/IMediaPlayer.h>
#include <media/IMediaDeathNotifier.h>
#include <media/IStreamSource.h>

#include <utils/KeyedVector.h>
#include <utils/String8.h>

#include "IMagPlayerClient.h"

class Surface;
class ISurfaceTexture;

enum media_event_type {
    MEDIA_NOP               = 0, // interface test message
    MEDIA_PREPARED          = 1,
    MEDIA_PLAYBACK_COMPLETE = 2,
    MEDIA_BUFFERING_UPDATE  = 3,
    MEDIA_SEEK_COMPLETE     = 4,
    MEDIA_SET_VIDEO_SIZE    = 5,
    MEDIA_TIMED_TEXT        = 99,
    MEDIA_ERROR             = 100,
    MEDIA_INFO              = 200,
};

enum mag_player_states {
    MAG_PLAYER_STATE_ERROR        = 0,
    MAG_PLAYER_IDLE               = 1 << 0,
    MAG_PLAYER_INITIALIZED        = 1 << 1,
    MAG_PLAYER_PREPARING          = 1 << 2,
    MAG_PLAYER_PREPARED           = 1 << 3,
    MAG_PLAYER_STARTED            = 1 << 4,
    MAG_PLAYER_PAUSED             = 1 << 5,
    MAG_PLAYER_STOPPED            = 1 << 6,
    MAG_PLAYER_PLAYBACK_COMPLETE  = 1 << 7
};

// Info and warning codes for the media player framework.  These are non fatal,
// the playback is going on but there might be some user visible issues.
//
// Info and warning messages are communicated back to the client using the
// MediaPlayerListener::notify method defined below.  In this situation,
// 'notify' is invoked with the following:
//   'msg' is set to MEDIA_INFO.
//   'ext1' should be a value from the enum media_info_type.
//   'ext2' contains an implementation dependant info code to provide
//          more details. Should default to 0 when not used.
//
// The codes are distributed as follow:
//   0xx: Reserved
//   7xx: Android Player info/warning (e.g player lagging behind.)
//   8xx: Media info/warning (e.g media badly interleaved.)
//
enum media_info_type {
    // 0xx
    MEDIA_INFO_UNKNOWN = 1,
    // The player was started because it was used as the next player for another
    // player, which just completed playback
    MEDIA_INFO_STARTED_AS_NEXT = 2,
    // The player just pushed the very first video frame for rendering
    MEDIA_INFO_RENDERING_START = 3,
    // 7xx
    // The video is too complex for the decoder: it can't decode frames fast
    // enough. Possibly only the audio plays fine at this stage.
    MEDIA_INFO_VIDEO_TRACK_LAGGING = 700,
    // MediaPlayer is temporarily pausing playback internally in order to
    // buffer more data.
    MEDIA_INFO_BUFFERING_START = 701,
    // MediaPlayer is resuming playback after filling buffers.
    MEDIA_INFO_BUFFERING_END = 702,
    // Bandwidth in recent past
    MEDIA_INFO_NETWORK_BANDWIDTH = 703,

    // 8xx
    // Bad interleaving means that a media has been improperly interleaved or not
    // interleaved at all, e.g has all the video samples first then all the audio
    // ones. Video is playing but a lot of disk seek may be happening.
    MEDIA_INFO_BAD_INTERLEAVING = 800,
    // The media is not seekable (e.g live stream).
    MEDIA_INFO_NOT_SEEKABLE = 801,
    // New media metadata is available.
    MEDIA_INFO_METADATA_UPDATE = 802,

    //9xx
    MEDIA_INFO_TIMED_TEXT_ERROR = 900,
};

// Generic error codes for the media player framework.  Errors are fatal, the
// playback must abort.
//
// Errors are communicated back to the client using the
// MediaPlayerListener::notify method defined below.
// In this situation, 'notify' is invoked with the following:
//   'msg' is set to MEDIA_ERROR.
//   'ext1' should be a value from the enum media_error_type.
//   'ext2' contains an implementation dependant error code to provide
//          more details. Should default to 0 when not used.
//
// The codes are distributed as follow:
//   0xx: Reserved
//   1xx: Android Player errors. Something went wrong inside the MediaPlayer.
//   2xx: Media errors (e.g Codec not supported). There is a problem with the
//        media itself.
//   3xx: Runtime errors. Some extraordinary condition arose making the playback
//        impossible.
//
enum media_error_type {
    // 0xx
    MEDIA_ERROR_UNKNOWN = 1,
    // 1xx
    MEDIA_ERROR_SERVER_DIED = 100,
    // 2xx
    MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200,
    // 3xx
};


// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class MagPlayerListener: virtual public RefBase
{
public:
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj) = 0;
};

class MagPlayerClient : public BnMagPlayerClient,
                           public virtual IMagPlayerDeathNotifier
{
public:
    MagPlayerClient();
    ~MagPlayerClient();

        void            died();
        void            disconnect();

        status_t        setDataSource(
                        const char *url,
                        const KeyedVector<String8, String8> *headers);

        status_t        setDataSource(int fd, int64_t offset, int64_t length);
        status_t        setDataSource(const sp<IStreamSource> &source);
        status_t        setVideoSurfaceTexture(
                                        const sp<ISurfaceTexture>& surfaceTexture);
        status_t        setListener(const sp<MagPlayerListener>& listener);
        status_t        prepare();
        status_t        prepareAsync();
        status_t        start();
        status_t        stop();
        status_t        pause();
        bool            isPlaying();
        status_t        getVideoWidth(int *w);
        status_t        getVideoHeight(int *h);
        status_t        seekTo(int msec);
        status_t        getCurrentPosition(int *msec);
        status_t        getDuration(int *msec);
        status_t        reset();
private:
        void            clear_l();
        status_t        seekTo_l(int msec);
        status_t        prepareAsync_l();
        status_t        getDuration_l(int *msec);
        status_t        attachNewPlayer(const sp<IMagPlayerClient>& player);
        status_t        reset_l();


    sp<IMagPlayerClient>          mPlayer;
    thread_id_t                 mLockThreadId;
    Mutex                       mLock;
    Mutex                       mNotifyLock;
    Condition                   mSignal;
    sp<MagPlayerListener>       mListener;
    void*                       mCookie;
    mag_player_states           mCurrentState;
    int                         mCurrentPosition;
    int                         mSeekPosition;
    bool                        mPrepareSync;
    status_t                    mPrepareStatus;
    bool                        mLoop;
    float                       mLeftVolume;
    float                       mRightVolume;
    int                         mVideoWidth;
    int                         mVideoHeight;
    float                       mSendLevel;
};

typedef sp<MagPlayerClient> MagPlayerClient_t;

enum mag_player_invoke_ids {
    MAG_INVOKE_ID_SET_WINDOW_SIZE = 1,
};

#endif
