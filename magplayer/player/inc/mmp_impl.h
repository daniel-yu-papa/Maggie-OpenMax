#ifndef __MMP_IMPL_H__
#define __MMP_IMPL_H__

#include "framework/MagFramework.h"
#include "MagSingleton.h"
#include "MagPlayerDriver.h"
#include "mmp.h"

class MagMediaPlayerImpl : public MagMediaPlayer, public MagSingleton<MagMediaPlayerImpl>{
    friend class MagSingleton<MagMediaPlayerImpl>;
	MagMediaPlayerImpl();
	virtual ~MagMediaPlayerImpl();
public:
	virtual int        setDataSource(const char *url);
    virtual int        setDataSource(unsigned int fd, signed long long offset, signed long long length);
    virtual int        prepare();
    virtual int        prepareAsync();
    virtual int        start();
    virtual int        stop();
    virtual int        pause();
    virtual bool       isPlaying();
    virtual int        seekTo(int msec);
    virtual int        flush();
    virtual int        fast(int speed);
    virtual int        getCurrentPosition(int* msec);
    virtual int        getDuration(int* msec);
    virtual int        reset();
    virtual int        setVolume(float leftVolume, float rightVolume);
    virtual int        setParameter(int key, void *request);
    virtual int        getParameter(int key, void **reply);
    virtual int        invoke(const unsigned int methodID, void *request, void *reply);
    virtual int        registerEventCallback(mmp_event_callback_t cb, void *handler);

    mmp_event_callback_t mAppEventCallback;
    void                 *mAppHandler;

private:
    bool mbInitialized;
    bool mbError;

    MagPlayerDriver *mpMediaPlayer;

    void initialize();
    void destroy();

    static void eventNotify(void* cookie, int msg, int ext1, int ext2);
};

#endif
