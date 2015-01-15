#ifndef __MAG_PLAYER_DRIVER_H__
#define __MAG_PLAYER_DRIVER_H__

#include "MagPlayerDriverImplBase.h"

class MagPlayerDriver{
public:
    MagPlayerDriver(void *client, notify_client_callback_f cb);
    ~MagPlayerDriver();
    
    _status_t        setDataSource(const char *url);
    _status_t        setDataSource(unsigned int fd, long long offset, long long length);
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
    _status_t        setParameter(int key, void *request);
    _status_t        getParameter(int key, void **reply);
    _status_t        invoke(const unsigned int methodID, void *request, void **reply);
    ui32             getVersion();

protected:
    MagPlayerDriverImplBase *getPlayerDriver();

private:
    void *mPrivData;
    notify_client_callback_f mEventCB;
    MagPlayerDriverImplBase *mPlayerDriver;

};


#endif