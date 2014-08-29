#include "MagOMX_Port_audio.h"

AllocateClass(MagOmxPortAudio, MagOmxPortImpl);

static MagOmxPortAudio getAudioPort(OMX_HANDLETYPE hPort){
    MagOmxPortAudio v;

    v = ooc_cast(hPort, MagOmxPortAudio);
    return v;
}

static OMX_PORTDOMAINTYPE virtual_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainAudio;
}

static OMX_ERRORTYPE virtual_SetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortAudio *aPort;
	OMX_AUDIO_PORTDEFINITIONTYPE *pAudioDef = (OMX_AUDIO_PORTDEFINITIONTYPE *)pFormat;
	List_t *next;
    MagOMX_Audio_PortFormat_t *item = NULL;
    
	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	aPort = getAudioPort(hPort);

	Mag_AcquireMutex(aPort->mhMutex);
	next = aPort->mPortFormatList.next;
    item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);

    if (item){
    	item->eEncoding = pAudioDef->eEncoding;
    }

	memcpy(aPort->mpPortDefinition, pAudioDef, sizeof(OMX_AUDIO_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(aPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_GetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortAudio *aPort;
	OMX_AUDIO_PORTDEFINITIONTYPE *pAudioDef = (OMX_AUDIO_PORTDEFINITIONTYPE *)pFormat;
	List_t *next;
    MagOMX_Audio_PortFormat_t *item = NULL;

	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	aPort = getAudioPort(hPort);

	Mag_AcquireMutex(aPort->mhMutex);
	next = aPort->mPortFormatList.next;
    item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);

	memcpy(pAudioDef, aPort->mpPortDefinition, sizeof(OMX_AUDIO_PORTDEFINITIONTYPE));
    if (item){
    	pAudioDef->eEncoding = item->eEncoding;
    }
	Mag_ReleaseMutex(aPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPortAudio aPort;
	OMX_U32 i;
	List_t *next;
	MagOMX_Audio_PortFormat_t *item = NULL;

	aPort = getAudioPort(hPort);

	swtich (nIndex){
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
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_GetDomainType;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.SetPortSpecificDef  = virtual_SetPortSpecificDef;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.GetPortSpecificDef  = virtual_GetPortSpecificDef;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.SetParameter        = virtual_SetParameter;
	MagOmxPortAudioVtableInstance.MagOmxPortImpl.MagOmxPort.GetParameter        = virtual_GetParameter;
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
    pFormat = (MagOMX_Audio_PortFormat_t *)mag_mallocz(sizeof(MagOMX_Audio_PortFormat_t));

    pInputFormat = (MagOMX_Audio_PortFormat_t *)(lparam->formatStruct);
    INIT_LIST(&pFormat->node);
    pFormat->eEncoding         = pInputFormat->eEncoding;
    list_add_tail(&pFormat->node, &thiz->mPortFormatList);
}

static void MagOmxPortAudio_destructor(MagOmxPortAudio thiz, MagOmxPortAudioVtable vtab){
	List_t *next;
	MagOMX_Audio_PortFormat_t *item = NULL;

    AGILE_LOGV("Enter!");

    mag_free(thiz->mpPortDefinition);

    next = thiz->mPortFormatList.next;
    while (next != &thiz->mPortFormatList){
    	item = (MagOMX_Audio_PortFormat_t *)list_entry(next, MagOMX_Audio_PortFormat_t, node);
    	list_del(next);
    	mag_free(item);
    	next = thiz->mPortFormatList.next;
    }
    
    destroyMagMiniDB(thiz->mParametersDB);
    Mag_DestroyMutex(thiz->mhMutex);
}