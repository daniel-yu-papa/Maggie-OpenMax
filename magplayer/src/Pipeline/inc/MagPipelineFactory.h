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

#ifndef __MAGPLAYER_PIPELINE_FACTORY_H__
#define __MAGPLAYER_PIPELINE_FACTORY_H__

#include "MagSingleton.h"
#include "MagAudioPipelineImplBase.h"
#include "MagVideoPipelineImplBase.h"
#include "MagClockImplBase.h"

typedef enum{
	MAG_OMX_PIPELINE
}Pipeline_Type_t;

typedef enum{
	MAG_OMX_CLOCK
}Clock_Type_t;

class MagPipelineFactory : public MagSingleton<MagPipelineFactory>{
    friend class MagSingleton<MagPipelineFactory>;
public:
	MagPipelineFactory();
	virtual ~MagPipelineFactory();

	MagVideoPipelineImplBase *createVideoPipeline(Pipeline_Type_t type);
	MagAudioPipelineImplBase *createAudioPipeline(Pipeline_Type_t type);
	MagClockImplBase         *createClock(Clock_Type_t type);
};

#endif