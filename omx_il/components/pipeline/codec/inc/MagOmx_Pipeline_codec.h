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

#ifndef __MAGOMX_PIPELINE_CODEC_H__
#define __MAGOMX_PIPELINE_CODEC_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

typedef struct{
    List_t node;
    OMX_CODEC_PIPELINE_COMP_PARAM param;
    OMX_HANDLETYPE   hComp;
    OMX_CALLBACKTYPE callbacks;
    MagEventHandle   stateTransitEvent;
}MagOmxPipelineCodecComp;

typedef struct{
    OMX_HANDLETYPE   hComp;
    OMX_U32          portIdx;
}MagOmxPipelineCompMap;

DeclareClass(MagOmxPipelineCodec, MagOmxComponentImpl);

Virtuals(MagOmxPipelineCodec, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    
EndOfVirtuals;

ClassMembers(MagOmxPipelineCodec, MagOmxComponentImpl, \
    OMX_ERRORTYPE (*compCallback_EventHandler)(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData); \
)
    List_t             mLinkList;
    OMX_PORTDOMAINTYPE mDomain;
    OMX_U32            mPortCount;
    MagOmxPipelineCompMap mOutputPortMap[MAG_PIPELINE_MAX_OUTPUT_PORTS];
    MagEventGroupHandle mStateTransitEvtGrp;

EndOfClassMembers;

#endif