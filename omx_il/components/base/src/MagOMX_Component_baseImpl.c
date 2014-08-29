#include "MagOMX_Component_baseImpl.h"

#define LOOPER_NAME        "CompImplLooper"
#define BUFFER_LOOPER_NAME "CompImplBufLooper"

AllocateClass(MagOmxComponentImpl, MagOmxComponent);

static MagOmxComponent getRoot(OMX_HANDLETYPE hComponent) {

    MagOmxComponent root;
    root = ooc_cast(hComponent, MagOmxComponent);
    return root;
}

static MagOmxComponentImpl getBase(OMX_HANDLETYPE hComponent) {

    MagOmxComponentImpl base;
    base = ooc_cast(hComponent, MagOmxComponentImpl);
    return base;
}

static void onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponentImpl base;
    OMX_U32 param;
    OMX_PTR cmd_data = NULL;
    OMX_ERRORTYPE ret;
    OMX_U32 cmd;
    MagOmxPort port;
    OMX_BUFFERHEADERTYPE *bufHeader;
    MagMessageHandle returnBufMsg;

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
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentImpl_CommandStateSetMsg:
            {
            OMX_STATETYPE target_state = (OMX_STATETYPE)param;
            Mag_AcquireMutex(base->mhMutex);
            ret = base->setState(priv, target_state);
            Mag_ReleaseMutex(base->mhMutex);
            base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandStateSet, target_state, &ret);
            }
            break;

        case MagOmxComponentImpl_CommandFlushMsg:
            {
            Mag_AcquireMutex(base->mhMutex);
            base->flushPort(priv, param);
            Mag_ReleaseMutex(base->mhMutex);
            // base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandFlush, param, &ret);
            }
            break;

        case MagOmxComponentImpl_CommandPortDisableMsg:
            {
            Mag_AcquireMutex(base->mhMutex);    
            base->disablePort(priv, param);
            Mag_ReleaseMutex(base->mhMutex);
            // base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandPortDisable, param, &ret);
            }
            break;

        case MagOmxComponentImpl_CommandPortEnableMsg:
            {
            Mag_AcquireMutex(base->mhMutex);
            base->enablePort(priv, param);
            Mag_ReleaseMutex(base->mhMutex);
            // base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandPortEnable, param, &ret);
            }
            break;
            
        case MagOmxComponentImpl_CommandMarkBufferMsg:
            AGILE_LOGD("the CommandMarkBuffer is not implemented!");
            break;

        default:
            break;
    }
}

static void onBufferMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponentImpl base;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_U32 cmd;
    MagOmxPort port;
    OMX_BUFFERHEADERTYPE *srcBufHeader = NULL;
    OMX_BUFFERHEADERTYPE *destBufHeader = NULL;
    MagMessageHandle returnBufMsg = NULL;
    OMX_BOOL isFlushing = OMX_FALSE;

    MagOmxPort destPort = NULL;
    MagMessageHandle outputBufMsg = NULL;

    if (!msg){
        AGILE_LOGE("msg is NULL!");
        return;
    }
    
    cmd = msg->what(msg);
    base = getBase(priv);

    if ((cmd >= MagOmxComponentImpl_PortDataFlowMsg) &&
        (cmd < MagOmxComponentImpl_PortDataFlowMsg + base->mPorts)){
        if (!msg->findPointer(msg, "buffer_header", &srcBufHeader)){
            AGILE_LOGE("[msg]: failed to find the buffer_header!");
            return;
        }

        if (!msg->findPointer(msg, "destination_port", &destPort)){
            AGILE_LOGE("failed to find the destination_port! The component might not have the output port");
        }

        if (base->mFlushingPorts != kInvalidPortIndex){
            if (base->mFlushingPorts = OMX_ALL){
                isFlushing = OMX_TRUE;
            }else if ((base->mFlushingPorts == srcBufHeader->nOutputPortIndex) ||
                      (base->mFlushingPorts == srcBufHeader->nInputPortIndex)){
                isFlushing = OMX_TRUE;
            }
        }

        if (MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer){
            if (!isFlushing){
                if (destPort){
                    outputBufMsg = destPort->GetOutputBufferMsg(destPort);
                    if (outputBufMsg){
                        if (!outputBufMsg->findPointer(outputBufMsg, "buffer_header", &destBufHeader)){
                            AGILE_LOGE("[outputBufMsg]: failed to find the buffer_header!");
                            return;
                        }
                        ret = MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer(base, 
                                                                                     srcBufHeader,
                                                                                     destBufHeader);
                        outputBufMsg->postMessage(outputBufMsg, 0);
                    }else{
                        AGILE_LOGE("failed to get outputBufMsg");
                    }
                }else{
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer(base, 
                                                                                 srcBufHeader,
                                                                                 NULL);
                }
            }

            if (ret == OMX_ErrorNone){
                if (msg->findMessage(msg, "return_buf_msg", &returnBufMsg)){
                    returnBufMsg->setPointer(returnBufMsg, "buffer_header", srcBufHeader, MAG_FALSE);
                    returnBufMsg->setPointer(returnBufMsg, "component_obj", priv, MAG_FALSE);
                    returnBufMsg->postMessage(returnBufMsg, 0);
                }else{
                    AGILE_LOGE("failed to find the return_buf_msg!");
                }
            }else{
                AGILE_LOGE("Failed to do MagOMX_ProceedBuffer(), ret = 0x%x", ret);
            }
        }else{
            AGILE_LOGE("pure virtual func: MagOMX_ProceedBuffer() is not overrided!!");
            return OMX_ErrorNotImplemented;
        }

    }else{
        AGILE_LOGE("Wrong commands(%d)!", cmd);
    }
}

/********************************
 *State Transition Actions
 ********************************/
static OMX_ERRORTYPE doStateLoaded_WaitforResources(OMX_IN OMX_HANDLETYPE hComponent){
    AGILE_LOGV("Enter!");
    return OMX_ErrorNone;
}

/*Acquire all of static resources*/
static OMX_ERRORTYPE doStateLoaded_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    base = getBase(hComponent);

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        if (port->isTunneled(port)){
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->AllocateTunnelBuffer(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to allocate the tunnel buffers!");
                    return err;
                }
            }
        }else{
            if (port->getDef_Enabled(port)){
                if (!port->getDef_Populated(port)){
                    AGILE_LOGE("The port %lld is unpopulated!", n->key);
                    return OMX_ErrorPortUnpopulated;
                }
            }
        }
    }

    if (MagOmxComponentImplVirtual(base)->MagOMX_Prepare)
        /*if the return is OMX_ErrorInsufficientResources, IL Client may elect to transition the component to  WaitforResources state*/
        return MagOmxComponentImplVirtual(base)->MagOMX_Prepare(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Prepare() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }     
}

