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

#include "MagOMX_Port_audio.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompAudio"

AllocateClass(MagOmxPortAudio, MagOmxPortImpl);

static MagOmxPortAudio getAudioPort(OMX_HANDLETYPE hPort){
    MagOmxPortAudio v;

    v = ooc_cast(hPort, MagOmxPortAudio);
    return v;
}

static OMX_PORTDOMAINTYPE virtual_MagOmxPortAudio_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainAudio;
}

static OMX_ERRORTYPE virtual_MagOmxPortAudio_SetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortAudio aPort;
	OMX_AUDIO_PORTDEFINITIONTYPE *pAudioDef = (OMX_AUDIO_PORTDEFINITIONTYPE *)pFormat;
	List_t *next;
    MagOMX_Audio_PortFormat_t *item = NULL;
    
	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	aPort = getAudioPort(hPort);

	Mag_AcquireMutex(aPort->mhMutex);
	next = aPort->mPortFormatList.next;
	if (next != &aPort->mPortFormatList)
	    item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);

    if (item){
    	item->eEncoding = pAudioDef->eEncoding;
    }

	memcpy(aPort->mpPortDefinition, pAudioDef, sizeof(OMX_AUDIO_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(aPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortAudio_GetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortAudio aPort;
	OMX_AUDIO_PORTDEFINITIONTYPE *pAudioDef = (OMX_AUDIO_PORTDEFINITIONTYPE *)pFormat;
	List_t *next;
    MagOMX_Audio_PortFormat_t *item = NULL;

	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	aPort = getAudioPort(hPort);

	Mag_AcquireMutex(aPort->mhMutex);
	next = aPort->mPortFormatList.next;
	if (next != &aPort->mPortFormatList)
	    item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);

	memcpy(pAudioDef, aPort->mpPortDefinition, sizeof(OMX_AUDIO_PORTDEFINITIONTYPE));
    if (item){
    	pAudioDef->eEncoding = item->eEncoding;
    }
	Mag_ReleaseMutex(aPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortAudio_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortAudio_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPortAudio aPort;
	OMX_U32 i;
	List_t *next;
	MagOMX_Audio_PortFormat_t *item = NULL;

	aPort = getAudioPort(hPort);

	switch (nIndex){
		case OMX_IndexParamAudioPortFormat:
		{
			OMX_AUDIO_PARAM_PORTFORMATTYPE *pFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)pPortParam;
			next = aPort->mPortFormatList.next;
			for (i = 0; i < pFormat->nIndex; i++){
				if (next != &aPort->mPortFormatList){
					next = next->next;
				}else{
					break;
				}
			}

			if (i == pFormat->nIndex){
				return OMX_ErrorNoMore;
			}else{
				item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);
				pFormat->eEncoding = item->eEncoding;
			}
		}
			break;

		default:
			return OMX_ErrorUnsupportedIndex;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortAudio_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
    OMX_DEMUXER_AVFRAME *avframe;
    MagOmxPort hPort;
    OMX_HANDLETYPE component;

    if (pBufHeader->pAppPrivate){
        hPort = ooc_cast(port, MagOmxPort);
        component = hPort->getAttachedComponent(hPort);
        avframe = (OMX_DEMUXER_AVFRAME *)pBufHeader->pAppPrivate;
        if(avframe->releaseFrame)
            avframe->releaseFrame(component, avframe);
    }
    return OMX_ErrorNone;
}

/*Member functions*/
static void MagOmxPortAudio_addFormat(MagOmxPortAudio hPort, MagOMX_Audio_PortFormat_t *pFormat){
	MagOMX_Audio_PortFormat_t *format;

	format = (MagOMX_Audio_PortFormat_t *)mag_mallocz(sizeof(MagOMX_Audio_PortFormat_t));
	INIT_LIST(&format->node);
    format->eEncoding         = pFormat->eEncoding;
    list_add_tail(&format->node, &hPort->mPortFormatList);
}

/*Class Constructor/Destructor*/
static void MagOmxPortAudio_initialize(Class this){
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_MagOmxPortAudio_GetDomainType;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.SetPortSpecificDef  = virtual_MagOmxPortAudio_SetPortSpecificDef;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.GetPortSpecificDef  = virtual_MagOmxPortAudio_GetPortSpecificDef;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.SetParameter        = virtual_MagOmxPortAudio_SetParameter;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.GetParameter        = virtual_MagOmxPortAudio_GetParameter;

    MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer   = virtual_MagOmxPortAudio_ProceedReturnedBuffer;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 */
static void MagOmxPortAudio_constructor(MagOmxPortAudio thiz, const void *params){
	MagOMX_Audio_PortFormat_t *pFormat;
	MagOMX_Audio_PortFormat_t *pInputFormat;
	MagOmxPort_Constructor_Param_t *lparam;

    MAG_ASSERT(ooc_isInitialized(MagOmxPortAudio));
    chain_constructor(MagOmxPortAudio, thiz, params);
    lparam  = (MagOmxPort_Constructor_Param_t *)(params);

    thiz->addFormat             = MagOmxPortAudio_addFormat;

    Mag_CreateMutex(&thiz->mhMutex);
    thiz->mParametersDB = createMagMiniDB(64);
    INIT_LIST(&thiz->mPortFormatList);

    thiz->mpPortDefinition = (OMX_AUDIO_PORTDEFINITIONTYPE *)mag_mallocz(sizeof(OMX_AUDIO_PORTDEFINITIONTYPE));
    
    if (lparam->formatStruct){
	    pFormat = (MagOMX_Audio_PortFormat_t *)mag_mallocz(sizeof(MagOMX_Audio_PortFormat_t));
	    pInputFormat = (MagOMX_Audio_PortFormat_t *)(lparam->formatStruct);
	    INIT_LIST(&pFormat->node);
	    pFormat->eEncoding         = pInputFormat->eEncoding;
	    list_add_tail(&pFormat->node, &thiz->mPortFormatList);
	}
}

static void MagOmxPortAudio_destructor(MagOmxPortAudio thiz, MagOmxPortAudioVtable vtab){
	List_t *next;
	MagOMX_Audio_PortFormat_t *item = NULL;

    AGILE_LOGV("Enter!");

    mag_freep((void **)&thiz->mpPortDefinition);

    next = thiz->mPortFormatList.next;
    while (next != &thiz->mPortFormatList){
    	item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);
    	list_del(next);
    	mag_free(item);
    	next = thiz->mPortFormatList.next;
    }
    
    destroyMagMiniDB(&thiz->mParametersDB);
    Mag_DestroyMutex(&thiz->mhMutex);
}