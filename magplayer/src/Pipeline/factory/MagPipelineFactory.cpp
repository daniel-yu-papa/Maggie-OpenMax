#include "MagPipelineFactory.h"
#include "Omxil_VideoPipeline.h"
#include "Omxil_AudioPipeline.h"
#include "Omxil_Clock.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

MAG_SINGLETON_STATIC_INSTANCE(MagPipelineFactory)

MagPipelineFactory::MagPipelineFactory(){

}

MagPipelineFactory::~MagPipelineFactory(){

}

MagVideoPipelineImplBase *MagPipelineFactory::createVideoPipeline(Pipeline_Type_t type){
	MagVideoPipelineImplBase *obj = NULL;

	switch (type){
		case MAG_OMX_PIPELINE:
			obj = new OmxilVideoPipeline();
			break;

		default:
			break;
	}
	return obj;
}

MagAudioPipelineImplBase *MagPipelineFactory::createAudioPipeline(Pipeline_Type_t type){
	MagAudioPipelineImplBase *obj = NULL;

	switch (type){
		case MAG_OMX_PIPELINE:
			obj = new OmxilAudioPipeline();
			break;
			
		default:
			break;
	}
	return obj;
}

MagClockImplBase *MagPipelineFactory::createClock(Clock_Type_t type){
	MagClockImplBase *obj = NULL;

	switch (type){
		case MAG_OMX_CLOCK:
			obj = new OmxilClock();
			break;
			
		default:
			break;
	}
	return obj;
}