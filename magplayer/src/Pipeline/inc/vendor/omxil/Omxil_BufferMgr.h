#ifndef __OMXIL_BUFFER_MANAGER_H__
#define __OMXIL_BUFFER_MANAGER_H__

#include "framework/MagFramework.h"
#include "MagOMX_IL.h"

typedef struct{
    List_t Node;    
    OMX_BUFFERHEADERTYPE *pBufHeader;
}Omxil_BufferNode_t;

class OmxilBufferMgr{
public:
    OmxilBufferMgr(ui32 buf_size, ui32 buf_num, bool block);
    ~OmxilBufferMgr();

    OMX_ERRORTYPE create(OMX_HANDLETYPE hComp, ui32 portIdx, OMX_HANDLETYPE privData);
    OMX_BUFFERHEADERTYPE *get();
    void put(OMX_BUFFERHEADERTYPE *bufHeader);
    
    bool needPushBuffers(void);

private:
    ui32 mBufSize;
    ui32 mBufNum;
    ui32 mFreeNodeNum;

    MagMutexHandle mListMutex;
    List_t mBufFreeListHead;
    List_t mBufBusyListHead;

    bool mBlock;

    MagEventHandle         mPutBufEvent;
    MagEventGroupHandle    mWaitPutBufEventGroup;

    MagEventHandle         mGetBufEvent;
    MagEventGroupHandle    mWaitGetBufEventGroup;

    OMX_HANDLETYPE mhComponent;
};

#endif
