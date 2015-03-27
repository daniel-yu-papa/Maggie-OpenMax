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

#include "MagOMX_Component_buffer.h"
#include "MagOMX_Port_buffer.h"
#include "MagOMX_Port_DataSource.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompBuf"

#define COMPONENT_NAME "OMX.Mag.buffer"
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      2

#define BUFFER_LOOPER_NAME        "CompBufferLooper"

AllocateClass(MagOmxComponentBuffer, MagOmxComponentImpl);

static void onPushBufferMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent       root;
    MagOmxComponentImpl   base;
    MagOmxComponentBuffer thiz;
    OMX_ERRORTYPE         ret;         
    MagOmxPort            port;
    MagOmxPortBuffer      portBuf;
    OMX_HANDLETYPE        hPort;
    OMX_U32 cmd;

    root = ooc_cast(priv, MagOmxComponent);
    base = ooc_cast(priv, MagOmxComponentImpl);
    thiz = ooc_cast(priv, MagOmxComponentBuffer);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    if (!msg->findUInt32(msg, "dest_port", &hPort)){
        COMP_LOGE(root, "failed to find the hPort!");
        return;
    }
    port = ooc_cast(priv, MagOmxPort);

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentBuffer_PushBufferMsg:
        {
            MagOmxStreamFrame_t  *frame = NULL;
            OMX_BUFFERHEADERTYPE *destbufHeader;

            destbufHeader = MagOmxPortVirtual(port)->GetOutputBuffer(port);

            frame = thiz->Queue_Get(thiz);
            if (frame){
                destbufHeader->pBuffer    = frame;
                destbufHeader->nFilledLen = frame->size;
                destbufHeader->nTimeStamp = frame->pts;
                MagOmxPortVirtual(port)->sendOutputBuffer(port, destbufHeader);
            }else{
                AGILE_LOGE("Should not be here, Failed to get the frame!");
            }

            break;
        }

        default:
            COMP_LOGE(root, "wrong message %d received!");
            break;
    }
}

