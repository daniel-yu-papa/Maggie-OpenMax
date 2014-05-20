#include "MagOMX_Component_baseImpl.h"

#define LOOPER_NAME "MagOmxComponentBaseLooper"

AllocateClass(MagOmxComponentBase, MagOmxComponent);

static MagOmxComponentBase getBase(OMX_HANDLETYPE hComponent) {
    MagOmxComponentBase base;
    base = ooc_cast(hComponent, MagOmxComponentBase);
    return base;
}

static void onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponentBase base;
    OMX_U32 param;
    OMX_PTR cmd_data;
    OMX_ERRORTYPE ret;
    OMX_U32 cmd;
    MagOmxPort port;
    
    if (!msg){
        AGILE_LOGE("msg is NULL!");
        return;
    }
    
    base = getBase(priv);

    if (!msg->findUInt32(msg, "param", &param)){
        AGILE_LOGE("failed to find the param!");
        return;
    }

    if (!msg->findPointer(msg, "cmd_data", &cmd_data)){
        AGILE_LOGE("failed to find the cmd_data!");
        return;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentBase_CommandStateSet:
            {
            OMX_STATETYPE target_state = (OMX_STATETYPE)param;
            base->setState(priv, target_state);
            }
            break;

        case MagOmxComponentBase_CommandFlush:
            {
            base->flushPort(priv, param);
            }
            break;

        case MagOmxComponentBase_CommandPortDisable:
        {
            base->disablePort(priv, param);
        }
            break;

        case MagOmxComponentBase_CommandPortEnable:
        {
            base->enablePort(priv, param);
        }
            break;
            
        case MagOmxComponentBase_CommandMarkBuffer:
            AGILE_LOGD("the CommandMarkBuffer is not implemented!");
            break;

        default:
            AGILE_LOGE("Wrong commands(%d)!", cmd);
            break;
    }
}

/********************************
 *State Transition Actions
 ********************************/
static OMX_ERRORTYPE doStateLoaded_WaitforResources(OMX_IN OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

/*Acquire all of static resources*/
static OMX_ERRORTYPE doStateLoaded_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    base->enablePort(hComponent, OMX_ALL);
    if (MagOmxComponentBaseVirtual(base)->MagOMX_Prepare)
        /*if the return is OMX_ErrorInsufficientResources, IL Client may elect to transition the component to  WaitforResources state*/
        return MagOmxComponentBaseVirtual(base)->MagOMX_Prepare(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Prepare() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }     
}

