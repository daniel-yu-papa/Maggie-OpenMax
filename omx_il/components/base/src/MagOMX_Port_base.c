#include "MagOMX_Port_base.h"

AllocateClass(MagOmxPort, Base);

static void checkPortParameters(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){
    if (setDef->nBufferCountActual < setDef->nBufferCountMin){
        PORT_LOGE(hPort, "the BufferCountActual(%d) is less than the BufferCountMin(%d). Ignore the setting!!", 
                    setDef->nBufferCountActual, 
                    setDef->nBufferCountMin);
        setDef->nBufferCountActual = hPort->mpPortDefinition->nBufferCountActual;
    }
}

/*Member Functions*/
static OMX_U8 *MagOmxPort_getPortName(MagOmxPort hPort){
    return hPort->mPortName;
}

static OMX_ERRORTYPE MagOmxPort_getPortDefinition(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    void *portDef = NULL;
    OMX_PORTDOMAINTYPE domain;

    Mag_AcquireMutex(hPort->mhMutex);
    memcpy(getDef, hPort->mpPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if (MagOmxPortVirtual(hPort)->GetPortSpecificDef){
        domain = MagOmxPortVirtual(hPort)->GetDomainType(hPort);
        if (domain == OMX_PortDomainAudio){
            portDef = &getDef->format.audio;
        }else if (domain == OMX_PortDomainVideo){
            portDef = &getDef->format.video;
        }else if (domain == OMX_PortDomainImage){
            portDef = &getDef->format.image;
        }else if (domain == OMX_PortDomainOther){
            portDef = &getDef->format.other;
        }else{
            PORT_LOGE(hPort, "invalid port domain: %d", domain);
            ret = OMX_ErrorPortsNotCompatible;
        }

        hPort->mpPortDefinition->eDomain = domain;
        getDef->eDomain = domain;

        if (portDef)
            MagOmxPortVirtual(hPort)->GetPortSpecificDef(hPort, portDef);
    }else{
        PORT_LOGE(hPort, "The pure virtual function (SetPortSpecificDef) is not overrided!");
        ret = OMX_ErrorUndefined;
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return ret;
}

static OMX_ERRORTYPE MagOmxPort_setPortDefinition(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    void *portDef = NULL;
    OMX_PORTDOMAINTYPE domain;

    domain = MagOmxPortVirtual(hPort)->GetDomainType(hPort);

    if (domain != setDef->eDomain){
        PORT_LOGE(hPort, "Port domain(%d) mismatches Set domain(%d)", domain, setDef->eDomain);
        return OMX_ErrorUndefined;
    }

    Mag_AcquireMutex(hPort->mhMutex);
    checkPortParameters(hPort, setDef);
    memcpy(hPort->mpPortDefinition, setDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    if (MagOmxPortVirtual(hPort)->SetPortSpecificDef){
        if (domain == OMX_PortDomainAudio){
            portDef = &setDef->format.audio;
        }else if (domain == OMX_PortDomainVideo){
            portDef = &setDef->format.video;
        }else if (domain == OMX_PortDomainImage){
            portDef = &setDef->format.image;
        }else if (domain == OMX_PortDomainOther){
            portDef = &setDef->format.other;
        }else{
            PORT_LOGE(hPort, "invalid port domain: %d", domain);
            ret = OMX_ErrorPortsNotCompatible;
        }

        if (portDef)
            MagOmxPortVirtual(hPort)->SetPortSpecificDef(hPort, portDef);
    }else{
        PORT_LOGE(hPort, "The pure virtual function (SetPortSpecificDef) is not overrided!");
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return ret;
}


static OMX_U32  MagOmxPort_getDef_BufferCountActual(MagOmxPort root){
    return root->mpPortDefinition->nBufferCountActual;
}

static OMX_U32  MagOmxPort_getDef_BufferSize(MagOmxPort root){
    return root->mpPortDefinition->nBufferSize;
}

static OMX_BOOL MagOmxPort_getDef_Populated(MagOmxPort root){
    return root->mpPortDefinition->bPopulated;
}

static OMX_BOOL MagOmxPort_getDef_Enabled(MagOmxPort root){
    return root->mpPortDefinition->bEnabled;
}

static void MagOmxPort_setDef_BufferCountActual(MagOmxPort root, OMX_U32 cnt){
    Mag_AcquireMutex(root->mhMutex);
    root->mpPortDefinition->nBufferCountActual = cnt;
    Mag_ReleaseMutex(root->mhMutex);
}

static void MagOmxPort_setDef_BufferSize(MagOmxPort root, OMX_U32 bufSize){
    Mag_AcquireMutex(root->mhMutex);
    root->mpPortDefinition->nBufferSize = bufSize;
    Mag_ReleaseMutex(root->mhMutex);
}

static void MagOmxPort_setDef_Populated(MagOmxPort root, OMX_BOOL flag){
    Mag_AcquireMutex(root->mhMutex);
    root->mpPortDefinition->bPopulated = flag;
    Mag_ReleaseMutex(root->mhMutex);
}

static void MagOmxPort_setDef_Enabled(MagOmxPort root, OMX_BOOL flag){
    Mag_AcquireMutex(root->mhMutex);
    root->mpPortDefinition->bEnabled = flag;
    Mag_ReleaseMutex(root->mhMutex);
}

static void MagOmxPort_setParameter(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 value){
    Mag_AcquireMutex(hPort->mhParamMutex);
    
    switch (nIndex){
        case OMX_IndexParamNumAvailableStreams:
            hPort->mParametersDB->setUInt32(hPort->mParametersDB, "NumAvailableStreams", value);
            break;
            
        case OMX_IndexParamActiveStream:
            hPort->mParametersDB->setUInt32(hPort->mParametersDB, "ActiveStream", value);
            break;

        case OMX_IndexConfigTunneledPortStatus:
            hPort->mParametersDB->setUInt32(hPort->mParametersDB, "TunneledPortStatus", value);
            break;
        
        case OMX_IndexParamCompBufferSupplier:
            hPort->mBufferSupplier = (OMX_BUFFERSUPPLIERTYPE)value;
            break;

        default:
            PORT_LOGE(hPort, "unsupported parameter: %s", OmxParameter2String(nIndex));
            break;
    }
    
    Mag_ReleaseMutex(hPort->mhParamMutex);
}

static OMX_ERRORTYPE MagOmxPort_getParameter(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 *pValue){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    Mag_AcquireMutex(hPort->mhParamMutex);
    switch (nIndex){
        case OMX_IndexParamNumAvailableStreams:
            if (!hPort->mParametersDB->findUInt32(hPort->mParametersDB, "NumAvailableStreams", pValue)){
                PORT_LOGE(hPort, "failed to find the parameter: NumAvailableStreams");
                ret = OMX_ErrorUnsupportedSetting;
            }
            break;

        case OMX_IndexParamActiveStream:
            if (!hPort->mParametersDB->findUInt32(hPort->mParametersDB, "ActiveStream", pValue)){
                PORT_LOGE(hPort, "failed to find the parameter: ActiveStream");
                ret = OMX_ErrorUnsupportedSetting;
            }
            break;

        case OMX_IndexConfigTunneledPortStatus:
            if (!hPort->mParametersDB->findUInt32(hPort->mParametersDB, "TunneledPortStatus", pValue)){
                PORT_LOGE(hPort, "failed to find the parameter: TunneledPortStatus");
                ret = OMX_ErrorUnsupportedSetting;
            }
            break;
        
        case OMX_IndexParamCompBufferSupplier:
            *pValue = (OMX_U32)hPort->mBufferSupplier;
            break;

        default:
            PORT_LOGE(hPort, "unsupported parameter: %s", OmxParameter2String(nIndex));
            ret = OMX_ErrorUnsupportedSetting;
            break;
    }
    Mag_ReleaseMutex(hPort->mhParamMutex);

    return ret;
}

static OMX_BOOL MagOmxPort_isInputPort(MagOmxPort hPort){
    if (hPort->mpPortDefinition->eDir == OMX_DirInput)
        return OMX_TRUE;
    else
        return OMX_FALSE;
}

static OMX_BOOL MagOmxPort_isBufferSupplier(MagOmxPort hPort){
    if ((hPort->mBufferSupplier == OMX_BufferSupplyInput) ||
        (hPort->mBufferSupplier == OMX_BufferSupplyOutput))
        return OMX_TRUE;
    else
        return OMX_FALSE;
}

static OMX_BOOL MagOmxPort_isTunneled(MagOmxPort hPort){
    return hPort->mIsTunneled;
}

static void     MagOmxPort_setTunneledFlag(MagOmxPort hPort, OMX_BOOL setting){
    hPort->mIsTunneled = setting;
}

static OMX_U32  MagOmxPort_getPortIndex(MagOmxPort hPort){
    return hPort->mpPortDefinition->nPortIndex;
}

static void     MagOmxPort_resetBufferSupplier(MagOmxPort root){
    root->mBufferSupplier = root->mInitialBufferSupplier;
}

static MagOmxPort_BufferPolicy_t MagOmxPort_getBufferPolicy(MagOmxPort root){
    return root->mBufferPolicy;
}

static void     MagOmxPort_setBufferPolicy(MagOmxPort root, MagOmxPort_BufferPolicy_t policy){
    root->mBufferPolicy = policy;
}

static MagOmxPort_State_t MagOmxPort_getState(MagOmxPort root){
    return root->mState;
}

static void MagOmxPort_setState(MagOmxPort root, MagOmxPort_State_t st){
    root->mState = st;
}

static void MagOmxPort_setAttachedComponent(MagOmxPort root, OMX_HANDLETYPE comp){
    root->mAttachedComponent = comp;
}


static OMX_HANDLETYPE MagOmxPort_getAttachedComponent(MagOmxPort root){
    return root->mAttachedComponent;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_initialize(Class this){
    AGILE_LOGV("Enter!");

    MagOmxPortVtableInstance.Enable                = NULL;
    MagOmxPortVtableInstance.Disable               = NULL;
    MagOmxPortVtableInstance.Run                   = NULL;
    MagOmxPortVtableInstance.Flush                 = NULL;
    MagOmxPortVtableInstance.Pause                 = NULL;
    MagOmxPortVtableInstance.Resume                = NULL;
    MagOmxPortVtableInstance.MarkBuffer            = NULL;
    MagOmxPortVtableInstance.UseBuffer             = NULL;
    MagOmxPortVtableInstance.AllocateBuffer        = NULL;
    MagOmxPortVtableInstance.AllocateTunnelBuffer  = NULL;
    MagOmxPortVtableInstance.FreeBuffer            = NULL;
    MagOmxPortVtableInstance.FreeAllBuffers        = NULL;
    MagOmxPortVtableInstance.FreeTunnelBuffer      = NULL;
    MagOmxPortVtableInstance.EmptyThisBuffer       = NULL;
    MagOmxPortVtableInstance.FillThisBuffer        = NULL;
    MagOmxPortVtableInstance.SetupTunnel           = NULL;
    MagOmxPortVtableInstance.RegisterBufferHandler = NULL;
    MagOmxPortVtableInstance.SendEvent             = NULL;
    MagOmxPortVtableInstance.GetSharedBufferMsg    = NULL;
    MagOmxPortVtableInstance.GetOutputBuffer       = NULL;
    MagOmxPortVtableInstance.PostOutputBufferMsg   = NULL;
    MagOmxPortVtableInstance.GetDomainType         = NULL;
    MagOmxPortVtableInstance.SetPortSpecificDef    = NULL;
    MagOmxPortVtableInstance.GetPortSpecificDef    = NULL;
    MagOmxPortVtableInstance.SetParameter          = NULL;
    MagOmxPortVtableInstance.GetParameter          = NULL;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 */
static void MagOmxPort_constructor(MagOmxPort thiz, const void *params){
    MagOmxPort_Constructor_Param_t *lparam;

    AGILE_LOGV("Enter!");

    MAG_ASSERT(ooc_isInitialized(MagOmxPort));
    chain_constructor(MagOmxPort, thiz, params);
    
    lparam  = (MagOmxPort_Constructor_Param_t *)(params);
    
    thiz->getPortName              = MagOmxPort_getPortName;
    thiz->getPortDefinition        = MagOmxPort_getPortDefinition;
    thiz->setPortDefinition        = MagOmxPort_setPortDefinition;
    thiz->setParameter             = MagOmxPort_setParameter;
    thiz->getParameter             = MagOmxPort_getParameter;

    thiz->isInputPort              = MagOmxPort_isInputPort;
    thiz->isBufferSupplier         = MagOmxPort_isBufferSupplier;
    thiz->isTunneled               = MagOmxPort_isTunneled;
    thiz->getPortIndex             = MagOmxPort_getPortIndex;

    thiz->getDef_BufferCountActual = MagOmxPort_getDef_BufferCountActual;
    thiz->getDef_BufferSize        = MagOmxPort_getDef_BufferSize;
    thiz->getDef_Populated         = MagOmxPort_getDef_Populated;
    thiz->getDef_Enabled           = MagOmxPort_getDef_Enabled;
    thiz->getBufferPolicy          = MagOmxPort_getBufferPolicy;
    thiz->getState                 = MagOmxPort_getState;

    thiz->setTunneledFlag          = MagOmxPort_setTunneledFlag;
    thiz->setDef_BufferCountActual = MagOmxPort_setDef_BufferCountActual;
    thiz->setDef_BufferSize        = MagOmxPort_setDef_BufferSize;
    thiz->setDef_Populated         = MagOmxPort_setDef_Populated;
    thiz->setDef_Enabled           = MagOmxPort_setDef_Enabled;
    thiz->setBufferPolicy          = MagOmxPort_setBufferPolicy;
    thiz->setState                 = MagOmxPort_setState;
    thiz->resetBufferSupplier      = MagOmxPort_resetBufferSupplier;

    thiz->setAttachedComponent     = MagOmxPort_setAttachedComponent;
    thiz->getAttachedComponent     = MagOmxPort_getAttachedComponent;

    thiz->mpPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)mag_mallocz(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    MAG_ASSERT(thiz->mpPortDefinition);

    initHeader(thiz->mpPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

    thiz->mpPortDefinition->nPortIndex         = lparam->portIndex;
    thiz->mpPortDefinition->eDir               = lparam->isInput ? OMX_DirInput : OMX_DirOutput;
    thiz->mpPortDefinition->nBufferCountActual = kPortBuffersMinNum;
    thiz->mpPortDefinition->nBufferCountMin    = kPortBuffersMinNum;
    thiz->mpPortDefinition->nBufferSize        = kPortBufferSize;
    thiz->mpPortDefinition->bEnabled           = OMX_TRUE;
    thiz->mpPortDefinition->bPopulated         = OMX_FALSE;
    thiz->mpPortDefinition->eDomain            = OMX_PortDomainMax;
    thiz->mpPortDefinition->bBuffersContiguous = OMX_FALSE;
    thiz->mpPortDefinition->nBufferAlignment   = 4; /*4 bytes*/

    thiz->mBufferSupplier        = lparam->bufSupplier;
    thiz->mInitialBufferSupplier = thiz->mBufferSupplier;
    thiz->mIsTunneled            = OMX_FALSE;
    thiz->mBufferPolicy          = kNoneSharedBuffer;
    thiz->mState                 = kPort_State_Stopped;
    thiz->mAttachedComponent     = NULL;

    strncpy((char *)thiz->mPortName, (char *)lparam->name, 32);

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhParamMutex);
    
    thiz->mParametersDB = createMagMiniDB(64);
}

static void MagOmxPort_destructor(MagOmxPort thiz, MagOmxPortVtable vtab){
    PORT_LOGV(thiz, "Enter!");

    mag_freep((void **)&thiz->mpPortDefinition);
    Mag_DestroyMutex(&thiz->mhMutex);
    Mag_DestroyMutex(&thiz->mhParamMutex);
    destroyMagMiniDB(&thiz->mParametersDB);
}

