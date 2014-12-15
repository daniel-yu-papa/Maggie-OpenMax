#ifndef __MAGPLAYER_VIDEO_PIPELINE_H__
#define __MAGPLAYER_VIDEO_PIPELINE_H__

#include "framework/MagFramework.h"
#include "MagPipelineFactory.h"
#include "MagVideoPipelineImplBase.h"

class MagVideoPipeline{
public:
    MagVideoPipeline(Pipeline_Type_t type);
    virtual ~MagVideoPipeline();

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    virtual MagMessageHandle getMagPlayerNotifier();
    virtual _status_t init(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush();
    virtual _status_t reset();
    virtual _status_t getClkConnectedComp(i32 *port, void **ppComp);
    
protected:
    MagVideoPipelineImplBase *getPipelineImpl();

private:
    MagVideoPipelineImplBase *mPipeline;
    Pipeline_Type_t mType;
};

#endif