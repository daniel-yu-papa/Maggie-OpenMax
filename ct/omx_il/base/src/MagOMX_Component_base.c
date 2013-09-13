#include <errno.h>
#include <stdlib.h>

#include "OMXComponent_base.h"
#include "OMXComponent_msg.h"

static OMXComponentPort_base_t *OMXComponent_Priv_GetPort(OMX_IN OMX_COMPONENTTYPE *comp, OMX_IN OMX_U32 nPortIndex){
    OMXComponentPrivateBase_t *priv = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXComponentPort_base_t *portEntry = NULL;
    List_t *tmpNode = priv->portListHead.next;
    
    while (tmpNode != &priv->portListHead){
        portEntry = (OMXComponentPort_base_t *)list_entry(tmpNode, OMXComponentPort_base_t, node);
        if (nPortIndex == portEntry->sPortParam.nPortIndex)
            return portEntry;

        tmpNode = tmpNode->next;
    }
    return NULL;
}

static OMX_ERRORTYPE OMXComponent_AllocateBuffer(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
                        OMX_IN OMX_U32 nPortIndex,
                        OMX_IN OMX_PTR pAppPrivate,
                        OMX_IN OMX_U32 nSizeBytes){
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;
    
    OMXComponentPort_base_t *port = OMXComponent_Priv_GetPort((OMX_COMPONENTTYPE *)hComponent, nPortIndex);

    if (NULL == port){
        AGILE_LOGE("failed to get the port in the index %d", nPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    ret = OMXComponentPort_base_AllocateBuffer(port, pBuffer, pAppPrivate, nSizeBytes);

    return ret;
}

static OMX_ERRORTYPE OMXComponent_DeInit(
                        OMX_IN OMX_HANDLETYPE hComponent){

}

static OMX_ERRORTYPE OMXComponent_RoleEnum(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_OUT OMX_STRING cRole,
                        OMX_IN OMX_U32 nIndex){

}

static OMX_ERRORTYPE OMXComponent_TunnelRequest(
                        OMX_IN OMX_HANDLETYPE hComp,
                        OMX_IN OMX_U32 nPort,
                        OMX_IN OMX_HANDLETYPE hTunneledComp,
                        OMX_IN OMX_U32 nTunneledPort,
                        OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup){

}

static OMX_ERRORTYPE OMXComponent_EmptyThisBuffer(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){

}

static OMX_ERRORTYPE OMXComponent_FillThisBuffer(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    portBufferOperationMsg_t msg;
    OMXComponentPort_base_t *port;
    OMX_DIRTYPE direction;
    
    port = getPortFromBufferHeader(pBuffer, &direction);

    if(OMX_DirOutput != direction){
        AGILE_LOGE("The FillThisBuffer action could only be done with output port(dir: %d)!", direction);
        return OMX_ErrorPortsNotCompatible;
    }
    msg.action = DO_FILL_BUFFER;
    msg.hComponent = hComponent;
    msg.pBuffer = pBuffer;
    Mag_MsgChannelSend(port->bufferMgrHandle, &msg, sizeof(portBufferOperationMsg_t));
}

static OMX_ERRORTYPE OMXComponent_FreeBuffer(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_U32 nPortIndex,
                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){

}

static OMX_ERRORTYPE OMXComponent_GetVersion(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_OUT OMX_STRING pComponentName,
                        OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                        OMX_OUT OMX_VERSIONTYPE* pSpecVersion){

}

static OMX_ERRORTYPE OMXComponent_GetConfig(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_INOUT OMX_PTR pComponentConfigStructure){
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_COMPONENTTYPE *pCompObj = NULL;
    
    if ((NULL == hComponent) || (NULL == pComponentConfigStructure)){
        return OMX_ErrorBadParameter;
    }

    pCompObj = (OMX_COMPONENTTYPE *)hComponent;
    pPrivData_base = (OMXComponentPrivateBase_t *)pCompObj->pComponentPrivate;

    if (NULL == pPrivData_base)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks->getConfig)
        return OMX_ErrorNotImplemented;

    return pPrivData_base->subComp_callbacks->getConfig(nIndex, pComponentConfigStructure);
}

static OMX_ERRORTYPE OMXComponent_GetExtensionIndex(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_STRING cParameterName,
                        OMX_OUT OMX_INDEXTYPE* pIndexType){

}

static OMX_ERRORTYPE OMXComponent_GetParameter(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nParamIndex,
                        OMX_INOUT OMX_PTR pComponentParameterStructure){
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_COMPONENTTYPE *pCompObj = NULL;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    pCompObj = (OMX_COMPONENTTYPE *)hComponent;
    pPrivData_base = (OMXComponentPrivateBase_t *)pCompObj->pComponentPrivate;

    if (NULL == pPrivData_base)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks->getParameter)
        return OMX_ErrorNotImplemented;

    return pPrivData_base->subComp_callbacks->getParameter(nParamIndex, pComponentParameterStructure);

}

static OMX_ERRORTYPE OMXComponent_GetState(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_OUT OMX_STATETYPE* pState){

}

/*Running in the thread space*/
void cmdProcessEntry(void *msg, void *priv){
    OMXCommandMsg_t *pCmdMsg = (OMXCommandMsg_t *)msg;
    OMX_ERRORTYPE ret;
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_COMPONENTTYPE *comp = (OMX_COMPONENTTYPE *)pCmdMsg->hComp;
    pPrivData_base = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    
    switch (pCmdMsg->Cmd)
    {
        case OMX_CommandStateSet :
            ret = OMX_StateTransition_Process((OMX_COMPONENTTYPE *)pCmdMsg->hComp, pCmdMsg->Cmd);

            if (pPrivData_base->callbacks.EventHandler){
                if (OMX_ErrorNone == ret){
                    pPrivData_base->callbacks.EventHandler(pCmdMsg->hComp, 
                                                           comp->pApplicationPrivate, 
                                                           OMX_EventCmdComplete, 
                                                           OMX_CommandStateSet,
                                                           pCmdMsg->nParam,
                                                           pCmdMsg->pCmdData);
                }else{
                    pPrivData_base->callbacks.EventHandler(pCmdMsg->hComp, 
                                                           comp->pApplicationPrivate, 
                                                           OMX_EventError, 
                                                           ret,
                                                           0,
                                                           NULL);
                }
            }else{
                AGILE_LOGE("the callback event handler is NULL.");
            }
            break;

        case OMX_CommandPortDisable :
            //port_disable(cmd);
            //goto bail;

        case OMX_CommandPortEnable :
            //port_enable(cmd);
            //goto bail;

        case OMX_CommandFlush :
            //port_flush(cmd);
            //goto bail;

        case OMX_CommandMarkBuffer :

        default :
            AGILE_LOGE("unhandled command : %d\n", pCmdMsg->Cmd);
            break;
    }

}

static OMX_ERRORTYPE OMXComponent_SendCommand(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_COMMANDTYPE Cmd,
                        OMX_IN OMX_U32 nParam,
                        OMX_IN OMX_PTR pCmdData){
                        
    OMXCommandMsg_t *msg;
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_COMPONENTTYPE *pCompObj = NULL;
    
    if (NULL == hComponent){
        return OMX_ErrorBadParameter;
    }

    pCompObj = (OMX_COMPONENTTYPE *)hComponent;
    pPrivData_base = (OMXComponentPrivateBase_t *)pCompObj->pComponentPrivate;
    
    msg = (OMXCommandMsg_t *)malloc(sizeof(OMXCommandMsg_t));

    msg->hComp    = hComponent;
    msg->Cmd      = Cmd;
    msg->nParam   = nParam;
    msg->pCmdData = pCmdData;

    Mag_MsgChannelSend(pPrivData_base->cmdChannel, msg, sizeof(OMXCommandMsg_t));
}

static OMX_ERRORTYPE OMXComponent_SetCallbacks(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_CALLBACKTYPE* pCallbacks,
                        OMX_IN OMX_PTR pAppData){
    OMX_COMPONENTTYPE *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMXComponentPrivateBase_t *priv = (OMXComponentPrivateBase_t *)pComp->pComponentPrivate;
    
    if (NULL == pComp){
        return OMX_ErrorBadParameter;
    }

    if(OMX_StateLoaded != priv->state){
        return OMX_ErrorInvalidState;
    }
    
    priv->callbacks = *pCallbacks;
    pComp->pApplicationPrivate = pAppData;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE OMXComponent_SetConfig(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_INOUT OMX_PTR pComponentConfigStructure){
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_COMPONENTTYPE *pCompObj = NULL;
    
    if ((NULL == hComponent) || (NULL == pComponentConfigStructure)){
        return OMX_ErrorBadParameter;
    }

    pCompObj = (OMX_COMPONENTTYPE *)hComponent;
    pPrivData_base = (OMXComponentPrivateBase_t *)pCompObj->pComponentPrivate;

    if (NULL == pPrivData_base)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks->setConfig)
        return OMX_ErrorNotImplemented;

    return pPrivData_base->subComp_callbacks->setConfig(nIndex, pComponentConfigStructure);
}

static OMX_ERRORTYPE OMXComponent_SetParameter(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_IN OMX_INDEXTYPE nIndex,
                        OMX_IN OMX_PTR pComponentParameterStructure){

    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_COMPONENTTYPE *pCompObj = NULL;
    
    if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    pCompObj = (OMX_COMPONENTTYPE *)hComponent;
    pPrivData_base = (OMXComponentPrivateBase_t *)pCompObj->pComponentPrivate;

    if (NULL == pPrivData_base)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks)
        return OMX_ErrorInsufficientResources;

    if (NULL == pPrivData_base->subComp_callbacks->setParameter)
        return OMX_ErrorNotImplemented;

    return pPrivData_base->subComp_callbacks->setParameter(nIndex, pComponentParameterStructure);
}

static OMX_ERRORTYPE OMXComponent_UseBuffer(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                        OMX_IN OMX_U32 nPortIndex,
                        OMX_IN OMX_PTR pAppPrivate,
                        OMX_IN OMX_U32 nSizeBytes,
                        OMX_IN OMX_U8* pBuffer){
    OMX_ERRORTYPE ret;
    
    if (NULL == hComponent)
        return OMX_ErrorBadParameter;
    
    OMXComponentPort_base_t *port = OMXComponent_Priv_GetPort((OMX_COMPONENTTYPE *)hComponent, nPortIndex);

    if (NULL == port){
        AGILE_LOGE("failed to get the port in the index %d", nPortIndex);
        return OMX_ErrorBadPortIndex;
    }

    ret = OMXComponentPort_base_UseBuffer(port, pBuffer, pAppPrivate, nSizeBytes, pBuffer);

    return ret;
}

static OMX_ERRORTYPE OMXComponent_UseEGLImage(
                        OMX_IN OMX_HANDLETYPE hComponent,
                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                        OMX_IN OMX_U32 nPortIndex,
                        OMX_IN OMX_PTR pAppPrivate,
                        OMX_IN void* pBuffer){

}

