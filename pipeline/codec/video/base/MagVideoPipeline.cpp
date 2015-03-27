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

_status_t MagVideoPipeline::getDecodedFrame(void **ppVideoFrame){
	return getPipelineImpl()->getDecodedFrame(ppVideoFrame);
}

_status_t MagVideoPipeline::putUsedFrame(void *pVideoFrame){
	return getPipelineImpl()->putUsedFrame(pVideoFrame);
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