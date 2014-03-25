#ifndef __MAGPLAYER_SERVICE_H__
#define __MAGPLAYER_SERVICE_H__

#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include "MagFramework.h"
#include "IMagPlayerService.h"
#include "MagPlayerDriver.h"
#include "IMagPlayerNotifier.h"

using namespace android;

class MagPlayerService : public BnMagPlayerService{
    class Client;

public:
    static  void                instantiate();

    virtual sp<IMagPlayerClient>    create(pid_t pid, const sp<IMagPlayerNotifier>& client);
    void                            removeClient(wp<Client> client);
    
private:
    class Client : public BnMagPlayerClient {
        virtual void             disconnect();
        virtual _status_t        setDataSource(const char *url);
        virtual _status_t        setDataSource(i32 fd, i64 offset, i64 length);
        virtual _status_t        setDataSource(const sp<IStreamBuffer>& source);
        virtual _status_t        setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture);
        virtual _status_t        prepare();
        virtual _status_t        prepareAsync();
        virtual _status_t        start();
        virtual _status_t        stop();
        virtual _status_t        pause();
        virtual _status_t        isPlaying(bool* state);
        virtual _status_t        seekTo(int msec);
        virtual _status_t        flush();
        virtual _status_t        fast(int speed);
        virtual _status_t        getCurrentPosition(int* msec);
        virtual _status_t        getDuration(int* msec);
        virtual _status_t        reset();
        virtual _status_t        setVolume(float leftVolume, float rightVolume);
        virtual _status_t        setParameter(int key, const Parcel& request);
        virtual _status_t        getParameter(int key, Parcel* reply);
        virtual _status_t        invoke(const Parcel& request, Parcel *reply);

        MagPlayerDriver*        createPlayer();


        static  void            notify(void* cookie, int msg,
                                       int ext1, int ext2);

                pid_t           pid() const { return mPid; }


    private:
        friend class MagPlayerService;
                                Client( const sp<MagPlayerService>& service,
                                        pid_t pid,
                                        int32_t connId,
                                        const sp<IMagPlayerNotifier>& client,
                                        uid_t uid);
        virtual                 ~Client();

        MagPlayerDriver*        getPlayer() const { Mutex::Autolock lock(mLock); return mPlayer; }


        // Disconnect from the currently connected ANativeWindow.
        //void disconnectNativeWindow();

        mutable     Mutex                       mLock;
                    MagPlayerDriver             *mPlayer; /*initialized as NULL*/
                    sp<MagPlayerService>        mService;
                    sp<IMagPlayerNotifier>      mClient;
                    pid_t                       mPid;
                    bool                        mLoop;
                    i32                         mConnId;
                    uid_t                       mUID;
                    sp<ANativeWindow>           mConnectedWindow;
                    sp<IBinder>                 mConnectedWindowBinder;
                    sp<Client>                  mNextClient;

    }; // Client

    // ----------------------------------------------------------------------------

                            MagPlayerService();
    virtual                 ~MagPlayerService();

    mutable     Mutex                       mLock;
                SortedVector< wp<Client> >  mClients;
                int32_t                     mNextConnId;
};

#endif