#ifndef __MAGPLAYER_PIPELINE_FACTORY_H__
#define __MAGPLAYER_PIPELINE_FACTORY_H__

#include "MagSingleton.h"
#include "MagAudioPipelineImplBase.h"
#include "MagVideoPipelineImplBase.h"
// #include "MagAudioPipelineImpl.h"
// #include "MagVideoPipelineImpl.h"
#include "MagClockImplBase.h"
// #include "MagClockImpl.h"

typedef enum{
	MARVELL_AMP_PIPELINE,
}Pipeline_Type_t;

typedef enum{
	MARVELL_AMP_CLOCK,
}Clock_Type_t;

class MagPipelineFactory : public MagSingleton<MagPipelineFactory>{
    friend class MagSingleton<MagPipelineFactory>;
public:
	MagPipelineFactory();
	virtual ~MagPipelineFactory();

	MagVideoPipelineImplBase *createVideoPipeline(Pipeline_Type_t type);
	MagAudioPipelineImplBase *createAudioPipeline(Pipeline_Type_t type);
	MagClockImplBase         *createClock(Clock_Type_t type);
};
#endif