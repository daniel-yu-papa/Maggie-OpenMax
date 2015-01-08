#ifndef __MAGPLAYER_VIDEO_PIPELINE_IMPLBASE_H__
#define __MAGPLAYER_VIDEO_PIPELINE_IMPLBASE_H__

#include "framework/MagFramework.h"
#include "MagStreamTrackManager.h"

class MagVideoPipelineImplBase{
public:
    enum{
        MagVideoPipeline_EmptyThisBuffer,
        MagVideoPipeline_Destroy,
    };

    enum{
        ST_INIT,
        ST_PLAY,
        ST_PAUSE,
        ST_STOP
    };

    MagVideoPipelineImplBase(){};
    virtual ~MagVideoPipelineImplBase(){};

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
    virtual _status_t getDecodedFrame(void **ppVideoFrame) = 0;
    virtual _status_t putUsedFrame(void *pVideoFrame) = 0;
};

#endif