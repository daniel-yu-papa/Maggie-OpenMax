#ifndef __MAGPLAYER_SERVICE_H__
#define __MAGPLAYER_SERVICE_H__

#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>

namespace android {

class MagPlayerService : public BnMagPlayerService{
    class Client;

public:
    static  void                instantiate();

    virtual sp<IMagPlayerClient>    create(pid_t pid, const sp<IMagPlayerClient>& client);
    void                removeClient(wp<Client> client);
    
private:
    class Client : public BnMagPlayerClient {
        // IMediaPlayer interface
        virtual void            disconnect();
        virtual status_t        setVideoSurfaceTexture(
                                        const sp<ISurfaceTexture>& surfaceTexture);
        virtual status_t        prepareAsync();
        virtual status_t        start();
        virtual status_t        stop();
        virtual status_t        pause();
        virtual status_t        isPlaying(bool* state);
        virtual status_t        seekTo(int msec);
        virtual status_t        getCurrentPosition(int* msec);
        virtual status_t        getDuration(int* msec);
        virtual status_t        reset();
        virtual status_t        setLooping(int loop);
        virtual status_t        setVolume(float leftVolume, float rightVolume);
        virtual status_t        invoke(const Parcel& request, Parcel *reply);
        virtual status_t        setParameter(int key, const Parcel &request);
        virtual status_t        getParameter(int key, Parcel *reply);

        MagPlayerDriver*        createPlayer();

        virtual status_t        setDataSource(
                                const char *url,
                                const MagMessageHandle hHeaders);

        virtual status_t        setDataSource(i32 fd, i64 offset, i64 length);

        virtual status_t        setDataSource(const sp<IStreamSource> &source);


        static  void            notify(void* cookie, int msg,
                                       int ext1, int ext2, const Parcel *obj);

                pid_t           pid() const { return mPid; }
        virtual status_t        dump(int fd, const Vector<String16>& args) const;


    private:
        friend class MagPlayerService;
                                Client( const sp<MagPlayerService>& service,
                                        pid_t pid,
                                        int32_t connId,
                                        const sp<IMagPlayerClient>& client,
                                        uid_t uid);
                                Client();
        virtual                 ~Client();

        void                    deletePlayer();

        sp<MagPlayerDriver>     getPlayer() const { Mutex::Autolock lock(mLock); return mPlayer; }


        // Disconnect from the currently connected ANativeWindow.
        //void disconnectNativeWindow();

        mutable     Mutex                       mLock;
                    MagPlayerDriver             *mPlayer; /*initialized as NULL*/
                    sp<MagPlayerService>        mService;
                    sp<IMagPlayerClient>        mClient;
                    pid_t                       mPid;
                    bool                        mLoop;
                    int32_t                     mConnId;
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

};
#endif