#include "MagOMX_Component_baseImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompBase"

#define LOOPER_NAME        "CompImplLooper"
#define BUFFER_LOOPER_NAME "CompImplBufLooper"

AllocateClass(MagOmxComponentImpl, MagOmxComponent);

static MagOmxComponent getRoot(OMX_HANDLETYPE hComponent) {
    return ooc_cast(hComponent, MagOmxComponent);
}

static MagOmxComponentImpl getBase(OMX_HANDLETYPE hComponent) {
    return ooc_cast(hComponent, MagOmxComponentImpl);
}

static OMX_BOOL isPortParam(OMX_IN OMX_INDEXTYPE nParamIndex){
    if (nParamIndex > OMX_IndexAudioStartUnused){
        if (nParamIndex < OMX_IndexOtherStartUnused){
            return OMX_TRUE;
        }else{
            switch (nParamIndex){
                case OMX_IndexParamOtherPortFormat:
                case OMX_IndexConfigTimeCurrentMediaTime:
                case OMX_IndexConfigTimeCurrentWallTime:
                case OMX_IndexConfigTimeMediaTimeRequest:
                case OMX_IndexConfigTimeClientStartTime:
                case OMX_IndexConfigTimePosition:
                case OMX_IndexConfigTimeCurrentReference:
                case OMX_IndexConfigTimeRenderingDelay:
                case OMX_IndexConfigCallbackRequest:
                case OMX_IndexParamReadOnlyBuffers:
                    return OMX_TRUE;
                default:
                    return OMX_FALSE;
            }
        }
    }else{
        if(nParamIndex < OMX_IndexPortStartUnused){
            return OMX_FALSE;
        }else{
            return OMX_TRUE;
        }
    } 
}

static void onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    OMX_U32 param;
    OMX_PTR cmd_data = NULL;
    OMX_ERRORTYPE ret;
    OMX_U32 cmd;
    MagOmxPort port;
    OMX_BUFFERHEADERTYPE *bufHeader;
    MagMessageHandle returnBufMsg;

    root = getRoot(priv);
    base = getBase(priv);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    if (!msg->findUInt32(msg, "param", &param)){
        COMP_LOGE(root, "failed to find the param!");
        return;
    }

    if (!msg->findPointer(msg, "cmd_data", &cmd_data)){
        COMP_LOGE(root, "failed to find the cmd_data!");
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentImpl_CommandStateSetMsg:
            {
            OMX_STATETYPE target_state = (OMX_STATETYPE)param;
            /*Mag_AcquireMutex(base->mhMutex);*/
            ret = base->setState(priv, target_state);
            /*Mag_ReleaseMutex(base->mhMutex);*/
            base->sendEvents(priv, OMX_EventCmdComplete, OMX_CommandStateSet, target_state, &ret);
            }
            break;

        case MagOmxComponentImpl_CommandFlushMsg:
            {
            /*Mag_AcquireMutex(base->mhMutex);*/
            base->flushPort(priv, param);
            /*Mag_ReleaseMutex(base->mhMutex);
            base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandFlush, param, &ret);*/
            }
            break;

        case MagOmxComponentImpl_CommandPortDisableMsg:
            {
            /*Mag_AcquireMutex(base->mhMutex);   */ 
            base->disablePort(priv, param);
            /*Mag_ReleaseMutex(base->mhMutex);
            base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandPortDisable, param, &ret);*/
            }
            break;

        case MagOmxComponentImpl_CommandPortEnableMsg:
            {
            /*Mag_AcquireMutex(base->mhMutex);*/
            base->enablePort(priv, param);
            /*Mag_ReleaseMutex(base->mhMutex);
            base->sendEvents(base, OMX_EventCmdComplete, OMX_CommandPortEnable, param, &ret);*/
            }
            break;
            
        case MagOmxComponentImpl_CommandMarkBufferMsg:
            COMP_LOGD(root, "the CommandMarkBuffer is not implemented!");
            break;

        default:
            break;
    }
}

static void onBufferMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_U32 cmd;
    MagOmxPort port;
    OMX_BUFFERHEADERTYPE *srcBufHeader = NULL;
    OMX_BUFFERHEADERTYPE *destBufHeader = NULL;
    MagMessageHandle returnBufMsg = NULL;
    OMX_BOOL isFlushing = OMX_FALSE;

    OMX_HANDLETYPE hDestPort = NULL;
    OMX_HANDLETYPE hSrcPort  = NULL;
    MagMessageHandle outputBufMsg = NULL;
    
    base = getBase(priv);
    root = getRoot(priv);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    cmd = msg->what(msg);

    COMP_LOGV(root, "get command: %d", cmd);
    if ((cmd >= MagOmxComponentImpl_PortDataFlowMsg) &&
        (cmd < MagOmxComponentImpl_PortDataFlowMsg + base->mPorts)){
        if (!msg->findPointer(msg, "buffer_header", (void **)&srcBufHeader)){
            COMP_LOGE(root, "[msg]: failed to find the buffer_header!");
            return;
        }

        if (!msg->findPointer(msg, "destination_port", (void **)&hDestPort)){
            COMP_LOGE(root, "failed to find the destination_port! The component might not have the output port");
            return;
        }

        if (!msg->findPointer(msg, "source_port", (void **)&hSrcPort)){
            COMP_LOGE(root, "failed to find the source_port! The component might not have the return port");
            return;
        }

        if (base->mFlushingPorts != kInvalidPortIndex){
            if (base->mFlushingPorts == OMX_ALL){
                isFlushing = OMX_TRUE;
            }else if ((base->mFlushingPorts == srcBufHeader->nOutputPortIndex) ||
                      (base->mFlushingPorts == srcBufHeader->nInputPortIndex)){
                isFlushing = OMX_TRUE;
            }
        }

        if ((base->mTransitionState == OMX_TransitionStateToIdle) ||
            (base->mState == OMX_StateIdle)){
            COMP_LOGD(root, "directly return the buffer!");
            /*return back the buffer immediately*/
            if (msg->findMessage(msg, "return_buf_msg", &returnBufMsg)){
                returnBufMsg->setPointer(returnBufMsg, "buffer_header", srcBufHeader, MAG_FALSE);
                returnBufMsg->setPointer(returnBufMsg, "component_obj", priv, MAG_FALSE);
                returnBufMsg->postMessage(returnBufMsg, 0);
            }else{
                COMP_LOGE(root, "failed to find the return_buf_msg!");
            }
            return;
        }

        if (MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer){
            if (!isFlushing){
                /*if (hDestPort){
                    base->putOutputBuffer(base, hDestPort, srcBufHeader);
                }*/

                /*do not return back the buffer at current moment*/
                if (msg->findMessage(msg, "return_buf_msg", &returnBufMsg)){
                    base->putReturnBuffer(base, hSrcPort, srcBufHeader, returnBufMsg, priv);
                }else{
                    COMP_LOGE(root, "failed to find the return_buf_msg!");
                }

                ret = MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer(base, 
                                                                             srcBufHeader,
                                                                             hDestPort);
                
                /*if (hDestPort){
                    outputBufMsg = MagOmxPortVirtual(destPort)->GetOutputBufferMsg(hDestPort);
                    if (outputBufMsg){
                        if (!outputBufMsg->findPointer(outputBufMsg, "buffer_header", (void **)&destBufHeader)){
                            COMP_LOGE(root, "[outputBufMsg]: failed to find the buffer_header!");
                            return;
                        }
                        ret = MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer(base, 
                                                                                     srcBufHeader,
                                                                                     hDestPort);
                        outputBufMsg->postMessage(outputBufMsg, 0);
                    }else{
                        COMP_LOGE(root, "failed to get outputBufMsg");
                    }
                }else{
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_ProceedBuffer(base, 
                                                                                 srcBufHeader,
                                                                                 NULL);
                }*/
            }

            if (ret == OMX_ErrorNone){
                /*return back the buffer immediately*/
                base->sendReturnBuffer(base, srcBufHeader);
            }
            #if 0
                /*return back the buffer immediately*/
                if (msg->findMessage(msg, "return_buf_msg", &returnBufMsg)){
                    returnBufMsg->setPointer(returnBufMsg, "buffer_header", srcBufHeader, MAG_FALSE);
                    returnBufMsg->setPointer(returnBufMsg, "component_obj", priv, MAG_FALSE);
                    returnBufMsg->postMessage(returnBufMsg, 0);
                }else{
                    COMP_LOGE(root, "[2]failed to find the return_buf_msg!");
                }
            #endif
        }else{
            COMP_LOGE(root, "pure virtual func: MagOMX_ProceedBuffer() is not overrided!!");
        }
    }else{
        COMP_LOGE(root, "Wrong commands(%d)!", cmd);
    }
}

/********************************
 *State Transition Actions
 ********************************/
