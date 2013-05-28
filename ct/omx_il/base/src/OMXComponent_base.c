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


