#include "MagOMX_Port_baseImpl.h"
#include "MagOMX_Component_baseImpl.h"

#define LOOPER_NAME        "PortImplLooper"

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

static OMX_ERRORTYPE FreeTunnelBufferInternal(OMX_HANDLETYPE hSupplierPort, OMX_HANDLETYPE hNoneSupplierPort){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    OMX_BUFFERHEADERTYPE *pBufferHeader;
    MagOmxPort     supplierPort       = ooc_cast(hSupplierPort, MagOmxPort);
    MagOmxPortImpl supplierPortImpl   = ooc_cast(hSupplierPort, MagOmxPortImpl);
    MagOmxPort     noSupplierPort     = ooc_cast(hNoneSupplierPort, MagOmxPort);
    MagOmxPortImpl noSupplierPortImpl = ooc_cast(hNoneSupplierPort, MagOmxPortImpl);

    Mag_AcquireMutex(supplierPortImpl->mhMutex);
    pNode = supplierPortImpl->mBufferList.next;
    while (pNode != &supplierPortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        list_del(pNode);
        pBufferHeader = item->pOmxBufferHeader;
        MagOmxPortVirtual(supplierPort)->FreeBuffer(hSupplierPort, pBufferHeader);
        pNode = supplierPortImpl->mBufferList.next;
    }
    Mag_ReleaseMutex(supplierPortImpl->mhMutex);

    Mag_AcquireMutex(noSupplierPortImpl->mhMutex);
    pNode = noSupplierPortImpl->mBufferList.next;
    while (pNode != &noSupplierPortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        list_del(pNode);
        pBufferHeader = item->pOmxBufferHeader;
        MagOmxPortVirtual(noSupplierPort)->FreeBuffer(hNoneSupplierPort, pBufferHeader);
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
    
    if (NULL == hPort){
        PORT_LOGE(NULL, "hPort is NULL!");
        return OMX_ErrorBadParameter;
    }
    
    base = getBase(hPort);
    root = getRoot(hPort);

    if (root->getDef_Populated(root)){
        PORT_LOGE(root, "the port buffers have been populated! To quit the allocation.");
        return OMX_ErrorNoMore;
    }

    if (root->isTunneled(root)){
        PORT_LOGE(root, "Could not allocated the buffer in tunneled port!");
        return OMX_ErrorIncorrectStateOperation;
    }

    pPortBufHeader = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));
    MAG_ASSERT(pPortBufHeader);

    if (MagOmxPortImplVirtual(base)->MagOMX_AllocateBuffer){
        ret = MagOmxPortImplVirtual(base)->MagOMX_AllocateBuffer(base, &pBuffer, nSizeBytes);

        if (ret != OMX_ErrorNone){
            PORT_LOGE(root, "failed to do MagOMX_AllocateBuffer(%d bytes)", nSizeBytes);
            return ret;
        }
    }else{
        PORT_LOGE(root, "pure virtual MagOMX_AllocateBuffer() must be overrided!");
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
    /*
     * To leave mRunningBufferList empty and mOutputBufferList is not used
     * OMX_FillThisBuffer()/OMX_EmptyThisBuffer() adds the node into mRunningBufferList
     */

    *ppBufferHdr = pPortBufHeader;
    if (++base->mBuffersTotal == root->getDef_BufferCountActual(root)){
        base->mFreeBuffersNum = base->mBuffersTotal;
        PORT_LOGD(root, "buffer populates(%d)", base->mBuffersTotal);
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
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;
    MagOMX_Port_Buffer_t *bufNode;

    if (NULL == hPort){
        PORT_LOGE(NULL, "the hPort is NULL");
        return OMX_ErrorBadParameter;
    }
    
    pPortBufHeader = (OMX_BUFFERHEADERTYPE *)mag_mallocz(sizeof(OMX_BUFFERHEADERTYPE));
    MAG_ASSERT(pPortBufHeader);

    base = getBase(hPort);
    root = getRoot(hPort);

    if (root->getDef_Populated(root)){
        PORT_LOGE(root, "the port buffers have been populated! ignore the allocation.");
        return OMX_ErrorNoMore;
    }

    initHeader(pPortBufHeader, sizeof(OMX_BUFFERHEADERTYPE));
    if (pBuffer != NULL){
        pPortBufHeader->pBuffer        = pBuffer;
        pPortBufHeader->nAllocLen      = nSizeBytes;
    }else{
        pPortBufHeader->pBuffer        = NULL;
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
        PORT_LOGD(root, "buffer populates(%d)", base->mBuffersTotal);
        root->setDef_Populated(root, OMX_TRUE);
        MagOmxPortVirtual(root)->SendEvent(hPort, kBufferPopulatedEvt);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE freeBufferInternal(
                       OMX_HANDLETYPE hPort,
                       OMX_BUFFERHEADERTYPE* pBufferHdr){

    OMX_U8 *pBuffer;
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE  ret = OMX_ErrorNone;
    MagOMX_Port_Buffer_t *pBufNode;

    if ((NULL == hPort) || (NULL == pBufferHdr))
        return OMX_ErrorBadParameter;

    base = getBase(hPort);
    root = getRoot(hPort);

    pBufNode = (MagOMX_Port_Buffer_t *)pBufferHdr->pPlatformPrivate;

    if (pBufNode->bufferOwner == MagOmxPortImpl_OwnedByThisPort){
        ret = MagOmxPortImplVirtual(base)->MagOMX_FreeBuffer(base, pBufferHdr->pBuffer);
        if (ret != OMX_ErrorNone){
            PORT_LOGE(root, "failed to free the pBuffer: %p", pBufferHdr->pBuffer);
        }
    }

    if ((pBufNode->bufferHeaderOwner == MagOmxPortImpl_OwnedByThisPort) ||
        (pBufNode->bufferHeaderOwner == MagOmxPortImpl_OwnedByTunneledPort &&
         !root->isBufferSupplier(root))){
        mag_free(pBufferHdr);
    }
    
    list_del(&pBufNode->node);
    mag_free(pBufNode);

    return ret;
}

static void onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxPort     root;
    MagOmxPortImpl base;
    OMX_BUFFERHEADERTYPE *bufHeader;
    OMX_BUFFERHEADERTYPE *freeBufHeader;
    OMX_ERRORTYPE ret;
    OMX_U32 cmd;
    MagOmxPort_State_t portSt;
    MagOmxComponent tunneledComp;

    if (!msg){
        PORT_LOGE(NULL, "msg is NULL!");
        return;
    }
    
    root = getRoot(priv);
    base = getBase(priv);

    if (!msg->findPointer(msg, "buffer_header", &bufHeader)){
        PORT_LOGE(root, "failed to find the buffer_header!");
        return;
    }

    portSt = root->getState(root);
    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxPortImpl_EmptyThisBufferMsg:
            if (root->isTunneled(root)){
                if (root->isInputPort(root) && root->isBufferSupplier(root)){
                    /*get returned buffer header*/
                    if ((portSt == kState_Running) || (portSt == kState_Paused)){
                        /*step #1: send out the next buffer to the tunneled peer*/
                        ret = base->getRunningNode(base, &freeBufHeader);
                        if (ret == OMX_ErrorNone){
                            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                            MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, freeBufHeader);
                        }

                        /*step #2: proceed the returned buffer*/
                        if (MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer){
                            /*Do base->putRunningNode(base, bufHeader) below if it is needed*/
                            MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer(priv, bufHeader);
                            base->putOutputNode(base, bufHeader);
                        }else{
                            PORT_LOGE(root, "pure virtual MagOMX_ProceedReturnedBuffer() must be overrided!");
                        }
                    }else{
                        /*Directly return the buffer header in flushing or stopped state*/
                        base->putRunningNode(base, bufHeader);
                    }

                    return;
                }
            }else{
                Mag_AcquireMutex(base->mhMutex);
                base->mFreeBuffersNum--;
                Mag_ReleaseMutex(base->mhMutex);
            }

            base->dispatchBuffers(priv, bufHeader);
            break;

        case MagOmxPortImpl_FillThisBufferMsg:
            if (root->isTunneled(root)){
                if (!root->isInputPort(root) && root->isBufferSupplier(root)){
                    /*get returned buffer header*/
                    if ((portSt == kState_Running) || (portSt == kState_Paused)){
                        /*step #1: send out the next buffer to the tunneled peer*/
                        ret = base->getRunningNode(base, &freeBufHeader);
                        if (ret == OMX_ErrorNone){
                            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                            MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, freeBufHeader);
                        }

                        /*step #2: proceed the returned buffer*/
                        if (MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer){
                            MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer(priv, bufHeader);
                            base->putOutputNode(base, bufHeader);
                        }else{
                            PORT_LOGE(root, "pure virtual MagOMX_ProceedReturnedBuffer() must be overrided!");
                        }
                    }else{
                        /*Directly return the buffer header in flushing or stopped state*/
                        PORT_LOGD(root, "in state: %d, directly put the buffer into the Output buffer list");
                        base->putOutputNode(base, bufHeader);
                    }

                    return;
                }
            }else{
                Mag_AcquireMutex(base->mhMutex);
                base->mFreeBuffersNum--;
                Mag_ReleaseMutex(base->mhMutex);
            }

            base->dispatchBuffers(priv, bufHeader);
            break;

        case MagOmxPortImpl_ReturnThisBufferMsg:
            PORT_LOGV(root, "MagOmxPortImpl_ReturnThisBufferMsg: buffer[%p]", bufHeader);
            if (root->isTunneled(root)){
                if (root->isBufferSupplier(root)){
                    base->putRunningNode(base, bufHeader);
                }else{
                    tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                    if (root->isInputPort(root)){
                        PORT_LOGV(root, "returned buffer, call FillThisBuffer");
                        bufHeader->nOutputPortIndex = base->mTunneledPortIndex;
                        MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, bufHeader);
                    }else{
                        PORT_LOGV(root, "returned buffer, call EmptyThisBuffer");
                        bufHeader->nInputPortIndex = base->mTunneledPortIndex;
                        MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, bufHeader);
                    }
                }
            }else{
                void *comp = NULL;
                MagOmxComponentImpl hCompImpl;

                if (!msg->findPointer(msg, "component_obj", &comp)){
                    PORT_LOGE(root, "failed to find the component_obj!");
                    return;
                }

                hCompImpl = ooc_cast(comp, MagOmxComponentImpl);
                if (root->isInputPort(root)){
                    hCompImpl->sendEmptyBufferDoneEvent(comp, bufHeader);
                }else{
                    hCompImpl->sendFillBufferDoneEvent(comp, bufHeader);
                }

                Mag_AcquireMutex(base->mhMutex);
                base->mFreeBuffersNum++;
                Mag_ReleaseMutex(base->mhMutex);
                
                if (base->mWaitOnAllBuffers){
                    if (base->mFreeBuffersNum == base->mBuffersTotal){
                        PORT_LOGD(root, "All buffers are returned!");
                        Mag_SetEvent(base->mAllBufReturnedEvent); 
                        base->mWaitOnAllBuffers = OMX_FALSE;
                    }
                }
            }
            break;

        case MagOmxPortImpl_SharedBufferMsg:
            if (root->isTunneled(root)){
                if (root->isBufferSupplier(root)){
                    tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                    if (root->isInputPort(root)){
                        MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, bufHeader);
                    }else{
                        MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, bufHeader);
                    }
                }else{
                    PORT_LOGE(root, "none-tunneled port can't do MagOmxPortImpl_SharedBufferMsg!");
                }
            }else{
                PORT_LOGE(root, "the port is NOT tunneled!");
            }
            break;

        case MagOmxPortImpl_OutputBufferMsg:
            PORT_LOGV(root, "MagOmxPortImpl_OutputBufferMsg: buffer[%p]", bufHeader);
            if (bufHeader){
                if (root->isTunneled(root)){
                    base->putRunningNode(base, bufHeader);
                    if (root->isBufferSupplier(root)){
                        ret = base->getRunningNode(base, &freeBufHeader);
                        if (ret == OMX_ErrorNone){
                            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                            if (!root->isInputPort(root)){
                                MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, freeBufHeader);
                            }else{
                                MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, freeBufHeader);
                            }
                        }else{
                            PORT_LOGE(root, "should not be here! the running list could not be wrong!!");
                        }
                    }
                }else{
                    PORT_LOGE(root, "None-Tunneled port should not handle the OutputBuffer Message");
                }
            }
            break;

        default:
            PORT_LOGE(root, "Wrong commands(%d)!", cmd);
            break;
    }
}

