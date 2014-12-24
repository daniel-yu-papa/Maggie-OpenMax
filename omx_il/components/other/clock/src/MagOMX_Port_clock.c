#include "MagOMX_Port_clock.h"
#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompClk"

AllocateClass(MagOmxPortClock, MagOmxPortImpl);

static MagOmxPortClock getClockPort(OMX_HANDLETYPE hPort){
    MagOmxPortClock v;

    v = ooc_cast(hPort, MagOmxPortClock);
    return v;
}

static OMX_PORTDOMAINTYPE virtual_MagOmxPortClock_GetDomainType(OMX_HANDLETYPE hPort){
	return OMX_PortDomainOther_Clock;
}

static OMX_ERRORTYPE virtual_MagOmxPortClock_SetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPort root;
	MagOmxComponentImpl hComp;
	MagOmxPortImpl portImpl;
	MagOmxComponent compRoot;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	root = ooc_cast(hPort, MagOmxPort);
	hComp = (MagOmxComponentImpl)(root->getAttachedComponent(root));	
	switch (nIndex){
		case OMX_IndexConfigTimeClientStartTime:
		{
			OMX_TIME_CONFIG_TIMESTAMPTYPE        *pStartTime;

			if (root->isInputPort(root)){ /*input port*/
				portImpl = ooc_cast(hPort, MagOmxPortImpl);
				pStartTime = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pPortParam;
				pStartTime->nPortIndex = portImpl->mTunneledPortIndex;
				compRoot = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);
				ret = MagOmxComponentVirtual(compRoot)->SetConfig(portImpl->mTunneledComponent, 
					                                              OMX_IndexConfigTimeClientStartTime, 
					                                              pStartTime);
			}else{ /*output port*/
				PORT_LOGV(root, "parameter OMX_IndexConfigTimeClientStartTime. clock port notify the clock component");
				ret = hComp->notify(hComp, MagOMX_Component_Notify_StartTime, pPortParam);
			}
		}
			break;

		case OMX_IndexConfigTimeMediaTimeRequest:
		{	
			OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE *data;

			if (root->isInputPort(root)){ /*input port*/
				portImpl = ooc_cast(hPort, MagOmxPortImpl);
				data = (OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE *)pPortParam;
				data->nPortIndex = portImpl->mTunneledPortIndex;
				compRoot = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);
				ret = MagOmxComponentVirtual(compRoot)->SetConfig(portImpl->mTunneledComponent, 
					                                              OMX_IndexConfigTimeMediaTimeRequest, 
					                                              data);
			}else{ /*output port*/
				ret = hComp->notify(hComp, MagOMX_Component_Notify_MediaTimeRequest, pPortParam);
			}
		}
			break;

		case OMX_IndexConfigTimeUpdate:
		{
			OMX_TIME_MEDIATIMETYPE *data;

			if (!root->isInputPort(root)){ /*output port*/
				portImpl = ooc_cast(hPort, MagOmxPortImpl);
				data = (OMX_TIME_MEDIATIMETYPE *)pPortParam;
				compRoot = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);
				ret = MagOmxComponentVirtual(compRoot)->SetConfig(portImpl->mTunneledComponent, 
					                                              OMX_IndexConfigTimeUpdate, 
					                                              data);
			}else{ /*input port*/
				PORT_LOGE(root, "Invalid parameter OMX_IndexConfigTimeUpdate setting to the input clock port!");
			}
		}
			break;

		case OMX_IndexConfigTimeCurrentReference:
			if (!root->isInputPort(root)){ /*output port*/
				ret = hComp->notify(hComp, MagOMX_Component_Notify_ReferenceTimeUpdate, pPortParam);
			}else{
				PORT_LOGE(root, "To setConfig(OMX_IndexConfigTimeCurrentReference) on input port is Illegal!");
				ret = OMX_ErrorUnsupportedSetting;
			}
			break;

		default:
			return OMX_ErrorUnsupportedIndex;
	}

	return ret;
}

