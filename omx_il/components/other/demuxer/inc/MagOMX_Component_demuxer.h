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

#ifndef __MAGOMX_COMPONENT_OTHER_DEMUXER_H__
#define __MAGOMX_COMPONENT_OTHER_DEMUXER_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

#define MAX_DYNAMIC_PORT_NUMBER 64
#define MAX_STREAMS_NUMBER      64

typedef struct MagOmx_Data_Operation{
    OMX_PTR    opaque;
    OMX_STRING url;
    OMX_S32 (*Data_Read)(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length);
    OMX_S32 (*Data_Write)(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length);
    OMX_S64 (*Data_Seek)(OMX_PTR opaque, OMX_S64 offset, OMX_SEEK_WHENCE whence);
}MagOmx_Data_Operation;

typedef void (*StreamEvtCallback)(OMX_HANDLETYPE hComponent, OMX_DEMUXER_STREAM_INFO sInfo) cbNewStreamAdded

enum{
    MagOmxComponentDemuxer_AVFrameMsg = 0
};

typedef struct MAG_DEMUXER_PORT_DESC
{
    OMX_U32                 portIdx;
    MagOmxPort              hPort;
    OMX_DYNAMIC_PORT_TYPE   portType;
    union {
        OMX_STRING              url;
        OMX_DEMUXER_STREAM_INFO stream_info;
    }info;
}MAG_DEMUXER_PORT_DESC;

typedef enum MAG_DEMUXER_AVFRAME_FLAG{
  MAG_AVFRAME_FLAG_NONE        = 0,
  MAG_AVFRAME_FLAG_KEY_FRAME   = (1 << 0),
  MAG_AVFRAME_FLAG_EOS         = (1 << 1),
} MAG_DEMUXER_AVFRAME_FLAG;

typedef struct MAG_DEMUXER_AVFRAME{
    OMX_U32                  stream_id;    /*the id of the stream that the frame belongs to*/
    OMX_U32                  size;         /*the size of frame buffer*/
    OMX_U8                   *buffer;      /*point to frame buffer*/
    OMX_TICKS                pts;          /*in 90K*/
    OMX_TICKS                dts;          /*in 90K*/  

    OMX_S32                  duration;     /*Duration of this packet in ns*/
    OMX_S64                  position;     /*byte position in stream*/
    MAG_DEMUXER_AVFRAME_FLAG flag;
}MAG_DEMUXER_AVFRAME;

DeclareClass(MagOmxComponentDemuxer, MagOmxComponentImpl);

Virtuals(MagOmxComponentDemuxer, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    OMX_ERRORTYPE (*MagOMX_Demuxer_SetUrl)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING url);

    OMX_ERRORTYPE (*MagOMX_Demuxer_SetDataSource)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN MagOmx_Data_Operation *pSource);

    OMX_ERRORTYPE (*MagOMX_Demuxer_SetAVFrameMsg)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN MagMessageHandle msg);

    OMX_ERRORTYPE (*MagOMX_Demuxer_DetectStreams)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN cbNewStreamAdded fn);

    OMX_ERRORTYPE (*MagOMX_Demuxer_StartDemuxing)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING url);
EndOfVirtuals;

ClassMembers(MagOmxComponentDemuxer, MagOmxComponentImpl, \
    _status_t (*avFrameLooper)(OMX_HANDLETYPE handle); \
    MagMessageHandle (*createAVFrameMessage)(OMX_HANDLETYPE handle, ui32 what);  \
    OMX_ERRORTYPE (*getPortIndex)(MagOmxComponentDemuxer thiz, OMX_U32 *pIdx); \
    OMX_ERRORTYPE (*returnPortIndex)(MagOmxComponentDemuxer thiz, OMX_U32 Index); \
    MagOmx_Data_Operation (*getDataHandler)(MagOmxComponentDemuxer thiz); \
)
    MagMutexHandle         mhMutex;
    
    OMX_BUFFER_TYPE        mBufferType;

    MagLooperHandle        mAVFrameLooper;
    MagHandlerHandle       mAVFrameMsgHandler;
    MagMessageHandle       mAVFrameMsg;

    MagEventHandle         mBufferFreeEvt;
    MagEventGroupHandle    mBufferFreeEvtGrp;

    OMX_U64                mPortMap;
    MAG_DEMUXER_PORT_DESC  mPortTable[MAX_DYNAMIC_PORT_NUMBER];

    MAG_DEMUXER_PORT_DESC  *mStreamTable[MAX_STREAMS_NUMBER];

    MagOmx_Data_Operation  mDataOperation;
EndOfClassMembers;

#endif