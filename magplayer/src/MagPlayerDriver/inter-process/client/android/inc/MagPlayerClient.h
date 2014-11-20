#ifndef __MAGPLAYER_CLIENT_H__
#define __MAGPLAYER_CLIENT_H__

#include "IMagPlayerClient.h"
#include "MagFramework.h"
#include "IMagPlayerDeathNotifier.h"
#include "IMagPlayerNotifier.h"

using namespace android;

enum mag_player_states {
    MAG_PLAYER_STATE_ERROR        = 0,
    MAG_PLAYER_IDLE               = 1 << 0,
    MAG_PLAYER_INITIALIZED        = 1 << 1,
    MAG_PLAYER_RUNNING            = 1 << 2,
};


// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class MagPlayerListener
{
public:
    virtual ~MagPlayerListener(){};
    virtual void notify(int msg, int ext1, int ext2) = 0;
};

class MagPlayerClient : public BnMagPlayerNotifier,
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
        _status_t        setListener(MagPlayerListener* listener);
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
        void             notify(int msg, int ext1, int ext2);
        _status_t        setVolume(float leftVolume, float rightVolume);
        bool             isPlaying();
        _status_t        invoke(const Parcel& request, Parcel *reply);
        _status_t        flush();
        
private:
        _status_t        attachNewPlayer(const sp<IMagPlayerClient>& player);


    sp<IMagPlayerClient>        mPlayer;
    Mutex                       mLock;
    Mutex                       mNotifyLock;
    enum mag_player_states      mCurrentState;
    MagPlayerListener           *mpListener;   
};

typedef sp<MagPlayerClient> MagPlayerClient_t;

#endif
