#include "MagOMX_IL.h"
#include "Omxil_VideoPipeline.h"
#include "MagPlatform.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline_OMX"

OmxilVideoPipeline::OmxilVideoPipeline():
                            mhVideoDecoder(NULL), 
                            mhVideoScheduler(NULL),
                            mhVideoRender(NULL),
                            mpBufferMgr(NULL),
                            mVSchClockPortIdx(-1){
    OMX_ERRORTYPE err;

    err = OMX_Init();
    if(err != OMX_ErrorNone) {
        AGILE_LOGE("OMX_Init() failed");
        exit(1);
    }

    Mag_CreateEventGroup(&mStIdleEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mVDecStIdleEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStIdleEventGroup, mVDecStIdleEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVSchStIdleEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStIdleEventGroup, mVSchStIdleEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVRenStIdleEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStIdleEventGroup, mVRenStIdleEvent);
    }

    Mag_CreateEventGroup(&mStLoadedEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mVDecStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mVDecStLoadedEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVSchStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mVSchStLoadedEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVRenStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mVRenStLoadedEvent);
    }

    mVideoDecCallbacks.EventHandler    = OmxilVideoPipeline::VideoDecoderEventHandler;
    mVideoDecCallbacks.EmptyBufferDone = OmxilVideoPipeline::VideoDecoderEmptyBufferDone;
    mVideoDecCallbacks.FillBufferDone  = OmxilVideoPipeline::VideoDecoderFillBufferDone;

    mVideoSchCallbacks.EventHandler    = OmxilVideoPipeline::VideoScheduleEventHandler;
    mVideoSchCallbacks.EmptyBufferDone = NULL;
    mVideoSchCallbacks.FillBufferDone  = NULL;

    mVideoRenCallbacks.EventHandler    = OmxilVideoPipeline::VideoRenderEventHandler;
    mVideoRenCallbacks.EmptyBufferDone = NULL;
    mVideoRenCallbacks.FillBufferDone  = NULL;
}

OmxilVideoPipeline::~OmxilVideoPipeline(){
    Mag_DestroyEvent(&mVDecStIdleEvent);
    Mag_DestroyEvent(&mVSchStIdleEvent);
    Mag_DestroyEvent(&mVRenStIdleEvent);
    Mag_DestroyEventGroup(&mStIdleEventGroup);

    Mag_DestroyEvent(&mVDecStLoadedEvent);
    Mag_DestroyEvent(&mVSchStLoadedEvent);
    Mag_DestroyEvent(&mVRenStLoadedEvent);
    Mag_DestroyEventGroup(&mStLoadedEventGroup);
    
    OMX_Deinit();
}

