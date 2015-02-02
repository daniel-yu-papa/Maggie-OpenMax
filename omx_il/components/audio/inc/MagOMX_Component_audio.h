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

#ifndef __MAGOMX_COMPONENT_AUDIO_H__
#define __MAGOMX_COMPONENT_AUDIO_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

DeclareClass(MagOmxComponentAudio, MagOmxComponentImpl);

Virtuals(MagOmxComponentAudio, MagOmxComponentImpl) 
	OMX_ERRORTYPE (*MagOmx_Audio_GetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure);
                    
    OMX_ERRORTYPE (*MagOmx_Audio_SetParameter)(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure);
EndOfVirtuals;

ClassMembers(MagOmxComponentAudio, MagOmxComponentImpl, \
	void (*updateRefTime)(MagOmxComponentAudio thiz, OMX_TICKS timeStamp, OMX_U32 frame_duration); \
)
    MagMutexHandle         mhMutex;

    OMX_BOOL mEnableRefClockUpdate;
    OMX_U32  mRefTimeUpdateInterval; 
    OMX_U32  mAllFrameDuration;

EndOfClassMembers;

#endif