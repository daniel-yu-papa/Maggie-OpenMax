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

#define MAG_DEMUX_START_PORT_INDEX    kCompPortStartNumber

typedef void (*StreamEvtCallback)(OMX_HANDLETYPE hComponent,  \
                                  MAG_DEMUXER_DATA_SOURCE *pDataSource, \
                                  OMX_DEMUXER_STREAM_INFO sInfo) cbNewStreamAdded

enum{
    MagOmxComponentDemuxer_SendFrameMsg = 0
};

typedef struct MAG_DEMUXER_OUTPUTPORT_DESC
{
    List_t                  node;

    OMX_U32                 portIdx;
    MagOmxPort              hPort;
    OMX_DYNAMIC_PORT_TYPE   portType;
    OMX_DEMUXER_STREAM_INFO *stream_info;
}MAG_DEMUXER_OUTPUTPORT_DESC;

typedef struct MAG_DEMUXER_DATA_SOURCE{
    List_t     node;

    OMX_STRING url;

    void       *hDemuxer;
    void       *hDemuxerOpts;
    void       *hCodecOpts;

    MagOmxPort hPort;
    OMX_U32    portIndex;

    OMX_PTR    opaque;
    OMX_S32    (*Data_Read)(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length);
    OMX_S32    (*Data_Write)(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length);
    OMX_S64    (*Data_Seek)(OMX_PTR opaque, OMX_S64 offset, OMX_SEEK_WHENCE whence);

    MAG_DEMUXER_PORT_DESC  **streamTable;
    OMX_U32     streamNumber;

    MagLooperHandle  sendFrameLooper;
    MagHandlerHandle sendFrameHandler;
    MagMessageHandle sendFrameMessage;
}MAG_DEMUXER_DATA_SOURCE;

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

    OMX_PTR                   priv;
    void                      (*releaseFrame)(OMX_PTR raw_frame);
}MAG_DEMUXER_AVFRAME;

DeclareClass(MagOmxComponentDemuxer, MagOmxComponentImpl);

Virtuals(MagOmxComponentDemuxer, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    OMX_ERRORTYPE (*MagOMX_Demuxer_DetectStreams)(
                                    OMX_IN OMX_HANDLETYPE hComponent,
                                    MAG_DEMUXER_DATA_SOURCE *pDataSource,
                                    OMX_IN cbNewStreamAdded fn);

    OMX_ERRORTYPE (*MagOMX_Demuxer_ReadFrame)(
                                    OMX_IN OMX_HANDLETYPE   hComponent,
                                    OMX_IN MAG_DEMUXER_DATA_SOURCE *pDataSource);
EndOfVirtuals;

ClassMembers(MagOmxComponentDemuxer, MagOmxComponentImpl, \
    _status_t (*getSendFrameLooper)(OMX_HANDLETYPE handle); \
    MagMessageHandle (*createSendFrameMessage)(OMX_HANDLETYPE handle, ui32 what);  \
    OMX_ERRORTYPE (*getPortIndex)(MagOmxComponentDemuxer thiz, OMX_U32 *pIdx); \
    OMX_ERRORTYPE (*returnPortIndex)(MagOmxComponentDemuxer thiz, OMX_U32 Index); \
    MAG_DATA_SOURCE (*getDataHandler)(MagOmxComponentDemuxer thiz); \
    OMX_S32 (*readDataBuffer)(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length); \
    OMX_S32 (*writeDataBuffer)(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length); \
    OMX_S64 (*seekDataBuffer)(OMX_PTR opaque, OMX_S64 offset, OMX_SEEK_WHENCE whence); \
)
    MagMutexHandle         mhMutex;
    
    OMX_BUFFER_TYPE        mBufferType;

    MagLooperHandle        mAVFrameLooper;
    MagHandlerHandle       mAVFrameMsgHandler;
    MagMessageHandle       mAVFrameMsg;

    MagEventHandle         mBufferFreeEvt;
    MagEventGroupHandle    mBufferFreeEvtGrp;

    OMX_U64                mPortMap;

    List_t                 mDataSourceList;

EndOfClassMembers;

#endif