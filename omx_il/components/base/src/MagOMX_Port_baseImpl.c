#include "OMXPort_base.h"

AllocateClass(MagOmxPortImpl, MagOmxPort);

static MagOmxPortImpl getBase(OMX_HANDLETYPE hPort){
    MagOmxPortImpl base;

    base = ooc_cast(hPort, MagOmxPortImpl);
    return base;
}

static MagOmxPort getRoot(OMX_HANDLETYPE hPort){
    MagOmxPort root;

    root = ooc_cast(hPort, MagOmxPort);
    return root;
}

static OMX_ERRORTYPE FreeTunnelBufferInternal(MagOmxPort supplierPort, MagOmxPort noSupplierPort){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    OMX_BUFFERHEADERTYPE *pBufferHeader;
    MagOmxPortImpl supplierPortImpl   = ooc_cast(MagOmxPortImpl, supplierPort);
    MagOmxPortImpl noSupplierPortImpl = ooc_cast(MagOmxPortImpl, noSupplierPort);

    Mag_AcquireMutex(supplierPortImpl->mhMutex);
    pNode = supplierPortImpl->mBufferList.next;
    while (pNode != &supplierPortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        pBufferHeader = item->pOmxBufferHeader;
        MagOmxPortVirtual(supplierPort)->FreeBuffer(supplierPort, pBufferHeader);
        pNode = supplierPortImpl->mBufferList.next;
    }
    Mag_ReleaseMutex(supplierPortImpl->mhMutex);

    Mag_AcquireMutex(noSupplierPortImpl->mhMutex);
    pNode = noSupplierPortImpl->mBufferList.next;
    while (pNode != &noSupplierPortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        pBufferHeader = item->pOmxBufferHeader;
        MagOmxPortVirtual(noSupplierPort)->FreeBuffer(noSupplierPort, pBufferHeader);
        pNode = noSupplierPortImpl->mBufferList.next;
    }
    Mag_ReleaseMutex(noSupplierPortImpl->mhMutex);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE allocateBufferInternal(
                      OMX_HANDLETYPE hPort,
                      OMX_BUFFERHEADERTYPE** ppBufferHdr,
                      OMX_PTR pAppPrivate,
                      OMX_U32 nSizeBytes){

    OMX_BUFFERHEADERTYPE *pPortBufHeader;
    OMX_U8 *pBuffer;
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE  ret;
    MagOMX_Port_Buffer_t *bufNode;
    
    if (NULL == hPort)
        return OMX_ErrorBadParameter;

    if (root->getDef_Populated(root)){
        AGILE_LOGE("the port buffers have been populated! To quit the allocation.");
        return OMX_ErrorNoMore;
    }
    
    base = getBase(hPort);
    root = getRoot(hPort);
    
    if (root->isTunneled(root)){
        AGILE_LOGE("Could not allocated the buffer in tunneled port!");
        return OMX_ErrorIncorrectStateOperation;
    }

    pPortBufHeader = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));
    MAG_ASSERT(pPortBufHeader);

    if (MagOmxPortImplVirtual(base)->MagOMX_AllocateBuffer){
        ret = MagOmxPortImplVirtual(base)->MagOMX_AllocateBuffer(base, &pBuffer, nSizeBytes);

        if (ret != OMX_ErrorNone){
            AGILE_LOGE("failed to do MagOMX_AllocateBuffer(%d bytes)", nSizeBytes);
            return ret;
        }
    }else{
        AGILE_LOGE("pure virtual MagOMX_AllocateBuffer() must be overrided!");
        return OMX_ErrorNotImplemented;
    }

    initHeader(pPortBufHeader, sizeof(OMX_BUFFERHEADERTYPE));
    pPortBufHeader->pBuffer            = pBuffer;
    pPortBufHeader->nAllocLen          = nSizeBytes;
    pPortBufHeader->nFilledLen         = 0;
    pPortBufHeader->nOffset            = 0;
    pPortBufHeader->pAppPrivate        = pAppPrivate;
    pPortBufHeader->pInputPortPrivate  = root->isInputPort(root) ? hPort : NULL;
    pPortBufHeader->pOutputPortPrivate = root->isInputPort(root) ? NULL : hPort;
    pPortBufHeader->nInputPortIndex    = root->isInputPort(root) ? root->getPortIndex(root) : kInvalidPortIndex;
    pPortBufHeader->nOutputPortIndex   = root->isInputPort(root) ? kInvalidPortIndex : root->getPortIndex(root);
    
    bufNode = base->allocBufferNode(pPortBufHeader);
    bufNode->bufferOwner       = MagOmxPortImpl_OwnedByThisPort;
    bufNode->bufferHeaderOwner = MagOmxPortImpl_OwnedByThisPort;
    list_add_tail(&bufNode->node, &base->mBufferList);

    *ppBufferHdr = pPortBufHeader;
    if (++base->mBuffersTotal == root->getDef_BufferCountActual(root)){
        base->mFreeBuffersNum = base->mBuffersTotal;
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

    OMX_BUFFERHEADERTYPE *pPortBufHeader;
    OMX_U8 *pBuffer;
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;
    MagOMX_Port_Buffer_t *bufNode;

    if (NULL == hPort)
        return OMX_ErrorBadParameter;

    if (root->getDef_Populated(root)){
        AGILE_LOGE("the port buffers have been populated! ignore the allocation.");
        return OMX_ErrorNoMore;
    }
    
    pPortBufHeader = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));
    MAG_ASSERT(pPortBufHeader);

    base = getBase(hPort);
    root = getRoot(hPort);

    initHeader(pPortBufHeader, sizeof(OMX_BUFFERHEADERTYPE));
    if (pBuffer != NULL){
        pPortBufHeader->pBuffer        = pBuffer;
        pPortBufHeader->nAllocLen      = nSizeBytes;
    }else{
        pPortBufHeader->pBuffer        = NULL
        pPortBufHeader->nAllocLen      = 0;
    }
    pPortBufHeader->nFilledLen         = 0;
    pPortBufHeader->nOffset            = 0;
    pPortBufHeader->pAppPrivate        = pAppPrivate;
    pPortBufHeader->pInputPortPrivate  = root->isInputPort(root) ? hPort : NULL;
    pPortBufHeader->pOutputPortPrivate = root->isInputPort(root) ? NULL : hPort;
    pPortBufHeader->nInputPortIndex    = root->isInputPort(root) ? root->getPortIndex(root) : kInvalidPortIndex;
    pPortBufHeader->nOutputPortIndex   = root->isInputPort(root) ? kInvalidPortIndex : root->getPortIndex(root);
    
    bufNode = base->allocBufferNode(pPortBufHeader);
    if (root->isTunneled(root))
        bufNode->bufferHeaderOwner = MagOmxPortImpl_OwnedByTunneledPort;
    else
        bufNode->bufferHeaderOwner = MagOmxPortImpl_OwnedByThisPort;
    list_add_tail(&bufNode->node, &base->mBufferList);

    *ppBufferHdr = pPortBufHeader;
    if (++base->mBuffersTotal == root->getDef_BufferCountActual(root)){
        base->mFreeBuffersNum = base->mBuffersTotal;
        AGILE_LOGD("the port(0x%p) populates, buffer number is %d!", hPort, base->mBuffersTotal);
        root->setDef_Populated(root, OMX_TRUE);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE freeBufferInternal(
                       OMX_HANDLETYPE hPort,
                       OMX_BUFFERHEADERTYPE* pBufferHdr){

    OMX_U8 *pBuffer;
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;
    MagOMX_Port_Buffer_t *pBufNode;

    if ((NULL == hPort) || (NULL == pBufferHdr))
        return OMX_ErrorBadParameter;

    base = getBase(hPort);
    root = getRoot(hPort);

    pBufNode = (MagOMX_Port_Buffer_t *)pBufferHdr->pPlatformPrivate;

    if (pBufNode->bufferOwner == MagOmxPortImpl_OwnedByThisPort){
        ret = MagOmxPortImplVirtual(base)->MagOMX_FreeBuffer(base, pBufferHdr->pBuffer);
        if (ret != OMX_ErrorNone){
            AGILE_LOGE("failed to free the pBuffer: 0x%p", pBufferHdr->pBuffer);
            return ret;
        }
    }

    if ((pBufNode->bufferHeaderOwner == MagOmxPortImpl_OwnedByThisPort) ||
        (pBufNode->bufferHeaderOwner == MagOmxPortImpl_OwnedByTunneledPort &&
         !root->isBufferSupplier(root))){
        mag_free(pBufferHdr);
    }

    list_del(&pBufNode->node);
    mag_free(pBufNode);

    return OMX_ErrorNone;
}

static void onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxPort     root;
    MagOmxPortImpl base;
    OMX_BUFFERHEADERTYPE *bufHeader;
    OMX_ERRORTYPE ret;
    OMX_U32 cmd;

    if (!msg){
        AGILE_LOGE("msg is NULL!");
        return;
    }
    
    root = getRoot(priv);
    base = getBase(priv);

    if (!msg->findPointer(msg, "buffer_header", &bufHeader)){
        AGILE_LOGE("failed to find the buffer_header!");
        return;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxPortImpl_EmptyThisBufferMsg:
            if (MagOmxPortImplVirtual(base)->MagOMX_EmptyThisBuffer){
                MagOmxPortImplVirtual(base)->MagOMX_EmptyThisBuffer(base, bufHeader);
            }else{
                AGILE_LOGE("pure virtual MagOMX_EmptyThisBuffer() must be overrided!");
            }

            if (root->isTunneled(root)){
                if (root->isInputPort(root) && root->isBufferSupplier(root)){
                    /*get buffer header returned*/
                    base->putRunningNode(base, bufHeader);
                    return;
                }
            }
            base->dispatchBuffers(base, bufHeader);
            break;

        case MagOmxPortImpl_FillThisBufferMsg:
            if (MagOmxPortImplVirtual(base)->MagOMX_FillThisBuffer){
                MagOmxPortImplVirtual(base)->MagOMX_FillThisBuffer(base, bufHeader);
            }else{
                AGILE_LOGE("pure virtual MagOMX_FillThisBuffer() must be overrided!");
            }

            if (root->isTunneled(root)){
                if (!root->isInputPort(root) && root->isBufferSupplier(root)){
                    /*get buffer header returned*/
                    base->putRunningNode(base, bufHeader);
                    return;
                }
            }
            base->dispatchBuffers(base, bufHeader);
            break;

        case MagOmxPortImpl_ReturnThisBufferMsg:
            if (root->isTunneled(root)){
                if (root->isBufferSupplier(root)){
                    base->putRunningNode(base, bufHeader);
                }else{
                    if (root->isInputPort(root)){
                        OMX_FillThisBuffer(base->mTunneledComponent, bufHeader);
                    }else{
                        OMX_EmptyThisBuffer(base->mTunneledComponent, bufHeader);
                    }
                }
            }else{
                void *comp = NULL;
                MagOmxComponentImpl hCompImpl;

                if (!msg->findPointer(msg, "component_obj", &comp)){
                    AGILE_LOGE("failed to find the component_obj!");
                    return;
                }
                hCompImpl = ooc_cast(comp, MagOmxComponentImpl);
                
                if (root->isInputPort(root)){
                    hCompImpl->sendEmptyBufferDoneEvent(hCompImpl, bufHeader);
                }else{
                    hCompImpl->sendFillBufferDoneEvent(hCompImpl, bufHeader);
                }
            }
            
            break;

        case MagOmxPortImpl_SharedBufferMsg:
            if (root->isTunneled(root)){
                if (root->isBufferSupplier(root)){
                    if (root->isInputPort(root)){
                        OMX_FillThisBuffer(base->mTunneledComponent, bufHeader);
                    }else{
                        OMX_EmptyThisBuffer(base->mTunneledComponent, bufHeader);
                    }
                }else{
                    AGILE_LOGE("Failed! the port is none-supplier.");
                }
            }else{
                AGILE_LOGE("the port is NOT tunneled!");
            }
            break;

        default:
            AGILE_LOGE("Wrong commands(%d)!", cmd);
            break;
    }
}

