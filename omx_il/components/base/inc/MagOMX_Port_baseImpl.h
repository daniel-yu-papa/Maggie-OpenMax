#ifndef __MAG_OMX_PORT_BASE_IMPL_H__
#define __MAG_OMX_PORT_BASE_IMPL_H__

#include "MagOMX_Port_base.h"

#define OMX_BUFFERFLAG_EXT_FREE   0x01000000
#define OMX_BUFFERFLAG_EXT_USING  0x02000000

enum{
    MagOmxPortImpl_EmptyThisBufferMsg,
    MagOmxPortImpl_FillThisBufferMsg,
    MagOmxPortImpl_ReturnThisBufferMsg,
    MagOmxPortImpl_SharedBufferMsg
};

typedef enum{
    MagOmxPortImpl_OwnedByNone,
    MagOmxPortImpl_OwnedByThisPort,
    MagOmxPortImpl_OwnedByTunneledPort,
    MagOmxPortImpl_OwnedByILClient,
}MagOMX_Buffer_Owner_t;

typedef struct{
    List_t node;
    List_t runNode;
    /*bufferHeaderOwner: only could be OwnedByThisPort or OwnedByTunneledPort*/
    MagOMX_Buffer_Owner_t bufferHeaderOwner;
    /*bufferOwner: only could be OwnedByThisPort or OwnedByILClient*/
    MagOMX_Buffer_Owner_t bufferOwner;
    OMX_BUFFERHEADERTYPE  *pOmxBufferHeader;
}MagOMX_Port_Buffer_t;

typedef struct{
    List_t node;
    MagEventHandle msg;
}BufferDispatcherNode_t;

DeclareClass(MagOmxPortImpl, MagOmxPort);

Virtuals(MagOmxPortImpl, MagOmxPort) 
    /*pure virtual functions and must be overrided by sub-component*/
    OMX_ERRORTYPE (*MagOMX_AllocateBuffer)(MagOmxPortImpl port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes);
    OMX_ERRORTYPE (*MagOMX_FreeBuffer)(MagOmxPortImpl port, OMX_U8 *pBuffer);
    OMX_ERRORTYPE (*MagOMX_EmptyThisBuffer)(MagOmxPortImpl port, OMX_BUFFERHEADERTYPE* pBufHeader);
    OMX_ERRORTYPE (*MagOMX_FillThisBuffer)(MagOmxPortImpl port, OMX_BUFFERHEADERTYPE* pBufHeader);
EndOfVirtuals;


ClassMembers(MagOmxPortImpl, MagOmxPort, \
    MagMessageHandle (*createMessage)(OMX_HANDLETYPE handle, OMX_U32 what);     \
    _status_t        (*getLooper)(OMX_HANDLETYPE handle);                       \  
    void             (*dispatchBuffers)(MagOmxPortImpl port, OMX_BUFFERHEADERTYPE *bufHeader); \
    MagOMX_Port_Buffer_t *(*allocBufferNode)(OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE (*putRunningNode)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE* pBuffer);  \
    OMX_ERRORTYPE (*getRunningNode)(MagOmxPortImpl hPort, OMX_BUFFERHEADERTYPE **ppBuffer); \

)
    MagMutexHandle         mhMutex;

    List_t                 mBufferList;
    List_t                 mRunningBufferList;
    OMX_U32                mBuffersTotal;
    OMX_U32                mFreeBuffersNum;

    MagEventHandle         mAllBufReturnedEvent;
    MagEventGroupHandle    mBufferEventGroup;
    OMX_BOOL               mWaitOnBuffers;

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

    OMX_BUFFERSUPPLIERTYPE mBufSupplierType;
    OMX_HANDLETYPE         mTunneledComponent;
    OMX_U32                mTunneledPortIndex;
    List_t                 mBufDispatcherList;

EndOfClassMembers;


#endif
