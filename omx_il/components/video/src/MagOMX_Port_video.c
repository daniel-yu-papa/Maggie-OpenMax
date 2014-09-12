#include "MagOMX_Port_video.h"

AllocateClass(MagOmxPortVideo, MagOmxPortImpl);

static MagOmxPortVideo getVideoPort(OMX_HANDLETYPE hPort){
    MagOmxPortVideo v;

    v = ooc_cast(hPort, MagOmxPortVideo);
    return v;
}

static OMX_PORTDOMAINTYPE virtual_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainVideo;
}

static OMX_ERRORTYPE virtual_SetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortVideo vPort;
	OMX_VIDEO_PORTDEFINITIONTYPE *pVideoDef = (OMX_VIDEO_PORTDEFINITIONTYPE *)pFormat;
	List_t *next;
    MagOMX_Video_PortFormat_t *item = NULL;
    
	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	vPort = getVideoPort(hPort);

	Mag_AcquireMutex(vPort->mhMutex);
	next = vPort->mPortFormatList.next;
	if (next != &vPort->mPortFormatList)
	    item = (MagOMX_Video_PortFormat_t *)list_entry(next, MagOMX_Video_PortFormat_t, node);

    if (item){
    	item->xFramerate = pVideoDef->xFramerate;
    	item->eCompressionFormat = pVideoDef->eCompressionFormat;
    	item->eColorFormat = pVideoDef->eColorFormat;
    }

	memcpy(vPort->mpPortDefinition, pVideoDef, sizeof(OMX_VIDEO_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(vPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_GetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortVideo vPort;
	OMX_VIDEO_PORTDEFINITIONTYPE *pVideoDef = (OMX_VIDEO_PORTDEFINITIONTYPE *)pFormat;
	List_t *next;
    MagOMX_Video_PortFormat_t *item = NULL;

	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	vPort = getVideoPort(hPort);

	Mag_AcquireMutex(vPort->mhMutex);
	next = vPort->mPortFormatList.next;
	if (next != &vPort->mPortFormatList)
    	item = (MagOMX_Video_PortFormat_t *)list_entry(next, MagOMX_Video_PortFormat_t, node);

	memcpy(pVideoDef, vPort->mpPortDefinition, sizeof(OMX_VIDEO_PORTDEFINITIONTYPE));
    if (item){
    	pVideoDef->xFramerate = item->xFramerate;
    	pVideoDef->eCompressionFormat = item->eCompressionFormat;
    	pVideoDef->eColorFormat = item->eColorFormat;
    }
	Mag_ReleaseMutex(vPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPortVideo vPort;
	OMX_U32 i;
	List_t *next;
	MagOMX_Video_PortFormat_t *item = NULL;

	vPort = getVideoPort(hPort);

	switch (nIndex){
		case OMX_IndexParamVideoPortFormat:
		{
			OMX_VIDEO_PARAM_PORTFORMATTYPE *pFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pPortParam;
			next = vPort->mPortFormatList.next;
			for (i = 0; i < pFormat->nIndex; i++){
				if (next != &vPort->mPortFormatList){
					next = next->next;
				}else{
					break;
				}
			}

			if (i == pFormat->nIndex){
				return OMX_ErrorNoMore;
			}else{
				item = (MagOMX_Video_PortFormat_t *)list_entry(next, MagOMX_Video_PortFormat_t, node);
				pFormat->xFramerate = item->xFramerate;
		    	pFormat->eCompressionFormat = item->eCompressionFormat;
		    	pFormat->eColorFormat = item->eColorFormat;
			}
		}
			break;

		default:
			return OMX_ErrorUnsupportedIndex;
	}
	return OMX_ErrorNone;
}

/*Member functions*/
static void MagOmxPortVideo_addFormat(MagOmxPortVideo hPort, MagOMX_Video_PortFormat_t *pFormat){
	MagOMX_Video_PortFormat_t *format;

	format = (MagOMX_Video_PortFormat_t *)mag_mallocz(sizeof(MagOMX_Video_PortFormat_t));
	INIT_LIST(&format->node);
    format->xFramerate         = pFormat->xFramerate;
    format->eCompressionFormat = pFormat->eCompressionFormat;
    format->eColorFormat       = pFormat->eColorFormat;
    list_add_tail(&format->node, &hPort->mPortFormatList);
}

/*Class Constructor/Destructor*/
static void MagOmxPortVideo_initialize(Class this){
	AGILE_LOGV("Enter!");

	MagOmxPortVideoVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_GetDomainType;
	MagOmxPortVideoVtableInstance.MagOmxPortImpl.MagOmxPort.SetPortSpecificDef  = virtual_SetPortSpecificDef;
	MagOmxPortVideoVtableInstance.MagOmxPortImpl.MagOmxPort.GetPortSpecificDef  = virtual_GetPortSpecificDef;
	MagOmxPortVideoVtableInstance.MagOmxPortImpl.MagOmxPort.SetParameter        = virtual_SetParameter;
	MagOmxPortVideoVtableInstance.MagOmxPortImpl.MagOmxPort.GetParameter        = virtual_GetParameter;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 */
static void MagOmxPortVideo_constructor(MagOmxPortVideo thiz, const void *params){
	MagOMX_Video_PortFormat_t *pFormat;
	MagOMX_Video_PortFormat_t *pInputFormat;
	MagOmxPort_Constructor_Param_t *lparam;

	AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxPortVideo));
    chain_constructor(MagOmxPortVideo, thiz, params);
    lparam  = (MagOmxPort_Constructor_Param_t *)(params);

    thiz->addFormat             = MagOmxPortVideo_addFormat;

    Mag_CreateMutex(&thiz->mhMutex);
    thiz->mParametersDB = createMagMiniDB(64);
    INIT_LIST(&thiz->mPortFormatList);

    thiz->mpPortDefinition = (OMX_VIDEO_PORTDEFINITIONTYPE *)mag_mallocz(sizeof(OMX_VIDEO_PORTDEFINITIONTYPE));
    
    if (lparam->formatStruct){
	    pFormat = (MagOMX_Video_PortFormat_t *)mag_mallocz(sizeof(MagOMX_Video_PortFormat_t));
	    pInputFormat = (MagOMX_Video_PortFormat_t *)(lparam->formatStruct);
	    INIT_LIST(&pFormat->node);
	    pFormat->xFramerate         = pInputFormat->xFramerate;
	    pFormat->eCompressionFormat = pInputFormat->eCompressionFormat;
	    pFormat->eColorFormat       = pInputFormat->eColorFormat;
	    list_add_tail(&pFormat->node, &thiz->mPortFormatList);
	}
}

static void MagOmxPortVideo_destructor(MagOmxPortVideo thiz, MagOmxPortVideoVtable vtab){
    List_t *next;
	MagOMX_Video_PortFormat_t *item = NULL;

    AGILE_LOGV("Enter!");

    mag_free(thiz->mpPortDefinition);

    next = thiz->mPortFormatList.next;
    while (next != &thiz->mPortFormatList){
    	item = (MagOMX_Video_PortFormat_t *)list_entry(next, MagOMX_Video_PortFormat_t, node);
    	list_del(next);
    	mag_free(item);
    	next = thiz->mPortFormatList.next;
    }

    destroyMagMiniDB(thiz->mParametersDB);
    Mag_DestroyMutex(thiz->mhMutex);
}