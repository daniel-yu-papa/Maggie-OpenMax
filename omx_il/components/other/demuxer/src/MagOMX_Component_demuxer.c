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

#include "MagOMX_Component_demuxer.h"
#include "MagOMX_Port_video.h"
#include "MagOMX_Port_audio.h"
#include "MagOMX_Port_buffer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompDemux"

#define FRAME_LOOPER_NAME "CompFrameLooper%d"

AllocateClass(MagOmxComponentDemuxer, MagOmxComponentImpl);

static OMX_S32 MagOmxComponentDemuxer_readDataBuffer(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length){
    MAG_DEMUXER_DATA_SOURCE *pDS;
    MagOmxComponent bufComp;
    OMX_CONFIG_DATABUFFER config;
    OMX_ERRORTYPE ret;

    pDS = (MAG_DEMUXER_DATA_SOURCE *)opaque;
    bufComp = ooc_cast(pDS->hBufferComp, MagOmxComponent);

    initHeader(&config, sizeof(OMX_CONFIG_DATABUFFER));
    config.pBuffer  = pData;
    config.nLen     = length;
    config.nDoneLen = 0;

    ret = MagOmxComponentVirtual(bufComp)->GetConfig( opaque, OMX_IndexConfigExtReadData, &config);
    if (ret == OMX_ErrorNone){
        return config.nDoneLen;
    }else{
        return 0;
    }
}

static OMX_S32 MagOmxComponentDemuxer_writeDataBuffer(OMX_PTR opaque, OMX_U8* pData, OMX_U32 length){
    MAG_DEMUXER_DATA_SOURCE *pDS;
    MagOmxComponent bufComp;
    OMX_CONFIG_DATABUFFER config;
    OMX_ERRORTYPE ret;

    pDS = (MAG_DEMUXER_DATA_SOURCE *)opaque;
    bufComp = ooc_cast(pDS->hBufferComp, MagOmxComponent);

    initHeader(&config, sizeof(OMX_CONFIG_DATABUFFER));
    config.pBuffer  = pData;
    config.nLen     = length;
    config.nDoneLen = 0;

    ret = MagOmxComponentVirtual(bufComp)->SetConfig( opaque, OMX_IndexConfigExtWriteData, &config);
    if (ret == OMX_ErrorNone){
        return config.nDoneLen;
    }else{
        return 0;
    }
}
    
static OMX_S64 MagOmxComponentDemuxer_seekDataBuffer(OMX_PTR opaque, OMX_S64 offset, OMX_SEEK_WHENCE whence){
    MAG_DEMUXER_DATA_SOURCE *pDS;
    MagOmxComponent bufComp;
    OMX_CONFIG_SEEKDATABUFFER config;
    OMX_ERRORTYPE ret;

    pDS = (MAG_DEMUXER_DATA_SOURCE *)opaque;
    bufComp = ooc_cast(pDS->hBufferComp, MagOmxComponent);

    initHeader(&config, sizeof(OMX_CONFIG_SEEKDATABUFFER));
    config.sOffset  = offset;
    config.sWhence  = whence;
    config.sCurPos  = 0;

    ret = MagOmxComponentVirtual(bufComp)->SetConfig( opaque, OMX_IndexConfigExtSeekData, &config);
    if (ret == OMX_ErrorNone){
        return config.sCurPos;
    }else{
        return 0;
    }
}

