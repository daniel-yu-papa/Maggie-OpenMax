#include "MagAudioPipeline.h"
#include "MagPipelineFactory.h"

MagAudioPipeline::MagAudioPipeline(Pipeline_Type_t type):
											mPipeline(NULL),
											mType(type){

}

MagAudioPipeline::~MagAudioPipeline(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	delete apl;
}

void MagAudioPipeline::setMagPlayerNotifier(MagMessageHandle notifyMsg){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	apl->setMagPlayerNotifier(notifyMsg);
}

MagMessageHandle MagAudioPipeline::getMagPlayerNotifier(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->getMagPlayerNotifier();
}

_status_t MagAudioPipeline::setup(i32 trackID, TrackInfo_t *sInfo){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->setup(trackID, sInfo);
}

_status_t MagAudioPipeline::start(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->start();
}

_status_t MagAudioPipeline::stop(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->stop();
}

_status_t MagAudioPipeline::pause(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->pause();
}

_status_t MagAudioPipeline::resume(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
    return apl->resume();
}

_status_t MagAudioPipeline::flush(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->flush();
}

_status_t MagAudioPipeline::reset(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->reset();
}

void *MagAudioPipeline::getClkConnectedComp(i32 *port){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->getClkConnectedComp(port);
}

_status_t MagAudioPipeline::setVolume(fp32 leftVolume, fp32 rightVolume){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->setVolume(leftVolume, rightVolume);
}

MagAudioPipelineImplBase *MagAudioPipeline::getPipelineImpl(){
	if (mPipeline)
		return mPipeline;
	else{
		MagPipelineFactory &factory = MagPipelineFactory::getInstance();
		mPipeline = factory.createAudioPipeline(mType);
		return mPipeline;
	}
}