/*
 * Virtual Functions Implementation
 */
static OMX_ERRORTYPE virtual_Enable(OMX_HANDLETYPE hPort, OMX_PTR AppData){
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->getDef_Enabled(root)){
        if (!root->getDef_Populated(root)){
            if (root->isTunneled(root)){
                ret = root->AllocateTunnelBuffer(root);
                if (ret == OMX_ErrorNone){
                    root->setDef_Enabled(hPort, OMX_TRUE);
                }
            }else{
                AGILE_LOGE("the port is not tunneled. Need IL Client to do AllocateBuffer() before enabling it");
                return OMX_ErrorPortUnpopulated;
            }
        }else{
            root->setDef_Enabled(hPort, OMX_TRUE);
        }
        
    }else{
        AGILE_LOGD("the port has already been enabled!");
    }
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Disable(OMX_HANDLETYPE hPort){
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    OMX_BUFFERHEADERTYPE *pBufferHeader;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->getDef_Enabled(root)){
        if (root->getDef_Populated(root)){
            if (root->isTunneled(root)){
                root->FreeTunnelBuffer(root);
            }else{
                Mag_AcquireMutex(supplierPortImpl->mhMutex);
                pNode = base->mBufferList.next;
                while (pNode != &base->mBufferList){
                    item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
                    pBufferHeader = item->pOmxBufferHeader;
                    MagOmxPortVirtual(root)->FreeBuffer(root, pBufferHeader);
                    pNode = base->mBufferList.next;
                }
                Mag_ReleaseMutex(supplierPortImpl->mhMutex);
            }
        }
        root->setDef_Enabled(hPort, OMX_FALSE);
    }else{
        AGILE_LOGD("the port has already been disabled!");
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Run(OMX_HANDLETYPE hPort){
    OMX_ERRORTYPE  err;
    MagOmxPort     root;
    MagOmxPortImpl base;
    OMX_BUFFERHEADERTYPE *buffer;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->isTunneled(root)){
        if (root->isBufferSupplier(root)){
recheck:
            if (root->getParameter(root, OMX_IndexConfigTunneledPortStatus, &status) == OMX_ErrorNone){
                if (OMX_PORTSTATUS_ACCEPTBUFFEREXCHANGE != status){
                    Mag_WaitForEventGroup(mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                    goto recheck;
                }else{
                    Mag_ClearEvent(mTunneledBufStEvt);
                }
            }else{
                Mag_WaitForEventGroup(mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                goto recheck;
            }

            err = base->getRunningNode(base, &buffer);
            if (err == OMX_ErrorNone){
                if (root->isInputPort(root))
                    OMX_FillThisBuffer(base->mTunneledComponent, buffer);
            }else{
                AGILE_LOGE("The running list is empty!");
                return OMX_ErrorInsufficientResources;
            }
        }else{
            MagOmxComponentImpl tunnelComp;
            MagOmxPort          tunneledPort;

            tunnelComp = ooc_cast(mTunneledComponent, MagOmxComponentImpl); 
            tunneledPort = tunnelComp->getPort(tunnelComp, mTunneledPortIndex);

            tunneledPort->setParameter(bufSupplier, 
                                  OMX_IndexConfigTunneledPortStatus, 
                                  OMX_PORTSTATUS_ACCEPTBUFFEREXCHANGE);
        }
    }
}

static OMX_ERRORTYPE virtual_Flush(OMX_HANDLETYPE hPort){
    OMX_ERRORTYPE  err;
    MagOmxPort     root;
    MagOmxPortImpl base;
    OMX_BUFFERHEADERTYPE *buffer;
    OMX_S32 diff;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->isTunneled(root)){
        if (root->isBufferSupplier(root)){
            diff = base->mBuffersTotal - base->mFreeBuffersNum;
            if (diff > 0){
                base->mWaitOnBuffers = OMX_TRUE;
                Mag_ClearEvent(mAllBufReturnedEvent);
                Mag_WaitForEventGroup(mBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            }else if(diff < 0){
                AGILE_LOGE("wrong buffer counts: mFreeBuffersNum(%d) - mBuffersTotal(%d)",
                            base->mFreeBuffersNum, base->mBuffersTotal);
                return OMX_ErrorUndefined;
            }
        }
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Pause(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->getDef_Enabled(root)){
        return base->mLooper->suspend(base->mLooper);
    }
}

static OMX_ERRORTYPE virtual_Resume(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->getDef_Enabled(root)){
        return base->mLooper->resume(base->mLooper);
    }
}

static OMX_ERRORTYPE virtual_MarkBuffer(OMX_HANDLETYPE hPort, OMX_MARKTYPE * mark){

}

/*request the component to allocate a new buffer and buffer header*/
static OMX_ERRORTYPE virtual_AllocateBuffer(
                    OMX_HANDLETYPE hPort,
                    OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_PTR pAppPrivate,
                    OMX_U32 nSizeBytes){

    OMX_ERRORTYPE ret;
    
    if (NULL == hPort){
        AGILE_LOGE("the parameter hPort is NULL!");
        return;
    }

    Mag_AcquireMutex(getBase(hPort)->mhMutex);
    ret = allocateBufferInternal(hPort, ppBufferHdr, pAppPrivate, nSizeBytes);
    Mag_ReleaseMutex(getBase(hPort)->mhMutex);
    return ret;
}

static OMX_ERRORTYPE virtual_AllocateTunnelBuffer(
                   OMX_HANDLETYPE hPort){
    OMX_ERRORTYPE ret;
    OMX_ERRORTYPE err;
    MagOmxPort     root;
    MagOmxPortImpl base;

    MagOmxComponentImpl tunnelComp;
    MagOmxPort          tunneledPort;

    OMX_BUFFERHEADERTYPE *bufferHeader;
    OMX_U8 *pBuffer;

    MagOmxPort     bufSupplier;
    MagOmxPortImpl bufSupplierImpl;
    MagOmxPort     non_bufSupplier;
    MagOMX_Port_Buffer_t *bufNode;

    OMX_BOOL isSupplier = OMX_FALSE;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->isTunneled(root)){
        AGILE_LOGE("the port[0x%p] is not tunneled", hPort);
        return OMX_ErrorPortsNotConnected;
    }

    tunnelComp = ooc_cast(mTunneledComponent, MagOmxComponentImpl); 
    tunneledPort = tunnelComp->getPort(tunnelComp, mTunneledPortIndex);

    if (root->getDef_Populated(root) && tunneledPort->getDef_Populated(tunneledPort)){
        AGILE_LOGD("the tunneled ports are all polulated!");
        return OMX_ErrorNone;
    }

    /*find the buffer supplier port within tunneled ports*/
    if (root->isBufferSupplier(root)){
        OMX_U32 status;

        bufSupplier = root;
        non_bufSupplier = tunneledPort;
        isSupplier = OMX_TRUE;

recheck:
        if (root->getParameter(root, OMX_IndexConfigTunneledPortStatus, &status) == OMX_ErrorNone){
            if (OMX_PORTSTATUS_ACCEPTUSEBUFFER != status){
                Mag_WaitForEventGroup(mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                goto recheck;
            }else{
                Mag_ClearEvent(mTunneledBufStEvt);
            }
        }else{
            Mag_WaitForEventGroup(mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            goto recheck;
        }
    }else if (tunneledPort->isBufferSupplier(tunneledPort)){
        tunneledPort->setParameter(bufSupplier, 
                                  OMX_IndexConfigTunneledPortStatus, 
                                  OMX_PORTSTATUS_ACCEPTUSEBUFFER);
    }else{
        AGILE_LOGE("None of the ports[0x%p - 0x%p] is buffer supplier", 
                    root, tunneledPort);
        return OMX_ErrorNotImplemented;
    }

    if (isSupplier){
        bufSupplierImpl = ooc_cast(bufSupplier, MagOmxPortImpl);

        for (i = 0; i < bufSupplier->getDef_BufferCountActual(bufSupplier), i++){
            ret = bufSupplierImpl->MagOMX_AllocateBuffer(bufSupplierImpl, 
                                                         &pBuffer, 
                                                         bufSupplier->getDef_BufferSize(bufSupplier));
            if (ret == OMX_ErrorNone){
                err = non_bufSupplier->UseBuffer(non_bufSupplier, 
                                                 &bufferHeader, 
                                                 NULL, 
                                                 bufSupplier->getDef_BufferSize(bufSupplier), 
                                                 pBuffer);

                if (err == OMX_ErrorNone){
                    bufNode = bufSupplierImpl->allocBufferNode(bufferHeader);
                    bufNode->bufferOwner = MagOmxPortImpl_OwnedByThisPort;
                    list_add_tail(&bufNode->node, &bufSupplierImpl->mBufferList);
                    Mag_AcquireMutex(bufSupplierImpl->mhMutex);
                    list_add_tail(&bufNode->runNode, &bufSupplierImpl->mRunningBufferList);
                    Mag_ReleaseMutex(bufSupplierImpl->mhMutex);
                }else{
                    FreeTunnelBufferInternal(bufSupplier, non_bufSupplier);
                    AGILE_LOGE("failed to do UseBuffer(0x%p)", non_bufSupplier);
                    return err;
                }
            }
        }
    }

    if (isSupplier){
        if (i == bufSupplier->getDef_BufferCountActual(bufSupplier)){
            bufSupplier->setDef_Populated(bufSupplier, OMX_TRUE);
        }
    }else{
        if (!root->getDef_Populated(root)){
            Mag_WaitForEventGroup(mBufPopulatedEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        }else{
            Mag_ClearEvent(mBufPopulatedEvt);
        }
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_UseBuffer(
                   OMX_HANDLETYPE hPort,
                   OMX_BUFFERHEADERTYPE **ppBufferHdr,
                   OMX_PTR pAppPrivate,
                   OMX_U32 nSizeBytes,
                   OMX_U8 *pBuffer){

    OMX_ERRORTYPE ret;
    
    if (NULL == hPort){
        AGILE_LOGE("the parameter hPort is NULL!");
        return; 
    }

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
    ret = freeBufferInternal(hPort, pBuffer);
    Mag_ReleaseMutex(getBase(hPort)->mhMutex);
    return ret;
}

OMX_ERRORTYPE virtual_FreeAllBuffers(OMX_HANDLETYPE hPort){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    OMX_BUFFERHEADERTYPE *pBufferHeader;
    MagOmxPortImpl PortImpl   = ooc_cast(MagOmxPortImpl, hPort);
    MagOmxPort     Port       = ooc_cast(MagOmxPort, hPort);

    Mag_AcquireMutex(PortImpl->mhMutex);
    pNode = PortImpl->mBufferList.next;
    while (pNode != &PortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        pBufferHeader = item->pOmxBufferHeader;
        MagOmxPortVirtual(Port)->FreeBuffer(Port, pBufferHeader);
        pNode = PortImpl->mBufferList.next;
    }
    Mag_ReleaseMutex(PortImpl->mhMutex);

    Port->setDef_Populated(Port, OMX_FALSE);
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FreeTunnelBuffer(
                  OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    MagOmxComponentImpl tunnelComp;
    MagOmxPort          tunneledPort;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->isTunneled(root)){
        AGILE_LOGE("the port[0x%p] is not tunneled", hPort);
        return OMX_ErrorPortsNotConnected;
    }

    tunnelComp = ooc_cast(mTunneledComponent, MagOmxComponentImpl); 
    tunneledPort = tunnelComp->getPort(tunnelComp, mTunneledPortIndex);

    if (root->isBufferSupplier(root)){
        /*supplier port: wait for all buffers returning*/
        root->Flush(root);
        FreeTunnelBufferInternal(root, tunneledPort);
        root->setDef_Populated(root, OMX_FALSE);
    }else if (tunneledPort->isBufferSupplier(tunneledPort)){
        /*supplier port: wait for all buffers returning*/
        tunneledPort->Flush(tunneledPort);
        FreeTunnelBufferInternal(tunneledPort, root);
        tunneledPort->setDef_Populated(tunneledPort, OMX_FALSE);
    }else{
        AGILE_LOGE("None of the ports[0x%p - 0x%p] is buffer supplier", 
                    root, tunneledPort);
        return OMX_ErrorNotImplemented;
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_EmptyThisBuffer(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->isInputPort(root)){
        if (base->mEmptyThisBufferMsg == NULL){
            base->mEmptyThisBufferMsg = base->createMessage(hPort, MagOmxPortImpl_EmptyThisBufferMsg);
        }

        base->mEmptyThisBufferMsg->setPointer(base->mEmptyThisBufferMsg, "buffer_header", (void *)pBuffer);
        base->mEmptyThisBufferMsg->postMessage(base->mEmptyThisBufferMsg, 0);
    }else{
        AGILE_LOGE("could not do it on Output port!");
        return OMX_ErrorIncorrectStateOperation;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FillThisBuffer(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->isInputPort(root)){
        if (base->mFillThisBufferMsg == NULL){
            base->mFillThisBufferMsg = base->createMessage(hPort, MagOmxPortImpl_FillThisBufferMsg);
        }

        base->mFillThisBufferMsg->setPointer(base->mFillThisBufferMsg, "buffer_header", (void *)pBuffer);
        base->mFillThisBufferMsg->postMessage(base->mFillThisBufferMsg, 0);
    }else{
        AGILE_LOGE("could not do it on Input port!");
        return OMX_ErrorIncorrectStateOperation;
    }
    return OMX_ErrorNone;
}

static OMX_BOOL checkPortsCompatible(OMX_PARAM_PORTDEFINITIONTYPE *outPorDef, OMX_PARAM_PORTDEFINITIONTYPE *inPorDef){
    if (outPorDef->eDomain == inPorDef->eDomain){
        if (outPorDef->eDomain == OMX_PortDomainAudio){
            if (outPorDef->format.audio.eEncoding != inPorDef->format.audio.eEncoding){
                AGILE_LOGE("Mismatching Audio Encoding: oport[%d] - iport[%d]",
                            outPorDef->format.audio.eEncoding,
                            inPorDef->format.audio.eEncoding);
                return OMX_FALSE;
            }
        }else if (outPorDef->eDomain == OMX_PortDomainVideo){
            if (outPorDef->format.video.eColorFormat != inPorDef->format.video.eColorFormat || 
                outPorDef->format.video.eCompressionFormat != inPorDef->format.video.eCompressionFormat){
                AGILE_LOGE("Mismatching Video Color/Compression Format: oport[%d:%d] - iport[%d:%d]",
                            outPorDef->format.video.eColorFormat,
                            outPorDef->format.video.eCompressionFormat,
                            inPorDef->format.video.eColorFormat,
                            inPorDef->format.video.eCompressionFormat);
                return OMX_FALSE;
            }
        }else if (outPorDef->eDomain == OMX_PortDomainOther){
            if(outPorDef->format.other.eFormat != inPorDef->format.other.eFormat){
                AGILE_LOGE("Mismatching Other Format: oport[%d] - iport[%d]",
                            outPorDef->format.other.eFormat,
                            inPorDef->format.other.eFormat);
                return OMX_FALSE;
            }
        }
        return OMX_TRUE;
    }else{
        return OMX_FALSE;
    }
}

static OMX_ERRORTYPE virtual_SetupTunnel(
                  OMX_HANDLETYPE hPort,
                  OMX_HANDLETYPE hTunneledComp,
                  OMX_U32        nTunneledPortIndex,
                  OMX_TUNNELSETUPTYPE* pTunnelSetup){

    MagOmxPort     root;
    MagOmxPortImpl base;
    OMX_PARAM_PORTDEFINITIONTYPE outPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE inPortDef;
    OMX_ERRORTYPE ret;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (hTunneledComp == NULL){
        if (root->isTunneled(root)){
            AGILE_LOGD("The port[0x%p] tears down the tunnel", hPort);
            base->mTunneledComponent = NULL;
            base->mTunneledPortIndex = nTunneledPortIndex;
            root->setTunneledFlag(root, OMX_FALSE);
            root->resetBufferSupplier(root);
            return OMX_ErrorNone;
        }else{
            AGILE_LOGE("Unable to tear down the port that is not tunneled!");
            return OMX_ErrorNone;
        }
    }else{
        if (root->isTunneled(root)){
            AGILE_LOGE("The port is tunneled and unable to do tunnel before it is teared down!");
            return OMX_ErrorTunnelingUnsupported;
        }
    }

    if (!root->isInputPort(root)){
        /*Output Port*/
        pTunnelSetup->eSupplier  = root->mBufferSupplier;
        base->mTunneledComponent = hTunneledComp;
        base->mTunneledPortIndex = nTunneledPortIndex;
        root->setTunneledFlag(root, OMX_TRUE);
    }else{
        OMX_PARAM_BUFFERSUPPLIERTYPE tunneled_supplier;

        /*Input Port*/
        outPortDef.nPortIndex = nTunneledPortIndex;
        ret = OMX_GetParameter(hTunneledComp, OMX_IndexParamPortDefinition, &outPortDef);
        if (ret != OMX_ErrorNone){
            AGILE_LOGE("Unable to get the buffer supplier, ret = 0x%x", ret);
            return OMX_ErrorPortsNotCompatible;
        }
        root->getPortDefinition(root, &inPortDef);

        /*check ports compatibility*/
        if (!checkPortsCompatible(&outPortDef, &inPortDef)){
            AGILE_LOGE("Ports Mismatching!");
            return OMX_ErrorPortsNotCompatible;
        }

        /*check buffer supplier configuration*/
        if ((root->mBufferSupplier   == OMX_BufferSupplyInput) &&
            (pTunnelSetup->eSupplier == OMX_BufferSupplyOutput)){
            AGILE_LOGE("The port buffer supplier conflicts!");
            return OMX_ErrorPortsNotCompatible;
        }else if ((root->mBufferSupplier   == OMX_BufferSupplyUnspecified) &&
                  (pTunnelSetup->eSupplier == OMX_BufferSupplyUnspecified)){
            initHeader(&tunneled_supplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
            tunneled_supplier.nPortIndex      = nTunneledPortIndex;
            tunneled_supplier.eBufferSupplier = OMX_BufferSupplyOutput;
            OMX_SetParameter(hTunneledComp, OMX_IndexParamCompBufferSupplier,
                             &tunneled_supplier);
            pTunnelSetup->eSupplier  = OMX_BufferSupplyOutput;
        }else{
            if (root->mBufferSupplier != OMX_BufferSupplyUnspecified){
                pTunnelSetup->eSupplier  = OMX_BufferSupplyInput;
            }
        }
        AGILE_LOGD("Negotiated buffer supplier: %s", 
                    pTunnelSetup->eSupplier == OMX_BufferSupplyInput ? "input port": "output port");

        /*check buffer count*/
        if (outPortDef.nBufferCountMin > inPortDef.nBufferCountMin){
            inPortDef.nBufferCountActual  = outPortDef.nBufferCountMin;
            outPortDef.nBufferCountActual = outPortDef.nBufferCountMin;
        }else{
            inPortDef.nBufferCountActual  = inPortDef.nBufferCountMin;
            outPortDef.nBufferCountActual = inPortDef.nBufferCountMin;
        }

        OMX_SetParameter(hTunneledComp, OMX_IndexParamPortDefinition, &outPortDef);
        root->setPortDefinition(root, &inPortDef);

        base->mTunneledComponent = hTunneledComp;
        base->mTunneledPortIndex = nTunneledPortIndex;
        root->setTunneledFlag(root, OMX_TRUE);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE virtual_RegisterBufferHandler(OMX_HANDLETYPE hPort,
                                            MagEventHandle pBufferHandler){
    MagOmxPort     root;
    MagOmxPortImpl base;
    BufferDispatcherNode_t *pNode;

    root = getRoot(hPort);
    base = getBase(hPort);

    if ((root->getBufferPolicy(root) != kNoneSharedBuffer) &&
        (root->isBufferSupplier(root))){
        AGILE_LOGE("Failed! Using shared buffer policy but the port is buffer supplier.");
        return OMX_ErrorUnsupportedSetting;
    }

    pNode = (BufferDispatcherNode_t *)mag_mallocz(sizeof(BufferDispatcherNode_t));
    INIT_LIST(&pNode->node);
    pNode->msg = pBufferHandler;
    list_add_tail(&pNode->node, &base->mBufDispatcherList);

    return OMX_ErrorNone;
}

static void virtual_SendEvent(OMX_HANDLETYPE hPort,
                       MagOmxPort_Event_t evtType){
    switch (evtType){
        case kTunneledPortStatusEvt:
            Mag_SetEvent(mTunneledBufStEvt); 
            break;

        case kBufferPopulatedEvt:
            Mag_SetEvent(mBufPopulatedEvt); 
            break;

        default:
            break;
    }
}

MagMessageHandle virtual_GetSharedBufferMsg(OMX_HANDLETYPE hPort){
    MagOmxPortImpl hPortImpl = NULL;
    
    if (NULL == hPort){
        return NULL;
    }

    hPortImpl = ooc_cast(hPort, MagOmxPortImpl);
    if (hPortImpl->mSharedBufferMsg == NULL){
        hPortImpl->mSharedBufferMsg = hPortImpl->createMessage(hPortImpl, MagOmxPortImpl_SharedBufferMsg);
    }
    return hPortImpl->mSharedBufferMsg;
}

/*Member functions*/
static MagMessageHandle MagOmxPortImpl_createMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxPortImpl hPort = NULL;
    
    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hPort = ooc_cast(handle, MagOmxPortImpl);
        
    hPort->getLooper(handle);
    
    MagMessageHandle msg = createMagMessage(hPort->mLooper, what, hPort->mMsgHandler->id(hPort->mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxPortImpl_getLooper(OMX_HANDLETYPE handle){
    MagOmxPortImpl hPort = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hPort = ooc_cast(handle, MagOmxPortImpl);
    
    if ((NULL != hPort->mLooper) && (NULL != hPort->mMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hPort->mLooper){
        hPort->mLooper = createLooper(LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hPort->mLooper);
    }
    
    if (NULL != hPort->mLooper){
        if (NULL == hPort->mMsgHandler){
            hPort->mMsgHandler = createHandler(hPort->mLooper, onMessageReceived, handle);

            if (NULL != hPort->mMsgHandler){
                hPort->mLooper->registerHandler(hPort->mLooper, hPort->mMsgHandler);
                hPort->mLooper->start(hPort->mLooper);
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

static void MagOmxPortImpl_dispatchBuffers(MagOmxPortImpl port, OMX_BUFFERHEADERTYPE *bufHeader){
    List_t *pNode;
    BufferDispatcherNode_t *item;
    MagEventHandle hMsg;

    pNode = port->mBufDispatcherList.next;
    while (pNode != &port->mBufDispatcherList){
        item = (BufferDispatcherNode_t *)list_entry(pNode, BufferDispatcherNode_t, node);
        hMsg = item->msg;
        if (port->mReturnThisBufferMsg == NULL){
            port->mReturnThisBufferMsg = port->createMessage(port, MagOmxPortImpl_ReturnThisBufferMsg);
        }

        hMsg->setMessage(hMsg, "return_buf_msg", port->mReturnThisBufferMsg, MAG_FALSE);
        hMsg->setPointer(hMsg, "buffer_header", bufHeader, MAG_FALSE);
        hMsg->postMessage(hMsg, 0);
        pNode = port->mBufDispatcherList.next;
    }
}

static MagOMX_Port_Buffer_t *MagOmxPortImpl_allocBufferNode(OMX_BUFFERHEADERTYPE* pBuffer){
    MagOMX_Port_Buffer_t *pBufNode;

    pBufNode = (MagOMX_Port_Buffer_t *)mag_mallocz(sizeof(MagOMX_Port_Buffer_t));
    MAG_ASSERT(pBufNode);

    INIT_LIST(&pBufNode->node);
    INIT_LIST(&pBufNode->runNode);
    pBufNode->bufferHeaderOwner = MagOmxPortImpl_OwnedByNone;
    pBufNode->bufferOwner       = MagOmxPortImpl_OwnedByNone;
    pBufNode->pOmxBufferHeader  = pBuffer;
    pBuffer->pPlatformPrivate   = (OMX_PTR)pBufNode;

    return pBufNode;
}

/*only called by buffer supplier port*/
static OMX_ERRORTYPE MagOmxPortImpl_putRunningNode(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;

    item = (MagOMX_Port_Buffer_t *)pBuffer->pPlatformPrivate;
    if (item == NULL){
        AGILE_LOGE("wrong buffer pointer: NULL!!");
        return OMX_ErrorBadParameter;
    }

    INIT_LIST(&item->runNode);

    Mag_AcquireMutex(hPort->mhMutex);
    list_add_tail(&item->runNode, &hPort->mRunningBufferList);
    hPort->mFreeBuffersNum++;
    if (hPort->mWaitOnBuffers){
        if (hPort->mFreeBuffersNum == hPort->mBuffersTotal){
            AGILE_LOGD("all buffers are returned!");
            Mag_SetEvent(mAllBufReturnedEvent); 
        }
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}

/*only called by buffer supplier port*/
static OMX_ERRORTYPE MagOmxPortImpl_getRunningNode(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;

    Mag_AcquireMutex(hPort->mhMutex);
    pNode = hPort->mRunningBufferList.next;
    if (pNode != &hPort->mRunningBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, runNode);
        list_del(pNode);
        hPort->mFreeBuffersNum--;
        *ppBuffer = item->pOmxBufferHeader;
    }else{
        Mag_ReleaseMutex(hPort->mhMutex);
        AGILE_LOGE("No buffer is in the running list");
        return OMX_ErrorUndefined;
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}


/*Class Constructor/Destructor*/
static void MagOmxPortImpl_initialize(Class this){
    MagOmxPortImplVtableInstance.MagOmxPort.Enable                = virtual_Enable;
    MagOmxPortImplVtableInstance.MagOmxPort.Disable               = virtual_Disable;
    MagOmxPortImplVtableInstance.MagOmxPort.Run                   = virtual_Run;
    MagOmxPortImplVtableInstance.MagOmxPort.Flush                 = virtual_Flush;
    MagOmxPortImplVtableInstance.MagOmxPort.Pause                 = virtual_Pause;
    MagOmxPortImplVtableInstance.MagOmxPort.Resume                = virtual_Resume;
    MagOmxPortImplVtableInstance.MagOmxPort.MarkBuffer            = virtual_MarkBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.AllocateBuffer        = virtual_AllocateBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.AllocateTunnelBuffer  = virtual_AllocateTunnelBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.UseBuffer             = virtual_UseBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.FreeBuffer            = virtual_FreeBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.FreeAllBuffers        = virtual_FreeAllBuffers;
    MagOmxPortImplVtableInstance.MagOmxPort.FreeTunnelBuffer      = virtual_FreeTunnelBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.EmptyThisBuffer       = virtual_EmptyThisBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.FillThisBuffer        = virtual_FillThisBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.SetupTunnel           = virtual_SetupTunnel;
    MagOmxPortImplVtableInstance.MagOmxPort.RegisterBufferHandler = virtual_RegisterBufferHandler;
    MagOmxPortImplVtableInstance.MagOmxPort.SendEvent             = virtual_SendEvent;
    MagOmxPortImplVtableInstance.MagOmxPort.GetSharedBufferMsg    = virtual_GetSharedBufferMsg;

    MagOmxPortImplVtableInstance.MagOMX_AllocateBuffer      = NULL;
    MagOmxPortImplVtableInstance.MagOMX_FreeBuffer          = NULL;
    MagOmxPortImplVtableInstance.MagOMX_EmptyThisBuffer     = NULL;
    MagOmxPortImplVtableInstance.MagOMX_FillThisBuffer      = NULL;
}

static void MagOmxPortImpl_constructor(MagOmxPortImpl thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(MagOmxPortImpl));
    chain_constructor(MagOmxPortImpl, thiz, params);
    
    thiz->createMessage      = MagOmxPortImpl_createMessage;
    thiz->getLooper          = MagOmxPortImpl_getLooper;
    thiz->dispatchBuffers    = MagOmxPortImpl_dispatchBuffers;
    thiz->allocBufferNode    = MagOmxPortImpl_allocBufferNode;
    thiz->putRunningNode     = MagOmxPortImpl_putRunningNode;
    thiz->getRunningNode     = MagOmxPortImpl_getRunningNode;

    Mag_CreateMutex(&thiz->mhMutex);
    INIT_LIST(&thiz->mBufferList);
    INIT_LIST(&thiz->mRunningBufferList);

    thiz->mBuffersTotal        = 0;
    thiz->mFreeBuffersNum      = 0;
    thiz->mWaitOnBuffers       = OMX_FALSE;
    thiz->mLooper              = NULL;
    thiz->mMsgHandler          = NULL;
    thiz->mEmptyThisBufferMsg  = NULL;
    thiz->mFillThisBufferMsg   = NULL;
    thiz->mReturnThisBufferMsg = NULL;
    thiz->mSharedBufferMsg     = NULL;

    thiz->mBufSupplierType    = OMX_BufferSupplyUnspecified;
    thiz->mTunneledComponent  = NULL;
    thiz->mTunneledPortIndex  = kInvalidPortIndex;

    INIT_LIST(&thiz->mBufDispatcherList);

    Mag_CreateEventGroup(&thiz->mBufferEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mAllBufReturnedEvent, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mBufferEventGroup, thiz->mAllBufReturnedEvent);

    Mag_CreateEventGroup(&thiz->mTunneledBufStEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mTunneledBufStEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mTunneledBufStEvtGrp, thiz->mTunneledBufStEvt);

    Mag_CreateEventGroup(&thiz->mBufPopulatedEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mBufPopulatedEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mBufPopulatedEvtGrp, thiz->mBufPopulatedEvt);
}

static void MagOmxPortImpl_destructor(MagOmxPortImpl thiz, MagOmxPortImplVtable vtab){
    AGILE_LOGV("Enter!");
}