/*
 * Virtual Functions Implementation
 */
static OMX_ERRORTYPE virtual_Enable(OMX_HANDLETYPE hPort){
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE ret;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->getDef_Enabled(root)){
        if (!root->getDef_Populated(root)){
            if (root->isTunneled(root)){
                ret = MagOmxPortVirtual(root)->AllocateTunnelBuffer(hPort);
                if (ret == OMX_ErrorNone){
                    root->setDef_Enabled(root, OMX_TRUE);
                }
            }else{
                PORT_LOGE(root, "the port is not tunneled. Need IL Client to do AllocateBuffer() before enabling it");
                return OMX_ErrorPortUnpopulated;
            }
        }else{
            root->setDef_Enabled(root, OMX_TRUE);
        }
    }else{
        PORT_LOGD(root, "the port has already been enabled!");
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
                MagOmxPortVirtual(root)->FreeTunnelBuffer(hPort);
            }else{
                Mag_AcquireMutex(base->mhMutex);
                pNode = base->mBufferList.next;
                while (pNode != &base->mBufferList){
                    item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
                    list_del(pNode);
                    pBufferHeader = item->pOmxBufferHeader;
                    MagOmxPortVirtual(root)->FreeBuffer(hPort, pBufferHeader);
                    pNode = base->mBufferList.next;
                }
                Mag_ReleaseMutex(base->mhMutex);
            }
        }
        root->setDef_Populated(root, OMX_FALSE);
        root->setDef_Enabled(root, OMX_FALSE);
    }else{
        PORT_LOGD(root, "the port has already been disabled!");
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Run(OMX_HANDLETYPE hPort){
    OMX_ERRORTYPE  err;
    MagOmxPort     root;
    MagOmxPortImpl base;
    MagOmxComponent tunneledComp;
    OMX_BUFFERHEADERTYPE *buffer;
    OMX_U32 status;

    root = getRoot(hPort);
    base = getBase(hPort);

    PORT_LOGV(root, "start to run");
    if (root->isTunneled(root)){
        if (root->isBufferSupplier(root)){  
recheck:
            if (root->getParameter(hPort, OMX_IndexConfigTunneledPortStatus, &status) == OMX_ErrorNone){
                if (OMX_PORTSTATUS_ACCEPTBUFFEREXCHANGE != status){
                    PORT_LOGD(root, "wait on OMX_IndexConfigTunneledPortStatus to be set");
                    Mag_WaitForEventGroup(base->mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                    PORT_LOGD(root, "OMX_IndexConfigTunneledPortStatus is set");
                    goto recheck;
                }else{
                    Mag_ClearEvent(base->mTunneledBufStEvt);
                }
            }else{
                Mag_WaitForEventGroup(base->mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                goto recheck;
            }

            if (root->isInputPort(root)){
                err = base->getRunningNode(base, &buffer);
                if (err == OMX_ErrorNone){
                    tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                    MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, buffer); 
                }else{
                    PORT_LOGE(root, "The running list is empty!");
                    return OMX_ErrorInsufficientResources;
                }
            }
            
        }else{
            MagOmxComponentImpl tunnelComp;
            MagOmxPort          tunneledPort;
            OMX_HANDLETYPE      hTunneledPort;

            tunnelComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
            hTunneledPort = tunnelComp->getPort(base->mTunneledComponent, base->mTunneledPortIndex);
            tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);

            tunneledPort->setParameter(hTunneledPort, 
                                       OMX_IndexConfigTunneledPortStatus, 
                                       OMX_PORTSTATUS_ACCEPTBUFFEREXCHANGE);

            MagOmxPortVirtual(tunneledPort)->SendEvent(hTunneledPort, kTunneledPortStatusEvt);
        }
    }
    root->setState(root, kState_Running);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Flush(OMX_HANDLETYPE hPort){
    OMX_ERRORTYPE  err;
    MagOmxPort     root;
    MagOmxPortImpl base;

    MagOmxComponentImpl tunnelComp;
    OMX_HANDLETYPE      hTunneledPort;
    MagOmxPort          tunneledPort;

    OMX_BUFFERHEADERTYPE *buffer;
    OMX_S32 diff;

    root = getRoot(hPort);
    base = getBase(hPort);

    PORT_LOGE(root, "flushing");
    root->setState(root, kState_Flushing);
    if (root->isTunneled(root)){
        tunnelComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
        hTunneledPort = tunnelComp->getPort(tunnelComp, base->mTunneledPortIndex);
        if (hTunneledPort == NULL){
            PORT_LOGE(root, "Failed to get the port with index: %d", base->mTunneledPortIndex);
            return OMX_ErrorBadPortIndex;
        }
        tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);

        if (root->isBufferSupplier(root)){
            MagOmxPortVirtual(tunneledPort)->Flush(hTunneledPort);
            diff = base->mBuffersTotal - base->mFreeBuffersNum;
            if (diff > 0){
                base->mWaitOnAllBuffers = OMX_TRUE;
                Mag_ClearEvent(base->mAllBufReturnedEvent);
                Mag_WaitForEventGroup(base->mBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            }else if(diff < 0){
                PORT_LOGE(root, "wrong buffer counts: mFreeBuffersNum(%d) - mBuffersTotal(%d)",
                                base->mFreeBuffersNum, base->mBuffersTotal);
                return OMX_ErrorUndefined;
            }

            /*flushing is complete. To set the state of both ports to running*/
            tunneledPort->setState(tunneledPort, kState_Running);
        }
    }else{
        diff = base->mBuffersTotal - base->mFreeBuffersNum;
        if (diff > 0){
            base->mWaitOnAllBuffers = OMX_TRUE;
            Mag_ClearEvent(base->mAllBufReturnedEvent);
            Mag_WaitForEventGroup(base->mBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        }else if(diff < 0){
            PORT_LOGE(root, "wrong buffer counts: mFreeBuffersNum(%d) - mBuffersTotal(%d)",
                            base->mFreeBuffersNum, base->mBuffersTotal);
            return OMX_ErrorUndefined;
        }
    }

    root->setState(root, kState_Running);
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Pause(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->getDef_Enabled(root)){
        base->mLooper->suspend(base->mLooper);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Resume(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->getDef_Enabled(root)){
        base->mLooper->resume(base->mLooper);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MarkBuffer(OMX_HANDLETYPE hPort, OMX_MARKTYPE * mark){
    return OMX_ErrorNone;
}

/*request the component to allocate a new buffer and buffer header*/
static OMX_ERRORTYPE virtual_AllocateBuffer(
                    OMX_HANDLETYPE hPort,
                    OMX_BUFFERHEADERTYPE** ppBufferHdr,
                    OMX_PTR pAppPrivate,
                    OMX_U32 nSizeBytes){

    OMX_ERRORTYPE ret;
    
    if (NULL == hPort){
        PORT_LOGE(NULL, "the parameter hPort is NULL!");
        return;
    }

    Mag_AcquireMutex(getBase(hPort)->mhMutex);
    ret = allocateBufferInternal(hPort, ppBufferHdr, pAppPrivate, nSizeBytes);
    Mag_ReleaseMutex(getBase(hPort)->mhMutex);
    return ret;
}

static OMX_ERRORTYPE virtual_AllocateTunnelBuffer(
                   OMX_HANDLETYPE hPort){
    OMX_ERRORTYPE  err;
    MagOmxPort     root;
    MagOmxPortImpl base;

    MagOmxComponentImpl tunnelComp;
    OMX_HANDLETYPE      hTunneledPort;
    MagOmxPort          tunneledPort;

    OMX_BUFFERHEADERTYPE *bufferHeader;
    OMX_U8 *pBuffer;

    MagOMX_Port_Buffer_t *bufNode;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->isTunneled(root)){
        PORT_LOGE(root, "not tunneled");
        return OMX_ErrorPortsNotConnected;
    }

    tunnelComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
    hTunneledPort = tunnelComp->getPort(base->mTunneledComponent, base->mTunneledPortIndex);
    tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);

    if (root->getDef_Populated(root) && tunneledPort->getDef_Populated(tunneledPort)){
        PORT_LOGD(root, "the tunneled ports are all polulated!");
        return OMX_ErrorNone;
    }

    PORT_LOGD(root, "To allocate the buffers");
    /*find the buffer supplier port within tunneled ports*/
    if (root->isBufferSupplier(root)){
        /*buffer supplier port*/
        OMX_U32 status;
        OMX_U32 i = 0;
        
recheck:
        if (root->getParameter(root, OMX_IndexConfigTunneledPortStatus, &status) == OMX_ErrorNone){
            if (OMX_PORTSTATUS_ACCEPTUSEBUFFER != status){
                Mag_WaitForEventGroup(base->mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                goto recheck;
            }else{
                Mag_ClearEvent(base->mTunneledBufStEvt);
            }
        }else{
            PORT_LOGD(root, "wait on OMX_PORTSTATUS_ACCEPTUSEBUFFER to be set");
            Mag_WaitForEventGroup(base->mTunneledBufStEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            PORT_LOGD(root, "OMX_PORTSTATUS_ACCEPTUSEBUFFER is set");
            goto recheck;
        }

        for (i = 0; i < root->getDef_BufferCountActual(root); i++){
            err = MagOmxPortImplVirtual(base)->MagOMX_AllocateBuffer(hPort, 
                                                                     &pBuffer, 
                                                                     root->getDef_BufferSize(root));
            if (err == OMX_ErrorNone){
                err = MagOmxPortVirtual(tunneledPort)->UseBuffer(hTunneledPort, 
                                                                 &bufferHeader, 
                                                                 NULL, 
                                                                 root->getDef_BufferSize(root), 
                                                                 pBuffer);

                if (err == OMX_ErrorNone){

                    bufNode = base->allocBufferNode(bufferHeader);
                    bufNode->bufferOwner = MagOmxPortImpl_OwnedByThisPort;
                    Mag_AcquireMutex(base->mhMutex);
                    list_add_tail(&bufNode->node, &base->mBufferList);
                    if (root->isInputPort(root)){
                        list_add_tail(&bufNode->runNode, &base->mRunningBufferList);
                        PORT_LOGV(root, "add the buffer[%p] into the RunningBufferList",
                                         bufferHeader);
                    }else{
                        list_add_tail(&bufNode->runNode, &base->mOutputBufferList);
                        PORT_LOGV(root, "add the buffer[%p] into the OutputBufferList",
                                         bufferHeader);
                    }
                    Mag_ReleaseMutex(base->mhMutex);
                }else{
                    FreeTunnelBufferInternal(hPort, hTunneledPort);
                    PORT_LOGE(root, "failed to do UseBuffer");
                    return err;
                }
            }
        }

        if (i == root->getDef_BufferCountActual(root)){
            base->mFreeBuffersNum = i;
            base->mBuffersTotal   = i;
            root->setDef_Populated(root, OMX_TRUE);
        }
    }else{
        /*none buffer supplier port*/
        tunneledPort->setParameter(hTunneledPort, 
                                  OMX_IndexConfigTunneledPortStatus, 
                                  OMX_PORTSTATUS_ACCEPTUSEBUFFER);

        MagOmxPortVirtual(tunneledPort)->SendEvent(hTunneledPort, kTunneledPortStatusEvt);

        if (!root->getDef_Populated(root)){
            PORT_LOGD(root, "before waiting on the buffer population!");
            Mag_WaitForEventGroup(base->mBufPopulatedEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            PORT_LOGD(root, "buffer is populated!");
        }else{
            Mag_ClearEvent(base->mBufPopulatedEvt);
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
        PORT_LOGE(NULL, "the parameter hPort is NULL!");
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
    
    ret = freeBufferInternal(hPort, pBuffer);
    return ret;
}

OMX_ERRORTYPE virtual_FreeAllBuffers(OMX_HANDLETYPE hPort){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    OMX_BUFFERHEADERTYPE *pBufferHeader;
    MagOmxPortImpl PortImpl   = ooc_cast(hPort, MagOmxPortImpl);
    MagOmxPort     Port       = ooc_cast(hPort, MagOmxPort);

    Mag_AcquireMutex(PortImpl->mhMutex);
    pNode = PortImpl->mBufferList.next;
    while (pNode != &PortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        list_del(pNode);
        pBufferHeader = item->pOmxBufferHeader;
        MagOmxPortVirtual(Port)->FreeBuffer(hPort, pBufferHeader);
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
    OMX_HANDLETYPE      hTunneledPort;
    MagOmxPort          tunneledPort;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (!root->isTunneled(root)){
        PORT_LOGD(root, "the port is not tunneled");
        return OMX_ErrorPortsNotConnected;
    }

    tunnelComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
    hTunneledPort = tunnelComp->getPort(tunnelComp, base->mTunneledPortIndex);
    tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);

    if (root->isBufferSupplier(root)){
        /*supplier port: wait for all buffers returning*/
        MagOmxPortVirtual(root)->Flush(hPort);
        FreeTunnelBufferInternal(hPort, hTunneledPort);
        root->setDef_Populated(root, OMX_FALSE);
    }

    if (tunneledPort->isBufferSupplier(tunneledPort)){
        /*supplier port: wait for all buffers returning*/
        MagOmxPortVirtual(tunneledPort)->Flush(hTunneledPort);
        FreeTunnelBufferInternal(hTunneledPort, hPort);
        tunneledPort->setDef_Populated(tunneledPort, OMX_FALSE);
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

        base->mEmptyThisBufferMsg->setPointer(base->mEmptyThisBufferMsg, "buffer_header", (void *)pBuffer, MAG_FALSE);
        base->mEmptyThisBufferMsg->postMessage(base->mEmptyThisBufferMsg, 0);
        PORT_LOGV(root, "post the EmptyThisBuffer message(buffer: %p)", pBuffer);
    }else{
        PORT_LOGE(root, "could not do it on the OUTPUT port!");
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

        base->mFillThisBufferMsg->setPointer(base->mFillThisBufferMsg, "buffer_header", (void *)pBuffer, MAG_FALSE);
        base->mFillThisBufferMsg->postMessage(base->mFillThisBufferMsg, 0);
    }else{
        PORT_LOGE(root, "could not do it on the INPUT port!");
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

    MagOmxPort          root;
    MagOmxPortImpl      base;
    MagOmxComponentImpl tunneledComp; 
    OMX_HANDLETYPE      hTunneledPort;
    MagOmxPort          tunneledPort;

    OMX_PARAM_PORTDEFINITIONTYPE outPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE inPortDef;
    OMX_ERRORTYPE ret;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (hTunneledComp == NULL){
        if (root->isTunneled(root)){
            PORT_LOGV(root, "tear down the tunnel");
            base->mTunneledComponent = NULL;
            base->mTunneledPortIndex = nTunneledPortIndex;
            root->setTunneledFlag(root, OMX_FALSE);
            root->resetBufferSupplier(root);
            return OMX_ErrorNone;
        }else{
            PORT_LOGE(root, "Unable to tear down the port that is not tunneled!");
            return OMX_ErrorNone;
        }
    }else{
        if (root->isTunneled(root)){
            PORT_LOGE(root, "The port is tunneled and unable to tunnel before it is teared down!");
            return OMX_ErrorTunnelingUnsupported;
        }
    }

    if (!root->isInputPort(root)){
        PORT_LOGD(root, "it is output port!");
        /*Output Port*/
        pTunnelSetup->eSupplier  = root->mBufferSupplier;
        base->mTunneledComponent = hTunneledComp;
        base->mTunneledPortIndex = nTunneledPortIndex;
        root->setTunneledFlag(root, OMX_TRUE);
    }else{
        PORT_LOGD(root, "it is input port!");
        OMX_PARAM_BUFFERSUPPLIERTYPE tunneled_supplier;

        /*Input Port*/
        tunneledComp = ooc_cast(hTunneledComp, MagOmxComponentImpl);
        hTunneledPort = tunneledComp->getPort(tunneledComp, nTunneledPortIndex);
        tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);

        tunneledPort->getPortDefinition(tunneledPort, &outPortDef);
        root->getPortDefinition(root, &inPortDef);

        /*check ports compatibility*/
        if (!checkPortsCompatible(&outPortDef, &inPortDef)){
            PORT_LOGE(root, "Ports Mismatching!");
            return OMX_ErrorPortsNotCompatible;
        }

        /*check buffer supplier configuration*/
        if ((root->mBufferSupplier   == OMX_BufferSupplyInput) &&
            (pTunnelSetup->eSupplier == OMX_BufferSupplyOutput)){
            PORT_LOGE(root, "The port buffer supplier conflicts!");
            return OMX_ErrorPortsNotCompatible;
        }else if ((root->mBufferSupplier   == OMX_BufferSupplyUnspecified) &&
                  (pTunnelSetup->eSupplier == OMX_BufferSupplyUnspecified)){
            tunneledPort->setParameter(tunneledPort, OMX_IndexParamCompBufferSupplier, OMX_BufferSupplyOutput);
            pTunnelSetup->eSupplier  = OMX_BufferSupplyOutput;
        }else{
            if (root->mBufferSupplier != OMX_BufferSupplyUnspecified){
                pTunnelSetup->eSupplier  = OMX_BufferSupplyInput;
            }
        }
        PORT_LOGE(root, "Negotiated buffer supplier: %s", 
                    pTunnelSetup->eSupplier == OMX_BufferSupplyInput ? "input port": "output port");

        /*check buffer count*/
        if (outPortDef.nBufferCountMin > inPortDef.nBufferCountMin){
            inPortDef.nBufferCountActual  = outPortDef.nBufferCountMin;
            outPortDef.nBufferCountActual = outPortDef.nBufferCountMin;
        }else{
            inPortDef.nBufferCountActual  = inPortDef.nBufferCountMin;
            outPortDef.nBufferCountActual = inPortDef.nBufferCountMin;
        }

        tunneledPort->setPortDefinition(tunneledPort, &outPortDef);
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
        PORT_LOGE(root, "Failed! Using shared buffer policy but the port is buffer supplier.");
        return OMX_ErrorUnsupportedSetting;
    }

    pNode = (BufferDispatcherNode_t *)mag_mallocz(sizeof(BufferDispatcherNode_t));
    INIT_LIST(&pNode->node);
    pNode->msg = pBufferHandler;
    list_add_tail(&pNode->node, &base->mBufDispatcherList);
    PORT_LOGD(root, "add the port index: %d", root->getPortIndex(root));

    return OMX_ErrorNone;
}

static void virtual_SendEvent(OMX_HANDLETYPE hPort,
                              MagOmxPort_Event_t evtType){
    MagOmxPortImpl base = getBase(hPort);

    switch (evtType){
        case kTunneledPortStatusEvt:
            Mag_SetEvent(base->mTunneledBufStEvt); 
            break;

        case kBufferPopulatedEvt:
            Mag_SetEvent(base->mBufPopulatedEvt); 
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

/*called from MagOMX_ProceedBuffer() to provide the destinated/source buffer*/
MagMessageHandle virtual_GetOutputBufferMsg(OMX_HANDLETYPE hPort){
    MagOmxPortImpl hPortImpl = NULL;
    OMX_BUFFERHEADERTYPE *pBufHeader;
    OMX_ERRORTYPE ret;

    if (NULL == hPort){
        return NULL;
    }

    hPortImpl = ooc_cast(hPort, MagOmxPortImpl);

    ret = hPortImpl->getOutputNode(hPortImpl, &pBufHeader);
    if (ret != OMX_ErrorNone){
        hPortImpl->mWaitOnOutputBuffer = OMX_TRUE;
        Mag_ClearEvent(hPortImpl->mGetOutputBufferEvent);
        PORT_LOGD(getRoot(hPort), "wait on getting new output buffer!");
        Mag_WaitForEventGroup(hPortImpl->mOutputBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        PORT_LOGD(getRoot(hPort), "get an output buffer!");
        hPortImpl->getOutputNode(hPortImpl, &pBufHeader);
    }
    
    if (pBufHeader){
        if (hPortImpl->mOutputBufferMsg == NULL){
            hPortImpl->mOutputBufferMsg = hPortImpl->createMessage(hPort, MagOmxPortImpl_OutputBufferMsg);
        }

        hPortImpl->mOutputBufferMsg->setPointer(hPortImpl->mOutputBufferMsg, "buffer_header", (void *)pBufHeader, MAG_FALSE);
    }else{
        hPortImpl->mOutputBufferMsg->setPointer(hPortImpl->mOutputBufferMsg, "buffer_header", NULL, MAG_FALSE);
        return NULL;
    }

    return hPortImpl->mOutputBufferMsg;
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

static void MagOmxPortImpl_dispatchBuffers(OMX_HANDLETYPE hPort, OMX_BUFFERHEADERTYPE *bufHeader){
    List_t *pNode;
    BufferDispatcherNode_t *item;
    MagMessageHandle hMsg;
    MagOmxPortImpl port = NULL;
    MagOmxPort     portRoot = NULL;

    if (hPort == NULL){
        return;
    }

    port = ooc_cast(hPort, MagOmxPortImpl);
    portRoot = ooc_cast(hPort, MagOmxPort);

    pNode = port->mBufDispatcherList.next;
    while (pNode != &port->mBufDispatcherList){
        item = (BufferDispatcherNode_t *)list_entry(pNode, BufferDispatcherNode_t, node);
        hMsg = item->msg;
        if (port->mReturnThisBufferMsg == NULL){
            port->mReturnThisBufferMsg = port->createMessage(hPort, MagOmxPortImpl_ReturnThisBufferMsg);
        }

        hMsg->setMessage(hMsg, "return_buf_msg", port->mReturnThisBufferMsg, MAG_FALSE);
        hMsg->setPointer(hMsg, "buffer_header", bufHeader, MAG_FALSE);
        hMsg->postMessage(hMsg, 0);
        PORT_LOGD(portRoot, "dispatch buffer header: %p", bufHeader);
        pNode = pNode->next;
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
    MagOmxPort portRoot = NULL;

    item = (MagOMX_Port_Buffer_t *)pBuffer->pPlatformPrivate;
    if (item == NULL){
        PORT_LOGD(NULL, "wrong buffer pointer: NULL!!");
        return OMX_ErrorBadParameter;
    }

    INIT_LIST(&item->runNode);
    portRoot = ooc_cast(hPort, MagOmxPort);

    Mag_AcquireMutex(hPort->mhMutex);
    list_add_tail(&item->runNode, &hPort->mRunningBufferList);
    if (portRoot->isInputPort(portRoot) &&
        portRoot->isBufferSupplier(portRoot)){
        hPort->mFreeBuffersNum++;
        PORT_LOGD(portRoot, "put Running node. mFreeBuffersNum = %d", hPort->mFreeBuffersNum);
        if (hPort->mWaitOnAllBuffers){
            if (hPort->mFreeBuffersNum == hPort->mBuffersTotal){
                PORT_LOGD(portRoot, "[running buffer list]: All buffers are returned!");
                Mag_SetEvent(hPort->mAllBufReturnedEvent); 
                hPort->mWaitOnAllBuffers = OMX_FALSE;
            }
        }
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}

/*only called by buffer supplier port*/
static OMX_ERRORTYPE MagOmxPortImpl_getRunningNode(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    MagOmxPort portRoot = NULL;

    *ppBuffer = NULL;
    portRoot = ooc_cast(hPort, MagOmxPort);

    Mag_AcquireMutex(hPort->mhMutex);
    pNode = hPort->mRunningBufferList.next;
    if (pNode != &hPort->mRunningBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, runNode);
        list_del(pNode);

        if (portRoot->isInputPort(portRoot) &&
            portRoot->isBufferSupplier(portRoot)){
            hPort->mFreeBuffersNum--;
        }
        *ppBuffer = item->pOmxBufferHeader;
    }else{
        Mag_ReleaseMutex(hPort->mhMutex);
        PORT_LOGD(portRoot, "No buffer is in the running list");
        return OMX_ErrorUndefined;
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}

/*only called by buffer supplier port*/
static OMX_ERRORTYPE MagOmxPortImpl_putOutputNode(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    MagOmxPort portRoot = NULL;

    portRoot = ooc_cast(hPort, MagOmxPort);
    item = (MagOMX_Port_Buffer_t *)pBuffer->pPlatformPrivate;
    if (item == NULL){
        PORT_LOGE(portRoot, "wrong buffer pointer: NULL!!");
        return OMX_ErrorBadParameter;
    }

    INIT_LIST(&item->runNode);

    Mag_AcquireMutex(hPort->mhMutex);
    list_add_tail(&item->runNode, &hPort->mOutputBufferList);

    if (portRoot->isTunneled(portRoot) &&
        !portRoot->isInputPort(portRoot) &&
        portRoot->isBufferSupplier(portRoot)){
        hPort->mFreeBuffersNum++;
        if (hPort->mFreeBuffersNum > hPort->mBuffersTotal){
            PORT_LOGE(portRoot, "free buffer number(%d) > total buffer number(%d)",
                                 hPort->mFreeBuffersNum, hPort->mBuffersTotal);
            MAG_ASSERT(0);
        }
        if (hPort->mWaitOnAllBuffers){
            if (hPort->mFreeBuffersNum == hPort->mBuffersTotal){
                PORT_LOGD(portRoot, "[output buffer list]: All buffers are returned!");
                Mag_SetEvent(hPort->mAllBufReturnedEvent); 
                hPort->mWaitOnAllBuffers = OMX_FALSE;
            }
        }
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    if (hPort->mWaitOnOutputBuffer){
        PORT_LOGD(portRoot, "The empty output buffer list get the new node added!");
        Mag_SetEvent(hPort->mGetOutputBufferEvent); 
        hPort->mWaitOnOutputBuffer = OMX_FALSE;
    }
    return OMX_ErrorNone;
}

/*only called by buffer supplier port*/
static OMX_ERRORTYPE MagOmxPortImpl_getOutputNode(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    MagOmxPort portRoot = NULL;

    *ppBuffer = NULL;
    portRoot = ooc_cast(hPort, MagOmxPort);

    Mag_AcquireMutex(hPort->mhMutex);
    pNode = hPort->mOutputBufferList.next;
    if (pNode != &hPort->mOutputBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, runNode);
        list_del(pNode);
        *ppBuffer = item->pOmxBufferHeader;

        if (portRoot->isTunneled(portRoot) &&
            !portRoot->isInputPort(portRoot) &&
            portRoot->isBufferSupplier(portRoot)){
            hPort->mFreeBuffersNum--;
        }
    }else{
        Mag_ReleaseMutex(hPort->mhMutex);
        PORT_LOGE(portRoot, "No buffer is in the output list");
        return OMX_ErrorUndefined;
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}


/*Class Constructor/Destructor*/
static void MagOmxPortImpl_initialize(Class this){
    AGILE_LOGV("Enter!");

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
    MagOmxPortImplVtableInstance.MagOmxPort.GetOutputBufferMsg    = virtual_GetOutputBufferMsg;

    MagOmxPortImplVtableInstance.MagOMX_AllocateBuffer            = NULL;
    MagOmxPortImplVtableInstance.MagOMX_FreeBuffer                = NULL;
    MagOmxPortImplVtableInstance.MagOMX_ProceedReturnedBuffer     = NULL;
}

/*
 * param[0] = OMX_U32  portIndex;
 * param[1] = OMX_BOOL isInput;
 * param[2] = OMX_BUFFERSUPPLIERTYPE bufSupplier
 * param[3] = OMX_U32  formatStruct
 * param[4] = OMX_U8   port name
 */
static void MagOmxPortImpl_constructor(MagOmxPortImpl thiz, const void *params){
    AGILE_LOGV("Enter!");

    if (params == NULL){
        AGILE_LOGE("The input params is NULL, quit!");
        return;
    }

    MAG_ASSERT(ooc_isInitialized(MagOmxPortImpl));
    chain_constructor(MagOmxPortImpl, thiz, params);
    
    thiz->createMessage      = MagOmxPortImpl_createMessage;
    thiz->getLooper          = MagOmxPortImpl_getLooper;
    thiz->dispatchBuffers    = MagOmxPortImpl_dispatchBuffers;
    thiz->allocBufferNode    = MagOmxPortImpl_allocBufferNode;
    thiz->putRunningNode     = MagOmxPortImpl_putRunningNode;
    thiz->getRunningNode     = MagOmxPortImpl_getRunningNode;
    thiz->putOutputNode      = MagOmxPortImpl_putOutputNode;
    thiz->getOutputNode      = MagOmxPortImpl_getOutputNode;

    Mag_CreateMutex(&thiz->mhMutex);

    INIT_LIST(&thiz->mBufferList);
    INIT_LIST(&thiz->mRunningBufferList);
    INIT_LIST(&thiz->mOutputBufferList);
    INIT_LIST(&thiz->mBufDispatcherList);

    thiz->mBuffersTotal        = 0;
    thiz->mFreeBuffersNum      = 0;
    thiz->mWaitOnAllBuffers       = OMX_FALSE;
    thiz->mWaitOnOutputBuffer  = OMX_FALSE;
    thiz->mLooper              = NULL;
    thiz->mMsgHandler          = NULL;
    thiz->mEmptyThisBufferMsg  = NULL;
    thiz->mFillThisBufferMsg   = NULL;
    thiz->mReturnThisBufferMsg = NULL;
    thiz->mSharedBufferMsg     = NULL;
    thiz->mOutputBufferMsg     = NULL;
    thiz->mBufSupplierType     = OMX_BufferSupplyUnspecified;
    thiz->mTunneledComponent   = NULL;
    thiz->mTunneledPortIndex   = kInvalidPortIndex;

    Mag_CreateEventGroup(&thiz->mBufferEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mAllBufReturnedEvent, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mBufferEventGroup, thiz->mAllBufReturnedEvent);

    Mag_CreateEventGroup(&thiz->mOutputBufferEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mGetOutputBufferEvent, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mOutputBufferEventGroup, thiz->mGetOutputBufferEvent);

    Mag_CreateEventGroup(&thiz->mTunneledBufStEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mTunneledBufStEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mTunneledBufStEvtGrp, thiz->mTunneledBufStEvt);

    Mag_CreateEventGroup(&thiz->mBufPopulatedEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mBufPopulatedEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mBufPopulatedEvtGrp, thiz->mBufPopulatedEvt);
}

static void MagOmxPortImpl_destructor(MagOmxPortImpl thiz, MagOmxPortImplVtable vtab){
    AGILE_LOGV("Enter!");

    Mag_DestroyMutex(thiz->mhMutex);

    destroyMagMessage(thiz->mEmptyThisBufferMsg);
    destroyMagMessage(thiz->mFillThisBufferMsg);
    destroyMagMessage(thiz->mReturnThisBufferMsg);
    destroyMagMessage(thiz->mSharedBufferMsg);
    destroyMagMessage(thiz->mOutputBufferMsg);

    destroyLooper(thiz->mLooper);
    destroyHandler(thiz->mMsgHandler);

    Mag_DestroyEvent(thiz->mAllBufReturnedEvent);
    Mag_DestroyEventGroup(thiz->mBufferEventGroup);

    Mag_DestroyEvent(thiz->mGetOutputBufferEvent);
    Mag_DestroyEventGroup(thiz->mOutputBufferEventGroup);

    Mag_DestroyEvent(thiz->mTunneledBufStEvt);
    Mag_DestroyEventGroup(thiz->mTunneledBufStEvtGrp);

    Mag_DestroyEvent(thiz->mBufPopulatedEvt);
    Mag_DestroyEventGroup(thiz->mBufPopulatedEvtGrp);
}


