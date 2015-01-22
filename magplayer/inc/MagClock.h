#ifndef __MAGPLAYER_CLOCK_H__
#define __MAGPLAYER_CLOCK_H__

#include "framework/MagFramework.h"
#include "MagPipelineFactory.h"
#include "MagClockImplBase.h"
 
class MagClock{
public:
    MagClock(Clock_Type_t type);
    virtual ~MagClock();

    virtual _status_t connectVideoPipeline(void *pVideoPipeline);
    virtual _status_t connectAudioPipeline(void *pAudioPipeline);
    virtual _status_t disconnectVideoPipeline(void *pVideoPipeline);
    virtual _status_t disconnectAudioPipeline(void *pAudioPipeline);
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
    
protected:
    MagClockImplBase *getClockImpl();

private:
    Clock_Type_t      mType;
    MagClockImplBase *mClock;
};

#endif