static MagOMX_Component_Type_t virtual_MagOmxComponentBuffer_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Other;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	MagOmxComponentBuffer thiz;
    MagOmxComponent      root;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentBuffer);
    root = ooc_cast(hComponent, MagOmxComponent);

    switch (nParamIndex){
    	case OMX_IndexParamExtBufferStatus:
        {
	    	OMX_BUFFER_STATUS *pStatus;

            pStatus = (OMX_BUFFER_STATUS *)pComponentParameterStructure;

            pStatus->uCurrentLevelFrames = thiz->mFrameBufferNumber;
            pStatus->uCurrentLevelBytes  = thiz->mBufferBytes;
            pStatus->uCurrentLevelTime   = thiz->mFrameBufferDuration;
        }
    		break;

    	default:
    		break;
    }

	return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	MagOmxComponentBuffer thiz;
    MagOmxComponent      root;
    OMX_ERRORTYPE        ret = OMX_ErrorNone;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentBuffer);
    root = ooc_cast(hComponent, MagOmxComponent);

    switch (nIndex){
    	case OMX_IndexParamExtBufferSetting:
        {
	    	OMX_BUFFER_PARAM *param;

            param = (OMX_BUFFER_PARAM *)pComponentParameterStructure;

            if (param->uHighPercent == 0)
                thiz->mHighPercent   = 99;
            else
                thiz->mHighPercent   = param->uHighPercent;

            if (param->uLowPercent == 0)
                thiz->mLowPercent    = 10;
            else
                thiz->mLowPercent    = param->uLowPercent;

            thiz->mMaxSizeBuffers    = param->uMaxSizeBuffers;
            thiz->mMaxSizeBytes      = param->uMaxSizeBytes;
            thiz->mMaxSizeTime       = param->uMaxSizeTime;
            thiz->mRingBufferMaxSize = param->uRingBufferMaxSize;
            thiz->mUseRateEstimate   = param->uUseRateEstimate;

            if (param->mode != OMX_BUFFER_MODE_NONE)
                thiz->mMode          = param->mode;

            AGILE_LOGD("high perc[%d], low perc[%d], max frames[%d], max bytes[%d], max time[%d], ringbuf max size[%d], bitrate[%d], mode[%s]",
                        thiz->mHighPercent, thiz->mLowPercent, thiz->mMaxSizeBuffers, thiz->mMaxSizeBytes, thiz->mMaxSizeTime,
                        thiz->mRingBufferMaxSize, thiz->mUseRateEstimate,
                        thiz->mMode == OMX_BUFFER_MODE_PUSH ? "push" : "pull");
        }
    		break;

        case OMX_IndexConfigExtSeekData:
        {
            OMX_CONFIG_SEEKDATABUFFER *seekConfig;

            seekConfig = (OMX_CONFIG_SEEKDATABUFFER *)pComponentParameterStructure;

            if (MagOmxComponentBufferVirtual(thiz)->MagOMX_SeekData){
                ret = MagOmxComponentBufferVirtual(thiz)->MagOMX_SeekData(thiz, seekConfig->sOffset, seekConfig->sWhence);
            }else{
                COMP_LOGE(root, "pure virtual function MagOMX_SeekData() should be overrided");
                return OMX_ErrorNotImplemented;
            }
        }
            break;

    	default:
    		break;
    }

	return ret;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentBuffer_TearDownTunnel(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_U32 portIdx){
    MagOmxComponentBuffer  clkComp;
    MagOmxComponentImpl   clkCompImpl;
    MagOmxComponent       clkCompRoot; 
    OMX_HANDLETYPE        clkPort;

    AGILE_LOGV("enter!");
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentBuffer thiz;
    MagOmxComponent       root;
    OMX_ERRORTYPE         ret = OMX_ErrorNone;
    OMX_U32               bp_size_time;
    OMX_U32               buffer_pool_size;

    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentBuffer);
    root = ooc_cast(hComponent, MagOmxComponent);

    if (thiz->mRingBufferMaxSize > 0){
        mhRingBuffer = Mag_createRingBuffer(thiz->mRingBufferMaxSize, 0);
    }else{
        bp_size_time = (thiz->mMaxSizeTime * thiz->mUseRateEstimate) / 1000000;

        if (bp_size_time < thiz->mMaxSizeBytes){
            buffer_pool_size = bp_size_time;
        }else{
            buffer_pool_size = thiz->mMaxSizeBytes;
        }

        AGILE_LOGV("buffer pool size by time: %d, param mMaxSizeBytes: %d. final buffer pool size: %d",
                    bp_size_time, thiz->mMaxSizeBytes, buffer_pool_size);

        thiz->mhFrameBufferPool = magMemPoolCreate(buffer_pool_size, 0);
        if (thiz->mhFrameBufferPool == NULL){
            AGILE_LOGE("Failed to create the buffer pool with size %d!", buffer_pool_size);
            return OMX_ErrorInsufficientResources;
        }
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_Flush(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
    MagOmxComponentBuffer compBuf;
    MagOmxStreamFrame_t *frame;

    compBuf = ooc_cast(hComponent, MagOmxComponentBuffer);
    frame = (MagOmxStreamFrame_t *)srcbufHeader->pBuffer;

    if (thiz->mRingBufferMaxSize > 0){
        compBuf->Ringbuffer_Write(compBuf, frame);
    }else{
        compBuf->Queue_Add(compBuf, frame);

        if (compBuf->mMode == OMX_BUFFER_MODE_PUSH){
            if (compBuf->mPushBufferMsg == NULL){
                compBuf->mPushBufferMsg = compBuf->createPushBufMessage(hComponent, MagOmxComponentBuffer_PushBufferMsg);
            }

            compBuf->mPushBufferMsg->setPointer(compBuf->mPushBufferMsg, "dest_port", hDestPort, MAG_FALSE);
            compBuf->mPushBufferMsg->postMessage(compBuf->mPushBufferMsg, 0);
        }
    }

    return OMX_ErrorNone;
}

static  OMX_ERRORTYPE virtual_MagOmxComponentBuffer_ProceedUsedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *usedBufHeader){
    MagOmxComponentBuffer compBuf;
    MagOmxStreamFrame_t *frame;

    compBuf = ooc_cast(hComponent, MagOmxComponentBuffer);
    frame = (MagOmxStreamFrame_t *)srcbufHeader->pBuffer;
    return compBuf->Queue_Put(compBuf, frame);
}

static OMX_U32 virtual_MagOmxComponentBuffer_ReadData(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_OUT OMX_U8* pData,
                    OMX_IN  OMX_U32 length){
    MagOmxComponentBuffer compBuf;

    compBuf = ooc_cast(hComponent, MagOmxComponentBuffer);

    if (compBuf->mRingBufferMaxSize > 0){
        return compBuf->Ringbuffer_Read(compBuf, pData, length);
    }else{
        return 0;
    }
}

