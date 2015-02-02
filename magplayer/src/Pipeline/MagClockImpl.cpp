/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

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

_status_t MagClockImpl::flush(){
	return MAG_NO_ERROR;
}

_status_t MagClockImpl::reset(){
	return MAG_NO_ERROR;
}

i64       MagClockImpl::getPlayingTime(){
	return 0;
}

i64       MagClockImpl::getMediaTime(){
	return 0;
}