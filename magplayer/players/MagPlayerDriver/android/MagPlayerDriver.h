#ifndef __MAG_PLAYER_DRIVER_H__
#define __MAG_PLAYER_DRIVER_H__

typedef void (*notify_client_callback_f)(void* cookie,
                                               int msg, int ext1, int ext2);

class MagPlayerDriver{
public:
    MagPlayerDriver(void *client, notify_client_callback_f cb);

    virtual _status_t        setDataSource(const char *url);
    virtual _status_t        setDataSource(i32 fd, i64 offset, i64 length);
    virtual _status_t        setDataSource(const sp<IStreamBuffer>& source);
    virtual _status_t        setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture);
    virtual _status_t        prepare();
    virtual _status_t        prepareAsync();
    virtual _status_t        start();
    virtual _status_t        stop();
    virtual _status_t        pause();
    virtual bool             isPlaying();
    virtual _status_t        seekTo(int msec);
    virtual _status_t        flush();
    virtual _status_t        fast(int multiple);
    virtual _status_t        getCurrentPosition(int* msec);
    virtual _status_t        getDuration(int* msec);
    virtual _status_t        reset();
    virtual _status_t        setVolume(float leftVolume, float rightVolume);
    virtual _status_t        setParameter(int key, const Parcel& request);
    virtual _status_t        getParameter(int key, Parcel* reply);
    virtual _status_t        invoke(const Parcel& request, Parcel *reply);

    static void PrepareCompleteEvtListener(void *priv);
    static void SeekCompleteEvtListener(void *priv);
    static void FlushCompleteEvtListener(void *priv);
    static void ErrorEvtListener(void *priv, i32 what, i32 extra);
    static void InfoEvtListener(void *priv, i32 what, i32 extra);
    
private:
    void *mpClient;
    notify_client_callback_f mClientNotifyFn;
    
    enum State_t{
        MPD_ST_ERROR              = 0,
        MPD_ST_IDLE               = 1 << 0,
        MPD_ST_INITIALIZED        = 1 << 1,
    };

    State_t mState;
    
    MagPlayer *mpPlayer;
    StreamBufferUser *mpStreamBufUser;
    
    void sendEvent(i32 msg, i32 ext1=0, i32 ext2=0);
};


#endif