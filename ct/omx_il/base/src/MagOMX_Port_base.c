#include "OMXPort_base.h"

AllocateClass(MagOmxPort, Base);


OMX_ERRORTYPE virtual_MagOmxPort_enablePort(MagOmxPort hPort){

}

OMX_ERRORTYPE virtual_MagOmxPort_disablePort(MagOmxPort hPort){

}

OMX_ERRORTYPE virtual_MagOmxPort_flushPort(MagOmxPort hPort){

}

OMX_ERRORTYPE virtual_MagOmxPort_markBuffer(MagOmxPort hPort, OMX_MARKTYPE * mark){

}

OMX_ERRORTYPE virtual_MagOmxPort_AllocateBuffer(
                  MagOmxPort hPort,
                  OMX_BUFFERHEADERTYPE** ppBufferHdr,
                  OMX_PTR pAppPrivate,
                  OMX_U32 nSizeBytes){
    OMX_BUFFERHEADERTYPE *pBuf          = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));
    OMXComponentPort_Buffer_t *pBufNode = (OMXComponentPort_Buffer_t *)mag_mallocz(sizeof(OMXComponentPort_Buffer_t));
    OMX_U8 *pMem = NULL;
    
    if((pBuf == NULL) ||
       (pBufNode == NULL)){
       AGILE_LOGE("Failed to allocate buffers: pBuf[0x%x], pBufNode[0x%x]", pBuf, pBufNode);
       if (pBuf)
           mag_freep(pBuf);
       if (pBufNode)
           mag_freep(pBufNode);
       return OMX_ErrorInsufficientResources;
    }
    
    Mag_AcquireMutex(hPort->mhMutex);
    
    if (NULL == hPort->mBufferPool)
        hPort->mBufferPool = magMemPoolCreate(hPort->mPortDef.nBufferSize * hPort->mPortDef.nBufferCountActual);

    pMem = (OMX_U8 *)magMemPoolGetBuffer(hPort->mBufferPool, nSizeBytes);

    if (pMem == NULL){
        AGILE_LOGE("Failed to allocate memory(%d bytes)", nSizeBytes);
        Mag_ReleaseMutex(hPort->mhMutex);
        return OMX_ErrorInsufficientResources;
    }
    
    MagOmx_Common_InitHeader((OMX_U8 *)pBuf, sizeof(OMX_BUFFERHEADERTYPE));
    pBuf->pBuffer            = pMem;
    pBuf->nAllocLen          = nSizeBytes;
    pBuf->nFilledLen         = 0;
    pBuf->nOffset            = 0;
    pBuf->pAppPrivate        = pAppPrivate;
    pBuf->pPlatformPrivate   = NULL;
    pBuf->pInputPortPrivate  = hPort->mPortDef.eDir == OMX_DirInput  ? hPort : NULL;
    pBuf->pOutputPortPrivate = hPort->mPortDef.eDir == OMX_DirOutput ? hPort : NULL;
    pBuf->nInputPortIndex    = hPort->mPortDef.eDir == OMX_DirInput  ? hPort->mPortDef.nPortIndex : 0;
    pBuf->nOutputPortIndex   = hPort->mPortDef.eDir == OMX_DirOutput ? hPort->mPortDef.nPortIndex : 0;
    
    pBufNode->isAllocator  = OMX_TRUE;
    pBufNode->pHeader      = pBuf;
    INIT_LIST(&pBufNode->node);
    
    list_add_tail(&pBufNode->node, &hPort->mBufListHeader);
    hPort->mNumAssignedBuffers++

    if (hPort->mNumAssignedBuffers == hPort->mPortDef.nBufferCountActual)
        hPort->mPortDef.bPopulated = OMX_TRUE;
    
    *ppBufferHdr = pBuf;
    
    Mag_ReleaseMutex(hPort->mhMutex);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE virtual_MagOmxPort_UseBuffer(
                   MagOmxPort hPort,
                   OMX_BUFFERHEADERTYPE **ppBufferHdr,
                   OMX_PTR pAppPrivate,
                   OMX_U32 nSizeBytes,
                   OMX_U8 *pBuffer){
    OMX_BUFFERHEADERTYPE *pBuf          = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));
    OMXComponentPort_Buffer_t *pBufNode = (OMXComponentPort_Buffer_t *)mag_mallocz(sizeof(OMXComponentPort_Buffer_t));
    
    if((pBuf == NULL) ||
       (pBufNode == NULL)){
        return OMX_ErrorInsufficientResources;
    }
    Mag_AcquireMutex(hPort->mhMutex);
    
    MagOmx_Common_InitHeader((OMX_U8 *)pBuf, sizeof(OMX_BUFFERHEADERTYPE));
    pBuf->pBuffer            = pBuffer;
    pBuf->nAllocLen          = nSizeBytes;
    pBuf->nFilledLen         = 0;
    pBuf->nOffset            = 0;
    pBuf->pAppPrivate        = pAppPrivate;
    pBuf->pPlatformPrivate   = NULL;
    pBuf->pInputPortPrivate  = hPort->mPortDef.eDir == OMX_DirInput  ? hPort : NULL;
    pBuf->pOutputPortPrivate = hPort->mPortDef.eDir == OMX_DirOutput ? hPort : NULL;
    pBuf->nInputPortIndex    = hPort->mPortDef.eDir == OMX_DirInput  ? hPort->mPortDef.nPortIndex : 0;
    pBuf->nOutputPortIndex   = hPort->mPortDef.eDir == OMX_DirOutput ? hPort->mPortDef.nPortIndex : 0;
    
    pBufNode->isAllocator  = OMX_FALSE;
    pBufNode->pHeader      = pBuf;
    INIT_LIST(&pBufNode->node);
    
    list_add_tail(&pBufNode->node, &hPort->mBufListHeader);
    hPort->mNumAssignedBuffers++

    if (hPort->mNumAssignedBuffers == hPort->mPortDef.nBufferCountActual)
        hPort->mPortDef.bPopulated = OMX_TRUE;
    
    *ppBufferHdr = pBuf;
    
    Mag_ReleaseMutex(hPort->mhMutex);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE virtual_MagOmxPort_FreeBuffer(
                  MagOmxPort hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer){
    List_t *tmpNode = hPort->mBufListHeader.next;
    OMXComponentPort_Buffer_t  *pBufNode;
    
    while (tmpNode != &hPort->mBufListHeader){
        pBufNode = (OMXComponentPort_Buffer_t *)list_entry(tmpNode, OMXComponentPort_Buffer_t, node);
        if (pBufNode->pHeader == pBuffer){
            if (pBufNode->isAllocator)
                magMemPoolPutBuffer(hPort->mBufferPool, pBuffer->pBuffer);

            mag_freep(pBuffer);
            list_del(&pBufNode->node);
            mag_freep(pBufNode);

            return OMX_ErrorNone;
        }

        tmpNode = tmpNode->next;
    }    
    
    return OMX_ErrorBadParameter;
}

void MagOmxPort_getPortDefinition(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *getDef){
    Mag_AcquireMutex(hPort->mhMutex);
    memcpy(getDef, &hPort->mPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    Mag_ReleaseMutex(hPort->mhMutex);
}

void MagOmxPort_setPortDefinition(MagOmxPort hPort, OMX_PARAM_PORTDEFINITIONTYPE *setDef){
    Mag_AcquireMutex(hPort->mhMutex);
    memcpy(&hPort->mPortDef, setDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    Mag_ReleaseMutex(hPort->mhMutex);
}


/*Class Constructor/Destructor*/
static void MagOmxPort_initialize(Class this){
    MagOmxPortVtableInstance.enablePort      = virtual_MagOmxPort_enablePort;
    MagOmxPortVtableInstance.disablePort     = virtual_MagOmxPort_disablePort;
    MagOmxPortVtableInstance.flushPort       = virtual_MagOmxPort_flushPort;
    MagOmxPortVtableInstance.markBuffer      = virtual_MagOmxPort_markBuffer;
    MagOmxPortVtableInstance.AllocateBuffer  = virtual_MagOmxPort_AllocateBuffer;
    MagOmxPortVtableInstance.UseBuffer       = virtual_MagOmxPort_UseBuffer;
    MagOmxPortVtableInstance.FreeBuffer      = virtual_MagOmxPort_FreeBuffer;
}

static void MagOmxPort_constructor(MagOmxPort thiz, const void *params){
    thiz->getPortDefinition = MagOmxPort_getPortDefinition;
    thiz->setPortDefinition = MagOmxPort_setPortDefinition;

    memset(&thiz->mPortDef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    MagOmx_Common_InitHeader((OMX_U8 *)&thiz->mPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    Mag_CreateMutex(&thiz->mhMutex);
    thiz->mNumAssignedBuffers = 0;
    
    thiz->mPortDef.nBufferCountActual = OMX_PORT_MIN_BUFFER_NUM;
    thiz->mPortDef.nBufferCountMin    = OMX_PORT_MIN_BUFFER_NUM;
    thiz->mPortDef.nBufferSize        = OMX_PORT_BUFFER_SIZE;
    thiz->mPortDef.bEnabled           = OMX_TRUE;
    thiz->mPortDef.bPopulated         = OMX_FALSE;
    thiz->mPortDef.bBuffersContiguous = OMX_FALSE;
    thiz->mPortDef.nBufferAlignment   = 4;

    thiz->mPortDef.nPortIndex         = *((OMX_U32 *)params + 0);
    thiz->mPortDef.eDir               = *((OMX_U32 *)params + 1) == OMX_FALSE ? OMX_DirOutput : OMX_DirInput;
    
    INIT_LIST(&thiz->mBufListHeader);

    thiz->mBufferPool = magMemPoolCreate(thiz->mPortDef.nBufferSize * thiz->mPortDef.nBufferCountActual);
}

MagOmxPort MagOmxPort_Create(OMX_U32 nPortIndex, OMX_BOOL isInput){
    OMX_U32 param[2];

    param[0] = nPortIndex;
    param[1] = isInput;

    return (MagOmxPort) ooc_new( MagOmxPort, (void *)param);
}


