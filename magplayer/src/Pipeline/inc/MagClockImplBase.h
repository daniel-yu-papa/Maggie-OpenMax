#ifndef __MAGPLAYER_CLOCK_IMPLBASE_H__
#define __MAGPLAYER_CLOCK_IMPLBASE_H__

#include "framework/MagFramework.h"


class MagClockImplBase{
public:
    MagClockImplBase(){};
    virtual ~MagClockImplBase(){};

    virtual _status_t connectVideoPipeline(void *pVpl) = 0;
    virtual _status_t connectAudioPipeline(void *pApl) = 0;
    virtual _status_t disconnectVideoPipeline(void *pVpl) = 0;
    virtual _status_t disconnectAudioPipeline(void *pApl) = 0;
    virtual _status_t init() = 0;
    virtual _status_t setup() = 0;
    virtual _status_t start() = 0;
    virtual _status_t stop() = 0;
    virtual _status_t pause() = 0;
    virtual _status_t resume() = 0;
    virtual _status_t reset() = 0;
    virtual i64       getPlayingTime() = 0;
};

#endif