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

AllocateClass(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio);

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

#ifdef CAPTURE_PCM_DATA_TO_FILE
	thiz->mfPCMFile = fopen("./audio.pcm","wb+");
	if (thiz->mfPCMFile == NULL){
		AGILE_LOGE("Failed to open the file: ./audio.pcm");
	}
#endif

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
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
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
	AVFrame *decodedFrame = NULL;
	OMX_TICKS timeStamp;
	MagOmxComponentImpl         arenCompImpl;

	if (hDestPort == NULL){
		return OMX_ErrorBadParameter;
	}

	if ((srcbufHeader->pBuffer != NULL) && (srcbufHeader->nFilledLen > 0)){
		arenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

		decodedFrame = (AVFrame *)srcbufHeader->pBuffer;
		timeStamp    = srcbufHeader->nTimeStamp;
		AGILE_LOGV("Put decoded Audio Frame: %p, time stamp: %lld to AVSync", decodedFrame, timeStamp);

		arenCompImpl->syncDisplay(arenCompImpl, srcbufHeader);
	}

	return OMX_ErrorNone;
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

	thiz         = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Aren);
	arenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

	ret = arenCompImpl->releaseDisplay(arenCompImpl, mediaTime->nMediaTimestamp, &getBuffer);
	if (ret != OMX_ErrorNone){
		AGILE_LOGE("failed to releaseDisplay!");
		return ret;
	}
	decodedFrame = (AVFrame *)getBuffer->pBuffer;
	timeStamp    = getBuffer->nTimeStamp;

	data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(decodedFrame),
                                                   decodedFrame->nb_samples,
                                                   decodedFrame->format, 1);
	AGILE_LOGV("Release decoded Audio Frame: %p[size: %d], time stamp: %lld to playback", 
		        decodedFrame, data_size, timeStamp);

#ifdef CAPTURE_PCM_DATA_TO_FILE
	fwrite(decodedFrame->data[0], 1, data_size, thiz->mfPCMFile);
#endif

	/*if (getBuffer->pInputPortPrivate && getBuffer->nInputPortIndex != kInvalidPortIndex){
		hPort = ooc_cast(getBuffer->pInputPortPrivate, MagOmxPort);
	}else if(getBuffer->pOutputPortPrivate && getBuffer->nOutputPortIndex != kInvalidPortIndex){
		hPort = ooc_cast(getBuffer->pOutputPortPrivate, MagOmxPort);
	}else{
		AGILE_LOGE("No port is found in buffer header: %p!", getBuffer);
		return OMX_ErrorUndefined;
	}

	MagOmxPortVirtual(hPort)->SendOutputBuffer(hPort, getBuffer);*/
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
}

static void MagOmxComponent_FFmpeg_Aren_constructor(MagOmxComponent_FFmpeg_Aren thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Aren));
    chain_constructor(MagOmxComponent_FFmpeg_Aren, thiz, params);
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

	AGILE_LOGV("Enter!");
	hArenComp = (MagOmxComponent_FFmpeg_Aren)compType->pComponentPrivate;

#ifdef CAPTURE_PCM_DATA_TO_FILE
	fclose(hArenComp->mfPCMFile);
#endif

	ooc_delete((Object)hArenComp);

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