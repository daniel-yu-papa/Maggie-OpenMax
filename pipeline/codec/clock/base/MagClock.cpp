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

i64       MagClock::getMediaTime(){
	MagClockImplBase *clk = getClockImpl();
	return clk->getMediaTime();
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