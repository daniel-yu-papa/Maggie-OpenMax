
AllocateClass(MagOmxComponent, Base);

static MagOmxComponent *getBase(OMX_HANDLETYPE hComponent) {
  return (MagOmxComponent *)
      ((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate;
}

/*Running in the thread space*/
void MagOmxComponent_CmdProcessEntry(void *msg, void *priv){
    OMX_ERRORTYPE ret;
    MagOmxCmdMsg_t *pCmdMsg = (MagOmxCmdMsg_t *)msg;
    MagOmxComponent compClass = (MagOmxComponent)priv;
    OMX_COMPONENTTYPE *compHandle = compClass->mComponentHandle;
        
    switch (pCmdMsg->Cmd)
    {
        case OMX_CommandStateSet :
            OMX_STATETYPE targetST = (OMX_STATETYPE)pCmdMsg->nParam;
            
            ret = compClass->setState(compClass, targetST);
            compClass->postEvent(compClass, OMX_EventCmdComplete, OMX_CommandStateSet, targetST, (OMX_PTR)&ret);
            break;

        case OMX_CommandPortDisable :

            break;
            
        case OMX_CommandPortEnable :

            break;
            
        case OMX_CommandFlush :

            break;
            
        case OMX_CommandMarkBuffer :
            break;
            
        default :
            AGILE_LOGE("unhandled command : %d\n", pCmdMsg->Cmd);
            break;
    }

}

static OMX_ERRORTYPE GetComponentVersionWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STRING pComponentName,
                OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                OMX_OUT OMX_UUIDTYPE* pComponentUUID){
    return getBase(hComponent)->GetComponentVersion(hComponent, 
                                                    pComponentName,
                                                    pComponentVersion,
                                                    pSpecVersion,
                                                    pComponentUUID);
}

static OMX_ERRORTYPE SendCommandWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData){
    return getBase(hComponent)->SendCommand(hComponent, 
                                            Cmd,
                                            nParam1,
                                            pCmdData);
}

static OMX_ERRORTYPE GetParameterWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nParamIndex,  
                OMX_INOUT OMX_PTR pComponentParameterStructure){
    return getBase(hComponent)->GetParameter(hComponent, 
                                             nParamIndex,
                                             pComponentParameterStructure);
}

static OMX_ERRORTYPE SetParameterWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){
    return getBase(hComponent)->SetParameter(hComponent, 
                                             nIndex,
                                             pComponentParameterStructure);
}

static OMX_ERRORTYPE GetConfigWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){
    return getBase(hComponent)->GetConfig(hComponent, 
                                          nIndex,
                                          pComponentConfigStructure);
}

static OMX_ERRORTYPE SetConfigWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){
    return getBase(hComponent)->SetConfig(hComponent, 
                                          nIndex,
                                          pComponentConfigStructure);
}

static OMX_ERRORTYPE GetExtensionIndexWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType){
    return getBase(hComponent)->GetExtensionIndex(hComponent, 
                                                  cParameterName,
                                                  pIndexType);
}


static OMX_ERRORTYPE GetStateWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STATETYPE* pState){
    return getBase(hComponent)->GetState(hComponent, 
                                         pState);
}

static OMX_ERRORTYPE ComponentTunnelRequestWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPort,
                OMX_IN  OMX_HANDLETYPE hTunneledComp,
                OMX_IN  OMX_U32 nTunneledPort,
                OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup){
    return getBase(hComponent)->ComponentTunnelRequest(hComponent, 
                                                       nPort,
                                                       hTunneledComp,
                                                       nTunneledPort,
                                                       pTunnelSetup);
}

static OMX_ERRORTYPE UseBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){
    return getBase(hComponent)->UseBuffer(hComponent, 
                                          ppBufferHdr,
                                          nPortIndex,
                                          pAppPrivate,
                                          nSizeBytes,
                                          pBuffer);
}

static OMX_ERRORTYPE AllocateBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){
    return getBase(hComponent)->AllocateBuffer(hComponent, 
                                               ppBuffer,
                                               nPortIndex,
                                               pAppPrivate,
                                               nSizeBytes);
}

static OMX_ERRORTYPE FreeBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    return getBase(hComponent)->FreeBuffer(hComponent, 
                                           nPortIndex,
                                           pBuffer);
}

static OMX_ERRORTYPE EmptyThisBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    return getBase(hComponent)->EmptyThisBuffer(hComponent, 
                                                pBuffer);
}

static OMX_ERRORTYPE FillThisBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    return getBase(hComponent)->FillThisBuffer(hComponent, 
                                               pBuffer);
}

static OMX_ERRORTYPE SetCallbacksWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){
    return getBase(hComponent)->SetCallbacks(hComponent, 
                                               pCallbacks,
                                               pAppData);
}

static OMX_ERRORTYPE ComponentDeInitWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent){
    return getBase(hComponent)->ComponentDeInit(hComponent);
}

static OMX_ERRORTYPE UseEGLImageWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){
    return getBase(hComponent)->UseEGLImage(hComponent,
                                            ppBufferHdr,
                                            nPortIndex,
                                            pAppPrivate,
                                            eglImage);
}

static OMX_ERRORTYPE ComponentRoleEnumWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
         		OMX_OUT OMX_U8 *cRole,
         		OMX_IN OMX_U32 nIndex){
    return getBase(hComponent)->ComponentRoleEnum(hComponent,
                                                  cRole,
                                                  nIndex);
}

/*Virtual member function implementation*/
OMX_ERRORTYPE virtual_MagOmxComponent_GetComponentVersion(
                OMX_IN  MagOmxComponent hComponent,
                OMX_OUT OMX_STRING pComponentName,
                OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                OMX_OUT OMX_UUIDTYPE* pComponentUUID){
    pSpecVersion->s.nVersionMajor = 1;
    pSpecVersion->s.nVersionMinor = 2;
    pSpecVersion->s.nRevision     = 0;
    pSpecVersion->s.nStep         = 0;

    pComponentVersion->nVersion   = 0x00000001;
    
}

OMX_ERRORTYPE virtual_MagOmxComponent_SendCommand(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData){
    MagOmxCmdMsg_t *msg;
    
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }
    
    msg = (MagOmxCmdMsg_t *)malloc(sizeof(MagOmxCmdMsg_t));

    msg->Cmd      = Cmd;
    msg->nParam   = nParam1;
    msg->pCmdData = pCmdData;

    Mag_MsgChannelSend(hComponent->mCmdEvtChannel, msg, sizeof(MagOmxCmdMsg_t));
}

