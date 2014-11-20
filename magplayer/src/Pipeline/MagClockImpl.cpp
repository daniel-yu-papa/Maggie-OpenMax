#include "MagClockImpl.h"


_status_t MagClockImpl::setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::start(){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::stop(){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::pause(){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::resume(){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::reset(){
	return MAG_NO_ERROR;
}

i64       MagClockImpl::getPlayingTime(){
	return 0;
}