#include "OMXComponent_port.h"

static OMX_ERRORTYPE OMXComponentPort_base_Destructor(OMXComponentPort_base_t *pPort){

}

static OMX_BOOL allPortsBufferPopulated(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *priv = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXComponentPort_base_t *portEntry = NULL;
    List_t *tmpNode = priv->portListHead.next;
    
    while (tmpNode != &priv->portListHead){
        portEntry = (OMXComponentPort_base_t *)list_entry(tmpNode, OMXComponentPort_base_t, node);
        if (!portEntry->sPortParam.bPopulated)
            return OMX_FALSE;

        tmpNode = tmpNode->next;
    }
    return OMX_TRUE;
}
static OMX_ERRORTYPE OMXComponentPort_base_AllocateBuffer(OMXComponentPort_base_t *pPort,
                                         OMX_BUFFERHEADERTYPE** ppBuffer, 
                                         OMX_PTR pAppPrivate,
                                         OMX_U32 nSizeBytes){
    OMX_U32 bufSize = 0;
    OMXComponentPort_Buffer_t *portBufNode = NULL;
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    int i;

    if (NULL == pPort){
        return OMX_ErrorBadParameter;
    }

    *ppBuffer = (OMX_BUFFERHEADERTYPE *)calloc(1, sizeof(OMX_BUFFERHEADERTYPE));
    if (NULL == *ppBuffer){
        AGILE_LOGE("failed to allocate memory");
        return OMX_ErrorInsufficientResources;
    }

    if (nSizeBytes < pPort->sPortParam.nBufferSize)
        bufSize = pPort->sPortParam.nBufferSize;
    else
        bufSize = nSizeBytes;

    for(i=0; i < pPort->sPortParam.nBufferCountActual; i++){
        (*ppBuffer)->pBuffer = (OMX_U8 *)calloc(1, bufSize);
        if (NULL == (*ppBuffer)->pBuffer){
            AGILE_LOGE("failed to allocate memory with the size %d", bufSize);
            goto OutOfMem;
        }

        OMXComponentSetHeader(*ppBuffer, sizeof(OMX_BUFFERHEADERTYPE));

        (*ppBuffer)->nAllocLen = bufSize;
        (*ppBuffer)->pAppPrivate = pAppPrivate;
        if (OMX_DirInput == pPort->sPortParam.eDir){
            (*ppBuffer)->pInputPortPrivate = pPort;
            (*ppBuffer)->nInputPortIndex = pPort->sPortParam.nPortIndex;
        }else{
            (*ppBuffer)->pOutputPortPrivate = pPort;
            (*ppBuffer)->nOutputPortIndex = pPort->sPortParam.nPortIndex;
        }

        portBufNode = (OMXComponentPort_Buffer_t *)calloc(1, sizeof(OMXComponentPort_Buffer_t));
        if (NULL == portBufNode){
            AGILE_LOGE("failed to allocate memory for OMXComponentPort_Buffer_t");
            goto OutOfMem;
        }

        INIT_LIST(&portBufNode->node);
        portBufNode->nBufSize = bufSize;
        portBufNode->pHeader = *ppBuffer;

        list_add(&portBufNode->node, &pPort->portBufListHead);

        pPort->sPortParam.numPortBufAlloc++;

        if (pPort->sPortParam.numPortBufAlloc >= pPort->sPortParam.nBufferCountActual){
            pPort->sPortParam.bPopulated = OMX_TRUE;
        }
    }

    if (pPort->pCompContainer){
        if (allPortsBufferPopulated(pPort->pCompContainer)){
            pPrivData_base = (OMXComponentPrivateBase_t *)pPort->pCompContainer->pComponentPrivate;
            if(pPrivData_base){
                Mag_SetEvent(pPrivData_base->Event_OMX_AllocateBufferDone);
            }else{
                AGILE_LOGE("the comp private data is NULL for port index %d", pPort->sPortParam.nPortIndex);
                return OMX_ErrorInvalidComponent;
            }
        }
    }else{
        AGILE_LOGE("the comp container is NULL for port index %d", pPort->sPortParam.nPortIndex);
        return OMX_ErrorInvalidComponent;
    }
    return OMX_ErrorNone;
    
OutOfMem:
    if (*ppBuffer)
        free(*ppBuffer);

    *ppBuffer = NULL;
    return OMX_ErrorInsufficientResources;
}