static OMX_ERRORTYPE doPrerollStateLoaded_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToIdle;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStateLoaded_WaitforResources(OMX_IN OMX_HANDLETYPE hComponent){
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStateIdle_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToLoaded;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStateIdle_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent root;
    MagOmxComponentImpl base;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToExecuting;

     if (MagOmxComponentImplVirtual(base)->MagOMX_Start){
        err = MagOmxComponentImplVirtual(base)->MagOMX_Start(hComponent);

        if (err != OMX_ErrorNone){
            COMP_LOGE(root, "failed to start the component!");
            return err;
        }
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStateIdle_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToPause;
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStateExecuting_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToIdle;

    if (MagOmxComponentImplVirtual(base)->MagOMX_Stop){
        return MagOmxComponentImplVirtual(base)->MagOMX_Stop(hComponent);
    }

    return OMX_ErrorNone; 
}

static OMX_ERRORTYPE doPrerollStateExecuting_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToPause;
    
    if (MagOmxComponentImplVirtual(base)->MagOMX_Pause){
        MagOmxComponentImplVirtual(base)->MagOMX_Pause(hComponent);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStatePause_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToIdle;
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStatePause_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToExecuting;
    
    if (MagOmxComponentImplVirtual(base)->MagOMX_Resume){
        return MagOmxComponentImplVirtual(base)->MagOMX_Resume(hComponent);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doPrerollStateWaitforResources_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToLoaded;
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doSPrerolltateWaitforResources_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponentImpl base;

    base = getBase(hComponent);
    base->mTransitionState = OMX_TransitionStateToIdle;
    
    return OMX_ErrorNone;
}


static OMX_ERRORTYPE doStateLoaded_WaitforResources(OMX_IN OMX_HANDLETYPE hComponent){
    COMP_LOGV(getRoot(hComponent), "Enter!");
    return OMX_ErrorNone;
}

/*Acquire all of static resources*/
static OMX_ERRORTYPE doStateLoaded_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    COMP_LOGV(root, "enter!");
    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        COMP_LOGV(root, "Traverse port:0x%x, index:%d", port, port->getPortIndex(port));
        if (port->isTunneled(port)){
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->AllocateTunnelBuffer(n->value);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(root, "failed to allocate the tunnel buffers!");
                    return err;
                }
            }else{
                COMP_LOGE(root, "The tunneled port is not enabled!");
            }
        }else{
            if (port->getDef_Enabled(port)){
                if (!port->getDef_Populated(port)){
                    COMP_LOGE(root, "The port %lld is unpopulated!", n->key);
                    return OMX_ErrorPortUnpopulated;
                }
            }else{
                COMP_LOGE(root, "The non-tunneled port is not enabled!");
            }
        }
    }

    if (MagOmxComponentImplVirtual(base)->MagOMX_Prepare)
        /*if the return is OMX_ErrorInsufficientResources, IL Client may elect to transition the component to  WaitforResources state*/
        return MagOmxComponentImplVirtual(base)->MagOMX_Prepare(hComponent);
    else{
        COMP_LOGE(root, "pure virtual func: MagOMX_Prepare() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   
    
    return OMX_ErrorNone; 
}

static OMX_ERRORTYPE doStateIdle_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        if (port->isTunneled(port)){
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->FreeTunnelBuffer(n->value);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(root, "failed to free the tunnel buffers from port %lld!", n->key);;
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
        COMP_LOGE(root, "pure virtual func: MagOMX_Reset() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 

    return OMX_ErrorNone;  
}

static OMX_ERRORTYPE doStateIdle_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        if (port->getDef_Enabled(port)){
            err = MagOmxPortVirtual(port)->Run(n->value);
            if (err != OMX_ErrorNone){
                COMP_LOGE(root, "failed to start the port data flow!");
                return err;
            }
        }
    }
    base->mbGetStartTime = OMX_FALSE;
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStateIdle_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    COMP_LOGD(root, "why do we need the state transition: Idle --> Pause??");

    if (MagOmxComponentImplVirtual(base)->MagOMX_Preroll){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Pause(n->value);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(root, "failed to flush the port!");
                    return err;
                }
            }
        }

        return MagOmxComponentImplVirtual(base)->MagOMX_Preroll(hComponent);
    }else{
        COMP_LOGE(root, "pure virtual func: MagOMX_Preroll() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }   

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStateExecuting_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    OMX_ERRORTYPE err;
    RBTreeNodeHandle n;
    MagOmxPort port;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Stop){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Flush(n->value);
                port->setState(port, kPort_State_Stopped);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(root, "failed to flush the port!");
                    return err;
                }
            }
        }

        /*return MagOmxComponentImplVirtual(base)->MagOMX_Stop(hComponent);*/
    }else{
        COMP_LOGE(root, "pure virtual func: MagOMX_Stop() is not overrided!!");
        return OMX_ErrorNotImplemented;
    }  

    return OMX_ErrorNone; 
}

static OMX_ERRORTYPE doStateExecuting_Pause(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);
    
    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        if (port->getDef_Enabled(port)){
            err = MagOmxPortVirtual(port)->Pause(n->value);
            if (err != OMX_ErrorNone){
                COMP_LOGE(root, "failed to pause the port!");
                return err;
            }
        }
    }

    return OMX_ErrorNone; 
}

static OMX_ERRORTYPE doStatePause_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    if (MagOmxComponentImplVirtual(base)->MagOMX_Stop){
        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Resume(n->value);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(root, "failed to resume the port!");
                    return err;
                }
            }
        }

        for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
            port = ooc_cast(n->value, MagOmxPort);
            if (port->getDef_Enabled(port)){
                err = MagOmxPortVirtual(port)->Flush(n->value);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(root, "failed to flush the port!");
                    return err;
                }
                port->setState(port, kPort_State_Stopped);
            }
        }

        return MagOmxComponentImplVirtual(base)->MagOMX_Stop(hComponent);
    }else{
        COMP_LOGE(root, "pure virtual func: MagOMX_Stop() is not overrided!!");
        return OMX_ErrorNotImplemented;
    } 

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStatePause_Executing(OMX_IN OMX_HANDLETYPE hComponent){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    RBTreeNodeHandle n;
    MagOmxPort port;
    OMX_ERRORTYPE err;

    root = getRoot(hComponent);
    base = getBase(hComponent);

    for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
        port = ooc_cast(n->value, MagOmxPort);
        if (port->getDef_Enabled(port)){
            err = MagOmxPortVirtual(port)->Resume(n->value);
            if (err != OMX_ErrorNone){
                COMP_LOGE(root, "failed to resume the port!");
                return err;
            }
        }
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStateWaitforResources_Loaded(OMX_IN OMX_HANDLETYPE hComponent){
    COMP_LOGD(getRoot(hComponent), "Don't support the state transition: WaitforResources --> Loaded");
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE doStateWaitforResources_Idle(OMX_IN OMX_HANDLETYPE hComponent){
    COMP_LOGD(getRoot(hComponent), "Don't support the state transition: WaitforResources --> Idle");
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
    if ((state == OMX_StateLoaded) ||
        (state > OMX_StateLoaded && !port->getDef_Enabled(port)))
        return OMX_TRUE;
    else
        return OMX_FALSE;
}

static OMX_ERRORTYPE MagOMX_SetParameter_internal(
                        OMX_IN  OMX_HANDLETYPE hComponent, 
                        OMX_IN  OMX_INDEXTYPE nIndex,
                        OMX_IN  OMX_PTR pComponentParameterStructure){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    MagOMX_Param_Header_t *header;

    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    root = getRoot(hComponent);
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

            if (base->mState != OMX_StateLoaded){
                COMP_LOGE(root, "disallow parameter(%d) setting in state %s!", 
                                 nIndex, OmxState2String(base->mState));
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
            COMP_LOGE(root, "OMX IL should not set %s to the component!", OmxParameter2String(nIndex));
        }
            break;

        /*Container parsing: Specifies the number of alternative streams available on a given output port.*/
        case OMX_IndexParamNumAvailableStreams:
        {
            OMX_PARAM_U32TYPE *value = (OMX_PARAM_U32TYPE *)pComponentParameterStructure;
            MagOmxPort port;

            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (!allowedSetParameter(port, base->mState)){
                COMP_LOGE(root, "[OMX_IndexParamNumAvailableStreams]: disallowed in state %s or on enabled port!", 
                                 OmxState2String(base->mState));
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                port->setParameter(port, OMX_IndexParamNumAvailableStreams, value->nU32);
            }else{
                COMP_LOGE(root, "[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
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

            if (!allowedSetParameter(port, base->mState)){
                COMP_LOGE(root, "[OMX_IndexParamActiveStream]: disallowed in state %s or on enabled port!", 
                                OmxState2String(base->mState));
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                port->setParameter(port, OMX_IndexParamActiveStream, value->nU32);
            }else{
                COMP_LOGE(root, "[OMX_IndexParamActiveStream]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        /*no further implemention for now*/
        case OMX_IndexParamSuspensionPolicy:
        {
            OMX_PARAM_SUSPENSIONPOLICYTYPE *value;

            if (base->mState != OMX_StateLoaded){
                COMP_LOGE(root, "[OMX_IndexParamSuspensionPolicy]: disallowed in state %s!", 
                                 OmxState2String(base->mState));
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

            if (base->mState != OMX_StateLoaded){
                COMP_LOGE(root, "disallowed in state %s!", OmxState2String(base->mState));
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

            if (base->mState != OMX_StateLoaded){
                COMP_LOGE(root, "[OMX_IndexParamStandardComponentRole]: disallowed setting in state %s!", 
                                 OmxState2String(base->mState));
                return OMX_ErrorIncorrectStateOperation;
            }

            value = (OMX_PARAM_COMPONENTROLETYPE *)pComponentParameterStructure;
            base->mParametersDB->setString(base->mParametersDB, "StandardComponentRole", (char *)value->cRole);
        }
            break;

        /*should not be set from OMX IL*/
        case OMX_IndexConfigTunneledPortStatus:
        {
            COMP_LOGE(root, "OMX IL should not set OMX_IndexConfigTunneledPortStatus to the component!");
        }
            break;

        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *value;
            MagOmxPort port;
            OMX_ERRORTYPE ret;

            value = (OMX_PARAM_PORTDEFINITIONTYPE *)pComponentParameterStructure;
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);
            if (!allowedSetParameter(port, base->mState)){
                COMP_LOGE(root, "[OMX_IndexParamPortDefinition]: disallowed in state %s or on enabled port!", 
                                 OmxState2String(base->mState));
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                ret = port->setPortDefinition(port, value);
                if ( OMX_ErrorNone != ret ){
                    return ret;
                }
            }else{
                COMP_LOGE(root, "[OMX_IndexParamPortDefinition]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        case OMX_IndexParamCompBufferSupplier:
        {
            OMX_PARAM_BUFFERSUPPLIERTYPE *value = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pComponentParameterStructure;
            MagOmxPort port;

            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (!allowedSetParameter(port, base->mState)){
                COMP_LOGE(root, "[OMX_IndexParamCompBufferSupplier]: disallowed in state %s or on enabled port!", 
                                  OmxState2String(base->mState));
                return OMX_ErrorIncorrectStateOperation;
            }

            if (port){
                port->setParameter(port, OMX_IndexParamCompBufferSupplier, value->eBufferSupplier);
            }else{
                COMP_LOGE(root, "[OMX_IndexParamCompBufferSupplier]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        case OMX_IndexConfigTimeUpdate:
        {
            OMX_TIME_MEDIATIMETYPE *mt = (OMX_TIME_MEDIATIMETYPE *)pComponentParameterStructure;
            OMX_ERRORTYPE ret = OMX_ErrorNone;

            if (mt->eUpdateType == OMX_TIME_UpdateClockStateChanged){
                if (mt->eState == OMX_TIME_ClockStateRunning){
                    COMP_LOGD(root, "Get clock state to running notification!");
                    Mag_SetEvent(base->mClkStartRunningEvt);
                }

                if (MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus){
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus(base, 
                                                                                   OMX_TIME_UpdateClockStateChanged, 
                                                                                   (OMX_S32)(mt->eState));
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(root, "Failed to do MagOMX_SetAVSyncStatus(OMX_TIME_UpdateClockStateChanged), ret = 0x%x", ret);
                    }
                }else{
                    /*ret = OMX_ErrorUndefined;*/
                    COMP_LOGD(root, "No MagOMX_SetAVSyncStatus() need to be executed!");
                }
            }else if (mt->eUpdateType == OMX_TIME_UpdateRequestFulfillment){
                if (MagOmxComponentImplVirtual(base)->MagOMX_DoAVSync){
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_DoAVSync(base, mt);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(root, "Failed to do MagOMX_DoAVSync(), ret = 0x%x", ret);
                    }
                }else{
                    ret = OMX_ErrorUndefined;
                    COMP_LOGE(root, "pure virtual func: MagOMX_DoAVSync() is not overrided!!");
                }
            }else if (mt->eUpdateType == OMX_TIME_UpdateScaleChanged){
                if (MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus){
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus(base, 
                                                                                   OMX_TIME_UpdateScaleChanged, 
                                                                                   mt->xScale);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(root, "Failed to do MagOMX_SetAVSyncStatus(OMX_TIME_UpdateScaleChanged), ret = 0x%x", ret);
                    }
                }else{
                    ret = OMX_ErrorUndefined;
                    COMP_LOGE(root, "pure virtual func: MagOMX_SetAVSyncStatus() is not overrided!!");
                }
            }else{
                ret = OMX_ErrorBadParameter;
                COMP_LOGE(root, "OMX_IndexConfigTimeUpdate: UpdateType %d is not correct!", mt->eUpdateType);
            }
            return ret;
        }
            break;

        default:
            if (isPortParam(nIndex)){
                /*the parameters or configures for port*/
                OMX_U32 portID;
                MagOmxPort port;

                portID = getPortIndex(pComponentParameterStructure);
                port = ooc_cast(base->getPort(base, portID), MagOmxPort);

                if (port){
                    return MagOmxPortVirtual(port)->SetParameter(port, nIndex, pComponentParameterStructure);
                }else{
                    COMP_LOGE(root, "To set port parameter[0x%x] - Failure. (port is invalid)", nIndex);
                    return OMX_ErrorBadPortIndex;
                }
            }else{
                /*pass the parameter/config to the sub-component for processing no matter what state the component is in. 
                 The sub-component decides how to handle them*/
                if (MagOmxComponentImplVirtual(base)->MagOMX_SetParameter)   
                    return MagOmxComponentImplVirtual(base)->MagOMX_SetParameter(hComponent, nIndex, pComponentParameterStructure);
                else{
                    COMP_LOGE(root, "pure virtual func: MagOMX_SetParameter() is not overrided!!");
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
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    MagOMX_Param_Header_t *header;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    root = getRoot(hComponent);
    base = getBase(hComponent);
    
    COMP_LOGD(root, "enter");
    switch (nIndex){
        case OMX_IndexParamPriorityMgmt:
        case OMX_IndexConfigPriorityMgmt:
        {
            MagOMX_Param_PRIORITYMGMTTYPE_t *param;
            OMX_PRIORITYMGMTTYPE *value = (OMX_PRIORITYMGMTTYPE *)pComponentParameterStructure;
            
            if (base->mParametersDB->findPointer(base->mParametersDB, "PriorityMgmt", (void **)&param)){
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
                COMP_LOGE(root, "pure virtual func: MagOMX_getType() is not overrided!!");
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
                COMP_LOGE(root, "invalid parameter:%s setting to component type: %d", OmxParameter2String(nIndex), type);
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
                COMP_LOGE(root, "[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
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
                COMP_LOGE(root, "[OMX_IndexParamNumAvailableStreams]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        case OMX_IndexParamSuspensionPolicy:
        {
            OMX_ERRORTYPE ret;
            
            OMX_PARAM_SUSPENSIONPOLICYTYPE *value = (OMX_PARAM_SUSPENSIONPOLICYTYPE *)pComponentParameterStructure;
            initHeader(value, sizeof(OMX_PARAM_SUSPENSIONPOLICYTYPE));
            ret = base->mParametersDB->findUInt32(base->mParametersDB, "SuspensionPolicy", &value->ePolicy);

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
            ret = base->mParametersDB->findUInt32(base->mParametersDB, "ComponentSuspended", &value->eType);

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
            ret = base->mParametersDB->findString(base->mParametersDB, "StandardComponentRole", (char **)&value->cRole);

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
                COMP_LOGE(root, "[OMX_IndexConfigTunneledPortStatus]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
        }
            break;

        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *value;
            MagOmxPort port;
            OMX_ERRORTYPE ret;

            if (!MagOmxComponentImplVirtual(base)->MagOMX_getType){
                COMP_LOGE(root, "pure virtual func: MagOMX_getType() is not overrided!!");
                return OMX_ErrorNotImplemented;
            }

            value = (OMX_PARAM_PORTDEFINITIONTYPE *)pComponentParameterStructure;
            port = ooc_cast(base->getPort(base, value->nPortIndex), MagOmxPort);

            if (port){
                ret = port->getPortDefinition(port, value);
                if ( OMX_ErrorNone != ret )
                    return ret;
            }else{
                COMP_LOGE(root, "[OMX_IndexParamPortDefinition]: failed to find port: %d", value->nPortIndex);
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
                COMP_LOGE(root, "[OMX_IndexParamCompBufferSupplier]: failed to find port: %d", value->nPortIndex);
                return OMX_ErrorBadPortIndex;
            }
            
        }
            break;
        
        case OMX_IndexConfigExtAddPort:
        {
            OMX_CONFIG_UI32TYPE *value = (OMX_CONFIG_UI32TYPE *)pComponentParameterStructure;

            if (MagOmxComponentImplVirtual(base)->MagOMX_AddPortOnRequest){
                return MagOmxComponentImplVirtual(base)->MagOMX_AddPortOnRequest(base, &value->uValue);
            }else{
                COMP_LOGV(root, "pure virtual func: MagOMX_AddPortOnRequest() is not overrided!!");
                return OMX_ErrorUndefined;
            }
        }   
            break;

        default:
            if (isPortParam(nIndex)){
                /*the parameters or configures for port*/
                OMX_U32 portID;
                MagOmxPort port;

                portID = getPortIndex(pComponentParameterStructure);
                if (portID == OMX_ALL){
                    goto component_handle;
                }else{
                    port = ooc_cast(base->getPort(base, portID), MagOmxPort);

                    if (port){
                        return MagOmxPortVirtual(port)->GetParameter(port, nIndex, pComponentParameterStructure);
                    }else{
                        COMP_LOGE(root, "To get port parameter[0x%x] - Failure. (port is invalid)", nIndex);
                        return OMX_ErrorBadPortIndex;
                    }
                }
            }

component_handle:
            /*pass it to sub-component for processing*/
            if (MagOmxComponentImplVirtual(base)->MagOMX_GetParameter)
                return MagOmxComponentImplVirtual(base)->MagOMX_GetParameter(hComponent, nIndex, pComponentParameterStructure);
            else{
                COMP_LOGE(root, "pure virtual func: MagOMX_GetParameter() is not overrided!!");
                return OMX_ErrorNotImplemented;
            } 
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
        COMP_LOGE(getRoot(hComponent), "pure virtual func: MagOMX_GetComponentUUID() is not overrided!!");
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_SendCommand(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData){

    MagOmxComponentImpl base;
    MagOmxComponent     root;

    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    base = getBase(hComponent);
    root = getRoot(hComponent);
    
    if (Cmd == OMX_CommandStateSet){
        OMX_STATETYPE target_state = (OMX_STATETYPE)nParam1;

        if (base->mState == OMX_StateMax){
            if (target_state == OMX_StateLoaded){
                base->mState = OMX_StateLoaded;
                return OMX_ErrorNone;
            }else{
                COMP_LOGE(getRoot(hComponent), "Invalid state transition: current state:OMX_StateMax --> target state:%s", OmxState2String(target_state));
                return OMX_ErrorIncorrectStateTransition;
            }
        }else{
            if ((target_state < OMX_StateLoaded) || (target_state > OMX_StateWaitForResources)){
                COMP_LOGE(root, "Invalid state transition: current state:%s --> target state:0x%x", OmxState2String(base->mState), target_state);
                return OMX_ErrorIncorrectStateTransition;
            }

            switch (target_state){
                case OMX_StateLoaded:
                    base->mTransitionState = OMX_TransitionStateToLoaded;
                    break;

                case OMX_StateIdle:
                    base->mTransitionState = OMX_TransitionStateToIdle;
                    break;
                
                case OMX_StateExecuting:
                    base->mTransitionState = OMX_TransitionStateToExecuting;
                    break;

                case OMX_StatePause:
                    base->mTransitionState = OMX_TransitionStateToPause;
                    break;

                default:
                    base->mTransitionState = OMX_TransitionStateNone;
                    break;
            }

            /*do set state preroll action if it has*/
            if (base->mPrerollStateTransitTable[toIndex(base->mState)][toIndex(target_state)]){
                base->mPrerollStateTransitTable[toIndex(base->mState)][toIndex(target_state)](hComponent);
            }

            if ( !base->mCmdSetStateMsg ){
                base->mCmdSetStateMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandStateSetMsg);  
            }
            base->mCmdSetStateMsg->setUInt32(base->mCmdSetStateMsg, "param", target_state);
            base->mCmdSetStateMsg->setPointer(base->mCmdSetStateMsg, "cmd_data", pCmdData, MAG_FALSE);

            base->mCmdSetStateMsg->postMessage(base->mCmdSetStateMsg, 0);
        }
    }else if (Cmd == OMX_CommandFlush){
        if ( !base->mCmdFlushMsg ){
            base->mCmdFlushMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandFlushMsg);  
        }
        base->mCmdFlushMsg->setUInt32(base->mCmdFlushMsg, "param", nParam1);
        base->mCmdFlushMsg->setPointer(base->mCmdFlushMsg, "cmd_data", pCmdData, MAG_FALSE);
        
        base->mCmdFlushMsg->postMessage(base->mCmdFlushMsg, 0);
    }else if (Cmd == OMX_CommandPortDisable){
        if ( !base->mCmdPortDisableMsg ){
            base->mCmdPortDisableMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandPortDisableMsg);  
        }
        base->mCmdPortDisableMsg->setUInt32(base->mCmdPortDisableMsg, "param", nParam1);
        base->mCmdPortDisableMsg->setPointer(base->mCmdPortDisableMsg, "cmd_data", pCmdData, MAG_FALSE);

        base->mCmdPortDisableMsg->postMessage(base->mCmdPortDisableMsg, 0);
    }else if (Cmd == OMX_CommandPortEnable){
        if ( !base->mCmdPortEnableMsg ){
            base->mCmdPortEnableMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandPortEnableMsg);  
        }
        base->mCmdPortEnableMsg->setUInt32(base->mCmdPortEnableMsg, "param", nParam1);
        base->mCmdPortEnableMsg->setPointer(base->mCmdPortEnableMsg, "cmd_data", pCmdData, MAG_FALSE);

        base->mCmdPortEnableMsg->postMessage(base->mCmdPortEnableMsg, 0);
    }else if (Cmd == OMX_CommandMarkBuffer){
        if ( !base->mCmdMarkBufferMsg ){
            base->mCmdMarkBufferMsg = base->createMessage(hComponent, MagOmxComponentImpl_CommandMarkBufferMsg);  
        }
        base->mCmdMarkBufferMsg->setUInt32(base->mCmdMarkBufferMsg, "param", nParam1);
        base->mCmdMarkBufferMsg->setPointer(base->mCmdMarkBufferMsg, "cmd_data", pCmdData, MAG_FALSE);

        base->mCmdMarkBufferMsg->postMessage(base->mCmdMarkBufferMsg, 0);
    }else{
        COMP_LOGE(getRoot(hComponent), "invalid command: 0x%x", Cmd);
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
    
    /*Mag_AcquireMutex(comp->mhMutex);*/
    ret = MagOMX_GetParameter_internal(hComponent, nParamIndex, pComponentParameterStructure);
    /*Mag_ReleaseMutex(comp->mhMutex);*/

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
    
    /*Mag_AcquireMutex(comp->mhMutex);*/
    ret = MagOMX_SetParameter_internal(hComponent, nIndex, pComponentParameterStructure);
    /*Mag_ReleaseMutex(comp->mhMutex);*/

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
    
    /*Mag_AcquireMutex(comp->mhMutex);*/
    ret = MagOMX_GetParameter_internal(hComponent, nIndex, pComponentConfigStructure);
    /*Mag_ReleaseMutex(comp->mhMutex);*/

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
    
    /*Mag_AcquireMutex(comp->mhMutex);*/
    ret = MagOMX_SetParameter_internal(hComponent, nIndex, pComponentConfigStructure);
    /*Mag_ReleaseMutex(comp->mhMutex);*/

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

    if (NULL == hComponent){
        COMP_LOGE(NULL, "Wrong parameters: hComponent(%p)", hComponent);
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    port = ooc_cast(comp->getPort(comp, nPort), MagOmxPort);
    
    if (hTunneledComp){
        tunneledComp = getBase(hTunneledComp);
        tunneledPort = ooc_cast(tunneledComp->getPort(tunneledComp, nTunneledPort), MagOmxPort);

        COMP_LOGD(getRoot(hComponent), "port[0x%x] to tunneledPort[0x%x]", port, tunneledPort);
    }
    
    /*Mag_AcquireMutex(comp->mhMutex);*/
    if ((comp->mState == OMX_StateLoaded) ||
        (!port->getDef_Enabled(port) && !tunneledPort->getDef_Enabled(tunneledPort))){
        ret = MagOmxPortVirtual(port)->SetupTunnel(port, hTunneledComp, nTunneledPort, pTunnelSetup);
    }else{
        COMP_LOGE(getRoot(hComponent), "wrong state: %s!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    /*Mag_ReleaseMutex(comp->mhMutex);*/

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
    OMX_HANDLETYPE hPort;
    OMX_ERRORTYPE ret;

    if ((NULL == hComponent) || (NULL == *ppBufferHdr)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    hPort = comp->getPort(comp, nPortIndex);
    port = ooc_cast(hPort, MagOmxPort);

    /*Mag_AcquireMutex(comp->mhMutex);*/
    if ((comp->mState == OMX_StateLoaded) ||
        (comp->mState == OMX_StateWaitForResources) ||
        (!port->getDef_Enabled(port))){
        ret = MagOmxPortVirtual(port)->UseBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer);
    }else{
        COMP_LOGE(getRoot(hComponent), "wrong state: %s or on enabled port!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    /*Mag_ReleaseMutex(comp->mhMutex);*/

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
    OMX_HANDLETYPE hPort;
    OMX_ERRORTYPE ret;

    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    hPort = comp->getPort(comp, nPortIndex);
    port = ooc_cast(hPort, MagOmxPort);

    if (NULL == port){
        COMP_LOGE(getRoot(hComponent), "failed to get the port with index %d", nPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    /*Mag_AcquireMutex(comp->mhMutex);*/
    if ((comp->mState == OMX_StateLoaded) ||
        (comp->mState == OMX_StateWaitForResources) ||
        (!port->getDef_Enabled(port))){
        ret = MagOmxPortVirtual(port)->AllocateBuffer(hPort, ppBuffer, pAppPrivate, nSizeBytes);
    }else{
        COMP_LOGE(getRoot(hComponent), "wrong state: %s or on enabled port!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    /*Mag_ReleaseMutex(comp->mhMutex);*/

    return ret;
}

static OMX_ERRORTYPE virtual_FreeBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
                
    MagOmxComponentImpl comp; 
    MagOmxPort port;
    OMX_HANDLETYPE hPort;
    OMX_ERRORTYPE ret;

    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    hPort = comp->getPort(comp, nPortIndex);
    port = ooc_cast(hPort, MagOmxPort);

    /*Mag_AcquireMutex(comp->mhMutex);*/
    if ((comp->mState  == OMX_StateIdle) ||
        ((comp->mState != OMX_StateLoaded) && (comp->mState != OMX_StateWaitForResources) && (!port->getDef_Enabled(port)))){
        ret = MagOmxPortVirtual(port)->FreeBuffer(hPort, pBuffer);
    }else{
        COMP_LOGE(getRoot(hComponent), "wrong state: %s or on enabled port!", OmxState2String(comp->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    /*Mag_ReleaseMutex(comp->mhMutex);*/

    return ret;
}

static OMX_ERRORTYPE virtual_EmptyThisBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    MagOmxComponentImpl comp; 
    MagOmxPort port;
    OMX_HANDLETYPE hPort;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }

    comp = getBase(hComponent);
    hPort = comp->getPort(comp, pBuffer->nInputPortIndex);
    if (hPort == NULL){
        COMP_LOGE(getRoot(hComponent), "failed to get the port with index: %d", pBuffer->nInputPortIndex);
        return OMX_ErrorUnsupportedIndex;
    }

    port = ooc_cast(hPort, MagOmxPort);
    
    if ( (comp->mState == OMX_StateExecuting ||
         comp->mState == OMX_StatePause)     &&
         port->getDef_Enabled(port)          &&
         port->isInputPort(port) ){
        if (comp->mTransitionState == OMX_TransitionStateNone || port->isTunneled(port)){
            ret = MagOmxPortVirtual(port)->EmptyThisBuffer(hPort, pBuffer);
        }else{
            COMP_LOGD(getRoot(hComponent), "ignore the command(transition state: %s)", 
                                            OmxTransState2String(comp->mTransitionState));
        }
    }else{
        COMP_LOGE(getRoot(hComponent), "Error condition to do: state[%s], port enabled[%s], input port[%s]", 
                                        OmxState2String(comp->mState),
                                        port->getDef_Enabled(port) ? "Yes" : "No",
                                        port->isInputPort(port) ? "Yes" : "No");
        ret = OMX_ErrorIncorrectStateOperation;
    }

    return ret;
}

static OMX_ERRORTYPE virtual_FillThisBuffer(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    MagOmxComponentImpl comp; 
    MagOmxPort port;
    OMX_HANDLETYPE hPort;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if ((NULL == hComponent) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }
    
    comp = getBase(hComponent);
    hPort = comp->getPort(comp, pBuffer->nOutputPortIndex);
    if (hPort == NULL){
        COMP_LOGE(getRoot(hComponent), "failed to get the port with index: %d", pBuffer->nOutputPortIndex);
        return OMX_ErrorUnsupportedIndex;
    }
    port = ooc_cast(hPort, MagOmxPort);

    /*Mag_AcquireMutex(comp->mhMutex);*/
    if ( (comp->mState == OMX_StateExecuting ||
         comp->mState == OMX_StatePause)     &&
         port->getDef_Enabled(port)          &&
         !port->isInputPort(port) ){
        if (comp->mTransitionState == OMX_TransitionStateNone || port->isTunneled(port)){
            ret = MagOmxPortVirtual(port)->FillThisBuffer(hPort, pBuffer);
        }else{
            COMP_LOGD(getRoot(hComponent), "ignore the command(transition state: %s)", 
                                            OmxTransState2String(comp->mTransitionState));
        }
    }else{
        COMP_LOGE(getRoot(hComponent), "Error condition to do: state[%s], port enabled[%s], input port[%s]", 
                                        OmxState2String(comp->mState),
                                        port->getDef_Enabled(port) ? "Yes" : "No",
                                        port->isInputPort(port) ? "Yes" : "No");
        ret = OMX_ErrorIncorrectStateOperation;
    }
    /*Mag_ReleaseMutex(comp->mhMutex);*/

    return ret;
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
        COMP_LOGE(getRoot(hComponent), "in the wrong state: %s", OmxState2String(comp->mState));
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

    /*Mag_AcquireMutex(base->mhMutex);*/
    if (base->mState == OMX_StateLoaded){
        if (MagOmxComponentImplVirtual(base)->MagOMX_Deinit){
            ret = MagOmxComponentImplVirtual(base)->MagOMX_Deinit(hComponent);
        }else{
            COMP_LOGE(getRoot(hComponent), "pure virtual func: MagOMX_Deinit() is not overrided!!");
            ret = OMX_ErrorNotImplemented;
        }
    }else{
        COMP_LOGE(getRoot(hComponent), "Error condition to do: state[%s]", OmxState2String(base->mState));
        ret = OMX_ErrorIncorrectStateOperation;
    }
    /*Mag_ReleaseMutex(base->mhMutex);*/

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
        COMP_LOGE(getRoot(hComponent), "pure virtual func: MagOMX_ComponentRoleEnum() is not overrided!!");
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
    
    AGILE_LOGV("enter!");
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
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_EVENTTYPE eEvent,
                OMX_IN OMX_U32 nData1,
                OMX_IN OMX_U32 nData2,
                OMX_IN OMX_PTR pEventData){
    MagOmxComponentImpl compImpl;
    MagOmxComponent     comp;

    comp     = getRoot(hComponent);
    compImpl = getBase(hComponent);
    if (compImpl->mpCallbacks){
        if (compImpl->mpCallbacks->EventHandler){
            return compImpl->mpCallbacks->EventHandler(comp->getComponentObj(comp),
                                                       compImpl->mpAppData, 
                                                       eEvent, 
                                                       nData1, 
                                                       nData2, 
                                                       pEventData);
        }else{
            COMP_LOGE(getRoot(hComponent), "the EventHandler Callback is NULL");
            return OMX_ErrorNotImplemented;
        }
    }else{
        COMP_LOGE(getRoot(hComponent), "the Callbacks are NULL");
        return OMX_ErrorNotImplemented;
    }
}

static OMX_ERRORTYPE MagOmxComponentImpl_sendEmptyBufferDoneEvent(
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxComponentImpl compImpl;
    MagOmxComponent     comp;

    comp     = getRoot(hComponent);
    compImpl = getBase(hComponent);

    if (compImpl->mpCallbacks){
        if (compImpl->mpCallbacks->EmptyBufferDone){
            return compImpl->mpCallbacks->EmptyBufferDone(comp->getComponentObj(comp),
                                                          compImpl->mpAppData, 
                                                          pBuffer);
        }else{
            COMP_LOGE(comp, "the EmptyBufferDone Callback is NULL");
            return OMX_ErrorNotImplemented;
        }
    }else{
        COMP_LOGE(comp, "the Callbacks are NULL");
        return OMX_ErrorNotImplemented;
    }
}

static OMX_ERRORTYPE MagOmxComponentImpl_sendFillBufferDoneEvent(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxComponentImpl compImpl;
    MagOmxComponent     comp;

    comp     = getRoot(hComponent);
    compImpl = getBase(hComponent);

    if (compImpl->mpCallbacks){
        if (compImpl->mpCallbacks->FillBufferDone){
            return compImpl->mpCallbacks->FillBufferDone(comp->getComponentObj(comp),
                                                         compImpl->mpAppData, 
                                                         pBuffer);
        }else{
            COMP_LOGE(comp, "the FillBufferDone Callback is NULL");
            return OMX_ErrorNotImplemented;
        }
    }else{
        COMP_LOGE(comp, "the Callbacks are NULL");
        return OMX_ErrorNotImplemented;
    }
}

static MagMessageHandle MagOmxComponentImpl_createMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentImpl hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentImpl);
    hComponent->getLooper(handle);
    
    msg = createMagMessage(hComponent->mLooper, what, hComponent->mMsgHandler->id(hComponent->mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static MagMessageHandle MagOmxComponentImpl_createBufferMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentImpl hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentImpl);  
    hComponent->getBufferLooper(handle);
    
    msg = createMagMessage(hComponent->mBufferLooper, what, hComponent->mBufferMsgHandler->id(hComponent->mBufferMsgHandler));
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

static OMX_ERRORTYPE MagOmxComponentImpl_setState(OMX_HANDLETYPE hComponent, OMX_STATETYPE target_state){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    MagOmxComponentImpl base;
    MagOmxComponent     root;

    if (!hComponent){
        COMP_LOGE(NULL, "hComponent is NULL");
        return OMX_ErrorBadParameter;
    }

    root = getRoot(hComponent);
    base = getBase(hComponent);

    COMP_LOGD(root, "Try (current state:%s --> target state:%s)", OmxState2String(base->mState), OmxState2String(target_state));

    if (base->mStateTransitTable[toIndex(base->mState)][toIndex(target_state)]){
        ret = base->mStateTransitTable[toIndex(base->mState)][toIndex(target_state)](hComponent);
        if (ret == OMX_ErrorNone){
            COMP_LOGD(root, "Transit current state:%s --> target state:%s -- OK!", OmxState2String(base->mState), OmxState2String(target_state));
            base->mState = target_state;
            base->mTransitionState = OMX_TransitionStateNone;
        }else{
            COMP_LOGE(root, "Transit current state:%s --> target state:%s -- Failure!", OmxState2String(base->mState), OmxState2String(target_state));
            if ((base->mState == OMX_StateLoaded) && (target_state == OMX_StateIdle)){
                base->mState = OMX_StateWaitForResources;
                base->mTransitionState = OMX_TransitionStateNone;
            }
        }
    }else{
        COMP_LOGE(root, "Invalid state transition: current state:%s --> target state:%s", OmxState2String(base->mState), OmxState2String(target_state));
        return OMX_ErrorIncorrectStateTransition;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentImpl_flushPort(OMX_HANDLETYPE hComponent, OMX_U32 port_index){
    RBTreeNodeHandle n;
    MagOmxPort port;
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (NULL != hComponent){
        comp = getBase(hComponent);
        comp->mFlushingPorts = port_index;

        if (port_index == OMX_ALL){
            for (n = rbtree_first(comp->mPortTreeRoot); n; n = rbtree_next(n)) {
                port = ooc_cast(n->value, MagOmxPort);
                if (port->getDef_Enabled(port)){
                    ret = MagOmxPortVirtual(port)->Flush(n->value);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(getRoot(hComponent), "failed to flush the port[OMX_ALL]: %d", port->getPortIndex(port));
                        break;
                    }
                }else{
                    COMP_LOGD(getRoot(hComponent), "the port %d is disabled, ignore the flush action [OMX_ALL]!", port->getPortIndex(port));
                }
            }
        }else{
            OMX_HANDLETYPE portHandle;

            portHandle = comp->getPort(comp, port_index);
            port = ooc_cast(portHandle, MagOmxPort);

            if (port->getDef_Enabled(port)){
                ret = MagOmxPortVirtual(port)->Flush(portHandle);
                if (ret != OMX_ErrorNone){
                    COMP_LOGE(getRoot(hComponent), "failed to flush the port: %d", port->getPortIndex(port));
                }
            }else{
                COMP_LOGE(getRoot(hComponent), "the port %d is disabled, ignore the flush action!", port->getPortIndex(port));
            }
        }
    }else{
        ret = OMX_ErrorBadParameter;
    }

    comp->mFlushingPorts = kInvalidPortIndex;
    comp->sendEvents(hComponent, OMX_EventCmdComplete, OMX_CommandFlush, port_index, &ret);
    return ret;;
}

static OMX_ERRORTYPE MagOmxComponentImpl_enablePort(OMX_HANDLETYPE hComponent, OMX_U32 port_index){  
    RBTreeNodeHandle n;          
    MagOmxPort port;
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (NULL != hComponent){
        comp = getBase(hComponent);
        
        if (port_index == OMX_ALL){
            for (n = rbtree_first(comp->mPortTreeRoot); n; n = rbtree_next(n)) {
                port = ooc_cast(n->value, MagOmxPort);
                if ((comp->mState == OMX_StateLoaded) ||
                    (comp->mState == OMX_StateWaitForResources)){
                    port->setDef_Enabled(port, OMX_TRUE);
                }else if (comp->mState == OMX_StateIdle){
                    ret = MagOmxPortVirtual(port)->Enable(n->value);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(getRoot(hComponent), "failed to enable the port[OMX_ALL]: %d in state Idle", port->getPortIndex(port));
                        break;
                    }
                }else if (comp->mState == OMX_StateExecuting){
                    ret = MagOmxPortVirtual(port)->Enable(n->value);
                    if (ret == OMX_ErrorNone){
                        ret = MagOmxPortVirtual(port)->Run(n->value);
                        if (ret != OMX_ErrorNone){
                            COMP_LOGE(getRoot(hComponent), "failed to run the port[OMX_ALL] %d", port->getPortIndex(port));
                            break;
                        }
                    }else{
                        COMP_LOGE(getRoot(hComponent), "failed to enable the port[OMX_ALL]: %d in state Executing", port->getPortIndex(port));
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
                ret = MagOmxPortVirtual(port)->Enable(portHandle);
                if (ret != OMX_ErrorNone){
                    COMP_LOGE(getRoot(hComponent), "failed to enable the port: %d in state Idle/Executing", port->getPortIndex(port));
                }else{
                    if (comp->mState == OMX_StateExecuting){
                        ret = MagOmxPortVirtual(port)->Run(portHandle);
                        if (ret != OMX_ErrorNone){
                            COMP_LOGE(getRoot(hComponent), "failed to run the port %d", port->getPortIndex(port));
                        }
                    }
                }
            }
        }
    }else{
        ret = OMX_ErrorBadParameter;
    }

    comp->sendEvents(hComponent, OMX_EventCmdComplete, OMX_CommandPortEnable, port_index, &ret);
    return ret;
}


static OMX_ERRORTYPE MagOmxComponentImpl_disablePort(OMX_HANDLETYPE hComponent, OMX_U32 port_index){
    RBTreeNodeHandle n;          
    MagOmxPort port;
    MagOmxComponentImpl comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (NULL != hComponent){
        comp = getBase(hComponent);
        
        if (port_index == OMX_ALL){
            for (n = rbtree_first(comp->mPortTreeRoot); n; n = rbtree_next(n)) {
                port = ooc_cast(n->value, MagOmxPort);
                if ((comp->mState == OMX_StateLoaded) ||
                    (comp->mState == OMX_StateWaitForResources)){
                    port->setDef_Enabled(port, OMX_FALSE);
                }else if (comp->mState == OMX_StateIdle){
                    ret = MagOmxPortVirtual(port)->Disable(n->value);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(getRoot(hComponent), "failed to disable the port[OMX_ALL]: %d in state Idle", port->getPortIndex(port));
                        break;
                    }
                }else if ((comp->mState == OMX_StateExecuting) ||
                          (comp->mState == OMX_StatePause)){
                    ret = MagOmxPortVirtual(port)->Flush(n->value);
                    if (ret == OMX_ErrorNone){
                        ret = MagOmxPortVirtual(port)->Disable(n->value);
                        if (ret != OMX_ErrorNone){
                            COMP_LOGE(getRoot(hComponent), "failed to disable the port[OMX_ALL]: %d in state Executing", port->getPortIndex(port));
                            break;
                        }
                    }else{
                        COMP_LOGE(getRoot(hComponent), "failed to flush the port[OMX_ALL]: %d in state Executing", port->getPortIndex(port));
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
                    COMP_LOGE(getRoot(hComponent), "failed to disable the port: %d in state Idle", port->getPortIndex(port));
                }
            }else if ((comp->mState == OMX_StateExecuting) ||
                      (comp->mState == OMX_StatePause)){
                ret = MagOmxPortVirtual(port)->Flush(portHandle);
                if (ret == OMX_ErrorNone){
                    ret = MagOmxPortVirtual(port)->Disable(portHandle);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(getRoot(hComponent), "failed to disable the port: %d in state Executing", port->getPortIndex(port));
                    }
                }else{
                    COMP_LOGE(getRoot(hComponent), "failed to flush the port: %d in state Executing", port->getPortIndex(port));
                }
            }
        }
    }else{
        ret = OMX_ErrorBadParameter;
    }

    comp->sendEvents(hComponent, OMX_EventCmdComplete, OMX_CommandPortDisable, port_index, &ret);
    return ret;
}

static void MagOmxComponentImpl_addPort(MagOmxComponentImpl hComponent, 
                                        OMX_U32 portIndex, 
                                        OMX_HANDLETYPE hPort){
    MagOmxPort portRoot;

    if ((NULL == hPort) || (NULL == hComponent)){
        return;
    }

    portRoot = ooc_cast(hPort, MagOmxPort);

    hComponent->mPortTreeRoot =  rbtree_insert(hComponent->mPortTreeRoot, portIndex, hPort);
    portRoot->setAttachedComponent(portRoot, hComponent);

    if (MagOmxPortVirtual(portRoot)->GetDomainType(portRoot) == OMX_PortDomainOther_Clock){ 
        hComponent->mhClockPort = hPort;
    }

    if (MagOmxComponentImplVirtual(hComponent)->MagOMX_DoAddPortAction){
        MagOmxComponentImplVirtual(hComponent)->MagOMX_DoAddPortAction(hComponent, portIndex, hPort);
    }
}

static OMX_HANDLETYPE MagOmxComponentImpl_getPort(MagOmxComponentImpl hComponent, 
                                                  OMX_U32 portIndex){

    OMX_HANDLETYPE hPort = NULL;
    
    if (NULL == hComponent){
        return NULL;
    }

    hPort = rbtree_get(hComponent->mPortTreeRoot, portIndex);

    if (!hPort){
        COMP_LOGE(getRoot(hComponent), "failed to get the port(%d) of the component:0x%p", portIndex, hComponent);
        return NULL;
    }

    return (hPort);
}

/*setup the port connection inside the component*/
static OMX_ERRORTYPE MagOmxComponentImpl_setupPortDataFlow(
                                        OMX_IN MagOmxComponentImpl hComponent,
                                        OMX_IN OMX_HANDLETYPE hSrcPort,
                                        OMX_IN OMX_HANDLETYPE hDestPort){
    OMX_ERRORTYPE err;
    MagOmxPort srcPort = NULL;
    MagOmxPort destPort = NULL;

    if (hSrcPort == NULL){
        COMP_LOGE(getRoot(hComponent), "invalid parameter: srcPort is NULL");
        return OMX_ErrorBadParameter;
    }

    srcPort = ooc_cast(hSrcPort, MagOmxPort);
    if (hDestPort)
        destPort = ooc_cast(hDestPort, MagOmxPort);

    if (srcPort->getBufferPolicy(srcPort) == kSharedBuffer){
        if (destPort && destPort->isBufferSupplier(destPort)){
            MagMessageHandle msg;
            msg = MagOmxPortVirtual(destPort)->GetSharedBufferMsg(hSrcPort);
            if (msg){
                err = MagOmxPortVirtual(srcPort)->RegisterBufferHandler(hSrcPort, msg);
                if (err != OMX_ErrorNone){
                    COMP_LOGE(getRoot(hComponent), "failed to setup the shared buffer!");
                    return err;
                }
            }
        }else{
            COMP_LOGE(getRoot(hComponent), "Failed! %s for shared buffer setup", 
                                            destPort == NULL ? "The destPort is NULL" : "The destPort is none-supplier");
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
            msg->setPointer(msg, "source_port", hSrcPort, MAG_FALSE);
            msg->setPointer(msg, "destination_port", hDestPort, MAG_FALSE);
            err = MagOmxPortVirtual(srcPort)->RegisterBufferHandler(hSrcPort, msg);
            if (err != OMX_ErrorNone){
                COMP_LOGE(getRoot(hComponent), "failed to register the message to port!");
                return err;
            }
        }else{
            COMP_LOGE(getRoot(hComponent), "the created message is NULL");
            return OMX_ErrorInsufficientResources;
        }
    }

    return OMX_ErrorNone;
}

/*the port notify the attached component to config it*/
static OMX_ERRORTYPE MagOmxComponentImpl_notify(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN MagOMX_Component_Notify_Type_t notifyIndex,
                        OMX_IN OMX_PTR pNotifyData){
    OMX_ERRORTYPE ret;

    if (MagOmxComponentImplVirtual(hComponent)->MagOMX_Port_Notify){
        ret = MagOmxComponentImplVirtual(hComponent)->MagOMX_Port_Notify(hComponent, notifyIndex, pNotifyData);
    }else{
        COMP_LOGE(getRoot(hComponent), "the pure virtual function MagOMX_Port_Notify is not overrided!");
        ret = OMX_ErrorNotImplemented;
    }
    return ret;
}

/*The port queries the attached component for the information*/
static OMX_ERRORTYPE MagOmxComponentImpl_query(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN MagOMX_Component_Query_Type_t queryIndex,
                        OMX_IN OMX_PTR pQueryData){
    OMX_ERRORTYPE ret;

    if (MagOmxComponentImplVirtual(hComponent)->MagOMX_Port_Query){
        ret = MagOmxComponentImplVirtual(hComponent)->MagOMX_Port_Query(hComponent, queryIndex, pQueryData);
    }else{
        COMP_LOGE(getRoot(hComponent), "the pure virtual function MagOMX_Port_Query is not overrided!");
        ret = OMX_ErrorNotImplemented;
    }
    return ret;
}

/*called from MagOMX_ProceedBuffer() to send buffer to clock component for av sync*/
static OMX_ERRORTYPE MagOmxComponentImpl_syncDisplay(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE *bufHeader){
    List_t *freeList;
    List_t *busyList;
    MagOMX_Component_Buffer_List_t *node;
    List_t *tmp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (hComponent->mhClockPort){
        OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE mtrt;
        OMX_TIME_CONFIG_TIMESTAMPTYPE        startTime;
        OMX_TICKS offset = 0;
        MagOmxPort portRoot = NULL;

        portRoot = ooc_cast(hComponent->mhClockPort, MagOmxPort);

        if (!hComponent->mbGetStartTime){
            initHeader(&startTime, sizeof(OMX_TIME_CONFIG_TIMESTAMPTYPE));
            startTime.nPortIndex = portRoot->getPortIndex(portRoot);
            startTime.nTimestamp = bufHeader->nTimeStamp;
            MagOmxPortVirtual(portRoot)->SetParameter(portRoot, 
                                                      OMX_IndexConfigTimeClientStartTime, 
                                                      &startTime);
            hComponent->mbGetStartTime = OMX_TRUE;

            /*wait on the clock component state transition to Running*/
            COMP_LOGD(getRoot(hComponent), "wait on the clock component state to Running");
            Mag_ClearEvent(hComponent->mClkStartRunningEvt);
            Mag_WaitForEventGroup(hComponent->mClkStChangeEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            COMP_LOGD(getRoot(hComponent), "clock component state is RuProcnning now!");
        }

        freeList = &hComponent->mAVSyncFreeBufLH;
        busyList = &hComponent->mAVSyncBusyBufLH;

        Mag_AcquireMutex(hComponent->mhAVSyncMutex);
        if (freeList->next == freeList){
            /*free list is empty*/
            node = (MagOMX_Component_Buffer_List_t *)mag_mallocz(sizeof(MagOMX_Component_Buffer_List_t));
            INIT_LIST(&node->node);
        }else{
            node = (MagOMX_Component_Buffer_List_t *)list_entry(freeList->next, MagOMX_Component_Buffer_List_t, node);
            list_del(&node->node);
        }
        node->buffer = bufHeader;
        list_add_tail(&node->node, busyList);
#if 0
        COMP_LOGD(getRoot(hComponent), "add pts 0x%llx into the busylist!", node->buffer->nTimeStamp);
        {
            List_t *tmp;
            tmp = busyList->next;
            while (tmp != busyList){
                node = (MagOMX_Component_Buffer_List_t *)list_entry(tmp, MagOMX_Component_Buffer_List_t, node);
                COMP_LOGD(getRoot(hComponent), "Traverse busylist node:[0x%llx]",
                                                node->buffer->nTimeStamp);
                tmp = tmp->next;
            }
        }
#endif
        Mag_ReleaseMutex(hComponent->mhAVSyncMutex);

        initHeader(&mtrt, sizeof(OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE));
        mtrt.pClientPrivate  = hComponent;
        mtrt.nMediaTimestamp = bufHeader->nTimeStamp;
        if (MagOmxComponentImplVirtual(hComponent)->MagOMX_GetClockActionOffset){
            ret = MagOmxComponentImplVirtual(hComponent)->MagOMX_GetClockActionOffset(hComponent, &offset);
            COMP_LOGD(getRoot(hComponent), "MagOMX_GetClockActionOffset(%d us)", offset);
        }else{
            COMP_LOGE(getRoot(hComponent), "the pure virtual function MagOMX_GetClockActionOffset is not overrided!");
        }
        mtrt.nOffset = offset;
        
        ret = MagOmxPortVirtual(portRoot)->SetParameter(portRoot, 
                                                        OMX_IndexConfigTimeMediaTimeRequest, 
                                                        &mtrt);
    }else{
        COMP_LOGD(getRoot(hComponent), "No clock port is attached!");
        ret = OMX_ErrorPortsNotConnected;
    }
    return ret;
}

/*called from MagOMX_DoAVSync() to release the buffer for displaying*/
static OMX_ERRORTYPE MagOmxComponentImpl_releaseDisplay(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN OMX_TICKS mediaTimestamp,
                        OMX_IN OMX_BUFFERHEADERTYPE **ppBufHeader){

    List_t *freeList;
    List_t *busyList;
    List_t *expected;
    List_t *tmp;
    MagOMX_Component_Buffer_List_t *node;
    OMX_U32 i = 1;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    freeList = &hComponent->mAVSyncFreeBufLH;
    busyList = &hComponent->mAVSyncBusyBufLH;

    *ppBufHeader = NULL;
    Mag_AcquireMutex(hComponent->mhAVSyncMutex);

    expected = busyList->next;
    if (expected != busyList){
        node = (MagOMX_Component_Buffer_List_t *)list_entry(expected, MagOMX_Component_Buffer_List_t, node);
        if (node->buffer->nTimeStamp == mediaTimestamp){
            list_del(&node->node);
            *ppBufHeader = node->buffer;
            list_add_tail(&node->node, freeList);
        }else{
            COMP_LOGE(getRoot(hComponent), "the expected buffer timestamp[0x%llx] is not the first one in the busy list",
                                            mediaTimestamp);
            tmp = expected->next;
            while (tmp != busyList){
                i++;
                node = (MagOMX_Component_Buffer_List_t *)list_entry(tmp, MagOMX_Component_Buffer_List_t, node);
                COMP_LOGD(getRoot(hComponent), "Traverse busylist node:%d[0x%llx], expected pts: 0x%llx",
                                                i, node->buffer->nTimeStamp, mediaTimestamp);
                if (node->buffer->nTimeStamp == mediaTimestamp){
                    list_del(&node->node);
                    *ppBufHeader = node->buffer;
                    list_add_tail(&node->node, freeList);
                    COMP_LOGD(getRoot(hComponent), "find the buffer timestamp[0x%llx] at place %d", 
                                                   mediaTimestamp, i);
                    break;
                }
                tmp = tmp->next;
            }

            if (tmp == busyList){
                COMP_LOGE(getRoot(hComponent), "Failed to find out the timestamp[0x%llx] in the busy list!",
                                                mediaTimestamp);
                ret = OMX_ErrorNoMore;
            }
        }
    }else{
        COMP_LOGE(getRoot(hComponent), "Error happens, the busy list is empty!");
        ret = OMX_ErrorNoMore;
    }

    Mag_ReleaseMutex(hComponent->mhAVSyncMutex);
    return ret;
}

static OMX_ERRORTYPE MagOmxComponentImpl_putReturnBuffer(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN MagOmxPort hSrcPort,
                        OMX_IN OMX_BUFFERHEADERTYPE *srcBufHeader,
                        OMX_IN MagMessageHandle returnMsg,
                        OMX_IN OMX_PTR priv){
    List_t *freeList;
    List_t *returnList;
    MagOmx_Port_Node_t *node;
    MagOMX_Port_Buffer_t *portBufMgr;

    portBufMgr = (MagOMX_Port_Buffer_t *)srcBufHeader->pPlatformPrivate;

    freeList   = &portBufMgr->freeBufPortListH;
    returnList = &portBufMgr->returnBufPortListH;

    if (freeList->next == freeList){
        /*free list is empty*/
        node = (MagOmx_Port_Node_t *)mag_mallocz(sizeof(MagOmx_Port_Node_t));
        INIT_LIST(&node->node);
    }else{
        node = (MagOmx_Port_Node_t *)list_entry(freeList->next, MagOmx_Port_Node_t, node);
        list_del(&node->node);
    }

    node->hPort = hSrcPort;
    list_add_tail(&node->node, returnList);

    return MagOmxPortVirtual(hSrcPort)->putReturnBuffer(hSrcPort, srcBufHeader, returnMsg, priv);
}

static OMX_ERRORTYPE MagOmxComponentImpl_sendReturnBuffer(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE *pBuf){
    List_t *freeList;
    List_t *returnList;
    MagOmx_Port_Node_t *node;
    MagOMX_Port_Buffer_t *portBufMgr;
    MagOmxPort hSrcPort = NULL;

    portBufMgr = (MagOMX_Port_Buffer_t *)pBuf->pPlatformPrivate;

    freeList   = &portBufMgr->freeBufPortListH;
    returnList = &portBufMgr->returnBufPortListH;

    if (returnList->prev != returnList){
        node = (MagOmx_Port_Node_t *)list_entry(returnList->prev, MagOmx_Port_Node_t, node);
        list_del(&node->node);
        hSrcPort = ooc_cast(node->hPort, MagOmxPort);
        list_add_tail(&node->node, freeList);
        return MagOmxPortVirtual(hSrcPort)->sendReturnBuffer(hSrcPort, pBuf);
    }else{
        COMP_LOGE(getRoot(hComponent), "No place to return the buffer!");
    }

    return OMX_ErrorUndefined;
}

static OMX_ERRORTYPE MagOmxComponentImpl_putOutputBuffer(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN MagOmxPort hDestPort,
                        OMX_IN OMX_BUFFERHEADERTYPE *pBufHeader){
    List_t *freeList;
    List_t *outputList;
    MagOmx_Port_Node_t *node;
    MagOMX_Port_Buffer_t *portBufMgr;

    portBufMgr = (MagOMX_Port_Buffer_t *)pBufHeader->pPlatformPrivate;

    freeList   = &portBufMgr->freeBufPortListH;
    outputList = &portBufMgr->outputBufPortListH;

    if (freeList->next == freeList){
        /*free list is empty*/
        node = (MagOmx_Port_Node_t *)mag_mallocz(sizeof(MagOmx_Port_Node_t));
        INIT_LIST(&node->node);
    }else{
        node = (MagOmx_Port_Node_t *)list_entry(freeList->next, MagOmx_Port_Node_t, node);
        list_del(&node->node);
    }

    node->hPort = hDestPort;
    list_add_tail(&node->node, outputList);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxComponentImpl_sendOutputBuffer(
                        OMX_IN MagOmxComponentImpl hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE *pBuf){
    List_t *freeList;
    List_t *outputList;
    MagOmx_Port_Node_t *node;
    MagOMX_Port_Buffer_t *portBufMgr;
    MagOmxPort hDestPort = NULL;

    portBufMgr = (MagOMX_Port_Buffer_t *)pBuf->pPlatformPrivate;

    freeList   = &portBufMgr->freeBufPortListH;
    outputList = &portBufMgr->outputBufPortListH;

    if (outputList->prev != outputList){
        node = (MagOmx_Port_Node_t *)list_entry(outputList->prev, MagOmx_Port_Node_t, node);
        list_del(&node->node);
        hDestPort = ooc_cast(node->hPort, MagOmxPort);
        list_add_tail(&node->node, freeList);
        return MagOmxPortVirtual(hDestPort)->sendOutputBuffer(hDestPort, pBuf);
    }else{
        COMP_LOGE(getRoot(hComponent), "No place to output the buffer!");
    }

    return OMX_ErrorUndefined;
}

/*Class Constructor/Destructor*/
static void MagOmxComponentImpl_initialize(Class this){
    AGILE_LOGV("Enter!");

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
    MagOmxComponentImplVtableInstance.MagOMX_getType                         = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_GetParameter                    = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_SetParameter                    = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_ComponentRoleEnum               = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_ProceedBuffer                   = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_DoAVSync                        = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_SetAVSyncStatus                 = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_SetRefClock                     = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_DoAddPortAction                 = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Port_Notify                     = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_Port_Query                      = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_GetRenderDelay                  = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_GetClockActionOffset            = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_AddPortOnRequest                = NULL;
    MagOmxComponentImplVtableInstance.MagOMX_TearDownTunnel                  = NULL;
}

/*
 * params[0]: the index of the first port(mStartPortNumber)
 * params[1]: the number of ports(mPorts)
 */
static void MagOmxComponentImpl_constructor(MagOmxComponentImpl thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    if (params == NULL){
        AGILE_LOGE("The input params is NULL, quit!");
        return;
    }

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
    thiz->notify                   = MagOmxComponentImpl_notify;
    thiz->syncDisplay              = MagOmxComponentImpl_syncDisplay;
    thiz->releaseDisplay           = MagOmxComponentImpl_releaseDisplay;
    thiz->putReturnBuffer          = MagOmxComponentImpl_putReturnBuffer;
    thiz->sendReturnBuffer         = MagOmxComponentImpl_sendReturnBuffer;
    thiz->putOutputBuffer          = MagOmxComponentImpl_putOutputBuffer;
    thiz->sendOutputBuffer         = MagOmxComponentImpl_sendOutputBuffer;

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
    thiz->mState = OMX_StateMax;
    thiz->mTransitionState = OMX_TransitionStateNone;
    memset(thiz->mStateTransitTable, 0, sizeof(doStateTransition)*25);
    thiz->mStateTransitTable[toIndex(OMX_StateLoaded)][toIndex(OMX_StateIdle)]             = doStateLoaded_Idle;
    thiz->mStateTransitTable[toIndex(OMX_StateLoaded)][toIndex(OMX_StateWaitForResources)] = doStateLoaded_WaitforResources;
    thiz->mStateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StateLoaded)]             = doStateIdle_Loaded;
    thiz->mStateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StateExecuting)]          = doStateIdle_Executing;
    thiz->mStateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StatePause)]              = doStateIdle_Pause;
    thiz->mStateTransitTable[toIndex(OMX_StateExecuting)][toIndex(OMX_StateIdle)]          = doStateExecuting_Idle;
    thiz->mStateTransitTable[toIndex(OMX_StateExecuting)][toIndex(OMX_StatePause)]         = doStateExecuting_Pause;
    thiz->mStateTransitTable[toIndex(OMX_StatePause)][toIndex(OMX_StateIdle)]              = doStatePause_Idle;
    thiz->mStateTransitTable[toIndex(OMX_StatePause)][toIndex(OMX_StateExecuting)]         = doStatePause_Executing;
    thiz->mStateTransitTable[toIndex(OMX_StateWaitForResources)][toIndex(OMX_StateLoaded)] = doStateWaitforResources_Loaded;
    thiz->mStateTransitTable[toIndex(OMX_StateWaitForResources)][toIndex(OMX_StateIdle)]   = doStateWaitforResources_Idle;

    memset(thiz->mPrerollStateTransitTable, 0, sizeof(doStateTransition)*25);
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateLoaded)][toIndex(OMX_StateIdle)]             = doPrerollStateLoaded_Idle;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateLoaded)][toIndex(OMX_StateWaitForResources)] = doPrerollStateLoaded_WaitforResources;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StateLoaded)]             = doPrerollStateIdle_Loaded;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StateExecuting)]          = doPrerollStateIdle_Executing;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateIdle)][toIndex(OMX_StatePause)]              = doPrerollStateIdle_Pause;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateExecuting)][toIndex(OMX_StateIdle)]          = doPrerollStateExecuting_Idle;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateExecuting)][toIndex(OMX_StatePause)]         = doPrerollStateExecuting_Pause;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StatePause)][toIndex(OMX_StateIdle)]              = doPrerollStatePause_Idle;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StatePause)][toIndex(OMX_StateExecuting)]         = doPrerollStatePause_Executing;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateWaitForResources)][toIndex(OMX_StateLoaded)] = doPrerollStateWaitforResources_Loaded;
    thiz->mPrerollStateTransitTable[toIndex(OMX_StateWaitForResources)][toIndex(OMX_StateIdle)]   = doSPrerolltateWaitforResources_Idle;

    thiz->mParametersDB            = createMagMiniDB(64);

    thiz->mPortTreeRoot            = NULL;
    thiz->mpCallbacks              = NULL;
    thiz->mpAppData                = NULL;

    thiz->mStartPortNumber         = *((OMX_U32 *)params + 0);
    thiz->mPorts                   = *((OMX_U32 *)params + 1);

    thiz->mFlushingPorts           = kInvalidPortIndex;

    INIT_LIST(&thiz->mAVSyncBusyBufLH);
    INIT_LIST(&thiz->mAVSyncFreeBufLH);

    thiz->mhClockPort              = NULL;

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhAVSyncMutex);

    Mag_CreateEventGroup(&thiz->mClkStChangeEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mClkStartRunningEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mClkStChangeEvtGrp, thiz->mClkStartRunningEvt);
}