static OMX_ERRORTYPE doStateIdle_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    base = getBase(hComponent);

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        if (port->isTunneled(port)){
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->FreeTunnelBuffer(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to free the tunnel buffers from port %lld!", n->key);;
                }
            }
        }else{
            if (port->getDef_Enabled(port)){
                MagOmxPortVirtual(port)->FreeAllBuffers(n->value);
            }
        }
    }

    if (MagOmxComponentImplVirtual(base)->MagOMX_Reset)
        return MagOmxComponentImplVirtual(base)->MagOMX_Reset(hComponent);
    else{
        AGILE_LOGE("pure virtual func: MagOMX_Reset() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateIdle_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Start){
        err = MagOmxComponentImplVirtual(base)->MagOMX_Start(hComponent);

        if (err != OMX_ErrorNone){
            AGILE_LOGE("failed to start the component!");
            return err;
        }

        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Run(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to start the port data flow!");
                    return err;
                }
            }
        }
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Start() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateIdle_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    base = getBase(hComponent);

    AGILE_LOGD("why do we need the state transition: Idle --> Pause??");

    if (MagOmxComponentImplVirtual(base)->MagOMX_Preroll){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Pause(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to flush the port!");
                    return err;
                }
            }
        }

        return MagOmxComponentImplVirtual(base)->MagOMX_Preroll(hComponent);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Preroll() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateExecuting_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    OMX_ERRORTYPE err;
    RBTreeNodeHandle n;
    MagOmxPort port;

    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Stop){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Flush(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to flush the port!");
                    return err;
                }
            }
        }

        return MagOmxComponentImplVirtual(base)->MagOMX_Stop(hComponent);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Stop() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStateExecuting_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;

    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Pause){
        MagOmxComponentImplVirtual(base)->MagOMX_Pause(hComponent);

        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Pause(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to flush the port!");
                    return err;
                }
            }
        }
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Pause() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
}

static OMX_ERRORTYPE doStatePause_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;

    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Stop){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Resume(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to resume the port!");
                    return err;
                }

                err = MagOmxPortVirtual(port)->Flush(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to flush the port!");
                    return err;
                }
            }
        }

        return MagOmxComponentImplVirtual(base)->MagOMX_Stop(hComponent);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Stop() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 
}

static OMX_ERRORTYPE doStatePause_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;

    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Resume){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Resume(n->value);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to resume the port!");
                    return err;
                }
            }
        }

        return MagOmxComponentImplVirtual(base)->MagOMX_Resume(hComponent);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Resume() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 
}

static OMX_ERRORTYPE doStateWaitforResources_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    AGILE_LOGD("Don't support the state transition: WaitforResources --> Loaded");
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStateWaitforResources_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    AGILE_LOGD("Don't support the state transition: WaitforResources --> Idle");
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

static OMX_BOOL allowedSetParameter(MagOmxPort port, OMX_STATETYPE state){
    if ((state > OMX_StateLoaded) && (!port->getDef_Enabled(port)))
        return OMX_TRUE;
    else
        return OMX_FALSE;
}

