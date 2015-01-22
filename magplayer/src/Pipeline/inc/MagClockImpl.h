#ifndef __MAGPLAYER_CLOCK_IMPL_H__
#define __MAGPLAYER_CLOCK_IMPL_H__

#include "framework/MagFramework.h"
#include "MagClockImplBase.h"

class MagClockImpl : public MagClockImplBase{
public:
    MagClockImpl(){};
    virtual ~MagClockImpl(){};

    virtual _status_t connectVideoPipeline(void *pVpl);
    virtual _status_t connectAudioPipeline(void *pApl);
    virtual _status_t disconnectVideoPipeline(void *pVpl);
    virtual _status_t disconnectAudioPipeline(void *pApl);
    virtual _status_t init();
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush();
    virtual _status_t reset();
    virtual i64       getPlayingTime();
    virtual i64       getMediaTime();
};

#endif