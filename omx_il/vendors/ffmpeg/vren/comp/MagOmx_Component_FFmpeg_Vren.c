#include "libavutil/frame.h"

#include "MagOmx_Component_FFmpeg_Vren.h"
#include "MagOmx_Port_FFmpeg_Vren.h"
#include "MagOmx_Port_FFmpeg_Vsch.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.vren.ffmpeg"
#define ROLE_NAME      OMX_ROLE_IV_RENDERER_YUV_BLTER
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      2

AllocateClass(MagOmxComponent_FFmpeg_Vren, MagOmxComponentVideo);

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxPort_FFmpeg_Vren         vrenInPort;
    MagOmxPort_FFmpeg_Vren         vrenOutPort;
    MagOmxPort_Constructor_Param_t param;
    MagOmxComponentImpl            vrenCompImpl;
    MagOmxComponent                vrenComp;
    MagOmxComponent_FFmpeg_Vren    thiz;

    AGILE_LOGV("enter!");

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vren);
    vrenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    vrenComp     = ooc_cast(hComponent, MagOmxComponent);

    vrenComp->setName(vrenComp, (OMX_U8 *)COMPONENT_NAME);

    param.portIndex    = START_PORT_INDEX + 0;
    param.isInput      = OMX_TRUE;
    param.bufSupplier  = OMX_BufferSupplyUnspecified;
    param.formatStruct = 0;
    sprintf((char *)param.name, "%s-In", VREN_PORT_NAME);

    ooc_init_class(MagOmxPort_FFmpeg_Vren);
    vrenInPort = ooc_new(MagOmxPort_FFmpeg_Vren, &param);
    MAG_ASSERT(vrenInPort);

    param.portIndex    = START_PORT_INDEX + 1;
    param.isInput      = OMX_FALSE;
    param.bufSupplier  = OMX_BufferSupplyUnspecified;
    param.formatStruct = 0;
    sprintf((char *)param.name, "%s-Out-App", VREN_PORT_NAME);

    ooc_init_class(MagOmxPort_FFmpeg_Vren);
    vrenOutPort = ooc_new(MagOmxPort_FFmpeg_Vren, &param);
    MAG_ASSERT(vrenOutPort);

    vrenCompImpl->addPort(vrenCompImpl, START_PORT_INDEX + 0, vrenInPort);
    vrenCompImpl->addPort(vrenCompImpl, START_PORT_INDEX + 1, vrenOutPort);

    vrenCompImpl->setupPortDataFlow(vrenCompImpl, vrenInPort, vrenOutPort);

#ifdef CAPTURE_YUV_DATA_TO_FILE
    thiz->mfYUVFile = fopen("./video.yuv","wb+");
    if (thiz->mfYUVFile == NULL){
        AGILE_LOGE("Failed to open the file: ./video.yuv");
    }
#endif
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponent_FFmpeg_Vren thiz;
    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vren);

    AGILE_LOGV("enter!");
#ifdef CAPTURE_YUV_DATA_TO_FILE
    if (!thiz->mfYUVFile){
        thiz->mfYUVFile = fopen("./video.yuv","wb+");
        if (thiz->mfYUVFile == NULL){
            AGILE_LOGE("Failed to open the file: ./video.yuv");
        }
    }
#endif
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponent_FFmpeg_Vren thiz;
    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vren);

	AGILE_LOGV("enter!");
#ifdef CAPTURE_YUV_DATA_TO_FILE
    if (thiz->mfYUVFile){
        fclose(thiz->mfYUVFile);
        thiz->mfYUVFile = NULL;
    }
