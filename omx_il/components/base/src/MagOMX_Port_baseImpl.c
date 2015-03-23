/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MagOMX_Port_baseImpl.h"
#include "MagOMX_Component_baseImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompBase"

#define LOOPER_NAME        "PortImplLooper"

/*#define PORT_BUFFER_SEQUENCE*/

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

static void printBufSequence(MagOmxPortImpl hPort, 
                             OMX_BUFFERHEADERTYPE *pBuffer, 
                             const char *caller,
                             int line){
#ifdef PORT_BUFFER_SEQUENCE
    MagOMX_Port_Buffer_t *item;

    item = (MagOMX_Port_Buffer_t *)pBuffer->pPlatformPrivate;
    if (item == NULL){
        PORT_LOGD(NULL, "wrong buffer pointer: NULL!!");
        return;
    }
    PORT_LOGD(getRoot(hPort), "%s[%d]: buffer seq: %d", caller, line, item->sequence);
#endif
}

/*
 * hPort: the buffer supplier port
 * hPeerPort: none-buffer supplier port
 */
static OMX_ERRORTYPE FreeTunnelBufferInternal(OMX_HANDLETYPE hPort, OMX_HANDLETYPE hPeerPort){
    List_t *pNode;
    List_t *pList;
    MagOMX_Port_Buffer_t *item;
    OMX_BUFFERHEADERTYPE *pBufferHeader;
    MagOmxPort     port         = ooc_cast(hPort, MagOmxPort);

    MagOmxPort     firstPort      = NULL;
    MagOmxPortImpl firstPortImpl  = NULL;
    MagOmxPort     secondPort     = NULL;
    MagOmxPortImpl secondPortImpl = NULL;

    PORT_LOGV(port, "enter!");

    if (port->isBufferSupplier(port)){
        firstPort      = port;
        firstPortImpl  = ooc_cast(hPort, MagOmxPortImpl);
        secondPort     = ooc_cast(hPeerPort, MagOmxPort);
        secondPortImpl = ooc_cast(hPeerPort, MagOmxPortImpl);
    }else{
        firstPort      = ooc_cast(hPeerPort, MagOmxPort);
        firstPortImpl  = ooc_cast(hPeerPort, MagOmxPortImpl);
        secondPort     = port;
        secondPortImpl = ooc_cast(hPort, MagOmxPortImpl);
    }

    if (firstPort->isInputPort(firstPort)){
        pList = &firstPortImpl->mRunningBufferList;       
    }else{
        pList = &firstPortImpl->mOutputBufferList;
    }

    Mag_AcquireMutex(firstPortImpl->mhMutex);
    pNode = pList->next;
    while (pNode != pList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, runNode);
        list_del(pNode);
        pBufferHeader = item->pOmxBufferHeader;
        /*free the buffer*/
        MagOmxPortVirtual(firstPort)->FreeBuffer(hPort, &pBufferHeader);
        pNode = pList->next;
    }
    firstPortImpl->mBuffersTotal = 0;
    Mag_ReleaseMutex(firstPortImpl->mhMutex);

    Mag_AcquireMutex(secondPortImpl->mhMutex);
    pNode = secondPortImpl->mBufferList.next;
    while (pNode != &secondPortImpl->mBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, node);
        list_del(pNode);
        pBufferHeader = item->pOmxBufferHeader;
        /*free the buffer header*/
        MagOmxPortVirtual(secondPort)->FreeBuffer(hPeerPort, &pBufferHeader);
        pNode = secondPortImpl->mBufferList.next;
    }
    secondPortImpl->mBuffersTotal = 0;
    Mag_ReleaseMutex(secondPortImpl->mhMutex);

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
    pPortBufHeader->nTimeStamp         = kInvalidTimeStamp;
    pPortBufHeader->pAppPrivate        = pAppPrivate;
    pPortBufHeader->pInputPortPrivate  = root->isInputPort(root) ? hPort : NULL;
    pPortBufHeader->pOutputPortPrivate = root->isInputPort(root) ? NULL : hPort;
    pPortBufHeader->nInputPortIndex    = root->isInputPort(root) ? root->getPortIndex(root) : kInvalidPortIndex;
    pPortBufHeader->nOutputPortIndex   = root->isInputPort(root) ? kInvalidPortIndex : root->getPortIndex(root);
    
    bufNode = base->allocBufferNode(pPortBufHeader);
    bufNode->bufferOwner       = root;
    bufNode->bufferHeaderOwner = root;
    bufNode->sequence          = base->mBuffersTotal;
    list_add_tail(&bufNode->node, &base->mBufferList);
    /*
     * To leave mRunningBufferList empty and mOutputBufferList is not used
     * OMX_FillThisBuffer()/OMX_EmptyThisBuffer() adds the node into mRunningBufferList
     */

    *ppBufferHdr = pPortBufHeader;

    if (root->getDef_BufferCountActual(root) > 0){
        ++base->mBuffersTotal;
    }

    if (base->mBuffersTotal == root->getDef_BufferCountActual(root)){
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
    bufNode->bufferHeaderOwner = root;
    bufNode->sequence          = base->mBuffersTotal;
    list_add_tail(&bufNode->node, &base->mBufferList);

    *ppBufferHdr = pPortBufHeader;

    if (root->getDef_BufferCountActual(root) > 0){
        ++base->mBuffersTotal;
    }

    if (base->mBuffersTotal == root->getDef_BufferCountActual(root)){
        base->mFreeBuffersNum = base->mBuffersTotal;
        PORT_LOGD(root, "buffer populates(%d)", base->mBuffersTotal);
        root->setDef_Populated(root, OMX_TRUE);
        MagOmxPortVirtual(root)->SendEvent(hPort, kBufferPopulatedEvt);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE freeBufferInternal(
                       OMX_HANDLETYPE hPort,
                       OMX_BUFFERHEADERTYPE **ppBufferHdr){

    OMX_U8 *pBuffer;
    MagOmxPortImpl base;
    MagOmxPort     root;
    OMX_ERRORTYPE  ret = OMX_ErrorNone;
    MagOMX_Port_Buffer_t *pBufNode;

    if ((NULL == hPort) || (NULL == *ppBufferHdr))
        return OMX_ErrorBadParameter;

    base = getBase(hPort);
    root = getRoot(hPort);

    pBufNode = (MagOMX_Port_Buffer_t *)(*ppBufferHdr)->pPlatformPrivate;

    if (pBufNode->bufferOwner == root){
        PORT_LOGD(root, "free buffer header(%p)->buffer(%p)", *ppBufferHdr, (*ppBufferHdr)->pBuffer);
        ret = MagOmxPortImplVirtual(base)->MagOMX_FreeBuffer(base, (*ppBufferHdr)->pBuffer);
        if (ret != OMX_ErrorNone){
            PORT_LOGE(root, "failed to free the pBuffer: %p", (*ppBufferHdr)->pBuffer);
        }
    }

    /*should be freed later than the buffer owner*/
    if (pBufNode->bufferHeaderOwner == root){
        PORT_LOGD(root, "free buffer header(%p)", *ppBufferHdr);
        mag_freep((void **)ppBufferHdr);
        /*list_del(&pBufNode->node);*/
        mag_freep((void **)&pBufNode);
    }
    
    return ret;
}

static void onMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxPort     root;
    MagOmxPortImpl base;
    OMX_BUFFERHEADERTYPE *bufHeader;
    OMX_BUFFERHEADERTYPE *freeBufHeader;
    OMX_ERRORTYPE ret;
    OMX_U32 cmd;
    MagOmxPort_State_t  portSt;
    /*MagOmxPort_State_t  tunneledPortSt = kPort_State_Flushed;*/
    MagOmxComponent     tunneledComp;
    MagOmxComponentImpl tunnelCompImpl = NULL;
    MagOmxPort          tunneledPort;
    OMX_HANDLETYPE      hTunneledPort;

    if (!msg){
        PORT_LOGE(NULL, "msg is NULL!");
        return;
    }
    
    root = getRoot(priv);
    base = getBase(priv);

    if (!msg->findPointer(msg, "buffer_header", (void **)&bufHeader)){
        PORT_LOGE(root, "failed to find the buffer_header!");
        return;
    }

    portSt = root->getState(root);

    if (root->isTunneled(root)){
        tunnelCompImpl = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
        hTunneledPort = tunnelCompImpl->getPort(base->mTunneledComponent, base->mTunneledPortIndex);
        tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);
        /*tunneledPortSt = tunneledPort->getState(tunneledPort);*/
    }
    
    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxPortImpl_EmptyThisBufferMsg:
            if (portSt == kPort_State_Flushed){
                if (/*tunneledPortSt == kPort_State_Flushed*/ tunnelCompImpl && !tunnelCompImpl->isFlushing(tunnelCompImpl)){
                    root->setState(root, kPort_State_Running);
                    portSt = kPort_State_Running;
                    tunneledPort->setState(tunneledPort, kPort_State_Running);
                }else if (tunnelCompImpl == NULL){
                    root->setState(root, kPort_State_Running);
                    portSt = kPort_State_Running;
                }else{
                    PORT_LOGD(root, "EmptyThisBufferMsg: in Flushed state, but the tunneled comp is in flushing state, directly return!");
                }
            }

            if (root->isTunneled(root)  && 
                root->isInputPort(root) && 
                root->isBufferSupplier(root)){
                /*get returned buffer header*/
                if (portSt == kPort_State_Running){
                    /*step #1: send out the next buffer to the tunneled peer*/
                    ret = base->getRunningNode(base, &freeBufHeader);
                    if (ret == OMX_ErrorNone){
                        tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                        ret = MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, freeBufHeader);
                        if (ret != OMX_ErrorNone){
                            PORT_LOGE(root, "EmptyThisBufferMsg: failed to do FillThisBuffer");
                            base->getBuffer(base);
                            base->putRunningNode(base, freeBufHeader);
                        }
                    }else{
                        base->getBuffer(base);
                    }

                    /*step #2: proceed the returned buffer*/
                    /*the buffer header might need to be relayed to next return port OR
                     * directly put the buffer into the output list if the port is the owner
                     */
                    /*base->relayReturnBuffer(base, bufHeader);*/

                    if (MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer){
                        /*Do base->putRunningNode(base, bufHeader) below if it is needed*/
                        MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer(priv, bufHeader);
                    }
                    base->putOutputNode(base, bufHeader);
                }else{
                    /*Directly return the buffer header in flushing or stopped state*/
                    PORT_LOGD(root, "EmptyThisBufferMsg: in %d state, directly put the buffer into the Running buffer list", portSt);
                    base->getBuffer(base);
                    base->putRunningNode(base, bufHeader);
                }
                break;
            }else{
                if (root->isInputPort(root)){
                    if (portSt == kPort_State_Running){
                        base->sendBuffer(base);
                    }else{
                        /*Directly return the buffer header in flushing or stopped state*/
                        PORT_LOGD(root, "EmptyThisBufferMsg: in %d state, directly return buffer", portSt);
                        if (root->isTunneled(root)){
                            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                            MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, bufHeader);
                        }else{
                            MagOmxComponentImpl hCompImpl;

                            hCompImpl = ooc_cast(root->getAttachedComponent(root), MagOmxComponentImpl);
                            hCompImpl->sendEmptyBufferDoneEvent(hCompImpl, bufHeader);
                        }
                        break;
                    }
                }else{
                    PORT_LOGE(root, "Can't do EmptyThisBuffer on the output port. quit!");
                    break;
                }
            }

            printBufSequence(base, bufHeader, __FUNCTION__, __LINE__);
            base->dispatchBuffers(priv, bufHeader);
            break;

        case MagOmxPortImpl_FillThisBufferMsg:
            if (portSt == kPort_State_Flushed){
                if (/*tunneledPortSt == kPort_State_Flushed*/ tunnelCompImpl && !tunnelCompImpl->isFlushing(tunnelCompImpl)){
                    root->setState(root, kPort_State_Running);
                    portSt = kPort_State_Running;
                    tunneledPort->setState(tunneledPort, kPort_State_Running);
                }else if (tunnelCompImpl == NULL){
                    root->setState(root, kPort_State_Running);
                    portSt = kPort_State_Running;
                }else{
                    PORT_LOGD(root, "FillThisBufferMsg: in Flushed state, but the tunneled port is in flushing state, directly return!");
                }
            }

            if (root->isTunneled(root)   &&
                !root->isInputPort(root) && 
                root->isBufferSupplier(root)){
                /*get returned buffer header*/
                if (portSt == kPort_State_Running){
                    /*step #1: send out the next buffer to the tunneled peer*/
                    ret = base->getRunningNode(base, &freeBufHeader);
                    if (ret == OMX_ErrorNone){
                        tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                        ret = MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, freeBufHeader);
                        if (ret != OMX_ErrorNone){
                            PORT_LOGE(root, "FillThisBufferMsg: failed to do EmptyThisBuffer");
                            base->getBuffer(base);
                            base->putOutputNode(base, freeBufHeader);
                        }
                    }else{
                        base->getBuffer(base);
                    }

                    /*step #2: proceed the returned buffer*/
                    /*the buffer header might need to be relayed to next return port OR
                     * directly put the buffer into the output list if the port is the owner
                     */
                    /*base->relayReturnBuffer(base, bufHeader);*/

                    if (MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer){
                        /*Do base->putRunningNode(base, bufHeader) below if it is needed*/
                        MagOmxPortImplVirtual(base)->MagOMX_ProceedReturnedBuffer(priv, bufHeader);
                    }
                    base->putOutputNode(base, bufHeader);
                }else{
                    /*Directly return the buffer header in flushing or stopped state*/
                    PORT_LOGD(root, "FillThisBufferMsg: in %d state, directly put the buffer into the Output buffer list", portSt);
                    base->getBuffer(base);
                    base->putOutputNode(base, bufHeader);
                }
                break;
            }else{
                if (!root->isInputPort(root)){
                    if (portSt == kPort_State_Running){
                        base->sendBuffer(base);
                    }else{
                        PORT_LOGD(root, "FillThisBufferMsg: in %d state, directly return buffer", portSt);
                        /*Directly return the buffer header in flushing or stopped state*/
                        if (root->isTunneled(root)){
                            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                            MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, bufHeader);
                        }else{
                            MagOmxComponentImpl hCompImpl;

                            hCompImpl = ooc_cast(root->getAttachedComponent(root), MagOmxComponentImpl);
                            hCompImpl->sendFillBufferDoneEvent(hCompImpl, bufHeader);
                        }
                        break;
                    }
                }else{
                    PORT_LOGE(root, "Can't do FillThisBuffer on the input port. quit!");
                    break;
                } 
            }

            printBufSequence(base, bufHeader, __FUNCTION__, __LINE__);
            base->dispatchBuffers(priv, bufHeader);
            break;

        case MagOmxPortImpl_ReturnThisBufferMsg:
            PORT_LOGV(root, "MagOmxPortImpl_ReturnThisBufferMsg: buffer header[%p], buf[%p]", bufHeader, bufHeader->pBuffer);
            base->getBuffer(base);

            if (root->isTunneled(root) &&
                root->isBufferSupplier(root)){
                if (root->isInputPort(root))
                    base->putRunningNode(base, bufHeader);
                else
                    base->putOutputNode(base, bufHeader);
            }else{
                if (root->isTunneled(root)){
                    tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                    if (root->isInputPort(root)){
                        PORT_LOGV(root, "returned buffer, call FillThisBuffer(%p)", bufHeader);
                        bufHeader->nOutputPortIndex = base->mTunneledPortIndex;
                        MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, bufHeader);
                    }else{
                        PORT_LOGV(root, "returned buffer, call EmptyThisBuffer(%p)", bufHeader);
                        bufHeader->nInputPortIndex = base->mTunneledPortIndex;
                        MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, bufHeader);
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
                }
            }

            printBufSequence(base, bufHeader, __FUNCTION__, __LINE__);
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
            PORT_LOGV(root, "MagOmxPortImpl_OutputBufferMsg: buffer header %p[%p]", bufHeader, bufHeader->pBuffer);
            if (bufHeader){
                if (root->isTunneled(root)){
                    if (root->isBufferSupplier(root)){
                        if (portSt == kPort_State_Running ||
                            portSt == kPort_State_Flushed){
                            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponent);
                            if (!root->isInputPort(root)){
                                ret = MagOmxComponentVirtual(tunneledComp)->EmptyThisBuffer(tunneledComp, bufHeader);
                                if (ret != OMX_ErrorNone){
                                    PORT_LOGE(root, "OutputBufferMsg: failed to do EmptyThisBuffer");
                                    base->putOutputNode(base, bufHeader);
                                }else{
                                    base->sendBuffer(base);
                                }
                            }else{
                                ret = MagOmxComponentVirtual(tunneledComp)->FillThisBuffer(tunneledComp, bufHeader);
                                if (ret != OMX_ErrorNone){
                                    PORT_LOGE(root, "OutputBufferMsg: failed to do FillThisBuffer");
                                    base->putRunningNode(base, bufHeader);
                                }else{
                                    base->sendBuffer(base);
                                }
                            }
                        }else{
                            if (!root->isInputPort(root)){
                                PORT_LOGD(root, "OutputBufferMsg: in %d state, directly place it to OutPutBuffer", portSt);
                                base->putOutputNode(base, bufHeader);
                            }else{
                                PORT_LOGD(root, "OutputBufferMsg: in %d state, directly place it to RunningBuffer", portSt);
                                base->putRunningNode(base, bufHeader);
                            }
                        }
                    }else{
                        /*base->putRunningNode(base, bufHeader);*/
                        PORT_LOGE(root, "[OutputBufferMsg]: Should not be here!!");
                    }
                }else{/*non-tunnelled*/
                    MagOmxComponentImpl hCompImpl;

                    hCompImpl = ooc_cast(root->getAttachedComponent(root), MagOmxComponentImpl);
                    base->getBuffer(base);

                    if (!root->isInputPort(root)){
                        hCompImpl->sendFillBufferDoneEvent(hCompImpl, bufHeader);
                    }else{
                        hCompImpl->sendEmptyBufferDoneEvent(hCompImpl, bufHeader);
                    }
                }
            }
            printBufSequence(base, bufHeader, __FUNCTION__, __LINE__);
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
                    MagOmxPortVirtual(root)->FreeBuffer(hPort, &pBufferHeader);
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
    root->setState(root, kPort_State_Running);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Flush(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    MagOmxComponentImpl tunnelComp;
    OMX_HANDLETYPE      hTunneledPort;
    MagOmxPort          tunneledPort;
    MagOmxPortImpl      tunneledPortBase;

    OMX_BUFFERHEADERTYPE *buffer;
    OMX_S32 diff;

    root = getRoot(hPort);
    base = getBase(hPort);

    
    if (base->mBuffersTotal == 0){
        PORT_LOGD(root, "No buffers allocated, don't need to flush the port!");
        return OMX_ErrorNone;
    }

    PORT_LOGD(root, "Flushing(%d - %d)", base->mFreeBuffersNum, base->mBuffersTotal);

    if (root->isTunneled(root)){
        Mag_AcquireMutex(base->mhTunnelFlushMutex);
        /*if (root->getState(root) == kPort_State_Flushed){
            PORT_LOGD(root, "In kPort_State_Flushed state and return!");
            Mag_ReleaseMutex(base->mhTunnelFlushMutex);
            return OMX_ErrorNone;
        }*/

        root->setState(root, kPort_State_Flushing);

        tunnelComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
        hTunneledPort = tunnelComp->getPort(tunnelComp, base->mTunneledPortIndex);

        if (hTunneledPort == NULL){
            PORT_LOGE(root, "Failed to get the port with index: %d", base->mTunneledPortIndex);
            Mag_ReleaseMutex(base->mhTunnelFlushMutex);
            return OMX_ErrorBadPortIndex;
        }
        tunneledPort     = ooc_cast(hTunneledPort, MagOmxPort);
        tunneledPortBase = ooc_cast(hTunneledPort, MagOmxPortImpl);

        /*need to flush the buffer-supplier port firstly and then do it in non-buffer-supplier port*/
        if (root->isBufferSupplier(root)){
            if (base->mLooper){
                base->mLooper->waitOnAllDone(base->mLooper);
                PORT_LOGD(root, "To flush buffer-supplier port looper done");
            }

            if (tunneledPort->getState(tunneledPort) == kPort_State_Running){
                MagOmxPortVirtual(tunneledPort)->Flush(hTunneledPort);
            }else{
                if (tunneledPortBase->mLooper){
                    tunneledPortBase->mLooper->waitOnAllDone(tunneledPortBase->mLooper);
                    PORT_LOGD(root, "To flush non-buffer-supplier port looper done");
                }
            }
        }else{
            if (tunneledPort->getState(tunneledPort) == kPort_State_Running){
                MagOmxPortVirtual(tunneledPort)->Flush(hTunneledPort);
            }else{
                if (tunneledPortBase->mLooper){
                    tunneledPortBase->mLooper->waitOnAllDone(tunneledPortBase->mLooper);
                    PORT_LOGD(root, "To flush buffer-supplier port looper done");
                }
            }

            if (base->mLooper){
                base->mLooper->waitOnAllDone(base->mLooper);
                PORT_LOGD(root, "To flush non-buffer-supplier port looper done");
            }
        }

        Mag_AcquireMutex(base->mhMutex);

        diff = base->mBuffersTotal - base->mFreeBuffersNum;
        if (diff > 0){
            PORT_LOGD(root, "Wait on all buffers back[%d - %d]", base->mFreeBuffersNum, base->mBuffersTotal);
            base->mWaitOnAllBuffers = OMX_TRUE;
            Mag_ClearEvent(base->mAllBufReturnedEvent);
            Mag_ReleaseMutex(base->mhMutex);

            Mag_WaitForEventGroup(base->mBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            PORT_LOGD(root, "get the event mAllBufReturnedEvent!");
        }else if(diff < 0){
            Mag_ReleaseMutex(base->mhMutex);
            Mag_ReleaseMutex(base->mhTunnelFlushMutex);
            PORT_LOGE(root, "wrong buffer counts: mFreeBuffersNum(%d) - mBuffersTotal(%d)",
                            base->mFreeBuffersNum, base->mBuffersTotal);
            return OMX_ErrorUndefined;
        }else{
            Mag_ReleaseMutex(base->mhMutex);
        }

        root->setState(root, kPort_State_Flushed);
        /*tunneledPort->setState(tunneledPort, kPort_State_Flushed);*/

        Mag_ReleaseMutex(base->mhTunnelFlushMutex);
    }else{
        root->setState(root, kPort_State_Flushing);

        if (base->mLooper){
            base->mLooper->waitOnAllDone(base->mLooper);
            PORT_LOGD(root, "To flush port looper done");
        }

        if (!root->isInputPort(root)){
            OMX_BUFFERHEADERTYPE *pBufHeader;

            /*send out all buffers back to APP*/
            while (base->getOutputNode(base, &pBufHeader, OMX_FALSE) == OMX_ErrorNone){
                MagOmxPortVirtual(root)->sendOutputBufferToAPP(root, pBufHeader);
            }
        }

        Mag_AcquireMutex(base->mhMutex);
        diff = base->mBuffersTotal - base->mFreeBuffersNum;
        if (diff > 0){
            PORT_LOGD(root, "Wait on all buffers back[%d - %d]", base->mFreeBuffersNum, base->mBuffersTotal);
            base->mWaitOnAllBuffers = OMX_TRUE;
            Mag_ClearEvent(base->mAllBufReturnedEvent);
            Mag_ReleaseMutex(base->mhMutex);

            Mag_WaitForEventGroup(base->mBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            PORT_LOGD(root, "get the event mAllBufReturnedEvent!");
        }else if(diff < 0){
            Mag_ReleaseMutex(base->mhMutex);
            PORT_LOGE(root, "wrong buffer counts: mFreeBuffersNum(%d) - mBuffersTotal(%d)",
                            base->mFreeBuffersNum, base->mBuffersTotal);
            return OMX_ErrorUndefined;
        }else{
            Mag_ReleaseMutex(base->mhMutex);
        }
        root->setState(root, kPort_State_Flushed);
    }

    PORT_LOGD(root, "exit Flushing");
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Pause(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    PORT_LOGV(root, "enter!");
    
    if (base->mLooper)
        base->mLooper->suspend(base->mLooper);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Resume(OMX_HANDLETYPE hPort){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    PORT_LOGV(root, "enter!");

    if (base->mLooper)
        base->mLooper->resume(base->mLooper);
    
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
                    bufNode = (MagOMX_Port_Buffer_t *)bufferHeader->pPlatformPrivate;
                    bufNode->bufferOwner = root;

                    Mag_AcquireMutex(base->mhMutex);
                    /*list_add_tail(&bufNode->node, &base->mBufferList);*/
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
            }else{
                PORT_LOGE(root, "Failed to do MagOMX_AllocateBuffer(), index %d", i);
            }
        }

        if (i == root->getDef_BufferCountActual(root)){
            base->mFreeBuffersNum = i;
            base->mBuffersTotal   = i;
            PORT_LOGD(root, "buffer populates(%d)", base->mBuffersTotal);
            root->setDef_Populated(root, OMX_TRUE);
        }
    }else{
        /*none buffer supplier port*/
        tunneledPort->setParameter(hTunneledPort, 
                                  OMX_IndexConfigTunneledPortStatus, 
                                  OMX_PORTSTATUS_ACCEPTUSEBUFFER);

        MagOmxPortVirtual(tunneledPort)->SendEvent(hTunneledPort, kTunneledPortStatusEvt);

        if ((root->getDef_BufferCountActual(root) > 0) &&
            !root->getDef_Populated(root)){
            PORT_LOGD(root, "before waiting on the buffer population!");
            Mag_WaitForEventGroup(base->mBufPopulatedEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            PORT_LOGD(root, "buffer is populated!");
        }else{
            Mag_ClearEvent(base->mBufPopulatedEvt);
            root->setDef_Populated(root, OMX_TRUE);
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
                  OMX_BUFFERHEADERTYPE **ppBuffer){

    OMX_ERRORTYPE ret;
    
    ret = freeBufferInternal(hPort, ppBuffer);
    return ret;
}

static OMX_ERRORTYPE virtual_FreeAllBuffers(OMX_HANDLETYPE hPort){
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
        MagOmxPortVirtual(Port)->FreeBuffer(hPort, &pBufferHeader);
        pNode = PortImpl->mBufferList.next;
    }
    PortImpl->mBuffersTotal = 0;
    Mag_ReleaseMutex(PortImpl->mhMutex);

    Port->setDef_Populated(Port, OMX_FALSE);
    Port->setState(Port, kPort_State_Stopped);

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

    if (!root->isTunneled(root) || !root->getDef_Populated(root)){
        PORT_LOGD(root, "the port is not tunneled or unpopulated");
        return OMX_ErrorPortsNotConnected;
    }

    tunnelComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl); 
    hTunneledPort = tunnelComp->getPort(tunnelComp, base->mTunneledPortIndex);
    if (hTunneledPort == NULL){
        PORT_LOGE(root, "failed to get the port with index: %d", base->mTunneledPortIndex);
        return OMX_ErrorBadPortIndex;
    }
    tunneledPort = ooc_cast(hTunneledPort, MagOmxPort);

    if (root->isBufferSupplier(root))
        FreeTunnelBufferInternal(hPort, hTunneledPort);
    else
        FreeTunnelBufferInternal(hTunneledPort, hPort);

    
    tunneledPort->setDef_Populated(tunneledPort, OMX_FALSE);
    tunneledPort->setState(tunneledPort, kPort_State_Stopped);

    root->setDef_Populated(root, OMX_FALSE);
    root->setState(root, kPort_State_Stopped);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_EmptyThisBuffer(
                  OMX_HANDLETYPE hPort,
                  OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxPort     root;
    MagOmxPortImpl base;

    root = getRoot(hPort);
    base = getBase(hPort);

    if (root->getState(root) == kPort_State_Stopped){
        PORT_LOGE(root, "the port is stopped, ignore the EmptyThisBuffer call");
        return OMX_ErrorIncorrectStateOperation;
    }
    
    if (root->isInputPort(root)){
        if (base->mEmptyThisBufferMsg == NULL){
            base->mEmptyThisBufferMsg = base->createMessage(hPort, MagOmxPortImpl_EmptyThisBufferMsg);
        }

        base->mEmptyThisBufferMsg->setPointer(base->mEmptyThisBufferMsg, "buffer_header", (void *)pBuffer, MAG_FALSE);
        base->mEmptyThisBufferMsg->postMessage(base->mEmptyThisBufferMsg, 0);
        printBufSequence(base, pBuffer, __FUNCTION__, __LINE__);
        PORT_LOGV(root, "post the EmptyThisBuffer message(buffer: %p(%p))", pBuffer, pBuffer->pBuffer);
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

    if (root->getState(root) == kPort_State_Stopped){
        PORT_LOGE(root, "the port is stopped, ignore the FillThisBuffer call");
        return OMX_ErrorIncorrectStateOperation;
    }

    if (!root->isInputPort(root)){
        if (base->mFillThisBufferMsg == NULL){
            base->mFillThisBufferMsg = base->createMessage(hPort, MagOmxPortImpl_FillThisBufferMsg);
            PORT_LOGD(root, "create mFillThisBufferMsg: %p", base->mFillThisBufferMsg);
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
#if 0
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
#endif
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

            /*Request the tunneled component to do tearing down action accordingly*/
            tunneledComp = ooc_cast(base->mTunneledComponent, MagOmxComponentImpl);
            if (MagOmxComponentImplVirtual(tunneledComp)->MagOMX_TearDownTunnel){
                MagOmxComponentImplVirtual(tunneledComp)->MagOMX_TearDownTunnel(tunneledComp, base->mTunneledPortIndex);
            }else{
                PORT_LOGD(root, "The tunneled component[%p] doesn't have the MagOMX_TearDownTunnel overrided!",
                                 tunneledComp);
            }

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
            PORT_LOGE(root, "The port is tunneled and unable to tunnel again before it is torn down!");
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
        OMX_PARAM_BUFFERSUPPLIERTYPE tunneled_supplier;

        PORT_LOGD(root, "it is input port!");
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
        PORT_LOGI(root, "Negotiated buffer supplier: %s", 
                    pTunnelSetup->eSupplier == OMX_BufferSupplyInput ? "input port": "output port");

        /*check buffer count*/
        if (inPortDef.nBufferCountActual != outPortDef.nBufferCountActual){
            /*always uses the less buffer count to set up both port of the tunnel*/
            if (inPortDef.nBufferCountActual > outPortDef.nBufferCountActual){
                inPortDef.nBufferCountActual = outPortDef.nBufferCountActual;
            }else{
                outPortDef.nBufferCountActual = inPortDef.nBufferCountActual;
            }
            tunneledPort->setPortDefinition(tunneledPort, &outPortDef);
            root->setPortDefinition(root, &inPortDef);
        }

        base->mTunneledComponent = hTunneledComp;
        base->mTunneledPortIndex = nTunneledPortIndex;
        root->setTunneledFlag(root, OMX_TRUE);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_RegisterBufferHandler(OMX_HANDLETYPE hPort,
                                            MagMessageHandle pBufferHandler){
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
    pNode->msg     = pBufferHandler;
    list_add_tail(&pNode->node, &base->mBufDispatcherList);
    PORT_LOGD(root, "port index: %d", root->getPortIndex(root));

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

static MagMessageHandle virtual_GetSharedBufferMsg(OMX_HANDLETYPE hPort){
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

/* Get the output buffer that holds the generated data from the Component
 * It is Block Function
*/
static OMX_BUFFERHEADERTYPE* virtual_GetOutputBuffer(OMX_HANDLETYPE hPort){
    MagOmxPortImpl hPortImpl = NULL;
    OMX_BUFFERHEADERTYPE *pBufHeader;

    hPortImpl = ooc_cast(hPort, MagOmxPortImpl);
    hPortImpl->getOutputNode(hPortImpl, &pBufHeader, OMX_TRUE);
    return pBufHeader;
}

static void virtual_PutOutputBuffer(OMX_HANDLETYPE hPort, OMX_BUFFERHEADERTYPE* pBuf){
    MagOmxPortImpl hPortImpl = NULL;

    hPortImpl = ooc_cast(hPort, MagOmxPortImpl);
    hPortImpl->putOutputNode(hPortImpl, pBuf);
}

static void virtual_sendOutputBufferToAPP(OMX_HANDLETYPE hPort, OMX_BUFFERHEADERTYPE* pBufHeader){
    MagOmxComponentImpl hCompImpl;
    MagOmxPort     root;
    MagOmxPortImpl base;
    
    root = getRoot(hPort);
    base = getBase(hPort);

    hCompImpl = ooc_cast(root->getAttachedComponent(root), MagOmxComponentImpl);

    base->getBuffer(base);

    if (!root->isInputPort(root)){
        hCompImpl->sendFillBufferDoneEvent(hCompImpl, pBufHeader);
    }else{
        hCompImpl->sendEmptyBufferDoneEvent(hCompImpl, pBufHeader);
    }
}

static OMX_ERRORTYPE virtual_putReturnBuffer(OMX_HANDLETYPE hPort, 
                                             OMX_BUFFERHEADERTYPE* pBuffer, 
                                             MagMessageHandle msg, 
                                             OMX_PTR priv){
    List_t *freeList;
    List_t *returnList;
    MagOMX_Pending_Buffer_Node_t *node;
    MagOmxPortImpl portImpl;

    portImpl = ooc_cast(hPort, MagOmxPortImpl);

    freeList   = &portImpl->mFreePendingBufListH;
    returnList = &portImpl->mPendingReturnBufListH;

    Mag_AcquireMutex(portImpl->mhRtnBufListMutex);
    if (freeList->next == freeList){
        /*free list is empty*/
        node = (MagOMX_Pending_Buffer_Node_t *)mag_mallocz(sizeof(MagOMX_Pending_Buffer_Node_t));
        INIT_LIST(&node->node);
    }else{
        node = (MagOMX_Pending_Buffer_Node_t *)list_entry(freeList->next, MagOMX_Pending_Buffer_Node_t, node);
        list_del(&node->node);
    }

    node->msg     = msg;
    node->priv    = priv;
    node->pBuffer = pBuffer;

    list_add_tail(&node->node, returnList);
    /*check return buffers*/
    /*{
        List_t *tmp;
        OMX_S32 i = 0;

        tmp = returnList->next;
        while (tmp != returnList){
            i++;
            tmp = tmp->next;
        }
        PORT_LOGD(getRoot(hPort), "return buffers: %d", i);
    }*/
    Mag_ReleaseMutex(portImpl->mhRtnBufListMutex);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_sendReturnBuffer(OMX_HANDLETYPE hPort, 
                                              OMX_BUFFERHEADERTYPE* pBuffer){
    List_t *freeList;
    List_t *returnList;
    List_t *expected;
    List_t *tmp;
    MagOMX_Pending_Buffer_Node_t *node = NULL;
    OMX_U32 i = 1;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    MagOmxPortImpl portImpl;

    portImpl = ooc_cast(hPort, MagOmxPortImpl);

    freeList   = &portImpl->mFreePendingBufListH;
    returnList = &portImpl->mPendingReturnBufListH;

    Mag_AcquireMutex(portImpl->mhRtnBufListMutex);

    expected = returnList->next;
    if (expected != returnList){
        node = (MagOMX_Pending_Buffer_Node_t *)list_entry(expected, MagOMX_Pending_Buffer_Node_t, node);
        
        if (node->pBuffer == pBuffer){
            list_del(&node->node);
            list_add_tail(&node->node, freeList);
        }else{
            PORT_LOGE(getRoot(portImpl), "the expected buffer [%p] is not the first one in the return buffer list",
                                       pBuffer);
            tmp = expected->next;
            while (tmp != returnList){
                i++;
                node = (MagOMX_Pending_Buffer_Node_t *)list_entry(tmp, MagOMX_Pending_Buffer_Node_t, node);
                if (node->pBuffer == pBuffer){
                    list_del(&node->node);
                    list_add_tail(&node->node, freeList);
                    PORT_LOGD(getRoot(portImpl), "find the return buffer[%p] at place %d", pBuffer, i);
                    break;
                }
                tmp = tmp->next;
            }

            if (tmp == returnList){
                PORT_LOGE(getRoot(portImpl), "Failed to find out the buffer[%p] in the return buffer list!", pBuffer);
                ret = OMX_ErrorNoMore;
            }
        }
    }else{
        PORT_LOGE(getRoot(portImpl), "Error happens, the return buffer list is empty!");
        ret = OMX_ErrorNoMore;
    }

    /*check the remaining return buffers*/
    /*{
        List_t *tmp;
        OMX_S32 i = 0;

        tmp = returnList->next;
        while (tmp != returnList){
            i++;
            tmp = tmp->next;
        }
        PORT_LOGD(getRoot(hPort), "remaining return buffers: %d", i);
    }*/
    
    if (ret == OMX_ErrorNone){
        node->msg->setPointer(node->msg, "buffer_header", pBuffer, MAG_FALSE);
        node->msg->setPointer(node->msg, "component_obj", node->priv, MAG_FALSE);
        node->msg->postMessage(node->msg, 0);
    }

    Mag_ReleaseMutex(portImpl->mhRtnBufListMutex);

    return ret;
}

/*Attach the input buffer header to the output message and post it*/
static OMX_ERRORTYPE virtual_sendOutputBuffer(OMX_HANDLETYPE hPort, 
                                              OMX_BUFFERHEADERTYPE* pBuffer){
    MagOmxPortImpl portImpl;

    portImpl = ooc_cast(hPort, MagOmxPortImpl);

    if (pBuffer == NULL){
        AGILE_LOGE("Input pBuffer is NULL, quit!");
        return OMX_ErrorBadParameter;
    }

    if (portImpl->mOutputBufferMsg == NULL){
        portImpl->mOutputBufferMsg = portImpl->createMessage(portImpl, MagOmxPortImpl_OutputBufferMsg);
    }

    portImpl->mOutputBufferMsg->setPointer(portImpl->mOutputBufferMsg, "buffer_header", (void *)pBuffer, MAG_FALSE);

    PORT_LOGV(getRoot(portImpl), "send output buffer: %p[%p]", pBuffer, pBuffer->pBuffer);
    portImpl->mOutputBufferMsg->postMessage(portImpl->mOutputBufferMsg, 0);

    return OMX_ErrorNone;
}

#if 0
/*called from MagOMX_ProceedBuffer() to provide the destinated/source buffer*/
static MagMessageHandle virtual_GetOutputBufferMsg(OMX_HANDLETYPE hPort){
    MagOmxPortImpl hPortImpl = NULL;
    OMX_BUFFERHEADERTYPE *pBufHeader;
    OMX_ERRORTYPE ret;

    if (NULL == hPort){
        return NULL;
    }

    hPortImpl = ooc_cast(hPort, MagOmxPortImpl);

    hPortImpl->getOutputNode(hPortImpl, &pBufHeader, OMX_TRUE);
    
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
#endif

/*Member functions*/
static MagMessageHandle MagOmxPortImpl_createMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxPortImpl hPort = NULL;
    MagMessageHandle msg;
    
    if (NULL == handle){
        return NULL;
    }
    
    hPort = ooc_cast(handle, MagOmxPortImpl);
        
    hPort->getLooper(handle);
    
    msg = createMagMessage(hPort->mLooper, what, hPort->mMsgHandler->id(hPort->mMsgHandler));
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

    if (pNode == &port->mBufDispatcherList){
        PORT_LOGD(portRoot, "no dispatcher message is registered!");
        if (!portRoot->isTunneled(portRoot)){
            /*put to output buffer list for filling data and sending out to APP*/
            PORT_LOGD(portRoot, "put buffer into output list! mFreeBuffersNum: %d", port->mFreeBuffersNum);
            port->putOutputNode(port, bufHeader);
        }
        return;
    }

    while (pNode != &port->mBufDispatcherList){
        item = (BufferDispatcherNode_t *)list_entry(pNode, BufferDispatcherNode_t, node);
        hMsg = item->msg;
        if (port->mReturnThisBufferMsg == NULL){
            port->mReturnThisBufferMsg = port->createMessage(hPort, MagOmxPortImpl_ReturnThisBufferMsg);
        }

        hMsg->setMessage(hMsg, "return_buf_msg", port->mReturnThisBufferMsg, MAG_FALSE);
        hMsg->setPointer(hMsg, "buffer_header", bufHeader, MAG_FALSE);
        PORT_LOGD(portRoot, "dispatch buffer header: %p, buffer: %p with msg: %p", 
                             bufHeader, bufHeader->pBuffer, hMsg);
        hMsg->postMessage(hMsg, 0);
        pNode = pNode->next;
    }
}

static MagOMX_Port_Buffer_t *MagOmxPortImpl_allocBufferNode(OMX_BUFFERHEADERTYPE* pBuffer){
    MagOMX_Port_Buffer_t *pBufNode;

    pBufNode = (MagOMX_Port_Buffer_t *)mag_mallocz(sizeof(MagOMX_Port_Buffer_t));
    MAG_ASSERT(pBufNode);

    INIT_LIST(&pBufNode->node);
    INIT_LIST(&pBufNode->runNode);
    INIT_LIST(&pBufNode->returnBufPortListH);
    INIT_LIST(&pBufNode->outputBufPortListH);
    INIT_LIST(&pBufNode->freeBufPortListH);

    pBufNode->bufferHeaderOwner = NULL;
    pBufNode->bufferOwner       = NULL;
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

    /*INIT_LIST(&item->runNode);*/
    portRoot = ooc_cast(hPort, MagOmxPort);

    Mag_AcquireMutex(hPort->mhMutex);
    if ((item->runNode.next != &item->runNode) ||
        (item->runNode.prev != &item->runNode)){
        PORT_LOGE(portRoot, "the MagOMX_Port_Buffer_t node is still in the list and should not be added into another list!");
        MAG_ASSERT(0);
    }

    list_add_tail(&item->runNode, &hPort->mRunningBufferList);
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

    Mag_AcquireMutex(hPort->mhMutex);
    if ((item->runNode.next != &item->runNode) ||
        (item->runNode.prev != &item->runNode)){
        PORT_LOGE(portRoot, "the MagOMX_Port_Buffer_t node is still in the list and should not be added into another list!");
        MAG_ASSERT(0);
    }

    if (portRoot->isTunneled(portRoot))
        hPort->resetBuffer(hPort, pBuffer);

    list_add_tail(&item->runNode, &hPort->mOutputBufferList);

    if (hPort->mWaitOnOutputBuffer){
        PORT_LOGD(portRoot, "The empty output buffer list get the new node added!");
        Mag_SetEvent(hPort->mGetOutputBufferEvent); 
        hPort->mWaitOnOutputBuffer = OMX_FALSE;
    }

    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}

/*only called by buffer supplier port*/
static OMX_ERRORTYPE MagOmxPortImpl_getOutputNode(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer, OMX_BOOL block){
    List_t *pNode;
    MagOMX_Port_Buffer_t *item;
    MagOmxPort portRoot = NULL;

    *ppBuffer = NULL;
    portRoot = ooc_cast(hPort, MagOmxPort);
    
    if (portRoot->getState(portRoot) == kPort_State_Stopped){
        return OMX_ErrorIncorrectStateOperation;
    }

retry:
    Mag_AcquireMutex(hPort->mhMutex);
    pNode = hPort->mOutputBufferList.next;
    if (pNode != &hPort->mOutputBufferList){
        item = (MagOMX_Port_Buffer_t *)list_entry(pNode, MagOMX_Port_Buffer_t, runNode);
        list_del(pNode);
        *ppBuffer = item->pOmxBufferHeader;
    }else{
        if (block){
            hPort->mWaitOnOutputBuffer = OMX_TRUE;
            Mag_ClearEvent(hPort->mGetOutputBufferEvent);
            PORT_LOGD(getRoot(hPort), "No buffer is in the output list, wait on getting new output buffer!");
            Mag_ReleaseMutex(hPort->mhMutex);
                
            Mag_WaitForEventGroup(hPort->mOutputBufferEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            PORT_LOGD(getRoot(hPort), "get an output buffer!");
            goto retry;
        }else{
            Mag_ReleaseMutex(hPort->mhMutex);
            PORT_LOGD(getRoot(hPort), "Don't get buffer from the output list!");
            return OMX_ErrorUndefined;
        }
    }
    Mag_ReleaseMutex(hPort->mhMutex);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE MagOmxPortImpl_relayReturnBuffer(MagOmxPortImpl hPort, 
                                                      OMX_BUFFERHEADERTYPE* pBuffer){
    List_t *freeList;
    List_t *returnList;
    MagOmx_Port_Node_t *node;
    MagOMX_Port_Buffer_t *portBufMgr;
    MagOmxPort hSrcPort = NULL;

    portBufMgr = (MagOMX_Port_Buffer_t *)pBuffer->pPlatformPrivate;

    freeList   = &portBufMgr->freeBufPortListH;
    returnList = &portBufMgr->returnBufPortListH;

    if (returnList->prev != returnList){
        node = (MagOmx_Port_Node_t *)list_entry(returnList->prev, MagOmx_Port_Node_t, node);
        list_del(&node->node);
        hSrcPort = ooc_cast(node->hPort, MagOmxPort);
        list_add_tail(&node->node, freeList);
        return MagOmxPortVirtual(hSrcPort)->sendReturnBuffer(hSrcPort, pBuffer);
    }else{
        PORT_LOGD(getRoot(hPort), "No need to return the buffer!");
    }

    return OMX_ErrorNone;
}

static void MagOmxPortImpl_resetBuffer(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer){
    MagOMX_Port_Buffer_t *portBufMgr;
    List_t *outputList;
    List_t *returnList;

    portBufMgr = (MagOMX_Port_Buffer_t *)pBuffer->pPlatformPrivate;

    outputList = &portBufMgr->outputBufPortListH;
    returnList = &portBufMgr->returnBufPortListH;

    if(outputList->next != outputList){
        PORT_LOGE(getRoot(hPort), "the output buffer list is not empty!");
    }

    if(returnList->next != returnList){
        PORT_LOGE(getRoot(hPort), "the return buffer list is not empty!");
    }

    INIT_LIST(&portBufMgr->outputBufPortListH);
    INIT_LIST(&portBufMgr->returnBufPortListH);

    pBuffer->pBuffer    = NULL;
    pBuffer->nFilledLen = 0;
    pBuffer->nOffset    = 0;
    pBuffer->nTimeStamp = kInvalidTimeStamp;
}

static void MagOmxPortImpl_sendBuffer(MagOmxPortImpl hPort){
    Mag_AcquireMutex(hPort->mhMutex);
    hPort->mFreeBuffersNum--;
    Mag_ReleaseMutex(hPort->mhMutex);
}

static void MagOmxPortImpl_getBuffer(MagOmxPortImpl hPort){
    MagOmxPort root;

    root = ooc_cast(hPort, MagOmxPort);

    Mag_AcquireMutex(hPort->mhMutex);
    if ((hPort->mFreeBuffersNum < 0) || (hPort->mFreeBuffersNum > hPort->mBuffersTotal)){
        PORT_LOGE(root, "ERROR: free buffer number(%d) > total buffer number(%d)",
                             hPort->mFreeBuffersNum, hPort->mBuffersTotal);
        MAG_ASSERT(0);
    }

    hPort->mFreeBuffersNum++;
    PORT_LOGD(root, "the free buffers are %d, total: %d, mWaitOnAllBuffers: %d", 
                     hPort->mFreeBuffersNum, hPort->mBuffersTotal, hPort->mWaitOnAllBuffers);
    
    if (hPort->mWaitOnAllBuffers){
        if (hPort->mFreeBuffersNum == hPort->mBuffersTotal){
            PORT_LOGD(root, "All buffers are returned!");
            Mag_SetEvent(hPort->mAllBufReturnedEvent); 
            hPort->mWaitOnAllBuffers = OMX_FALSE;
        }
    }
    Mag_ReleaseMutex(hPort->mhMutex);
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
    MagOmxPortImplVtableInstance.MagOmxPort.GetOutputBuffer       = virtual_GetOutputBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.PutOutputBuffer       = virtual_PutOutputBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.putReturnBuffer       = virtual_putReturnBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.sendReturnBuffer      = virtual_sendReturnBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.sendOutputBuffer      = virtual_sendOutputBuffer;
    MagOmxPortImplVtableInstance.MagOmxPort.sendOutputBufferToAPP = virtual_sendOutputBufferToAPP;

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
    thiz->relayReturnBuffer  = MagOmxPortImpl_relayReturnBuffer;
    thiz->resetBuffer        = MagOmxPortImpl_resetBuffer;
    thiz->sendBuffer         = MagOmxPortImpl_sendBuffer;
    thiz->getBuffer          = MagOmxPortImpl_getBuffer;

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhTunnelFlushMutex);
    Mag_CreateMutex(&thiz->mhRtnBufListMutex);

    INIT_LIST(&thiz->mBufferList);
    INIT_LIST(&thiz->mRunningBufferList);
    INIT_LIST(&thiz->mOutputBufferList);
    INIT_LIST(&thiz->mBufDispatcherList);

    INIT_LIST(&thiz->mPendingReturnBufListH);
    INIT_LIST(&thiz->mFreePendingBufListH);

    thiz->mBuffersTotal        = 0;
    thiz->mFreeBuffersNum      = 0;
    thiz->mWaitOnAllBuffers    = OMX_FALSE;
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
    MagOmxComponentImpl tunnelComp;
    OMX_HANDLETYPE      hTunneledPort;
    MagOmxPort          tunneledPort;
    MagOmxPortImpl      tunneledPortImpl;

    PORT_LOGV(getRoot(thiz), "Enter!");

    if (thiz->mLooper){
        thiz->mLooper->clear(thiz->mLooper);
        thiz->mLooper->waitOnAllDone(thiz->mLooper);
    }

    destroyMagMessage(&thiz->mEmptyThisBufferMsg);
    PORT_LOGV(getRoot(thiz), "mEmptyThisBufferMsg: %p", thiz->mEmptyThisBufferMsg);
    destroyMagMessage(&thiz->mFillThisBufferMsg);
    PORT_LOGV(getRoot(thiz), "mFillThisBufferMsg: %p", thiz->mFillThisBufferMsg);
    destroyMagMessage(&thiz->mReturnThisBufferMsg);
    destroyMagMessage(&thiz->mSharedBufferMsg);
    destroyMagMessage(&thiz->mOutputBufferMsg);
    Mag_DestroyEvent(&thiz->mAllBufReturnedEvent);
    Mag_DestroyEventGroup(&thiz->mBufferEventGroup);
    Mag_DestroyEvent(&thiz->mGetOutputBufferEvent);
    Mag_DestroyEventGroup(&thiz->mOutputBufferEventGroup);
    Mag_DestroyEvent(&thiz->mTunneledBufStEvt);
    Mag_DestroyEventGroup(&thiz->mTunneledBufStEvtGrp);
    Mag_DestroyEvent(&thiz->mBufPopulatedEvt);
    Mag_DestroyEventGroup(&thiz->mBufPopulatedEvtGrp);

    Mag_DestroyMutex(&thiz->mhMutex);
    Mag_DestroyMutex(&thiz->mhTunnelFlushMutex);
    Mag_DestroyMutex(&thiz->mhRtnBufListMutex);

    destroyLooper(&thiz->mLooper);
    destroyHandler(&thiz->mMsgHandler);

    PORT_LOGV(getRoot(thiz), "exit!");
}


