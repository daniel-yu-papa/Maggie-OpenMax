#include "MagPipelineFactory.h"
#include "MrvlAMPVideoPipeline.h"
#include "MrvlAMPAudioPipeline.h"
#include "MrvlAMPClock.h"

MAG_SINGLETON_STATIC_INSTANCE(MagPipelineFactory)

MagPipelineFactory::MagPipelineFactory(){

}

MagPipelineFactory::~MagPipelineFactory(){

}

MagVideoPipelineImplBase *MagPipelineFactory::createVideoPipeline(Pipeline_Type_t type){
	MagVideoPipelineImplBase *obj = NULL;

	switch (type){
		case MARVELL_AMP_PIPELINE:
			obj = new MrvlAMPVideoPipeline();
			break;

		default:
			break;
	}
	return obj;
}

MagAudioPipelineImplBase *MagPipelineFactory::createAudioPipeline(Pipeline_Type_t type){
	MagAudioPipelineImplBase *obj = NULL;

	switch (type){
		case MARVELL_AMP_PIPELINE:
			obj = new MrvlAMPAudioPipeline();
			break;
			
		default:
			break;
	}
	return obj;
}

MagClockImplBase *MagPipelineFactory::createClock(Clock_Type_t type){
	MagClockImplBase *obj = NULL;

	switch (type){
		case MARVELL_AMP_CLOCK:
			obj = new MrvlAMPClock();
			break;
			
		default:
			break;
	}
	return obj;
}