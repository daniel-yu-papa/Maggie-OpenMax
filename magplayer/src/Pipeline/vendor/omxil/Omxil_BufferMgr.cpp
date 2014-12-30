#include "Omxil_BufferMgr.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline_OMX"

OmxilBufferMgr::OmxilBufferMgr(ui32 size, ui32 num):
                                    mBufSize(size),
                                    mBufNum(num),
                                    mFreeNodeNum(num){
    Mag_CreateMutex(&mListMutex);
    INIT_LIST(&mBufFreeListHead);
    INIT_LIST(&mBufBusyListHead);
}

OmxilBufferMgr::~OmxilBufferMgr(){
    List_t *next;
    Omxil_BufferNode_t *bufNode;
    
    next = mBufFreeListHead.next;
    while(next != &mBufFreeListHead){
        bufNode = (Omxil_BufferNode_t *)list_entry(next, Omxil_BufferNode_t, Node);
        list_del(next);
        mag_free(bufNode);
        next = mBufFreeListHead.next;
    }

    Mag_DestroyMutex(&mListMutex);
    AGILE_LOGD("Cleaned up the component[%p] buffers", mhComponent);
}

OMX_ERRORTYPE OmxilBufferMgr::create(OMX_HANDLETYPE hComp, ui32 portIdx, OMX_HANDLETYPE privData){
    ui32 i = 0;
    OMX_ERRORTYPE err;
    Omxil_BufferNode_t *bufNode;
    OMX_BUFFERHEADERTYPE *bufHeader;

    Mag_AcquireMutex(mListMutex);

    mhComponent = hComp;
    for (i = 0; i < mBufNum; i++){
        /*the field pAppPrivate is NULL for now, which is used for hold MagOmxMediaBuffer_t *buf*/
        err = OMX_AllocateBuffer(hComp, &bufHeader, portIdx, privData, mBufSize);
        if (err != OMX_ErrorNone) {
            AGILE_LOGE("Failed to Allocate %dth Buffer for Component %p", i, hComp);
            return err;
        }

        bufNode = (Omxil_BufferNode_t *)mag_mallocz(sizeof(Omxil_BufferNode_t));

        INIT_LIST(&bufNode->Node);
        bufNode->pBufHeader = bufHeader;
        list_add_tail(&bufNode->Node, &mBufFreeListHead);
    }

    Mag_ReleaseMutex(mListMutex);
    
    return err;
}


OMX_BUFFERHEADERTYPE *OmxilBufferMgr::get(){
    List_t *next = NULL;
    Omxil_BufferNode_t *bufHeader = NULL;
    
    Mag_AcquireMutex(mListMutex);

    next = mBufFreeListHead.next;
    if (next != &mBufFreeListHead){
        list_del(next);
        bufHeader = (Omxil_BufferNode_t *)list_entry(next, Omxil_BufferNode_t, Node);
        list_add_tail(next, &mBufBusyListHead);
        mFreeNodeNum--;
    }else{
        AGILE_LOGE("[Component[%p] get]: no node in the mBufFreeListHead! mFreeNodeNum = %d", 
                    mhComponent, mFreeNodeNum);
    }
    
    Mag_ReleaseMutex(mListMutex);

    if (bufHeader){
        return bufHeader->pBufHeader;
    }else{
        return NULL;
    }
}

void OmxilBufferMgr::put(OMX_BUFFERHEADERTYPE *bufHeader){
    List_t *next;
    Omxil_BufferNode_t *bufNode = NULL;
    
    Mag_AcquireMutex(mListMutex);
    
    next = mBufBusyListHead.next;
    if(next != &mBufBusyListHead){
        bufNode = (Omxil_BufferNode_t *)list_entry(next, Omxil_BufferNode_t, Node);
        list_del(next);
        bufNode->pBufHeader = bufHeader;
        list_add_tail(next, &mBufFreeListHead);
        mFreeNodeNum++;
        AGILE_LOGD("[Component[%p] put]: mFreeNodeNum = %d", mhComponent, mFreeNodeNum);
    }else{
        AGILE_LOGE("[Component[%p] put]: should not be here, some nodes are missing. mFreeNodeNum = %d", 
                    mhComponent, mFreeNodeNum);
    }
    
    Mag_ReleaseMutex(mListMutex);
}

bool OmxilBufferMgr::needPushBuffers(void){
    if (mFreeNodeNum > 1){
        return true; 
    }else{
        AGILE_LOGV("[Component[%p] needPushBuffers], free buffers = %d", 
                    mhComponent, mFreeNodeNum);
        return false;
    }
}