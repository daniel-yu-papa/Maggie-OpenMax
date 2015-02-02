/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

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

#define FILE_YUV_PRE_AYSYNC "./pre_avsync.yuv"

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
	thiz         = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vsch);

	vschComp->setName(vschComp, (OMX_U8 *)COMPONENT_NAME);
	vschCompImpl->addPort(vschCompImpl, START_PORT_INDEX + 0, vschInPort);
	vschCompImpl->addPort(vschCompImpl, START_PORT_INDEX + 1, clkInPort);
	vschCompImpl->addPort(vschCompImpl, START_PORT_INDEX + 2, vrenOutPort);

	vschCompImpl->setupPortDataFlow(vschCompImpl, vschInPort, vrenOutPort);

#ifdef CAPTURE_YUV_FRAME_BEFORE_AVSYNC
    thiz->mfYUVPreAvsync = fopen(FILE_YUV_PRE_AYSYNC,"wb+");
    if (!thiz->mfYUVPreAvsync){
        COMP_LOGE(vschComp, "Failed to open the file: %s", FILE_YUV_PRE_AYSYNC);
    }
#endif

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
    MagOmxComponent_FFmpeg_Vsch thiz;

	AGILE_LOGV("Enter!");
	vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    thiz         = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vsch);

#ifdef CAPTURE_YUV_FRAME_BEFORE_AVSYNC
    fclose(thiz->mfYUVPreAvsync);
#endif

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
	MagOmxComponentImpl  vschCompImpl;
    MagOmxComponent_FFmpeg_Vsch thiz;
    OMX_BUFFERHEADERTYPE *destbufHeader;
    MagOmxPort destPort;
    AVFrame *destFrame = NULL;
    AVFrame *srcFrame = NULL;
    int i;
    OMX_ERRORTYPE ret;

	vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    thiz = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Vsch);
    destPort = ooc_cast(hDestPort, MagOmxPort);
    
    AGILE_LOGV("proceed buffer header: %p!", srcbufHeader);

    destbufHeader = MagOmxPortVirtual(destPort)->GetOutputBuffer(destPort);

    if (srcbufHeader->pBuffer != NULL && srcbufHeader->nFilledLen > 0){
        int ret;

        srcFrame = (AVFrame *)srcbufHeader->pBuffer;
        AGILE_LOGV("srcFrame: w:%d, h:%d, format:%d", srcFrame->width, srcFrame->height, srcFrame->format);

        destbufHeader->nFilledLen = srcbufHeader->nFilledLen;
        destbufHeader->nOffset    = srcbufHeader->nOffset;
        destbufHeader->nTimeStamp = srcbufHeader->nTimeStamp;

        destFrame = av_frame_alloc();
        ret = av_frame_ref(destFrame, (AVFrame *)srcbufHeader->pBuffer);
        destbufHeader->pBuffer = (OMX_U8 *)destFrame;
        AGILE_LOGV("Put decoded Video Src Frame: %p - Dest Frame: %p, time stamp: 0x%llx, len: %d to AVSync",
                srcbufHeader->pBuffer, 
                destbufHeader->pBuffer, 
                destbufHeader->nTimeStamp, 
                destbufHeader->nFilledLen);
    }else{
        destbufHeader->pBuffer    = NULL;
        destbufHeader->nFilledLen = 0;
        destbufHeader->nOffset    = 0;
        destbufHeader->nTimeStamp = srcbufHeader->nTimeStamp;
        AGILE_LOGD("proceed the EOS buffer header!!!, nTimeStamp = 0x%llx", destbufHeader->nTimeStamp);
    }

#ifdef CAPTURE_YUV_FRAME_BEFORE_AVSYNC
        if (thiz->mfYUVPreAvsync){
            for (i = 0; i < destFrame->height; i++){
                fwrite(destFrame->data[0] + i * destFrame->linesize[0], 
                       1, destFrame->width, 
                       thiz->mfYUVPreAvsync);
            }

            for (i = 0; i < destFrame->height / 2; i++){
                fwrite(destFrame->data[1] + i * destFrame->linesize[1], 
                       1, destFrame->width / 2, 
                       thiz->mfYUVPreAvsync);
            }

            for (i = 0; i < destFrame->height / 2; i++){
                fwrite(destFrame->data[2] + i * destFrame->linesize[2], 
                       1, destFrame->width / 2, 
                       thiz->mfYUVPreAvsync);
            }

            fflush(thiz->mfYUVPreAvsync);
        }
#endif

    vschCompImpl->putOutputBuffer(vschCompImpl, destPort, destbufHeader);
	ret = vschCompImpl->syncDisplay(vschCompImpl, destbufHeader);
    if (ret == OMX_ErrorNotReady){
        vschCompImpl->discardOutputBuffer(vschCompImpl, destbufHeader);
    }

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Vsch_DoAVSync(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_TIME_MEDIATIMETYPE *mediaTime){
	MagOmxComponentImpl         vschCompImpl;
	OMX_BUFFERHEADERTYPE        *getBuffer;
	OMX_ERRORTYPE               ret;
    MagOMX_AVSync_Action_t      action;
	/*MagOmxPort                  hPort;*/
	
	vschCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

    action = (MagOMX_AVSync_Action_t)mediaTime->nClientPrivate;
	ret = vschCompImpl->releaseDisplay(vschCompImpl, mediaTime->nMediaTimestamp, &getBuffer);
	if (ret != OMX_ErrorNone){
		AGILE_LOGE("failed to releaseDisplay!");
		return ret;
	}

    if (action == AVSYNC_PLAY){
        AGILE_LOGV("Release decoded Video Frame: time stamp: 0x%llx, len: %d to Vren", 
                    mediaTime->nMediaTimestamp, getBuffer->nFilledLen);

    	vschCompImpl->sendOutputBuffer(vschCompImpl, getBuffer);
    }else{
        AGILE_LOGV("Drop decoded Video Frame: time stamp: 0x%llx, len: %d", 
                    mediaTime->nMediaTimestamp, getBuffer->nFilledLen);

        vschCompImpl->discardOutputBuffer(vschCompImpl, getBuffer);
    }

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Vsch_GetClockActionOffset(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_TICKS *pClockOffset){
    /*in us*/
	*pClockOffset = 2000;

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_FFmpeg_Vsch_GetRenderDelay(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_TICKS *pRenderDelay){
    /*in us*/
    *pRenderDelay = 1000;

    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_Vsch_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetComponentUUID     = virtual_FFmpeg_Vsch_GetComponentUUID;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Prepare              = virtual_FFmpeg_Vsch_Prepare;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Preroll              = virtual_FFmpeg_Vsch_Preroll;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Start                = virtual_FFmpeg_Vsch_Start;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Stop                 = virtual_FFmpeg_Vsch_Stop;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Pause                = virtual_FFmpeg_Vsch_Pause;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Resume               = virtual_FFmpeg_Vsch_Resume;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Deinit               = virtual_FFmpeg_Vsch_Deinit;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Reset                = virtual_FFmpeg_Vsch_Reset;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ComponentRoleEnum    = virtual_FFmpeg_Vsch_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ProceedBuffer        = virtual_FFmpeg_Vsch_ProceedBuffer;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_DoAVSync             = virtual_FFmpeg_Vsch_DoAVSync;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetClockActionOffset = virtual_FFmpeg_Vsch_GetClockActionOffset;
    MagOmxComponent_FFmpeg_VschVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetRenderDelay       = virtual_FFmpeg_Vsch_GetRenderDelay;
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
    AGILE_LOGV("hVschComp=%p!", hVschComp);

    parent = ooc_cast(hVschComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hVschComp, pAppData, pCallBacks);
    
    if (*hComponent){
    	return localSetupComponent(hVschComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Vsch_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Vsch hVschComp;

	AGILE_LOGD("enter!");
	hVschComp = (MagOmxComponent_FFmpeg_Vsch)compType->pComponentPrivate;

	ooc_delete((Object)hVschComp);
    AGILE_LOGD("exit!");
    
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