OMX_ERRORTYPE OmxilVideoPipeline::VideoDecoderEventHandler(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    OmxilVideoPipeline *pVpipeline;

    AGILE_LOGV("Vdec event: %d", eEvent);
    pVpipeline = static_cast<OmxilVideoPipeline *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                    AGILE_LOGD("OMX_StateLoaded\n");
                    Mag_SetEvent(pVpipeline->mVDecStLoadedEvent); 
                    break;

                case OMX_StateIdle:
                    AGILE_LOGD("OMX_StateIdle\n");
                    Mag_SetEvent(pVpipeline->mVDecStIdleEvent); 
                    break;

                case OMX_StateExecuting:
                    AGILE_LOGD("OMX_StateExecuting\n");
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    break;

                case OMX_StateWaitForResources:
                    AGILE_LOGD("OMX_StateWaitForResources\n");
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Vdec component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Vdec component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Vdec component flushes port %d is done!", Data2);
        }
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilVideoPipeline::VideoDecoderEmptyBufferDone(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_BUFFERHEADERTYPE* pBuffer){
    OmxilVideoPipeline *pVpipeline;

    pVpipeline = static_cast<OmxilVideoPipeline *>(pAppData);

    pVpipeline->mpBufferMgr->put(pBuffer);

    if (pVpipeline->getFillBufferFlag()){
        if (pVpipeline->isPlaying() && !pVpipeline->mIsFlushed){
            pVpipeline->postFillThisBuffer();
            AGILE_LOGD("retrigger the fillThisBuffer event!");
        }
        pVpipeline->setFillBufferFlag(false);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilVideoPipeline::VideoDecoderFillBufferDone(
                                            OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_PTR pAppData,
                                            OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilVideoPipeline::VideoScheduleEventHandler(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    OmxilVideoPipeline *pVpipeline;

    AGILE_LOGV("Vsch event: %d", eEvent);
    pVpipeline = static_cast<OmxilVideoPipeline *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            AGILE_LOGD("OMX_CommandStateSet");
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                    AGILE_LOGD("OMX_StateLoaded\n");
                    Mag_SetEvent(pVpipeline->mVSchStLoadedEvent); 
                    break;

                case OMX_StateIdle:
                    AGILE_LOGD("OMX_StateIdle\n");
                    Mag_SetEvent(pVpipeline->mVSchStIdleEvent); 
                    break;

                case OMX_StateExecuting:
                    AGILE_LOGD("OMX_StateExecuting\n");
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    break;

                case OMX_StateWaitForResources:
                    AGILE_LOGD("OMX_StateWaitForResources\n");
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Vsch component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Vsch component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Vsch component flushes port %d is done!", Data2);
        }
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilVideoPipeline::VideoRenderEventHandler(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    OmxilVideoPipeline *pVpipeline;

    AGILE_LOGV("Vren event: %d", eEvent);
    pVpipeline = static_cast<OmxilVideoPipeline *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            AGILE_LOGD("OMX_CommandStateSet");
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                    AGILE_LOGD("OMX_StateLoaded\n");
                    Mag_SetEvent(pVpipeline->mVRenStLoadedEvent); 
                    break;

                case OMX_StateIdle:
                    AGILE_LOGD("OMX_StateIdle\n");
                    Mag_SetEvent(pVpipeline->mVRenStIdleEvent); 
                    break;

                case OMX_StateExecuting:
                    AGILE_LOGD("OMX_StateExecuting\n");
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    break;

                case OMX_StateWaitForResources:
                    AGILE_LOGD("OMX_StateWaitForResources\n");
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Vren component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Vren component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Vren component flushes port %d is done!", Data2);
        }
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

const char *OmxilVideoPipeline::getRoleByCodecId(ui32 OMXCodec){
    switch(OMXCodec){
        case OMX_VIDEO_CodingAVC:
            return OMX_ROLE_VIDEO_DECODER_AVC;

        case OMX_VIDEO_CodingMPEG4:
            return OMX_ROLE_VIDEO_DECODER_MPEG4;

        case OMX_VIDEO_CodingMPEG2:
            return OMX_ROLE_VIDEO_DECODER_EXT_MPEG2;

        case OMX_VIDEO_CodingWMV:
            return OMX_ROLE_VIDEO_DECODER_WMV;

        case OMX_VIDEO_CodingMJPEG:
            return OMX_ROLE_VIDEO_DECODER_EXT_MJPEG;

        case OMX_VIDEO_CodingH263:
            return OMX_ROLE_VIDEO_DECODER_H263;

        default:
            AGILE_LOGE("Unrecognized Video OMX Codec: 0x%x", OMXCodec);
            return NULL;
    }
}

_status_t OmxilVideoPipeline::init(i32 trackID, TrackInfo_t *sInfo){
    OMX_STRING role;
    char       compName[128];
    OMX_ERRORTYPE err;
    OMX_PORT_PARAM_TYPE vDecPortParam;
    OMX_PORT_PARAM_TYPE vSchPortParam;
    OMX_PORT_PARAM_TYPE vRenPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    OMX_CONFIG_FFMPEG_DATA_TYPE ffmpegData;
    OMX_U32 i;
    OMX_U32 vDecTunnelOutPortIdx;
    OMX_U32 vDecNoneTunnelPortIdx;
    OMX_U32 vSchTunnelInPortIdx;
    OMX_U32 vSchTunnelOutPortIdx;
    OMX_U32 vRenTunnelInPortIdx;

    AGILE_LOGD("Enter!");
    MagVideoPipelineImpl::init(trackID, sInfo);

    mTrackInfo = sInfo;

    initHeader(&portDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    role = (OMX_STRING)getRoleByCodecId(sInfo->codec);
    if (role != NULL){
        /*
         * To setup the video decoder component
         * only pick up the first one for loading
         */
        err = OMX_ComponentOfRoleEnum(compName, role, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, role);
            err = OMX_GetHandle(&mhVideoDecoder, compName, static_cast<OMX_PTR>(this), &mVideoDecCallbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return MAG_BAD_VALUE;
            }

            initHeader(&vDecPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            err = OMX_GetParameter(mhVideoDecoder, OMX_IndexParamVideoInit, &vDecPortParam);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting videoDec OMX_PORT_PARAM_TYPE parameter");
                return MAG_BAD_VALUE;
            }

            AGILE_LOGD("get vdec component param(OMX_IndexParamVideoInit): StartPortNumber-%d, Ports-%d",
                vDecPortParam.nStartPortNumber, vDecPortParam.nPorts);

            for (i = vDecPortParam.nStartPortNumber; i < vDecPortParam.nStartPortNumber + vDecPortParam.nPorts; i++){
                portDef.nPortIndex = i;
                err = OMX_GetParameter(mhVideoDecoder, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting videoDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                portDef.nBufferCountActual              = VIDEO_PORT_BUFFER_NUMBER;
                /*the buffer size is variable in terms of demuxed audio frame*/
                portDef.nBufferSize                     = 0;
                portDef.format.video.eCompressionFormat = (OMX_VIDEO_CODINGTYPE)sInfo->codec;

                err = OMX_SetParameter(mhVideoDecoder, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting videoDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                if (portDef.eDir == OMX_DirOutput){
                    AGILE_LOGD("get vDecTunnelOutPortIdx: %d", i);
                    vDecTunnelOutPortIdx = i;
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

        /*
         * To setup the video scheduler component
         */
        err = OMX_ComponentOfRoleEnum(compName, (OMX_STRING)OMX_ROLE_VIDEO_SCHEDULER_BINARY, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, OMX_ROLE_VIDEO_SCHEDULER_BINARY);
            err = OMX_GetHandle(&mhVideoScheduler, compName, static_cast<OMX_PTR>(this), &mVideoSchCallbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return MAG_BAD_VALUE;
            }

            initHeader(&vSchPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            err = OMX_GetParameter(mhVideoScheduler, OMX_IndexParamVideoInit, &vSchPortParam);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting videoSch OMX_PORT_PARAM_TYPE parameter");
                return MAG_BAD_VALUE;
            }

            AGILE_LOGD("get vsch component param(OMX_IndexParamVideoInit): StartPortNumber-%d, Ports-%d",
                vSchPortParam.nStartPortNumber, vSchPortParam.nPorts);

            for (i = vSchPortParam.nStartPortNumber; i < vSchPortParam.nStartPortNumber + vSchPortParam.nPorts; i++){
                portDef.nPortIndex = i;
                err = OMX_GetParameter(mhVideoScheduler, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting videoSch port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                if ((OMX_U32)portDef.eDomain == (OMX_U32)OMX_PortDomainVideo){
                    portDef.nBufferCountActual = VIDEO_PORT_BUFFER_NUMBER;
                    /*the buffer size is variable in terms of video decoded frame*/
                    portDef.nBufferSize        = 0; 

                    err = OMX_SetParameter(mhVideoScheduler, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in setting videoSch port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                        return MAG_BAD_VALUE;
                    }

                    if (portDef.eDir == OMX_DirOutput){
	                    vSchTunnelOutPortIdx = i;
	                }else{
	                    vSchTunnelInPortIdx = i;
	                }
                }else if ((OMX_U32)portDef.eDomain == (OMX_U32)OMX_PortDomainOther_Clock){
                    mVSchClockPortIdx = i;
                }
            }
        }else{
            AGILE_LOGE("Failed to get component name with role name %s", OMX_ROLE_VIDEO_SCHEDULER_BINARY);
            return MAG_NAME_NOT_FOUND;
        }

        /*
         * To setup the video render component
         */
        err = OMX_ComponentOfRoleEnum(compName, (OMX_STRING)OMX_ROLE_IV_RENDERER_YUV_BLTER, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, OMX_ROLE_IV_RENDERER_YUV_BLTER);
            err = OMX_GetHandle(&mhVideoRender, compName, static_cast<OMX_PTR>(this), &mVideoRenCallbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return MAG_BAD_VALUE;
            }

            initHeader(&vRenPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            err = OMX_GetParameter(mhVideoRender, OMX_IndexParamVideoInit, &vRenPortParam);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting videoRen OMX_PORT_PARAM_TYPE parameter");
                return MAG_BAD_VALUE;
            }

            AGILE_LOGD("get vren component param(OMX_IndexParamVideoInit): StartPortNumber-%d, Ports-%d",
                vRenPortParam.nStartPortNumber, vRenPortParam.nPorts);

            for (i = vRenPortParam.nStartPortNumber; i < vRenPortParam.nStartPortNumber + vRenPortParam.nPorts; i++){
                portDef.nPortIndex = i;
                err = OMX_GetParameter(mhVideoRender, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting videoRen port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                portDef.nBufferCountActual = VIDEO_PORT_BUFFER_NUMBER;
                /*the buffer size is variable in terms of video decoded frame*/
                portDef.nBufferSize        = 0; 

                err = OMX_SetParameter(mhVideoRender, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting videoDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                if (portDef.eDir == OMX_DirInput){
                    vRenTunnelInPortIdx = i;
                }
            }
        }else{
            AGILE_LOGE("Failed to get component name with role name %s", OMX_ROLE_IV_RENDERER_YUV_BLTER);
            return MAG_NAME_NOT_FOUND;
        }

        err = OMX_SetupTunnel(mhVideoDecoder, vDecTunnelOutPortIdx, mhVideoScheduler, vSchTunnelInPortIdx);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("To setup up tunnel between vDec[port: %d] and vSch[port: %d] - FAILURE!",
                        vDecTunnelOutPortIdx, vSchTunnelInPortIdx);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To setup tunnel between vDec[port: %d] and vSch[port: %d] - OK!",
                        vDecTunnelOutPortIdx, vSchTunnelInPortIdx);
        }

        err = OMX_SetupTunnel(mhVideoScheduler, vSchTunnelOutPortIdx, mhVideoRender, vRenTunnelInPortIdx);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("To setup up tunnel between vSch[port: %d] and vRen[port: %d] - FAILURE!",
                        vSchTunnelOutPortIdx, vRenTunnelInPortIdx);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To setup tunnel between vSch[port: %d] and vRen[port: %d] - OK!",
                        vSchTunnelOutPortIdx, vRenTunnelInPortIdx);
        }

        if (mpBufferMgr == NULL){
            mpBufferMgr = new OmxilBufferMgr(0, VIDEO_PORT_BUFFER_NUMBER);
            mpBufferMgr->create(mhVideoDecoder, vDecNoneTunnelPortIdx, static_cast<void *>(this));
        }
    }

    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::setup(){
    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);

    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::start(){
    _status_t ret;

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    ret = MagVideoPipelineImpl::start();
    return ret;
}

_status_t OmxilVideoPipeline::stop(){
    MagVideoPipelineImpl::stop();

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::pause(){
    MagVideoPipelineImpl::pause();

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StatePause, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StatePause, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StatePause, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::resume(){
    MagVideoPipelineImpl::resume();

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::flush(){
    MagVideoPipelineImpl::flush();

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::reset(){
    MagVideoPipelineImpl::reset();

    if ( mState == ST_PLAY || mState == ST_PAUSE ){
        mState = ST_STOP;
        OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
	    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateIdle, NULL);
	    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    }

    Mag_ClearEvent(mVDecStIdleEvent);
    Mag_ClearEvent(mVSchStIdleEvent);
    Mag_ClearEvent(mVRenStIdleEvent);
    Mag_WaitForEventGroup(mStIdleEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);

    Mag_ClearEvent(mVDecStLoadedEvent);
    Mag_ClearEvent(mVSchStLoadedEvent);
    Mag_ClearEvent(mVRenStLoadedEvent);
    Mag_WaitForEventGroup(mStLoadedEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    OMX_FreeHandle(mhVideoRender);
    mhVideoRender = NULL;

    OMX_FreeHandle(mhVideoScheduler);
    mhVideoScheduler = NULL;

    OMX_FreeHandle(mhVideoDecoder);
    mhVideoDecoder = NULL;

    return MAG_NO_ERROR;
}

void OmxilVideoPipeline::setDisplayRect(i32 x, i32 y, i32 w, i32 h){

}

_status_t OmxilVideoPipeline::pushEsPackets(MediaBuffer_t *buf){
    OMX_BUFFERHEADERTYPE *pBufHeader;
    OMX_ERRORTYPE ret;
    _status_t err = MAG_NO_ERROR;

    pBufHeader = mpBufferMgr->get();
    if (pBufHeader){
        pBufHeader->pAppPrivate = buf;
        pBufHeader->pBuffer     = static_cast<OMX_U8 *>(buf->buffer);
        pBufHeader->nAllocLen   = buf->buffer_size;
        pBufHeader->nFilledLen  = buf->buffer_size;
        pBufHeader->nOffset     = 0;
        pBufHeader->nTimeStamp  = buf->pts;

        ret = OMX_EmptyThisBuffer(mhVideoDecoder, pBufHeader);
        if (ret != OMX_ErrorNone){
            AGILE_LOGE("Do OMX_EmptyThisBuffer(buf:%p, size:%d, pts:0x%x) - Failed!",
                        pBufHeader->pBuffer, pBufHeader->nFilledLen, pBufHeader->nTimeStamp);
            err = MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGV("Do OMX_EmptyThisBuffer(buf:%p, size:%d, pts:0x%x) - Ok!",
                        pBufHeader->pBuffer, pBufHeader->nFilledLen, pBufHeader->nTimeStamp);
        }
    }else{
        AGILE_LOGE("Failed to get free buffer header!");
        err = MAG_NO_MEMORY;
    }

    return err;
}

bool OmxilVideoPipeline::needData(){
    if (mpBufferMgr){
        if (mpBufferMgr->needPushBuffers()){
            return true;
        }else{
            return false;
        }
    }else{
        AGILE_LOGV("mpBufferMgr is NULL!");
        return true;
    }
}

bool OmxilVideoPipeline::isPlaying(){
    if (mState == ST_PLAY)
        return true;
    return false;
}

_status_t OmxilVideoPipeline::getClkConnectedComp(i32 *port, void **ppComp){
    _status_t ret = MAG_NO_ERROR;

    if (mVSchClockPortIdx != -1){
        *port = mVSchClockPortIdx;
    }else{
        AGILE_LOGE("invalid video scheduler clock port index: %d!", mVSchClockPortIdx);
        ret = MAG_UNKNOWN_ERROR;
    }

    if (mhVideoScheduler != NULL){
        *ppComp = static_cast<void *>(mhVideoScheduler);
    }else{
        AGILE_LOGE("invalid video scheduler component: NULL!");
        ret = MAG_UNKNOWN_ERROR;
    }

    return ret;
}