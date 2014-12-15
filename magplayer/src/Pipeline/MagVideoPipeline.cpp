#include "MagVideoPipeline.h"
#include "MagPipelineFactory.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

MagVideoPipeline::MagVideoPipeline(Pipeline_Type_t type):
										mPipeline(NULL),
									    mType(type){

}

MagVideoPipeline::~MagVideoPipeline(){
	MagVideoPipelineImplBase *vpl;
	vpl = getPipelineImpl();
	delete vpl;
}

void MagVideoPipeline::setMagPlayerNotifier(MagMessageHandle notifyMsg){
	getPipelineImpl()->setMagPlayerNotifier(notifyMsg);
}

MagMessageHandle MagVideoPipeline::getMagPlayerNotifier(){
	return getPipelineImpl()->getMagPlayerNotifier();
}

_status_t MagVideoPipeline::init(i32 trackID, TrackInfo_t *sInfo){
	return getPipelineImpl()->init(trackID, sInfo);
}

_status_t MagVideoPipeline::setup(){
	return getPipelineImpl()->setup();
}

_status_t MagVideoPipeline::start(){
	return getPipelineImpl()->start();
}

_status_t MagVideoPipeline::stop(){
	return getPipelineImpl()->stop();
}

_status_t MagVideoPipeline::pause(){
	return getPipelineImpl()->pause();
}

_status_t MagVideoPipeline::resume(){
    return getPipelineImpl()->resume();
}

_status_t MagVideoPipeline::flush(){
	return getPipelineImpl()->flush();
}

_status_t MagVideoPipeline::reset(){
	return getPipelineImpl()->reset();
}

_status_t MagVideoPipeline::getClkConnectedComp(i32 *port, void **ppComp){
	return getPipelineImpl()->getClkConnectedComp(port, ppComp);
}

MagVideoPipelineImplBase *MagVideoPipeline::getPipelineImpl(){
	if (mPipeline)
		return mPipeline;
	else{
		MagPipelineFactory &factory = MagPipelineFactory::getInstance();
		mPipeline = factory.createVideoPipeline(mType);
		return mPipeline;
	}
}