#ifndef __MAG_PLAYER_DRIVER_H__
#define __MAG_PLAYER_DRIVER_H__

#include <binder/Parcel.h>
#include <gui/ISurfaceTexture.h>
#include "MagPlayer.h"

using namespace android;

typedef void (*notify_client_callback_f)(void* cookie,
                                               int msg, int ext1, int ext2);

class MagPlayerDriver{
public:
    MagPlayerDriver(void *client, notify_client_callback_f cb);
    ~MagPlayerDriver();
    
    _status_t        setDataSource(const char *url);
    _status_t        setDataSource(i32 fd, i64 offset, i64 length);
    _status_t        setDataSource(const sp<IStreamBuffer>& source);
    _status_t        setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture);
    _status_t        prepare();
    _status_t        prepareAsync();
    _status_t        start();
    _status_t        stop();
    _status_t        pause();
    bool             isPlaying();
    _status_t        seekTo(int msec);
    _status_t        flush();
    _status_t        fast(int speed);
    _status_t        getCurrentPosition(int* msec);
    _status_t        getDuration(int* msec);
    _status_t        reset();
    _status_t        setVolume(float leftVolume, float rightVolume);
    _status_t        setParameter(int key, const Parcel& request);
    _status_t        getParameter(int key, Parcel* reply);
    _status_t        invoke(const Parcel& request, Parcel *reply);

    static void PrepareCompleteEvtListener(void *priv);
    static void SeekCompleteEvtListener(void *priv);
    static void FlushCompleteEvtListener(void *priv);
    static void ErrorEvtListener(void *priv, i32 what, i32 extra);
    static void InfoEvtListener(void *priv, i32 what, i32 extra);
    
    enum State_t{
        MPD_ST_ERROR              = 0,
        MPD_ST_IDLE               = 1 << 0,
        MPD_ST_INITIALIZED        = 1 << 1,
    };

    State_t mState;
    
private:
    void *mpClient;
    notify_client_callback_f mClientNotifyFn;

    MagPlayer *mpPlayer;
    StreamBufferUser *mpStreamBufUser;
    
    void sendEvent(i32 msg, i32 ext1=0, i32 ext2=0);

    i32 mSetTrackIndex;
};


#endif