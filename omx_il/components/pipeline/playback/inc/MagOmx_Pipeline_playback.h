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

#ifndef __MAGOMX_PIPELINE_PLAYBACK_H__
#define __MAGOMX_PIPELINE_PLAYBACK_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

typedef struct {
    List_t node;

    OMX_U32             demuxerOutPortIdx;

    MagOmxPipelineCodec pipeline;
    MAG_DEMUXER_OUTPUTPORT_DESC *portDesc;

    OMX_U32             pipelineInPortIdx;
    OMX_U32             pipelineOutPortIdx;

    OMX_U32             pipelineClkPortIdx;
    OMX_U32             clockCompOutPortIdx;

    MagEventHandle      pipelineStateEvent;
}CodecPipelineEntry;

typedef struct {
    List_t node;

    OMX_STRING      url;
    OMX_BOOL        free_run;

    OMX_BUFFER_TYPE buffer_type;
    OMX_U32         buffer_size; 
    OMX_U32         buffer_time;
    OMX_U32         buffer_low_threshold;
    OMX_U32         buffer_high_threshold;

    OMX_HANDLETYPE  hDemuxerComponent;
    MagEventHandle  demuxerCompStateEvent;
    OMX_S32         demuxerInbufportidx;

    OMX_HANDLETYPE  hBufferComponent;
    MagEventHandle  bufCompStateEvent;

    OMX_HANDLETYPE  hDataSrcComponent;
    MagEventHandle  dsCompStateEvent;

    MagEventGroupHandle stateTransitEvtGrp;

    List_t codecPipelineList;

    MagEventGroupHandle cplStateTransitEvtGrp;
}PlaybackPipelineDatasource;

DeclareClass(MagOmxPipelinePlayback, MagOmxComponentImpl);

Virtuals(MagOmxPipelinePlayback, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    OMX_ERRORTYPE (*MagOMX_Playback_CreateCodecPipeline)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_DYNAMIC_PORT_TYPE plt,
                                    OMX_IN OMX_PTR pAppData,
                                    OMX_IN OMX_CALLBACKTYPE *pCb,
                                    OMX_OUT MagOmxPipelineCodec *phCodecPipeline);

EndOfVirtuals;

ClassMembers(MagOmxPipelinePlayback, MagOmxComponentImpl, \
    OMX_ERRORTYPE (*demuxerCb_EventHandler)(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData); \

    OMX_ERRORTYPE (*bufferCb_EventHandler)(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData); \

    OMX_ERRORTYPE (*datasourceCb_EventHandler)(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData); \

    OMX_ERRORTYPE (*codecPipelineCb_EventHandler)(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData); \

    OMX_ERRORTYPE (*clockCb_EventHandler)(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData); \
)
    List_t              mDataSourceList;  

    OMX_HANDLETYPE      mhClockComponent;
    MagEventHandle      mClkCompStateEvent;
    OMX_CALLBACKTYPE    mClockCallbacks;

    OMX_CALLBACKTYPE    mDemuxerCallbacks;
    OMX_CALLBACKTYPE    mBufferCallbacks;
    OMX_CALLBACKTYPE    mDataSrcCallbacks;

    MagEventHandle      mClockStateEvent;
    MagEventGroupHandle mClockStateEvtGrp;

    MagEventHandle      mCodecPlAllReadyEvent;
    MagEventGroupHandle mCodecPlAllReadyEvtGrp;

    OMX_U32             mPortNumber;
    MagOmxPipelineCompMap mOutputPortMap[MAG_PIPELINE_MAX_OUTPUT_PORTS];
EndOfClassMembers;

#endif