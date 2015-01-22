#ifndef __MAGPLAYER_PIPELINE_MANAGER_H__
#define __MAGPLAYER_PIPELINE_MANAGER_H__

#include "MagClock.h"
#include "MagAudioPipeline.h"
#include "MagVideoPipeline.h"

#define PAUSE_CLOCK_FLAG 0x01
#define PAUSE_AV_FLAG    0x02

typedef enum{
    MAG_PIPELINE_VIDEO_TYPE,
    MAG_PIPELINE_AUDIO_TYPE,
    MAG_PIPELINE_SUBTITLE_TYPE
}MAG_PIPELINE_TYPE_t;

typedef struct{
	List_t node;

	void *pPipeline;
	boolean connected;
}MAG_PIPELINE_ENTRY_t;

class MagPipelineManager{
public:
    MagPipelineManager(MagClock *pClockComp);
    virtual ~MagPipelineManager();

    _status_t addVideoPipeline(MagVideoPipeline *pVideoPipeline, boolean connectToClk = MAG_TRUE);
    _status_t addAudioPipeline(MagAudioPipeline *pAudioPipeline, boolean connectToClk = MAG_TRUE); 
	
	_status_t removeVideoPipeline(MagVideoPipeline *pVideoPipeline);
    _status_t removeAudioPipeline(MagAudioPipeline *pAudioPipeline);

    _status_t getDecodedFrame(MAG_PIPELINE_TYPE_t type, void *pPipeline, void **ppFrame);
    _status_t putUsedFrame(MAG_PIPELINE_TYPE_t type, void *pPipeline, void *pFrame);

    _status_t setup();
	_status_t start();
    _status_t stop();
    _status_t pause(ui8 flag);
    _status_t resume();
    _status_t flush();
    _status_t reset();

    _status_t setVolume(fp32 leftVolume, fp32 rightVolume);
    i64       getMediaTime();
    i64       getPlayingPosition();

private:
	List_t mVideoPipelineHead;
	List_t mAudioPipelineHead;
	MagClock *mpClockComp;
};

#endif