static OMX_ERRORTYPE MagOMX_SetParameter_internal(
                        OMX_IN  OMX_HANDLETYPE hComponent, 
                        OMX_IN  OMX_INDEXTYPE nIndex,
                        OMX_IN  OMX_PTR pComponentParameterStructure){

    MagOmxComponentImpl base;
    MagOMX_Param_Header_t *header;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
    header = (MagOMX_Param_Header_t *)pComponentParameterStructure;
    if (!isVersionMatched(header)){
        return OMX_ErrorVersionMismatch;
    }
    
    switch (nIndex){
        case OMX_IndexParamPriorityMgmt:
        case OMX_IndexConfigPriorityMgmt:
        {
            MagOMX_Param_PRIORITYMGMTTYPE_t *param;
            OMX_PRIORITYMGMTTYPE *input = (OMX_PRIORITYMGMTTYPE *)pComponentParameterStructure;

            if (comp->mState != OMX_StateLoaded){
                AGILE_LOGE("disallow parameter(%d) setting in state %s!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            param = (MagOMX_Param_PRIORITYMGMTTYPE_t *)mag_mallocz(sizeof(MagOMX_Param_PRIORITYMGMTTYPE_t));
            if (param){
                param->nGroupPriority = input->nGroupPriority;
                param->nGroupID = input->nGroupID;
                base->mParametersDB->setPointer(base->mParametersDB, "PriorityMgmt", (void *)param);
            }
        }
            break;

        /*should not be set from OMX IL*/
        case OMX_IndexParamAudioInit:
        case OMX_IndexParamImageInit:
        case OMX_IndexParamVideoInit:
        case OMX_IndexParamOtherInit:
        {
            AGILE_LOGE("OMX IL should not set %s to the component!", OmxParameter2String(nIndex));
        }
            break;

        /*Container parsing: Specifies the number of alternative streams available on a given output port.*/
        case OMX_IndexParamNumAvailableStreams:
        {
            OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
            MagOmxPort port;

            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (!allowedSetParameter(port, comp->mState)){
                AGILE_LOGE("disallow parameter(%d) setting in state %s or on enabled port!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                port->setParameter(port, OMX_IndexParamNumAvailableStreams, value->nU32);
            }else{
                AGILE_LOGE("[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        /*Container parsing: Specifies the active stream (among those available) on a given output port.*/
        case OMX_IndexParamActiveStream:
        {
            OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
            MagOmxPort port;
            
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (!allowedSetParameter(port, comp->mState)){
                AGILE_LOGE("disallow parameter(%d) setting in state %s or on enabled port!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                port->setParameter(port, OMX_IndexParamActiveStream, value->nU32);
            }else{
                AGILE_LOGE("[OMX_IndexParamActiveStream]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        /*no further implemention for now*/
        case OMX_IndexParamSuspensionPolicy:
        {
            OMX_PARAM_SUSPENSIONPOLICYTYPE *value;

            if (comp->mState != OMX_StateLoaded){
                AGILE_LOGE("disallow parameter(%d) setting in state %s!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            value = (OMX_PARAM_SUSPENSIONPOLICYTYPE *)pComponentParameterStructure;
            base->mParametersDB->setUInt32(base->mParametersDB, "SuspensionPolicy", value->ePolicy);
        }
            break;

        /*no further implemention for now*/
        case OMX_IndexParamComponentSuspended:
        {
            OMX_PARAM_SUSPENSIONTYPE *value;

            if (comp->mState != OMX_StateLoaded){
                AGILE_LOGE("disallow parameter(%d) setting in state %s!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            value = (OMX_PARAM_SUSPENSIONTYPE *)pComponentParameterStructure;
            base->mParametersDB->setUInt32(base->mParametersDB, "ComponentSuspended", value->eType);
        }
            break;

        /*no further implemention for now*/
        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *value;

            if (comp->mState != OMX_StateLoaded){
                AGILE_LOGE("disallow parameter(%d) setting in state %s!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            value = (OMX_PARAM_COMPONENTROLETYPE *)pComponentParameterStructure;
            base->mParametersDB->setString(base->mParametersDB, "StandardComponentRole", value->cRole);
        }
            break;

        /*should not be set from OMX IL*/
        case OMX_IndexConfigTunneledPortStatus:
        {
            AGILE_LOGE("OMX IL should not set OMX_IndexConfigTunneledPortStatus to the component!");
        }
            break;

        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *value;
            MagOmxPort port;

            value = (OMX_PARAM_PORTDEFINITIONTYPE *)pComponentParameterStructure;
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);
            if (!allowedSetParameter(port, comp->mState)){
                AGILE_LOGE("disallow parameter(%d) setting in state %s or on enabled port!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                ret = port->setPortDefinition(port, value);
                if ( OMX_ErrorNone != ret ){
                    return ret;
                }
            }else{
                AGILE_LOGE("[OMX_IndexParamPortDefinition]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        case OMX_IndexParamCompBufferSupplier:
        {
            OMX_PARAM_BUFFERSUPPLIERTYPE *value = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pComponentParameterStructure;
            MagOmxPort port;

            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (!allowedSetParameter(port, comp->mState)){
                AGILE_LOGE("disallow parameter(%d) setting in state %s or on enabled port!", 
                            nIndex, OmxState2String(comp->mState))
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                port->setParameter(port, OMX_IndexParamCompBufferSupplier, value->eBufferSupplier);
            }else{
                AGILE_LOGE("[OMX_IndexParamCompBufferSupplier]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            
        }
            break;
        
        case OMX_IndexParamStandardComponentRole:
            break;

        default:
            if (nIndex > OMX_IndexAudioStartUnused && nIndex < OMX_IndexOtherStartUnused){
                /*the parameters or configures for port*/
                OMX_U32 portID;
                MagOmxPort port;

                portID = getPortIndex(pComponentParameterStructure);
                port = ooc_cast(base->getPort(base, portID), MagOmxPort);

                if (port){
                    return MagOmxPortVirtual(port)->SetParameter(port, nIndex, pComponentParameterStructure);
                }else{
                    AGILE_LOGE("To set port parameter[0x%x] - Failure. (port is invalid)", nIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }else{
                /*pass the parameter/config to the sub-component for processing no matter what state the component is in. 
                 The sub-component decides how to handle them*/
                if (MagOmxComponentImplVirtual(base)->MagOMX_SetParameter)   
                    return MagOmxComponentImplVirtual(base)->MagOMX_SetParameter(hComponent, nIndex, pComponentParameterStructure);
                else{
                    AGILE_LOGE("pure virtual func: MagOMX_SetParameter() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                } 
            }
            break;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOMX_GetParameter_internal(
                        OMX_IN  OMX_HANDLETYPE hComponent, 
                        OMX_IN  OMX_INDEXTYPE nIndex,  
                        OMX_INOUT OMX_PTR pComponentParameterStructure){

    MagOmxComponentImpl base;
    MagOMX_Param_Header_t *header;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
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
            
            if (!MagOmxComponentImplVirtual(base)->MagOMX_getType){
                AGILE_LOGE("pure virtual func: MagOMX_getType() is not overrided!!");
                return OMX_ErrorNotImplemented;
            }

            value = (OMX_PORT_PARAM_TYPE *)pComponentParameterStructure;
            type = MagOmxComponentImplVirtual(base)->MagOMX_getType(hComponent);

            if ( ((nIndex == OMX_IndexParamAudioInit) && (type == MagOMX_Component_Audio)) ||
                 ((nIndex == OMX_IndexParamImageInit) && (type == MagOMX_Component_Image)) ||
                 ((nIndex == OMX_IndexParamVideoInit) && (type == MagOMX_Component_Video)) ||
                 ((nIndex == OMX_IndexParamOtherInit) && ((type == MagOMX_Component_Subtitle)||(type == MagOMX_Component_Other)))){
                initHeader(value, sizeof(OMX_PORT_PARAM_TYPE));
                value->nStartPortNumber = base->mStartPortNumber;
                value->nPorts           = base->mPorts;
            }else{
                AGILE_LOGE("invalid parameter:%s setting to component type: %d", OmxParameter2String(nIndex), type);
                value->nStartPortNumber = 0;
                value->nPorts           = 0;
                return OMX_ErrorUnsupportedSetting;
            }
        }
            break;

        case OMX_IndexParamNumAvailableStreams:
        {
            OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
            MagOmxPort port;
            OMX_ERRORTYPE ret;
            
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);
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
            
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);
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
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);
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
            MagOmxPort port;

            if (!MagOmxComponentImplVirtual(base)->MagOMX_getType){
                AGILE_LOGE("pure virtual func: MagOMX_getType() is not overrided!!");
                return OMX_ErrorNotImplemented;
            }

            value = (OMX_PARAM_PORTDEFINITIONTYPE *)pComponentParameterStructure;
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (port){
                ret = port->getPortDefinition(port, value);
                if ( OMX_ErrorNone != ret )
                    return ret;
            }else{
                AGILE_LOGE("[OMX_IndexParamPortDefinition]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        case OMX_IndexParamCompBufferSupplier:
        {
            OMX_PARAM_BUFFERSUPPLIERTYPE *value = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pComponentParameterStructure;
            MagOmxPort port;
            OMX_ERRORTYPE ret;
            
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);
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
            if (nIndex > OMX_IndexAudioStartUnused && nIndex < OMX_IndexOtherStartUnused){
                /*the parameters or configures for port*/
                OMX_U32 portID;
                MagOmxPort port;

                portID = getPortIndex(pComponentParameterStructure);
                port = ooc_cast(base->getPort(base, portID), MagOmxPort);

                if (port){
                    return MagOmxPortVirtual(port)->GetParameter(port, nIndex, pComponentParameterStructure);
                }else{
                    AGILE_LOGE("To get port parameter[0x%x] - Failure. (port is invalid)", nIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }else{
                /*pass it to sub-component for processing*/
                if (MagOmxComponentImplVirtual(base)->MagOMX_GetParameter)
                    return MagOmxComponentImplVirtual(base)->MagOMX_GetParameter(hComponent, nIndex, pComponentParameterStructure);
                else{
                    AGILE_LOGE("pure virtual func: MagOMX_GetParameter() is not overrided!!");
                    return OMX_ErrorNotImplemented;
                } 
            }
            break;
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
    
    if (MagOmxComponentImplVirtual(getBase(hComponent))->MagOMX_GetComponentUUID){
        return MagOmxComponentImplVirtual(getBase(hComponent))->MagOMX_GetComponentUUID(hComponent, pComponentUUID);
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

    MagOmxComponentImpl base;
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    
    if (Cmd == OMX_CommandStateSet){
        if ( !base->mCmdSetStateMsg ){
            base->mCmdSetStateMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandStateSetMsg);  
        }
        base->mCmdSetStateMsg->setUInt32(base->mCmdSetStateMsg, "param", nParam1);
        base->mCmdSetStateMsg->setPointer(base->mCmdSetStateMsg, "cmd_data", pCmdData);

        base->mCmdSetStateMsg->postMessage(base->mCmdSetStateMsg, 0);
    }else if (Cmd == OMX_CommandFlush){
        if ( !base->mCmdFlushMsg ){
            base->mCmdFlushMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandFlushMsg);  
        }
        base->mCmdFlushMsg->setUInt32(base->mCmdFlushMsg, "param", nParam1);
        base->mCmdFlushMsg->setPointer(base->mCmdFlushMsg, "cmd_data", pCmdData);
        
        base->mCmdFlushMsg->postMessage(base->mCmdFlushMsg, 0);
    }else if (Cmd == OMX_CommandPortDisable){
        if ( !base->mCmdPortDisableMsg ){
            base->mCmdPortDisableMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandPortDisableMsg);  
        }
        base->mCmdPortDisableMsg->setUInt32(base->mCmdPortDisableMsg, "param", nParam1);
        base->mCmdPortDisableMsg->setPointer(base->mCmdPortDisableMsg, "cmd_data", pCmdData);

        base->mCmdPortDisableMsg->postMessage(base->mCmdPortDisableMsg, 0);
    }else if (Cmd == OMX_CommandPortEnable){
        if ( !base->mCmdPortEnableMsg ){
            base->mCmdPortEnableMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandPortEnableMsg);  
        }
        base->mCmdPortEnableMsg->setUInt32(base->mCmdPortEnableMsg, "param", nParam1);
        base->mCmdPortEnableMsg->setPointer(base->mCmdPortEnableMsg, "cmd_data", pCmdData);

        base->mCmdPortEnableMsg->postMessage(base->mCmdPortEnableMsg, 0);
    }else if (Cmd == OMX_CommandMarkBuffer){
        if ( !base->mCmdMarkBufferMsg ){
            base->mCmdMarkBufferMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandMarkBufferMsg);  
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

    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    // Mag_AcquireMutex(comp->mhMutex);
    ret = MagOMX_GetParameter_internal(hComponent, nParamIndex, pComponentParameterStructure);
    // Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_SetParameter(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){

    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    Mag_AcquireMutex(comp->mhMutex);
    ret = MagOMX_SetParameter_internal(hComponent, nIndex, pComponentParameterStructure);
    Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_GetConfig(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){

    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    // Mag_AcquireMutex(comp->mhMutex);
    ret = MagOMX_GetParameter_internal(hComponent, nIndex, pComponentConfigStructure);
    // Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_SetConfig(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){
                
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;

    comp = getBase(hComponent);
    
    Mag_AcquireMutex(comp->mhMutex);
    ret = MagOMX_SetParameter_internal(hComponent, nIndex, pComponentConfigStructure);
    Mag_ReleaseMutex(comp->mhMutex);

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

    MagOmxComponentImpl comp;
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

    MagOmxComponentImpl comp; 
    MagOmxPort port;
    MagOmxComponentImpl tunneledComp; 
    MagOmxPort tunneledPort;
    OMX_ERRORTYPE ret;
    OMX_HANDLETYPE portHandle;

    if ((NULL == hComponent) || (NULL == hTunneledComp)){
        AGILE_LOGE("Wrong parameters: hComponent(0x%p), hTunneledComp(0x%p)",
                    hComponent, hTunneledComp);
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, nPort), MagOmxPort);
    
    tunneledComp = getBase(hTunneledComp);
    tunneledPort = ooc_cast(tunneledComp->getPort(tunneledComp, nTunneledPort), MagOmxPort);

    Mag_AcquireMutex(comp->mhMutex);
    if ((comp->mState == OMX_StateLoaded) ||
        (!port->getDef_Enabled(port) && !tunneledPort->getDef_Enabled(tunneledPort))){
        ret = port->SetupTunnel(port, hTunneledComp, nTunneledPort, pTunnelSetup);
    }else{
        AGILE_LOGE("wrong state: %s!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_UseBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){
                
    MagOmxComponentImpl comp; 
    MagOmxPort port;
    OMX_ERRORTYPE ret;

    if ((NULL == hComponent) || (NULL == *ppBufferHdr)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, nPortIndex), MagOmxPort);

    Mag_AcquireMutex(comp->mhMutex);
    if ((comp->mState == OMX_StateLoaded) ||
        (comp->mState == OMX_StateWaitForResources) ||
        (!port->getDef_Enabled(port))){
        ret = MagOmxPortVirtual(port)->UseBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer);
    }else{
        AGILE_LOGE("wrong state: %s or on enabled port!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_AllocateBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){
                
    MagOmxComponentImpl comp; 
    MagOmxPort port;
    OMX_ERRORTYPE ret;

    if ((NULL == hComponent) || (NULL == *ppBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, nPortIndex), MagOmxPort);

    if (NULL == port){
        AGILE_LOGE("failed to get the port with index %d", nPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    Mag_AcquireMutex(comp->mhMutex);
    if ((comp->mState == OMX_StateLoaded) ||
        (comp->mState == OMX_StateWaitForResources) ||
        (!port->getDef_Enabled(port))){
        ret = MagOmxPortVirtual(port)->AllocateBuffer(port, ppBuffer, pAppPrivate, nSizeBytes);
    }else{
        AGILE_LOGE("wrong state: %s or on enabled port!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_FreeBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
                
    MagOmxComponentImpl comp; 
    MagOmxPort port;
    OMX_ERRORTYPE ret;

    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, nPortIndex), MagOmxPort);

    Mag_AcquireMutex(comp->mhMutex);
    if ((comp->mState  == OMX_StateIdle) ||
        ((comp->mState != OMX_StateLoaded) && (comp->mState != OMX_StateWaitForResources) && (!port->getDef_Enabled(port)))){
        ret = MagOmxPortVirtual(port)->FreeBuffer(port, pBuffer);
    }else{
        AGILE_LOGE("wrong state: %s or on enabled port!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    Mag_ReleaseMutex(comp->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_EmptyThisBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    MagOmxComponentImpl comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, pBuffer->nInputPortIndex), MagOmxPort);

    if (((comp->mState == OMX_StateExecuting) ||
        (comp->mState  == OMX_StatePause)) && 
        port->getDef_Enabled(port) &&
        port->isInputPort(port)){
        return MagOmxPortVirtual(port)->EmptyThisBuffer(port, pBuffer);
    }else{
        AGILE_LOGE("Error condition to do: state[%s], port enabled[%s], input port[%s]", 
                    OmxState2String(comp->mState),
                    port->getDef_Enabled(port) ? "Yes" : "No",
                    port->isInputPort(port) ? "Yes" : "No");
        return OMX_ErrorIncorrectStateOperation;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FillThisBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    MagOmxComponentImpl comp; 
    MagOmxPort port;
    
    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, pBuffer->nOutputPortIndex), MagOmxPort);

    if (((comp->mState == OMX_StateExecuting) ||
        (comp->mState  == OMX_StatePause)) && 
        port->getDef_Enabled(port) &&
        !port->isInputPort(port)){
        return MagOmxPortVirtual(port)->FillThisBuffer(port, pBuffer);
    }else{
        AGILE_LOGE("Error condition to do: state[%s], port enabled[%s], output port[%s]", 
                    OmxState2String(comp->mState),
                    port->getDef_Enabled(port) ? "Yes" : "No",
                    port->isInputPort(port) ? "No" : "Yes");
        return OMX_ErrorBadPortIndex;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_SetCallbacks(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){

    MagOmxComponent parent;
    MagOmxComponentImpl comp; 

    parent = ooc_cast(hComponent, MagOmxComponent);
    comp = getBase(hComponent);

    if (comp->mState != OMX_StateLoaded){
        AGILE_LOGE("in the wrong state: %s", OmxState2String(comp->mState));
        return OMX_ErrorIncorrectStateOperation;
    }

    parent->mpComponentObject->pApplicationPrivate = pAppData;
    comp->mpAppData   = pAppData;
    comp->mpCallbacks = pCallbacks;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_UseEGLImage(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){
    return OMX_ErrorNotImplemented;
}

/*TODO*/
static OMX_ERRORTYPE virtual_ComponentDeInit(
                OMX_IN  OMX_HANDLETYPE hComponent){

    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE ret;

    base = getBase(hComponent);

    Mag_AcquireMutex(base->mhMutex);
    if (MagOmxComponentImplVirtual(base)->MagOMX_Deinit){
        ret = MagOmxComponentImplVirtual(base)->MagOMX_Deinit(hComponent);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_Deinit() is not overrided!!");
        ret = OMX_ErrorNotImplemented;
    }
    Mag_ReleaseMutex(base->mhMutex);

    return ret;
}

static OMX_ERRORTYPE virtual_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){

    MagOmxComponentImpl base;

    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_ComponentRoleEnum){
        return MagOmxComponentImplVirtual(base)->MagOMX_ComponentRoleEnum(hComponent, cRole, nIndex);
    }else{
        AGILE_LOGE("pure virtual func: MagOMX_ComponentRoleEnum() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 
}

static OMX_COMPONENTTYPE *virtual_Create(
                OMX_IN OMX_HANDLETYPE hComponent, 
                OMX_IN OMX_PTR pAppData,
                OMX_IN OMX_CALLBACKTYPE *pCallbacks){

    OMX_COMPONENTTYPE *comp;
    MagOmxComponentImpl self;
    MagOmxComponent parent;
    
    parent = ooc_cast(hComponent, MagOmxComponent);
    comp = MagOmxComponentVirtual(parent)->Create(parent, pAppData);
    comp->pComponentPrivate = hComponent;

    self = getBase(hComponent);
    
    self->mpAppData   = pAppData;
    self->mpCallbacks = pCallbacks;

    return comp;
}

/*******************
* Member functions
********************/

static OMX_ERRORTYPE MagOmxComponentImpl_sendEvents(
                OMX_IN MagOmxComponentImpl hComponent,
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

static OMX_ERRORTYPE MagOmxComponentImpl_sendEmptyBufferDoneEvent(
                OMX_IN MagOmxComponentImpl hComponent,
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

static OMX_ERRORTYPE MagOmxComponentImpl_sendFillBufferDoneEvent(
                        OMX_IN MagOmxComponentImpl hComponent,
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

static MagMessageHandle MagOmxComponentImpl_createMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentImpl hComponent = NULL;
    
    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentImpl);
    hComponent->getLooper(handle);
    
    MagMessageHandle msg = createMagMessage(hComponent->mLooper, what, hComponent->mMsgHandler->id(hComponent->mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static MagMessageHandle MagOmxComponentImpl_createBufferMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentImpl hComponent = NULL;
    
    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentImpl);  
    hComponent->getBufferLooper(handle);
    
    MagMessageHandle msg = createMagMessage(hComponent->mBufferLooper, what, hComponent->mBufferMsgHandler->id(hComponent->mBufferMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxComponentImpl_getLooper(OMX_HANDLETYPE handle){
    MagOmxComponentImpl hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentImpl);
    
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

static _status_t MagOmxComponentImpl_getBufferLooper(OMX_HANDLETYPE handle){
    MagOmxComponentImpl hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentImpl);
    
    if ((NULL != hComponent->mBufferLooper) && (NULL != hComponent->mBufferMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mBufferLooper){
        hComponent->mBufferLooper = createLooper(BUFFER_LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mBufferLooper);
    }
    
    if (NULL != hComponent->mBufferLooper){
        if (NULL == hComponent->mBufferMsgHandler){
            hComponent->mBufferMsgHandler = createHandler(hComponent->mBufferLooper, onBufferMessageReceived, handle);

            if (NULL != hComponent->mBufferMsgHandler){
                hComponent->mBufferLooper->registerHandler(hComponent->mBufferLooper, hComponent->mBufferMsgHandler);
                hComponent->mBufferLooper->start(hComponent->mBufferLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", BUFFER_LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

static OMX_ERRORTYPE MagOmxComponentImpl_setState(OMX_HANDLETYPE handle, OMX_STATETYPE target_state){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    MagOmxComponentImpl base;
    MagOmxComponent     root;

    if (!handle){
        AGILE_LOGE("handle is NULL");
        return OMX_ErrorBadParameter;
    }

    root = getRoot(handle);
    base = getBase(handle);

    AGILE_LOGV("Try (current state:%s --> target state:%s)", OmxState2String(base->mState), OmxState2String(target_state));
    
    if ((target_state < OMX_StateLoaded) || (target_state > OMX_StateWaitForResources)){
        AGILE_LOGE("Invalid state transition: current state:%s --> target state:0x%x", OmxState2String(base->mState), target_state);
        return OMX_ErrorIncorrectStateTransition;
    }

    if (base->mState == OMX_StateMax){
        if (target_state == OMX_StateLoaded){
            base->mState = OMX_StateLoaded;
            return OMX_ErrorNone;
        }else{
            AGILE_LOGE("Invalid state transition: current state:OMX_StateMax --> target state:%s", OmxState2String(target_state));
            return OMX_ErrorIncorrectStateTransition;
        }
    }

    if (base->mStateTransitTable[toIndex(base->mState)][toIndex(target_state)]){
        ret = base->mStateTransitTable[toIndex(base->mState)][toIndex(target_state)](handle);
        if (ret == OMX_ErrorNone){
            AGILE_LOGV("Transit current state:%s --> target state:%s -- OK!", OmxState2String(base->mState), OmxState2String(target_state))
            base->mState = target_state;
        }else{
            AGILE_LOGE("Transit current state:%s --> target state:%s -- Failure!", OmxState2String(base->mState), OmxState2String(target_state))
            if ((base->mState == OMX_StateLoaded) && (target_state == OMX_StateIdle)){
                base->mState = OMX_StateWaitForResources;
            }
        }
    }else{
        AGILE_LOGE("Invalid state transition: current state:%s --> target state:%s", OmxState2String(base->mState), OmxState2String(target_state));
        return OMX_ErrorIncorrectStateTransition;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentImpl_flushPort(OMX_HANDLETYPE handle, OMX_U32 port_index){
    RBTreeNodeHandle n;
    MagOmxPort port;
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (NULL != handle){
        comp = getBase(handle);
        comp->mFlushingPorts = port_index;

        if (port_index == OMX_ALL){
            for (n = rbtree_first(comp->mPortTreeRoot); n; n = rbtree_next(n)) {
                port = ooc_cast(n->value, MagOmxPort);
                if (port->getDef_Enabled(port)){
                    ret = MagOmxPortVirtual(port)->Flush(n->value);
                    if (ret != OMX_ErrorNone){
                        AGILE_LOGE("failed to flush the port[OMX_ALL]: %d", port->getPortIndex(port));
                        break;
                    }
                }else{
                    AGILE_LOGD("the port %d is disabled, ignore the flush action [OMX_ALL]!", port->getPortIndex(port));
                }
            }
        }else{
            OMX_HANDLETYPE portHandle;

            portHandle = comp->getPort(comp, port_index);
            port = ooc_cast(portHandle, MagOmxPort);

            if (port->getDef_Enabled(port)){
                ret = MagOmxPortVirtual(port)->Flush(portHandle);
                if (ret != OMX_ErrorNone){
                    AGILE_LOGE("failed to flush the port: %d", port->getPortIndex(port));
                }
            }else{
                AGILE_LOGD("the port %d is disabled, ignore the flush action!", port->getPortIndex(port));
            }
        }
    }else{
        ret = OMX_ErrorBadParameter;
    }

    comp->mFlushingPorts = kInvalidPortIndex;
    comp->sendEvents(comp, OMX_EventCmdComplete, OMX_CommandFlush, port_index, &ret);
    return ret;;
}

static OMX_ERRORTYPE MagOmxComponentImpl_enablePort(OMX_HANDLETYPE handle, OMX_U32 port_index){  
    RBTreeNodeHandle n;          
    MagOmxPort port;
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (NULL != handle){
        comp = getBase(handle);
        
        if (port_index == OMX_ALL){
            for (n = rbtree_first(comp->mPortTreeRoot); n; n = rbtree_next(n)) {
                port = ooc_cast(n->value, MagOmxPort);
                if ((comp->mState == OMX_StateLoaded) ||
                    (comp->mState == OMX_StateWaitForResources)){
                    port->setDef_Enabled(port, OMX_TRUE);
                }else if (comp->mState == OMX_StateIdle){
                    ret = MagOmxPortVirtual(port)->Enable(n->value, comp->mpAppData);
                    if (ret != OMX_ErrorNone){
                        AGILE_LOGE("failed to enable the port[OMX_ALL]: %d in state Idle", port->getPortIndex(port));
                        break;
                    }
                }else if (comp->mState == OMX_StateExecuting){
                    ret = MagOmxPortVirtual(port)->Enable(n->value, comp->mpAppData);
                    if (ret == OMX_ErrorNone){
                        ret = MagOmxPortVirtual(port)->Run(n->value);
                        if (ret != OMX_ErrorNone){
                            AGILE_LOGE("failed to run the port[OMX_ALL] %d", port->getPortIndex(port));
                            break;
                        }
                    }else{
                        AGILE_LOGE("failed to enable the port[OMX_ALL]: %d in state Executing", port->getPortIndex(port));
                        break;
                    }
                }
            }
        }else{
            OMX_HANDLETYPE portHandle;

            portHandle = comp->getPort(comp, port_index);
            port = ooc_cast(portHandle, MagOmxPort);

            if ((comp->mState == OMX_StateLoaded) ||
                (comp->mState == OMX_StateWaitForResources)){
                port->setDef_Enabled(port, OMX_TRUE);
            }else if ((comp->mState == OMX_StateIdle) ||
                      (comp->mState == OMX_StateExecuting)){
                ret = MagOmxPortVirtual(port)->Enable(portHandle, comp->mpAppData);
                if (ret != OMX_ErrorNone){
                    AGILE_LOGE("failed to enable the port: %d in state Idle/Executing", port->getPortIndex(port));
                }else{
                    if (comp->mState == OMX_StateExecuting){
                        ret = MagOmxPortVirtual(port)->Run(portHandle);
                        if (ret != OMX_ErrorNone){
                            AGILE_LOGE("failed to run the port %d", port->getPortIndex(port));
                        }
                    }
                }
            }
        }
    }else{
        ret = OMX_ErrorBadParameter;
    }

    comp->sendEvents(comp, OMX_EventCmdComplete, OMX_CommandPortEnable, port_index, &ret);
    return ret;
}


static OMX_ERRORTYPE MagOmxComponentImpl_disablePort(OMX_HANDLETYPE handle, OMX_U32 port_index){
    RBTreeNodeHandle n;          
    MagOmxPort port;
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (NULL != handle){
        comp = getBase(handle);
        
        if (port_index == OMX_ALL){
            for (n = rbtree_first(comp->mPortTreeRoot); n; n = rbtree_next(n)) {
                port = ooc_cast(n->value, MagOmxPort);
                if ((comp->mState == OMX_StateLoaded) ||
                    (comp->mState == OMX_StateWaitForResources)){
                    port->setDef_Enabled(port, OMX_FALSE);
                }else if (comp->mState == OMX_StateIdle){
                    ret = MagOmxPortVirtual(port)->Disable(n->value);
                    if (ret != OMX_ErrorNone){
                        AGILE_LOGE("failed to disable the port[OMX_ALL]: %d in state Idle", port->getPortIndex(port));
                        break;
                    }
                }else if ((comp->mState == OMX_StateExecuting) ||
                          (comp->mState == OMX_StatePause)){
                    ret = MagOmxPortVirtual(port)->Flush(n->value);
                    if (ret == OMX_ErrorNone){
                        ret = MagOmxPortVirtual(port)->Disable(n->value);
                        if (ret != OMX_ErrorNone){
                            AGILE_LOGE("failed to disable the port[OMX_ALL]: %d in state Executing", port->getPortIndex());
                            break;
                        }
                    }else{
                        AGILE_LOGE("failed to flush the port[OMX_ALL]: %d in state Executing", port->getPortIndex());
                        break;
                    }
                }
            }
        }else{
            OMX_HANDLETYPE portHandle;

            portHandle = comp->getPort(comp, port_index);
            port = ooc_cast(portHandle, MagOmxPort);
            if ((comp->mState == OMX_StateLoaded) ||
                (comp->mState == OMX_StateWaitForResources)){
                port->setDef_Enabled(port, OMX_FALSE);
            }else if (comp->mState == OMX_StateIdle){
                ret = MagOmxPortVirtual(port)->Disable(portHandle);
                if (ret != OMX_ErrorNone){
                    AGILE_LOGE("failed to disable the port: %d in state Idle", port->getPortIndex(port));
                    break;
                }
            }else if ((comp->mState == OMX_StateExecuting) ||
                      (comp->mState == OMX_StatePause)){
                ret = MagOmxPortVirtual(port)->Flush(portHandle);
                if (ret == OMX_ErrorNone){
                    ret = MagOmxPortVirtual(port)->Disable(portHandle);
                    if (ret != OMX_ErrorNone){
                        AGILE_LOGE("failed to disable the port: %d in state Executing", port->getPortIndex(port));
                        break;
                    }
                }else{
                    AGILE_LOGE("failed to flush the port: %d in state Executing", port->getPortIndex(port));
                    break;
                }
            }
        }
    }else{
        ret = OMX_ErrorBadParameter;
    }

    comp->sendEvents(comp, OMX_EventCmdComplete, OMX_CommandPortDisable, port_index, &ret);
    return ret;
}

static void MagOmxComponentImpl_addPort(MagOmxComponentImpl hComponent, 
                                        OMX_U32 portIndex, 
                                        OMX_HANDLETYPE hPort){

    if ((NULL == hPort) || (NULL == hComponent)){
        return OMX_ErrorBadParameter;
    }

    hComponent->mPortTreeRoot =  rbtree_insert(hComponent->mPortTreeRoot, portIndex, hPort);
}

static OMX_HANDLETYPE MagOmxComponentImpl_getPort(MagOmxComponentImpl hComponent, 
                                                  OMX_U32 portIndex){

    OMX_HANDLETYPE hPort = NULL;
    
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    hPort = rbtree_get(hComponent->mPortTreeRoot, portIndex);

    if (!hPort){
        AGILE_LOGE("failed to get the port(%d) of the component:0x%p", portIndex, hComponent);
        return NULL;
    }

    return (hPort);
}

/*setup the port connection inside the component*/
static OMX_ERRORTYPE MagOmxComponentImpl_setupPortDataFlow(
                                        OMX_IN MagOmxComponentImpl hComponent,
                                        OMX_IN MagOmxPort srcPort,
                                        OMX_IN MagOmxPort destPort){
    OMX_ERRORTYPE err;

    if (srcPort == NULL){
        AGILE_LOGE("invalid parameter: srcPort is NULL");
        return OMX_ErrorBadParameter;
    }

    if (srcPort->getBufferPolicy(srcPort) == kSharedBuffer){
        if (destPort && destPort->isBufferSupplier(destPort)){
            MagMessageHandle msg;
            msg = destPort->GetSharedBufferMsg(destPort);
            if (msg){
                err = srcPort->RegisterBufferHandler(srcPort, msg);
                if (err != OMX_ErrorNone){
                    AGILE_LOGE("failed to setup the shared buffer!");
                    return err;
                }
            }
        }else{
            AGILE_LOGE("Failed! %s for shared buffer setup", destPort == NULL ? "The destPort is NULL" : "The destPort is none-supplier");
            return OMX_ErrorBadParameter;
        }
    }else{
        MagMessageHandle msg = NULL;

        if (hComponent->mPortDataMsgList == NULL){
            hComponent->mPortDataMsgList = mag_mallocz(sizeof(MagMessageHandle) * hComponent->mPorts);
        }

        if (hComponent->mPortDataMsgList[srcPort->getPortIndex(srcPort)] == NULL){
            msg = hComponent->createBufferMessage(hComponent, MagOmxComponentImpl_PortDataFlowMsg + srcPort->getPortIndex(srcPort));
            hComponent->mPortDataMsgList[srcPort->getPortIndex(srcPort)] = msg;
        }else{
            msg = hComponent->mPortDataMsgList[srcPort->getPortIndex(srcPort)];
        }
        
        if (msg){
            msg->setPointer(msg, "destination_port", destPort, MAG_FALSE);
            err = srcPort->RegisterBufferHandler(srcPort, msg);
            if (err != OMX_ErrorNone){
                AGILE_LOGE("failed to register the message to port!");
                return err;
            }
        }else{
            AGILE_LOGE("the created message is NULL");
            return OMX_ErrorInsufficientResources;
        }
    }

    return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/

static void MagOmxComponentImpl_initialize(Class this){
    /*Override the base component pure virtual functions*/
    MagOmxComponentImplVtableInstance.MagOmxComponent.GetComponentVersion    = virtual_GetComponentVersion;
    MagOmxComponentImplVtableInstance.MagOmxComponent.SendCommand            = virtual_SendCommand;
    MagOmxComponentImplVtableInstance.MagOmxComponent.GetParameter           = virtual_GetParameter;
    MagOmxComponentImplVtableInstance.MagOmxComponent.SetParameter           = virtual_SetParameter;
    MagOmxComponentImplVtableInstance.MagOmxComponent.GetConfig              = virtual_GetConfig;
    MagOmxComponentImplVtableInstance.MagOmxComponent.SetConfig              = virtual_SetConfig;
    MagOmxComponentImplVtableInstance.MagOmxComponent.GetExtensionIndex      = virtual_GetExtensionIndex;
    MagOmxComponentImplVtableInstance.MagOmxComponent.GetState               = virtual_GetState;
    MagOmxComponentImplVtableInstance.MagOmxComponent.ComponentTunnelRequest = virtual_ComponentTunnelRequest;
    MagOmxComponentImplVtableInstance.MagOmxComponent.UseBuffer              = virtual_UseBuffer;
    MagOmxComponentImplVtableInstance.MagOmxComponent.AllocateBuffer         = virtual_AllocateBuffer;
    MagOmxComponentImplVtableInstance.MagOmxComponent.FreeBuffer             = virtual_FreeBuffer;
    MagOmxComponentImplVtableInstance.MagOmxComponent.EmptyThisBuffer        = virtual_EmptyThisBuffer;
    MagOmxComponentImplVtableInstance.MagOmxComponent.FillThisBuffer         = virtual_FillThisBuffer;
    MagOmxComponentImplVtableInstance.MagOmxComponent.SetCallbacks           = virtual_SetCallbacks; 
    MagOmxComponentImplVtableInstance.MagOmxComponent.ComponentDeInit        = virtual_ComponentDeInit;
    MagOmxComponentImplVtableInstance.MagOmxComponent.ComponentRoleEnum      = virtual_ComponentRoleEnum;
    MagOmxComponentImplVtableInstance.MagOmxComponent.UseEGLImage            = virtual_UseEGLImage;
    
    MagOmxComponentImplVtableInstance.Create                                 = virtual_Create;
    
    /*pure virtual functions to be overrided by sub-components*/
    MagOmxComponentImplVtableInstance.MagOMX_GetComponentUUID                = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Prepare                         = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Preroll                         = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Start                           = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Stop                            = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Pause                           = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Resume                          = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Deinit                          = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Reset                           = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_GetParameter                    = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_SetParameter                    = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_ComponentRoleEnum               = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_ProceedBuffer                   = NULL;
}

/*
 * params[0]: the index of the first port(mStartPortNumber)
 * params[1]: the number of ports(mPorts)
 */
static void MagOmxComponentImpl_constructor(MagOmxComponentImpl thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmxComponentImpl));
    chain_constructor(MagOmxComponentImpl, thiz, params);

    thiz->createMessage            = MagOmxComponentImpl_createMessage;
    thiz->getLooper                = MagOmxComponentImpl_getLooper;
    thiz->createBufferMessage      = MagOmxComponentImpl_createBufferMessage;
    thiz->getBufferLooper          = MagOmxComponentImpl_getBufferLooper;
    thiz->setState                 = MagOmxComponentImpl_setState;
    thiz->flushPort                = MagOmxComponentImpl_flushPort;
    thiz->enablePort               = MagOmxComponentImpl_enablePort;
    thiz->disablePort              = MagOmxComponentImpl_disablePort;
    thiz->addPort                  = MagOmxComponentImpl_addPort;
    thiz->getPort                  = MagOmxComponentImpl_getPort;
    thiz->sendEvents               = MagOmxComponentImpl_sendEvents;
    thiz->sendEmptyBufferDoneEvent = MagOmxComponentImpl_sendEmptyBufferDoneEvent;
    thiz->sendFillBufferDoneEvent  = MagOmxComponentImpl_sendFillBufferDoneEvent;
    thiz->setupPortDataFlow        = MagOmxComponentImpl_setupPortDataFlow;

    thiz->mLooper                  = NULL;
    thiz->mMsgHandler              = NULL;
    thiz->mBufferLooper            = NULL;
    thiz->mBufferMsgHandler        = NULL;
    thiz->mCmdSetStateMsg          = NULL;
    thiz->mCmdPortDisableMsg       = NULL;
    thiz->mCmdPortEnableMsg        = NULL;
    thiz->mCmdFlushMsg             = NULL;
    thiz->mCmdMarkBufferMsg        = NULL;
    thiz->mPortDataMsgList         = NULL;

    /*set initial state as OMX_StateMax*/
    thiz->mState                   = OMX_StateMax;
    thiz->mStateTransitTable[5][5] = { {NULL, doStateLoaded_Idle, NULL, NULL, doStateLoaded_WaitforResources},
                                       {doStateIdle_Loaded, NULL, doStateIdle_Executing, doStateIdle_Pause, NULL},
                                       {NULL, doStateExecuting_Idle, NULL, doStateExecuting_Pause, NULL},
                                       {NULL, doStatePause_Idle, doStatePause_Executing, NULL, NULL},
                                       {doStateWaitforResources_Loaded, doStateWaitforResources_Idle, NULL, NULL, NULL} };

    thiz->mParametersDB            = createMagMiniDB(64);

    thiz->mPortTreeRoot            = NULL;
    thiz->mpCallbacks              = NULL;
    thiz->mpAppData                = NULL;

    thiz->mStartPortNumber         = *((OMX_U32 *)params + 0);
    thiz->mPorts                   = *((OMX_U32 *)params + 1);

    thiz->mFlushingPorts           = kInvalidPortIndex;
    
    Mag_CreateMutex(&thiz->mhMutex);
}

static void MagOmxComponentImpl_destructor(MagOmxComponentImpl thiz, MagOmxComponentImplVtable vtab){
    AGILE_LOGV("Enter!");
    OMX_U32 i;

    destroyMagMiniDB(thiz->mParametersDB);
    Mag_DestroyMutex(thiz->mhMutex);

    destroyMagMessage(mCmdSetStateMsg);
    destroyMagMessage(mCmdPortDisableMsg);
    destroyMagMessage(mCmdPortEnableMsg);
    destroyMagMessage(mCmdFlushMsg);
    destroyMagMessage(mCmdMarkBufferMsg);
    if (mPortDataMsgList){
        for (i = 0; i < thiz->mPorts; i++){
            destroyMagMessage(mPortDataMsgList[i]);
        }
        mag_free(mPortDataMsgList);
    }

    destroyLooper(thiz->mLooper);
    destroyHandler(thiz->mMsgHandler);

    destroyLooper(thiz->mBufferLooper);
    destroyHandler(thiz->mBufferMsgHandler);
}


