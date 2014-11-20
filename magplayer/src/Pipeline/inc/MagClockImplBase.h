#ifndef __MAGPLAYER_CLOCK_IMPLBASE_H__
#define __MAGPLAYER_CLOCK_IMPLBASE_H__

#include "framework/MagFramework.h"

class MagClockImplBase{
public:
    MagClockImplBase(){};
    virtual ~MagClockImplBase(){};

    virtual _status_t setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort) = 0;
    virtual _status_t start() = 0;
    virtual _status_t stop() = 0;
    virtual _status_t pause() = 0;
    virtual _status_t resume() = 0;
    virtual _status_t reset() = 0;
    virtual i64       getPlayingTime() = 0;
};

#endif