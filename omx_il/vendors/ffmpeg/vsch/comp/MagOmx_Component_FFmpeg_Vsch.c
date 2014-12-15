#include "libavutil/frame.h"

#include "MagOmx_Component_FFmpeg_Vsch.h"
#include "MagOmx_Port_FFmpeg_Vdec.h"
#include "MagOmx_Port_FFmpeg_Vren.h"
#include "MagOmx_Port_FFmpeg_Vsch.h"
#include "MagOmx_Port_FFmpeg_Clk.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.vsch.ffmpeg"
#define ROLE_NAME      OMX_ROLE_VIDEO_SCHEDULER_BINARY
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      3

AllocateClass(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo);

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_FFmpeg_Clk          clkInPort;
	MagOmxPort_FFmpeg_Vsch         vschInPort;
	MagOmxPort_FFmpeg_Vren         vrenOutPort;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponentImpl            vschCompImpl;
	MagOmxComponent                vschComp;
	MagOmxComponent_FFmpeg_Vsch    thiz;

	AGILE_LOGV("enter!");

	thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vsch);

#ifdef CAPTURE_YUV_DATA_TO_FILE
	thiz->mfYUVFile = fopen("./video.yuv","wb+");
	if (thiz->mfYUVFile == NULL){
		AGILE_LOGE("Failed to open the file: ./video.yuv");
	}
#endif

	param.portIndex    = START_PORT_INDEX + 0;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", VSCH_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Vsch);
	vschInPort = ooc_new(MagOmxPort_FFmpeg_Vsch, &param);
	MAG_ASSERT(vschInPort);

	param.portIndex    = START_PORT_INDEX + 1;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", CLOCK_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Clk);
	clkInPort = ooc_new(MagOmxPort_FFmpeg_Clk, &param);
	MAG_ASSERT(clkInPort);

	param.portIndex    = START_PORT_INDEX + 2;
	param.isInput      = OMX_FALSE;
	param.bufSupplier  = OMX_BufferSupplyOutput;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-Out", VREN_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Vren);
	vrenOutPort = ooc_new(MagOmxPort_FFmpeg_Vren, &param);
	MAG_ASSERT(vrenOutPort);

	vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	vschComp     = ooc_cast(hComponent, MagOmxComponent);
	
	vschComp->setName(vschComp, (OMX_U8 *)COMPONENT_NAME);
	vschCompImpl->addPort(vschCompImpl, START_PORT_INDEX + 0, vschInPort);
	vschCompImpl->addPort(vschCompImpl, START_PORT_INDEX + 1, clkInPort);
	vschCompImpl->addPort(vschCompImpl, START_PORT_INDEX + 2, vrenOutPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	OMX_HANDLETYPE vschInPort;
	OMX_HANDLETYPE clkInPort;
	OMX_HANDLETYPE vrenOutPort;
	MagOmxComponentImpl vschCompImpl;

	AGILE_LOGV("Enter!");
	vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	vschInPort  = vschCompImpl->getPort(vschCompImpl, START_PORT_INDEX + 0);
	clkInPort   = vschCompImpl->getPort(vschCompImpl, START_PORT_INDEX + 1);
	vrenOutPort = vschCompImpl->getPort(vschCompImpl, START_PORT_INDEX + 2);

	ooc_delete((Object)vschInPort);
	ooc_delete((Object)clkInPort);
	ooc_delete((Object)vrenOutPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_FFmpeg_Vsch_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
	AVFrame *decodedFrame = NULL;
	OMX_TICKS timeStamp;
	MagOmxComponentImpl         vschCompImpl;

	if (hDestPort == NULL){
		return OMX_ErrorBadParameter;
	}

	if ((srcbufHeader->pBuffer != NULL) && (srcbufHeader->nFilledLen > 0)){
		vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

		decodedFrame = (AVFrame *)srcbufHeader->pBuffer;
		timeStamp    = srcbufHeader->nTimeStamp;
		AGILE_LOGV("Put decoded Video Frame: %p, time stamp: %lld to AVSync", decodedFrame, timeStamp);

		vschCompImpl->syncDisplay(vschCompImpl, srcbufHeader);
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Vsch_DoAVSync(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime){
	MagOmxComponent_FFmpeg_Vsch thiz;
	MagOmxComponentImpl         vschCompImpl;
	OMX_BUFFERHEADERTYPE        *getBuffer;
	OMX_ERRORTYPE               ret;
	/*MagOmxPort                  hPort;*/
	AVFrame                     *decodedFrame;
	OMX_TICKS                   timeStamp;

	thiz         = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vsch);
	vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

	ret = vschCompImpl->releaseDisplay(vschCompImpl, mediaTime->nMediaTimestamp, &getBuffer);
	if (ret != OMX_ErrorNone){
		AGILE_LOGE("failed to releaseDisplay!");
		return ret;
	}
	decodedFrame = (AVFrame *)getBuffer->pBuffer;
	timeStamp    = getBuffer->nTimeStamp;

	AGILE_LOGV("Release decoded Video Frame: %p[w: %d, h: %d, pixelFormat: %d], time stamp: %lld to playback", 
		        decodedFrame, decodedFrame->width, decodedFrame->height, decodedFrame->format, timeStamp);

#ifdef CAPTURE_YUV_DATA_TO_FILE
	fwrite(decodedFrame->data[0], 1, decodedFrame->linesize[0], thiz->mfYUVFile);
	fwrite(decodedFrame->data[1], 1, decodedFrame->linesize[1], thiz->mfYUVFile);
	fwrite(decodedFrame->data[2], 1, decodedFrame->linesize[2], thiz->mfYUVFile);
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
static void MagOmxComponent_FFmpeg_Vsch_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_FFmpeg_Vsch_GetComponentUUID;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Prepare           = virtual_FFmpeg_Vsch_Prepare;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Preroll           = virtual_FFmpeg_Vsch_Preroll;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Start             = virtual_FFmpeg_Vsch_Start;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Stop              = virtual_FFmpeg_Vsch_Stop;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Pause             = virtual_FFmpeg_Vsch_Pause;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Resume            = virtual_FFmpeg_Vsch_Resume;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Deinit            = virtual_FFmpeg_Vsch_Deinit;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Reset             = virtual_FFmpeg_Vsch_Reset;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_FFmpeg_Vsch_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_FFmpeg_Vsch_ProceedBuffer;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_DoAVSync          = virtual_FFmpeg_Vsch_DoAVSync;
}

static void MagOmxComponent_FFmpeg_Vsch_constructor(MagOmxComponent_FFmpeg_Vsch thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Vsch));
    chain_constructor(MagOmxComponent_FFmpeg_Vsch, thiz, params);
}

static void MagOmxComponent_FFmpeg_Vsch_destructor(MagOmxComponent_FFmpeg_Vsch thiz, MagOmxComponent_FFmpeg_VschVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vsch_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_FFmpeg_Vsch hVschComp;
	MagOmxComponentImpl     parent;
    OMX_U32 param[2];

    AGILE_LOGV("Enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Vsch);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hVschComp = (MagOmxComponent_FFmpeg_Vsch) ooc_new( MagOmxComponent_FFmpeg_Vsch, (void *)param);
    MAG_ASSERT(hVschComp);

    parent = ooc_cast(hVschComp, MagOmxComponentImpl);
    AGILE_LOGV("before Create()!");
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hVschComp, pAppData, pCallBacks);
    AGILE_LOGV("after Create(), *hComponent=%p!", *hComponent);
    if (*hComponent){
    	return localSetupComponent(hVschComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vsch_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Vsch hVschComp;

	AGILE_LOGV("Enter!");
	hVschComp = (MagOmxComponent_FFmpeg_Vsch)compType->pComponentPrivate;

#ifdef CAPTURE_YUV_DATA_TO_FILE
	fclose(hVschComp->mfYUVFile);
#endif

	ooc_delete((Object)hVschComp);

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {ROLE_NAME, NULL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_FFmpeg_Vsch_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Vsch_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef ROLE_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER