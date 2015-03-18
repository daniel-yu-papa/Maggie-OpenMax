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

#ifndef __MAGOMX_COMPONENT_OTHER_BUFFER_H__
#define __MAGOMX_COMPONENT_OTHER_BUFFER_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

enum{
    MagOmxComponentBuffer_PushBufferMsg = 0
};

DeclareClass(MagOmxComponentBuffer, MagOmxComponentImpl);

Virtuals(MagOmxComponentBuffer, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    
EndOfVirtuals;

ClassMembers(MagOmxComponentBuffer, MagOmxComponentImpl, \
    _status_t getPushBufferLooper(OMX_HANDLETYPE handle); \
    MagMessageHandle createPushBufMessage(OMX_HANDLETYPE handle, ui32 what);  \
    MagOmxMediaBuffer_t *(*Queue_Get)(MagOmxComponentBuffer compBuf); \
    OMX_ERRORTYPE (*Queue_Add)(MagOmxComponentBuffer compBuf, MagOmxMediaBuffer_t *mb); \
    OMX_ERRORTYPE (*Queue_Put)(MagOmxComponentBuffer compBuf, MagOmxMediaBuffer_t *mb); \
    OMX_ERRORTYPE (*Queue_Flush)(MagOmxComponentBuffer compBuf); \
    OMX_S32       (*Ringbuffer_Read)(MagOmxComponentBuffer compBuf, OMX_U8* pData, OMX_U32 length); \
    OMX_ERRORTYPE (*Ringbuffer_Write)(MagOmxComponentBuffer compBuf, MagOmxMediaBuffer_t *mb); \
    OMX_ERRORTYPE (*Ringbuffer_Seek)(MagOmxComponentBuffer compBuf, OMX_S64 offset, OMX_SEEK_WHENCE whence); \
    void          (*CalcBufferPercentage)(MagOmxComponentBuffer compBuf); \

)
    MagMutexHandle         mhMutex;
    
    OMX_BUFFER_MODE        mMode;

    MagLooperHandle        mPushBufLooper;
    MagHandlerHandle       mPushBufMsgHandler;
    MagMessageHandle       mPushBufferMsg;

    /*keep buffering until the buffer level reaches mHighPercent*/
    OMX_U32                mHighPercent;         /*High threshold for buffering to finish. Allowed [0, 100]. default: 99*/
    OMX_U32                mLowPercent;          /*Low threshold for buffering to start. Allowed [0, 100]. default: 10 (no need for now)*/
    OMX_U32                mCurrentLevel;        /*current buffer percentage*/

    /*Data is queued until one of the limits specified by the “max-size-buffers”, “max-size-bytes” and/or “max-size-time” properties has been reached.*/
    OMX_U32                mMaxSizeBuffers;      /*Max. number of buffers in the queue*/
    OMX_U32                mMaxSizeBytes;        /*Max. amount of data in the queue in Bytes*/
    OMX_U32                mMaxSizeTime;         /*Max. amount of data in the queue in ns*/
    OMX_U32                mRingBufferMaxSize;   /*The maximum size of the ring buffer in bytes. If it is 0, the ring buffer is disabled*/
    OMX_U32                mUseRateEstimate;     /*Estimate the bitrate of the stream to calculate time level.(in bytes/second)*/

    MagRingBufferHandle    mhRingBuffer;
    magMempoolHandle       mhFrameBufferPool;
    List_t                 mFrameBufferList;
    List_t                 mFrameBufferFreeList;

    OMX_TICKS              mFrameBufferStartPTS;
    OMX_TICKS              mFrameBufferEndPTS;
    OMX_S32                mFrameBufferDuration;  //the duration of the frame buffer (in ns)
    OMX_U32                mFrameBufferNumber;    //the number of the frames in the buffer
    
    OMX_U32                mBufferBytes;          //the bytes of the frame buffer OR ring buffer

    OMX_BOOL               mbWaitOnFreeSpace;
    MagEventHandle         mBufferFreeEvt;
    MagEventGroupHandle    mBufferFreeEvtGrp;

EndOfClassMembers;

#endif