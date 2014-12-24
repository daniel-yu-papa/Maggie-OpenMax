#include "MagOMX_IL.h"
#include "Omxil_AudioPipeline.h"
#include "MagPlatform.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline_OMX"

OmxilAudioPipeline::OmxilAudioPipeline():
                            mhAudioDecoder(NULL),
                            mhAudioRender(NULL),
                            mpBufferMgr(NULL),
                            mARenClockPortIdx(-1){
    OMX_ERRORTYPE err;

    err = OMX_Init();
    if(err != OMX_ErrorNone) {
        AGILE_LOGE("OMX_Init() failed");
        exit(1);
    }

    Mag_CreateEventGroup(&mStIdleEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mADecStIdleEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStIdleEventGroup, mADecStIdleEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mARenStIdleEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStIdleEventGroup, mARenStIdleEvent);
    }

    Mag_CreateEventGroup(&mStLoadedEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mADecStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mADecStLoadedEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mARenStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mARenStLoadedEvent);
    }

    Mag_CreateEventGroup(&mStLoadedEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mADecStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mADecStLoadedEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mARenStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mARenStLoadedEvent);
    }

    Mag_CreateEventGroup(&mStExecutingEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mADecStExecutingEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStExecutingEventGroup, mADecStExecutingEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mARenStExecutingEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStExecutingEventGroup, mARenStExecutingEvent);
    }

    Mag_CreateEventGroup(&mStPauseEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mADecStPauseEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStPauseEventGroup, mADecStPauseEvent);
    }
    if (MAG_ErrNone == Mag_CreateEvent(&mARenStPauseEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStPauseEventGroup, mARenStPauseEvent);
    }

    mAudioDecCallbacks.EventHandler    = OmxilAudioPipeline::AudioDecoderEventHandler;
    mAudioDecCallbacks.EmptyBufferDone = OmxilAudioPipeline::AudioDecoderEmptyBufferDone;
    mAudioDecCallbacks.FillBufferDone  = OmxilAudioPipeline::AudioDecoderFillBufferDone;

    mAudioRenCallbacks.EventHandler    = OmxilAudioPipeline::AudioRenderEventHandler;
    mAudioRenCallbacks.EmptyBufferDone = NULL;
    mAudioRenCallbacks.FillBufferDone  = NULL;
}

OmxilAudioPipeline::~OmxilAudioPipeline(){
    Mag_DestroyEvent(&mADecStIdleEvent);
    Mag_DestroyEvent(&mARenStIdleEvent);
    Mag_DestroyEventGroup(&mStIdleEventGroup);

    Mag_DestroyEvent(&mADecStLoadedEvent);
    Mag_DestroyEvent(&mARenStLoadedEvent);
    Mag_DestroyEventGroup(&mStLoadedEventGroup);
    
    OMX_Deinit();
}

OMX_ERRORTYPE OmxilAudioPipeline::AudioDecoderEventHandler(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    OmxilAudioPipeline *pApipeline;

    AGILE_LOGV("Adec event: %d", eEvent);
    pApipeline = static_cast<OmxilAudioPipeline *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            AGILE_LOGD("OMX_CommandStateSet");
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                    AGILE_LOGD("OMX_StateLoaded\n");
                    Mag_SetEvent(pApipeline->mADecStLoadedEvent); 
                    break;

                case OMX_StateIdle:
                    AGILE_LOGD("OMX_StateIdle\n");
                    Mag_SetEvent(pApipeline->mADecStIdleEvent); 
                    break;

                case OMX_StateExecuting:
                    AGILE_LOGD("OMX_StateExecuting\n");
                    Mag_SetEvent(pApipeline->mADecStExecutingEvent); 
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    Mag_SetEvent(pApipeline->mADecStPauseEvent); 
                    break;

                case OMX_StateWaitForResources:
                    AGILE_LOGD("OMX_StateWaitForResources\n");
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Adec component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Adec component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Adec component flushes port %d is done!", Data2);
        }
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilAudioPipeline::AudioDecoderEmptyBufferDone(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_BUFFERHEADERTYPE* pBuffer){
    OmxilAudioPipeline *pApipeline;
    MagOmxMediaBuffer_t *mediaBuf;

    pApipeline = static_cast<OmxilAudioPipeline *>(pAppData);

    mediaBuf = static_cast<MagOmxMediaBuffer_t *>(pBuffer->pAppPrivate);
    mediaBuf->release(mediaBuf);
    pApipeline->mpBufferMgr->put(pBuffer);

    if (pApipeline->getFillBufferFlag()){
        if (pApipeline->isPlaying() && !pApipeline->mIsFlushed){
            pApipeline->postFillThisBuffer();
            AGILE_LOGD("retrigger the fillThisBuffer event!");
        }
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilAudioPipeline::AudioDecoderFillBufferDone(
                                            OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_PTR pAppData,
                                            OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OmxilAudioPipeline::AudioRenderEventHandler(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    OmxilAudioPipeline *pApipeline;

    AGILE_LOGV("Adec event: %d", eEvent);
    pApipeline = static_cast<OmxilAudioPipeline *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            AGILE_LOGD("OMX_CommandStateSet");
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                    AGILE_LOGD("OMX_StateLoaded\n");
                    Mag_SetEvent(pApipeline->mARenStLoadedEvent); 
                    break;

                case OMX_StateIdle:
                    AGILE_LOGD("OMX_StateIdle\n");
                    Mag_SetEvent(pApipeline->mARenStIdleEvent); 
                    break;

                case OMX_StateExecuting:
                    AGILE_LOGD("OMX_StateExecuting\n");
                    Mag_SetEvent(pApipeline->mARenStExecutingEvent);
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    Mag_SetEvent(pApipeline->mARenStPauseEvent);
                    break;

                case OMX_StateWaitForResources:
                    AGILE_LOGD("OMX_StateWaitForResources\n");
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Aren component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Aren component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Aren component flushes port %d is done!", Data2);
        }
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

const char *OmxilAudioPipeline::getRoleByCodecId(ui32 OMXCodec){
    switch(OMXCodec){
        case OMX_AUDIO_CodingAAC:
            return OMX_ROLE_AUDIO_DECODER_AAC;

        case OMX_AUDIO_CodingMP3:
            return OMX_ROLE_AUDIO_DECODER_MP3;

        case OMX_AUDIO_CodingMP2:
            return OMX_ROLE_AUDIO_DECODER_EXT_MP2;

        case OMX_AUDIO_CodingAC3:
            return OMX_ROLE_AUDIO_DECODER_EXT_AC3;

        case OMX_AUDIO_CodingDDPlus:
            return OMX_ROLE_AUDIO_DECODER_EXT_DDPLUS;

        case OMX_AUDIO_CodingDTS:
            return OMX_ROLE_AUDIO_DECODER_EXT_DTS;

        default:
            AGILE_LOGE("Unrecognized Audio OMX Codec: 0x%x", OMXCodec);
            return NULL;
    }
}

_status_t OmxilAudioPipeline::init(i32 trackID, TrackInfo_t *sInfo){
    OMX_STRING role;
    char       compName[128];
    OMX_ERRORTYPE err;
    OMX_PORT_PARAM_TYPE aDecPortParam;
    OMX_PORT_PARAM_TYPE aRenPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    OMX_CONFIG_FFMPEG_DATA_TYPE ffmpegData;
    OMX_U32 i;
    OMX_U32 aDecTunnelPortIdx;
    OMX_U32 aDecNoneTunnelPortIdx;
    OMX_U32 aRenTunnelPortIdx;

    AGILE_LOGD("Enter!");
    MagAudioPipelineImpl::init(trackID, sInfo);

    mTrackInfo = sInfo;

    initHeader(&portDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    role = (OMX_STRING)getRoleByCodecId(sInfo->codec);
    if (role != NULL){
        /*
         * To setup the audio decoder component
         * only pick up the first one for loading
         */
        err = OMX_ComponentOfRoleEnum(compName, role, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, role);
            err = OMX_GetHandle(&mhAudioDecoder, compName, static_cast<OMX_PTR>(this), &mAudioDecCallbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return MAG_BAD_VALUE;
            }

            initHeader(&aDecPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            err = OMX_GetParameter(mhAudioDecoder, OMX_IndexParamAudioInit, &aDecPortParam);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting audioDec OMX_PORT_PARAM_TYPE parameter");
                return MAG_BAD_VALUE;
            }

            AGILE_LOGD("get adec component param(OMX_IndexParamAudioInit): StartPortNumber-%d, Ports-%d",
                aDecPortParam.nStartPortNumber, aDecPortParam.nPorts);

            for (i = aDecPortParam.nStartPortNumber; i < aDecPortParam.nStartPortNumber + aDecPortParam.nPorts; i++){
                portDef.nPortIndex = i;
                err = OMX_GetParameter(mhAudioDecoder, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting audioDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                portDef.nBufferCountActual     = AUIDO_PORT_BUFFER_NUMBER;
                /*the buffer size is variable in terms of demuxed audio frame*/
                portDef.nBufferSize            = 0; 
                portDef.format.audio.eEncoding = (OMX_AUDIO_CODINGTYPE)sInfo->codec;

                err = OMX_SetParameter(mhAudioDecoder, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in setting audioDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                if (portDef.eDir == OMX_DirOutput){
                    aDecTunnelPortIdx = i;
                }else{
                    aDecNoneTunnelPortIdx = i;
                }
            }

            initHeader(&ffmpegData, sizeof(OMX_CONFIG_FFMPEG_DATA_TYPE));
            ffmpegData.avformat = sInfo->avformat;
            ffmpegData.avstream = sInfo->avstream;
            err = OMX_SetParameter(mhAudioDecoder, (OMX_INDEXTYPE)OMX_IndexConfigExtFFMpegData, &ffmpegData);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in setting audioDec component OMX_CONFIG_FFMPEG_DATA_TYPE parameter");
                return MAG_BAD_VALUE;
            }
        }else{
            AGILE_LOGE("Failed to get component name with role name %s", role);
            return MAG_NAME_NOT_FOUND;
        }

        /*
         * To setup the audio render component
         */
        err = OMX_ComponentOfRoleEnum(compName, (OMX_STRING)OMX_ROLE_AUDIO_RENDERER_PCM, 1);
        if (err == OMX_ErrorNone){
            AGILE_LOGV("get the component name[%s] that has the role[%s]",
                        compName, OMX_ROLE_AUDIO_RENDERER_PCM);
            err = OMX_GetHandle(&mhAudioRender, compName, static_cast<OMX_PTR>(this), &mAudioRenCallbacks);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
                return MAG_BAD_VALUE;
            }

            initHeader(&aRenPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            err = OMX_GetParameter(mhAudioRender, OMX_IndexParamAudioInit, &aRenPortParam);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting audioDec OMX_PORT_PARAM_TYPE parameter");
                return MAG_BAD_VALUE;
            }

            AGILE_LOGD("get aren component param(OMX_IndexParamAudioInit): StartPortNumber-%d, Ports-%d",
                aRenPortParam.nStartPortNumber, aRenPortParam.nPorts);

            for (i = aRenPortParam.nStartPortNumber; i < aRenPortParam.nStartPortNumber + aRenPortParam.nPorts; i++){
                portDef.nPortIndex = i;
                err = OMX_GetParameter(mhAudioRender, OMX_IndexParamPortDefinition, &portDef);
                if(err != OMX_ErrorNone){
                    AGILE_LOGE("Error in getting audioDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                    return MAG_BAD_VALUE;
                }

                if (portDef.eDomain == OMX_PortDomainAudio){
                    portDef.nBufferCountActual = AUIDO_PORT_BUFFER_NUMBER;
                    /*the buffer size is variable in terms of demuxed audio frame*/
                    portDef.nBufferSize        = 0; 

                    err = OMX_SetParameter(mhAudioRender, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in setting audioDec port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                        return MAG_BAD_VALUE;
                    }

                    aRenTunnelPortIdx = i;
                }else{
                    mARenClockPortIdx = i;
                }
            }
        }else{
            AGILE_LOGE("Failed to get component name with role name %s", OMX_ROLE_AUDIO_RENDERER_PCM);
            return MAG_NAME_NOT_FOUND;
        }

        err = OMX_SetupTunnel(mhAudioDecoder, aDecTunnelPortIdx, mhAudioRender, aRenTunnelPortIdx);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("To setup up tunnel between aDec[port: %d] and aRen[port: %d] - FAILURE!",
                        aDecTunnelPortIdx, aRenTunnelPortIdx);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To setup tunnel between aDec[port: %d] and aRen[port: %d] - OK!",
                        aDecTunnelPortIdx, aRenTunnelPortIdx);
        }

        if (mpBufferMgr == NULL){
            mpBufferMgr = new OmxilBufferMgr(0, AUIDO_PORT_BUFFER_NUMBER);
            mpBufferMgr->create(mhAudioDecoder, aDecNoneTunnelPortIdx, NULL);
        }
    }

    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::setup(){
    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);

    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::start(){
    _status_t ret;

    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    Mag_ClearEvent(mADecStExecutingEvent);
    Mag_ClearEvent(mARenStExecutingEvent);
    Mag_WaitForEventGroup(mStExecutingEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    ret = MagAudioPipelineImpl::start();
    return ret;
}

_status_t OmxilAudioPipeline::stop(){
    MagAudioPipelineImpl::stop();

    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::pause(){
    MagAudioPipelineImpl::pause();

    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StatePause, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StatePause, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::resume(){
    MagAudioPipelineImpl::resume();

    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::flush(){
    MagAudioPipelineImpl::flush();

    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::reset(){
    MagAudioPipelineImpl::reset();

    if ( mState == ST_PLAY || mState == ST_PAUSE ){
        mState = ST_STOP;
        OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);  
    }

    Mag_ClearEvent(mADecStIdleEvent);
    Mag_ClearEvent(mARenStIdleEvent);
    Mag_WaitForEventGroup(mStIdleEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    OMX_SendCommand(mhAudioRender, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    OMX_SendCommand(mhAudioDecoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);

    Mag_ClearEvent(mADecStLoadedEvent);
    Mag_ClearEvent(mARenStLoadedEvent);
    Mag_WaitForEventGroup(mStLoadedEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);

    OMX_FreeHandle(mhAudioRender);
    mhAudioRender = NULL;

    OMX_FreeHandle(mhAudioDecoder);
    mhAudioDecoder = NULL;

    return MAG_NO_ERROR;
}

_status_t OmxilAudioPipeline::setVolume(fp32 leftVolume, fp32 rightVolume){
    return OMX_ErrorNone;
}

_status_t OmxilAudioPipeline::pushEsPackets(MagOmxMediaBuffer_t *buf){
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

        AGILE_LOGD("push audio buffer header: %p(buf:%p, size:%d, pts:0x%x)",
                    pBufHeader, pBufHeader->pBuffer, pBufHeader->nFilledLen, pBufHeader->nTimeStamp);

        ret = OMX_EmptyThisBuffer(mhAudioDecoder, pBufHeader);
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

bool OmxilAudioPipeline::needData(){
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

bool OmxilAudioPipeline::isPlaying(){
    if (mState == ST_PLAY)
        return true;
    return false;
}

_status_t OmxilAudioPipeline::getClkConnectedComp(i32 *port, void **ppComp){
    _status_t ret = MAG_NO_ERROR;

    if (mARenClockPortIdx != -1){
        *port = mARenClockPortIdx;
    }else{
        AGILE_LOGE("invalid audio renderer clock port index: %d!", mARenClockPortIdx);
        ret = MAG_UNKNOWN_ERROR;
    }

    if (mhAudioRender != NULL){
        *ppComp = static_cast<void *>(mhAudioRender);
    }else{
        AGILE_LOGE("invalid audio renderer component: NULL!");
        ret = MAG_UNKNOWN_ERROR;
    }

    return ret;
}