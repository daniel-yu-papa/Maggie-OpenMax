#ifndef __MAG_PLAYER_DRIVER_H__
#define __MAG_PLAYER_DRIVER_H__

typedef void (*notify_client_callback_f)(void* cookie,
               int msg, int ext1, int ext2, const Parcel *obj);

class MagPlayerDriver{
public:
    MagPlayerDriver(void *client, notify_client_callback_f cb);

     virtual _status_t setDataSource(
             const char *url, const MagMiniDBHandle settings);

    virtual _status_t setDataSource(i32 fd, i64 offset, i64 length);
    virtual _status_t setDataSource(const sp<IStreamSource> &source);

    virtual _status_t setVideoSurface(const sp<Surface> &surface);
    virtual _status_t setVideoSurfaceTexture(
            const sp<ISurfaceTexture> &surfaceTexture);

    virtual _status_t prepare();
    virtual _status_t prepareAsync();
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual bool isPlaying();
    virtual _status_t seekTo(int msec);
    virtual _status_t getCurrentPosition(int *msec);
    virtual _status_t getDuration(int *msec);
    virtual _status_t reset();
    virtual _status_t setParameter(int key, const Parcel &request);
    virtual _status_t getParameter(int key, Parcel *reply);
    virtual _status_t invoke(const Parcel &request, Parcel *reply);

    void PrepareCompleteEvtListener();
    void FlushCompleteEvtListener();
    void ErrorEvtListener(i32 what, i32 extra);

    void setNotifyCallback(void* client, notify_client_callback_f notifyFunc);
    
private:
    void *mpClient;
    notify_client_callback_f mClientNotifyFn;
    
    enum State_t{
        MPD_IDLE = 0,
        MPD_INITIALIZED,
        MPD_PREPARING,
        MPD_PREPARED,
        MPD_FLUSHING,
        MPD_SEEKING,
        MPD_RUNNING,
        MPD_FASTING,
        MPD_PAUSED,
        MPD_STOPPED,
        MPD_ERROR,
        MPD_PLAYBACK_COMPLETED,
    };

    State_t mState;
    State_t mSeekBackState;
    State_t mFlushBackState;
    
    MagPlayer *mpPlayer;

    MagEventGroupHandle mPrepareEvtGroup;
    MagEventHandle      mPrepareDoneEvt;
    MagEventHandle      mPrepareErrorEvt;

    MagMutexHandle      mNotifyLock;

    void sendEvent(int msg, int ext1=0, int ext2=0, const Parcel *obj=NULL);
};


#endif