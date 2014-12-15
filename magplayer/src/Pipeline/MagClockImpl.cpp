#include "MagClockImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

_status_t MagClockImpl::connectVideoPipeline(void *pVpl){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::connectAudioPipeline(void *pApl){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::disconnectVideoPipeline(void *pVpl){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::disconnectAudioPipeline(void *pApl){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::init(){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::setup(){
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