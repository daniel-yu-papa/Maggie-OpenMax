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

#ifndef __MAG_OMX_PORT_BASE_IMPL_H__
#define __MAG_OMX_PORT_BASE_IMPL_H__

#include "MagOMX_Port_base.h"

#define OMX_BUFFERFLAG_EXT_FREE   0x01000000
#define OMX_BUFFERFLAG_EXT_USING  0x02000000

enum{
    MagOmxPortImpl_EmptyThisBufferMsg,
    MagOmxPortImpl_FillThisBufferMsg,
    MagOmxPortImpl_ReturnThisBufferMsg,
    MagOmxPortImpl_SharedBufferMsg,
    MagOmxPortImpl_OutputBufferMsg
};

typedef struct{
    List_t node;
    MagMessageHandle msg;
}BufferDispatcherNode_t;

DeclareClass(MagOmxPortImpl, MagOmxPort);

Virtuals(MagOmxPortImpl, MagOmxPort) 
    /*pure virtual functions and must be overrided by sub-component*/
    OMX_ERRORTYPE (*MagOMX_AllocateBuffer)(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes);
    OMX_ERRORTYPE (*MagOMX_FreeBuffer)(OMX_HANDLETYPE port, OMX_U8 *pBuffer);
    OMX_ERRORTYPE (*MagOMX_ProceedReturnedBuffer)(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader);
EndOfVirtuals;


ClassMembers(MagOmxPortImpl, MagOmxPort, \
    MagMessageHandle      (*createMessage)(OMX_HANDLETYPE handle, OMX_U32 what);     \
    _status_t             (*getLooper)(OMX_HANDLETYPE handle);                       \
    void                  (*dispatchBuffers)(OMX_HANDLETYPE hPort, OMX_BUFFERHEADERTYPE *bufHeader); \
    MagOMX_Port_Buffer_t *(*allocBufferNode)(OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE         (*putRunningNode)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE         (*getRunningNode)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer); \
    OMX_ERRORTYPE         (*putOutputNode)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE         (*getOutputNode)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer, OMX_BOOL block); \
    OMX_ERRORTYPE         (*relayReturnBuffer)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer); \
    void                  (*resetBuffer)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer); \
)
    MagMutexHandle         mhMutex;
    MagMutexHandle         mhTunnelFlushMutex;

    /*Link all buffer headers*/
    List_t                 mBufferList;
    /*Link the buffer headers that are ready to be sent out to tunneled port*/
    List_t                 mRunningBufferList;
    /*Link the buffer headers that are ready for the ports data transferring inside the component */
    List_t                 mOutputBufferList;

    OMX_S32                mBuffersTotal;
    OMX_S32                mFreeBuffersNum;

    MagEventHandle         mAllBufReturnedEvent;
    MagEventGroupHandle    mBufferEventGroup;
    OMX_BOOL               mWaitOnAllBuffers;

    MagEventHandle         mGetOutputBufferEvent;
    MagEventGroupHandle    mOutputBufferEventGroup;
    OMX_BOOL               mWaitOnOutputBuffer;

    MagEventHandle         mTunneledBufStEvt;
    MagEventGroupHandle    mTunneledBufStEvtGrp;

    MagEventHandle         mBufPopulatedEvt;
    MagEventGroupHandle    mBufPopulatedEvtGrp;

    MagLooperHandle        mLooper;
    MagHandlerHandle       mMsgHandler;
    
    MagMessageHandle       mEmptyThisBufferMsg;
    MagMessageHandle       mFillThisBufferMsg;
    MagMessageHandle       mReturnThisBufferMsg;
    MagMessageHandle       mSharedBufferMsg;
    MagMessageHandle       mOutputBufferMsg;

    OMX_BUFFERSUPPLIERTYPE mBufSupplierType;
    OMX_HANDLETYPE         mTunneledComponent;
    OMX_U32                mTunneledPortIndex;
    List_t                 mBufDispatcherList;

    List_t                 mPendingReturnBufListH;
    List_t                 mFreePendingBufListH;
    MagMutexHandle         mhRtnBufListMutex;

EndOfClassMembers;


#endif
