#ifndef __MAGPLAYER_AUDIO_PIPELINE_H__
#define __MAGPLAYER_AUDIO_PIPELINE_H__

#include "framework/MagFramework.h"
#include "MagPipelineFactory.h"
#include "MagAudioPipelineImplBase.h"
 
class MagAudioPipeline{
public:
    MagAudioPipeline(Pipeline_Type_t type);
    virtual ~MagAudioPipeline();

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    virtual MagMessageHandle getMagPlayerNotifier();
    virtual _status_t setup(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush();
    virtual _status_t reset();
    virtual void *getClkConnectedComp(i32 *port);
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume);
    
protected:
    MagAudioPipelineImplBase *getPipelineImpl();

private:
    MagAudioPipelineImplBase *mPipeline;
    Pipeline_Type_t mType;
};

#endif