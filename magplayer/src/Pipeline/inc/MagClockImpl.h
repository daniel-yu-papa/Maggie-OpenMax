#ifndef __MAGPLAYER_CLOCK_IMPL_H__
#define __MAGPLAYER_CLOCK_IMPL_H__

#include "framework/MagFramework.h"
#include "MagClockImplBase.h"

class MagClockImpl : public MagClockImplBase{
public:
    MagClockImpl(){};
    virtual ~MagClockImpl(){};

    virtual _status_t setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort);
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t reset();
    virtual i64       getPlayingTime();
};

#endif