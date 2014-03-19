#ifndef __MAGPLAYER_CLIENT_H__
#define __MAGPLAYER_CLIENT_H__


#include "IMagPlayerClient.h"

using namespace android;

class Surface;
class ISurfaceTexture;

enum media_event_type {
    MEDIA_NOP               = 0, // interface test message
    MEDIA_PREPARED          = 1,
    MEDIA_PLAYBACK_COMPLETE = 2,
    MEDIA_BUFFERING_UPDATE  = 3,
    MEDIA_SEEK_COMPLETE     = 4,
    MEDIA_FLUSH_COMPLETE    = 5,
    MEDIA_ERROR             = 100,
    MEDIA_INFO              = 200,
};

enum mag_player_invoke_ids {
    MAG_INVOKE_ID_SET_WINDOW_SIZE = 1,
};

enum mag_player_states {
    MAG_PLAYER_STATE_ERROR        = 0,
    MAG_PLAYER_IDLE               = 1 << 0,
    MAG_PLAYER_INITIALIZED        = 1 << 1,
    MAG_PLAYER_RUNNING            = 1 << 2,
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
class MagPlayerListener
{
public:
    virtual void notify(int msg, int ext1, int ext2) = 0;
};

class MagPlayerClient : public BnMagPlayerClient,
                             public virtual IMagPlayerDeathNotifier
{
public:
    MagPlayerClient();
    ~MagPlayerClient();

        void            died();
        void            disconnect();

        _status_t        setDataSource(const char *url);
        _status_t        setDataSource(i32 fd, i64 offset, i64 length);
        _status_t        setDataSource(const sp<IStreamBuffer> &source);
        _status_t        setVideoSurfaceTexture(
                                        const sp<ISurfaceTexture>& surfaceTexture);
        _status_t        setListener(const sp<MagPlayerListener>& listener);
        _status_t        prepare();
        _status_t        prepareAsync();
        _status_t        start();
        _status_t        stop();
        _status_t        pause();
        _status_t        isPlaying(bool* state);
        _status_t        seekTo(int msec);
        _status_t        fast(int multiple);
        _status_t        getCurrentPosition(int *msec);
        _status_t        getDuration(int *msec);
        _status_t        reset();
        _status_t        setParameter(int key, const Parcel& request);
        _status_t        getParameter(int key, Parcel *reply);
        void             notify(int msg, int ext1, int ext2, const Parcel *obj);
        _status_t        setVolume(float leftVolume, float rightVolume);
        bool             isPlaying();
        _status_t        invoke(const Parcel& request, Parcel *reply);
        _status_t        flush();
        
private:
        _status_t        attachNewPlayer(const sp<IMagPlayerClient>& player);


    sp<IMagPlayerClient>        mPlayer;
    Mutex                       mLock;
    Mutex                       mNotifyLock;
    MagPlayerListener           *mpListener;
    mag_player_states           mCurrentState;
};

typedef sp<MagPlayerClient> MagPlayerClient_t;

#endif
