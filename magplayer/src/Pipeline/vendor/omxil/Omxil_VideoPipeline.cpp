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

    Mag_CreateEventGroup(&mStExecutingEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mVDecStExecutingEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStExecutingEventGroup, mVDecStExecutingEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVSchStExecutingEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStExecutingEventGroup, mVSchStExecutingEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVRenStExecutingEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStExecutingEventGroup, mVRenStExecutingEvent);
    }

    Mag_CreateEventGroup(&mStPauseEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mVDecStPauseEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStPauseEventGroup, mVDecStPauseEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVSchStPauseEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStPauseEventGroup, mVSchStPauseEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVRenStPauseEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStPauseEventGroup, mVRenStPauseEvent);
    }

    Mag_CreateEventGroup(&mFlushDoneEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mVDecFlushDoneEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mFlushDoneEventGroup, mVDecFlushDoneEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVSchFlushDoneEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mFlushDoneEventGroup, mVSchFlushDoneEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mVRenFlushDoneEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mFlushDoneEventGroup, mVRenFlushDoneEvent);
    }

    mVideoDecCallbacks.EventHandler    = OmxilVideoPipeline::VideoDecoderEventHandler;
    mVideoDecCallbacks.EmptyBufferDone = OmxilVideoPipeline::VideoDecoderEmptyBufferDone;
    mVideoDecCallbacks.FillBufferDone  = OmxilVideoPipeline::VideoDecoderFillBufferDone;

    mVideoSchCallbacks.EventHandler    = OmxilVideoPipeline::VideoScheduleEventHandler;
    mVideoSchCallbacks.EmptyBufferDone = NULL;
    mVideoSchCallbacks.FillBufferDone  = NULL;

    mVideoRenCallbacks.EventHandler    = OmxilVideoPipeline::VideoRenderEventHandler;
    mVideoRenCallbacks.EmptyBufferDone = NULL;
    mVideoRenCallbacks.FillBufferDone  = OmxilVideoPipeline::VideoRenderFillBufferDone;

    mpBufferMgr        = NULL;
    mpDecodedBufferMgr = NULL;
    mpFeedVrenBufMgr   = NULL;
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
    
    Mag_DestroyEvent(&mVDecStExecutingEvent);
    Mag_DestroyEvent(&mVSchStExecutingEvent);
    Mag_DestroyEvent(&mVRenStExecutingEvent);
    Mag_DestroyEventGroup(&mStExecutingEventGroup);

    Mag_DestroyEvent(&mVDecStPauseEvent);
    Mag_DestroyEvent(&mVSchStPauseEvent);
    Mag_DestroyEvent(&mVRenStPauseEvent);
    Mag_DestroyEventGroup(&mStPauseEventGroup);

    Mag_DestroyEvent(&mVDecFlushDoneEvent);
    Mag_DestroyEvent(&mVSchFlushDoneEvent);
    Mag_DestroyEvent(&mVRenFlushDoneEvent);
    Mag_DestroyEventGroup(&mFlushDoneEventGroup);

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
                    Mag_SetEvent(pVpipeline->mVDecStExecutingEvent);
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    Mag_SetEvent(pVpipeline->mVDecStPauseEvent);
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
            Mag_SetEvent(pVpipeline->mVDecFlushDoneEvent);
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
    MagOmxMediaBuffer_t *mediaBuf;

    pVpipeline = static_cast<OmxilVideoPipeline *>(pAppData);
    mediaBuf = static_cast<MagOmxMediaBuffer_t *>(pBuffer->pAppPrivate);
    mediaBuf->release(mediaBuf);
    pVpipeline->mpBufferMgr->put(pBuffer);

    if (pVpipeline->getFillBufferFlag()){
        if (pVpipeline->isPlaying() && !pVpipeline->mIsFlushed){
            pVpipeline->postFillThisBuffer();
            AGILE_LOGD("retrigger the fillThisBuffer event!");
        }
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
                    Mag_SetEvent(pVpipeline->mVSchStExecutingEvent);
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    Mag_SetEvent(pVpipeline->mVSchStPauseEvent);
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
            Mag_SetEvent(pVpipeline->mVSchFlushDoneEvent);
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
                    Mag_SetEvent(pVpipeline->mVRenStExecutingEvent);
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    Mag_SetEvent(pVpipeline->mVRenStPauseEvent);
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
            Mag_SetEvent(pVpipeline->mVRenFlushDoneEvent);
        }
    }else if (eEvent == OMX_EventBufferFlag){
        /*detected the EOS*/
        AGILE_LOGI("get the EOS event and notify APP the playback completion");
        pVpipeline->notifyPlaybackComplete();
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilVideoPipeline::VideoRenderFillBufferDone(
                                            OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_PTR pAppData,
                                            OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    OmxilVideoPipeline *pVpipeline;

    pVpipeline = static_cast<OmxilVideoPipeline *>(pAppData);
    pVpipeline->mpDecodedBufferMgr->put(pBuffer);
    AGILE_LOGI("put buffer: %p", pBuffer);

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
    OMX_U32 vRenNoneTunnelPortIdx;

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
                }else{
                    vRenNoneTunnelPortIdx = i;
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
            mpBufferMgr = new OmxilBufferMgr(0, VIDEO_PORT_BUFFER_NUMBER, false);
            mpBufferMgr->create(mhVideoDecoder, vDecNoneTunnelPortIdx, static_cast<void *>(this));
        }

        if (mpDecodedBufferMgr == NULL) {
            mpDecodedBufferMgr = new OmxilBufferMgr(0, VIDEO_PORT_BUFFER_NUMBER, true);
            mpDecodedBufferMgr->create(NULL, vRenNoneTunnelPortIdx, static_cast<void *>(this));
        }

        if (mpFeedVrenBufMgr == NULL) {
            mpFeedVrenBufMgr = new OmxilBufferMgr(0, VIDEO_PORT_BUFFER_NUMBER, false);
            mpFeedVrenBufMgr->create(mhVideoRender, vRenNoneTunnelPortIdx, static_cast<void *>(this));
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

    Mag_ClearEvent(mVDecStExecutingEvent);
    Mag_ClearEvent(mVSchStExecutingEvent);
    Mag_ClearEvent(mVRenStExecutingEvent);

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    Mag_WaitForEventGroup(mStExecutingEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    AGILE_LOGI("exit!");

    ret = MagVideoPipelineImpl::start();
    return ret;
}

_status_t OmxilVideoPipeline::stop(){
    MagVideoPipelineImpl::stop();

    Mag_ClearEvent(mVDecStIdleEvent);
    Mag_ClearEvent(mVSchStIdleEvent);
    Mag_ClearEvent(mVRenStIdleEvent);

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);

    Mag_WaitForEventGroup(mStIdleEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    
    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::pause(){
    MagVideoPipelineImpl::pause();

    Mag_ClearEvent(mVDecStPauseEvent);
    Mag_ClearEvent(mVSchStPauseEvent);
    Mag_ClearEvent(mVRenStPauseEvent);

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StatePause, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StatePause, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StatePause, NULL);

    Mag_WaitForEventGroup(mStPauseEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::resume(){
    MagVideoPipelineImpl::resume();

    Mag_ClearEvent(mVDecStExecutingEvent);
    Mag_ClearEvent(mVSchStExecutingEvent);
    Mag_ClearEvent(mVRenStExecutingEvent);

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    Mag_WaitForEventGroup(mStExecutingEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::flush(){
    MagVideoPipelineImpl::flush();

    Mag_ClearEvent(mVDecFlushDoneEvent);
    Mag_ClearEvent(mVSchFlushDoneEvent);
    Mag_ClearEvent(mVRenFlushDoneEvent);

    OMX_SendCommand(mhVideoRender, OMX_CommandFlush, OMX_ALL, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandFlush, OMX_ALL, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandFlush, OMX_ALL, NULL);

    Mag_WaitForEventGroup(mFlushDoneEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
    mIsFlushed = false;
    AGILE_LOGI("exit!");

    return MAG_NO_ERROR;
}

_status_t OmxilVideoPipeline::reset(){
    MagVideoPipelineImpl::reset();

    if ( mState == ST_PLAY || mState == ST_PAUSE ){
        mState = ST_STOP;
        Mag_ClearEvent(mVDecStIdleEvent);
        Mag_ClearEvent(mVSchStIdleEvent);
        Mag_ClearEvent(mVRenStIdleEvent);

        OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
	    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateIdle, NULL);
	    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    }

    Mag_WaitForEventGroup(mStIdleEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    Mag_ClearEvent(mVDecStLoadedEvent);
    Mag_ClearEvent(mVSchStLoadedEvent);
    Mag_ClearEvent(mVRenStLoadedEvent);

    OMX_SendCommand(mhVideoRender, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_SendCommand(mhVideoScheduler, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_SendCommand(mhVideoDecoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);

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

_status_t OmxilVideoPipeline::pushEsPackets(MagOmxMediaBuffer_t *buf){
    OMX_BUFFERHEADERTYPE *pBufHeader;
    OMX_BUFFERHEADERTYPE *pFeedVrenBufHeader;
    OMX_ERRORTYPE ret;
    _status_t err = MAG_NO_ERROR;

    pBufHeader = mpBufferMgr->get();
    if (pBufHeader){
        if (!buf->eosFrame){
            pBufHeader->pAppPrivate = static_cast<OMX_PTR>(buf);
            pBufHeader->pBuffer     = static_cast<OMX_U8 *>(buf->buffer);
            pBufHeader->nAllocLen   = buf->buffer_size;
            pBufHeader->nFilledLen  = buf->buffer_size;
            pBufHeader->nOffset     = 0;
            pBufHeader->nTimeStamp  = buf->pts;
        }else{
            pBufHeader->pAppPrivate = static_cast<OMX_PTR>(buf);
            pBufHeader->pBuffer     = NULL;
            pBufHeader->nAllocLen   = 0;
            pBufHeader->nFilledLen  = 0;
            pBufHeader->nOffset     = 0;
            pBufHeader->nTimeStamp  = kInvalidTimeStamp;
        }

        pFeedVrenBufHeader = mpFeedVrenBufMgr->get();
        if (pFeedVrenBufHeader){
            ret = OMX_FillThisBuffer(mhVideoRender, pFeedVrenBufHeader);
            if (ret != OMX_ErrorNone){
                AGILE_LOGE("Do OMX_FillThisBuffer() - Failed!");
            }else{
                AGILE_LOGV("Do OMX_FillThisBuffer() - OK!");
            }
        }else{
            AGILE_LOGV("Failed to get the buffer from mpFeedVrenBufMgr!");
        }

        AGILE_LOGD("push video buffer header: %p(buf:%p, size:%d, pts:0x%x)[%s]",
                    pBufHeader, pBufHeader->pBuffer, pBufHeader->nFilledLen, pBufHeader->nTimeStamp,
                    buf->eosFrame ? "EOS" : "PKT");
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

_status_t OmxilVideoPipeline::getDecodedFrame(void **ppVideoFrame){
    OMX_BUFFERHEADERTYPE* pBuffer;
    ui64 time;

    Mag_TimeTakenStatistic(MAG_TRUE, __FUNCTION__, NULL);
    pBuffer = mpDecodedBufferMgr->get();
    if (pBuffer){
        Mag_TimeTakenStatistic(MAG_FALSE, __FUNCTION__, NULL);
        *ppVideoFrame = static_cast<void *>(pBuffer);
        return MAG_NO_ERROR;
    }
    AGILE_LOGV("Don't get decoded frame!");
    *ppVideoFrame = NULL;
    return MAG_NAME_NOT_FOUND;
}

_status_t OmxilVideoPipeline::putUsedFrame(void *pVideoFrame){
    OMX_BUFFERHEADERTYPE* pBuffer;
    AVFrame *frame;

    pBuffer = static_cast<OMX_BUFFERHEADERTYPE *>(pVideoFrame);
    AGILE_LOGV("Put used frame %p!", pBuffer);
    if (pBuffer){
        frame = (AVFrame *)pBuffer->pBuffer;
        av_frame_unref(frame);
    }
    mpFeedVrenBufMgr->put(pBuffer);

    return MAG_NO_ERROR;
}