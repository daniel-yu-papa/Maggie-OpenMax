#include "StreamBufferDef.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-StreamBuffer"

StreamBuffer::StreamBuffer(){
    mBufferNum = 0;
    mCBMgr     = NULL;
    mbQuit     = false;
    mType      = INVALID;
    
    Mag_CreateEventGroup(&mBufStatusEvtGrp);

    if (MAG_ErrNone == Mag_CreateEvent(&mBufFreeEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mBufStatusEvtGrp, mBufFreeEvt);
}

StreamBuffer::~StreamBuffer(){
    mbQuit = true;
    Mag_SetEvent(mBufFreeEvt);
    Mag_DestroyEvent(mBufFreeEvt);
    Mag_DestroyEventGroup(mBufStatusEvtGrp); 

    List_t *tmpNode;
    ui32 size = 0;
    BufferNode_t *pBufNode;

    tmpNode = mBufferHead->next;
    
    while(tmpNode != mBufferHead){
        list_del(tmpNode);
        mag_free(tmpNode);
        tmpNode = mBufferHead->next;
    }

    if (mCBMgr != NULL){
        Mag_DestroyMutex(mCBMgr->lock);
        mag_free(mCBMgr);
    }
}

void StreamBuffer::setUser(const sp<IStreamBufferUser> &user){
    AGILE_LOGI("enter!");
    mUser = user;
}

void StreamBuffer::setBuffers(List_t *bufListHead){
    List_t *tmpNode;
    ui32 size = 0;
    BufferNode_t *pBufNode;

    AGILE_LOGI("enter!");
    
    tmpNode = bufListHead->next;
    while(tmpNode != bufListHead){
        size++;
        tmpNode = tmpNode->next;
    }
    mBufferNum  = size;
    if (1 == mBufferNum){
        mCBMgr = (CircularBufferMgr_t *)mag_mallocz(sizeof(CircularBufferMgr_t));
        
        tmpNode = bufListHead->next;
        pBufNode = (BufferNode_t *)list_entry(tmpNode, BufferNode_t, node);
        mCBMgr->totalSize = pBufNode->buffer->size();
        mCBMgr->pointer   = static_cast<ui8 *>(pBufNode->buffer->pointer());
        AGILE_LOGI("buf num is 1. buffer size = %u, pointer = 0x%x", mCBMgr->totalSize, mCBMgr->pointer);
        Mag_CreateMutex(&mCBMgr->lock);
    }else{
        
    }
    mBufferHead = bufListHead;
}

_size_t StreamBuffer::writeData_CircularBuffer(void *data, _size_t size, bool block){
    i32 r;
    _size_t total_write = 0;
    
    if ((mCBMgr->totalSize - 1) < size){
        AGILE_LOGE("copy size:%u > buffer size:%d. Quit!", size, mCBMgr->totalSize);
        return 0;
    }
    
restart:
    if (mbQuit){
        AGILE_LOGW("Quit the writing in the middle!");
        return 0;
    }
    
    Mag_AcquireMutex(mCBMgr->lock);
    r = mCBMgr->readIndex;
    Mag_ReleaseMutex(mCBMgr->lock);
    
    if (mCBMgr != NULL){
        //AGILE_LOGI("readIndex=%d, writeIndex=%d", r, mCBMgr->writeIndex);
        if (r <= mCBMgr->writeIndex){
            /*if w == r: the buffer is empty*/
            if ((mCBMgr->writeIndex - r + size) <= (mCBMgr->totalSize - 1)){
                if (mCBMgr->totalSize >= mCBMgr->writeIndex + static_cast<i32>(size)){
                    memcpy(static_cast<void *>(mCBMgr->pointer + mCBMgr->writeIndex), data, size);
                    mCBMgr->writeIndex += size;
                }else{
                    ui32 firstPart = mCBMgr->totalSize - mCBMgr->writeIndex;
                    ui32 secondPart = size - firstPart;
                    if (firstPart > 0)
                        memcpy(static_cast<void *>(mCBMgr->pointer + mCBMgr->writeIndex), data, firstPart);
                    memcpy(static_cast<void *>(mCBMgr->pointer), 
                           static_cast<void *>(static_cast<ui8 *>(data) + firstPart), secondPart);
                    mCBMgr->writeIndex = secondPart;  
                }
                mUser->onBufferFilled(0, size);
            }else{
                if (block){
                    /*wait on buffer free event*/
                    Mag_WaitForEventGroup(mBufStatusEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                    goto restart;
                }else{
                    ui32 firstPart  = (r == 0 ? mCBMgr->totalSize - mCBMgr->writeIndex - 1 : mCBMgr->totalSize - mCBMgr->writeIndex);
                    ui32 secondPart = (r == 0 ? 0 : r - 1);
                    if (firstPart > 0){
                        memcpy(static_cast<void *>(mCBMgr->pointer + mCBMgr->writeIndex), data, firstPart);
                        if (secondPart > 0){
                            memcpy(static_cast<void *>(mCBMgr->pointer), 
                                   static_cast<void *>(static_cast<ui8 *>(data) + firstPart), secondPart);
                        }
                    }
                    total_write = firstPart + secondPart;

                    mCBMgr->writeIndex += total_write;
                    mCBMgr->writeIndex = mCBMgr->writeIndex % mCBMgr->totalSize;
                    
                    return total_write;
                }
            }
        }else{
            if ((r - mCBMgr->writeIndex - 1) >= (i32)size){
                memcpy(static_cast<void *>(mCBMgr->pointer + mCBMgr->writeIndex), data, size);
                mCBMgr->writeIndex += size;
                mUser->onBufferFilled(0, size);
            }else{
                if (block){
                    /*wait on buffer free event*/
                    AGILE_LOGD("wait on buffer-free-event!");
                    Mag_WaitForEventGroup(mBufStatusEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
                    goto restart;
                }else{
                    total_write = r - mCBMgr->writeIndex - 1;
                    if (total_write > 0)
                       memcpy(static_cast<void *>(mCBMgr->pointer + mCBMgr->writeIndex), data, total_write); 

                    mCBMgr->writeIndex += total_write;
                    
                    return total_write;
                }
            }
        }
    }else{
        AGILE_LOGE("mCBMgr is NULL. invalid state!");
        return 0;
    }
    return size;
}

_size_t StreamBuffer::WriteData(void *data, _size_t size, bool block){
    if (mBufferNum == 1){
        /*organize the buffer as the circular buffer*/
        return writeData_CircularBuffer(data, size, block);
    }else{
        return 0;
    }
}

sp<IStreamBufferUser>& StreamBuffer::getUser(){
    return mUser;
}

void StreamBuffer::drain_CircularBuffer(_size_t size){
    if (mCBMgr != NULL){
        Mag_AcquireMutex(mCBMgr->lock);
        //AGILE_LOGI("consumed %u bytes data", size);
        mCBMgr->readIndex += size;
        mCBMgr->readIndex = mCBMgr->readIndex % mCBMgr->totalSize;
        Mag_ReleaseMutex(mCBMgr->lock);
        
        Mag_SetEvent(mBufFreeEvt);
    }else{
       AGILE_LOGE("mCBMgr is NULL. invalid state!"); 
    }
}

void StreamBuffer::onBufferEmpty(_size_t index, _size_t size){
    if (mBufferNum == 1){
        /*organize the buffer as the circular buffer*/
        drain_CircularBuffer(size);
    }else{

    }
}

IStreamBuffer::Type StreamBuffer::getType(void){
    return mType;
}

void StreamBuffer::setType(IStreamBuffer::Type t){
    mType = t;
}

