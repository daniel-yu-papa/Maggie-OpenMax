#include "MagOMX_Port_base.h"

AllocateClass(MagOmxPort, Base);

static void checkPortParameters(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){
    if (setDef->nBufferCountActual < setDef->nBufferCountMin){
        AGILE_LOGE("the BufferCountActual(%d) is less than the BufferCountMin(%d). Ignore the setting!!", 
                    setDef->nBufferCountActual, 
                    setDef->nBufferCountMin);
        setDef->nBufferCountActual = hPort->mPortDefinition.nBufferCountActual;
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
    return root->mPortDefinition.nBufferCountActual;
}

static OMX_BOOL MagOmxPort_getDef_Populated(MagOmxPort root){
    return root->mPortDefinition.bPopulated;
}

static OMX_BOOL MagOmxPort_getDef_Enabled(MagOmxPort root){
    return root->mPortDefinition.bEnabled;
}

static void     MagOmxPort_setDef_Populated(MagOmxPort root, OMX_BOOL flag){
    Mag_AcquireMutex(root->mhMutex);
    root->mPortDefinition.bPopulated = flag;
    Mag_ReleaseMutex(root->mhMutex);
}

static void     MagOmxPort_setDef_Enabled(MagOmxPort root, OMX_BOOL flag){
    Mag_AcquireMutex(root->mhMutex);
    root->mPortDefinition.bEnabled = flag;
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
            break;
            
        default:
            AGILE_LOGE("unsupported parameter: %s", OmxParameter2String(nIndex));
            break;
    }
    
    Mag_ReleaseMutex(hPort->mhParamMutex);
}

static OMX_ERRORTYPE MagOmxPort_getParameter(MagOmxPort hPort, OMX_INDEXTYPE nIndex, OMX_U32 *pValue){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
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
            
        default:
            AGILE_LOGE("unsupported parameter: %s", OmxParameter2String(nIndex));
            ret = OMX_ErrorUnsupportedSetting;
            break;
    }
    
    return ret;
}

static OMX_BOOL MagOmxPort_isInputPort(MagOmxPort hPort){
    if (hPort->mPortDefinition.eDir == OMX_DirInput)
        return OMX_TRUE;
    else
        return OMX_FALSE;
}

static OMX_U32 MagOmxPort_getPortIndex(MagOmxPort hPort){
    return hPort->mPortDefinition.nPortIndex;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_initialize(Class this){
    MagOmxPortVtableInstance.enablePort      = NULL;
    MagOmxPortVtableInstance.disablePort     = NULL;
    MagOmxPortVtableInstance.flushPort       = NULL;
    MagOmxPortVtableInstance.markBuffer      = NULL;
    MagOmxPortVtableInstance.AllocateBuffer  = NULL;
    MagOmxPortVtableInstance.UseBuffer       = NULL;
    MagOmxPortVtableInstance.FreeBuffer      = NULL;
    MagOmxPortVtableInstance.EmptyThisBuffer = NULL;
    MagOmxPortVtableInstance.FillThisBuffer  = NULL;
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
    thiz->getPortIndex             = MagOmxPort_getPortIndex;
    thiz->getDef_BufferCountActual = MagOmxPort_getDef_BufferCountActual;
    thiz->getDef_Populated         = MagOmxPort_getDef_Populated;
    thiz->getDef_Enabled           = MagOmxPort_getDef_Enabled;
    thiz->setDef_Populated         = MagOmxPort_setDef_Populated;
    thiz->setDef_Enabled           = MagOmxPort_setDef_Enabled;
    
    memset(&thiz->mPortDefinition, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    initHeader((void *)&thiz->mPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    thiz->mPortDefinition.nPortIndex = lparam->portIndex;
    thiz->mPortDefinition.eDir       = lparam->isInput ? OMX_DirInput : OMX_DirOutput;
        
    thiz->mPortDefinition.nBufferCountActual = kPortBuffersMinNum;
    thiz->mPortDefinition.nBufferCountMin    = kPortBuffersMinNum;
    thiz->mPortDefinition.nBufferSize        = kPortBufferSize;
    thiz->mPortDefinition.bEnabled           = OMX_TRUE;
    thiz->mPortDefinition.bPopulated         = OMX_FALSE;
    thiz->mPortDefinition.bBuffersContiguous = OMX_FALSE;
    thiz->mPortDefinition.nBufferAlignment   = 4; /*4 bytes*/

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhParamMutex);
    
    thiz->mParametersDB = createMagMiniDB(64);
}

static void MagOmxPort_destructor(MagOmxPort thiz, MagOmxPortVtable vtab){
    AGILE_LOGV("Enter!");
    Mag_DestroyMutex(thiz->mhMutex);
}

