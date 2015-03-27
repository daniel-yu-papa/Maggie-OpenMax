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

#include "MagOMX_Component_DataSource.h"
#include "MagOMX_Port_buffer.h"

#define DATA_SOURCE_LOOPER_NAME        "CompDataSrcLooper"

AllocateClass(MagOmxPipelineCodec, MagOmxComponentImpl);

static void onReadDataMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent       root;
    MagOmxComponentImpl   base;
    MagOmxPipelineCodec thiz;
    OMX_ERRORTYPE         ret;         
    MagOmxPort            port;
    MagOmxPortBuffer      portBuf;
    OMX_HANDLETYPE        hPort;
    OMX_U32               cmd;
    RBTreeNodeHandle      n;

    root = ooc_cast(priv, MagOmxComponent);
    base = ooc_cast(priv, MagOmxComponentImpl);
    thiz = ooc_cast(priv, MagOmxPipelineCodec);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        break;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxPipelineCodec_ReadDataMsg:
        {
            MagOmxStreamFrame_t  *frame = NULL;
            OMX_BUFFERHEADERTYPE *destbufHeader;

            destbufHeader = MagOmxPortVirtual(port)->GetOutputBuffer(port);

            if (MagOmxPipelineCodecVirtual(thiz)->MagOMX_DataSource_Read){
                MagOmxPipelineCodecVirtual(thiz)->MagOMX_DataSource_Read(thiz, destbufHeader);
                MagOmxPortVirtual(port)->sendOutputBuffer(port, destbufHeader);
            }else{
                COMP_LOGE(root, "pure virtual function MagOMX_DataSource_Read() should be overrided");
                return OMX_ErrorNotImplemented;
            }

            msg->postMessage(msg, 0);
            break;
        }

        default:
            COMP_LOGE(root, "wrong message %d received!");
            break;
    }
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_GetParameter(
                                    OMX_IN  OMX_HANDLETYPE hComponent, 
                                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                                    OMX_INOUT OMX_PTR pComponentParameterStructure){
    return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_SetParameter(
                                    OMX_IN  OMX_HANDLETYPE hComponent, 
                                    OMX_IN  OMX_INDEXTYPE nIndex,
                                    OMX_IN  OMX_PTR pComponentParameterStructure){
    MagOmxPipelineCodec thiz;
    MagOmxComponent      root;
    OMX_ERRORTYPE        ret = OMX_ErrorNone;

    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);
    root = ooc_cast(hComponent, MagOmxComponent);

    switch (nIndex){
        case OMX_IndexParamCodecPipelineSetting:
        {
            OMX_CODEC_PIPELINE_SETTING *setting;
            MagOmxPipelineCodecComp *entry;
            OMX_U32 i;

            setting = (OMX_CODEC_PIPELINE_SETTING *)pComponentParameterStructure;
            for (i = 0; i < setting->compNum; i++){
                entry = (MagOmxPipelineCodecComp *)mag_mallocz(sizeof(MagOmxPipelineCodecComp));
                INIT_LIST(&entry->node);
                entry->param.role          = mag_strdup(setting->compList[i].role);
                entry->param.buffer_number = setting->compList[i].buffer_number;
                entry->param.buffer_size   = setting->compList[i].buffer_size;
                list_add_tail(&entry->node, &thiz->mLinkList);
            }
        }
            break;

        default:
            break;
    }

    return ret;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Prepare(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelineCodec thiz;
    MagOmxComponent       root;
    OMX_ERRORTYPE         err = OMX_ErrorNone;
    List_t *next;
    MagOmxPipelineCodecComp *compDesc;
    char       compName[128];
    OMX_PORT_PARAM_TYPE portParam;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);
    root = ooc_cast(hComponent, MagOmxComponent);

    next = thiz->mLinkList.next;

    while (next != &thiz->mLinkList){
        compDesc = (MagOmxPipelineCodecComp *)list_entry( next, 
                                                           MagOmxPipelineCodecComp, 
                                                           node);

        err = OMX_ComponentOfRoleEnum(compName, compDesc->param.role, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, compDesc->param.role);

            err = OMX_GetHandle(&compDesc->hComp, compName, (OMX_PTR)hComponent, &compDesc->callbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return MAG_BAD_VALUE;
            }

            initHeader(&portParam, sizeof(OMX_PORT_PARAM_TYPE));
            if (compDesc->param.type == OMX_COMPONENT_VIDEO){
                err = OMX_GetParameter(compDesc->hComp, OMX_IndexParamVideoInit, &portParam);
            }else if (compDesc->param.type == OMX_COMPONENT_AUDIO){
                err = OMX_GetParameter(compDesc->hComp, OMX_IndexParamAudioInit, &portParam);
            }else if (compDesc->param.type == OMX_COMPONENT_OTHER){
                err = OMX_GetParameter(compDesc->hComp, OMX_IndexParamOtherInit, &portParam);
            }else{
                AGILE_LOGE("Unsupportted component type: %d", compDesc->param.type);
                return OMX_ErrorBadParameter;
            }

            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting %s OMX_PORT_PARAM_TYPE parameter", compName);
                return OMX_ErrorBadParameter;
            }

            AGILE_LOGD("get component[%s] param: StartPortNumber-%d, Ports-%d",
                        compName,
                        portParam.nStartPortNumber, portParam.nPorts);

            for (i = portParam.nStartPortNumber; i < portParam.nStartPortNumber + portParam.nPorts; i++){
                portDef.nPortIndex = i;
                err = OMX_GetParameter(compDesc->hComp, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Failed to get %s port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", compName, i);
                    return OMX_ErrorBadParameter;
                }

                portDef.nBufferCountActual              = compDesc->param.buffer_number;
                /*the buffer size is variable in terms of demuxed audio frame*/
                portDef.nBufferSize                     = compDesc->param.buffer_size;
                if (compDesc->param.type == OMX_COMPONENT_VIDEO){
                    portDef.format.video.eCompressionFormat = (OMX_VIDEO_CODINGTYPE)compDesc->param.codec_id;
                }else if (compDesc->param.type == OMX_COMPONENT_AUDIO){
                    portDef.format.audio.eEncoding = (OMX_AUDIO_CODINGTYPE)compDesc->param.codec_id;
                }else if (compDesc->param.type == OMX_COMPONENT_OTHER){
                }else{
                    AGILE_LOGE("Unsupportted component type: %d", compDesc->param.type);
                    return OMX_ErrorBadParameter;
                }   

                err = OMX_SetParameter(compDesc->hComp, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting comp[%s] port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", 
                                compName, i);
                    return MAG_BAD_VALUE;
                }

                if (portDef.eDir == OMX_DirOutput){
                    AGILE_LOGD("get mVDecTunnelOutPortIdx: %d", i);
                    mVDecTunnelOutPortIdx = i;
                }else{
                    AGILE_LOGD("get vDecNoneTunnelPortIdx: %d", i);
                    vDecNoneTunnelPortIdx = i;
                }
            }

            initHeader(&ffmpegData, sizeof(OMX_CONFIG_FFMPEG_DATA_TYPE));
            ffmpegData.avformat = sInfo->avformat;
            ffmpegData.avstream = sInfo->avstream;
            err = OMX_SetParameter(mhVideoDecoder, (OMX_INDEXTYPE)OMX_IndexConfigExtFFMpegData, &ffmpegData);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in setting videoDec component OMX_CONFIG_FFMPEG_DATA_TYPE parameter");
                return MAG_BAD_VALUE;
            }
        }else{
            AGILE_LOGE("Failed to get component name with role name %s", role);
            return MAG_NAME_NOT_FOUND;
        }

    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Preroll(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelineCodec thiz;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);
    if (thiz->mReadDataMsg == NULL){
        thiz->mReadDataMsg = thiz->createReadDataMessage(thiz, MagOmxPipelineCodec_ReadDataMsg);
    }

    thiz->mReadDataMsg->postMessage(pDataSource->mReadDataMsg, 0);
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Pause(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Resume(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Flush(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_ProceedBuffer(
                                        OMX_IN  OMX_HANDLETYPE hComponent, 
                                        OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                                        OMX_IN  OMX_HANDLETYPE hDestPort){
   
    return OMX_ErrorNone;
}

static MagMessageHandle MagOmxPipelineCodec_createReadDataMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxPipelineCodec hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxPipelineCodec);
    hComponent->getReadDataLooper(handle);
    
    msg = createMagMessage(hComponent->mReadDataLooper, what, hComponent->mReadDataMsgHandler->id(hComponent->mReadDataMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxPipelineCodec_getReadDataLooper(OMX_HANDLETYPE handle){
    MagOmxPipelineCodec hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxPipelineCodec);
    
    if ((NULL != hComponent->mReadDataLooper) && (NULL != hComponent->mReadDataMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mReadDataLooper){
        hComponent->mReadDataLooper = createLooper(DATA_SOURCE_LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mReadDataLooper);
    }
    
    if (NULL != hComponent->mReadDataLooper){
        if (NULL == hComponent->mReadDataMsgHandler){
            hComponent->mReadDataMsgHandler = createHandler(hComponent->mReadDataLooper, onReadDataMessageReceived, handle);

            if (NULL != hComponent->mReadDataMsgHandler){
                hComponent->mReadDataLooper->registerHandler(hComponent->mReadDataLooper, hComponent->mReadDataMsgHandler);
                hComponent->mReadDataLooper->start(hComponent->mReadDataLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", DATA_SOURCE_LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

/*Class Constructor/Destructor*/
static void MagOmxPipelineCodec_initialize(Class this){
    AGILE_LOGV("Enter!");
    
    /*Override the base component pure virtual functions*/
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter      = virtual_MagOmxPipelineCodec_GetParameter;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter      = virtual_MagOmxPipelineCodec_SetParameter;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Prepare           = virtual_MagOmxPipelineCodec_Prepare;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Preroll           = virtual_MagOmxPipelineCodec_Preroll;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Stop              = virtual_MagOmxPipelineCodec_Stop;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Start             = virtual_MagOmxPipelineCodec_Start;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Pause             = virtual_MagOmxPipelineCodec_Pause;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Resume            = virtual_MagOmxPipelineCodec_Resume;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Flush             = virtual_MagOmxPipelineCodec_Flush;
}

static void MagOmxPipelineCodec_constructor(MagOmxPipelineCodec thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxPipelineCodec));
    chain_constructor(MagOmxPipelineCodec, thiz, params);

    thiz->mReadDataLooper        = NULL;
    thiz->mReadDataMsgHandler    = NULL;
    thiz->mReadDataMsg           = NULL;
    
    thiz->getReadDataLooper      = MagOmxPipelineCodec_getReadDataLooper;
    thiz->createReadDataMessage  = MagOmxPipelineCodec_createReadDataMessage;
}

static void MagOmxPipelineCodec_destructor(MagOmxPipelineCodec thiz, MagOmxPipelineCodecVtable vtab){
    AGILE_LOGV("Enter!");

    destroyMagMessage(&thiz->mReadDataMsg);
    destroyHandler(&thiz->mReadDataMsgHandler);
    destroyLooper(&thiz->mReadDataLooper);
}