static void MagOmxComponentDemuxer_StreamEvtCallback(OMX_HANDLETYPE hComponent, 
                                                     MAG_DEMUXER_DATA_SOURCE *pDataSource,
                                                     OMX_DEMUXER_STREAM_INFO *pSInfo){
    MagOmxComponentDemuxer thiz;
    MagOmxComponent        root;
    MagOmxComponentImpl    base;
    MagOmxPort_Constructor_Param_t param;
    OMX_U32 portIndex;
    OMX_ERRORTYPE ret;
    MAG_DEMUXER_OUTPUTPORT_DESC *outputPort;
    MagOmxPort streamPort;

    if (pDataSource == NULL || pSInfo == NULL){
        /*report out the end*/
        base->sendEvents( thiz,
                      OMX_EventExtDynamicPortAdding,
                      kInvalidCompPortNumber,
                      OMX_DynamicPort_Unknown,
                      NULL );
        return;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentDemuxer);
    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);

    outputPort = (MAG_DEMUXER_OUTPUTPORT_DESC *)mag_mallocz(sizeof(MAG_DEMUXER_OUTPUTPORT_DESC));
    if (outputPort == NULL){
        COMP_LOGE(root, "Failed to malloc MAG_DEMUXER_OUTPUTPORT_DESC!!");
        return;
    }
    INIT_LIST(&outputPort->node);

    if (pSInfo->type == OMX_DynamicPort_Video){
        MagOmxPortVideo vPort;

        ret = thiz->getPortIndex(thiz, &portIndex);
        if (ret == OMX_ErrorNone)
            param.portIndex = portIndex;
        else
            return;
        param.isInput       = OMX_FALSE;
        param.bufSupplier   = OMX_BufferSupplyOutput;
        param.formatStruct  = 0;
        sprintf((char *)param.name, "Demuxer-Video-Out-%d", portIndex);

        ooc_init_class(MagOmxPortVideo);
        vPort = ooc_new(MagOmxPortVideo, &param);
        MAG_ASSERT(vPort);

        outputPort->portType = OMX_DynamicPort_Video;
        streamPort = ooc_cast(vPort, MagOmxPort);

        base->addPort(base, portIndex, vPort);
    }else if (pSInfo->type == OMX_DynamicPort_Audio){
        MagOmxPortAudio aPort;
        
        ret = thiz->getPortIndex(thiz, &portIndex);
        if (ret == OMX_ErrorNone)
            param.portIndex = portIndex;
        else
            return;
        param.isInput       = OMX_FALSE;
        param.bufSupplier   = OMX_BufferSupplyOutput;
        param.formatStruct  = 0;
        sprintf((char *)param.name, "Demuxer-Audio-Out-%d", portIndex);

        ooc_init_class(MagOmxPortAudio);
        aPort = ooc_new(MagOmxPortAudio, &param);
        MAG_ASSERT(aPort);

        outputPort->portType = OMX_DynamicPort_Audio;
        streamPort = ooc_cast(aPort, MagOmxPort);

        base->addPort(base, portIndex, aPort);
    }else if (pSInfo->type == OMX_DynamicPort_Subtitle){

    }else{
        COMP_LOGE(root, "invalid dynamic port type: %d!", pSInfo->type);
        return;
    }

    outputPort->portIdx     = portIndex;
    outputPort->hPort       = streamPort;
    outputPort->stream_info = pSInfo;

    pDataSource->streamTable[pSInfo->stream_id] = outputPort;

    base->sendEvents( thiz,
                      OMX_EventExtDynamicPortAdding,
                      portIndex,
                      outputPort->portType,
                      (OMX_PTR)(outputPort) );
}

