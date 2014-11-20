#include "MagClock.h"
#include "MagPipelineFactory.h"

MagClock::MagClock(Clock_Type_t type):
		                mType(type),
						mClock(NULL){

}

MagClock::~MagClock(){
	MagClockImplBase *clk = getClockImpl();
	delete clk;
}

_status_t MagClock::setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort){
	MagClockImplBase *clk = getClockImpl();
	return clk->setup(AudioComp, AudioPort, VideoComp, VideoPort);
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