OMX_ERRORTYPE virtual_MagOmxComponent_GetParameter(
                OMX_IN  MagOmxComponent hComponent, 
                OMX_IN  OMX_INDEXTYPE nParamIndex,  
                OMX_INOUT OMX_PTR pComponentParameterStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_SetParameter(
                OMX_IN  MagOmxComponent hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_GetConfig(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_SetConfig(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){

}

OMX_ERRORTYPE virtual_MagOmxComponent_GetExtensionIndex(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType){

}


OMX_ERRORTYPE virtual_MagOmxComponent_GetState(
                OMX_IN  MagOmxComponent hComponent,
                OMX_OUT OMX_STATETYPE* pState){
    
}

OMX_ERRORTYPE virtual_MagOmxComponent_ComponentTunnelRequest(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_U32 nPort,
                OMX_IN  OMX_HANDLETYPE hTunneledComp,
                OMX_IN  OMX_U32 nTunneledPort,
                OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup){

}

OMX_ERRORTYPE virtual_MagOmxComponent_UseBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){
    MagOmxPort port = hComponent->getPort(hComponent, nPortIndex);
    if (NULL != port){
        return port->UseBuffer(port, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

OMX_ERRORTYPE virtual_MagOmxComponent_AllocateBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){
    MagOmxPort port = hComponent->getPort(hComponent, nPortIndex);
    if (NULL != port){
        return port->AllocateBuffer(port, ppBuffer, pAppPrivate, nSizeBytes);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

OMX_ERRORTYPE virtual_MagOmxComponent_FreeBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxPort port = hComponent->getPort(hComponent, nPortIndex);
    if (NULL != port){
        return port->FreeBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

OMX_ERRORTYPE virtual_MagOmxComponent_EmptyThisBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    /*TODO: add the port enable and component state judgement*/
    MagOmxPort port = hComponent->getPort(hComponent, pBuffer->nInputPortIndex);
    if ((NULL != port) && (port->mPortDef.eDir == OMX_DirInput)){
        return port->sendBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

OMX_ERRORTYPE virtual_MagOmxComponent_FillThisBuffer(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){
    /*TODO: add the port enable and component state judgement*/
    MagOmxPort port = hComponent->getPort(hComponent, pBuffer->nOutputPortIndex);
    if ((NULL != port) && (port->mPortDef.eDir == OMX_DirOutput)){
        return port->sendBuffer(port, pBuffer);
    }else{
        return OMX_ErrorBadPortIndex;
    }
}

OMX_ERRORTYPE virtual_MagOmxComponent_SetCallbacks(
                OMX_IN  MagOmxComponent hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){
    hComponent->mCallbacks = pCallbacks;
    hComponent->mAppData   = pAppData;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE virtual_MagOmxComponent_ComponentDeInit(
                OMX_IN  MagOmxComponent hComponent){

}

OMX_ERRORTYPE virtual_MagOmxComponent_UseEGLImage(
                OMX_IN  MagOmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){

}

OMX_ERRORTYPE virtual_MagOmxComponent_ComponentRoleEnum(
                OMX_IN  MagOmxComponent hComponent,
         		OMX_OUT OMX_U8 *cRole,
         		OMX_IN OMX_U32 nIndex){

}

/*Member function implementation*/
OMX_COMPONENTTYPE *MagOmxComponent_Create(MagOmxComponent pBase){
    OMX_COMPONENTTYPE* comp = (OMX_COMPONENTTYPE *)calloc(1, sizeof(OMX_COMPONENTTYPE));

    if (NULL != comp){
        comp->nSize = sizeof(OMX_COMPONENTTYPE);
        comp->nVersion.s.nVersionMajor = 1;
        comp->nVersion.s.nVersionMinor = 2;
        comp->nVersion.s.nRevision     = 0;
        comp->nVersion.s.nStep         = 0;
        comp->pComponentPrivate        = pBase;
        comp->pApplicationPrivate      = NULL;

        comp->GetComponentVersion      = GetComponentVersionWrapper;
        comp->SendCommand              = SendCommandWrapper;
        comp->GetParameter             = GetParameterWrapper;
        comp->SetParameter             = SetParameterWrapper;
        comp->GetConfig                = GetConfigWrapper;
        comp->SetConfig                = SetConfigWrapper;
        comp->GetExtensionIndex        = GetExtensionIndexWrapper;
        comp->GetState                 = GetStateWrapper;
        comp->ComponentTunnelRequest   = ComponentTunnelRequestWrapper;
        comp->UseBuffer                = UseBufferWrapper;
        comp->AllocateBuffer           = AllocateBufferWrapper;
        comp->FreeBuffer               = FreeBufferWrapper;
        comp->EmptyThisBuffer          = EmptyThisBufferWrapper;
        comp->FillThisBuffer           = FillThisBufferWrapper;
        comp->SetCallbacks             = SetCallbacksWrapper;
        comp->ComponentDeInit          = ComponentDeInitWrapper;
        comp->UseEGLImage              = UseEGLImageWrapper;
        comp->ComponentRoleEnum        = ComponentRoleEnumWrapper;

        comp->setHandle(pBase, comp);
    }
    
    return comp;
}

void MagOmxComponent_setHandle(MagOmxComponent pBase, OMX_COMPONENTTYPE *handle){
    pBase->mComponentHandle = handle;
}

OMX_ERRORTYPE MagOmxComponent_postEvent(MagOmxComponent pBase, OMX_EVENTTYPE event, OMX_U32 param1, OMX_U32 param2, OMX_PTR pEventData){
    if (NULL != pBase->mCallbacks){
        return pBase->mCallbacks->EventHandler(pBase->mComponentHandle, pBase->mAppData, event, param1, param2, pEventData);
    }
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE MagOmxComponent_postFillBufferDone(MagOmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer){
    if (NULL != pBase->mCallbacks){
        return pBase->mCallbacks->FillBufferDone(pBase->mComponentHandle, pBase->mAppData, pBuffer);
    }
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE MagOmxComponent_postEmptyBufferDone(MagOmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer){
    if (NULL != pBase->mCallbacks){
        return pBase->mCallbacks->EmptyBufferDone(pBase->mComponentHandle, pBase->mAppData, pBuffer);
    }
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE MagOmxComponent_setState(MagOmxComponent pBase, OMX_STATETYPE targetState){
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    if (targetState == OMX_StateLoaded){
        if (pBase->mState == OMX_StateMax){
            pBase->mState = targetState;
        }else if (pBase->mState == OMX_StateIdle){
            ret = pBase->Release();
            if (OMX_ErrorNone == ret){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Failed to do Component Release. ret = %d. Keep current state: Idle", ret);
            }
        }else if (pBase->mState == OMX_StateWaitForResources){
            ret = pBase->FreeResources();
            if (OMX_ErrorNone != ret){
                AGILE_LOGE("Failed to do Component FreeResources(). Should not be here!!");
            }
            pBase->mState = targetState;
        }else if (pBase->mState == targetState){
            ret = OMX_ErrorSameState;
        }else{
            ret = OMX_ErrorIncorrectStateTransition;
        }
    }else if (targetState == OMX_StateIdle){
        if (pBase->mState == OMX_StateLoaded){
            ret = pBase->Prepare();
            if (OMX_ErrorNone == ret){
                pBase->mState = targetState;
            }else if (OMX_ErrorResourcesLost == ret){
                pBase->mState = OMX_StateWaitForResources;
            }else{
                AGILE_LOGE("Failed to do Component Prepare. ret = %d. Keep current state: Loaded", ret);
            }
        }else if (pBase->mState == OMX_StateWaitForResources){
            if (pBase->AllResourcesReady()){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Not all resources are ready. Keep current state: WaitForResources");
                ret = OMX_ErrorResourcesLost;
            }
        }else if ((pBase->mState == OMX_StateExecuting) ||
                  (pBase->mState == OMX_StatePause)){
            if (pBase->Stop()){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Failed to do Component Stop. ret = %d. Keep current state: Executing", ret);
            }
        }else if (pBase->mState == targetState){
            ret = OMX_ErrorSameState;
        }else{
            ret = OMX_ErrorIncorrectStateTransition;
        }
    }else if (targetState == OMX_StateExecuting){
        if (pBase->mState == OMX_StateIdle){
            if (pBase->Start()){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Failed to do Component Start. ret = %d. Keep current state: Idle");
            }
        }else if (pBase->mState == OMX_StatePause){
            if (pBase->Resume()){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Failed to do Component Resume. ret = %d. Keep current state: Pause");
            }
        }else if (pBase->mState == targetState){
            ret = OMX_ErrorSameState;
        }else{
            ret = OMX_ErrorIncorrectStateTransition;
        }
    }else if (targetState == OMX_StatePause){
        if (pBase->mState == OMX_StateExecuting){
            if (pBase->Pause()){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Failed to do Component Pause. ret = %d. Keep current state: Executing");
            }
        }else if (pBase->mState == OMX_StateIdle){
            if (pBase->Preroll()){
                pBase->mState = targetState;
            }else{
                AGILE_LOGE("Failed to do Component Preroll. ret = %d. Keep current state: Idle");
            }
        }else if (pBase->mState == targetState){
            ret = OMX_ErrorSameState;
        }else{
            ret = OMX_ErrorIncorrectStateTransition;
        }
    }else{
        AGILE_LOGE("Invalid target state: %d", targetState);
        ret = OMX_ErrorIncorrectStateTransition;
    }
    return ret;
}

OMX_ERRORTYPE MagOmxComponent_addPort(MagOmxComponent pBase, OMX_U32 nPortIndex, MagOmxPort pPort){
    pBase->mPortTreeRoot = rbtree_insert(pBase->mPortTreeRoot, nPortIndex, (void *)pPort);
    return OMX_ErrorNone;
}

MagOmxPort MagOmxComponent_getPort(MagOmxComponent pBase, OMX_U32 nPortIndex){
    return (MagOmxPort)rbtree_get(pBase->mPortTreeRoot, nPortIndex);
}


/*Class Constructor/Destructor*/
static void MagOmxComponent_initialize(Class this){
    MagOmxComponentVtableInstance.GetComponentVersion    = virtual_MagOmxComponent_GetComponentVersion;
    MagOmxComponentVtableInstance.SendCommand            = virtual_MagOmxComponent_SendCommand;
    MagOmxComponentVtableInstance.GetParameter           = virtual_MagOmxComponent_GetParameter;
    MagOmxComponentVtableInstance.SetParameter           = virtual_MagOmxComponent_SetParameter;
    MagOmxComponentVtableInstance.GetConfig              = virtual_MagOmxComponent_GetConfig;
    MagOmxComponentVtableInstance.SetConfig              = virtual_MagOmxComponent_SetConfig;
    MagOmxComponentVtableInstance.GetExtensionIndex      = virtual_MagOmxComponent_GetExtensionIndex;
    MagOmxComponentVtableInstance.GetState               = virtual_MagOmxComponent_GetState;
    MagOmxComponentVtableInstance.ComponentTunnelRequest = virtual_MagOmxComponent_ComponentTunnelRequest;
    MagOmxComponentVtableInstance.UseBuffer              = virtual_MagOmxComponent_UseBuffer;
    MagOmxComponentVtableInstance.AllocateBuffer         = virtual_MagOmxComponent_AllocateBuffer;
    MagOmxComponentVtableInstance.FreeBuffer             = virtual_MagOmxComponent_FreeBuffer;
    MagOmxComponentVtableInstance.EmptyThisBuffer        = virtual_MagOmxComponent_EmptyThisBuffer;
    MagOmxComponentVtableInstance.FillThisBuffer         = virtual_MagOmxComponent_FillThisBuffer;
    MagOmxComponentVtableInstance.SetCallbacks           = virtual_MagOmxComponent_SetCallbacks;
    MagOmxComponentVtableInstance.ComponentDeInit        = virtual_MagOmxComponent_ComponentDeInit;
    MagOmxComponentVtableInstance.UseEGLImage            = virtual_MagOmxComponent_UseEGLImage;
    MagOmxComponentVtableInstance.ComponentRoleEnum      = virtual_MagOmxComponent_ComponentRoleEnum;

    /*pure virtual functions and must be overrided*/
    MagOmxComponentVtableInstance.Start                  = NULL;
    MagOmxComponentVtableInstance.Stop                   = NULL;
    MagOmxComponentVtableInstance.Resume                 = NULL;
    MagOmxComponentVtableInstance.Pause                  = NULL;
    MagOmxComponentVtableInstance.Prepare                = NULL;
    MagOmxComponentVtableInstance.Preroll                = NULL;
    MagOmxComponentVtableInstance.AllResourcesReady      = NULL;
    MagOmxComponentVtableInstance.FreeResources          = NULL;
}

static void MagOmxComponent_constructor(MagOmxComponent thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmxComponent));
    chain_constructor(MagOmxComponent, thiz, params);

    thiz->mCallbacks = NULL;
    thiz->mAppData   = NULL;
    thiz->mState     = OMX_StateMax;
    thiz->mPortTreeRoot = NULL;
    
    thiz->Create              = MagOmxComponent_Create;
    thiz->setHandle           = MagOmxComponent_setHandle;
    thiz->postEvent           = MagOmxComponent_postEvent;
    thiz->postFillBufferDone  = MagOmxComponent_postFillBufferDone;
    thiz->postEmptyBufferDone = MagOmxComponent_postEmptyBufferDone;
    thiz->setState            = MagOmxComponent_setState;
    thiz->addPort             = MagOmxComponent_addPort;
    thiz->getPort             = MagOmxComponent_getPort;
    
    mc_ret = Mag_MsgChannelCreate(&(thiz->mCmdEvtChannel));
    if (MAG_ErrNone == mc_ret){
        Mag_MsgChannelReceiverAttach(thiz->mCmdEvtChannel, MagOmxComponent_CmdProcessEntry, thiz);
    }  
}

static void MagOmxComponent_destructor(MagOmxComponent thiz, TriangleVtable vtab){
    AGILE_LOGV("Enter!");
}