static OMX_ERRORTYPE OMXComponentPort_base_UseBuffer(OMXComponentPort_base_t *pPort,
                                    OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                    OMX_PTR pAppPrivate,
                                    OMX_U32 nSizeBytes,
                                    OMX_U8* pBuffer){
    OMXComponentPort_Buffer_t *portBufNode = NULL;
    OMXComponentPrivateBase_t *pPrivData_base = NULL;
    int i;

    if ((NULL == pPort) || (NULL == pBuffer)){
        return OMX_ErrorBadParameter;
    }

    *ppBufferHdr = (OMX_BUFFERHEADERTYPE *)calloc(1, sizeof(OMX_BUFFERHEADERTYPE));
    if (NULL == *ppBufferHdr){
        AGILE_LOGE("failed to allocate memory");
        return OMX_ErrorInsufficientResources;
    }

    (*ppBufferHdr)->pBuffer = pBuffer;
    OMXComponentSetHeader(*ppBufferHdr, sizeof(OMX_BUFFERHEADERTYPE));

    (*ppBufferHdr)->nAllocLen = nSizeBytes;
    (*ppBufferHdr)->pAppPrivate = pAppPrivate;
    if (OMX_DirInput == pPort->sPortParam.eDir){
        (*ppBufferHdr)->pInputPortPrivate = pPort;
        (*ppBufferHdr)->nInputPortIndex = pPort->sPortParam.nPortIndex;
    }else{
        (*ppBufferHdr)->pOutputPortPrivate = pPort;
        (*ppBufferHdr)->nOutputPortIndex = pPort->sPortParam.nPortIndex;
    }

    portBufNode = (OMXComponentPort_Buffer_t *)calloc(1, sizeof(OMXComponentPort_Buffer_t));
    if (NULL == portBufNode){
        AGILE_LOGE("failed to allocate memory for OMXComponentPort_Buffer_t");
        goto OutOfMem;
    }

    INIT_LIST(&portBufNode->node);
    portBufNode->nBufSize = nSizeBytes;
    portBufNode->pHeader = *ppBufferHdr;

    list_add(&portBufNode->node, &pPort->portBufListHead);

    pPort->sPortParam.numPortBufAlloc++;

    if (pPort->sPortParam.numPortBufAlloc >= pPort->sPortParam.nBufferCountActual){
        pPort->sPortParam.bPopulated = OMX_TRUE;
    }

    if (pPort->pCompContainer){
        if (allPortsBufferPopulated(pPort->pCompContainer)){
            pPrivData_base = (OMXComponentPrivateBase_t *)pPort->pCompContainer->pComponentPrivate;
            if(pPrivData_base){
                Mag_SetEvent(pPrivData_base->Event_OMX_UseBufferDone);
            }else{
                AGILE_LOGE("the comp private data is NULL for port index %d", pPort->sPortParam.nPortIndex);
                return OMX_ErrorInvalidComponent;
            }
        }
    }else{
        AGILE_LOGE("the comp container is NULL for port index %d", pPort->sPortParam.nPortIndex);
        return OMX_ErrorInvalidComponent;
    }
    
    return OMX_ErrorNone;
    
OutOfMem:
    if (*ppBufferHdr)
        free(*ppBufferHdr);

    *ppBufferHdr = NULL;
    return OMX_ErrorInsufficientResources;
}

static OMX_ERRORTYPE OMXComponentPort_base_FreeBuffer(OMXComponentPort_base_t *pPort,
                                     OMX_U32 nPortIndex,
                                     OMX_BUFFERHEADERTYPE* pBuffer){

}

/*void OMXComponentPort_base_BufferManager(void *msg, void *priv){
    AGILE_LOGE("Should be overrided by sub port method. Wrong be there");
}*/

