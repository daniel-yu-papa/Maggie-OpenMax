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

#include "MagAudioPipeline.h"
#include "MagPipelineFactory.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

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

_status_t MagAudioPipeline::init(i32 trackID, TrackInfo_t *sInfo){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->init(trackID, sInfo);
}

_status_t MagAudioPipeline::setup(){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->setup();
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

_status_t MagAudioPipeline::getClkConnectedComp(i32 *port, void **ppComp){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->getClkConnectedComp(port, ppComp);
}

_status_t MagAudioPipeline::setVolume(fp32 leftVolume, fp32 rightVolume){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->setVolume(leftVolume, rightVolume);
}

_status_t MagAudioPipeline::getDecodedFrame(void **ppAudioFrame){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->getDecodedFrame(ppAudioFrame);
}

_status_t MagAudioPipeline::putUsedFrame(void *pAudioFrame){
	MagAudioPipelineImplBase *apl = getPipelineImpl();
	return apl->putUsedFrame(pAudioFrame);
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