static void onSendFrameMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent         root;
    MAG_DEMUXER_DATA_SOURCE *pDataSource;
    OMX_ERRORTYPE           ret;         
    MagOmxPort              port;
    OMX_PTR                 frame;
    OMX_U32                 cmd;
    MagOmxComponentDemuxer  thiz;

    pDataSource = (MAG_DEMUXER_DATA_SOURCE *)priv;
    thiz = ooc_cast(pDataSource->hPort->getAttachedComponent(pDataSource->hPort), MagOmxComponentDemuxer);
    root = ooc_cast(thiz, MagOmxComponent);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    if (!msg->findPointer(msg, "av_frame", &frame)){
        COMP_LOGE(root, "failed to find the av_frame!");
        return;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentDemuxer_SendFrameMsg:
        {
            MAG_DEMUXER_OUTPUTPORT_DESC *portDesc;
            MAG_DEMUXER_AVFRAME   *avframe = (MAG_DEMUXER_AVFRAME *)frame;
            OMX_BUFFERHEADERTYPE  *destbufHeader;

            portDesc = pDataSource->streamTable[avframe->frame.stream_id];
            port = portDesc->hPort;
            destbufHeader = MagOmxPortVirtual(port)->GetOutputBuffer(port);

            if (destbufHeader){
                if (avframe){
                    if (avframe->frame.flag != OMX_AVFRAME_FLAG_EOS){
                        destbufHeader->pAppPrivate = (OMX_PTR)(&avframe->frame);
                        destbufHeader->pBuffer     = avframe->frame.buffer;
                        destbufHeader->nAllocLen   = avframe->frame.size;
                        destbufHeader->nFilledLen  = avframe->frame.size;
                        destbufHeader->nOffset     = 0;
                        destbufHeader->nTimeStamp  = avframe->frame.pts;
                    }else{
                        destbufHeader->pAppPrivate = (OMX_PTR)(&avframe->frame);
                        destbufHeader->pBuffer     = NULL;
                        destbufHeader->nAllocLen   = 0;
                        destbufHeader->nFilledLen  = 0;
                        destbufHeader->nOffset     = 0;
                        destbufHeader->nTimeStamp  = kInvalidTimeStamp;
                    }

                    MagOmxPortVirtual(port)->sendOutputBuffer(port, destbufHeader);

                    if (MagOmxComponentDemuxerVirtual(thiz)->MagOMX_Demuxer_ReadFrame){
                        MagOmxComponentDemuxerVirtual(thiz)->MagOMX_Demuxer_ReadFrame(thiz, pDataSource);
                    }else{
                        COMP_LOGE(root, "pure virtual function MagOMX_Demuxer_ReadFrame() should be overrided");
                    }
                }else{
                    COMP_LOGE(root, "Should not be here, Failed to get the frame!");
                }
            }else{
                /*the port is not running, so drop it*/
                avframe->frame.releaseFrame(thiz, avframe);
            }

            break;
        }

        default:
            COMP_LOGE(root, "wrong message %d received!");
            break;
    }
}

