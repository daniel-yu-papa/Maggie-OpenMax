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

#include "MagOmx_Pipeline_playback.h"

#define DFAULT_BUFFER_SIZE (1 * 1024 * 1024)
#define DFAULT_BUFFER_TIME (30000)
#define DFAULT_BUFFER_LOW_THRESHOLD  (20)
#define DFAULT_BUFFER_HIGH_THRESHOLD (99)

#define URI_DATA_SOURCE_BUFFER_NUMBER  16
#define URI_DATA_SOURCE_BUFFER_SIZE    (2 * 1024)

#define DEMXER_AUDIO_OUTPUT_PORT_BUFFER_NUM    32
#define DEMXER_VIDEO_OUTPUT_PORT_BUFFER_NUM    16
#define DEMXER_SUBTITLE_OUTPUT_PORT_BUFFER_NUM 32

AllocateClass(MagOmxPipelinePlayback, MagOmxComponentImpl);

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_GetParameter(
                                    OMX_IN  OMX_HANDLETYPE hComponent, 
                                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                                    OMX_INOUT OMX_PTR pComponentParameterStructure){
    return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_SetParameter(
                                    OMX_IN  OMX_HANDLETYPE hComponent, 
                                    OMX_IN  OMX_INDEXTYPE nIndex,
                                    OMX_IN  OMX_PTR pComponentParameterStructure){
    MagOmxPipelinePlayback  thiz;
    MagOmxComponent         root;
    OMX_ERRORTYPE           ret = OMX_ErrorNone;

    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);
    root = ooc_cast(hComponent, MagOmxComponent);

    switch (nIndex){
        case OMX_IndexParamPlaybackPipelineSetting:
        {
            OMX_PLAYBACK_PIPELINE_SETTING *setting;
            PlaybackPipelineDatasource *ds;

            setting = (OMX_PLAYBACK_PIPELINE_SETTING *)pComponentParameterStructure;

            ds = (PlaybackPipelineDatasource *)mag_mallocz(sizeof(PlaybackPipelineDatasource));
            ds->demuxerInbufportidx = kInvalidCompPortNumber;
            INIT_LIST(&ds->node);
            INIT_LIST(&ds->codecPipelineList);

            Mag_CreateEventGroup(&ds->cplStateTransitEvtGrp);
            Mag_CreateEventGroup(&ds->stateTransitEvtGrp);
            if (MAG_ErrNone == Mag_CreateEvent(&ds->demuxerCompStateEvent, MAG_EVT_PRIO_DEFAULT)){
                Mag_AddEventGroup(ds->stateTransitEvtGrp, ds->demuxerCompStateEvent);
            }

            if (MAG_ErrNone == Mag_CreateEvent(&ds->bufCompStateEvent, MAG_EVT_PRIO_DEFAULT)){
                Mag_AddEventGroup(ds->stateTransitEvtGrp, ds->bufCompStateEvent);
            }

            if (MAG_ErrNone == Mag_CreateEvent(&ds->dsCompStateEvent, MAG_EVT_PRIO_DEFAULT)){
                Mag_AddEventGroup(ds->stateTransitEvtGrp, ds->dsCompStateEvent);
            }

            ds->url = mag_strdup(setting->url);
            if (setting->buffer_type == OMX_BUFFER_TYPE_NONE){
                ds->buffer_type = OMX_BUFFER_TYPE_BYTE; /*default*/
            }else{
                ds->buffer_type = setting->buffer_type;
            }

            if (setting->buffer_size == 0){
                ds->buffer_size = DFAULT_BUFFER_SIZE; /*default*/
            }else{
                ds->buffer_size = setting->buffer_size;
            }

            if (setting->buffer_time == 0){
                ds->buffer_time = DFAULT_BUFFER_TIME; /*default*/
            }else{
                ds->buffer_time = setting->buffer_time;
            }
            
            if (setting->buffer_low_threshold == 0){
                ds->buffer_low_threshold = DFAULT_BUFFER_LOW_THRESHOLD; /*default*/
            }else{
                ds->buffer_low_threshold = setting->buffer_low_threshold;
            }

            if (setting->buffer_high_threshold == 0){
                ds->buffer_high_threshold = DFAULT_BUFFER_HIGH_THRESHOLD; /*default*/
            }else{
                ds->buffer_high_threshold = setting->buffer_high_threshold;
            }

            ds->free_run = setting->free_run;
            list_add_tail(&ds->node, &thiz->mDataSourceList);
        }
            break;

        default:
            break;
    }

    return ret;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Prepare(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelinePlayback       thiz;
    MagOmxComponent              root;
    MagOmxComponentImpl          base;

    OMX_ERRORTYPE                err = OMX_ErrorNone;
    List_t                       *next;
    PlaybackPipelineDatasource   *dsDesc;
    char                         compName[128];
    OMX_U32                      i;

    OMX_DEMUXER_SETTING          demuxerSetting;
    OMX_BUFFER_PARAM             bufferSetting;
    OMX_PORT_PARAM_TYPE          portParam;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    OMX_BOOL                     isLastComp = OMX_FALSE;

    OMX_U32                      bufComp_outportidx;
    OMX_U32                      bufComp_inportidx;
    OMX_U32                      dsComp_outportidx;

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);
    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);

    next = thiz->mDataSourceList.next;
    while (next != &thiz->mDataSourceList){
        dsDesc = (PlaybackPipelineDatasource *)list_entry( next, 
                                                           PlaybackPipelineDatasource, 
                                                           node);

        err = OMX_ComponentOfRoleEnum(compName, OMX_ROLE_CONTAINER_DEMUXER_EXT_AUTO, 1);
        if (err == OMX_ErrorNone){
            err = OMX_GetHandle(&dsDesc->hDemuxerComponent, compName, (OMX_PTR)dsDesc, &thiz->mDemuxerCallbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) is failure", compName);
                return err;
            }

            initHeader(&demuxerSetting, sizeof(OMX_DEMUXER_SETTING));
            demuxerSetting.eBufferType = dsDesc->buffer_type;
            demuxerSetting.cStreamUrl  = dsDesc->url;
            err = OMX_SetParameter(dsDesc->hDemuxerComponent, OMX_IndexParamExtDemuxerSetting, &demuxerSetting);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in setting %s OMX_IndexParamExtDemuxerSetting parameter", compName);
                return err;
            }

            if (dsDesc->buffer_type == OMX_BUFFER_TYPE_BYTE){
                if (dsDesc->demuxerInbufportidx == kInvalidCompPortNumber){
                    AGILE_LOGE("Failed to get demuxer[%s] input port for byte buffer component connection", compName);
                    return OMX_ErrorBadPortIndex;
                }
                
                /*create the byte buffer component*/
                err = OMX_ComponentOfRoleEnum(compName, OMX_ROLE_BUFFER, 1);
                if(err != OMX_ErrorNone) {
                    AGILE_LOGE("OMX_ComponentOfRoleEnum(role: %s) failed", OMX_ROLE_BUFFER);
                    return err;
                }

                err = OMX_GetHandle(&dsDesc->hBufferComponent, compName, (OMX_PTR)dsDesc, &thiz->mBufferCallbacks);
                if(err != OMX_ErrorNone) {
                    AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                    return err;
                }

                initHeader(&bufferSetting, sizeof(OMX_BUFFER_PARAM));
                memset(&bufferSetting, 0, sizeof(OMX_BUFFER_PARAM));
                bufferSetting.uHighPercent       = dsDesc->buffer_high_threshold;
                bufferSetting.uLowPercent        = dsDesc->buffer_low_threshold;
                bufferSetting.uRingBufferMaxSize = dsDesc->buffer_size;
                bufferSetting.mode               = OMX_BUFFER_MODE_PULL;

                err = OMX_SetParameter(dsDesc->hBufferComponent, OMX_IndexParamExtBufferSetting, &bufferSetting);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting %s OMX_IndexParamExtBufferSetting parameter", compName);
                    return OMX_ErrorBadParameter;
                }

                initHeader(&portParam, sizeof(OMX_PORT_PARAM_TYPE));
                err = OMX_GetParameter(dsDesc->hBufferComponent, OMX_IndexParamOtherInit, &portParam);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting the parameter of %s buffer component port", compName);
                    return OMX_ErrorBadParameter;
                }

                for (i = portParam.nStartPortNumber; i < portParam.nStartPortNumber + portParam.nPorts; i++){
                    portDef.nPortIndex = i;
                    err = OMX_GetParameter(dsDesc->hBufferComponent, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Failed to get %s port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", compName, i);
                        return OMX_ErrorBadParameter;
                    }

                    if (portDef.eDir == OMX_DirOutput){
                        portDef.nBufferCountActual = 0;
                        portDef.nBufferSize        = 0;
                        bufComp_outportidx = i;
                    }else{
                        portDef.nBufferCountActual = URI_DATA_SOURCE_BUFFER_NUMBER;
                        portDef.nBufferSize        = URI_DATA_SOURCE_BUFFER_SIZE;
                        bufComp_inportidx = i;
                    }

                    err = OMX_SetParameter(dsDesc->hBufferComponent, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in setting comp[%s] port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", 
                                    compName, i);
                        return err;
                    }
                }

                /*create the data source component*/
                err = OMX_ComponentOfRoleEnum(compName, OMX_ROLE_DATA_SOURCE_AUTO, 1);
                if(err != OMX_ErrorNone) {
                    AGILE_LOGE("OMX_ComponentOfRoleEnum(role: %s) failed", OMX_ROLE_DATA_SOURCE_AUTO);
                    return err;
                }

                err = OMX_GetHandle(&dsDesc->hDataSrcComponent, compName, (OMX_PTR)dsDesc, &thiz->mDataSrcCallbacks);
                if(err != OMX_ErrorNone) {
                    AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                    return err;
                }

                initHeader(&portParam, sizeof(OMX_PORT_PARAM_TYPE));
                err = OMX_GetParameter(dsDesc->hDataSrcComponent, OMX_IndexParamOtherInit, &portParam);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting the parameter of %s data source component port", compName);
                    return OMX_ErrorBadParameter;
                }

                for (i = portParam.nStartPortNumber; i < portParam.nStartPortNumber + portParam.nPorts; i++){
                    portDef.nPortIndex = i;
                    err = OMX_GetParameter(dsDesc->hDataSrcComponent, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Failed to get %s port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", compName, i);
                        return OMX_ErrorBadParameter;
                    }

                    if (portDef.eDir == OMX_DirOutput){
                        portDef.nBufferCountActual = URI_DATA_SOURCE_BUFFER_NUMBER;
                        portDef.nBufferSize        = URI_DATA_SOURCE_BUFFER_SIZE;
                        dsComp_outportidx = i;
                    }

                    err = OMX_SetParameter(dsDesc->hDataSrcComponent, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in setting comp[%s] port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", 
                                    compName, i);
                        return err;
                    }
                }

                err = OMX_SetupTunnel(dsDesc->hBufferComponent, bufComp_outportidx, dsDesc->hDemuxerComponent, dsDesc->demuxerInbufportidx);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("To setup up tunnel between buffer comp[port: %d] and demuxer comp[port: %d] - FAILURE!",
                                bufComp_outportidx, dsDesc->demuxerInbufportidx);
                    return err;
                }else{
                    AGILE_LOGI("To setup up tunnel between buffer comp[port: %d] and demuxer comp[port: %d] - OK!",
                                bufComp_outportidx, dsDesc->demuxerInbufportidx);
                }

                err = OMX_SetupTunnel(dsDesc->hDataSrcComponent, dsComp_outportidx, dsDesc->hBufferComponent, bufComp_inportidx);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("To setup up tunnel between datasource comp[port: %d] and buffer comp[port: %d] - FAILURE!",
                                dsComp_outportidx, bufComp_inportidx);
                    return err;
                }else{
                    AGILE_LOGI("To setup up tunnel between datasource comp[port: %d] and buffer comp[port: %d] - OK!",
                                dsComp_outportidx, bufComp_inportidx);
                }                

                OMX_SendCommand(dsDesc->hDataSrcComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
                OMX_SendCommand(dsDesc->hBufferComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
                OMX_SendCommand(dsDesc->hDemuxerComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);

                AGILE_LOGD("Before waiting on all components state transition to IDLE!");
                Mag_WaitForEventGroup(dsDesc->stateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
                AGILE_LOGD("After the state transition to IDLE!");

                OMX_SendCommand(dsDesc->hDataSrcComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
                OMX_SendCommand(dsDesc->hBufferComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
                OMX_SendCommand(dsDesc->hDemuxerComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);

                AGILE_LOGD("Before waiting on all components state transition to EXECUTING!");
                Mag_WaitForEventGroup(dsDesc->stateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
                AGILE_LOGD("After the state transition to EXECUTING!");
            }
        }else{
            AGILE_LOGE("OMX_ComponentOfRoleEnum(role: %s) failed", OMX_ROLE_CONTAINER_DEMUXER_EXT_AUTO);
            return err;
        }

        AGILE_LOGD("Before waiting on the state transition of all connected codec pipelines to IDLE!");
        Mag_WaitForEventGroup(thiz->mCodecPlAllReadyEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("All connected codec pipelines transit to IDLE!");

        next = next->next;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Preroll(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelinePlayback thiz;
    List_t *next;
    List_t *pl_next;
    PlaybackPipelineDatasource *ds;
    CodecPipelineEntry *pl;  

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);

    if (thiz->mhClockComponent){
        OMX_SendCommand(thiz->mhClockComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
        Mag_WaitForEventGroup(thiz->mClockStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    }

    next = thiz->mDataSourceList.next;
    while (next != &thiz->mDataSourceList){
        ds = (PlaybackPipelineDatasource *)list_entry( next, 
                                                       PlaybackPipelineDatasource, 
                                                       node);
        pl_next =  ds->codecPipelineList.next;
        while (pl_next != &ds->codecPipelineList){
            pl = (CodecPipelineEntry *)list_entry( next, 
                                                   CodecPipelineEntry, 
                                                   node);
            if (pl->pipeline){
                OMX_SendCommand(pl->pipeline, OMX_CommandStateSet, OMX_StateIdle, NULL);
            }

            pl_next = pl_next->next;
        }

        AGILE_LOGD("Before waiting on all pipelines state transition to IDLE!");
        Mag_WaitForEventGroup(ds->cplStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("All pipelines state transit to IDLE!");

        OMX_SendCommand(ds->hDemuxerComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_SendCommand(ds->hBufferComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_SendCommand(ds->hDataSrcComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);

        Mag_WaitForEventGroup(ds->stateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("The playback pipeline state transit to IDLE!");

        next = next->next;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelinePlayback thiz;
    List_t *next;
    List_t *pl_next;
    PlaybackPipelineDatasource *ds;
    CodecPipelineEntry *pl;
    OMX_DEMUXER_KICKOFF kickoffdemuxer;
    OMX_S32 strm_id[64];
    OMX_U32 total_strms = 0;

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);

    if (thiz->mhClockComponent){
        OMX_TIME_CONFIG_CLOCKSTATETYPE clockSt;

        OMX_SendCommand(thiz->mhClockComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
        initHeader(&clockSt, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
        clockSt.eState = OMX_TIME_ClockStateWaitingForStartTime;
        clockSt.nOffset = 0;
        OMX_SetConfig(thiz->mhClockComponent, OMX_IndexConfigTimeClockState, &clockSt);
    }
    

    next = thiz->mDataSourceList.next;
    while (next != &thiz->mDataSourceList){
        ds = (PlaybackPipelineDatasource *)list_entry( next, 
                                                       PlaybackPipelineDatasource, 
                                                       node);
        pl_next =  ds->codecPipelineList.next;
        while (pl_next != &ds->codecPipelineList){
            pl = (CodecPipelineEntry *)list_entry( next, 
                                                   CodecPipelineEntry, 
                                                   node);
            if (pl->pipeline){
                strm_id[total_strms] = pl->portDesc->stream_info->stream_id;
                OMX_SendCommand(pl->pipeline, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            }else{
                strm_id[total_strms] = -1;
            }
            total_strms++;

            pl_next = pl_next->next;
        }

        AGILE_LOGD("Before waiting on all pipelines state transition to EXECUTING!");
        Mag_WaitForEventGroup(ds->cplStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("All pipelines state transition to EXECUTING!");

        initHeader(&kickoffdemuxer, sizeof(OMX_DEMUXER_KICKOFF));
        kickoffdemuxer.url        = ds->url;
        kickoffdemuxer.pnStreamId = strm_id;
        kickoffdemuxer.nStreamNum = total_strms;
        err = OMX_SetParameter(ds->hDemuxerComponent, OMX_IndexConfigExtStartDemuxing, &kickoffdemuxer);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("Error in setting demuxer comp OMX_IndexConfigExtStartDemuxing parameter");
            return OMX_ErrorBadParameter;
        }

        next = next->next;
    }

    return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Pause(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelinePlayback thiz;
    List_t *next;
    List_t *pl_next;
    PlaybackPipelineDatasource *ds;
    CodecPipelineEntry *pl;

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);

    if (thiz->mhClockComponent){
        OMX_SendCommand(thiz->mhClockComponent, OMX_CommandStateSet, OMX_StatePause, NULL);
        Mag_WaitForEventGroup(thiz->mClockStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    }

    next = thiz->mDataSourceList.next;
    while (next != &thiz->mDataSourceList){
        ds = (PlaybackPipelineDatasource *)list_entry( next, 
                                                       PlaybackPipelineDatasource, 
                                                       node);
        pl_next =  ds->codecPipelineList.next;
        while (pl_next != &ds->codecPipelineList){
            pl = (CodecPipelineEntry *)list_entry( next, 
                                                   CodecPipelineEntry, 
                                                   node);
            if (pl->pipeline){
                OMX_SendCommand(pl->pipeline, OMX_CommandStateSet, OMX_StatePause, NULL);
            }

            pl_next = pl_next->next;
        }

        AGILE_LOGD("Before waiting on all pipelines state transition to EXECUTING!");
        Mag_WaitForEventGroup(ds->cplStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("All pipelines state transition to EXECUTING!");

        next = next->next;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Resume(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPipelinePlayback thiz;
    List_t *next;
    List_t *pl_next;
    PlaybackPipelineDatasource *ds;
    CodecPipelineEntry *pl;

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);

    if (thiz->mhClockComponent){
        OMX_SendCommand(thiz->mhClockComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
        Mag_WaitForEventGroup(thiz->mClockStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    }

    next = thiz->mDataSourceList.next;
    while (next != &thiz->mDataSourceList){
        ds = (PlaybackPipelineDatasource *)list_entry( next, 
                                                       PlaybackPipelineDatasource, 
                                                       node);
        pl_next =  ds->codecPipelineList.next;
        while (pl_next != &ds->codecPipelineList){
            pl = (CodecPipelineEntry *)list_entry( next, 
                                                   CodecPipelineEntry, 
                                                   node);
            if (pl->pipeline){
                OMX_SendCommand(pl->pipeline, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            }

            pl_next = pl_next->next;
        }

        AGILE_LOGD("Before waiting on all pipelines state transition to EXECUTING!");
        Mag_WaitForEventGroup(ds->cplStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("All pipelines state transition to EXECUTING!");

        next = next->next;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Flush(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_ProceedBuffer(
                                        OMX_IN  OMX_HANDLETYPE hComponent, 
                                        OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                                        OMX_IN  OMX_HANDLETYPE hDestPort){
   
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPipelinePlayback_Pipeline_Map(
                    OMX_IN   OMX_HANDLETYPE hComponent, 
                    OMX_OUT  OMX_U32        mPortIdx,
                    OMX_OUT  OMX_HANDLETYPE *hCompMapped,
                    OMX_OUT  OMX_U32        *nPortIdxMapped){
    MagOmxPipelinePlayback   thiz;

    thiz = ooc_cast(hComponent, MagOmxPipelinePlayback);
    *hCompMapped    = thiz->mOutputPortMap[mPortIdx].hComp;
    *nPortIdxMapped = thiz->mOutputPortMap[mPortIdx].portIdx;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxPipelinePlayback_Demuxer_EventHandlerCB(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    PlaybackPipelineDatasource *dsDesc;
    MagOmxComponent compRoot;
    MagOmxPipelinePlayback thiz;
    MagOmxComponentImpl    base;
    OMX_ERRORTYPE ret;

    dsDesc = (PlaybackPipelineDatasource *)(pAppData);
    compRoot = ooc_cast(dsDesc->hComp, MagOmxComponent);
    thiz     = ooc_cast(hComponent, MagOmxPipelinePlayback);
    base     = ooc_cast(hComponent, MagOmxComponentImpl);

    COMP_LOGD(compRoot, "event: %d", eEvent);

    initHeader(&portDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));    
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
                    Mag_SetEvent(dsDesc->demuxerCompStateEvent); 
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            COMP_LOGD(compRoot, "component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            COMP_LOGD(compRoot, "component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            COMP_LOGD(compRoot, "component flushes port %d is done!", Data2);
        }
    }else if (eEvent == OMX_EventDynamicPortAdding){
        OMX_DYNAMIC_PORT_TYPE portType;
        MagOmxPipelineCodec   codecPipeline = NULL;
        OMX_PARAM_PORTDEFINITIONTYPE portDef;
        OMX_PORT_PARAM_TYPE portParam;
        CodecPipelineEntry *ple;
        OMX_U32 i;
        OMX_U32 buf_count;
        MAG_DEMUXER_OUTPUTPORT_DESC *codecPort;

        portType = (OMX_DYNAMIC_PORT_TYPE)Data2;
        if (portType == OMX_DynamicPort_Buffer){
            dsDesc->demuxerInbufportidx = Data1;
        }else if (portType == OMX_DynamicPort_Video ||
                  portType == OMX_DynamicPort_Audio ||
                  portType == OMX_DynamicPort_Subtitle){
            /*create the codec pipeline entry*/
            ple = (CodecPipelineEntry *)mag_mallocz(sizeof(CodecPipelineEntry));
            INIT_LIST(&ple->node);
            ple->pipelineInPortIdx   = kInvalidCompPortNumber;
            ple->pipelineOutPortIdx  = kInvalidCompPortNumber;
            ple->pipelineClkPortIdx  = kInvalidCompPortNumber;
            ple->clockCompOutPortIdx = kInvalidCompPortNumber;

            /*retrieve the stream info and codec port description*/
            ple->portDesc = (MAG_DEMUXER_OUTPUTPORT_DESC *)pEventData;

            /*set the demuxer output port*/
            portDef.nPortIndex = Data1;
            err = OMX_GetParameter(dsDesc->hDemuxerComponent, OMX_IndexParamPortDefinition, &portDef);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Failed to get demuxer comp port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", portDef.nPortIndex);
                return err;
            }
            if (portType == OMX_DynamicPort_Video)
                buf_count = DEMXER_VIDEO_OUTPUT_PORT_BUFFER_NUM;
            else if (portType == OMX_DynamicPort_Audio){
                buf_count = DEMXER_AUDIO_OUTPUT_PORT_BUFFER_NUM;
            }else if (portType == OMX_DynamicPort_Subtitle){
                buf_count = DEMXER_SUBTITLE_OUTPUT_PORT_BUFFER_NUM;
            }
            portDef.nBufferCountActual = buf_count;
            portDef.nBufferSize = 0;

            err = OMX_SetParameter(dsDesc->hDemuxerComponent, OMX_IndexParamPortDefinition, &portDef);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in setting demuxer comp port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", 
                            portDef.nPortIndex);
                return err;
            }

            /*create the codec pipeline in terms of the stream type*/
            if (MagOmxPipelinePlaybackVirtual(thiz)->MagOMX_Playback_CreateCodecPipeline){
                /*it leaves the vendor playback pipeline to decide how many codec pipelines could be created. 
                 * for example: more than 1 audio codec pipelines are created for mixture audio effects if the hardware supports it.
                 */
                ret = MagOmxPipelinePlaybackVirtual(thiz)->MagOMX_Playback_CreateCodecPipeline(hComponent,
                                                                                               portType,
                                                                                               ple,
                                                                                               &thiz->mCodecPipelineCallbacks,
                                                                                               &codecPipeline);

                if (ret == OMX_ErrorNone){
                    ple->pipeline = codecPipeline;
                    ple->demuxerOutPortIdx = Data1;
                }else{
                    ple->pipeline = NULL;
                    ple->demuxerOutPortIdx = Data1;
                    COMP_LOGD(compRoot, "failed to create the codec pipeline[type: %d]", portType);
                }

                /*Set the pipeline ports and connect to demuxer component*/
                if (ple->pipeline){
                    if (MAG_ErrNone == Mag_CreateEvent(&ple->pipelineStateEvent, MAG_EVT_PRIO_DEFAULT)){
                        Mag_AddEventGroup(dsDesc->cplStateTransitEvtGrp, ple->pipelineStateEvent);
                    }

                    initHeader(&portParam, sizeof(OMX_PORT_PARAM_TYPE));
                    err = OMX_GetParameter(ple->pipeline, OMX_IndexParamOtherInit, &portParam);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in getting codec pipeline OMX_PORT_PARAM_TYPE parameter");
                        return err;
                    }

                    for (i = portParam.nStartPortNumber; i < portParam.nStartPortNumber + portParam.nPorts; i++){
                        portDef.nPortIndex = i;
                        err = OMX_GetParameter(ple->pipeline, OMX_IndexParamPortDefinition, &portDef);
                        if(err != OMX_ErrorNone){
                            AGILE_LOGE("Failed to get codec pipeline port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                            return err;
                        }

                        if (portDef.eDomain == OMX_PortDomainOther_Clock){
                            ple->pipelineClkPortIdx = i;
                        }else{
                            if (portDef.eDir == OMX_DirInput){
                                portDef.nBufferCountActual = buf_count;
                                portDef.nBufferSize        = 0;
                                ple->pipelineInPortIdx     = i;
                            }else{
                                ple->pipelineOutPortIdx    = i;
                            }
                        }

                        err = OMX_SetParameter(ple->pipeline, OMX_IndexParamPortDefinition, &portDef);
                        if(err != OMX_ErrorNone){
                            AGILE_LOGE("Failed to get codec pipeline port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                            return err;
                        }
                    }

                    err = OMX_SetupTunnel(dsDesc->hDemuxerComponent, ple->demuxerOutPortIdx, ple->pipeline, ple->pipelineInPortIdx);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("To setup up tunnel between demuxer comp[port: %d] and %s pipeline[port: %d] - FAILURE!",
                                    ple->demuxerOutPortIdx,
                                    portType == OMX_DynamicPort_Video ? "video" : portType == OMX_DynamicPort_Audio ? "audio" : "subtitle",
                                    ple->pipelineInPortIdx);
                        return err;
                    }else{
                        AGILE_LOGE("To setup up tunnel between demuxer comp[port: %d] and %s pipeline[port: %d] - OK!",
                                    ple->demuxerOutPortIdx,
                                    portType == OMX_DynamicPort_Video ? "video" : portType == OMX_DynamicPort_Audio ? "audio" : "subtitle",
                                    ple->pipelineInPortIdx);
                    }
                }
                

                COMP_LOGD(compRoot, "To add the codec pipeline[type: %d, port index: %d, hpl: %p] to the list", 
                                     portType, ple->demuxerOutPortIdx, ple->pipeline);

                list_add_tail(&ple->node, &dsDesc->codecPipelineList);
            }else{
                COMP_LOGE(getRoot(hComponent), "pure virtual func: MagOMX_Playback_CreateCodecPipeline() is not overrided!!");
                ret = OMX_ErrorNotImplemented;
            }

            /*send out the stream info to APP*/
            base->sendEvents( thiz,
                              OMX_EventAVStreamInfo,
                              portType,
                              0,
                              (OMX_PTR)(ple->portDesc->stream_info) );
        }else{
            List_t *next;
            CodecPipelineEntry *e;
            char compName[128];
            OMX_CONFIG_UI32TYPE newPortConfig;

            COMP_LOGD(compRoot, "All codec pipelines are created.");

            if (!dsDesc->free_run){
                err = OMX_ComponentOfRoleEnum(compName, (OMX_STRING)OMX_ROLE_CLOCK_BINARY, 1);
                if (err == OMX_ErrorNone){
                    AGILE_LOGV("get the component name[%s] that has the role[%s]",
                                compName, OMX_ROLE_CLOCK_BINARY);
                    err = OMX_GetHandle(&thiz->mhClockComponent, compName, (OMX_PTR)(thiz), &thiz->mClockCallbacks);
                    if(err != OMX_ErrorNone) {
                        AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                        return err;
                    }
                }else{
                    AGILE_LOGV("Failed to get the component name with the role[%s]",
                                OMX_ROLE_CLOCK_BINARY);
                    return err;
                }

                initHeader(&newPortConfig, sizeof(OMX_CONFIG_UI32TYPE));
            }

            next = dsDesc->codecPipelineList.next;
            while (next != &dsDesc->codecPipelineList){
                e = (CodecPipelineEntry *)list_entry( next, 
                                                      CodecPipelineEntry, 
                                                      node);
                if (e->pipeline){
                    if (!dsDesc->free_run){
                        ret = OMX_GetConfig(thiz->mhClockComponent, 
                                            (OMX_INDEXTYPE)OMX_IndexConfigExtAddPort, 
                                            &newPortConfig);
                        if (ret != OMX_ErrorNone){
                            AGILE_LOGE("Failed to add the port dynamically. ret = 0x%x", ret);
                            return ret;
                        }
                        e->clockCompOutPortIdx = newPortConfig.uValue;

                        err = OMX_SetupTunnel(thiz->mhClockComponent, 
                                              e->clockCompOutPortIdx, 
                                              e->pipeline, 
                                              e->pipelineClkPortIdx);
                        if(err != OMX_ErrorNone){
                            AGILE_LOGE("To setup up tunnel between clock comp[port: %d] and %s pipeline[port: %d] - FAILURE!",
                                        e->clockCompOutPortIdx,
                                        e->portDesc->portType == OMX_DynamicPort_Video ? "video" : e->portDesc->portType == OMX_DynamicPort_Audio ? "audio" : "subtitle",
                                        e->pipelineClkPortIdx);
                            return err;
                        }else{
                            AGILE_LOGE("To setup up tunnel between demuxer comp[port: %d] and %s pipeline[port: %d] - OK!",
                                        e->clockCompOutPortIdx,
                                        e->portDesc->portType == OMX_DynamicPort_Video ? "video" : e->portDesc->portType == OMX_DynamicPort_Audio ? "audio" : "subtitle",
                                        e->pipelineClkPortIdx);
                        } 
                    }

                    /*add the output port exported from codec pipeline into the playback pipeline*/
                    if (e->pipelineOutPortIdx != kInvalidCompPortNumber){
                        MagOmxComponentImpl pipelineImpl;
                        OMX_HANDLETYPE outputPort;

                        pipelineImpl = ooc_cast(e->pipeline, MagOmxComponentImpl);
                        outputPort = pipelineImpl->getPort(pipelineImpl, e->pipelineOutPortIdx);
                        base->addPort(base, thiz->mPortNumber++, outputPort);
                    }

                    OMX_SendCommand(e->pipeline, OMX_CommandStateSet, OMX_StateIdle, NULL);
                }
            }

            AGILE_LOGD("Before waiting on codec pipelines state transition to IDLE!");
            Mag_WaitForEventGroup(dsDesc->cplStateTransitEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
            AGILE_LOGD("codec pipelines state transition to IDLE!");

            if (thiz->mhClockComponent){
                OMX_SendCommand(thiz->mhClockComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
                Mag_WaitForEventGroup(thiz->mClockStateEvtGrp, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
                AGILE_LOGD("clock state transition to IDLE!");
            }

            Mag_SetEvent(thiz->mCodecPlAllReadyEvent); 
        }
    }else{
        COMP_LOGD(compRoot, "unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxPipelinePlayback_Buf_EventHandlerCB(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    PlaybackPipelineDatasource *dsDesc;
    MagOmxComponent compRoot;

    dsDesc = (PlaybackPipelineDatasource *)(pAppData);
    compRoot = ooc_cast(dsDesc->hComp, MagOmxComponent);
    
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
                    Mag_SetEvent(dsDesc->bufCompStateEvent); 
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            COMP_LOGD(compRoot, "component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            COMP_LOGD(compRoot, "component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            COMP_LOGD(compRoot, "component flushes port %d is done!", Data2);
        }
    }else if (eEvent == OMX_EventBufferFlag){
        COMP_LOGD(compRoot, "buffer level %d%%!", Data1);
    }else{
        COMP_LOGD(compRoot, "unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxPipelinePlayback_DS_EventHandlerCB(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    PlaybackPipelineDatasource *dsDesc;
    MagOmxComponent compRoot;

    dsDesc = (PlaybackPipelineDatasource *)(pAppData);
    compRoot = ooc_cast(dsDesc->hComp, MagOmxComponent);
    
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
                    Mag_SetEvent(dsDesc->dsCompStateEvent); 
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

static OMX_ERRORTYPE MagOmxPipelinePlayback_CPL_EventHandlerCB(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    CodecPipelineEntry *cple;
    MagOmxComponent compRoot;

    cple = (CodecPipelineEntry *)(pAppData);
    compRoot = ooc_cast(dsDesc->hComp, MagOmxComponent);
    
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
                    Mag_SetEvent(cple->pipelineStateEvent); 
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

static OMX_ERRORTYPE MagOmxPipelinePlayback_Clk_EventHandlerCB(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    MagOmxPipelinePlayback thiz;
    MagOmxComponent compRoot;

    thiz     = ooc_cast(hComponent, MagOmxPipelinePlayback);
    compRoot = ooc_cast(hComponent, MagOmxComponent);
    
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
                    Mag_SetEvent(thiz->mClockStateEvent); 
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

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPipelinePlayback_initialize(Class this){
    AGILE_LOGV("Enter!");
    
    /*Override the base component pure virtual functions*/
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter      = virtual_MagOmxPipelinePlayback_GetParameter;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter      = virtual_MagOmxPipelinePlayback_SetParameter;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Prepare           = virtual_MagOmxPipelinePlayback_Prepare;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Preroll           = virtual_MagOmxPipelinePlayback_Preroll;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Stop              = virtual_MagOmxPipelinePlayback_Stop;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Start             = virtual_MagOmxPipelinePlayback_Start;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Pause             = virtual_MagOmxPipelinePlayback_Pause;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Resume            = virtual_MagOmxPipelinePlayback_Resume;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Flush             = virtual_MagOmxPipelinePlayback_Flush;
    MagOmxPipelinePlaybackVtableInstance.MagOmxComponentImpl.MagOMX_Pipeline_Map      = virtual_MagOmxPipelinePlayback_Pipeline_Map;
}

static void MagOmxPipelinePlayback_constructor(MagOmxPipelinePlayback thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxPipelinePlayback));
    chain_constructor(MagOmxPipelinePlayback, thiz, params);
    
    thiz->demuxerCb_EventHandler       = MagOmxPipelinePlayback_Demuxer_EventHandlerCB;
    thiz->bufferCb_EventHandler        = MagOmxPipelinePlayback_Buf_EventHandlerCB;
    thiz->datasourceCb_EventHandler    = MagOmxPipelinePlayback_DS_EventHandlerCB;
    thiz->codecPipelineCb_EventHandler = MagOmxPipelinePlayback_CPL_EventHandlerCB;
    thiz->clockCb_EventHandler         = MagOmxPipelinePlayback_Clk_EventHandlerCB;

    thiz->mDemuxerCallbacks.EventHandler    = thiz->demuxerCb_EventHandler;
    thiz->mDemuxerCallbacks.EmptyBufferDone = NULL;
    thiz->mDemuxerCallbacks.FillBufferDone  = NULL;

    thiz->mBufferCallbacks.EventHandler     = thiz->bufferCb_EventHandler;
    thiz->mBufferCallbacks.EmptyBufferDone  = NULL;
    thiz->mBufferCallbacks.FillBufferDone   = NULL;

    thiz->mDataSrcCallbacks.EventHandler    = thiz->datasourceCb_EventHandler;
    thiz->mDataSrcCallbacks.EmptyBufferDone = NULL;
    thiz->mDataSrcCallbacks.FillBufferDone  = NULL;

    thiz->mCodecPipelineCallbacks.EventHandler    = thiz->codecPipelineCb_EventHandler;
    thiz->mCodecPipelineCallbacks.EmptyBufferDone = NULL;
    thiz->mCodecPipelineCallbacks.FillBufferDone  = NULL;

    thiz->mClockCallbacks.EventHandler    = thiz->clockCb_EventHandler;
    thiz->mClockCallbacks.EmptyBufferDone = NULL;
    thiz->mClockCallbacks.FillBufferDone  = NULL;

    Mag_CreateEventGroup(&thiz->mClockStateEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mClockStateEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(thiz->mClockStateEvtGrp, thiz->mClockStateEvent);
    }

    Mag_CreateEventGroup(&thiz->mCodecPlAllReadyEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mCodecPlAllReadyEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(thiz->mCodecPlAllReadyEvtGrp, thiz->mCodecPlAllReadyEvent);
    }

    INIT_LIST(&thiz->mDataSourceList);
    thiz->mhClockComponent = NULL;
    thiz->mPortNumber      = kCompPortStartNumber;
}

static void MagOmxPipelinePlayback_destructor(MagOmxPipelinePlayback thiz, MagOmxPipelinePlaybackVtable vtab){
    AGILE_LOGV("Enter!");
}