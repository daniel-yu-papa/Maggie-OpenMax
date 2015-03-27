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

#include "MagOMX_Port_DataSource.h"
#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompDataSource"

AllocateClass(MagOmxPortDataSource, MagOmxPortImpl);

static OMX_PORTDOMAINTYPE virtual_MagOmxPortDataSource_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainOther_DataSource;
}

static OMX_ERRORTYPE virtual_MagOmxPortDataSource_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortDataSource_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortDataSource_SetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortDataSource bufPort;
	OMX_OTHER_PORTDEFINITIONTYPE *pBufDef = (OMX_OTHER_PORTDEFINITIONTYPE *)pFormat;
    
	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	bufPort = ooc_cast(hPort, MagOmxPortDataSource);

	Mag_AcquireMutex(bufPort->mhMutex);
	memcpy(&bufPort->mPortDefinition, pBufDef, sizeof(OMX_OTHER_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(bufPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortDataSource_GetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortDataSource bufPort;
	OMX_OTHER_PORTDEFINITIONTYPE *pBufDef = (OMX_OTHER_PORTDEFINITIONTYPE *)pFormat;

	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	bufPort = ooc_cast(hPort, MagOmxPortDataSource);

	Mag_AcquireMutex(bufPort->mhMutex);
	memcpy(pBufDef, &bufPort->mPortDefinition, sizeof(OMX_OTHER_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(bufPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortDataSource_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	if (nSizeBytes != 0){
		AGILE_LOGE("no buffer is allocated, the size[%d] should be 0", nSizeBytes);
	}

	*ppBuffer = (OMX_U8 *)mag_malloc(nSizeBytes);
	if (*ppBuffer == NULL){
		return OMX_ErrorInsufficientResources;
	}
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortDataSource_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
    mag_free(pBuffer);
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPortDataSource_initialize(Class this){
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_MagOmxPortDataSource_GetDomainType;
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOmxPort.SetPortSpecificDef  = virtual_MagOmxPortDataSource_SetPortSpecificDef;
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOmxPort.GetPortSpecificDef  = virtual_MagOmxPortDataSource_GetPortSpecificDef;
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOmxPort.SetParameter        = virtual_MagOmxPortDataSource_SetParameter;
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOmxPort.GetParameter        = virtual_MagOmxPortDataSource_GetParameter;
	
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOMX_AllocateBuffer          = virtual_MagOmxPortDataSource_AllocateBuffer;
	MagOmxPortDataSourceVtableInstance.MagOmxPortImpl.MagOMX_FreeBuffer              = virtual_MagOmxPortDataSource_FreeBuffer;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 */
static void MagOmxPortDataSource_constructor(MagOmxPortDataSource thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(MagOmxPortDataSource));
    chain_constructor(MagOmxPortDataSource, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
}

static void MagOmxPortDataSource_destructor(MagOmxPortDataSource thiz, MagOmxPortDataSourceVtable vtab){
    AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
}