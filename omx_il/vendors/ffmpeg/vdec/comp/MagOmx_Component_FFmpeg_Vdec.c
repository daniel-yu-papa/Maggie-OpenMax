#include "MagOmx_Component_FFmpeg_Vdec.h"
#include "MagOmx_Port_FFmpeg_Vdec.h"
#include "MagOmx_Port_FFmpeg_Vsch.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.vdec.ffmpeg"
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      2

AllocateClass(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo);

static OMX_U32 getFFmpegCodecId(OMX_VIDEO_CODINGTYPE videoEncoding){
	switch (videoEncoding){
		case OMX_VIDEO_CodingAVC:
			return AV_CODEC_ID_H264;

		case OMX_VIDEO_CodingMPEG2:
			return AV_CODEC_ID_MPEG2VIDEO;

		case OMX_VIDEO_CodingH263:
			return AV_CODEC_ID_H263;

		case OMX_VIDEO_CodingMPEG4:
			return AV_CODEC_ID_MPEG4;

		case OMX_VIDEO_CodingWMV:
			return AV_CODEC_ID_WMV2;

		case OMX_VIDEO_CodingMJPEG:
			return AV_CODEC_ID_MJPEG;

		case OMX_VIDEO_CodingVP8:
			return AV_CODEC_ID_VP8;

		default:
			AGILE_LOGE("unsupported OMX Video Encoding: %d", videoEncoding);
	}

	return AV_CODEC_ID_NONE;
}

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_FFmpeg_Vdec         vdecInPort;
	MagOmxPort_FFmpeg_Vsch         vschOutPort;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponentImpl            vdecCompImpl;
	MagOmxComponent                vdecComp;

	AGILE_LOGV("enter!");

	param.portIndex    = START_PORT_INDEX + 0;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", VDEC_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Vdec);
	vdecInPort = ooc_new(MagOmxPort_FFmpeg_Vdec, &param);
	MAG_ASSERT(vdecInPort);

	param.portIndex    = START_PORT_INDEX + 1;
	param.isInput      = OMX_FALSE;
	param.bufSupplier  = OMX_BufferSupplyOutput;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-Out", VSCH_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Vsch);
	vschOutPort = ooc_new(MagOmxPort_FFmpeg_Vsch, &param);
	MAG_ASSERT(vschOutPort);

	vdecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	vdecComp     = ooc_cast(hComponent, MagOmxComponent);
	
	vdecComp->setName(vdecComp, (OMX_U8 *)COMPONENT_NAME);
	vdecCompImpl->addPort(vdecCompImpl, START_PORT_INDEX + 0, vdecInPort);
	vdecCompImpl->addPort(vdecCompImpl, START_PORT_INDEX + 1, vschOutPort);

	vdecCompImpl->setupPortDataFlow(vdecCompImpl, vdecInPort, vschOutPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_FFmpeg_Vdec vdecInPort;
	MagOmxPort             vdecInPortRoot;
	MagOmxComponentImpl    vdecCompImpl;
	MagOmxComponent_FFmpeg_Vdec thiz;
	OMX_VIDEO_PORTDEFINITIONTYPE VideoDef;
	OMX_ERRORTYPE ret;
	OMX_U32 codec_id = AV_CODEC_ID_NONE;

	vdecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	thiz         = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vdec);

	vdecInPort = vdecCompImpl->getPort(vdecCompImpl, START_PORT_INDEX);
	
	if (vdecInPort){
		vdecInPortRoot = ooc_cast(vdecInPort, MagOmxPort);
		ret = MagOmxPortVirtual(vdecInPortRoot)->GetPortSpecificDef(vdecInPort, &VideoDef);
		if (ret == OMX_ErrorNone){
			codec_id = getFFmpegCodecId(VideoDef.eCompressionFormat);
			if (codec_id != AV_CODEC_ID_NONE){
				if (thiz->mpVideoCodec == NULL){
					thiz->mpVideoCodec = avcodec_find_decoder(codec_id);
					if (thiz->mpVideoCodec == NULL){
						AGILE_LOGE("Failed to find the decoder with id: %d", codec_id);
						return OMX_ErrorUndefined;
					}
				}else{
					AGILE_LOGE("The VideoCodec has been created! ignore the preparation");
				}
			}else{
				AGILE_LOGE("The video codec id is none!");
				return OMX_ErrorUnsupportedIndex;
			}
		}else{
			AGILE_LOGE("Failed to get the video port definition!");
			return OMX_ErrorUndefined;
		}
	}else{
		AGILE_LOGE("Failed to get the video port with the port index %d!", START_PORT_INDEX);
		return OMX_ErrorUndefined;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Vdec vdecComp;
	int ret;

	vdecComp = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vdec);
	if (vdecComp->mpVideoStream == NULL || vdecComp->mpVideoCodec == NULL){
		AGILE_LOGE("Bad action: mpVideoStream=%p, mpVideoCodec=%p.", 
			        vdecComp->mpVideoStream, vdecComp->mpVideoCodec);
		return OMX_ErrorUndefined;
	}

	ret = avcodec_open2(vdecComp->mpVideoStream->codec, vdecComp->mpVideoCodec, NULL);
	if (ret < 0){
		AGILE_LOGE("Failed to do avcodec_open2(), ret = %d", ret);
        return OMX_ErrorUndefined;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	OMX_HANDLETYPE vdecInPort;
	OMX_HANDLETYPE vschOutPort;
	MagOmxComponentImpl vdecCompImpl;

	AGILE_LOGV("vdec enter!");
	vdecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	vdecInPort  = vdecCompImpl->getPort(vdecCompImpl, START_PORT_INDEX + 0);
	vschOutPort = vdecCompImpl->getPort(vdecCompImpl, START_PORT_INDEX + 1);

	ooc_delete((Object)vdecInPort);
	ooc_delete((Object)vschOutPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_FFmpeg_Vdec_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
	MagOmxPort port;
	OMX_BUFFERHEADERTYPE *destbufHeader;
	AVPacket codedPkt;
	int got_frame;
	int decodedLen;
	AVFrame *decodedFrame = NULL;
	MagOmxComponent_FFmpeg_Vdec vdecComp;

	if (hDestPort == NULL){
		return OMX_ErrorBadParameter;
	}

	if ((srcbufHeader->pBuffer != NULL) && (srcbufHeader->nFilledLen > 0)){
		port = ooc_cast(hDestPort, MagOmxPort);
	    
		vdecComp     = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vdec);

		av_init_packet(&codedPkt);
		codedPkt.data = srcbufHeader->pBuffer;
		codedPkt.size = srcbufHeader->nFilledLen;
		codedPkt.pts  = srcbufHeader->nTimeStamp;
		codedPkt.dts  = srcbufHeader->nTimeStamp;

		decodedFrame = av_frame_alloc();
		decodedLen = avcodec_decode_video2(vdecComp->mpVideoStream->codec, 
			                               decodedFrame, &got_frame, &codedPkt);
		
		if (got_frame && decodedLen > 0){
			destbufHeader = MagOmxPortVirtual(port)->GetOutputBuffer(port);

			decodedFrame->pts = av_frame_get_best_effort_timestamp(decodedFrame);
			decodedFrame->sample_aspect_ratio = av_guess_sample_aspect_ratio(vdecComp->mpAVFormat, 
				                                                             vdecComp->mpVideoStream, 
				                                                             decodedFrame);
			destbufHeader->pBuffer = (OMX_U8 *)decodedFrame;
			if (decodedFrame->pts != AV_NOPTS_VALUE)
				decodedFrame->pts = av_q2d(vdecComp->mpVideoStream->time_base) * decodedFrame->pts;
			destbufHeader->nTimeStamp = decodedFrame->pts;
			MagOmxPortVirtual(port)->SendOutputBuffer(port, destbufHeader);
			AGILE_LOGV("Send src buffer: 0x%x, output buffer: 0x%x", srcbufHeader, destbufHeader);
		}
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Vdec_DoAVSync(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime){
	AGILE_LOGE("Invalid action!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_GetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure){
	return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vdec_SetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure){

	MagOmxComponent_FFmpeg_Vdec thiz;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vdec);

    switch (nIndex){
        case OMX_IndexConfigExtFFMpegData:
        {
            OMX_CONFIG_FFMPEG_DATA_TYPE *ffmpegData = (OMX_CONFIG_FFMPEG_DATA_TYPE *)pComponentParameterStructure;
            AGILE_LOGI("set parameter: avstream=%p, avformat=%p",
            	        ffmpegData->avstream,
            	        ffmpegData->avformat);
            thiz->mpVideoStream = (AVStream *)ffmpegData->avstream;
	    	thiz->mpAVFormat    = (AVFormatContext *)ffmpegData->avformat;
        }
        	break;

    	default:
        	return OMX_ErrorUnsupportedIndex;
    }

	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_Vdec_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_FFmpeg_Vdec_GetComponentUUID;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Prepare           = virtual_FFmpeg_Vdec_Prepare;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Preroll           = virtual_FFmpeg_Vdec_Preroll;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Start             = virtual_FFmpeg_Vdec_Start;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Stop              = virtual_FFmpeg_Vdec_Stop;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Pause             = virtual_FFmpeg_Vdec_Pause;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Resume            = virtual_FFmpeg_Vdec_Resume;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Deinit            = virtual_FFmpeg_Vdec_Deinit;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Reset             = virtual_FFmpeg_Vdec_Reset;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_FFmpeg_Vdec_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_FFmpeg_Vdec_ProceedBuffer;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_DoAVSync          = virtual_FFmpeg_Vdec_DoAVSync;

    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmx_Video_GetParameter = virtual_FFmpeg_Vdec_GetParameter;
    MagOmxComponent_FFmpeg_VdecVtableInstance.MagOmxComponentVideo.MagOmx_Video_SetParameter = virtual_FFmpeg_Vdec_SetParameter;
}

static void MagOmxComponent_FFmpeg_Vdec_constructor(MagOmxComponent_FFmpeg_Vdec thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Vdec));
    chain_constructor(MagOmxComponent_FFmpeg_Vdec, thiz, params);

    thiz->mpVideoCodec  = NULL;
    thiz->mpAVFormat    = NULL;
    thiz->mpVideoStream = NULL;
}

