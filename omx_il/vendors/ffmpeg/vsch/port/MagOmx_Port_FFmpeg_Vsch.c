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

#include "MagOmx_Port_FFmpeg_Vsch.h"

#include "libavutil/frame.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

AllocateClass(MagOmxPort_FFmpeg_Vsch, MagOmxPortVideo);

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	if (nSizeBytes != 0){
		AGILE_LOGE("no buffer is allocated, the size[%d] should be 0", nSizeBytes);
	}

	/*don't pre-allocate the memory, it uses the ffmpeg decoded AVFrame*/
	*ppBuffer = NULL;
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
    /*don't free any buffer*/
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Vsch_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	AVFrame *frame;

	AGILE_LOGD("proceed buffer header: 0x%x", pBufHeader);
	if (pBufHeader == NULL){
		AGILE_LOGE("wrong pBufHeader is NULL!");
		return OMX_ErrorBadParameter;
	}

	/*release the used video frame buffer*/
	if (pBufHeader->pBuffer){
		frame = (AVFrame *)pBufHeader->pBuffer;
		av_frame_unref(frame);
	}
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_FFmpeg_Vsch_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	MagOmxPort_FFmpeg_VschVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_AllocateBuffer        = virtual_FFmpeg_Vsch_AllocateBuffer;
	MagOmxPort_FFmpeg_VschVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_FreeBuffer            = virtual_FFmpeg_Vsch_FreeBuffer;
	MagOmxPort_FFmpeg_VschVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer = virtual_FFmpeg_Vsch_ProceedReturnedBuffer;
}

static void MagOmxPort_FFmpeg_Vsch_constructor(MagOmxPort_FFmpeg_Vsch thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxPort_FFmpeg_Vsch));
    chain_constructor(MagOmxPort_FFmpeg_Vsch, thiz, params);
}

static void MagOmxPort_FFmpeg_Vsch_destructor(MagOmxPort_FFmpeg_Vsch thiz, MagOmxPort_FFmpeg_VschVtable vtab){
}