OMX_ERRORTYPE OMXComponentPort_base_Constructor(OMX_IN OMX_COMPONENTTYPE *pCompContainer,
                                                              OMX_IN OMX_U32 nPortIndex,
                                                              OMX_IN OMX_BOOL isInput,
                                                              OMX_OUT OMXComponentPort_base_t **ppCompPort){
    OMXComponentPrivateBase_t *pPriv = NULL;
    
    if (NULL == pCompContainer){
        return OMX_ErrorBadParameter;
    }

    if(NULL == *ppCompPort){
        *ppCompPort = (OMXComponentPort_base_t *)calloc(1, sizeof(OMXComponentPort_base_t));
        if (NULL == *ppCompPort){
            AGILE_LOGE("failed to allocate memory for OMXComponentPort_base_t");
            return OMX_ErrorInsufficientResources;
        }
    }

    if (MAG_ErrNone != Mag_MsgChannelCreate(&(*ppCompPort)->bufferMgrHandle)){
        AGILE_LOGE("failed to create MsgChannel!");
        return OMX_ErrorInsufficientResources;
    }
    
    (*ppCompPort)->pCompContainer = pCompContainer;
    INIT_LIST(&(*ppCompPort)->portBufListHead);
    
    OMXComponentSetHeader(&(*ppCompPort)->sPortParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    (*ppCompPort)->sPortParam.nPortIndex = nPortIndex;
    (*ppCompPort)->sPortParam.eDir = (isInput == OMX_TRUE) ? OMX_DirInput : OMX_DirOutput;
    (*ppCompPort)->sPortParam.nBufferCountActual = DEFAULT_BUFFER_NUM_PER_PORT;
    (*ppCompPort)->sPortParam.nBufferCountMin = DEFAULT_MIN_BUFFER_NUM_PER_PORT;
    (*ppCompPort)->sPortParam.nBufferSize = DEFAULT_BASE_PORT_BUF_SIZE;
    (*ppCompPort)->sPortParam.bEnabled = OMX_TRUE;
    (*ppCompPort)->sPortParam.bPopulated = OMX_FALSE;

    (*ppCompPort)->PortDestructor = OMXComponentPort_base_Destructor;
    (*ppCompPort)->Port_AllocateBuffer = OMXComponentPort_base_AllocateBuffer;
    (*ppCompPort)->Port_UseBuffer = OMXComponentPort_base_UseBuffer;
    (*ppCompPort)->Port_FreeBuffer = OMXComponentPort_base_FreeBuffer;
    
    pPriv = (OMXComponentPrivateBase_t *)pCompContainer->pComponentPrivate;
    list_add_tail(&(*ppCompPort)->node, &pPriv->portListHead);
    
    return OMX_ErrorNone;
}


/*******************************************/

#include "OMXPort_base.h"

AllocateClass(OmxPort, Base);


OMX_ERRORTYPE virtual_OmxPort_enablePort(OmxPort hPort){

}

OMX_ERRORTYPE virtual_OmxPort_disablePort(OmxPort hPort){

}

OMX_ERRORTYPE virtual_OmxPort_flushPort(OmxPort hPort){

}

OMX_ERRORTYPE virtual_OmxPort_markBuffer(OmxPort hPort, OMX_MARKTYPE * mark){

}

OMX_ERRORTYPE virtual_OmxPort_AllocateBuffer(
                  OmxPort hPort,
                  OMX_BUFFERHEADERTYPE** ppBuffer,
                  OMX_PTR pAppPrivate,
                  OMX_U32 nSizeBytes){

}

OMX_ERRORTYPE virtual_OmxPort_FreeBuffer(
                  OmxPort hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer){

}

OMX_PARAM_PORTDEFINITIONTYPE *OmxPort_getPortDefinition(OmxPort hPort){

}

OMX_ERRORTYPE OmxPort_setPortDefinition(OmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){

}


/*Class Constructor/Destructor*/
static void OmxPort_initialize(Class this){
    OmxPortVtableInstance.enablePort      = virtual_OmxPort_enablePort;
    OmxPortVtableInstance.disablePort     = virtual_OmxPort_disablePort;
    OmxPortVtableInstance.flushPort       = virtual_OmxPort_flushPort;
    OmxPortVtableInstance.markBuffer      = virtual_OmxPort_markBuffer;
    OmxPortVtableInstance.AllocateBuffer  = virtual_OmxPort_AllocateBuffer;
    OmxPortVtableInstance.FreeBuffer      = virtual_OmxPort_FreeBuffer;
}

static void OmxPort_constructor(OmxComponent thiz, const void *params){
    thiz->getPortDefinition = OmxPort_getPortDefinition;
    thiz->setPortDefinition = OmxPort_setPortDefinition;

    memset(&thiz->mPortDef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    thiz->mPortDef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    thiz->mPortDef.nVersion.nVersion
}

