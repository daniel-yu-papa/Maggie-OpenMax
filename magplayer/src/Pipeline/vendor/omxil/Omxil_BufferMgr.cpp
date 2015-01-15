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
    mWaitPutBufEventGroup = NULL;
    mPutBufEvent          = NULL;
    mWaitGetBufEventGroup = NULL;
    mGetBufEvent          = NULL;

    if (mBlock){
        Mag_CreateEventGroup(&mWaitPutBufEventGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&mPutBufEvent, MAG_EVT_PRIO_DEFAULT)){
            Mag_AddEventGroup(mWaitPutBufEventGroup, mPutBufEvent);
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

    next = mBufBusyListHead.next;
    while(next != &mBufBusyListHead){
        bufNode = (Omxil_BufferNode_t *)list_entry(next, Omxil_BufferNode_t, Node);
        list_del(next);
        mag_free(bufNode);
        next = mBufBusyListHead.next;
    }

    Mag_DestroyEvent(&mPutBufEvent);
    Mag_DestroyEventGroup(&mWaitPutBufEventGroup);
    Mag_DestroyEvent(&mGetBufEvent);
    Mag_DestroyEventGroup(&mWaitGetBufEventGroup);
    Mag_DestroyMutex(&mListMutex);
    AGILE_LOGD("Cleaned up the component[%p] buffers", mhComponent);
}

/*
 * if hComp != NULL: it is get-put operation sequence
 * if hComp == NULL: it is put-get operation sequence
 */
OMX_ERRORTYPE OmxilBufferMgr::create(OMX_HANDLETYPE hComp, ui32 portIdx, OMX_HANDLETYPE privData){
    ui32 i = 0;
    OMX_ERRORTYPE err;
    Omxil_BufferNode_t *bufNode;
    OMX_BUFFERHEADERTYPE *bufHeader = NULL;

    Mag_AcquireMutex(mListMutex);

    mhComponent = hComp;
    
    if (mhComponent == NULL){
        Mag_CreateEventGroup(&mWaitGetBufEventGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&mGetBufEvent, MAG_EVT_PRIO_DEFAULT)){
            Mag_AddEventGroup(mWaitGetBufEventGroup, mGetBufEvent);
        }
    }

    for (i = 0; i < mBufNum; i++){
        if (mhComponent){
            /*the field pAppPrivate is NULL for now, which is used for hold MagOmxMediaBuffer_t *buf*/
            err = OMX_AllocateBuffer(hComp, &bufHeader, portIdx, privData, mBufSize);
            if (err != OMX_ErrorNone) {
                Mag_ReleaseMutex(mListMutex);
                AGILE_LOGE("Failed to Allocate %dth Buffer for Component %p", i, hComp);
                return err;
            }
        }
        bufNode = (Omxil_BufferNode_t *)mag_mallocz(sizeof(Omxil_BufferNode_t));

        INIT_LIST(&bufNode->Node);
        bufNode->pBufHeader = bufHeader;
        if (mhComponent){
            list_add_tail(&bufNode->Node, &mBufFreeListHead);
        }else{
            list_add_tail(&bufNode->Node, &mBufBusyListHead);
        }
    }

    Mag_ReleaseMutex(mListMutex);
    
    return err;
}


OMX_BUFFERHEADERTYPE *OmxilBufferMgr::get(){
    List_t *next = NULL;
    Omxil_BufferNode_t *bufHeader = NULL;
    bool empty = false;

get_again:
    Mag_AcquireMutex(mListMutex);

    next = mBufFreeListHead.next;
    if (next != &mBufFreeListHead){
        if (mBufBusyListHead.next == &mBufBusyListHead){
            empty = true;
        }
        list_del(next);
        bufHeader = (Omxil_BufferNode_t *)list_entry(next, Omxil_BufferNode_t, Node);
        list_add_tail(next, &mBufBusyListHead);
        mFreeNodeNum--;
        if ((mhComponent == NULL) && empty && mBlock){
            /*put-get sequence*/
            Mag_SetEvent(mGetBufEvent);
        }
        Mag_ReleaseMutex(mListMutex);
    }else{
        if (mBlock){
            Mag_ClearEvent(mPutBufEvent);
            Mag_ReleaseMutex(mListMutex);
            AGILE_LOGD("Wait on getting the buffer!");
            Mag_WaitForEventGroup(mWaitPutBufEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            AGILE_LOGD("get buffer!");
            goto get_again;
        }else{
            Mag_ReleaseMutex(mListMutex);
            AGILE_LOGD("[Component[%p] get]: no node in the mBufFreeListHead! mFreeNodeNum = %d", 
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

put_again:
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
        Mag_ReleaseMutex(mListMutex);
    }else{
        if (mhComponent == NULL){
            /*get-put sequence*/
            Mag_ClearEvent(mGetBufEvent);
            Mag_ReleaseMutex(mListMutex);
            Mag_WaitForEventGroup(mWaitGetBufEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            AGILE_LOGD("[put-get] get a buffer!");
            goto put_again;
        }else{
            Mag_ReleaseMutex(mListMutex);
            AGILE_LOGE("[Component[%p] put]: should not be here, some nodes missed. mFreeNodeNum = %d", 
                        mhComponent, mFreeNodeNum);
        }
    }
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