static OMX_ERRORTYPE virtual_MagOmxComponentBuffer_SeekData(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_S64 offset,
                    OMX_IN  OMX_SEEK_WHENCE whence){
    MagOmxComponentBuffer compBuf;

    compBuf = ooc_cast(hComponent, MagOmxComponentBuffer);

    if (compBuf->mRingBufferMaxSize > 0){
        return compBuf->Ringbuffer_Seek(compBuf, offset, whence);
    }else{
        return compBuf->Queue_Flush(compBuf);
    }
}

static MagOmxStreamFrame_t* MagOmxComponentBuffer_Queue_Get(MagOmxComponentBuffer compBuf){
    List_t *head;
    MagOmxStreamFrame_t *item = NULL;
    OMX_U32 buf_perc = 0;

    head = compBuf->mFrameBufferList.next;
    if (!is_list_empty(&compBuf->mFrameBufferList)){
        item = (MagOmxStreamFrame_t *)list_entry(head, 
                                                 MagOmxStreamFrame_t, 
                                                 node);
        list_del(head);

        Mag_AcquireMutex(compBuf->mhMutex);
        if (item->duration > 0){
            compBuf->mFrameBufferDuration -= item->duration;
        }else{
            if (item->pts != kInvalidTimeStamp){
                compBuf->mFrameBufferStartPTS = item->pts;
                compBuf->mFrameBufferDuration = (OMX_U32)((compBuf->mFrameBufferEndPTS - compBuf->mFrameBufferStartPTS) * 1000/90);
            }
        }
        compBuf->mFrameBufferNumber--;
        compBuf->mBufferBytes -= item->size;
        Mag_ReleaseMutex(compBuf->mhMutex);

        buf_perc = compBuf->CalcBufferPercentage(compBuf);
        if (buf_perc != compBuf->mCurrentLevel){
            base->sendEvents(compBuf, OMX_EventBufferFlag, buf_perc, 0, NULL);
        }
        compBuf->mCurrentLevel = buf_perc;

        /*AGILE_LOGV("%s: delete buffer(0x%x, pts: 0x%x) from stream track queue", 
                        getInfo()->name, item->buffer, item->pts);*/
    }

    return item;
}