static OMX_ERRORTYPE doStateIdle_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    base->disablePort(hComponent, OMX_ALL);
    if (MagOmxComponentBaseVirtual(base)->MagOMX_FreeResources)
        return MagOmxComponentBaseVirtual(base)->MagOMX_FreeResources(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_FreeResources() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateIdle_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    if (MagOmxComponentBaseVirtual(base)->MagOMX_Start)
        return MagOmxComponentBaseVirtual(base)->MagOMX_Start(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Start() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateIdle_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    if (MagOmxComponentBaseVirtual(base)->MagOMX_Preroll)
        return MagOmxComponentBaseVirtual(base)->MagOMX_Preroll(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Preroll() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateExecuting_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    if (MagOmxComponentBaseVirtual(base)->MagOMX_Stop)
        return MagOmxComponentBaseVirtual(base)->MagOMX_Stop(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Stop() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateExecuting_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    if (MagOmxComponentBaseVirtual(base)->MagOMX_Pause)
        return MagOmxComponentBaseVirtual(base)->MagOMX_Pause(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Pause() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStatePause_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    if (MagOmxComponentBaseVirtual(base)->MagOMX_Stop)
        return MagOmxComponentBaseVirtual(base)->MagOMX_Stop(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Stop() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 
}

static OMX_ERRORTYPE doStatePause_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentBase base;
    
    base = getBase(hComponent);

    if (MagOmxComponentBaseVirtual(base)->MagOMX_Resume)
        return MagOmxComponentBaseVirtual(base)->MagOMX_Resume(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Resume() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 
}

static OMX_ERRORTYPE doStateWaitforResources_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStateWaitforResources_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    /*get the OMX_EventResourcesAcquired event.*/
    return OMX_ErrorNone;
}

static OMX_BOOL  isVersionMatched(MagOMX_Param_Header_t *header){
    if (!((header->nVersion.s.nVersionMajor == kVersionMajor) && 
         (header->nVersion.s.nVersionMinor == kVersionMinor))){
        AGILE_LOGE("the version(%d.%d.%d.%d) is not matching to the component version(%d.%d.%d.%d)",
                     header->nVersion.s.nVersionMajor,
                     header->nVersion.s.nVersionMinor,
                     header->nVersion.s.nRevision,
                     header->nVersion.s.nStep,
                     kVersionMajor, kVersionMinor, kVersionRevision, kVersionStep);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

static OMX_ERRORTYPE MagOMX_SetParameter_internal(
                        OMX_IN  OMX_HANDLETYPE hComponent, 
                        OMX_IN  OMX_INDEXTYPE nIndex,
                        OMX_IN  OMX_PTR pComponentParameterStructure){

    MagOmxComponentBase base;
    MagOMX_Param_Header_t *header;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
    header = (MagOMX_Param_Header_t *)pComponentParameterStructure;
    if (!isVersionMatched(header)){
        return OMX_ErrorVersionMismatch;
    }

    if (!((comp->mState == OMX_StateLoaded) || (comp->mState == OMX_StateWaitForResources))){
        AGILE_LOGE("invalid state: %s!", OmxState2String(comp->mState));
        return OMX_ErrorIncorrectStateOperation;
        
    }
    
    if ((base->mState == OMX_StateLoaded) || (base->mState == OMX_StateWaitForResources){
        switch (nIndex){
            case OMX_IndexParamPriorityMgmt:
            case OMX_IndexConfigPriorityMgmt:
            {
                MagOMX_Param_PRIORITYMGMTTYPE_t *param;

                param = (MagOMX_Param_PRIORITYMGMTTYPE_t *)mag_mallocz(sizeof(MagOMX_Param_PRIORITYMGMTTYPE_t));
                if (param){
                    base->mParametersDB->setPointer(base->mParametersDB, "PriorityMgmt", (void *)param);
                }
            }
                break;

            case OMX_IndexParamAudioInit:
            case OMX_IndexParamImageInit:
            case OMX_IndexParamVideoInit:
            case OMX_IndexParamOtherInit:
            {
                MagOMX_Component_Type_t type;
                OMX_PORT_PARAM_TYPE *value;
                
                if (!MagOmxComponentBaseVirtual(base)->MagOMX_getType){
                    AGILE_LOGE("pure virtual func: MagOMX_getType() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                }

                value = (OMX_PORT_PARAM_TYPE *)pComponentParameterStructure;
                type = MagOmxComponentBaseVirtual(base)->MagOMX_getType(hComponent);

                if ( ((nIndex == OMX_IndexParamAudioInit) && (type == MagOMX_Component_Audio)) ||
                     ((nIndex == OMX_IndexParamImageInit) && (type == MagOMX_Component_Image)) ||
                     ((nIndex == OMX_IndexParamVideoInit) && (type == MagOMX_Component_Video)) ||
                     ((nIndex == OMX_IndexParamOtherInit) && ((type == MagOMX_Component_Subtitle)||(type == MagOMX_Component_Other)))){
                    base->mParametersDB->setUInt32(base->mParametersDB, "PortNumber", value->nPorts);
                    base->mParametersDB->setUInt32(base->mParametersDB, "StartPortNumber", value->nStartPortNumber);
                }else{
                    AGILE_LOGE("invalid parameter:%s setting to component type: %d", OmxParameter2String(nIndex), type);
                    return OMX_ErrorUnsupportedSetting;
                }
            }
                break;

            case OMX_IndexParamNumAvailableStreams:
            {
                OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
                MagOmxPort port;
                
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    port->setParameter(port, OMX_IndexParamNumAvailableStreams, value->nU32);
                }else{
                    AGILE_LOGE("[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }
                break;

            case OMX_IndexParamActiveStream:
            {
                OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
                MagOmxPort port;
                
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    port->setParameter(port, OMX_IndexParamActiveStream, value->nU32);
                }else{
                    AGILE_LOGE("[OMX_IndexParamActiveStream]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }
                break;

            case OMX_IndexParamSuspensionPolicy:
            {
                OMX_PARAM_SUSPENSIONPOLICYTYPE *value = (OMX_PARAM_SUSPENSIONPOLICYTYPE *)pComponentParameterStructure;
                base->mParametersDB->setUInt32(base->mParametersDB, "SuspensionPolicy", value->ePolicy);
            }
                break;

            case OMX_IndexParamComponentSuspended:
            {
                OMX_PARAM_SUSPENSIONTYPE *value = (OMX_PARAM_SUSPENSIONTYPE *)pComponentParameterStructure;
                base->mParametersDB->setUInt32(base->mParametersDB, "ComponentSuspended", value->eType);
            }
                break;

            case OMX_IndexParamStandardComponentRole:
            {
                OMX_PARAM_COMPONENTROLETYPE *value = (OMX_PARAM_COMPONENTROLETYPE *)pComponentParameterStructure;
                base->mParametersDB->setString(base->mParametersDB, "StandardComponentRole", value->cRole);
            }
                break;

            case OMX_IndexConfigTunneledPortStatus:
            {
                OMX_CONFIG_TUNNELEDPORTSTATUSTYPE *value = (OMX_CONFIG_TUNNELEDPORTSTATUSTYPE *)pComponentParameterStructure;
                MagOmxPort port;
                
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    port->setParameter(port, OMX_IndexConfigTunneledPortStatus, value->nTunneledPortStatus);
                }else{
                    AGILE_LOGE("[OMX_IndexConfigTunneledPortStatus]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }
                break;

            case OMX_IndexParamPortDefinition:
            {
                OMX_PARAM_PORTDEFINITIONTYPE *value;
                MagOMX_Component_Type_t type;
                MagOmxPort port;

                if (!MagOmxComponentBaseVirtual(base)->MagOMX_getType){
                    AGILE_LOGE("pure virtual func: MagOMX_getType() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                }

                value = (OMX_PARAM_PORTDEFINITIONTYPE *)pComponentParameterStructure;
                type = MagOmxComponentBaseVirtual(base)->MagOMX_getType(hComponent);

                if ( ((value->eDomain == OMX_PortDomainAudio) && (type == MagOMX_Component_Audio)) ||
                     ((value->eDomain == OMX_PortDomainImage) && (type == MagOMX_Component_Image)) ||
                     ((value->eDomain == OMX_PortDomainVideo) && (type == MagOMX_Component_Video)) ||
                     ((value->eDomain == OMX_PortDomainOther) && ((type == MagOMX_Component_Subtitle)||(type == MagOMX_Component_Other)))){
                    port = base->getPort(base, value->nPortIndex);
                    if (port){
                        port->setPortDefinition(port, value);
                    }else{
                        AGILE_LOGE("[OMX_IndexParamPortDefinition]: failed to find port: %d", value->nPortIndex);
                        return OMX_ErrorBadPortIndex;
                    }
                }else{
                    AGILE_LOGE("invalid parameter:OMX_IndexParamPortDefinition setting to component type: %d", type);
                    return OMX_ErrorUnsupportedSetting;
                }   
            }
                break;

            case OMX_IndexParamCompBufferSupplier:
            {
                OMX_PARAM_BUFFERSUPPLIERTYPE *value = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pComponentParameterStructure;
                MagOmxPort port;

                port = base->getPort(base, value->nPortIndex);
                if (port){
                    port->setParameter(port, OMX_IndexParamCompBufferSupplier, value->eBufferSupplier);
                }else{
                    AGILE_LOGE("[OMX_IndexParamCompBufferSupplier]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
                
            }
                break;
                
            default:
                /*pass it to sub-component for processing*/
                if (MagOmxComponentBaseVirtual(base)->MagOMX_SetParameter)
                    return MagOmxComponentBaseVirtual(base)->MagOMX_SetParameter(hComponent, nIndex, pComponentParameterStructure);
                else{
                    AGILE_LOGE("pure virtual func: MagOMX_SetParameter() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                } 
                break;
        }
    }else{
        return OMX_ErrorIncorrectStateOperation;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOMX_GetParameter_internal(
                        OMX_IN  OMX_HANDLETYPE hComponent, 
                        OMX_IN  OMX_INDEXTYPE nIndex,  
                        OMX_INOUT OMX_PTR pComponentParameterStructure){

    MagOmxComponentBase base;
    MagOMX_Param_Header_t *header;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
    if ((base->mState >= OMX_StateLoaded) && (base->mState <= OMX_StateWaitForResources){
        switch (nIndex){
            case OMX_IndexParamPriorityMgmt:
            case OMX_IndexConfigPriorityMgmt:
            {
                MagOMX_Param_PRIORITYMGMTTYPE_t *param;
                OMX_PRIORITYMGMTTYPE *value = (OMX_PRIORITYMGMTTYPE *)pComponentParameterStructure;
                
                if (base->mParametersDB->findPointer(base->mParametersDB, "PriorityMgmt", &param)){
                    initHeader(value, sizeof(OMX_PRIORITYMGMTTYPE));
                    value->nGroupID       = param->nGroupID;
                    value->nGroupPriority = param->nGroupPriority;
                }
            }
                break;

            /*to ports*/
            case OMX_IndexParamAudioInit:
            case OMX_IndexParamImageInit:
            case OMX_IndexParamVideoInit:
            case OMX_IndexParamOtherInit:
            {
                MagOMX_Component_Type_t type;
                OMX_PORT_PARAM_TYPE *value;
                
                if (!MagOmxComponentBaseVirtual(base)->MagOMX_getType){
                    AGILE_LOGE("pure virtual func: MagOMX_getType() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                }

                value = (OMX_PORT_PARAM_TYPE *)pComponentParameterStructure;
                type = MagOmxComponentBaseVirtual(base)->MagOMX_getType(hComponent);

                if ( ((nIndex == OMX_IndexParamAudioInit) && (type == MagOMX_Component_Audio)) ||
                     ((nIndex == OMX_IndexParamImageInit) && (type == MagOMX_Component_Image)) ||
                     ((nIndex == OMX_IndexParamVideoInit) && (type == MagOMX_Component_Video)) ||
                     ((nIndex == OMX_IndexParamOtherInit) && ((type == MagOMX_Component_Subtitle)||(type == MagOMX_Component_Other)))){
                    initHeader(value, sizeof(OMX_PORT_PARAM_TYPE));
                    if (base->mParametersDB->findUInt32(base->mParametersDB, "PortNumber", &value->nPorts)){
                        if (!base->mParametersDB->findUInt32(base->mParametersDB, "StartPortNumber", &value->nStartPortNumber)){
                            AGILE_LOGE("failed to find the parameter: StartPortNumber");
                            return OMX_ErrorBadParameter;
                        }
                    }else{
                        AGILE_LOGE("failed to find the parameter: PortNumber");
                        return OMX_ErrorBadParameter;
                    }
                    
                }else{
                    AGILE_LOGE("invalid parameter:%s setting to component type: %d", OmxParameter2String(nIndex), type);
                    return OMX_ErrorUnsupportedSetting;
                }
            }
                break;

            case OMX_IndexParamNumAvailableStreams:
            {
                OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
                MagOmxPort port;
                OMX_ERRORTYPE ret;
                
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    initHeader(value, sizeof(OMX_PORT_PARAM_TYPE));
                    ret = port->getParameter(port, OMX_IndexParamNumAvailableStreams, &value->nU32);
                    if ( OMX_ErrorNone != ret ){
                        return ret;
                    }
                        
                }else{
                    AGILE_LOGE("[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }
                break;

            case OMX_IndexParamActiveStream:
            {
                OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
                MagOmxPort port;
                OMX_ERRORTYPE ret;
                
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    initHeader(value, sizeof(OMX_PORT_PARAM_TYPE));
                    ret = port->getParameter(port, OMX_IndexParamActiveStream, &value->nU32);
                    if ( OMX_ErrorNone != ret){
                        return ret;
                    }
                        
                }else{
                    AGILE_LOGE("[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }
                break;

            case OMX_IndexParamSuspensionPolicy:
            {
                OMX_ERRORTYPE ret;
                
                OMX_PARAM_SUSPENSIONPOLICYTYPE *value = (OMX_PARAM_SUSPENSIONPOLICYTYPE *)pComponentParameterStructure;
                initHeader(value, sizeof(OMX_PARAM_SUSPENSIONPOLICYTYPE));
                ret = base->mParametersDB->getUInt32(base->mParametersDB, "SuspensionPolicy", &value->ePolicy);

                if ( OMX_ErrorNone != ret ){
                    /*Not found and set it as default*/
                    value->ePolicy = OMX_SuspensionDisabled;
                }
            }
                break;

            case OMX_IndexParamComponentSuspended:
            {   
                OMX_ERRORTYPE ret;
                
                OMX_PARAM_SUSPENSIONTYPE *value = (OMX_PARAM_SUSPENSIONTYPE *)pComponentParameterStructure;
                initHeader(value, sizeof(OMX_PARAM_SUSPENSIONTYPE));
                ret = base->mParametersDB->getUInt32(base->mParametersDB, "ComponentSuspended", &value->eType);

                if ( OMX_ErrorNone != ret ){
                    /*Not found and set it as default*/
                    value->eType = OMX_NotSuspended;
                }
            }
                break;

            case OMX_IndexParamStandardComponentRole:
            {
                OMX_ERRORTYPE ret;
                
                OMX_PARAM_COMPONENTROLETYPE *value = (OMX_PARAM_COMPONENTROLETYPE *)pComponentParameterStructure;
                initHeader(value, sizeof(OMX_PARAM_COMPONENTROLETYPE));
                ret = base->mParametersDB->getString(base->mParametersDB, "StandardComponentRole", &value->cRole);

                if ( OMX_ErrorNone != ret ){
                    return ret;
                }
            }
                break;

            case OMX_IndexConfigTunneledPortStatus:
            {
                OMX_CONFIG_TUNNELEDPORTSTATUSTYPE *value = (OMX_CONFIG_TUNNELEDPORTSTATUSTYPE *)pComponentParameterStructure;
                MagOmxPort port;
                OMX_ERRORTYPE ret;
                
                initHeader(value, sizeof(OMX_CONFIG_TUNNELEDPORTSTATUSTYPE));
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    ret = port->getParameter(port, OMX_IndexConfigTunneledPortStatus, &value->nTunneledPortStatus);
                    if ( OMX_ErrorNone != ret )
                        return ret;
                }else{
                    AGILE_LOGE("[OMX_IndexConfigTunneledPortStatus]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }
                break;

            case OMX_IndexParamPortDefinition:
            {
                OMX_PARAM_PORTDEFINITIONTYPE *value;
                MagOMX_Component_Type_t type;
                MagOmxPort port;

                if (!MagOmxComponentBaseVirtual(base)->MagOMX_getType){
                    AGILE_LOGE("pure virtual func: MagOMX_getType() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                }

                value = (OMX_PARAM_PORTDEFINITIONTYPE *)pComponentParameterStructure;
                type = MagOmxComponentBaseVirtual(base)->MagOMX_getType(hComponent);

                if ( ((value->eDomain == OMX_PortDomainAudio) && (type == MagOMX_Component_Audio)) ||
                     ((value->eDomain == OMX_PortDomainImage) && (type == MagOMX_Component_Image)) ||
                     ((value->eDomain == OMX_PortDomainVideo) && (type == MagOMX_Component_Video)) ||
                     ((value->eDomain == OMX_PortDomainOther) && ((type == MagOMX_Component_Subtitle)||(type == MagOMX_Component_Other)))){
                    port = base->getPort(base, value->nPortIndex);
                    if (port){
                        port->getPortDefinition(port, value);
                    }else{
                        AGILE_LOGE("[OMX_IndexParamPortDefinition]: failed to find port: %d", value->nPortIndex);
                        return OMX_ErrorBadPortIndex;
                    }
                }else{
                    AGILE_LOGE("invalid parameter:OMX_IndexParamPortDefinition setting to component type: %d", type);
                    return OMX_ErrorUnsupportedSetting;
                }   
            }
                break;

            case OMX_IndexParamCompBufferSupplier:
            {
                OMX_PARAM_BUFFERSUPPLIERTYPE *value = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pComponentParameterStructure;
                MagOmxPort port;
                OMX_ERRORTYPE ret;
                
                port = base->getPort(base, value->nPortIndex);
                if (port){
                    ret = port->getParameter(port, OMX_IndexParamCompBufferSupplier, &value->eBufferSupplier);

                    if ( OMX_ErrorNone != ret ){
                        return ret;
                    }
                }else{
                    AGILE_LOGE("[OMX_IndexParamCompBufferSupplier]: failed to find port: %d", value->nPortIndex);
                    return OMX_ErrorBadPortIndex;
                }
                
            }
                break;
                
            default:
                /*pass it to sub-component for processing*/
                if (MagOmxComponentBaseVirtual(base)->MagOMX_GetParameter)
                    return MagOmxComponentBaseVirtual(base)->MagOMX_GetParameter(hComponent, nIndex, pComponentParameterStructure);
                else{
                    AGILE_LOGE("pure virtual func: MagOMX_GetParameter() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                } 
                break;
        }
    }else{
        return OMX_ErrorIncorrectStateOperation;
    }
    return OMX_ErrorNone;
}


/********************************
 *Virtual member function implementation
 ********************************/
static OMX_ERRORTYPE virtual_GetComponentVersion(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STRING pComponentName,
                OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                OMX_OUT OMX_UUIDTYPE* pComponentUUID){

    setSpecVersion(pSpecVersion);
    setComponentVersion(pComponentVersion);
    
    if (MagOmxComponentBaseVirtual(getBase(hComponent))->MagOMX_GetComponentUUID){
        return MagOmxComponentBaseVirtual(getBase(hComponent))->MagOMX_GetComponentUUID(hComponent, pComponentUUID);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_GetComponentUUID() is not overrided!!");
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_SendCommand(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData){

    MagOmxComponentBase base;
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
    if (Cmd == OMX_CommandStateSet){
        if ( !base->mCmdSetStateMsg ){
            base->mCmdSetStateMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandStateSet);  
        }
        base->mCmdSetStateMsg->setUInt32(base->mCmdSetStateMsg, "param", nParam1);
        base->mCmdSetStateMsg->setPointer(base->mCmdSetStateMsg, "cmd_data", pCmdData);

        base->mCmdSetStateMsg->postMessage(base->mCmdSetStateMsg, 0);
    }else if (Cmd == OMX_CommandFlush){
        if ( !base->mCmdFlushMsg ){
            base->mCmdFlushMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandFlush);  
        }
        base->mCmdFlushMsg->setUInt32(base->mCmdFlushMsg, "param", nParam1);
        base->mCmdFlushMsg->setPointer(base->mCmdFlushMsg, "cmd_data", pCmdData);
        
        base->mCmdFlushMsg->postMessage(base->mCmdFlushMsg, 0);
    }else if (Cmd == OMX_CommandPortDisable){
        if ( !base->mCmdPortDisableMsg ){
            base->mCmdPortDisableMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandPortDisable);  
        }
        base->mCmdPortDisableMsg->setUInt32(base->mCmdPortDisableMsg, "param", nParam1);
        base->mCmdPortDisableMsg->setPointer(base->mCmdPortDisableMsg, "cmd_data", pCmdData);

        base->mCmdPortDisableMsg->postMessage(base->mCmdPortDisableMsg, 0);
    }else if (Cmd == OMX_CommandPortEnable){
        if ( !base->mCmdPortEnableMsg ){
            base->mCmdPortEnableMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandPortEnable);  
        }
        base->mCmdPortEnableMsg->setUInt32(base->mCmdPortEnableMsg, "param", nParam1);
        base->mCmdPortEnableMsg->setPointer(base->mCmdPortEnableMsg, "cmd_data", pCmdData);

        base->mCmdPortEnableMsg->postMessage(base->mCmdPortEnableMsg, 0);
    }else if (Cmd == OMX_CommandMarkBuffer){
        if ( !base->mCmdMarkBufferMsg ){
            base->mCmdMarkBufferMsg = base->createMessage(hComponent, MagOmxComponentBase_CommandMarkBuffer);  
        }
        base->mCmdMarkBufferMsg->setUInt32(base->mCmdMarkBufferMsg, "param", nParam1);
        base->mCmdMarkBufferMsg->setPointer(base->mCmdMarkBufferMsg, "cmd_data", pCmdData);

        base->mCmdMarkBufferMsg->postMessage(base->mCmdMarkBufferMsg, 0);
    }else{
        AGILE_LOGE("invalid command: 0x%x", Cmd);
        return OMX_ErrorBadParameter;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_GetParameter(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nParamIndex,  
                OMX_INOUT OMX_PTR pComponentParameterStructure){

    MagOmxComponentBase comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    Mag_AcquireMutex(comp->mhParamMutex);
    ret = MagOMX_GetParameter_internal(hComponent, nParamIndex, pComponentParameterStructure);
    Mag_ReleaseMutex(comp->mhParamMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_SetParameter(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){

    MagOmxComponentBase comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    Mag_AcquireMutex(comp->mhParamMutex);
    ret = MagOMX_SetParameter_internal(hComponent, nIndex, pComponentParameterStructure);
    Mag_ReleaseMutex(comp->mhParamMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_GetConfig(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){

    MagOmxComponentBase comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    Mag_AcquireMutex(comp->mhParamMutex);
    ret = MagOMX_GetParameter_internal(hComponent, nIndex, pComponentConfigStructure);
    Mag_ReleaseMutex(comp->mhParamMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_SetConfig(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){
                
    MagOmxComponentBase comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    Mag_AcquireMutex(comp->mhParamMutex);
    ret = MagOMX_SetParameter_internal(hComponent, nIndex, pComponentConfigStructure);
    Mag_ReleaseMutex(comp->mhParamMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_GetExtensionIndex(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType){
                
    return OMX_ErrorNotImplemented;
}


static OMX_ERRORTYPE virtual_GetState(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STATETYPE* pState){

    MagOmxComponentBase comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);

    *pState = comp->mState;
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_ComponentTunnelRequest(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPort,
                OMX_IN  OMX_HANDLETYPE hTunneledComp,
                OMX_IN  OMX_U32 nTunneledPort,
                OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup){

}

static OMX_ERRORTYPE virtual_UseBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){
                
    MagOmxComponentBase comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == *ppBufferHdr)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    
    port = comp->getPort(comp, nPortIndex);
    if (NULL != port){
        return MagOmxPortVirtual(port)->UseBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_AllocateBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){
                
    MagOmxComponentBase comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == *ppBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    
    port = comp->getPort(comp, nPortIndex);
    if (NULL != port){
        return MagOmxPortVirtual(port)->AllocateBuffer(port, ppBuffer, pAppPrivate, nSizeBytes);
    }else{
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FreeBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
                
    MagOmxComponentBase comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    
    port = comp->getPort(comp, nPortIndex);
    if (NULL != port){
        return MagOmxPortVirtual(port)->FreeBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_EmptyThisBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    MagOmxComponentBase comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    
    port = comp->getPort(comp, pBuffer->nInputPortIndex);
    if ( (NULL != port) && port->isInputPort(port) ){
        return MagOmxPortVirtual(port)->EmptyThisBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FillThisBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    MagOmxComponentBase comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    
    port = comp->getPort(comp, pBuffer->nOutputPortIndex);
    if ( (NULL != port) && !port->isInputPort(port) ){
        return MagOmxPortVirtual(port)->FillThisBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_SetCallbacks(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){

    MagOmxComponent parent;
    parent = ooc_cast(hComponent, MagOmxComponent);

    parent->mComponentObject.pApplicationPrivate = pAppData;
    getBase(hComponent)->mpAppData   = pAppData;
    getBase(hComponent)->mpCallbacks = pCallbacks;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_UseEGLImage(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){

}

static OMX_COMPONENTTYPE *virtual_Create(
                OMX_IN OMX_HANDLETYPE hComponent, 
                OMX_IN OMX_PTR pAppData,
                OMX_IN OMX_CALLBACKTYPE *pCallbacks){

    OMX_COMPONENTTYPE *comp;
    MagOmxComponentBase self;
    MagOmxComponent parent;
    
    parent = ooc_cast(hComponent, MagOmxComponent);
    comp = MagOmxComponentVirtual(parent)->Create(parent, pAppData);

    self = getBase(hComponent);
    
    self->mpAppData   = pAppData;
    self->mpCallbacks = pCallbacks;
    
    self->setState(hComponent, OMX_StateLoaded);
    
    return comp;
}

static OMX_ERRORTYPE virtual_sendEvents(
                OMX_IN MagOmxComponentBase hComponent,
                OMX_IN OMX_EVENTTYPE eEvent,
                OMX_IN OMX_U32 nData1,
                OMX_IN OMX_U32 nData2,
                OMX_IN OMX_PTR pEventData){

    if (hComponent->mpCallbacks){
        if (hComponent->mpCallbacks->EventHandler){
            return hComponent->mpCallbacks->EventHandler(hComponent, 
                                                         hComponent->mpAppData, 
                                                         eEvent, 
                                                         nData1, 
                                                         nData2, 
                                                         pEventData);
        }else{
            AGILE_LOGE("the EventHandler Callback is NULL");
            return OMX_ErrorNotImplemented;
        }
    }else{
        AGILE_LOGE("the Callbacks are NULL");
        return OMX_ErrorNotImplemented;
    }
}

static OMX_ERRORTYPE virtual_sendEmptyBufferDoneEvent(
                OMX_IN MagOmxComponentBase hComponent,
                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){

    if (hComponent->mpCallbacks){
        if (hComponent->mpCallbacks->EmptyBufferDone){
            return hComponent->mpCallbacks->EmptyBufferDone(hComponent, 
                                                            hComponent->mpAppData, 
                                                            pBuffer);
        }else{
            AGILE_LOGE("the EmptyBufferDone Callback is NULL");
            return OMX_ErrorNotImplemented;
        }
    }else{
        AGILE_LOGE("the Callbacks are NULL");
        return OMX_ErrorNotImplemented;
    }
}

static OMX_ERRORTYPE virtual_sendFillBufferDoneEvent(
                        OMX_IN MagOmxComponentBase hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){

    if (hComponent->mpCallbacks){
        if (hComponent->mpCallbacks->FillBufferDone){
            return hComponent->mpCallbacks->FillBufferDone(hComponent, 
                                                           hComponent->mpAppData, 
                                                           pBuffer);
        }else{
            AGILE_LOGE("the FillBufferDone Callback is NULL");
            return OMX_ErrorNotImplemented;
        }
    }else{
        AGILE_LOGE("the Callbacks are NULL");
        return OMX_ErrorNotImplemented;
    }
}

/*Member functions*/

static MagMessageHandle MagOmxComponentBase_createMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentBase hComponent = NULL;
    
    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentBase);
        
    hComponent->getLooper(handle);
    
    MagMessageHandle msg = createMagMessage(hComponent->mLooper, what, hComponent->mMsgHandler->id(hComponent->mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxComponentBase_getLooper(OMX_HANDLETYPE handle){
    MagOmxComponentBase hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentBase);
    
    if ((NULL != hComponent->mLooper) && (NULL != hComponent->mMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mLooper){
        hComponent->mLooper = createLooper(LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mLooper);
    }
    
    if (NULL != hComponent->mLooper){
        if (NULL == hComponent->mMsgHandler){
            hComponent->mMsgHandler = createHandler(hComponent->mLooper, onMessageReceived, handle);

            if (NULL != hComponent->mMsgHandler){
                hComponent->mLooper->registerHandler(hComponent->mLooper, hComponent->mMsgHandler);
                hComponent->mLooper->start(hComponent->mLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

static OMX_ERRORTYPE MagOmxComponentBase_setState(OMX_HANDLETYPE handle, OMX_STATETYPE target_state){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    MagOmxComponentBase base;

    if (!handle){
        AGILE_LOGE("handle is NULL");
        return OMX_ErrorBadParameter;
    }

    base = getBase(handle);

    AGILE_LOGV("Try (current state:%s --> target state:%s)", OmxState2String(base->mState), OmxState2String(target_state));
    if (target_state == OMX_StateMax){
        if (base->mState == OMX_StateLoaded){
            /*do OMX_FreeHandle()*/
            base->mState = OMX_StateMax;
        }else{
            AGILE_LOGE("Invalid state transition: current state:%s --> target state:%s", OmxState2String(base->mState), OmxState2String(target_state));
            return OMX_ErrorIncorrectStateTransition;
        }
    }

    if (base->mState == OMX_StateMax){
        if (target_state == OMX_StateLoaded){
            /*do OMX_GetHandle() and before allocation of its resources*/
            base->mState = OMX_StateLoaded;
        }else{
            AGILE_LOGE("Invalid state transition: current state:%s --> target state:%s", OmxState2String(base->mState), OmxState2String(target_state));
            return OMX_ErrorIncorrectStateTransition;
        }
    }

    if (base->mStateTransitTable[toIndex(base->mState)][toIndex(target_state)]){
        ret = base->mStateTransitTable[toIndex(base->mState)][toIndex(target_state)](handle);
        if (ret == OMX_ErrorNone){
            AGILE_LOGV("To do current state:%s --> target state:%s -- OK!", OmxState2String(base->mState), OmxState2String(target_state))
            base->mState = target_state;
        }else{
            AGILE_LOGE("To do current state:%s --> target state:%s -- Failure!", OmxState2String(base->mState), OmxState2String(target_state))
        }
    }else{
        AGILE_LOGE("Invalid state transition: current state:%s --> target state:%s", OmxState2String(base->mState), OmxState2String(target_state));
        return OMX_ErrorIncorrectStateTransition;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentBase_flushPort(OMX_HANDLETYPE handle, OMX_U32 port_index){
    MagOmxPort port;
    MagOmxComponentBase comp;

    if (NULL == handle){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(handle);

    port = comp->getPort(comp, port_index);
    return MagOmxPortVirtual(port)->flushPort(port);
}

static OMX_ERRORTYPE MagOmxComponentBase_enablePort(OMX_HANDLETYPE handle, OMX_U32 port_index){            
    MagOmxPort port;
    MagOmxComponentBase comp;

    if (NULL == handle){
        return OMX_ErrorBadParameter;
    }

    comp = getBase(handle);
    
    if (port_index == OMX_ALL){
        OMX_S32 i = 0;
        for (i = 0; i < comp->mPortsNumber; i++){
            port = comp->getPort(comp, i);
            
            if (MagOmxPortVirtual(port)->enablePort(port, comp->mpAppData) == OMX_ErrorNone){
                /*send back the completion event*/
                
            }else{

            }
        }
    }else{
        port = comp->getPort(comp, port_index);
        
        if (MagOmxPortVirtual(port)->enablePort(port, comp->mpAppData) == OMX_ErrorNone){
            /*send back the completion event*/
            
        }else{

        }
    }
    /*TODO: disable both ends of a tunnel */
}

static OMX_ERRORTYPE MagOmxComponentBase_disablePort(OMX_HANDLETYPE handle, OMX_U32 port_index){
    MagOmxPort port;
    MagOmxComponentBase comp;

    if (NULL == handle){
        return OMX_ErrorBadParameter;
    }

    comp = getBase(handle);
    
    if (port_index == OMX_ALL){
        OMX_S32 i = 0;
        for (i = 0; i < comp->mPortsNumber; i++){
            port = comp->getPort(comp, i);
            
            if (MagOmxPortVirtual(port)->disablePort(port, comp->mpAppData) == OMX_ErrorNone){
                /*send back the completion event*/
                
            }else{

            }
        }
    }else{
        port = comp->getPort(comp, port_index);
        
        if (MagOmxPortVirtual(port)->disablePort(port, comp->mpAppData) == OMX_ErrorNone){
            /*send back the completion event*/
            
        }else{

        }
    }
    /*TODO: disable both ends of a tunnel */
}

static void          MagOmxComponentBase_addPort(MagOmxComponentBase hComponent, 
                                                        OMX_U32 portIndex, 
                                                        OMX_HANDLETYPE hPort){

    if ((NULL == hPort) || (NULL == hComponent)){
        return OMX_ErrorBadParameter;
    }

    hComponent->mPortsTable =  rbtree_insert(hComponent->mPortsTable, portIndex, hPort);

    if (hComponent->mPortsTable){
        hComponent->mPortsNumber++;
    }
}

static MagOmxPort MagOmxComponentBase_getPort(MagOmxComponentBase hComponent, 
                                       OMX_U32 portIndex){

    OMX_HANDLETYPE hPort = NULL;
    
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    hPort = rbtree_get(hComponent->mPortsTable, portIndex);

    if (!hPort){
        AGILE_LOGE("failed to get the port(%d) of the component:0x%p", portIndex, hComponent);
        return NULL;
    }

    return (ooc_cast(hPort, MagOmxPort));
}


/*Class Constructor/Destructor*/
static void MagOmxComponentBase_initialize(Class this){
    /*Override the base component pure virtual functions*/
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetComponentVersion    = virtual_GetComponentVersion;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SendCommand            = virtual_SendCommand;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetParameter           = virtual_GetParameter;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SetParameter           = virtual_SetParameter;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetConfig              = virtual_GetConfig;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SetConfig              = virtual_SetConfig;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetExtensionIndex      = virtual_GetExtensionIndex;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.GetState               = virtual_GetState;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.ComponentTunnelRequest = virtual_ComponentTunnelRequest;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.UseBuffer              = virtual_UseBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.AllocateBuffer         = virtual_AllocateBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.FreeBuffer             = virtual_FreeBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.EmptyThisBuffer        = virtual_EmptyThisBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.FillThisBuffer         = virtual_FillThisBuffer;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.UseEGLImage            = virtual_UseEGLImage;
    MagOmxComponentBaseVtableInstance.MagOmxComponent.SetCallbacks           = virtual_SetCallbacks; 
    
    MagOmxComponentBaseVtableInstance.Create                                 = virtual_Create;
    MagOmxComponentBaseVtableInstance.sendEvents                             = virtual_sendEvents;
    MagOmxComponentBaseVtableInstance.sendEmptyBufferDoneEvent               = virtual_sendEmptyBufferDoneEvent;
    MagOmxComponentBaseVtableInstance.sendFillBufferDoneEvent                = virtual_sendFillBufferDoneEvent;
    
    /*pure virtual functions to be overrided by sub-components*/
    MagOmxComponentBaseVtableInstance.MagOMX_GetComponentUUID = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_Prepare          = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_Preroll          = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_Start            = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_Stop             = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_Pause            = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_Resume           = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_FreeResources    = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_GetParameter     = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_SetParameter     = NULL;
    MagOmxComponentBaseVtableInstance.MagOMX_ComponentRoleEnum = NULL;
}

static void MagOmxComponentBase_constructor(MagOmxComponentBase thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmxComponentBase));
    chain_constructor(MagOmxComponentBase, thiz, params);

    thiz->createMessage     = MagOmxComponentBase_createMessage;
    thiz->getLooper         = MagOmxComponentBase_getLooper;
    thiz->setState          = MagOmxComponentBase_setState;
    thiz->flushPort         = MagOmxComponentBase_flushPort;
    thiz->enablePort        = MagOmxComponentBase_enablePort;
    thiz->disablePort       = MagOmxComponentBase_disablePort;
    thiz->addPort           = MagOmxComponentBase_addPort;
    thiz->getPort           = MagOmxComponentBase_getPort;
    
    thiz->mLooper     = NULL;
    thiz->mMsgHandler = NULL;

    thiz->mCmdSetStateMsg    = NULL;
    thiz->mCmdPortDisableMsg = NULL;
    thiz->mCmdPortEnableMsg  = NULL;
    thiz->mCmdFlushMsg       = NULL;
    thiz->mCmdMarkBufferMsg  = NULL;

    /*set initial state as OMX_StateMax*/
    thiz->mState = OMX_StateMax;
    thiz->mStateTransitTable[5][5] = { {NULL, doStateLoaded_Idle, NULL, NULL, doStateLoaded_WaitforResources},
                                       {doStateIdle_Loaded, NULL, doStateIdle_Executing, doStateIdle_Pause, NULL},
                                       {NULL, doStateExecuting_Idle, NULL, doStateExecuting_Pause, NULL},
                                       {NULL, doStatePause_Idle, doStatePause_Executing, NULL, NULL},
                                       {doStateWaitforResources_Loaded, doStateWaitforResources_Idle, NULL, NULL, NULL}};

    thiz->mParametersDB = createMagMiniDB(64);
    thiz->mConfigDB     = createMagMiniDB(64);

    thiz->mPortsTable   = NULL;
    thiz->mpCallbacks   = NULL;
    thiz->mpAppData     = NULL;
    thiz->mPortsNumber  = 0;
    
    Mag_CreateMutex(&thiz->mhParamMutex);
}

static void MagOmxComponentBase_destructor(MagOmxComponentBase thiz, MagOmxComponentBaseVtable vtab){
    AGILE_LOGV("Enter!");
}


