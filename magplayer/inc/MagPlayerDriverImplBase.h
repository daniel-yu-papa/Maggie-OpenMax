#ifndef __MAGPLAYER_DRIVER_IMPL_BASE_H__
#define __MAGPLAYER_DRIVER_IMPL_BASE_H__

#include "framework/MagFramework.h"

typedef void (*notify_client_callback_f)(void* cookie, int msg, int ext1, int ext2);

class MagPlayerDriverImplBase{
public:
    MagPlayerDriverImplBase(){};
    virtual ~MagPlayerDriverImplBase(){};
    
    virtual _status_t        setDataSource(const char *url) = 0;
    virtual _status_t        setDataSource(ui32 fd, i64 offset, i64 length) = 0;
    virtual _status_t        prepare() = 0;
    virtual _status_t        prepareAsync() = 0;
    virtual _status_t        start() = 0;
    virtual _status_t        stop() = 0;
    virtual _status_t        pause() = 0;
    virtual bool             isPlaying() = 0;
    virtual _status_t        seekTo(int msec) = 0;
    virtual _status_t        flush() = 0;
    virtual _status_t        fast(int speed) = 0;
    virtual _status_t        getCurrentPosition(int* msec) = 0;
    virtual _status_t        getDuration(int* msec) = 0;
    virtual _status_t        reset() = 0;
    virtual _status_t        setVolume(float leftVolume, float rightVolume) = 0;
    virtual _status_t        setParameter(int key, void *request) = 0;
    virtual _status_t        getParameter(int key, void **reply) = 0;
    virtual _status_t        invoke(const ui32 methodID, void *request, void **reply) = 0;
    virtual ui32             getVersion() = 0;
};

#endif