static OMX_ERRORTYPE virtual_MagOmxPortClock_GetParameter(OMX_HANDLETYPE hPort, OMX_INDEXTYPE nIndex, OMX_PTR pPortParam){
	MagOmxPort root;
	MagOmxPortImpl portImpl;
	MagOmxComponent compRoot;
	MagOmxComponentImpl hComp;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	root = ooc_cast(hPort, MagOmxPort);
	portImpl = ooc_cast(hPort, MagOmxPortImpl);
	hComp = (MagOmxComponentImpl)(root->getAttachedComponent(root));
	switch (nIndex){
		case OMX_IndexConfigTimeRenderingDelay:
		{	
			OMX_TIME_CONFIG_RENDERINGDELAYTYPE *data = (OMX_TIME_CONFIG_RENDERINGDELAYTYPE *)pPortParam;
			if (!root->isInputPort(root)){ /*output port*/
				data->nPortIndex = portImpl->mTunneledPortIndex;
				compRoot = ooc_cast(portImpl->mTunneledComponent, MagOmxComponent);
				ret = MagOmxComponentVirtual(compRoot)->GetConfig(portImpl->mTunneledComponent, OMX_IndexConfigTimeRenderingDelay, data);
			}else{ /*input port*/
				if (MagOmxComponentImplVirtual(hComp)->MagOMX_GetRenderDelay){
	                ret = MagOmxComponentImplVirtual(hComp)->MagOMX_GetRenderDelay(hComp, &data->nRenderingDelay);
	            }else{
	                PORT_LOGE(root, "The pure virtual function MagOMX_GetRenderDelay() is not overrided!");
	                ret = OMX_ErrorNotImplemented;
	            }
			}
		}
			break;

		default:
			return OMX_ErrorUnsupportedIndex;
	}
	return ret;
}

static OMX_ERRORTYPE virtual_MagOmxPortClock_SetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortClock clkPort;
	OMX_OTHER_PORTDEFINITIONTYPE *pClkDef = (OMX_OTHER_PORTDEFINITIONTYPE *)pFormat;
    
	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	clkPort = ooc_cast(hPort, MagOmxPortClock);

	Mag_AcquireMutex(clkPort->mhMutex);
	memcpy(&clkPort->mPortDefinition, pClkDef, sizeof(OMX_OTHER_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(clkPort->mhMutex);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxPortClock_GetPortSpecificDef(OMX_HANDLETYPE hPort, void *pFormat){
	MagOmxPortClock clkPort;
	OMX_OTHER_PORTDEFINITIONTYPE *pClkDef = (OMX_OTHER_PORTDEFINITIONTYPE *)pFormat;

	if ((hPort == NULL) || (pFormat == NULL)){
		return OMX_ErrorBadParameter;
	}

	clkPort = ooc_cast(hPort, MagOmxPortClock);

	Mag_AcquireMutex(clkPort->mhMutex);
	memcpy(pClkDef, &clkPort->mPortDefinition, sizeof(OMX_OTHER_PORTDEFINITIONTYPE));
	Mag_ReleaseMutex(clkPort->mhMutex);

	return OMX_ErrorNone;
}


/*Class Constructor/Destructor*/
static void MagOmxPortClock_initialize(Class this){
	MagOmxPortClockVtableInstance.MagOmxPortImpl.MagOmxPort.GetDomainType       = virtual_MagOmxPortClock_GetDomainType;
	MagOmxPortClockVtableInstance.MagOmxPortImpl.MagOmxPort.SetPortSpecificDef  = virtual_MagOmxPortClock_SetPortSpecificDef;
	MagOmxPortClockVtableInstance.MagOmxPortImpl.MagOmxPort.GetPortSpecificDef  = virtual_MagOmxPortClock_GetPortSpecificDef;
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
	MagOmxPort root;

    MAG_ASSERT(ooc_isInitialized(MagOmxPortClock));
    chain_constructor(MagOmxPortClock, thiz, params);

    root = ooc_cast(thiz, MagOmxPort);

    Mag_CreateMutex(&thiz->mhMutex);
    thiz->mPortDefinition.eFormat = OMX_OTHER_FormatTime;
    root->setDef_Populated(root, OMX_TRUE);
}

static void MagOmxPortClock_destructor(MagOmxPortClock thiz, MagOmxPortClockVtable vtab){
    AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
}