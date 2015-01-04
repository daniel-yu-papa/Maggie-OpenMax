#include "MagClock.h"
#include "MagPipelineFactory.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

MagClock::MagClock(Clock_Type_t type):
		                mType(type),
						mClock(NULL){

}

MagClock::~MagClock(){
	MagClockImplBase *clk = getClockImpl();
	delete clk;
}

_status_t MagClock::connectVideoPipeline(void *pVpl){
	MagClockImplBase *clk = getClockImpl();
	return clk->connectVideoPipeline(pVpl);
}

_status_t MagClock::connectAudioPipeline(void *pApl){
	MagClockImplBase *clk = getClockImpl();
	return clk->connectAudioPipeline(pApl);
}

_status_t MagClock::disconnectVideoPipeline(void *pVpl){
	MagClockImplBase *clk = getClockImpl();
	return clk->disconnectVideoPipeline(pVpl);
}

_status_t MagClock::disconnectAudioPipeline(void *pApl){
	MagClockImplBase *clk = getClockImpl();
	return clk->disconnectAudioPipeline(pApl);
}

_status_t MagClock::init(){
	MagClockImplBase *clk = getClockImpl();
	return clk->init();
}

_status_t MagClock::setup(){
	MagClockImplBase *clk = getClockImpl();
	return clk->setup();
}

_status_t MagClock::start(){
	MagClockImplBase *clk = getClockImpl();
	return clk->start();
}

_status_t MagClock::stop(){
	MagClockImplBase *clk = getClockImpl();
	return clk->stop();
}

_status_t MagClock::pause(){
	MagClockImplBase *clk = getClockImpl();
	return clk->pause();
}

_status_t MagClock::resume(){
	MagClockImplBase *clk = getClockImpl();
	return clk->resume();
}

_status_t MagClock::flush(){
	MagClockImplBase *clk = getClockImpl();
	return clk->flush();
}

_status_t MagClock::reset(){
	MagClockImplBase *clk = getClockImpl();
	return clk->reset();
}

i64       MagClock::getPlayingTime(){
	MagClockImplBase *clk = getClockImpl();
	return clk->getPlayingTime();
}

MagClockImplBase *MagClock::getClockImpl(){
	if (mClock)
		return mClock;
	else{
		MagPipelineFactory &factory = MagPipelineFactory::getInstance();
		mClock = factory.createClock(mType);
		return mClock;
	}
}