static MagOMX_Component_Type_t virtual_MagOmxComponentDemuxer_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Other;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	MagOmxComponentDemuxer thiz;
    MagOmxComponent      root;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentDemuxer);
    root = ooc_cast(hComponent, MagOmxComponent);

	return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	MagOmxComponentDemuxer  thiz;
    MagOmxComponentImpl     base;
    MagOmxComponent         root;
    OMX_ERRORTYPE           ret = OMX_ErrorNone;
    MAG_DEMUXER_DATA_SOURCE *pDataSource;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentDemuxer);
    base = ooc_cast(hComponent, MagOmxComponentImpl);
    root = ooc_cast(hComponent, MagOmxComponent);

    switch (nIndex){
    	case OMX_IndexParamExtDemuxerSetting:
        {
	    	OMX_DEMUXER_SETTING *setting;
            OMX_U32 i;
            MagOmxPort_Constructor_Param_t param;
            OMX_U32 portIndex;
            OMX_ERRORTYPE ret;
            MagOmxPortBuffer bufferPort;

            setting = (OMX_DEMUXER_SETTING *)pComponentParameterStructure;

            pDataSource = (MAG_DEMUXER_DATA_SOURCE *)mag_mallocz(sizeof(MAG_DEMUXER_DATA_SOURCE));
            if (pDataSource == NULL){
                COMP_LOGE(root, "Failed to malloc MAG_DEMUXER_DATA_SOURCE!!");
                return OMX_ErrorInsufficientResources;
            }
            INIT_LIST(&pDataSource->node);

            if (setting->eBufferType == OMX_BUFFER_TYPE_BYTE){
                ret = thiz->getPortIndex(thiz, &portIndex);
                if (ret == OMX_ErrorNone)
                    param.portIndex    = portIndex;
                else
                    return ret;
                param.isInput      = OMX_TRUE;
                param.bufSupplier  = OMX_BufferSupplyUnspecified;
                param.formatStruct = 0;
                sprintf((char *)param.name, "Demuxer-%s-In-%d", BUFFER_PORT_NAME, portIndex);

                ooc_init_class(MagOmxPortBuffer);
                bufferPort = ooc_new(MagOmxPortBuffer, &param);
                MAG_ASSERT(bufferPort);

                pDataSource->portIndex = portIndex;
                pDataSource->hPort     = ooc_cast(bufferPort, MagOmxPort);

                base->addPort(base, portIndex, bufferPort);

                base->sendEvents( thiz,
                                  OMX_EventExtDynamicPortAdding,
                                  portIndex,
                                  OMX_DynamicPort_Buffer,
                                  (OMX_PTR)(setting->cStreamUrl) );

            }else{
                pDataSource->portIndex = kInvalidCompPortNumber;
            }

            pDataSource->url = mag_strdup(setting->cStreamUrl);
            list_add_tail(&pDataSource->node, &thiz->mDataSourceList);

            COMP_LOGD(root, "%dth: mBufferType[%s], url[%s]",
                             i,
                             setting->eBufferType == OMX_BUFFER_TYPE_BYTE ? "byte" : "frame",
                             setting->cStreamUrl);
        }
    		break;

        case OMX_IndexConfigExtStartDemuxing:
        {
            OMX_DEMUXER_KICKOFF *sKickOff;
            OMX_STRING url;
            List_t *next;
            MAG_DEMUXER_DATA_SOURCE *dataSource;
            MAG_DEMUXER_OUTPUTPORT_DESC *portDesc;
            OMX_U32 i;
            OMX_ERRORTYPE err;

            sKickOff = (OMX_DEMUXER_KICKOFF *)pComponentParameterStructure;
            url = sKickOff->url;

            next = thiz->mDataSourceList.next;
            while(next != &thiz->mDataSourceList){
                dataSource = (MAG_DEMUXER_DATA_SOURCE *)list_entry( next, 
                                                                    MAG_DEMUXER_DATA_SOURCE, 
                                                                    node);

                if (!strcmp(dataSource->url, url)){
                    for (i = 0; i < sKickOff->nStreamNum; i++){
                        if (sKickOff->pnStreamId[i] >= 0){
                            portDesc = dataSource->streamTable[sKickOff->pnStreamId[i]];
                            err = MagOmxPortVirtual(portDesc->hPort)->Run(portDesc->hPort);
                            if (err != OMX_ErrorNone){
                                COMP_LOGE(root, "failed to start the port of the stream id[%d]!", sKickOff->pnStreamId[i]);
                                continue;
                            }
                        }
                    }
                    
                    if (MagOmxComponentDemuxerVirtual(thiz)->MagOMX_Demuxer_ReadFrame){
                        ret = MagOmxComponentDemuxerVirtual(thiz)->MagOMX_Demuxer_ReadFrame(thiz, dataSource);
                    }else{
                        COMP_LOGE(root, "pure virtual function MagOMX_Demuxer_ReadFrame() should be overrided");
                        ret = OMX_ErrorNotImplemented;
                    }
                    break;
                }else{
                    next = next->next;
                }
            }
        }
            break;

    	default:
    		break;
    }

	return ret;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentDemuxer_TearDownTunnel(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_U32 portIdx){

    AGILE_LOGV("enter!");
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Prepare(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPortImpl          portImpl;
    MagOmxComponentDemuxer  thiz;
    MagOmxComponentImpl     base;
    MAG_DEMUXER_DATA_SOURCE *pDataSource;
    OMX_ERRORTYPE           ret;
    List_t                  *next;
    OMX_U32                 index = 0;

    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentDemuxer);
    base = ooc_cast(hComponent, MagOmxComponentImpl);

    next = thiz->mDataSourceList.next;
    while(next != &thiz->mDataSourceList){
        pDataSource = (MAG_DEMUXER_DATA_SOURCE *)list_entry(next, 
                                                            MAG_DEMUXER_DATA_SOURCE, 
                                                            node);

        if (pDataSource->portIndex == kInvalidCompPortNumber){
            continue;
        }

        portImpl     = ooc_cast(pDataSource->hPort, MagOmxPortImpl);

        pDataSource->hBufferComp = portImpl->mTunneledComponent;

        pDataSource->opaque     = pDataSource;
        pDataSource->Data_Read  = thiz->readDataBuffer;
        pDataSource->Data_Write = thiz->writeDataBuffer;
        pDataSource->Data_Seek  = thiz->seekDataBuffer;

        pDataSource->sendFrameMessage = thiz->createSendFrameMessage(thiz, pDataSource, index++, MagOmxComponentDemuxer_SendFrameMsg);

        next = next->next;  
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Preroll(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentDemuxer  thiz;
    MagOmxComponent         root;
    MAG_DEMUXER_DATA_SOURCE *pDataSource;
    List_t                  *next;

    thiz = ooc_cast(hComponent, MagOmxComponentDemuxer);
    root = ooc_cast(hComponent, MagOmxComponent);

    next = thiz->mDataSourceList.next;
    while(next != &thiz->mDataSourceList){
        pDataSource = (MAG_DEMUXER_DATA_SOURCE *)list_entry(next, 
                                                            MAG_DEMUXER_DATA_SOURCE, 
                                                            node);

        if (MagOmxComponentDemuxerVirtual(thiz)->MagOMX_Demuxer_DetectStreams){
            MagOmxComponentDemuxerVirtual(thiz)->MagOMX_Demuxer_DetectStreams( thiz,
                                                                               pDataSource,
                                                                               MagOmxComponentDemuxer_StreamEvtCallback);
        }else{
            COMP_LOGE(root, "pure virtual function MagOMX_Demuxer_DetectStreams() should be overrided");
            return OMX_ErrorNotImplemented;
        }

        next = next->next;
    }
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Flush(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentDemuxer_AddPortOnRequest(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U32 *pPortIdx){
    MagOmxComponentDemuxer  thiz;
    MagOmxComponentImpl     base;
    MagOmxPort_Constructor_Param_t param;
    OMX_ERRORTYPE ret;
    OMX_U32 portIndex;
    MagOmxPortBuffer bufferPort;

    thiz = ooc_cast(hComponent, MagOmxComponentDemuxer);
    base = ooc_cast(hComponent, MagOmxComponentImpl);

    ret = thiz->getPortIndex(thiz, &portIndex);
    if (ret == OMX_ErrorNone)
        param.portIndex    = portIndex;
    else
        return ret;
    param.isInput      = OMX_FALSE;
    param.bufSupplier  = OMX_BufferSupplyOutput;
    param.formatStruct = 0;
    sprintf((char *)param.name, "Demuxer-%s-Out-%d", BUFFER_PORT_NAME, portIndex);

    ooc_init_class(MagOmxPortBuffer);
    bufferPort = ooc_new(MagOmxPortBuffer, &param);
    MAG_ASSERT(bufferPort);

    base->addPort(base, portIndex, bufferPort);
    *pPortIdx = portIndex;

    return OMX_ErrorNone;
}   

static OMX_ERRORTYPE virtual_MagOmxComponentDemuxer_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    OMX_U32 i = 0;
    OMX_HANDLETYPE port;
    MagOmxComponentImpl demuxCompImpl;
    MagOmxComponentDemuxer demuxComp;

    AGILE_LOGV("enter!");

    demuxCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    demuxComp     = ooc_cast(hComponent, MagOmxComponentDemuxer);

    while (demuxComp->mPortMap){
        if ( (demuxComp->mPortMap >> i) & 0x1 ){
            port = demuxCompImpl->getPort(demuxCompImpl, i);
            ooc_delete((Object)port);
            demuxComp->mPortMap &= ~(1 << i);
            i++;
        }
    }

    return OMX_ErrorNone;
}

static MagMessageHandle MagOmxComponentDemuxer_createSendFrameMessage(MagOmxComponentDemuxer thiz,
                                                                      MAG_DEMUXER_DATA_SOURCE *pDataSource, 
                                                                      OMX_U32 index, 
                                                                      OMX_U32 what) {
    MagMessageHandle msg;

    thiz->getSendFrameLooper(pDataSource, index);
    
    msg = createMagMessage(pDataSource->sendFrameLooper, what, pDataSource->sendFrameHandler->id(pDataSource->sendFrameHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static OMX_ERRORTYPE MagOmxComponentDemuxer_getSendFrameLooper(MAG_DEMUXER_DATA_SOURCE *pDataSource, OMX_U32 index){
    char looper_name[128];

    if ((NULL != pDataSource->sendFrameLooper) && (NULL != pDataSource->sendFrameHandler)){
        return OMX_ErrorNone;
    }
    
    if (NULL == pDataSource->sendFrameLooper){
        sprintf(looper_name, FRAME_LOOPER_NAME, index);
        pDataSource->sendFrameLooper = createLooper(looper_name);
    }
    
    if (NULL != pDataSource->sendFrameLooper){
        if (NULL == pDataSource->sendFrameHandler){
            pDataSource->sendFrameHandler = createHandler(pDataSource->sendFrameLooper, onSendFrameMessageReceived, pDataSource);

            if (NULL != pDataSource->sendFrameHandler){
                pDataSource->sendFrameLooper->registerHandler(pDataSource->sendFrameLooper, pDataSource->sendFrameHandler);
                pDataSource->sendFrameLooper->start(pDataSource->sendFrameLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return OMX_ErrorInsufficientResources;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", looper_name);
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentDemuxer_getPortIndex(MagOmxComponentDemuxer thiz, OMX_U32 *pIdx){
    OMX_U32 i;

    for (i = MAG_DEMUX_START_PORT_INDEX; i < (MAX_DYNAMIC_PORT_NUMBER + MAG_DEMUX_START_PORT_INDEX); i++){
        if (!( (thiz->mPortMap >> i) & 0x1 )){
            *pIdx = i;
            thiz->mPortMap |= (1 << i);
            return OMX_ErrorNone;
        }
    }

    return OMX_ErrorUndefined;
}

static OMX_ERRORTYPE MagOmxComponentDemuxer_returnPortIndex(MagOmxComponentDemuxer thiz, OMX_U32 Index){
    thiz->mPortMap &= ~(1 << Index);

    return OMX_ErrorNone;
}

static MAG_DEMUXER_AVFRAME *MagOmxComponentDemuxer_getAVFrame(MagOmxComponentDemuxer thiz){
    MAG_DEMUXER_AVFRAME *frame = NULL;
    List_t *next;

    if ( is_list_empty(&thiz->mFreeAVFrameList) ){
        frame = (MAG_DEMUXER_AVFRAME *)mag_mallocz(sizeof(MAG_DEMUXER_AVFRAME));
        INIT_LIST(&frame->node);
        frame->frame.opaque = frame;
    }else{
        next = thiz->mFreeAVFrameList.next;
        frame = (MAG_DEMUXER_AVFRAME *)list_entry(next, 
                                                  MAG_DEMUXER_AVFRAME, 
                                                  node);
        list_del(next);
    }
    return frame;
}

static void MagOmxComponentDemuxer_putAVFrame(MagOmxComponentDemuxer thiz, MAG_DEMUXER_AVFRAME *avframe){
    list_add_tail(&avframe->node, &thiz->mFreeAVFrameList);
}

/*Class Constructor/Destructor*/
static void MagOmxComponentDemuxer_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_getType           = virtual_MagOmxComponentDemuxer_getType;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter      = virtual_MagOmxComponentDemuxer_GetParameter;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter      = virtual_MagOmxComponentDemuxer_SetParameter;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_TearDownTunnel    = virtual_MagOmxComponentDemuxer_TearDownTunnel;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Prepare           = virtual_MagOmxComponentDemuxer_Prepare;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Preroll           = virtual_MagOmxComponentDemuxer_Preroll;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Stop              = virtual_MagOmxComponentDemuxer_Stop;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Start             = virtual_MagOmxComponentDemuxer_Start;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Pause             = virtual_MagOmxComponentDemuxer_Pause;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Resume            = virtual_MagOmxComponentDemuxer_Resume;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Flush             = virtual_MagOmxComponentDemuxer_Flush;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_AddPortOnRequest  = virtual_MagOmxComponentDemuxer_AddPortOnRequest;
    MagOmxComponentDemuxerVtableInstance.MagOmxComponentImpl.MagOMX_Deinit            = virtual_MagOmxComponentDemuxer_Deinit;

}

static void MagOmxComponentDemuxer_constructor(MagOmxComponentDemuxer thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentDemuxer));
    chain_constructor(MagOmxComponentDemuxer, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);

    Mag_CreateEventGroup(&thiz->mBufferFreeEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mBufferFreeEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mBufferFreeEvtGrp, thiz->mBufferFreeEvt);

    thiz->mBufferType = OMX_BUFFER_TYPE_BYTE;
    thiz->mPortMap    = 0;
    INIT_LIST(&thiz->mDataSourceList);
    INIT_LIST(&thiz->mFreeAVFrameList);

    thiz->getSendFrameLooper     = MagOmxComponentDemuxer_getSendFrameLooper;
    thiz->createSendFrameMessage = MagOmxComponentDemuxer_createSendFrameMessage;
    thiz->getPortIndex           = MagOmxComponentDemuxer_getPortIndex;
    thiz->returnPortIndex        = MagOmxComponentDemuxer_returnPortIndex;
    thiz->readDataBuffer         = MagOmxComponentDemuxer_readDataBuffer;
    thiz->writeDataBuffer        = MagOmxComponentDemuxer_writeDataBuffer;
    thiz->seekDataBuffer         = MagOmxComponentDemuxer_seekDataBuffer;
    thiz->getAVFrame             = MagOmxComponentDemuxer_getAVFrame;
    thiz->putAVFrame             = MagOmxComponentDemuxer_putAVFrame;
}

static void MagOmxComponentDemuxer_destructor(MagOmxComponentDemuxer thiz, MagOmxComponentDemuxerVtable vtab){
    List_t *next;
    MAG_DEMUXER_DATA_SOURCE     *pDataSource = NULL;
    MAG_DEMUXER_OUTPUTPORT_DESC *pOPort = NULL;
    MAG_DEMUXER_AVFRAME         *pFrame  = NULL;
    OMX_U32 i;

	AGILE_LOGV("Enter!");

    while (!is_list_empty(&thiz->mDataSourceList)){
        next = thiz->mDataSourceList.next;
        pDataSource = (MAG_DEMUXER_DATA_SOURCE *)list_entry(next, 
                                                            MAG_DEMUXER_DATA_SOURCE, 
                                                            node);
        for (i = 0; i < pDataSource->streamNumber; i++){
            pOPort = pDataSource->streamTable[i];
            mag_freep((void **)&pOPort);
        }
        mag_freep((void **)pDataSource->streamTable);

        destroyMagMessage(&pDataSource->sendFrameMessage);
        destroyHandler(&pDataSource->sendFrameHandler);
        destroyLooper(&pDataSource->sendFrameLooper);

        list_del(next);
        mag_freep((void **)&pDataSource);
    }

    while (!is_list_empty(&thiz->mFreeAVFrameList)){
        next = thiz->mFreeAVFrameList.next;
        pFrame = (MAG_DEMUXER_AVFRAME *)list_entry(next, 
                                                   MAG_DEMUXER_AVFRAME, 
                                                   node);
        list_del(next);
        mag_freep((void **)&pFrame);
    }

    Mag_DestroyMutex(&thiz->mhMutex);
    Mag_DestroyEvent(&thiz->mBufferFreeEvt);
    Mag_DestroyEventGroup(&thiz->mBufferFreeEvtGrp);
    
}