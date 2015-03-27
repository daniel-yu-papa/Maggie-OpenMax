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

#include "MagPipelineFactory.h"
#include "Omxil_VideoPipeline.h"
#include "Omxil_AudioPipeline.h"
#include "Omxil_Clock.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

MAG_SINGLETON_STATIC_INSTANCE(MagPipelineFactory)

MagPipelineFactory::MagPipelineFactory(){

}

MagPipelineFactory::~MagPipelineFactory(){

}

MagVideoPipelineImplBase *MagPipelineFactory::createVideoPipeline(Pipeline_Type_t type){
	MagVideoPipelineImplBase *obj = NULL;

	switch (type){
		case MAG_OMX_PIPELINE:
			obj = new OmxilVideoPipeline();
			break;

		default:
			break;
	}
	return obj;
}

MagAudioPipelineImplBase *MagPipelineFactory::createAudioPipeline(Pipeline_Type_t type){
	MagAudioPipelineImplBase *obj = NULL;

	switch (type){
		case MAG_OMX_PIPELINE:
			obj = new OmxilAudioPipeline();
			break;
			
		default:
			break;
	}
	return obj;
}

MagClockImplBase *MagPipelineFactory::createClock(Clock_Type_t type){
	MagClockImplBase *obj = NULL;

	switch (type){
		case MAG_OMX_CLOCK:
			obj = new OmxilClock();
			break;
			
		default:
			break;
	}
	return obj;
}