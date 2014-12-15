#ifndef __OMXIL_CLOCK_H__
#define __OMXIL_CLOCK_H__

#include "MagClockImpl.h"
#include "framework/MagFramework.h"

class OmxilClock : public MagClockImpl{
public:
	OmxilClock();
    virtual ~OmxilClock();

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
    virtual _status_t reset();
    virtual i64       getPlayingTime();

private:
	OMX_HANDLETYPE   mhClock;

    OMX_CALLBACKTYPE mClockCallbacks;

    OMX_U32          mPortIdxToARen;
    OMX_U32          mPortIdxToVSch;

    MagEventHandle         mClkStIdleEvent;
    MagEventGroupHandle    mStIdleEventGroup;

    MagEventHandle         mClkStLoadedEvent;
    MagEventGroupHandle    mStLoadedEventGroup;

    static OMX_ERRORTYPE ClockEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);
};

#endif