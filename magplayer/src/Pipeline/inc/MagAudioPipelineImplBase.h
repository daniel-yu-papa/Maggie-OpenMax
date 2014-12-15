#ifndef __MAGPLAYER_AUDIO_PIPELINE_IMPLBASE_H__
#define __MAGPLAYER_AUDIO_PIPELINE_IMPLBASE_H__

#include "framework/MagFramework.h"
#include "MagStreamTrackManager.h"

class MagAudioPipelineImplBase{
public:
    enum{
        MagAudioPipeline_EmptyThisBuffer,
        MagAudioPipeline_Destroy,
    };

    enum{
        ST_INIT,
        ST_PLAY,
        ST_PAUSE,
        ST_STOP
    };

    MagAudioPipelineImplBase(){};
    virtual ~MagAudioPipelineImplBase(){};

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg) = 0;
    virtual MagMessageHandle getMagPlayerNotifier() = 0;
    virtual _status_t init(i32 trackID, TrackInfo_t *sInfo) = 0;
    virtual _status_t setup()  = 0;
    virtual _status_t start()  = 0;
    virtual _status_t stop()   = 0;
    virtual _status_t pause()  = 0;
    virtual _status_t resume() = 0;
    virtual _status_t flush()  = 0;
    virtual _status_t reset()  = 0;
    virtual _status_t getClkConnectedComp(i32 *port, void **ppComp) = 0;
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume) = 0;
};

#endif