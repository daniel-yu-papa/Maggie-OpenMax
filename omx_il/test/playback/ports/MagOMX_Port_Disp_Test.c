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

#include "MagOMX_Port_Disp_Test.h"

AllocateClass(MagOmxPort_DispTest, MagOmxPortVideo);

static OMX_ERRORTYPE virtual_PortDisp_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	OMX_U8 *buf;

	AGILE_LOGV("enter!");
	buf = (OMX_U8 *)mag_mallocz(nSizeBytes);
	if (buf){
		*ppBuffer = buf;
		AGILE_LOGD("allocate buffer: %p", buf);
		return OMX_ErrorNone;
	}else{
		return OMX_ErrorInsufficientResources;
	}
}

static OMX_ERRORTYPE virtual_PortDisp_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
	AGILE_LOGD("free buffer: %p", pBuffer);
	mag_free(pBuffer);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_PortDisp_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	AGILE_LOGD("proceed buffer header: 0x%x", pBufHeader);
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_DispTest_initialize(Class this){
	MagOmxPort_DispTestVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_AllocateBuffer        = virtual_PortDisp_AllocateBuffer;
	MagOmxPort_DispTestVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_FreeBuffer            = virtual_PortDisp_FreeBuffer;
	MagOmxPort_DispTestVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer = virtual_PortDisp_ProceedReturnedBuffer;
}

static void MagOmxPort_DispTest_constructor(MagOmxPort_DispTest thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxPort_DispTest));
    chain_constructor(MagOmxPort_DispTest, thiz, params);
}

static void MagOmxPort_DispTest_destructor(MagOmxPort_DispTest thiz, MagOmxPort_DispTestVtable vtab){
}