/*it is blocking call*/
static OMX_ERRORTYPE MagOmxComponentBuffer_Queue_Add(MagOmxComponentBuffer compBuf, 
                                                     MagOmxStreamFrame_t *frame){
    MagOmxComponentImpl   base;
    void *buf;
    MagErr_t err;
    MagOmxStreamFrame_t *dest_frame;
    List_t *next = NULL;
    OMX_U32 buf_perc = 0;

    base = ooc_cast(compBuf, MagOmxComponentImpl);

again:
    buf_perc = compBuf->CalcBufferPercentage(compBuf);
    if (buf_perc < mHighPercent){
        err = mhFrameBufferPool->get(mhFrameBufferPool, frame->size, &buf);
        if (MAG_ErrNone == err){
            if (!is_list_empty(&compBuf->mFrameBufferFreeList)){
                next = compBuf->mFrameBufferFreeList.next;
                dest_frame = (MagOmxStreamFrame_t *)list_entry(next, MagOmxStreamFrame_t, node);
                list_del(next);
            }else{
                dest_frame = (MagOmxStreamFrame_t *)mag_mallocz(sizeof(MagOmxStreamFrame_t));
                if (NULL != dest_frame){
                    INIT_LIST(&dest_frame->node);
                }else{
                    AGILE_LOGE("Failed to malloc the MagOmxStreamFrame_t!!");
                    return OMX_ErrorInsufficientResources;
                }
            }

            memcpy(buf, frame->buffer, frame->size);

            dest_frame->buffer   = buf;
            dest_frame->size     = frame->size;
            dest_frame->pts      = frame->pts;
            dest_frame->dts      = frame->dts;
            dest_frame->duration = frame->duration;
            dest_frame->position = frame->position;
            dest_frame->flag     = frame->flag;

            Mag_AcquireMutex(compBuf->mhMutex);
            if (is_list_empty(&compBuf->mFrameBufferList)){
                compBuf->mFrameBufferStartPTS = dest_frame->pts;
            }
            compBuf->mFrameBufferEndPTS = dest_frame->pts;
            compBuf->mFrameBufferNumber++;
            if (dest_frame->duration > 0){
                compBuf->mFrameBufferDuration += dest_frame->duration;
            }else{
                if (dest_frame->pts != kInvalidTimeStamp){
                    compBuf->mFrameBufferDuration = (OMX_U32)((compBuf->mFrameBufferEndPTS - compBuf->mFrameBufferStartPTS) * 1000/90);
                }
            }
            compBuf->mBufferBytes += frame->size;
            Mag_ReleaseMutex(compBuf->mhMutex);

            list_add_tail(&dest_frame->node, &compBuf->mFrameBufferList);
            compBuf->mbWaitOnFreeSpace = OMX_FALSE;

            buf_perc = compBuf->CalcBufferPercentage(compBuf);
            if (buf_perc != compBuf->mCurrentLevel){
                base->sendEvents(compBuf, OMX_EventBufferFlag, buf_perc, 0, NULL);
            }
            compBuf->mCurrentLevel = buf_perc;

            return OMX_ErrorNone;
        }
    }

    Mag_ClearEvent(thiz->mBufferFreeEvt);
    compBuf->mbWaitOnFreeSpace = OMX_TRUE;
    AGILE_LOGE("No room for filling in the frame[%d bytes], wait...", frame->size);
    Mag_WaitForEventGroup(thiz->mBufferFreeEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    goto again;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentBuffer_Queue_Put(MagOmxComponentBuffer compBuf, MagOmxStreamFrame_t *frame){
    if (frame != NULL){
        mhFrameBufferPool->put(mhFrameBufferPool, frame->buffer);
        list_add_tail(&frame->node, &compBuf->mFrameBufferFreeList);
        if (compBuf->mbWaitOnFreeSpace){
            Mag_SetEvent(thiz->mBufferFreeEvt);
        }
        return OMX_ErrorNone;
    }else{
        return OMX_ErrorBadParameter;
    }
}

static OMX_ERRORTYPE MagOmxComponentBuffer_Queue_Flush(MagOmxComponentBuffer compBuf){
    MagOmxComponentImpl compBufImpl;
    List_t *next;
    MagOmxStreamFrame_t *item = NULL;

    compBufImpl = ooc_cast(compBuf, MagOmxComponentImpl);

    compBufImpl->flushPort(compBufImpl, OMX_ALL);
    while (!is_list_empty(&compBuf->mFrameBufferList)){
        next = compBuf->mFrameBufferList.next;
        item = (MagOmxStreamFrame_t *)list_entry(next, 
                                                 MagOmxStreamFrame_t, 
                                                 node);
        list_del(next);
        list_add_tail(&item->node, &compBuf->mFrameBufferFreeList);  
    }

    compBuf->mFrameBufferStartPTS = 0;
    compBuf->mFrameBufferEndPTS   = 0;
    compBuf->mFrameBufferDuration = 0;
    compBuf->mFrameBufferNumber   = 0;
    compBuf->mBufferBytes         = 0;
    compBuf->mCurrentLevel        = 0;

    return OMX_ErrorNone;
}

static OMX_S32 MagOmxComponentBuffer_Ringbuffer_Read(MagOmxComponentBuffer compBuf, OMX_U8* pData, OMX_U32 length){
    OMX_S32 read;

    read = compBuf->mhRingBuffer->read( compBuf->mhRingBuffer, length, pData );
    if (compBuf->mbWaitOnFreeSpace){
        Mag_SetEvent(thiz->mBufferFreeEvt);
    }

    return read;
}

/*blocking call*/
static OMX_ERRORTYPE MagOmxComponentBuffer_Ringbuffer_Write(MagOmxComponentBuffer compBuf, MagOmxMediaBuffer_t *mb){
    OMX_S32 wnum;
    OMX_U32 remaining;

    remaining = mb->size;

write_again:
    wnum = compBuf->mhRingBuffer->write( compBuf->mhRingBuffer, remaining, ((OMX_U8 *)mb->buffer + (mb->size - remaining)) );
    if (wnum < remaining){
        Mag_ClearEvent(thiz->mBufferFreeEvt);
        compBuf->mbWaitOnFreeSpace = OMX_TRUE;
        AGILE_LOGE("No room for filling in the ring buffer[%d vs %d bytes], wait...", wnum, remaining);
        remaining -= wnum;
        Mag_WaitForEventGroup(thiz->mBufferFreeEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        goto write_again;
    }
}

static OMX_ERRORTYPE MagOmxComponentBuffer_Ringbuffer_Seek(MagOmxComponentBuffer compBuf, 
                                                           OMX_IN  OMX_S64 offset,
                                                           OMX_IN  OMX_SEEK_WHENCE whence){
    MagOmxComponentImpl compBufImpl;
    i64 rb_seek_ret;
    OMX_ERRORTYPE source_seek_ret;
    OMX_S64 offsetToStart = -1;
    OMX_S64 start;
    OMX_S64 end;

    if (whence == OMX_SEEK_SET){
        offsetToStart = offset;
    }else if (whence == OMX_SEEK_CUR){
        compBuf->mhRingBuffer->getSourceRange(compBuf->mhRingBuffer, &start, &end);
        offsetToStart = offset + end;
    }else{
        AGILE_LOGE("Don't support OMX_SEEK_END!");
    }

    rb_seek_ret = compBuf->mhRingBuffer->seek(compBuf->mhRingBuffer, offsetToStart);
    if (rb_seek_ret != 0){
        RBTreeNodeHandle n;
        MagOmxPort port;
        MagOmxPortImpl portImpl;
        MagOmxComponent tunneledComp;
        OMX_CONFIG_DATABUFFER config_seek;

        compBufImpl = ooc_cast(compBuf, MagOmxComponentImpl);
        compBufImpl->flushPort(compBufImpl, OMX_ALL);

        for (n = rbtree_first(compBufImpl->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->isInputPort(port)){
                portImpl = ooc_cast(port, MagOmxPortImpl);
                tunneledComp = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);

                compBuf->mhRingBuffer->flush(compBuf->mhRingBuffer);

                config_seek.sOffset = offset;
                config_seek.sWhence = whence;
                source_seek_ret = MagOmxComponentVirtual(tunneledComp)->SetConfig(tunneledComp, 
                                                                                  OMX_IndexConfigExtSeekData, 
                                                                                  &config_seek);
                
                compBuf->mhRingBuffer->setSourcePos(compBuf->mhRingBuffer, config_seek.sCurPos);

                return source_seek_ret;
            }
        }

        return OMX_ErrorUndefined;
    }

    return OMX_ErrorNone;
}

static OMX_U32 MagOmxComponentBuffer_CalcBufferPercentage(MagOmxComponentBuffer compBuf){
    OMX_U32 time_percentage = 0;
    OMX_U32 bytes_percentage = 0;
    OMX_U32 frames_percentage = 0;
    OMX_U32 buffer_percentage = 0;

    Mag_AcquireMutex(compBuf->mhMutex);

    if (compBuf->mRingBufferMaxSize == 0){
        if (compBuf->mMaxSizeTime){
            if (compBuf->mFrameBufferDuration){
                time_percentage = (compBuf->mFrameBufferDuration * 100) / compBuf->mMaxSizeTime;
            }

            if (buffer_percentage < time_percentage){
                buffer_percentage = time_percentage;
            }
        }

        if (compBuf->mMaxSizeBytes){
            bytes_percentage = (compBuf->mBufferBytes * 100) / compBuf->mMaxSizeBytes;

            if (buffer_percentage < bytes_percentage){
                buffer_percentage = bytes_percentage;
            }
        }

        if (compBuf->mMaxSizeBuffers){
            frames_percentage = (compBuf->mFrameBufferNumber * 100) / compBuf->mMaxSizeBuffers;

            if (buffer_percentage < frames_percentage){
                buffer_percentage = frames_percentage;
            }
        }

        AGILE_LOGD("[frame buffer percentage]time: %d, bytes: %d, frames: %d, final: %d",
                    time_percentage, bytes_percentage, frames_percentage, buffer_percentage, )
    }else{
        bytes_percentage = (compBuf->mBufferBytes * 100) / compBuf->mRingBufferMaxSize;
        buffer_percentage = bytes_percentage;
        AGILE_LOGD("[ring buffer percentage]bytes: %d", bytes_percentage);
    }

    Mag_ReleaseMutex(compBuf->mhMutex);

    return buffer_percentage;
}

static MagMessageHandle MagOmxComponentBuffer_createPushBufMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentBuffer hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentBuffer);
    hComponent->getPushBufferLooper(handle);
    
    msg = createMagMessage(hComponent->mPushBufLooper, what, hComponent->mPushBufMsgHandler->id(hComponent->mPushBufMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxComponentBuffer_getPushBufferLooper(OMX_HANDLETYPE handle){
    MagOmxComponentBuffer hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentBuffer);
    
    if ((NULL != hComponent->mPushBufLooper) && (NULL != hComponent->mPushBufMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mPushBufLooper){
        hComponent->mPushBufLooper = createLooper(BUFFER_LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mPushBufLooper);
    }
    
    if (NULL != hComponent->mPushBufLooper){
        if (NULL == hComponent->mPushBufMsgHandler){
            hComponent->mPushBufMsgHandler = createHandler(hComponent->mPushBufLooper, onPushBufferMessageReceived, handle);

            if (NULL != hComponent->mPushBufMsgHandler){
                hComponent->mPushBufLooper->registerHandler(hComponent->mPushBufLooper, hComponent->mPushBufMsgHandler);
                hComponent->mPushBufLooper->start(hComponent->mPushBufLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", BUFFER_LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

/*Class Constructor/Destructor*/
static void MagOmxComponentBuffer_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_getType           = virtual_MagOmxComponentBuffer_getType;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter      = virtual_MagOmxComponentBuffer_GetParameter;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter      = virtual_MagOmxComponentBuffer_SetParameter;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_TearDownTunnel    = virtual_MagOmxComponentBuffer_TearDownTunnel;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Prepare           = virtual_MagOmxComponentBuffer_Prepare;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Preroll           = virtual_MagOmxComponentBuffer_Preroll;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Stop              = virtual_MagOmxComponentBuffer_Stop;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Start             = virtual_MagOmxComponentBuffer_Start;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Pause             = virtual_MagOmxComponentBuffer_Pause;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Resume            = virtual_MagOmxComponentBuffer_Resume;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_Flush             = virtual_MagOmxComponentBuffer_Flush;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_MagOmxComponentBuffer_ProceedBuffer;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_ProceedUsedBuffer = virtual_MagOmxComponentBuffer_ProceedUsedBuffer;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_ReadData          = virtual_MagOmxComponentBuffer_ReadData;
    MagOmxComponentBufferVtableInstance.MagOmxComponentImpl.MagOMX_SeekData          = virtual_MagOmxComponentBuffer_SeekData;
}

static void MagOmxComponentBuffer_constructor(MagOmxComponentBuffer thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentBuffer));
    chain_constructor(MagOmxComponentBuffer, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);

    thiz->mMode                  = OMX_BUFFER_MODE_PUSH;

    thiz->mPushBufLooper         = NULL;
    thiz->mPushBufMsgHandler     = NULL;
    thiz->mPushBufferMsg         = NULL;
    thiz->mhRingBuffer           = NULL;
    thiz->mhFrameBufferPool      = NULL;

    thiz->mHighPercent           = 99;
    thiz->mLowPercent            = 10;
    thiz->mCurrentLevel          = 0;
    thiz->mMaxSizeBuffers        = 0;
    thiz->mMaxSizeBytes          = 0;
    thiz->mMaxSizeTime           = 0;
    thiz->mRingBufferMaxSize     = 0;
    thiz->mUseRateEstimate       = 0;
    thiz->mFrameBufferStartPTS   = 0;
    thiz->mFrameBufferEndPTS     = 0;
    thiz->mFrameBufferDuration   = 0;
    thiz->mFrameBufferNumber     = 0;
    thiz->mBufferBytes           = 0;
    thiz->mbWaitOnFreeSpace      = OMX_FALSE;

    INIT_LIST(&thiz->mFrameBufferList);
    INIT_LIST(&thiz->mFrameBufferFreeList);

    Mag_CreateEventGroup(&thiz->mBufferFreeEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mBufferFreeEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mBufferFreeEvtGrp, thiz->mBufferFreeEvt);

    thiz->getPushBufferLooper    = MagOmxComponentBuffer_getPushBufferLooper;
    thiz->createPushBufMessage   = MagOmxComponentBuffer_createPushBufMessage;
    thiz->Queue_Get              = MagOmxComponentBuffer_Queue_Get;
    thiz->Queue_Put              = MagOmxComponentBuffer_Queue_Put;
    thiz->Queue_Add              = MagOmxComponentBuffer_Queue_Add;
    thiz->Queue_Flush            = MagOmxComponentBuffer_Queue_Flush;

    thiz->Ringbuffer_Read        = MagOmxComponentBuffer_Ringbuffer_Read;
    thiz->Ringbuffer_Write       = MagOmxComponentBuffer_Ringbuffer_Write;
    thiz->Ringbuffer_Seek        = MagOmxComponentBuffer_Ringbuffer_Seek;

    thiz->CalcBufferPercentage   = MagOmxComponentBuffer_CalcBufferPercentage;
}

static void MagOmxComponentBuffer_destructor(MagOmxComponentBuffer thiz, MagOmxComponentBufferVtable vtab){
    List_t *next;
    MagOmxStreamFrame_t *item = NULL;

	AGILE_LOGV("Enter!");

    if (thiz->mhRingBuffer){
        Mag_destroyRingBuffer(&thiz->mhRingBuffer);
    }

    if (thiz->mhFrameBufferPool){
        while (!is_list_empty(&thiz->mFrameBufferFreeList)){
            next = thiz->mFrameBufferFreeList.next;
            item = (MagOmxStreamFrame_t *)list_entry(next, 
                                                     MagOmxStreamFrame_t, 
                                                     node);
            list_del(next);
            mag_freep(&item);
        }

        Mag_destroyMemoryPool(&thiz->mhFrameBufferPool);
    }

    Mag_DestroyMutex(&thiz->mhMutex);
    Mag_DestroyEvent(&thiz->mBufferFreeEvt);
    Mag_DestroyEventGroup(&thiz->mBufferFreeEvtGrp);
    destroyMagMessage(&thiz->mPushBufferMsg);
    destroyHandler(&thiz->mPushBufMsgHandler);
    destroyLooper(&thiz->mPushBufLooper);
}

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPortDataSource     datasourceInPort;
    MagOmxPortBuffer         bufferOutPort;
    MagOmxPort_Constructor_Param_t param;
    MagOmxComponentImpl      bufferCompImpl;
    MagOmxComponent          bufferComp;

    AGILE_LOGV("enter!");

    param.portIndex    = START_PORT_INDEX + 0;
    param.isInput      = OMX_TRUE;
    param.bufSupplier  = OMX_BufferSupplyUnspecified;
    param.formatStruct = 0;
    sprintf((char *)param.name, "%s-In", DATA_SOURCE_PORT_NAME);

    ooc_init_class(MagOmxPortDataSource);
    datasourceInPort = ooc_new(MagOmxPortDataSource, &param);
    MAG_ASSERT(datasourceInPort);

    param.portIndex    = START_PORT_INDEX + 1;
    param.isInput      = OMX_FALSE;
    param.bufSupplier  = OMX_BufferSupplyOutput;
    param.formatStruct = 0;
    sprintf((char *)param.name, "%s-Out", BUFFER_PORT_NAME);

    ooc_init_class(MagOmxPortBuffer);
    bufferOutPort = ooc_new(MagOmxPortBuffer, &param);
    MAG_ASSERT(bufferOutPort);

    bufferCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    bufferComp     = ooc_cast(hComponent, MagOmxComponent);
    
    bufferComp->setName(bufferComp, (OMX_U8 *)COMPONENT_NAME);
    bufferCompImpl->addPort(bufferCompImpl, START_PORT_INDEX + 0, datasourceInPort);
    bufferCompImpl->addPort(bufferCompImpl, START_PORT_INDEX + 1, bufferOutPort);

    bufferCompImpl->setupPortDataFlow(bufferCompImpl, datasourceInPort, bufferOutPort);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentBuffer_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                                OMX_IN  OMX_PTR pAppData,
                                                OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
    MagOmxComponentBuffer hBufferComp;
    MagOmxComponentImpl   parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponentBuffer);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hBufferComp = (MagOmxComponentBuffer) ooc_new( MagOmxComponentBuffer, (void *)param);
    MAG_ASSERT(hBufferComp);

    parent = ooc_cast(hBufferComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hBufferComp, pAppData, pCallBacks);
    if (*hComponent){
        return localSetupComponent(hBufferComp);
    }else{
        return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponentBuffer_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
    OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
    MagOmxComponentBuffer hBufferComp;

    AGILE_LOGD("enter!");
    hBufferComp = (MagOmxComponentBuffer)compType->pComponentPrivate;
    ooc_delete((Object)hBufferComp);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
    static char * roles[] = {OMX_ROLE_BUFFER};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponentBuffer_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
    MagOmxComponentBuffer_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER