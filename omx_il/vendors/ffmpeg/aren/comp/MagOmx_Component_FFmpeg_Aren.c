#include "libavutil/frame.h"

#include "MagOmx_Component_FFmpeg_Aren.h"
#include "MagOmx_Port_FFmpeg_Aren.h"
#include "MagOmx_Port_FFmpeg_Clk.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.aren.ffmpeg"
#define ROLE_NAME      OMX_ROLE_AUDIO_RENDERER_PCM
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      2

/* SDL audio buffer size, in samples. Should be small to have precise
   A/V sync as SDL does not have hardware buffer fullness info. */
#define SDL_AUDIO_BUFFER_SIZE 1024

AllocateClass(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio);

/* prepare a new audio buffer */
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    MagOmxComponent_FFmpeg_Aren thiz; 
    MagOmxComponentImpl  arenCompImpl;
    OMX_BUFFERHEADERTYPE *bufHeader;
    int len1;
    AVFrame *decodedFrame;
    uint8_t *pAudioData;

    thiz = ooc_cast(opaque, MagOmxComponent_FFmpeg_Aren);
    arenCompImpl = ooc_cast(opaque, MagOmxComponentImpl);

    len1 = len;
    while(len1){
        bufHeader = thiz->getBuffer(thiz);

        decodedFrame = (AVFrame *)getBuffer->pBuffer;
        pAudioData = decodedFrame->data[0];

        if (bufHeader->nFilledLen - bufHeader->nOffset > len1){
            memcpy(stream, (uint8_t *)pAudioData + bufHeader->nOffset, len1);
            bufHeader->nOffset += len1;
            Mag_AcquireMutex(thiz->mListMutex);
            list_add(&bufHeader->Node, &thiz->mBufBusyListHead);
            Mag_ReleaseMutex(thiz->mListMutex);
            len1 = 0;
        }else{
            memcpy( stream, 
                    (uint8_t *)pAudioData + bufHeader->nOffset, 
                    bufHeader->nFilledLen - bufHeader->nOffset);
            len1 = len1 - (bufHeader->nFilledLen - bufHeader->nOffset);
            list_add_tail(&bufHeader->Node, &thiz->mBufFreeListHead);
            arenCompImpl->sendReturnBuffer(arenCompImpl, bufHeader);
        }
    }   
}

static int FFmpeg_Aren_OpenAudio(MagOmxComponent_FFmpeg_Aren thiz){
    int sample_rate;
    int nb_channels;
    i64 channel_layout;
    AVCodecContext *avctx;
    SDL_AudioSpec wanted_spec;
    SDL_AudioSpec spec;
    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};

    if (thiz->mpAudioStream){
        avctx = thiz->mpAudioStream->codec;

        sample_rate    = avctx->sample_rate;
        nb_channels    = avctx->channels;
        channel_layout = avctx->channel_layout;

        if (!channel_layout || nb_channels != av_get_channel_layout_nb_channels(channel_layout)) {
            channel_layout = av_get_default_channel_layout(nb_channels);
            channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
        }

        wanted_spec.channels = av_get_channel_layout_nb_channels(channel_layout);
        wanted_spec.freq = sample_rate;
        if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
            AGILE_LOGE("Invalid sample rate or channel count!");
            return -1;
        }
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = sdl_audio_callback;
        wanted_spec.userdata = thiz;

        while (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
            AGILE_LOGW("SDL_OpenAudio (%d channels): %s\n", wanted_spec.channels, SDL_GetError());
            wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
            if (!wanted_spec.channels) {
                AGILE_LOGE("No more channel combinations to try, audio open failed\n");
                return -1;
            }
            channel_layout = av_get_default_channel_layout(wanted_spec.channels);
        }

        if (spec.format != AUDIO_S16SYS) {
            AGILE_LOGE("SDL advised audio format %d is not supported!\n", spec.format);
            return -1;
        }

        if (spec.channels != wanted_spec.channels) {
            channel_layout = av_get_default_channel_layout(spec.channels);
            if (!channel_layout) {
                AGILE_LOGE("SDL advised channel count %d is not supported!\n", spec.channels);
                return -1;
            }
        }
    }
    

}

static OMX_BUFFERHEADERTYPE *FFmpeg_Aren_getBuffer(MagOmxComponent_FFmpeg_Aren thiz){
    List_t *next = NULL;
    MagOmx_Aren_BufferNode_t *bufHeader = NULL;
    bool empty = false;

get_again:
    Mag_AcquireMutex(thiz->mListMutex);

    next = thiz->mBufBusyListHead.next;
    if (next != &mBufBusyListHead){
        list_del(next);
        bufHeader = (MagOmx_Aren_BufferNode_t *)list_entry(next, MagOmx_Aren_BufferNode_t, Node);
        Mag_ReleaseMutex(mListMutex);
    }else{
        Mag_ClearEvent(thiz->mNewBufEvent);
        Mag_ReleaseMutex(thiz->mListMutex);

        AGILE_LOGD("Wait on getting the buffer!");
        Mag_WaitForEventGroup(thiz->mWaitBufEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("get buffer!");
        goto get_again;
    }

    if (bufHeader){
        return bufHeader->pBufHeader;
    }else{
        return NULL;
    }
}

static void FFmpeg_Aren_putBuffer(MagOmxComponent_FFmpeg_Aren thiz, OMX_BUFFERHEADERTYPE *bufHeader){
    List_t *next = NULL;
    MagOmx_Aren_BufferNode_t *bufHeader = NULL;
    bool empty = false;

    Mag_AcquireMutex(thiz->mListMutex);

    next = thiz->mBufFreeListHead.next;
    if (next != &thiz->mBufFreeListHead){
        list_del(next);
        bufHeader = (MagOmx_Aren_BufferNode_t *)list_entry(next, MagOmx_Aren_BufferNode_t, Node);
        bufHeader->pBufHeader = bufHeader;
        list_add_tail(&bufHeader->Node, &thiz->mBufBusyListHead);
    }else{
        bufHeader = (MagOmx_Aren_BufferNode_t *)mag_mallocz(sizeof(MagOmx_Aren_BufferNode_t));

        INIT_LIST(&bufHeader->Node);
        bufHeader->pBufHeader = bufHeader;
        list_add_tail(&bufHeader->Node, &thiz->mBufBusyListHead);
    }

    Mag_ReleaseMutex(thiz->mListMutex);

    Mag_SetEvent(thiz->mNewBufEvent);
}

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_FFmpeg_Clk          clkInPort;
	MagOmxPort_FFmpeg_Aren         arenInPort;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponentImpl            arenCompImpl;
	MagOmxComponent                arenComp;
	MagOmxComponent_FFmpeg_Aren    thiz;

	AGILE_LOGV("enter!");

	thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Aren);

	param.portIndex    = START_PORT_INDEX + 0;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", AREN_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Aren);
	arenInPort = ooc_new(MagOmxPort_FFmpeg_Aren, &param);
	MAG_ASSERT(arenInPort);

	param.portIndex    = START_PORT_INDEX + 1;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", CLOCK_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Clk);
	clkInPort = ooc_new(MagOmxPort_FFmpeg_Clk, &param);
	MAG_ASSERT(clkInPort);

	arenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	arenComp     = ooc_cast(hComponent, MagOmxComponent);
	
	arenComp->setName(arenComp, (OMX_U8 *)COMPONENT_NAME);
	arenCompImpl->addPort(arenCompImpl, START_PORT_INDEX + 0, arenInPort);
	arenCompImpl->addPort(arenCompImpl, START_PORT_INDEX + 1, clkInPort);

	arenCompImpl->setupPortDataFlow(arenCompImpl, arenInPort, NULL);

#ifdef CAPTURE_PCM_DATA_TO_FILE
    thiz->mfPCMFile = fopen("./audio.pcm","wb+");
    if (thiz->mfPCMFile == NULL){
        AGILE_LOGE("Failed to open the file: ./audio.pcm");
    }
#endif
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponent_FFmpeg_Aren thiz;

    AGILE_LOGV("enter!");
    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Aren);

#ifdef CAPTURE_PCM_DATA_TO_FILE
    if (!thiz->mfPCMFile){
        thiz->mfPCMFile = fopen("./audio.pcm","wb+");
        if (thiz->mfPCMFile == NULL){
            AGILE_LOGE("Failed to open the file: ./audio.pcm");
        }
    }
#endif
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponent_FFmpeg_Aren thiz;

	AGILE_LOGV("enter!");
    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Aren);

#ifdef CAPTURE_PCM_DATA_TO_FILE
    if (thiz->mfPCMFile){
        fclose(thiz->mfPCMFile);
        thiz->mfPCMFile = NULL;
    }
#endif
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	OMX_HANDLETYPE arenInPort;
	OMX_HANDLETYPE clkInPort;
	MagOmxComponentImpl arenCompImpl;

	AGILE_LOGV("Enter!");
	arenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	arenInPort  = arenCompImpl->getPort(arenCompImpl, START_PORT_INDEX + 0);
	clkInPort   = arenCompImpl->getPort(arenCompImpl, START_PORT_INDEX + 1);

	ooc_delete((Object)arenInPort);
	ooc_delete((Object)clkInPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_FFmpeg_Aren_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
	MagOmxComponentImpl arenCompImpl;
    OMX_ERRORTYPE ret;

	arenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

	AGILE_LOGV("Put decoded Audio Frame: %p, time stamp: 0x%llx, len: %d to AVSync", 
		        srcbufHeader->pBuffer, srcbufHeader->nTimeStamp, srcbufHeader->nFilledLen);

	ret = arenCompImpl->syncDisplay(arenCompImpl, srcbufHeader);
    if (ret == OMX_ErrorNotReady){
        /*directly return the buffer*/
        return OMX_ErrorNone;
    }

	return OMX_ErrorNotReady;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Aren_DoAVSync(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime){
	MagOmxComponent_FFmpeg_Aren thiz;
	MagOmxComponentImpl         arenCompImpl;
	OMX_BUFFERHEADERTYPE        *getBuffer;
	OMX_ERRORTYPE               ret;
	/*MagOmxPort                  hPort;*/
	AVFrame                     *decodedFrame;
	OMX_TICKS                   timeStamp;
	int                         data_size;
    MagOMX_AVSync_Action_t      action;

	thiz         = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Aren);
	arenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

	ret = arenCompImpl->releaseDisplay(arenCompImpl, mediaTime->nMediaTimestamp, &getBuffer);
	if (ret != OMX_ErrorNone){
		AGILE_LOGE("failed to releaseDisplay!");
		return ret;
	}

    if (getBuffer->pBuffer    == NULL && 
        getBuffer->nFilledLen == 0    && 
        getBuffer->nTimeStamp == kInvalidTimeStamp){
        AGILE_LOGV("get the EOS frame and send out the eos event to application");
        arenCompImpl->sendReturnBuffer(arenCompImpl, getBuffer);
        arenCompImpl->sendEvents(hComponent, OMX_EventBufferFlag, 0, 0, NULL);
        return OMX_ErrorNone;
    }

	decodedFrame = (AVFrame *)getBuffer->pBuffer;
    action       = (MagOMX_AVSync_Action_t)mediaTime->nClientPrivate;
	timeStamp    = getBuffer->nTimeStamp;

	data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(decodedFrame),
                                                   decodedFrame->nb_samples,
                                                   decodedFrame->format, 1);
	AGILE_LOGV("Release decoded Audio Frame: %p[size: %d], time stamp: 0x%llx to playback[%s]", 
		        decodedFrame, data_size, timeStamp, action == AVSYNC_PLAY ? "play" : "drop");

#ifdef CAPTURE_PCM_DATA_TO_FILE
    if (thiz->mfPCMFile){
    	fwrite(decodedFrame->data[0], 1, data_size, thiz->mfPCMFile);
        fflush(thiz->mfPCMFile);
    }
#endif

    getBuffer->nFilledLen = data_size;
    getBuffer->nOffset    = 0;
    thiz->putBuffer(thiz, getBuffer);
	/*arenCompImpl->sendReturnBuffer(arenCompImpl, getBuffer);*/

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Aren_GetClockActionOffset(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_TICKS *pClockOffset){
    /*in us*/
	*pClockOffset = 3000;

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Aren_GetRenderDelay(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_TICKS *pRenderDelay){
    /*in us*/
    *pRenderDelay = 3000;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_SetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure){

    MagOmxComponent_FFmpeg_Aren thiz;

    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Aren);

    switch (nIndex){
        case OMX_IndexConfigExtFFMpegData:
        {
            OMX_CONFIG_FFMPEG_DATA_TYPE *ffmpegData = (OMX_CONFIG_FFMPEG_DATA_TYPE *)pComponentParameterStructure;
            AGILE_LOGI("set parameter: avstream=%p, avformat=%p",
                        ffmpegData->avstream,
                        ffmpegData->avformat);
            thiz->mpAudioStream = (AVStream *)ffmpegData->avstream;
            thiz->mpAVFormat    = (AVFormatContext *)ffmpegData->avformat;
        }
            break;

        default:
            return OMX_ErrorUnsupportedIndex;
    }

    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_Aren_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_FFmpeg_Aren_GetComponentUUID;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Prepare           = virtual_FFmpeg_Aren_Prepare;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Preroll           = virtual_FFmpeg_Aren_Preroll;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Start             = virtual_FFmpeg_Aren_Start;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Stop              = virtual_FFmpeg_Aren_Stop;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Pause             = virtual_FFmpeg_Aren_Pause;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Resume            = virtual_FFmpeg_Aren_Resume;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Deinit            = virtual_FFmpeg_Aren_Deinit;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Reset             = virtual_FFmpeg_Aren_Reset;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_FFmpeg_Aren_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_FFmpeg_Aren_ProceedBuffer;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_DoAVSync          = virtual_FFmpeg_Aren_DoAVSync;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_GetClockActionOffset = virtual_FFmpeg_Aren_GetClockActionOffset;
    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_GetRenderDelay = virtual_FFmpeg_Aren_GetRenderDelay;

    MagOmxComponent_FFmpeg_ArenVtableInstance.MagOmxComponentAudio.MagOmx_Audio_SetParameter = virtual_FFmpeg_Aren_SetParameter;
}

static void MagOmxComponent_FFmpeg_Aren_constructor(MagOmxComponent_FFmpeg_Aren thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Aren));
    chain_constructor(MagOmxComponent_FFmpeg_Aren, thiz, params);

    thiz->getBuffer        = FFmpeg_Aren_getBuffer;
    thiz->putBuffer        = FFmpeg_Aren_putBuffer;

    thiz->mpAudioStream = NULL;
    thiz->mpAVFormat    = NULL;

    Mag_CreateMutex(&thiz->mListMutex);
    INIT_LIST(&thiz->mBufFreeListHead);
    INIT_LIST(&thiz->mBufBusyListHead);

    Mag_CreateEventGroup(&mWaitBufEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mNewBufEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mWaitBufEventGroup, mNewBufEvent);
    }
}

static void MagOmxComponent_FFmpeg_Aren_destructor(MagOmxComponent_FFmpeg_Aren thiz, MagOmxComponent_FFmpeg_ArenVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Aren_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_FFmpeg_Aren hArenComp;
	MagOmxComponentImpl     parent;
    OMX_U32 param[2];

    AGILE_LOGV("Enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Aren);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hArenComp = (MagOmxComponent_FFmpeg_Aren) ooc_new( MagOmxComponent_FFmpeg_Aren, (void *)param);
    MAG_ASSERT(hArenComp);

    parent = ooc_cast(hArenComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hArenComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hArenComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Aren_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Aren hArenComp;

	AGILE_LOGD("enter!");
	hArenComp = (MagOmxComponent_FFmpeg_Aren)compType->pComponentPrivate;

#ifdef CAPTURE_PCM_DATA_TO_FILE
	fclose(hArenComp->mfPCMFile);
#endif

	ooc_delete((Object)hArenComp);
    AGILE_LOGD("exit!");

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {ROLE_NAME, NULL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_FFmpeg_Aren_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Aren_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef ROLE_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER