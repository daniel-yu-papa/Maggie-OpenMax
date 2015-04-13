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

#include "MagOMX_Port_buffer.h"
#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"
#include "MagOMX_Component_buffer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompBuf"

AllocateClass(MagOmxPortBuffer, MagOmxPortImpl);

static OMX_PORTDOMAINTYPE virtual_MagOmxPortBuffer_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainOther_Buffer;
}

static OMX_ERRORTYPE virtual_MagOmxPortBuffer_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortBuffer_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortBuffer_SetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortBuffer bufPort;
	OMX_OTHER_PORTDEFINITIONTYPE *pBufDef = (OMX_OTHER_PORTDEFINITIONTYPE *)pFormat;
    
	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	bufPort = ooc_cast(hPort, MagOmxPortBuffer);

	Mag_AcquireMutex(bufPort->mhMutex);
	memcpy(&bufPort->mPortDefinition, pBufDef, sizeof(OMX_OTHER_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(bufPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortBuffer_GetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortBuffer bufPort;
	OMX_OTHER_PORTDEFINITIONTYPE *pBufDef = (OMX_OTHER_PORTDEFINITIONTYPE *)pFormat;

	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	bufPort = ooc_cast(hPort, MagOmxPortBuffer);

	Mag_AcquireMutex(bufPort->mhMutex);
	memcpy(pBufDef, &bufPort->mPortDefinition, sizeof(OMX_OTHER_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(bufPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortBuffer_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	MagOmxPort            root;
	MagOmxComponentImpl   hCompImpl;

	root = ooc_cast(port, MagOmxPort);

	hCompImpl = ooc_cast(root->getAttachedComponent(root), MagOmxComponentImpl);
	if (MagOmxComponentImplVirtual(hCompImpl)->MagOMX_ProceedUsedBuffer){
		return MagOmxComponentImplVirtual(hCompImpl)->MagOMX_ProceedUsedBuffer(hCompImpl, pBufHeader);
	}

	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPortBuffer_initialize(Class this){
	MagOmxPortBufferVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_MagOmxPortBuffer_GetDomainType;
	MagOmxPortBufferVtableInstance.MagOmxPortImpl.MagOmxPort.SetPortSpecificDef  = virtual_MagOmxPortBuffer_SetPortSpecificDef;
	MagOmxPortBufferVtableInstance.MagOmxPortImpl.MagOmxPort.GetPortSpecificDef  = virtual_MagOmxPortBuffer_GetPortSpecificDef;
	MagOmxPortBufferVtableInstance.MagOmxPortImpl.MagOmxPort.SetParameter        = virtual_MagOmxPortBuffer_SetParameter;
	MagOmxPortBufferVtableInstance.MagOmxPortImpl.MagOmxPort.GetParameter        = virtual_MagOmxPortBuffer_GetParameter;
	
	MagOmxPortBufferVtableInstance.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer   = virtual_MagOmxPortBuffer_ProceedReturnedBuffer;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 */
static void MagOmxPortBuffer_constructor(MagOmxPortBuffer thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(MagOmxPortBuffer));
    chain_constructor(MagOmxPortBuffer, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
}

static void MagOmxPortBuffer_destructor(MagOmxPortBuffer thiz, MagOmxPortBufferVtable vtab){
    AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
}