static void MagOmxComponent_FFmpeg_Vdec_destructor(MagOmxComponent_FFmpeg_Vdec thiz, MagOmxComponent_FFmpeg_VdecVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vdec_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_FFmpeg_Vdec hVdecComp;
	MagOmxComponentImpl     parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Vdec);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hVdecComp = (MagOmxComponent_FFmpeg_Vdec) ooc_new( MagOmxComponent_FFmpeg_Vdec, (void *)param);
    MAG_ASSERT(hVdecComp);

    parent = ooc_cast(hVdecComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hVdecComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hVdecComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vdec_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Vdec hVdecComp;

	AGILE_LOGV("MagOmxComponent_FFmpeg_Vdec_DeInit enter!");
	hVdecComp = (MagOmxComponent_FFmpeg_Vdec)compType->pComponentPrivate;
	ooc_delete((Object)hVdecComp);

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {OMX_ROLE_VIDEO_DECODER_AVC,
	                         OMX_ROLE_VIDEO_DECODER_MPEG4,
	                         OMX_ROLE_VIDEO_DECODER_EXT_MPEG2,
	                         OMX_ROLE_VIDEO_DECODER_WMV,
	                         OMX_ROLE_VIDEO_DECODER_EXT_MJPEG,
	                         OMX_ROLE_VIDEO_DECODER_H263};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 6, MagOmxComponent_FFmpeg_Vdec_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Vdec_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER