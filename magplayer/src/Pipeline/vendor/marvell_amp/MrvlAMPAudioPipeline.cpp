#include "MrvlAMPAudioPipeline.h"
#include "MagPlatform.h"

#define ADEC_INPUT_PORT_NUM    1
#define ADEC_OUTPUT_PORT_NUM   1
#define AREN_INPUT_PORT_NUM    1
#define AREN_OUTPUT_PORT_NUM   1

#define AMP_AUDIO_ES_BUFFER_SIZE  6*1024
#define AMP_AUDIO_STREAM_NUM      16 /*16 for bg3cd-Z1/Z2*/

static char *amp_client_argv[] = {"client", "iiop:1.0//127.0.0.1:999/AMP::FACTORY/factory"};

MrvlAMPAudioPipeline::MrvlAMPAudioPipeline():
                                mhFactory(NULL),
                                mhAdec(NULL),
                                mhAren(NULL),
                                mAMPSoundTunnel(NULL),
                                mAdecEvtListener(NULL),
                                mpAMPAudioBuf(NULL),
                                mpEOSBuffer(NULL),
                                mTrackInfo(NULL){
    AMP_GetFactory(&mhFactory);
    if (mhFactory == NULL) {
        AGILE_LOGD("Initialize amp client");
        MV_OSAL_Init();
        AMP_Initialize(2, amp_client_argv, &mhFactory);
    }
#ifdef AMP_AUDIO_STREAM_DUMP
    mDumpAudioFile = NULL;
#endif
}

MrvlAMPAudioPipeline::~MrvlAMPAudioPipeline(){
    reset(); 
}

ui32 MrvlAMPAudioPipeline::getAMPAudioFormat(ui32 OMXCodec){
    switch(OMXCodec){
        case OMX_AUDIO_CodingAAC:
            AGILE_LOGD("[AMPObject::getAMPAudioFormat]: AAC");
            return MEDIA_AES_HEAAC;

        case OMX_AUDIO_CodingMP3:
            AGILE_LOGD("[AMPObject::getAMPAudioFormat]: MP3");
            return MEDIA_AES_MP3;
            
        case OMX_AUDIO_CodingMP2:
            AGILE_LOGD("[AMPObject::getAMPAudioFormat]: MP2");
            return MEDIA_AES_MPG;

        case OMX_AUDIO_CodingDDPlus:
            AGILE_LOGD("[AMPObject::getAMPAudioFormat]: DD+");
            return MEDIA_AES_DDDCV;
            
        default:
            AGILE_LOGE("[AMPObject::getAMPAudioFormat]: unsupported audio format: %d", OMXCodec);
            break;
    }
    return AMP_UNKNOW_AUD;
}

bool MrvlAMPAudioPipeline::needData(){
    if (mpAMPAudioBuf){
        if (mpAMPAudioBuf->needPushBuffers()){
            return true;
        }else{
            return false;
        }
    }else{
        AGILE_LOGV("mpAMPAudioBuf is NULL!");
        return true;
    }
}

bool MrvlAMPAudioPipeline::isPlaying(){
    if (mState == ST_PLAY)
        return true;
    return false;
}

HRESULT MrvlAMPAudioPipeline::audioPushBufferDone(CORBA_Object hCompObj, AMP_PORT_IO ePortIo,
                                                  UINT32 uiPortIdx, struct AMP_BD_ST *hBD,
                                                  AMP_IN void *pUserData){
    MrvlAMPAudioPipeline *driver = static_cast<MrvlAMPAudioPipeline *>(pUserData);
    AMPBuffer *ampbuf;

    ampbuf = driver->getEOSBuffer();
    driver->mpAMPAudioBuf->putAMPBuffer(hBD);
    
    if (ampbuf != NULL){
        if (ampbuf->getBD() == hBD){
            AGILE_LOGI("get the EOS frame back and notify APP the playback completion");
            driver->notifyPlaybackComplete();
        }
    }else{
        if (driver->getFillBufferFlag()){
            if (driver->isPlaying() && !driver->mIsFlushed){
                driver->postFillThisBuffer();
                AGILE_LOGD("retrigger the fillThisBuffer event!");
            }
            driver->setFillBufferFlag(false);
        }
    }

    return SUCCESS;
}

HRESULT MrvlAMPAudioPipeline::AdecEventHandle(HANDLE hListener, AMP_EVENT *pEvent, VOID *pUserData){
  if (!pEvent) {
    AGILE_LOGE("pEvent is NULL!");
    return !SUCCESS;
  }
  
  AMP_ADEC_STRMINFO_EVENT *pStreamInfo = reinterpret_cast<AMP_ADEC_STRMINFO_EVENT *>(AMP_EVENT_PAYLOAD_PTR(pEvent));
  
  if(AMP_EVENT_ADEC_CALLBACK_STRMINFO == pEvent->stEventHead.eEventCode) {
      // MrvlAMPAudioPipeline *pComp = static_cast<MrvlAMPAudioPipeline *>(pUserData);
      
      AGILE_LOGD("event type:%d decformat:%ld streamformat:%d samplerate:%d channelnum:%d"
               "channelmode :%d lfe mode :%d bit depth:%d\n", pEvent->stEventHead.uiParam1,
               pStreamInfo->uiAdecFmt, pStreamInfo->uiStreamFmt,
               pStreamInfo->uiSampleRate, pStreamInfo->uiChannelNum,
               pStreamInfo->uiPriChanMode, pStreamInfo->uiLfeMode,
               pStreamInfo->uiBitDepth);
      
      // pComp->mAudioInfoPQ.decoder_format = pStreamInfo->uiAdecFmt;
      // pComp->mAudioInfoPQ.stream_format = pStreamInfo->uiStreamFmt;
      // pComp->mAudioInfoPQ.sample_rate = pStreamInfo->uiSampleRate;
      // pComp->mAudioInfoPQ.channels = pStreamInfo->uiChannelNum;
      // pComp->mAudioInfoPQ.pri_channel_mode = pStreamInfo->uiPriChanMode;
      // pComp->mAudioInfoPQ.lfe_mode = pStreamInfo->uiLfeMode;
      // pComp->mAudioInfoPQ.bit_depth = pStreamInfo->uiBitDepth;
  }
  return SUCCESS;
}

_status_t MrvlAMPAudioPipeline::setup(i32 trackID, TrackInfo_t *sInfo){
    HRESULT ret = SUCCESS;
    ui32 ampAudioFormat;
    AMP_COMPONENT_CONFIG amp_config;
    AMP_AREN_PARAST ArenParaSt;

    AGILE_LOGD("Enter!");
    MagAudioPipelineImpl::setup(trackID, sInfo);

    mTrackInfo = sInfo;
    if ((NULL == mhAdec) && (NULL == mhAren)){
        // ret = AMP_SND_Init();
        ret = AMP_SND_Initialize(mhFactory, &mhAMPSound);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_SND_Initialize(). ret = %d", ret);
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("AMP do AMP_SND_Initialize() -- OK!");
        }

        AMP_RPC(ret, AMP_FACTORY_CreateComponent, mhFactory, AMP_COMPONENT_ADEC, 1, &mhAdec);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to create component: adec. ret = %d", ret);
            mhAdec = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("AMP create component ADEC -- OK!");
        }

        AMP_RPC(ret, AMP_FACTORY_CreateComponent, mhFactory, AMP_COMPONENT_AREN, 1, &mhAren);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to create component: aren. ret = %d", ret);
            mhAren = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("AMP create component AREN -- OK!");
        }

        ampAudioFormat = getAMPAudioFormat(sInfo->codec);
        if (AMP_UNKNOW_AUD == ampAudioFormat)
            return MAG_NO_INIT;

        AmpMemClear(&amp_config, sizeof(AMP_COMPONENT_CONFIG));
        amp_config._d                    = AMP_COMPONENT_ADEC;
        amp_config._u.pADEC.mode         = AMP_SECURE_TUNNEL; /*AMP_SECURE_TUNNEL, AMP_TUNNEL*/
        amp_config._u.pADEC.eAdecFmt     = ampAudioFormat;
        amp_config._u.pADEC.uiInPortNum  = ADEC_INPUT_PORT_NUM;
        amp_config._u.pADEC.uiOutPortNum = ADEC_OUTPUT_PORT_NUM;
        amp_config._u.pADEC.ucFrameIn    = AMP_ADEC_FRAME;
        AMP_RPC(ret, AMP_ADEC_Open, mhAdec, &amp_config);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_ADEC_Open(). ret = %d", ret);
            AMP_RPC(ret, AMP_ADEC_Destroy, mhAdec);
            mhAdec = NULL;
            return MAG_NO_INIT;
        }

        AmpMemClear(&amp_config, sizeof(AMP_COMPONENT_CONFIG));
        amp_config._d                       = AMP_COMPONENT_AREN;
        amp_config._u.pAREN.mode            = AMP_TUNNEL;
        amp_config._u.pAREN.uiInClkPortNum  = 1;
        amp_config._u.pAREN.uiInPcmPortNum  = AREN_INPUT_PORT_NUM;
        amp_config._u.pAREN.uiOutPcmPortNum = AREN_OUTPUT_PORT_NUM;
        AMP_RPC(ret, AMP_AREN_Open, mhAren, &amp_config);
        
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_AREN_Open(). ret = %d", ret);
            AMP_RPC(ret, AMP_AREN_Destroy, mhAren);
            mhAren = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("do AMP_AREN_Open() -- OK!");
        }

        ret = AMP_ConnectApp(mhAdec, AMP_PORT_INPUT, 0, audioPushBufferDone, static_cast<void *>(this));
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_ConnectApp() for adec. ret = %d", ret);
            return MAG_NO_INIT;
        }

        // Register a callback so that it can receive event - from ADEC.
        mAdecEvtListener = AMP_Event_CreateListener(AMP_EVENT_TYPE_MAX, 0);
        if (mAdecEvtListener) {
            ret = AMP_Event_RegisterCallback(mAdecEvtListener, AMP_EVENT_ADEC_CALLBACK_STRMINFO, 
                                             AdecEventHandle, static_cast<void *>(this));
            if (ret != SUCCESS){
                  AGILE_LOGE("register ADEC notify - Failure!");
            }else{
                AMP_RPC(ret, AMP_ADEC_RegisterNotify, mhAdec,
                        AMP_Event_GetServiceID(mAdecEvtListener), AMP_EVENT_ADEC_CALLBACK_STRMINFO);
                if (ret != SUCCESS)
                    AGILE_LOGE("register ADEC callback - Failure!");
                else
                    AGILE_LOGD("register ADEC callback - OK!");
            }
        }

        ret = AMP_ConnectComp(mhAdec, 0, mhAren, 1);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_ConnectApp() between adec and aren. ret = %d", ret);
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("AMP Connect component: Adec[%d] and Aren[%d] - OK!", 0, 1);
        }

        ret = AMP_SND_SetupTunnel(mhAren, 0, &mAMPSoundTunnel);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_SND_SetupTunnel() between sound and aren. ret = %d", ret);
            return MAG_NO_INIT;
        }

        // Associate ARen's data input port to clock port.
        memset(&ArenParaSt, 0x00, sizeof(AMP_AREN_PARAST));
        ArenParaSt._d = AMP_AREN_PARAIDX_PORTASSOCCLK;
        ArenParaSt._u.PORTASSOCCLK.uiAssocIdx = 0;
        AMP_RPC(ret, AMP_AREN_SetPortParameter, mhAren, AMP_PORT_INPUT, 1,
                AMP_AREN_PARAIDX_PORTASSOCCLK, &ArenParaSt);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_AREN_SetPortParameter, ret = 0x%x", ret);
        }
    }

    if (NULL == mpAMPAudioBuf){
        mpAMPAudioBuf = new AMPBufferMgr(&mhAdec, AMP_COMPONENT_ADEC_PIPELINE, AMP_AUDIO_ES_BUFFER_SIZE, AMP_AUDIO_STREAM_NUM);

        if (mpAMPAudioBuf->Create()  == SUCCESS){
            AGILE_LOGD("create the Audio AMP buffers -- OK!");
        }else{
            return MAG_NO_MEMORY;
        }
    }

    AMP_RPC(ret, AMP_ADEC_SetState, mhAdec, AMP_IDLE);
    AMP_RPC(ret, AMP_AREN_SetState, mhAren, AMP_IDLE);
    
    return MAG_NO_ERROR;
}

_status_t MrvlAMPAudioPipeline::start(){
    _status_t ret;
    HRESULT res = SUCCESS;

    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return ret;
    }

#ifdef AMP_AUDIO_STREAM_DUMP
    mDumpAudioFile = fopen("/lmp/ampaudio.es","wb+");
    if (NULL == mDumpAudioFile)
        AGILE_LOGE("To create the file: /lmp/ampaudio.es -- Fail");
    else
        AGILE_LOGD("To create the file: /lmp/ampaudio.es -- OK");
#endif

    mpAMPAudioBuf->waitForAllBufFree(1);
    if (NULL != mAMPSoundTunnel){
        res = AMP_SND_StartTunnel((ui32)mAMPSoundTunnel);
        if (res != SUCCESS){
            AGILE_LOGE("failed to AMP_SND_StartTunnel, ret = %d", res);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_SND_StartTunnel - OK!");
        }
    }

    AMP_RPC(res, AMP_AREN_SetState, mhAren, AMP_EXECUTING);
    if (res != SUCCESS){
        AGILE_LOGE("failed to AMP_AREN_SetState: AMP_EXECUTING. ret = %d", res);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_AREN_SetState: AMP_EXECUTING - OK!");
    }

    AMP_RPC(res, AMP_ADEC_SetState, mhAdec, AMP_EXECUTING);
    if (res != SUCCESS){
        AGILE_LOGE("failed to AMP_ADEC_SetState: AMP_EXECUTING. ret = %d", res);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_ADEC_SetState: AMP_EXECUTING - OK!");
    }

    ret = MagAudioPipelineImpl::start();
    return ret;
}

_status_t MrvlAMPAudioPipeline::stop(){
    _status_t ret;

    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return ret;
    }

#ifdef AMP_AUDIO_STREAM_DUMP
    mDumpAudioFile = fopen("/tmp/ampaudio.es","wb+");
    if (mDumpAudioFile){
        fclose(mDumpAudioFile);
        mDumpAudioFile = NULL;
    }
#endif

    MagAudioPipelineImpl::stop();
    mpEOSBuffer = NULL;

    if (NULL != mAMPSoundTunnel){
        ret = AMP_SND_StopTunnel((ui32)mAMPSoundTunnel);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_SND_StopTunnel, ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_SND_StopTunnel - OK!");
        }
    }

    if (NULL != mhAdec){
        AMP_RPC(ret, AMP_ADEC_SetState, mhAdec, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_ADEC_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_ADEC_SetState: AMP_IDLE - OK!");
        }
    }

    if (NULL != mhAren){
        AMP_RPC(ret, AMP_AREN_SetState, mhAren, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_AREN_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_AREN_SetState: AMP_IDLE - OK!");
        } 
    }
    
    return MAG_NO_ERROR;
}

_status_t MrvlAMPAudioPipeline::pause(){
    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return MAG_NO_ERROR;
    }

    MagAudioPipelineImpl::pause();
    return MAG_NO_ERROR;
}

_status_t MrvlAMPAudioPipeline::resume(){
    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return MAG_NO_ERROR;
    }

    MagAudioPipelineImpl::resume();
    return MAG_NO_ERROR;
}

_status_t MrvlAMPAudioPipeline::flush(){
    HRESULT ret = SUCCESS;

    AGILE_LOGV("enter!");

    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return ret;
    }

    MagAudioPipelineImpl::flush();

    if ((mState == ST_INIT) || (mState == ST_STOP)){
        AGILE_LOGV("do nothing while the state is in INIT/STOP");
        return MAG_NO_ERROR;
    }

    if (NULL != mhAdec){
        AMP_RPC(ret, AMP_ADEC_SetState, mhAdec, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_ADEC_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_ADEC_SetState: AMP_IDLE - OK!");
        }
    }

    if (NULL != mhAren){
        AMP_RPC(ret, AMP_AREN_SetState, mhAren, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_AREN_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_AREN_SetState: AMP_IDLE - OK!");
        } 
    }
    
    mpAMPAudioBuf->waitForAllBufFree(1);

    AMP_RPC(ret, AMP_AREN_SetState, mhAren, AMP_EXECUTING);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to AMP_AREN_SetState: AMP_EXECUTING. ret = %d", ret);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_AREN_SetState: AMP_EXECUTING - OK!");
    }

    AMP_RPC(ret, AMP_ADEC_SetState, mhAdec, AMP_EXECUTING);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to AMP_ADEC_SetState: AMP_EXECUTING. ret = %d", ret);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_ADEC_SetState: AMP_EXECUTING - OK!");
    }
    AGILE_LOGV("exit!");

    return MAG_NO_ERROR;
}

_status_t MrvlAMPAudioPipeline::reset(){
    HRESULT ret;

    AGILE_LOGD("Enter!");

    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return ret;
    }

    if ((mState == ST_PLAY) || (mState == ST_PAUSE)){
        stop();
        mpAMPAudioBuf->waitForAllBufFree(1);
    }

    MagAudioPipelineImpl::reset();

    if (NULL != mpAMPAudioBuf){
        mpAMPAudioBuf->unregisterBDs();
    }

    if (NULL != mAdecEvtListener) {
        AMP_RPC(ret, AMP_ADEC_UnregisterNotify, mhAdec,
                AMP_Event_GetServiceID(mAdecEvtListener), AMP_EVENT_ADEC_CALLBACK_STRMINFO);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_ADEC_UnregisterNotify. ret = %d", ret);
        }
  
        if (!ret) {
            ret = AMP_Event_UnregisterCallback(mAdecEvtListener, AMP_EVENT_ADEC_CALLBACK_STRMINFO, AdecEventHandle);
            if (ret != SUCCESS){
                AGILE_LOGE("failed to do ADEC AMP_Event_UnregisterCallback. ret = %d", ret);
            }
        }

        AMP_Event_DestroyListener(mAdecEvtListener);
        mAdecEvtListener = NULL;
    }

    if (NULL != mAMPSoundTunnel){
        ret = AMP_SND_RemoveTunnel(mAMPSoundTunnel);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_SND_RemoveInputTunnel. ret = %d", ret);
        }
        mAMPSoundTunnel = NULL;
    }

    if ((NULL != mhAdec) && (NULL != mhAren)){
        ret = AMP_DisconnectComp(mhAdec, 0, mhAren, 1);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_DisconnectComp (Adec to Aren). ret = %d", ret);
        }
    }

    if (NULL != mhAdec){
        ret = AMP_DisconnectApp(mhAdec, AMP_PORT_INPUT, 0, audioPushBufferDone);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_DisconnectApp (audio). ret = %d", ret);
        }
    }

    if (NULL != mhAren){
        AMP_RPC(ret, AMP_AREN_Close, mhAren);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_AREN_Close. ret = %d", ret);
        }
    }

    if (NULL != mhAdec){
        AMP_RPC(ret, AMP_ADEC_Close, mhAdec);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_ADEC_Close. ret = %d", ret);
        }
    }

    if (NULL != mhAren){
        AMP_RPC(ret, AMP_AREN_Destroy, mhAren);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_AREN_Destroy. ret = %d", ret);
        }else{
            AGILE_LOGD("do AMP_AREN_Destroy - OK!");
        }
        AMP_FACTORY_Release(mhAren);
        mhAren = NULL;
    }

    if (NULL != mhAdec){
        AMP_RPC(ret, AMP_ADEC_Destroy, mhAdec);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_ADEC_Destroy. ret = %d", ret);
        }else{
            AGILE_LOGD("do AMP_ADEC_Destroy - OK!");
        }
        AMP_FACTORY_Release(mhAdec);
        mhAdec = NULL;
    }

    if (NULL != mpAMPAudioBuf){
        delete mpAMPAudioBuf;
        mpAMPAudioBuf = NULL;
    }

    AGILE_LOGV("exit!");
    return MAG_NO_ERROR; 
}

void *MrvlAMPAudioPipeline::getClkConnectedComp(i32 *port){
    *port = 0;
    return static_cast<void *>(mhAren);
}

_status_t MrvlAMPAudioPipeline::setVolume(fp32 leftVolume, fp32 rightVolume){
    HRESULT res;
    ui32 setVolume;

    if ((NULL == mhAdec) && (NULL == mhAren)){
        AGILE_LOGE("Audio Pipeline doesn't setup, exit!");
        return MAG_NO_ERROR;
    }
    
    if (leftVolume > 1.0){
        setVolume = 100;
    }else{
        setVolume = (ui32)(leftVolume * 100.0);
    }

    res = AMP_SND_SetMasterVolume(setVolume);
    if (res != SUCCESS){
       AGILE_LOGE("failed to do AMP_SND_SetMasterVolume(%d)", setVolume);
       return MAG_INVALID_OPERATION;
    }else{
       AGILE_LOGD("AMP_SND_SetMasterVolume(%d) -- OK!", setVolume);
    }
    return MAG_NO_ERROR;
}

_status_t MrvlAMPAudioPipeline::pushEsPackets(MediaBuffer_t *buf){
    ui8  *pEsPacket   = static_cast<ui8 *>(buf->buffer);
    ui32 sizeEsPacket = buf->buffer_size;
    i64 iPts90k       = buf->pts;
    AMPBufInter_t *pGetBufDesc = NULL;
    AMPBuffer *ampCodecBuf;
    ui8 *aBuf;
    ui32 offset;
    ui32 free_size;
    bool formatted = false;

    if ((mState != ST_PLAY) && (mState != ST_PAUSE)){
        AGILE_LOGD("in non-playing state, exit!");
        return MAG_NO_ERROR;
    }

    AGILE_LOGD("packet size: %d, pts = 0x%llx", sizeEsPacket, iPts90k);

    while (sizeEsPacket > 0){
        if ((pGetBufDesc = mpAMPAudioBuf->getAMPBuffer()) == NULL){
            AGILE_LOGE("no Audio AMP buffer available! [should not be here]");
            return MAG_UNKNOWN_ERROR;
        }

        ampCodecBuf = reinterpret_cast<AMPBuffer *>(pGetBufDesc->pAMPBuf);
        aBuf = (ui8 *)ampCodecBuf->getBufferStatus(&offset, &free_size);

        // AGILE_LOGD("get aBuf=0x%p, offset=%d, free_size=%d", aBuf, offset, free_size);
        if (!formatted){
            ui8 header[32];
            ui32 header_size;
            MagESFormat *formatter = static_cast<MagESFormat *>(buf->esFormatter);

            if (formatter){
                header_size = formatter->formatES(header, pEsPacket, sizeEsPacket);
                if ((header_size > 0) && (header_size <= free_size)){
                    memcpy(aBuf, header, header_size);
                #ifdef AMP_AUDIO_STREAM_DUMP
                    if (mDumpAudioFile)
                        fwrite(aBuf, 1, header_size, mDumpAudioFile);
                #endif
                    aBuf += header_size;
                    ampCodecBuf->updateDataSize(header_size);
                }else{
                    AGILE_LOGE("failed to format the audio es data!");
                }
            }
            formatted = true;
        }

        if (sizeEsPacket > free_size){
            if (sizeEsPacket > 0){
                if (sizeEsPacket <= free_size){
                    ampCodecBuf->updateUnitStartPtsTag(iPts90k, -1);   
                    ampCodecBuf->updateDataSize(sizeEsPacket);
                    memcpy(aBuf + offset, pEsPacket, sizeEsPacket);
                #ifdef AMP_AUDIO_STREAM_DUMP
                    if (mDumpAudioFile)
                        fwrite(aBuf + offset, 1, sizeEsPacket, mDumpAudioFile);
                #endif
                    if (ampCodecBuf->updateMemInfo(&mhAdec, 1) != 1){
                        AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[1] - NOT pushing BD!");
                        // return MAG_NO_MEMORY;
                    }
                    
                    sizeEsPacket = 0;
                }else{
                    ampCodecBuf->updateUnitStartPtsTag(iPts90k, -1);
                    ampCodecBuf->updateDataSize(free_size);
                    memcpy(aBuf + offset, pEsPacket, free_size);
                #ifdef AMP_AUDIO_STREAM_DUMP
                    if (mDumpAudioFile)
                        fwrite(aBuf + offset, 1, free_size, mDumpAudioFile);
                #endif
                    if (ampCodecBuf->updateMemInfo(&mhAdec, 1) != 1){
                        AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[2] - NOT pushing BD!");
                        // return MAG_NO_MEMORY;
                    }
                    
                    pEsPacket    += free_size;
                    sizeEsPacket -= free_size;
                }
            }
        }else{
            ampCodecBuf->updateUnitStartPtsTag(iPts90k, -1);
            if (sizeEsPacket > 0){
                ampCodecBuf->updateDataSize(sizeEsPacket);
                memcpy(aBuf + offset, pEsPacket, sizeEsPacket);
            #ifdef AMP_AUDIO_STREAM_DUMP
                if (mDumpAudioFile)
                    fwrite(aBuf + offset, 1, sizeEsPacket, mDumpAudioFile);
            #endif
            }

            if (ampCodecBuf->updateMemInfo(&mhAdec, 1) != 1){
                AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[3] - NOT pushing BD!");
            }

            sizeEsPacket = 0;
        }
    }

    if (buf->eosFrame){
        AGILE_LOGV("proceed EOS frame!");
        if ((pGetBufDesc = mpAMPAudioBuf->getAMPBuffer()) == NULL){
            AGILE_LOGE("no Audio AMP buffer available! [should not be here]");
            return MAG_UNKNOWN_ERROR;
        }
        ampCodecBuf = reinterpret_cast<AMPBuffer *>(pGetBufDesc->pAMPBuf);
        ampCodecBuf->updateMemInfo(&mhAdec, 1, true);
        mpEOSBuffer = ampCodecBuf;
    }

    AGILE_LOGV("Exit!");
    return MAG_NO_ERROR;
}

AMPBuffer *MrvlAMPAudioPipeline::getEOSBuffer(){
    return mpEOSBuffer;
}