static void MagOmxComponentImpl_destructor(MagOmxComponentImpl thiz, MagOmxComponentImplVtable vtab){
    OMX_U32 i;

    COMP_LOGV(getRoot(thiz), "Enter!");

    thiz->mLooper->clear(thiz->mLooper);
    thiz->mLooper->waitOnAllDone(thiz->mLooper);

    thiz->mBufferLooper->clear(thiz->mBufferLooper);
    thiz->mBufferLooper->waitOnAllDone(thiz->mBufferLooper);

    destroyMagMiniDB(&thiz->mParametersDB);
    Mag_DestroyMutex(&thiz->mhMutex);

    Mag_DestroyEvent(&thiz->mClkStartRunningEvt);
    Mag_DestroyEventGroup(&thiz->mClkStChangeEvtGrp);

    destroyMagMessage(&thiz->mCmdSetStateMsg);
    destroyMagMessage(&thiz->mCmdPortDisableMsg);
    destroyMagMessage(&thiz->mCmdPortEnableMsg);
    destroyMagMessage(&thiz->mCmdFlushMsg);
    destroyMagMessage(&thiz->mCmdMarkBufferMsg);
    if (thiz->mPortDataMsgList){
        for (i = 0; i < thiz->mPorts; i++){
            destroyMagMessage(&thiz->mPortDataMsgList[i]);
        }
        mag_freep((void **)&thiz->mPortDataMsgList);
    }

    destroyLooper(&thiz->mLooper);
    destroyHandler(&thiz->mMsgHandler);

    destroyLooper(&thiz->mBufferLooper);
    destroyHandler(&thiz->mBufferMsgHandler);
}