#endif
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_FFmpeg_Vren_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
    MagOmxComponentImpl     vrenCompImpl;
    AVFrame                 *decodedFrame;
    AVFrame                 *destFrame = NULL;
    OMX_BUFFERHEADERTYPE    *destbufHeader;
    MagOmxPort              destPort;
    int ret;
    int i;

    MagOmxComponent_FFmpeg_Vren thiz;

    vrenCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vren);
    destPort = ooc_cast(hDestPort, MagOmxPort);

    if (srcbufHeader->pBuffer    == NULL && 
        srcbufHeader->nFilledLen == 0    && 
        srcbufHeader->nTimeStamp == kInvalidTimeStamp){
        AGILE_LOGV("get the EOS frame and send out the eos event to application");
        vrenCompImpl->sendEvents(hComponent, OMX_EventBufferFlag, 0, 0, NULL);
        return OMX_ErrorNone;
    }

    destbufHeader = MagOmxPortVirtual(destPort)->GetOutputBuffer(destPort);

    decodedFrame = (AVFrame *)srcbufHeader->pBuffer;

    AGILE_LOGV("Display decoded Video Frame: %p[w: %d, h: %d, pixelFormat: %d][linesize: %d, %d, %d], time stamp: 0x%llx to playback", 
                decodedFrame, decodedFrame->width, decodedFrame->height, decodedFrame->format, 
                decodedFrame->linesize[0], decodedFrame->linesize[1], decodedFrame->linesize[2],
                srcbufHeader->nTimeStamp);

    destbufHeader->nFilledLen = srcbufHeader->nFilledLen;
    destbufHeader->nOffset    = srcbufHeader->nOffset;
    destbufHeader->nTimeStamp = srcbufHeader->nTimeStamp;

    destFrame = av_frame_alloc();
    ret = av_frame_ref(destFrame, (AVFrame *)srcbufHeader->pBuffer);
    destbufHeader->pBuffer = (OMX_U8 *)destFrame;

    MagOmxPortVirtual(destPort)->sendOutputBufferToAPP(destPort, destbufHeader);

#ifdef CAPTURE_YUV_DATA_TO_FILE
    if (thiz->mfYUVFile){
        for (i = 0; i < decodedFrame->height; i++){
            fwrite(decodedFrame->data[0] + i * decodedFrame->linesize[0], 
                   1, decodedFrame->width, 
                   thiz->mfYUVFile);
        }

        for (i = 0; i < decodedFrame->height / 2; i++){
            fwrite(decodedFrame->data[1] + i * decodedFrame->linesize[1], 
                   1, decodedFrame->width / 2, 
                   thiz->mfYUVFile);
        }

        for (i = 0; i < decodedFrame->height / 2; i++){
            fwrite(decodedFrame->data[2] + i * decodedFrame->linesize[2], 
                   1, decodedFrame->width / 2, 
                   thiz->mfYUVFile);
        }

        fflush(thiz->mfYUVFile);
    }
#endif

    /*vrenCompImpl->sendReturnBuffer(vrenCompImpl, srcbufHeader);*/

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Vren_DoAVSync(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime){
	AGILE_LOGE("Invalid action!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_GetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure){

	return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vren_SetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure){

	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_Vren_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_FFmpeg_Vren_GetComponentUUID;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Prepare           = virtual_FFmpeg_Vren_Prepare;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Preroll           = virtual_FFmpeg_Vren_Preroll;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Start             = virtual_FFmpeg_Vren_Start;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Stop              = virtual_FFmpeg_Vren_Stop;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Pause             = virtual_FFmpeg_Vren_Pause;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Resume            = virtual_FFmpeg_Vren_Resume;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Deinit            = virtual_FFmpeg_Vren_Deinit;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Reset             = virtual_FFmpeg_Vren_Reset;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_FFmpeg_Vren_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_FFmpeg_Vren_ProceedBuffer;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_DoAVSync          = virtual_FFmpeg_Vren_DoAVSync;

    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmx_Video_GetParameter = virtual_FFmpeg_Vren_GetParameter;
    MagOmxComponent_FFmpeg_VrenVtableInstance.MagOmxComponentVideo.MagOmx_Video_SetParameter = virtual_FFmpeg_Vren_SetParameter;
}

static void MagOmxComponent_FFmpeg_Vren_constructor(MagOmxComponent_FFmpeg_Vren thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Vren));
    chain_constructor(MagOmxComponent_FFmpeg_Vren, thiz, params);
}

static void MagOmxComponent_FFmpeg_Vren_destructor(MagOmxComponent_FFmpeg_Vren thiz, MagOmxComponent_FFmpeg_VrenVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vren_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_FFmpeg_Vren hVrenComp;
	MagOmxComponentImpl     parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Vren);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hVrenComp = (MagOmxComponent_FFmpeg_Vren) ooc_new( MagOmxComponent_FFmpeg_Vren, (void *)param);
    MAG_ASSERT(hVrenComp);

    parent = ooc_cast(hVrenComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hVrenComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hVrenComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vren_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Vren hVrenComp;

	AGILE_LOGV("MagOmxComponent_FFmpeg_Vren_DeInit enter!");
	hVrenComp = (MagOmxComponent_FFmpeg_Vren)compType->pComponentPrivate;
	ooc_delete((Object)hVrenComp);
#ifdef CAPTURE_YUV_DATA_TO_FILE
    fclose(hVrenComp->mfYUVFile);
#endif
	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {ROLE_NAME, NULL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_FFmpeg_Vren_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Vren_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef ROLE_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER