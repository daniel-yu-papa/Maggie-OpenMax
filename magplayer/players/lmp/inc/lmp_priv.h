#ifndef __LMP_PRIV_H__
#define __LMP_PRIV_H__

#include "framework/MagFramework.h"
#include "MagSingleton.h"
#include "MagPlayerDriver.h"
#include "lmp.h"

class DonglePlayer : public LinuxMediaPlayer, public MagSingleton<DonglePlayer>{
    friend class MagSingleton<DonglePlayer>;
	DonglePlayer();
	virtual ~DonglePlayer();
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
    virtual int        invoke(const unsigned int methodID, const void *request, void *reply);
    virtual int        registerEventCallback(lmp_event_callback_t cb, void *handler);

    lmp_event_callback_t mAppEventCallback;
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
