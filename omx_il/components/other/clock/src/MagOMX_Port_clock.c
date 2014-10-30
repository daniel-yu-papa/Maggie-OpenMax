#include "MagOMX_Port_clock.h"

AllocateClass(MagOmxPortClock, MagOmxPortImpl);

static MagOmxPortClock getClockPort(OMX_HANDLETYPE hPort){
    MagOmxPortClock v;

    v = ooc_cast(hPort, MagOmxPortClock);
    return v;
}

static OMX_PORTDOMAINTYPE virtual_MagOmxPortClock_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainOther;
}

static OMX_ERRORTYPE virtual_MagOmxPortClock_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPort root;
	MagOmxComponentImpl hComp;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	root = ooc_cast(hPort, MagOmxPort);
	hComp = (MagOmxComponentImpl)(root->getAttachedComponent(root));	
	switch (nIndex){
		case OMX_IndexConfigTimeClientStartTime:
		{
			ret = hComp->notify(hComp, MagOMX_Component_Notify_StartTime, pPortParam);
		}
			break;

		case OMX_IndexConfigTimeMediaTimeRequest:
		{
			ret = hComp->notify(hComp, MagOMX_Component_Notify_MediaTimeRequest, pPortParam);
		}
			break;

		case OMX_IndexConfigTimeUpdate:
		{
			MagOmxPortImpl portImpl;
			MagOmxComponent compRoot;
			OMX_TIME_MEDIATIMETYPE *data;

			portImpl = ooc_cast(hPort, MagOmxPortImpl);
			data = (OMX_TIME_MEDIATIMETYPE *)pPortParam;
			data->nPortIndex = portImpl->mTunneledPortIndex;
			compRoot = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);
			ret = MagOmxComponentVirtual(compRoot)->SetConfig(portImpl->mTunneledComponent, OMX_IndexConfigTimeUpdate, data);
		}
			break;

		case OMX_IndexConfigTimeCurrentReference:
			ret = hComp->notify(hComp, MagOMX_Component_Notify_ReferenceTimeUpdate, pPortParam);
			break;

		default:
			return OMX_ErrorUnsupportedIndex;
	}

	return ret;
}

static OMX_ERRORTYPE virtual_MagOmxPortClock_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPortImpl portImpl;
	MagOmxComponent compRoot;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	portImpl = ooc_cast(hPort, MagOmxPortImpl);
	switch (nIndex){
		case OMX_IndexConfigTimeRenderingDelay:
		{	
			OMX_TIME_CONFIG_RENDERINGDELAYTYPE *data = (OMX_TIME_CONFIG_RENDERINGDELAYTYPE *)pPortParam;
			data->nPortIndex = portImpl->mTunneledPortIndex;
			compRoot = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);
			ret = MagOmxComponentVirtual(compRoot)->GetConfig(portImpl->mTunneledComponent, OMX_IndexConfigTimeRenderingDelay, data);
		}
			break;

		default:
			return OMX_ErrorUnsupportedIndex;
	}
	return ret;
}


/*Class Constructor/Destructor*/
static void MagOmxPortClock_initialize(Class this){
	MagOmxPortClockVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_MagOmxPortClock_GetDomainType;
	MagOmxPortClockVtableInstance.MagOmxPortImpl.MagOmxPort.SetParameter        = virtual_MagOmxPortClock_SetParameter;
	MagOmxPortClockVtableInstance.MagOmxPortImpl.MagOmxPort.GetParameter        = virtual_MagOmxPortClock_GetParameter;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 */
static void MagOmxPortClock_constructor(MagOmxPortClock thiz, const void *params){
	MagOMX_Audio_PortFormat_t *pFormat;
	MagOMX_Audio_PortFormat_t *pInputFormat;

    MAG_ASSERT(ooc_isInitialized(MagOmxPortClock));
    chain_constructor(MagOmxPortClock, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
    thiz->mParametersDB = createMagMiniDB(64);
}

static void MagOmxPortClock_destructor(MagOmxPortClock thiz, MagOmxPortClockVtable vtab){
    AGILE_LOGV("Enter!");

    destroyMagMiniDB(&thiz->mParametersDB);
    Mag_DestroyMutex(&thiz->mhMutex);
}