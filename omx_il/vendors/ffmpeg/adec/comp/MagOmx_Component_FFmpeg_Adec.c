#include "MagOmx_Component_FFmpeg_Adec.h"
#include "MagOmx_Port_FFmpeg_Adec.h"
#include "MagOmx_Port_FFmpeg_Aren.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME "OMX.Mag.adec.ffmpeg"
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      2

AllocateClass(MagOmxComponent_FFmpeg_Adec, MagOmxComponentAudio);

static OMX_U32 getFFmpegCodecId(OMX_U32 audioEncoding){
	switch (audioEncoding){
		case OMX_AUDIO_CodingAAC:
			return AV_CODEC_ID_AAC;

		case OMX_AUDIO_CodingMP3:
			return AV_CODEC_ID_MP3;

		case OMX_AUDIO_CodingMP2:
			return AV_CODEC_ID_MP2;

		case OMX_AUDIO_CodingAC3:
			return AV_CODEC_ID_AC3;

		case OMX_AUDIO_CodingDTS:
			return AV_CODEC_ID_DTS;

		case OMX_AUDIO_CodingDDPlus:
			return AV_CODEC_ID_EAC3;

		default:
			AGILE_LOGE("unsupported OMX Audio Encoding: %d", audioEncoding);
	}

	return AV_CODEC_ID_NONE;
}

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_FFmpeg_Adec         adecInPort;
	MagOmxPort_FFmpeg_Aren         arenOutPort;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponentImpl            adecCompImpl;
	MagOmxComponent                adecComp;

	AGILE_LOGV("enter!");

	param.portIndex    = START_PORT_INDEX + 0;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", ADEC_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Adec);
	adecInPort = ooc_new(MagOmxPort_FFmpeg_Adec, &param);
	MAG_ASSERT(adecInPort);

	param.portIndex    = START_PORT_INDEX + 1;
	param.isInput      = OMX_FALSE;
	param.bufSupplier  = OMX_BufferSupplyOutput;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-Out", AREN_PORT_NAME);

	ooc_init_class(MagOmxPort_FFmpeg_Aren);
	arenOutPort = ooc_new(MagOmxPort_FFmpeg_Aren, &param);
	MAG_ASSERT(arenOutPort);

	adecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	adecComp = ooc_cast(hComponent, MagOmxComponent);
	
	adecComp->setName(adecComp, (OMX_U8 *)COMPONENT_NAME);
	adecCompImpl->addPort(adecCompImpl, START_PORT_INDEX + 0, adecInPort);
	adecCompImpl->addPort(adecCompImpl, START_PORT_INDEX + 1, arenOutPort);

	adecCompImpl->setupPortDataFlow(adecCompImpl, adecInPort, arenOutPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_FFmpeg_Adec adecInPort;
	MagOmxPort             adecInPortRoot;
	MagOmxComponentImpl    adecCompImpl;
	MagOmxComponent_FFmpeg_Adec thiz;
	OMX_AUDIO_PORTDEFINITIONTYPE AudioDef;
	OMX_ERRORTYPE ret;
	OMX_U32 codec_id = AV_CODEC_ID_NONE;

	adecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	thiz     = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Adec);

	adecInPort = adecCompImpl->getPort(adecCompImpl, START_PORT_INDEX);
	
	if (adecInPort){
		adecInPortRoot = ooc_cast(adecInPort, MagOmxPort);
		ret = MagOmxPortVirtual(adecInPortRoot)->GetPortSpecificDef(adecInPort, &AudioDef);
		if (ret == OMX_ErrorNone){
			codec_id = getFFmpegCodecId((OMX_U32)AudioDef.eEncoding);
			if (codec_id != AV_CODEC_ID_NONE){
				if (thiz->mpAudioCodec == NULL){
					thiz->mpAudioCodec = avcodec_find_decoder(codec_id);
					if (thiz->mpAudioCodec == NULL){
						AGILE_LOGE("Failed to find the decoder with id: %d", codec_id);
						return OMX_ErrorUndefined;
					}
				}else{
					AGILE_LOGE("The AudioCodec has been created! ignore the preparation");
				}
			}else{
				AGILE_LOGE("The audio codec id is none!");
				return OMX_ErrorUnsupportedIndex;
			}
		}else{
			AGILE_LOGE("Failed to get the audio port definition!");
			return OMX_ErrorUndefined;
		}
	}else{
		AGILE_LOGE("Failed to get the audio port with the port index %d!", START_PORT_INDEX);
		return OMX_ErrorUndefined;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Adec adecComp;
	int ret;

	adecComp = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Adec);
	if (adecComp->mpAudioStream == NULL || adecComp->mpAudioCodec == NULL){
		AGILE_LOGE("Bad action: mpAudioStream=%p, mpAudioCodec=%p.", 
			        adecComp->mpAudioStream, adecComp->mpAudioCodec);
		return OMX_ErrorUndefined;
	}

	ret = avcodec_open2(adecComp->mpAudioStream->codec, adecComp->mpAudioCodec, NULL);
	if (ret < 0){
		AGILE_LOGE("Failed to do avcodec_open2(), ret = %d", ret);
        return OMX_ErrorUndefined;
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	OMX_HANDLETYPE adecInPort;
	OMX_HANDLETYPE arenOutPort;
	MagOmxComponentImpl adecCompImpl;

	AGILE_LOGV("adec enter!");
	adecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	adecInPort  = adecCompImpl->getPort(adecCompImpl, START_PORT_INDEX + 0);
	arenOutPort = adecCompImpl->getPort(adecCompImpl, START_PORT_INDEX + 1);

	ooc_delete((Object)adecInPort);
	ooc_delete((Object)arenOutPort);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_FFmpeg_Adec_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
	MagOmxPort port;
	OMX_BUFFERHEADERTYPE *destbufHeader;
	AVPacket codedPkt;
	int got_frame;
	int decodedLen;
	AVFrame *decodedFrame = NULL;
	MagOmxComponent_FFmpeg_Adec adecComp;
	OMX_BOOL continueDec = OMX_TRUE;
	AVRational tb;

	if (hDestPort == NULL){
		return OMX_ErrorBadParameter;
	}

	if ((srcbufHeader->pBuffer != NULL) && (srcbufHeader->nFilledLen > 0)){
		port = ooc_cast(hDestPort, MagOmxPort);
	    
		adecComp     = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Adec);

		av_init_packet(&codedPkt);
		codedPkt.data = srcbufHeader->pBuffer;
		codedPkt.size = srcbufHeader->nFilledLen;
		codedPkt.pts  = srcbufHeader->nTimeStamp;
		codedPkt.dts  = srcbufHeader->nTimeStamp;

		do {
			decodedFrame = av_frame_alloc();
			decodedLen = avcodec_decode_audio4(adecComp->mpAudioStream->codec, 
				                               decodedFrame, &got_frame, &codedPkt);

			codedPkt.dts  =  AV_NOPTS_VALUE;
			codedPkt.pts  =  AV_NOPTS_VALUE;
			codedPkt.data += decodedLen;
			codedPkt.size -= decodedLen;

			if ((codedPkt.data && codedPkt.size <= 0) || (!codedPkt.data && !got_frame))
				continueDec = OMX_FALSE;

			if (!got_frame){
				continue;
			}else{
				destbufHeader = MagOmxPortVirtual(port)->GetOutputBuffer(port);

				tb = (AVRational){1, decodedFrame->sample_rate};
				if (decodedFrame->pts != AV_NOPTS_VALUE){
                    decodedFrame->pts = av_rescale_q(decodedFrame->pts, 
                    	                             adecComp->mpAudioStream->codec->time_base, 
                    	                             tb);
				}else if (decodedFrame->pkt_pts != AV_NOPTS_VALUE){
                    decodedFrame->pts = av_rescale_q(decodedFrame->pkt_pts, 
                    	                             adecComp->mpAudioStream->time_base, 
                    	                             tb);
				}else{
					AGILE_LOGE("audio decoded frame pts(0x%x) error", decodedFrame->pts);
                }

				destbufHeader->pBuffer = (OMX_U8 *)decodedFrame;
				destbufHeader->nTimeStamp = decodedFrame->pts;
				MagOmxPortVirtual(port)->SendOutputBuffer(port, destbufHeader);
				AGILE_LOGV("Send src buffer: 0x%x, output buffer: 0x%x", srcbufHeader, destbufHeader);
			}
		}while(continueDec);
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Adec_DoAVSync(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime){
	AGILE_LOGE("Invalid action!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_GetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nParamIndex,  
                    OMX_INOUT OMX_PTR pComponentParameterStructure){
	return OMX_ErrorUnsupportedIndex;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_SetParameter(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_INDEXTYPE nIndex,
                    OMX_IN  OMX_PTR pComponentParameterStructure){

	MagOmxComponent_FFmpeg_Adec thiz;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Adec);

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
static void MagOmxComponent_FFmpeg_Adec_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_FFmpeg_Adec_GetComponentUUID;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Prepare           = virtual_FFmpeg_Adec_Prepare;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Preroll           = virtual_FFmpeg_Adec_Preroll;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Start             = virtual_FFmpeg_Adec_Start;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Stop              = virtual_FFmpeg_Adec_Stop;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Pause             = virtual_FFmpeg_Adec_Pause;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Resume            = virtual_FFmpeg_Adec_Resume;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Deinit            = virtual_FFmpeg_Adec_Deinit;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_Reset             = virtual_FFmpeg_Adec_Reset;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_FFmpeg_Adec_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_FFmpeg_Adec_ProceedBuffer;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmxComponentImpl.MagOMX_DoAVSync          = virtual_FFmpeg_Adec_DoAVSync;

    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmx_Audio_GetParameter = virtual_FFmpeg_Adec_GetParameter;
    MagOmxComponent_FFmpeg_AdecVtableInstance.MagOmxComponentAudio.MagOmx_Audio_SetParameter = virtual_FFmpeg_Adec_SetParameter;
}

static void MagOmxComponent_FFmpeg_Adec_constructor(MagOmxComponent_FFmpeg_Adec thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Adec));
    chain_constructor(MagOmxComponent_FFmpeg_Adec, thiz, params);

    thiz->mpAudioCodec = NULL;
}

static void MagOmxComponent_FFmpeg_Adec_destructor(MagOmxComponent_FFmpeg_Adec thiz, MagOmxComponent_FFmpeg_AdecVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Adec_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_FFmpeg_Adec hAdecComp;
	MagOmxComponentImpl     parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Adec);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hAdecComp = (MagOmxComponent_FFmpeg_Adec) ooc_new( MagOmxComponent_FFmpeg_Adec, (void *)param);
    MAG_ASSERT(hAdecComp);

    parent = ooc_cast(hAdecComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hAdecComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hAdecComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Adec_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Adec hAdecComp;

	AGILE_LOGV("MagOmxComponent_FFmpeg_Adec_DeInit enter!");
	hAdecComp = (MagOmxComponent_FFmpeg_Adec)compType->pComponentPrivate;
	ooc_delete((Object)hAdecComp);

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {OMX_ROLE_AUDIO_DECODER_AAC, 
		                     OMX_ROLE_AUDIO_DECODER_MP3,
		                     OMX_ROLE_AUDIO_DECODER_EXT_MP2,
		                     OMX_ROLE_AUDIO_DECODER_EXT_AC3,
		                     OMX_ROLE_AUDIO_DECODER_EXT_DDPLUS,
		                     OMX_ROLE_AUDIO_DECODER_EXT_DTS};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 6, MagOmxComponent_FFmpeg_Adec_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Adec_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER