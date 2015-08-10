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

#include "MagOmx_Pipeline_codec.h"
#include "MagOMX_Component_DataSource.h"
#include "MagOMX_Port_buffer.h"

#define START_PORT_INDEX kCompPortStartNumber

AllocateClass(MagOmxPipelineCodec, MagOmxComponentImpl);

static MagOMX_Component_Type_t virtual_MagOmxPipelineCodec_getType(
                                    OMX_IN  OMX_HANDLETYPE hComponent){
    return MagOMX_Component_Other;
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
    MagOmxPipelineCodec  thiz;
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
            MagOmxCodecPipelineSetting *entry;
            OMX_U32 i;

            setting = (OMX_CODEC_PIPELINE_SETTING *)pComponentParameterStructure;

            thiz->mDomain    = setting->domain;
            thiz->mCompCount = setting->compNum;
            for (i = 0; i < setting->compNum; i++){
                entry = (MagOmxCodecPipelineSetting *)mag_mallocz(sizeof(MagOmxCodecPipelineSetting));
                INIT_LIST(&entry->node);
                entry->param.role          = mag_strdup(setting->compList[i].role);
                entry->param.buffer_number = setting->compList[i].buffer_number;
                entry->param.buffer_size   = setting->compList[i].buffer_size;
                entry->param.type          = setting->compList[i].type;
                entry->param.codec_id      = setting->compList[i].codec_id;
                entry->param.stream_handle = setting->compList[i].stream_handle;
                
                entry->callbacks.EventHandler    = thiz->compCallback_EventHandler;
                entry->callbacks.EmptyBufferDone = NULL;
                entry->callbacks.FillBufferDone  = NULL;

                if (MAG_ErrNone == Mag_CreateEvent(&entry->stateTransitEvent, MAG_EVT_PRIO_DEFAULT)){
                    Mag_AddEventGroup(thiz->mStateTransitEvtGrp, entry->stateTransitEvent);
                }

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
    MagOmxPipelineCodec     thiz;
    MagOmxComponent         root;
    MagOmxComponentImpl     base;

    OMX_ERRORTYPE           err = OMX_ErrorNone;
    List_t                  *next;
    MagOmxCodecPipelineSetting *cpls;
    char                    compName[128];
    OMX_PORT_PARAM_TYPE     portParam;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    OMX_CONFIG_STREAM_HANDLE     codecStrmHandle;

    OMX_S32        port_out_idx     = kInvalidCompPortNumber;
    OMX_HANDLETYPE comp_out_hanlde  = NULL;
    OMX_S32        tmp_port_out_idx = kInvalidCompPortNumber;
    char           comp_out_name[128];
    OMX_HANDLETYPE hPort;
    OMX_U32        i;
    OMX_U32        compIdx = 1;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);
    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);

    initHeader(&portDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    next = thiz->mLinkList.next;
    while (next != &thiz->mLinkList){
        cpls = (MagOmxCodecPipelineSetting *)list_entry( next, 
                                                          MagOmxCodecPipelineSetting, 
                                                          node);

        err = OMX_ComponentOfRoleEnum(compName, cpls->param.role, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, cpls->param.role);

            err = OMX_GetHandle(&cpls->hComp, compName, (OMX_PTR)cpls, &cpls->callbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return err;
            }

            if (cpls->param.stream_handle){
                initHeader(&codecStrmHandle, sizeof(OMX_CONFIG_STREAM_HANDLE));
                codecStrmHandle.hAVStream = cpls->param.stream_handle;
                err = OMX_SetParameter(cpls->hComp, (OMX_INDEXTYPE)OMX_IndexConfigExtStreamHandle, &codecStrmHandle);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting codec component OMX_CONFIG_STREAM_HANDLE parameter");
                    return err;
                }
            }

            initHeader(&portParam, sizeof(OMX_PORT_PARAM_TYPE));
            if (cpls->param.type == OMX_COMPONENT_VIDEO){
                err = OMX_GetParameter(cpls->hComp, OMX_IndexParamVideoInit, &portParam);
            }else if (cpls->param.type == OMX_COMPONENT_AUDIO){
                err = OMX_GetParameter(cpls->hComp, OMX_IndexParamAudioInit, &portParam);
            }else if (cpls->param.type == OMX_COMPONENT_OTHER){
                err = OMX_GetParameter(cpls->hComp, OMX_IndexParamOtherInit, &portParam);
            }else{
                AGILE_LOGE("Unsupportted component type: %d", cpls->param.type);
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
                err = OMX_GetParameter(cpls->hComp, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Failed to get %s port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", compName, i);
                    return err;
                }

                portDef.nBufferCountActual              = cpls->param.buffer_number;
                /*the buffer size is variable in terms of demuxed audio frame*/
                portDef.nBufferSize                     = cpls->param.buffer_size;
                if (cpls->param.type == OMX_COMPONENT_VIDEO){
                    portDef.format.video.eCompressionFormat = (OMX_VIDEO_CODINGTYPE)cpls->param.codec_id;
                }else if (cpls->param.type == OMX_COMPONENT_AUDIO){
                    portDef.format.audio.eEncoding = (OMX_AUDIO_CODINGTYPE)cpls->param.codec_id;
                }else if (cpls->param.type == OMX_COMPONENT_OTHER){
                }else{
                    AGILE_LOGE("Unsupportted component type: %d", cpls->param.type);
                    return OMX_ErrorBadParameter;
                }   

                err = OMX_SetParameter(cpls->hComp, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting comp[%s] port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", 
                                compName, i);
                    return err;
                }

                if (portDef.eDomain == thiz->mDomain){
                    if (portDef.eDir == OMX_DirOutput){
                        if (port_out_idx == kInvalidCompPortNumber){
                            port_out_idx     = i;
                            tmp_port_out_idx = kInvalidCompPortNumber;
                            comp_out_hanlde  = cpls->hComp;
                            strcpy(comp_out_name, compName);
                        }else{
                            tmp_port_out_idx = i;
                        }
                    }else{
                        if (comp_out_hanlde != NULL && comp_out_hanlde != cpls->hComp){
                            if (port_out_idx != kInvalidCompPortNumber){
                                err = OMX_SetupTunnel(comp_out_hanlde, port_out_idx, cpls->hComp, i);
                                if(err != OMX_ErrorNone){
                                    AGILE_LOGE("To setup up tunnel between &s[port: %d] and %s[port: %d] - FAILURE!",
                                                comp_out_name, port_out_idx,
                                                compName, i);
                                    return MAG_UNKNOWN_ERROR;
                                }else{
                                    AGILE_LOGI("To setup tunnel between &s[port: %d] and %s[port: %d] - OK!",
                                                comp_out_name, port_out_idx,
                                                compName, i);
                                }
                            }

                            if (tmp_port_out_idx != kInvalidCompPortNumber){
                                port_out_idx     = tmp_port_out_idx;
                                comp_out_hanlde  = cpls->hComp;
                                strcpy(comp_out_name, compName);
                            }else{
                                port_out_idx    = kInvalidCompPortNumber;
                                comp_out_hanlde = NULL;
                                memset(comp_out_name, 0, 128);
                            }
                            tmp_port_out_idx = kInvalidCompPortNumber;
                        }
                    }
                }

                if ( (compIdx == 1 && portDef.eDir == OMX_DirInput) ||
                     (compIdx == thiz->mCompCount && portDef.eDir == OMX_DirOutput) ||
                     (portDef.eDomain != thiz->mDomain) ){
                    MagOmxComponentImpl cplCompBase = ooc_cast(cpls->hComp, MagOmxComponentImpl);

                    hPort = cplCompBase->getPort(cplCompBase, i);
                    base->addPort(base, START_PORT_INDEX + thiz->mPortCount, hPort);
                    thiz->mOutputPortMap[thiz->mPortCount].hComp   = cpls->hComp;
                    thiz->mOutputPortMap[thiz->mPortCount].portIdx = i;
                    thiz->mPortCount++;
                    AGILE_LOGD("To add the component %s %s port %d to the codec pipeline", 
                                compName, 
                                portDef.eDir == OMX_DirInput ? "Input" : "Output",
                                i);
                }
            }
        }else{
            AGILE_LOGE("Failed to get component name with role name %s", cpls->param.role);
            return MAG_NAME_NOT_FOUND;
        }

        compIdx++;
        next = next->next;
    }

    next = thiz->mLinkList.next;
    while (next != &thiz->mLinkList){
        cpls = (MagOmxCodecPipelineSetting *)list_entry( next, 
                                                         MagOmxCodecPipelineSetting, 
                                                         node);
        OMX_SendCommand(cpls->hComp, OMX_CommandStateSet, OMX_StateIdle, NULL);
        next = next->next;
    }

    AGILE_LOGD("Before waiting on all components state transition to IDLE!");
    Mag_WaitForEventGroup(thiz->mStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Preroll(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelineCodec thiz;
    List_t *next;
    MagOmxCodecPipelineSetting *cpls;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);

    next = thiz->mLinkList.next;
    while (next != &thiz->mLinkList){
        cpls = (MagOmxCodecPipelineSetting *)list_entry( next, 
                                                          MagOmxCodecPipelineSetting, 
                                                          node);
        OMX_SendCommand(cpls->hComp, OMX_CommandStateSet, OMX_StateIdle, NULL);
        next = next->next;
    }

    AGILE_LOGD("Before waiting on all components state transition to IDLE!");
    Mag_WaitForEventGroup(thiz->mStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelineCodec thiz;
    List_t *next;
    MagOmxCodecPipelineSetting *cpls;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);

    next = thiz->mLinkList.next;
    while (next != &thiz->mLinkList){
        cpls = (MagOmxCodecPipelineSetting *)list_entry( next, 
                                                          MagOmxCodecPipelineSetting, 
                                                          node);
        OMX_SendCommand(cpls->hComp, OMX_CommandStateSet, OMX_StateExecuting, NULL);
        next = next->next;
    }

    AGILE_LOGD("Before waiting on all components state transition to EXECUTING!");
    Mag_WaitForEventGroup(thiz->mStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Pause(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelineCodec thiz;
    List_t *next;
    MagOmxCodecPipelineSetting *cpls;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);

    next = thiz->mLinkList.next;
    while (next != &thiz->mLinkList){
        cpls = (MagOmxCodecPipelineSetting *)list_entry( next, 
                                                          MagOmxCodecPipelineSetting, 
                                                          node);
        OMX_SendCommand(cpls->hComp, OMX_CommandStateSet, OMX_StatePause, NULL);
        next = next->next;
    }

    AGILE_LOGD("Before waiting on all components state transition to PAUSE!");
    Mag_WaitForEventGroup(thiz->mStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Resume(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelineCodec thiz;
    List_t *next;
    MagOmxCodecPipelineSetting *cpls;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);

    next = thiz->mLinkList.next;
    while (next != &thiz->mLinkList){
        cpls = (MagOmxCodecPipelineSetting *)list_entry( next, 
                                                          MagOmxCodecPipelineSetting, 
                                                          node);
        OMX_SendCommand(cpls->hComp, OMX_CommandStateSet, OMX_StateExecuting, NULL);
        next = next->next;
    }

    AGILE_LOGD("Before waiting on all components state transition to EXECUTING!");
    Mag_WaitForEventGroup(thiz->mStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");

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

static OMX_ERRORTYPE virtual_MagOmxPipelineCodec_Pipeline_Map(
                    OMX_IN   OMX_HANDLETYPE hComponent, 
                    OMX_OUT  OMX_U32        mPortIdx,
                    OMX_OUT  OMX_HANDLETYPE *hCompMapped,
                    OMX_OUT  OMX_U32        *nPortIdxMapped){
    MagOmxPipelineCodec   thiz;

    thiz = ooc_cast(hComponent, MagOmxPipelineCodec);
    *hCompMapped    = thiz->mOutputPortMap[mPortIdx].hComp;
    *nPortIdxMapped = thiz->mOutputPortMap[mPortIdx].portIdx;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxPipelineCodec_EventHandlerCB(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    MagOmxCodecPipelineSetting *cpls;
    MagOmxComponent compRoot;

    cpls = (MagOmxCodecPipelineSetting *)(pAppData);
    compRoot = ooc_cast(cpls->hComp, MagOmxComponent);
    
    COMP_LOGD(compRoot, "event: %d", eEvent);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            switch ((int)Data2) {
                case OMX_StateMax:
                    COMP_LOGD(compRoot, "state: OMX_StateMax");
                    break;

                case OMX_StateLoaded:
                case OMX_StateIdle:
                case OMX_StateExecuting:
                case OMX_StatePause:
                case OMX_StateWaitForResources:
                    COMP_LOGD(compRoot, "state: %d", Data2);
                    Mag_SetEvent(cpls->stateTransitEvent); 
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            COMP_LOGD(compRoot, "component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            COMP_LOGD(compRoot, "component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            COMP_LOGD(compRoot, "component flushes port %d is done!", Data2);
        }
    }else{
        COMP_LOGD(compRoot, "unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPipelineCodec_initialize(Class this){
    AGILE_LOGV("Enter!");
    
    /*Override the base component pure virtual functions*/
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_getType      = virtual_MagOmxPipelineCodec_getType;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter = virtual_MagOmxPipelineCodec_GetParameter;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter = virtual_MagOmxPipelineCodec_SetParameter;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Prepare      = virtual_MagOmxPipelineCodec_Prepare;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Preroll      = virtual_MagOmxPipelineCodec_Preroll;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Stop         = virtual_MagOmxPipelineCodec_Stop;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Start        = virtual_MagOmxPipelineCodec_Start;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Pause        = virtual_MagOmxPipelineCodec_Pause;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Resume       = virtual_MagOmxPipelineCodec_Resume;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Flush        = virtual_MagOmxPipelineCodec_Flush;
    MagOmxPipelineCodecVtableInstance.MagOmxComponentImpl.MagOMX_Pipeline_Map = virtual_MagOmxPipelineCodec_Pipeline_Map;
}

static void MagOmxPipelineCodec_constructor(MagOmxPipelineCodec thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxPipelineCodec));
    chain_constructor(MagOmxPipelineCodec, thiz, params);
    
    thiz->compCallback_EventHandler = MagOmxPipelineCodec_EventHandlerCB;

    Mag_CreateEventGroup(&thiz->mStateTransitEvtGrp);
    thiz->mPortCount = 0;
    INIT_LIST(&thiz->mLinkList);
    thiz->mDomain    = OMX_PortDomainMax;
    thiz->mCompCount = 0;
}

static void MagOmxPipelineCodec_destructor(MagOmxPipelineCodec thiz, MagOmxPipelineCodecVtable vtab){
    AGILE_LOGV("Enter!");
}
