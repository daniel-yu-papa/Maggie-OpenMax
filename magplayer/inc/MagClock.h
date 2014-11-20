#ifndef __MAGPLAYER_CLOCK_H__
#define __MAGPLAYER_CLOCK_H__

#include "framework/MagFramework.h"
#include "MagPipelineFactory.h"
#include "MagClockImplBase.h"
 
class MagClock{
public:
    MagClock(Clock_Type_t type);
    virtual ~MagClock();

    virtual _status_t setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort);
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t reset();
    virtual i64       getPlayingTime();
    
protected:
    MagClockImplBase *getClockImpl();

private:
    Clock_Type_t      mType;
    MagClockImplBase *mClock;
};

#endif