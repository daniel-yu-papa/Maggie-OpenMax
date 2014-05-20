#include "OMXPort_base.h"

AllocateClass(MagOmxPortBase, MagOmxPort);

static MagOmxPortBase getBase(OMX_HANDLETYPE hPort){
    MagOmxPortBase base;

    base = ooc_cast(hPort, MagOmxPortBase);
    return base;
}

static MagOmxPort getRoot(OMX_HANDLETYPE hPort){
    MagOmxPort root;

    root = ooc_cast(hPort, MagOmxPort);
    return root;
}

static OMX_ERRORTYPE allocateBufferInternal(
                                              OMX_HANDLETYPE hPort,
                                              OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                              OMX_PTR pAppPrivate,
                                              OMX_U32 nSizeBytes){

    OMX_BUFFERHEADERTYPE *pPortBuf;
    OMX_U8 *pBuffer;
    MagOmxPortBase base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;
    MagOMX_Port_Buffer_t *pBufNode;
    
    if (NULL == hPort)
        return OMX_ErrorBadParameter;

    if (root->getDef_Populated(root)){
        AGILE_LOGE("the port buffers have been populated! To quit the allocation.");
        return OMX_ErrorNoMore;
    }
    
    pPortBuf = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));

    if (NULL != pPortBuf){
        base = getBase(hPort);
        root = getRoot(hPort);
        
        if (MagOmxPortBaseVirtual(base)->MagOMX_AllocateBuffer){
            ret = MagOmxPortBaseVirtual(base)->MagOMX_AllocateBuffer(&pBuffer, nSizeBytes);

            if (ret != OMX_ErrorNone){
                AGILE_LOGE("failed to do MagOMX_AllocateBuffer(%d bytes)", nSizeBytes);
                return ret;
            }
        }else{
            AGILE_LOGE("pure virtual MagOMX_AllocateBuffer() must be overrided!");
            return OMX_ErrorNotImplemented;
        }

        initHeader(pPortBuf, sizeof(OMX_BUFFERHEADERTYPE));
        pPortBuf->pBuffer            = pBuffer;
        pPortBuf->nAllocLen          = nSizeBytes;
        pPortBuf->nFilledLen         = 0;
        pPortBuf->nOffset            = 0;
        pPortBuf->pAppPrivate        = pAppPrivate;
        //pPortBuf->pPlatformPrivate   = NULL;
        pPortBuf->pInputPortPrivate  = root->isInputPort(root) ? hPort : NULL;
        pPortBuf->pOutputPortPrivate = root->isInputPort(root) ? NULL : hPort;
        pPortBuf->nInputPortIndex    = root->isInputPort(root) ? root->getPortIndex(root) : kInvalidPortIndex;
        pPortBuf->nOutputPortIndex   = root->isInputPort(root) ? kInvalidPortIndex : root->getPortIndex(root);
        pPortBuf->nFlags             |= OMX_BUFFERFLAG_EXT_INFREELIST;
        
        pBufNode = (MagOMX_Port_Buffer_t *)mag_mallocz(sizeof(MagOMX_Port_Buffer_t));
        INIT_LIST(&pBufNode->node);
        pBufNode->omx_buffer = pPortBuf;
        list_add_tail(&pBufNode->node, &base->mFreeBufListHeader);

        pPortBuf->pPlatformPrivate   = (OMX_PTR)pBufNode;
    }else{
        *ppBufferHdr = NULL;
        AGILE_LOGE("failed to allocate OMX_BUFFERHEADERTYPE!");
        return OMX_ErrorInsufficientResources;
    }
    
    *ppBufferHdr = pPortBuf;
    if (++base->mBuffersTotal == root->getDef_BufferCountActual(root)){
        AGILE_LOGD("the port(0x%p) populates, buffer number is %d!", hPort, base->mBuffersTotal);
        root->setDef_Populated(root, OMX_TRUE);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE useBufferInternal(   
                                              OMX_HANDLETYPE hPort,
                                              OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                              OMX_PTR pAppPrivate,
                                              OMX_U32 nSizeBytes,
                                              OMX_U8 *pBuffer){

    OMX_BUFFERHEADERTYPE *pPortBuf;
    OMX_U8 *pBuffer;
    MagOmxPortBase base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;
    MagOMX_Port_Buffer_t *pBufNode;
    
    if ((NULL == hPort) || (NULL == pBuffer))
        return OMX_ErrorBadParameter;

    if (root->getDef_Populated(root)){
        AGILE_LOGE("the port buffers have been populated! ignore the allocation.");
        return OMX_ErrorNoMore;
    }
    
    pPortBuf = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));

    if (NULL != pPortBuf){
        base = getBase(hPort);
        root = getRoot(hPort);

        initHeader(pPortBuf, sizeof(OMX_BUFFERHEADERTYPE));
        pPortBuf->pBuffer            = pBuffer;
        pPortBuf->nAllocLen          = nSizeBytes;
        pPortBuf->nFilledLen         = 0;
        pPortBuf->nOffset            = 0;
        pPortBuf->pAppPrivate        = pAppPrivate;
        //pPortBuf->pPlatformPrivate   = NULL;
        pPortBuf->pInputPortPrivate  = root->isInputPort(root) ? hPort : NULL;
        pPortBuf->pOutputPortPrivate = root->isInputPort(root) ? NULL : hPort;
        pPortBuf->nInputPortIndex    = root->isInputPort(root) ? root->getPortIndex(root) : kInvalidPortIndex;
        pPortBuf->nOutputPortIndex   = root->isInputPort(root) ? kInvalidPortIndex : root->getPortIndex(root);
        pPortBuf->nFlags             |= OMX_BUFFERFLAG_EXT_INFREELIST;
        
        pBufNode = (MagOMX_Port_Buffer_t *)mag_mallocz(sizeof(MagOMX_Port_Buffer_t));
        INIT_LIST(&pBufNode->node);
        pBufNode->omx_buffer = pPortBuf;
        list_add_tail(&pBufNode->node, &base->mFreeBufListHeader);
        
        pPortBuf->pPlatformPrivate   = (OMX_PTR)pBufNode;
    }else{
        *ppBufferHdr = NULL;
        AGILE_LOGE("failed to allocate OMX_BUFFERHEADERTYPE!");
        return OMX_ErrorInsufficientResources;
    }
    
    *ppBufferHdr = pPortBuf;
    if (++base->mBuffersTotal == root->getDef_BufferCountActual(root)){
        AGILE_LOGD("the port(0x%p) populates, buffer number is %d!", hPort, base->mBuffersTotal);
        root->setDef_Populated(root, OMX_TRUE);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE freeBufferInternal(
                                           OMX_HANDLETYPE hPort,
                                           OMX_BUFFERHEADERTYPE* pBufferHdr){

    OMX_U8 *pBuffer;
    MagOmxPortBase base;
    OMX_ERRORTYPE ret;
    
    if ((NULL == hPort) || (NULL == pBufferHdr))
        return OMX_ErrorBadParameter;

    base = getBase(hPort);

    if (pBufferHdr->nFlags & OMX_BUFFERFLAG_EXT_INBUSYLIST){
        ret = MagOmxPortBaseVirtual(base)->MagOMX_FreeBuffer(pBufferHdr->pBuffer);
        if (ret != OMX_ErrorNone){
            AGILE_LOGE("failed to free the pBuffer: 0x%p", pBufferHdr->pBuffer);
            return ret;
        }
        pBufferHdr->nFlags &= ~OMX_BUFFERFLAG_EXT_INBUSYLIST;
        pBufferHdr->nFlags |= OMX_BUFFERFLAG_EXT_INFREELIST;
    }
    return OMX_ErrorNone;
}
/*
 * Virtual Functions Implementation
 */
static OMX_ERRORTYPE virtual_enablePort(OMX_HANDLETYPE hPort, OMX_PTR AppData){
    MagOmxPortBase base;
    MagOmxPort     root;
    OMX_BUFFERHEADERTYPE *pBufferHdr;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    OMX_ERRORTYPE ret;
    OMX_S32 i;
    
    base = getBase(hPort);
    root = getRoot(hPort);
    
    if (!root->getDef_Enabled(root)){
        if (root->getDef_Populated(root)){
            AGILE_LOGD("the port(0x%p) has been enabled and populated!", hPort);
            return OMX_ErrorNone;
        }else{
            root->getPortDefinition(hPort, &portDef);
            for (i = 0; i < base->getDef_BufferCountActual(root); i++){
                ret = MagOmxPortBaseVirtual(base)->AllocateBuffer(hPort, &pBufferHdr, AppData, portDef.nBufferSize);

                if ((ret != OMX_ErrorNone) && (ret != OMX_ErrorNoMore)){
                    return ret;
                }
            }
        }
    }
    
    
    
    MagOmxPortBaseVirtual(base)->setDef_Enabled(hPort, OMX_TRUE);
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_disablePort(OMX_HANDLETYPE hPort){

}

static OMX_ERRORTYPE virtual_flushPort(OMX_HANDLETYPE hPort){

}

static OMX_ERRORTYPE virtual_markBuffer(OMX_HANDLETYPE hPort, OMX_MARKTYPE * mark){

}

/*request the component to allocate a new buffer and buffer header*/
static OMX_ERRORTYPE virtual_AllocateBuffer(
                                              OMX_HANDLETYPE hPort,
                                              OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                              OMX_PTR pAppPrivate,
                                              OMX_U32 nSizeBytes){

    OMX_ERRORTYPE ret;
    
    Mag_AcquireMutex(getBase(hPort)->mhMutex);
    ret = allocateBufferInternal(hPort, ppBufferHdr, pAppPrivate, nSizeBytes);
    Mag_ReleaseMutex(getBase(hPort)->mhMutex);
    return ret;
}

static OMX_ERRORTYPE virtual_UseBuffer(
                   OMX_HANDLETYPE hPort,
                   OMX_BUFFERHEADERTYPE **ppBufferHdr,
                   OMX_PTR pAppPrivate,
                   OMX_U32 nSizeBytes,
                   OMX_U8 *pBuffer){

    OMX_ERRORTYPE ret;
    
    Mag_AcquireMutex(getBase(hPort)->mhMutex);
    ret = useBufferInternal(hPort, ppBufferHdr, pAppPrivate, nSizeBytes, pBuffer);
    Mag_ReleaseMutex(getBase(hPort)->mhMutex);
    return ret;
}

static OMX_ERRORTYPE virtual_FreeBuffer(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer){

    OMX_ERRORTYPE ret;
    
    Mag_AcquireMutex(getBase(hPort)->mhMutex);

    Mag_ReleaseMutex(getBase(hPort)->mhMutex);
}


/*Class Constructor/Destructor*/
static void MagOmxPortBase_initialize(Class this){
    MagOmxPortBaseVtableInstance.MagOmxPort.enablePort      = virtual_enablePort;
    MagOmxPortBaseVtableInstance.MagOmxPort.disablePort     = virtual_disablePort;
    MagOmxPortBaseVtableInstance.MagOmxPort.flushPort       = virtual_flushPort;
    MagOmxPortBaseVtableInstance.MagOmxPort.markBuffer      = virtual_markBuffer;
    MagOmxPortBaseVtableInstance.MagOmxPort.AllocateBuffer  = virtual_AllocateBuffer;
    MagOmxPortBaseVtableInstance.MagOmxPort.UseBuffer       = virtual_UseBuffer;
    MagOmxPortBaseVtableInstance.MagOmxPort.FreeBuffer      = virtual_FreeBuffer;
    MagOmxPortBaseVtableInstance.MagOmxPort.EmptyThisBuffer = virtual_EmptyThisBuffer;
    MagOmxPortBaseVtableInstance.MagOmxPort.FillThisBuffer  = virtual_FillThisBuffer;

    MagOmxPortBaseVtableInstance.MagOMX_AllocateBuffer      = NULL;
    MagOmxPortBaseVtableInstance.MagOMX_FreeBuffer          = NULL;
}

static void MagOmxPortBase_constructor(MagOmxPortBase thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(MagOmxPortBase));
    chain_constructor(MagOmxPortBase, thiz, params);
    
    Mag_CreateMutex(&thiz->mhMutex);
    INIT_LIST(&thiz->mFreeBufListHeader);
    INIT_LIST(&thiz->mBusyBufListHeader);

    thiz->mBuffersTotal = 0;
}

static void MagOmxPortBase_destructor(MagOmxPortBase thiz, MagOmxPortBaseVtable vtab){
    AGILE_LOGV("Enter!");
}


