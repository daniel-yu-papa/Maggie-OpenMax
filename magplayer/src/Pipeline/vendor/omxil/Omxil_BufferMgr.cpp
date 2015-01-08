#include "Omxil_BufferMgr.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline_OMX"

OmxilBufferMgr::OmxilBufferMgr(ui32 size, ui32 num, bool block):
                                    mBufSize(size),
                                    mBufNum(num),
                                    mFreeNodeNum(num),
                                    mBlock(block){
    Mag_CreateMutex(&mListMutex);
    INIT_LIST(&mBufFreeListHead);
    INIT_LIST(&mBufBusyListHead);

    if (mBlock){
        Mag_CreateEventGroup(&mWaitBufEventGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&mPutBufEvent, MAG_EVT_PRIO_DEFAULT)){
            Mag_AddEventGroup(mWaitBufEventGroup, mPutBufEvent);
        }
    }
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

    Mag_DestroyEvent(&mPutBufEvent);
    Mag_DestroyEventGroup(&mWaitBufEventGroup);
    Mag_DestroyMutex(&mListMutex);
    AGILE_LOGD("Cleaned up the component[%p] buffers", mhComponent);
}

OMX_ERRORTYPE OmxilBufferMgr::create(OMX_HANDLETYPE hComp, ui32 portIdx, OMX_HANDLETYPE privData){
    ui32 i = 0;
    OMX_ERRORTYPE err;
    Omxil_BufferNode_t *bufNode;
    OMX_BUFFERHEADERTYPE *bufHeader = NULL;

    Mag_AcquireMutex(mListMutex);

    mhComponent = hComp;
    
    for (i = 0; i < mBufNum; i++){
        if (mhComponent){
            /*the field pAppPrivate is NULL for now, which is used for hold MagOmxMediaBuffer_t *buf*/
            err = OMX_AllocateBuffer(hComp, &bufHeader, portIdx, privData, mBufSize);
            if (err != OMX_ErrorNone) {
                AGILE_LOGE("Failed to Allocate %dth Buffer for Component %p", i, hComp);
                return err;
            }
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
        Mag_ReleaseMutex(mListMutex);
    }else{
        if (mBlock){
            Mag_ReleaseMutex(mListMutex);
            Mag_WaitForEventGroup(mWaitBufEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            AGILE_LOGD("get buffer!");
        }else{
            Mag_ReleaseMutex(mListMutex);
            AGILE_LOGE("[Component[%p] get]: no node in the mBufFreeListHead! mFreeNodeNum = %d", 
                        mhComponent, mFreeNodeNum);
        }
    }

    if (bufHeader){
        return bufHeader->pBufHeader;
    }else{
        return NULL;
    }
}

void OmxilBufferMgr::put(OMX_BUFFERHEADERTYPE *bufHeader){
    List_t *next;
    Omxil_BufferNode_t *bufNode = NULL;
    bool empty = false;

    Mag_AcquireMutex(mListMutex);
    
    next = mBufBusyListHead.next;
    if(next != &mBufBusyListHead){
        if (mBufFreeListHead.next == &mBufFreeListHead){
            empty = true;
        }
        bufNode = (Omxil_BufferNode_t *)list_entry(next, Omxil_BufferNode_t, Node);
        list_del(next);
        bufNode->pBufHeader = bufHeader;
        list_add_tail(next, &mBufFreeListHead);
        mFreeNodeNum++;
        if (empty && mBlock){
            Mag_SetEvent(mPutBufEvent);
        }
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