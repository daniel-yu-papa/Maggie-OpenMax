#include "MagOMX_Port_base.h"

AllocateClass(MagOmxPort, Base);

static void checkPortParameters(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){
    if (setDef->nBufferCountActual < setDef->nBufferCountMin){
        AGILE_LOGE("the BufferCountActual(%d) is less than the BufferCountMin(%d). Ignore the setting!!", 
                    setDef->nBufferCountActual, 
                    setDef->nBufferCountMin);
        setDef->nBufferCountActual = hPort->mpPortDefinition->nBufferCountActual;
    }
}

/*Member Functions*/
static void MagOmxPort_getPortDefinition(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef){
    Mag_AcquireMutex(hPort->mhMutex);
    memcpy(getDef, &hPort->mPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    Mag_ReleaseMutex(hPort->mhMutex);
}

static void MagOmxPort_setPortDefinition(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){
    Mag_AcquireMutex(hPort->mhMutex);
    checkPortParameters(hPort, setDef);
    memcpy(&hPort->mPortDefinition, setDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    Mag_ReleaseMutex(hPort->mhMutex);
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
    root->mpPortDefinition->nBufferCountActual = cnt;
}

static void MagOmxPort_setDef_BufferSize(MagOmxPort root, OMX_U32 bufSize){
    root->mpPortDefinition->nBufferSize = bufSize;
}

static void     MagOmxPort_setDef_Populated(MagOmxPort root, OMX_BOOL flag){
    Mag_AcquireMutex(root->mhMutex);
    root->mpPortDefinition->bPopulated = flag;
    MagOmxPortVirtual(root)->SendEvent(hPort, kBufferPopulatedEvt);
    Mag_ReleaseMutex(root->mhMutex);
}

static void     MagOmxPort_setDef_Enabled(MagOmxPort root, OMX_BOOL flag){
    Mag_AcquireMutex(root->mhMutex);
    root->mpPortDefinition->bEnabled = flag;
    Mag_ReleaseMutex(root->mhMutex);
}

static void     MagOmxPort_setParameter(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 value){
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
            MagOmxPortVirtual(hPort)->SendEvent(hPort, kTunneledPortStatusEvt);
            break;
        
        case OMX_IndexParamCompBufferSupplier:
            hPort->mBufferSupplier = (OMX_BUFFERSUPPLIERTYPE)value;
            break;

        default:
            AGILE_LOGE("unsupported parameter: %s", OmxParameter2String(nIndex));
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
                AGILE_LOGE("failed to find the parameter: NumAvailableStreams");
                ret = OMX_ErrorUnsupportedSetting;
            }
            break;

        case OMX_IndexParamActiveStream:
            if (!hPort->mParametersDB->findUInt32(hPort->mParametersDB, "ActiveStream", pValue)){
                AGILE_LOGE("failed to find the parameter: ActiveStream");
                ret = OMX_ErrorUnsupportedSetting;
            }
            break;

        case OMX_IndexConfigTunneledPortStatus:
            if (!hPort->mParametersDB->findUInt32(hPort->mParametersDB, "TunneledPortStatus", pValue)){
                AGILE_LOGE("failed to find the parameter: TunneledPortStatus");
                ret = OMX_ErrorUnsupportedSetting;
            }
            break;
        
        case OMX_IndexParamCompBufferSupplier:
            *pValue = (OMX_U32)hPort->mBufferSupplier;
            break;

        default:
            AGILE_LOGE("unsupported parameter: %s", OmxParameter2String(nIndex));
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

static OMX_U32 MagOmxPort_getPortIndex(MagOmxPort hPort){
    return hPort->mpPortDefinition->nPortIndex;
}

static void    MagOmxPort_resetBufferSupplier(MagOmxPort root){
    root->mBufferSupplier = root->mInitialBufferSupplier;
}

MagOmxPort_BufferPolicy_t MagOmxPort_getBufferPolicy(MagOmxPort root){
    return mBufferPolicy;
}

void MagOmxPort_setBufferPolicy(MagOmxPort root, MagOmxPort_BufferPolicy_t policy){
    mBufferPolicy = policy;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_initialize(Class this){
    MagOmxPortVtableInstance.Start                 = NULL;
    MagOmxPortVtableInstance.Stop                  = NULL;
    MagOmxPortVtableInstance.EnablePort            = NULL;
    MagOmxPortVtableInstance.DisablePort           = NULL;
    MagOmxPortVtableInstance.FlushPort             = NULL;
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
    MagOmxPortVtableInstance.GetSharedBufferMsg    = NULL;
}

/*
 * param[0] = nPortIndex;
 * param[1] = isInput;
 */
static void MagOmxPort_constructor(MagOmxPort thiz, const void *params){
    MagOmxPort_Constructor_Param_t *lparam;

    MAG_ASSERT(ooc_isInitialized(MagOmxPort));
    chain_constructor(MagOmxPort, thiz, params);
    
    lparam  = (MagOmxPort_Constructor_Param_t *)(params);
    
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

    thiz->setTunneledFlag          = MagOmxPort_setTunneledFlag;
    thiz->setDef_BufferCountActual = MagOmxPort_setDef_BufferCountActual;
    thiz->setDef_BufferSize        = MagOmxPort_setDef_BufferSize;
    thiz->setDef_Populated         = MagOmxPort_setDef_Populated;
    thiz->setDef_Enabled           = MagOmxPort_setDef_Enabled;
    thiz->resetBufferSupplier      = MagOmxPort_resetBufferSupplier;
    
    thiz->mpPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)mag_mallocz(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    MAG_ASSERT(thiz->mpPortDefinition);

    initHeader((void *)thiz->mpPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    thiz->mpPortDefinition->nPortIndex = lparam->portIndex;
    thiz->mpPortDefinition->eDir       = lparam->isInput ? OMX_DirInput : OMX_DirOutput;
    if ((lparam->isInput == OMX_DirInput) &&
        (lparam->bufSupplier == OMX_BufferSupplyOutput)){
        AGILE_LOGE("wrong buffer supplier: SupplyOutput on input port!");
        thiz->mBufferSupplier = OMX_BufferSupplyUnspecified;
    }else if ((lparam->isInput == OMX_DirOutput) &&
              (lparam->bufSupplier == OMX_BufferSupplyInput)){
        AGILE_LOGE("wrong buffer supplier: SupplyInput on output port!");
        thiz->mBufferSupplier = OMX_BufferSupplyUnspecified;
    }else{
        thiz->mBufferSupplier = lparam->bufSupplier;
    }
    thiz->mInitialBufferSupplier = thiz->mBufferSupplier;

    thiz->mpPortDefinition->nBufferCountActual = kPortBuffersMinNum;
    thiz->mpPortDefinition->nBufferCountMin    = kPortBuffersMinNum;
    thiz->mpPortDefinition->nBufferSize        = kPortBufferSize;
    thiz->mpPortDefinition->bEnabled           = OMX_TRUE;
    thiz->mpPortDefinition->bPopulated         = OMX_FALSE;
    thiz->mpPortDefinition->bBuffersContiguous = OMX_FALSE;
    thiz->mpPortDefinition->nBufferAlignment   = 4; /*4 bytes*/

    thiz->mIsTunneled  = OMX_FALSE;
    thiz->mBufferPolicy = kNoneSharedBuffer;

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhParamMutex);
    
    thiz->mParametersDB = createMagMiniDB(64);
}

static void MagOmxPort_destructor(MagOmxPort thiz, MagOmxPortVtable vtab){
    AGILE_LOGV("Enter!");

    mag_freep(&thiz->mpPortDefinition);
    Mag_DestroyMutex(thiz->mhMutex);
    Mag_DestroyMutex(thiz->mhParamMutex);
    destroyMagMiniDB(thiz->mParametersDB);
}