static OMX_ERRORTYPE OMXComponent_Base_Constructor(
                     OMX_HANDLETYPE *hComp,
                     OMX_BOOL       isSyncMode){

    OMX_COMPONENTTYPE *pCompObj = NULL;
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    OMX_S32 rc;
    MagErr_t mc_ret;
    
    if (NULL == *hComp){
        *hComp = (OMX_COMPONENTTYPE *)calloc(1, sizeof(OMX_COMPONENTTYPE));
        if (NULL == *hComp){
            AGILE_LOG_FATAL("failed to allocate memory for OMX Component");
            goto failure;
        }
    }
    pCompObj = (OMX_COMPONENTTYPE *)(*hComp);

    if (NULL == pCompObj->pComponentPrivate){
        pPrivData_base = (OMXComponentPrivateBase_t *)calloc(1, sizeof(OMXComponentPrivateBase_t));
        if(NULL == pPrivData_base){
            AGILE_LOG_FATAL("failed to allocate memory for OMX Base Component Private Data");
            goto failure;
        }
        pCompObj->pComponentPrivate = pPrivData_base;
    }else{
        pPrivData_base = (OMXComponentPrivateBase_t *)pCompObj->pComponentPrivate;
    }
    
    pCompObj->AllocateBuffer         = OMXComponent_AllocateBuffer;
    pCompObj->ComponentDeInit        = OMXComponent_DeInit;
    pCompObj->ComponentRoleEnum      = OMXComponent_RoleEnum;
    pCompObj->ComponentTunnelRequest = OMXComponent_TunnelRequest;
    pCompObj->EmptyThisBuffer        = OMXComponent_EmptyThisBuffer;
    pCompObj->FillThisBuffer         = OMXComponent_FillThisBuffer;
    pCompObj->FreeBuffer             = OMXComponent_FreeBuffer;
    pCompObj->GetComponentVersion    = OMXComponent_GetVersion;
    pCompObj->GetConfig              = OMXComponent_GetConfig;
    pCompObj->GetExtensionIndex      = OMXComponent_GetExtensionIndex;
    pCompObj->GetParameter           = OMXComponent_GetParameter;
    pCompObj->GetState               = OMXComponent_GetState;
    pCompObj->SendCommand            = OMXComponent_SendCommand;
    pCompObj->SetCallbacks           = OMXComponent_SetCallbacks;
    pCompObj->SetConfig              = OMXComponent_SetConfig;
    pCompObj->SetParameter           = OMXComponent_SetParameter;
    pCompObj->UseBuffer              = OMXComponent_UseBuffer;
    pCompObj->UseEGLImage            = OMXComponent_UseEGLImage;

    /*initialize the base component private data structure*/
    rc = pthread_mutex_init(&pPrivData_base->stateTransMutex, NULL);
    if (rc){
        AGILE_LOGE("failed to initialize the stateTransMutex. (error: %s)", strerror(errno));
        goto failure;
    }
    
    mc_ret = Mag_MsgChannelCreate(&pPrivData_base->cmdChannel);
    if (MAG_ErrNone != mc_ret)
        goto failure;

    Mag_MsgChannelReceiverAttach(pPrivData_base->cmdChannel, cmdProcessEntry, pCompObj);

    if (MAG_ErrNone != Mag_CreateEvent(&pPrivData_base->Event_OMX_AllocateBufferDone)){
        AGILE_LOGE("failed to create the event: Event_OMX_AllocateBufferDone");
        goto event_failure;
    }
    
    if (MAG_ErrNone != Mag_CreateEvent(&pPrivData_base->Event_OMX_UseBufferDone)){
        AGILE_LOGE("failed to create the event: Event_OMX_UseBufferDone");
        goto event_failure;
    }
    
    if (MAG_ErrNone != Mag_CreateEventGroup(&pPrivData_base->EventGroup_PortBufferAllocated)){
        AGILE_LOGE("failed to create the event group: EventGroup_PortBufferAllocated");
        goto event_failure;
    }
    
    Mag_AddEventGroup(pPrivData_base->Event_OMX_AllocateBufferDone, pPrivData_base->EventGroup_PortBufferAllocated);
    Mag_AddEventGroup(pPrivData_base->Event_OMX_UseBufferDone, pPrivData_base->EventGroup_PortBufferAllocated);
    
    pPrivData_base->state = OMX_Priv_StateLoaded;
    
    return OMX_ErrorNone;

event_failure:
    if (pPrivData_base->Event_OMX_AllocateBufferDone)
        Mag_DestroyEvent(pPrivData_base->Event_OMX_AllocateBufferDone);

    if (pPrivData_base->Event_OMX_UseBufferDone)
        Mag_DestroyEvent(pPrivData_base->Event_OMX_UseBufferDone);
failure:  
    if (pPrivData_base)
        free(pPrivData_base);
    
    if (pCompObj)
        free(pCompObj);
    
    return OMX_ErrorInsufficientResources;
}

static OMX_ERRORTYPE OMXComponent_Base_Destructor(
                     OMX_HANDLETYPE *hComp,
                     OMX_BOOL       dynamic){
}

OMX_ERRORTYPE OMXComponent_Base_SetCallbacks(OMX_HANDLETYPE hComp, OMXSubComponentCallbacks_t *cb){
    OMX_COMPONENTTYPE *pComp;
    OMXComponentPrivateBase_t *priv;
    
    if(NULL == hComp)
        return OMX_ErrorBadParameter;
    
    pComp = (OMX_COMPONENTTYPE *)hComp;

    priv = (OMXComponentPrivateBase_t *)pComp->pComponentPrivate;
    priv->baseComp_callbacks = cb;
    
    return OMX_ErrorNone;
}

/************************************************/

AllocateClass(OmxComponent, Base);

static OmxComponent *getBase(OMX_HANDLETYPE hComponent) {
  return (OmxComponent *)
      ((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate;
}

/*Running in the thread space*/
void OmxComponent_CmdProcessEntry(void *msg, void *priv){
    OMX_ERRORTYPE ret;
    MagOmxCmdMsg_t *pCmdMsg = (MagOmxCmdMsg_t *)msg;
    OmxComponent compClass = (OmxComponent)priv;
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
OMX_ERRORTYPE virtual_OmxComponent_GetComponentVersion(
                OMX_IN  OmxComponent hComponent,
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

OMX_ERRORTYPE virtual_OmxComponent_SendCommand(
                OMX_IN  OmxComponent hComponent,
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

OMX_ERRORTYPE virtual_OmxComponent_GetParameter(
                OMX_IN  OmxComponent hComponent, 
                OMX_IN  OMX_INDEXTYPE nParamIndex,  
                OMX_INOUT OMX_PTR pComponentParameterStructure){

}

OMX_ERRORTYPE virtual_OmxComponent_SetParameter(
                OMX_IN  OmxComponent hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){

}

OMX_ERRORTYPE virtual_OmxComponent_GetConfig(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){

}

OMX_ERRORTYPE virtual_OmxComponent_SetConfig(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){

}

OMX_ERRORTYPE virtual_OmxComponent_GetExtensionIndex(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType){

}


OMX_ERRORTYPE virtual_OmxComponent_GetState(
                OMX_IN  OmxComponent hComponent,
                OMX_OUT OMX_STATETYPE* pState){
    
}

OMX_ERRORTYPE virtual_OmxComponent_ComponentTunnelRequest(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_U32 nPort,
                OMX_IN  OMX_HANDLETYPE hTunneledComp,
                OMX_IN  OMX_U32 nTunneledPort,
                OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup){

}

OMX_ERRORTYPE virtual_OmxComponent_UseBuffer(
                OMX_IN  OmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){

}

OMX_ERRORTYPE virtual_OmxComponent_AllocateBuffer(
                OMX_IN  OmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){

}

OMX_ERRORTYPE virtual_OmxComponent_FreeBuffer(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

}

OMX_ERRORTYPE virtual_OmxComponent_EmptyThisBuffer(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

}

OMX_ERRORTYPE virtual_OmxComponent_FillThisBuffer(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

}

OMX_ERRORTYPE virtual_OmxComponent_SetCallbacks(
                OMX_IN  OmxComponent hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){
    hComponent->mCallbacks = pCallbacks;
    hComponent->mAppData   = pAppData;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE virtual_OmxComponent_ComponentDeInit(
                OMX_IN  OmxComponent hComponent){

}

OMX_ERRORTYPE virtual_OmxComponent_UseEGLImage(
                OMX_IN  OmxComponent hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){

}

OMX_ERRORTYPE virtual_OmxComponent_ComponentRoleEnum(
                OMX_IN  OmxComponent hComponent,
         		OMX_OUT OMX_U8 *cRole,
         		OMX_IN OMX_U32 nIndex){

}

/*Member function implementation*/
OMX_COMPONENTTYPE *OmxComponent_Create(OmxComponent pBase){
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

void OmxComponent_setHandle(OmxComponent pBase, OMX_COMPONENTTYPE *handle){
    pBase->mComponentHandle = handle;
}

OMX_ERRORTYPE OmxComponent_postEvent(OmxComponent pBase, OMX_EVENTTYPE event, OMX_U32 param1, OMX_U32 param2, OMX_PTR pEventData){
    if (NULL != pBase->mCallbacks){
        return pBase->mCallbacks->EventHandler(pBase->mComponentHandle, pBase->mAppData, event, param1, param2, pEventData);
    }
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE OmxComponent_postFillBufferDone(OmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer){
    if (NULL != pBase->mCallbacks){
        return pBase->mCallbacks->FillBufferDone(pBase->mComponentHandle, pBase->mAppData, pBuffer);
    }
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE OmxComponent_postEmptyBufferDone(OmxComponent pBase, OMX_BUFFERHEADERTYPE* pBuffer){
    if (NULL != pBase->mCallbacks){
        return pBase->mCallbacks->EmptyBufferDone(pBase->mComponentHandle, pBase->mAppData, pBuffer);
    }
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE OmxComponent_setState(OmxComponent pBase, OMX_STATETYPE targetState){
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


/*Class Constructor/Destructor*/
static void OmxComponent_initialize(Class this){
    OmxComponentVtableInstance.GetComponentVersion    = virtual_OmxComponent_GetComponentVersion;
    OmxComponentVtableInstance.SendCommand            = virtual_OmxComponent_SendCommand;
    OmxComponentVtableInstance.GetParameter           = virtual_OmxComponent_GetParameter;
    OmxComponentVtableInstance.SetParameter           = virtual_OmxComponent_SetParameter;
    OmxComponentVtableInstance.GetConfig              = virtual_OmxComponent_GetConfig;
    OmxComponentVtableInstance.SetConfig              = virtual_OmxComponent_SetConfig;
    OmxComponentVtableInstance.GetExtensionIndex      = virtual_OmxComponent_GetExtensionIndex;
    OmxComponentVtableInstance.GetState               = virtual_OmxComponent_GetState;
    OmxComponentVtableInstance.ComponentTunnelRequest = virtual_OmxComponent_ComponentTunnelRequest;
    OmxComponentVtableInstance.UseBuffer              = virtual_OmxComponent_UseBuffer;
    OmxComponentVtableInstance.AllocateBuffer         = virtual_OmxComponent_AllocateBuffer;
    OmxComponentVtableInstance.FreeBuffer             = virtual_OmxComponent_FreeBuffer;
    OmxComponentVtableInstance.EmptyThisBuffer        = virtual_OmxComponent_EmptyThisBuffer;
    OmxComponentVtableInstance.FillThisBuffer         = virtual_OmxComponent_FillThisBuffer;
    OmxComponentVtableInstance.SetCallbacks           = virtual_OmxComponent_SetCallbacks;
    OmxComponentVtableInstance.ComponentDeInit        = virtual_OmxComponent_ComponentDeInit;
    OmxComponentVtableInstance.UseEGLImage            = virtual_OmxComponent_UseEGLImage;
    OmxComponentVtableInstance.ComponentRoleEnum      = virtual_OmxComponent_ComponentRoleEnum;

    /*pure virtual functions and must be overrided*/
    OmxComponentVtableInstance.Start                  = NULL;
    OmxComponentVtableInstance.Stop                   = NULL;
    OmxComponentVtableInstance.Resume                 = NULL;
    OmxComponentVtableInstance.Pause                  = NULL;
    OmxComponentVtableInstance.Prepare                = NULL;
    OmxComponentVtableInstance.Preroll                = NULL;
    OmxComponentVtableInstance.AllResourcesReady      = NULL;
    OmxComponentVtableInstance.FreeResources          = NULL;
}

static void OmxComponent_constructor(OmxComponent thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(OmxComponent));
    chain_constructor(OmxComponent, thiz, params);

    thiz->mCallbacks = NULL;
    thiz->mAppData   = NULL;
    thiz->mState     = OMX_StateMax;
    
    thiz->Create              = OmxComponent_Create;
    thiz->setHandle           = OmxComponent_setHandle;
    thiz->postEvent           = OmxComponent_postEvent;
    thiz->postFillBufferDone  = OmxComponent_postFillBufferDone;
    thiz->postEmptyBufferDone = OmxComponent_postEmptyBufferDone;
    thiz->setState            = OmxComponent_setState;
    
    mc_ret = Mag_MsgChannelCreate(&(thiz->mCmdEvtChannel));
    if (MAG_ErrNone == mc_ret){
        Mag_MsgChannelReceiverAttach(thiz->mCmdEvtChannel, OmxComponent_CmdProcessEntry, thiz);
    }  
}

static void OmxComponent_destructor(OmxComponent thiz, TriangleVtable vtab){
    AGILE_LOGV("Enter!");
}


