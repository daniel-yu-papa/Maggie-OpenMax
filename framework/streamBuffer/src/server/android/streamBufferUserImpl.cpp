#include "StreamBufferDef.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-StreamBuffer"

StreamBufferUser::StreamBufferUser(const sp<IStreamBuffer> &buffer, _size_t bufSize, _size_t bufNum):
                  mEOS(false){
    INIT_LIST(&mBufferListHead);
    
    mBufferSize   = bufSize;
    mBufferNumber = bufNum;
    mStreamBuffer = buffer;
    mReadPolicy   = StreamBufferUser::READ_ANY;
        
    init();
}

StreamBufferUser::~StreamBufferUser(){
    List_t *tmpNode;
    ui32 size = 0;
    BufferNode_t *pBufNode;

    tmpNode = mBufferListHead.next;
    
    while(tmpNode != &mBufferListHead){
        list_del(tmpNode);
        mag_free(tmpNode);
        tmpNode = mBufferListHead.next;
    }

    if (mCBMgr != NULL){
        Mag_DestroyMutex(mCBMgr->lock);
        mag_free(mCBMgr);
    }
}

void StreamBufferUser::init(){
    BufferNode_t *p;

    mMemDealer = new MemoryDealer(mBufferNumber * mBufferSize);
    for (_size_t i = 0; i < mBufferNumber; ++i) {
        p = (BufferNode_t *)mag_mallocz(sizeof(BufferNode_t));
        p->buffer = mMemDealer->allocate(mBufferSize);
        INIT_LIST(&p->node);
        list_add_tail(&p->node, &mBufferListHead);
    }
    
    if (mBufferNumber == 1){
        /*organize the buffer as the circular buffer*/
        List_t *tmpNode;
        BufferNode_t *pBufNode;
    
        mCBMgr = (CircularBufferMgr_t *)mag_mallocz(sizeof(CircularBufferMgr_t));
        
        tmpNode = mBufferListHead.next;
        pBufNode = (BufferNode_t *)list_entry(tmpNode, BufferNode_t, node);
        mCBMgr->totalSize = pBufNode->buffer->size();
        mCBMgr->pointer   = (ui8 *)pBufNode->buffer->pointer();
        Mag_CreateMutex(&mCBMgr->lock);
    }else{

    }

    mStreamBuffer->setUser(this);
    mStreamBuffer->setBuffers(&mBufferListHead);
}

void StreamBufferUser::fill_CircularBuffer(_size_t size){
    if (mCBMgr != NULL){
        Mag_AcquireMutex(mCBMgr->lock);
        //AGILE_LOGV("filled %u bytes data", size);
        mCBMgr->writeIndex += size;
        mCBMgr->writeIndex = mCBMgr->writeIndex % mCBMgr->totalSize;
        Mag_ReleaseMutex(mCBMgr->lock);
    }else{
       AGILE_LOGE("mCBMgr is NULL. invalid state!"); 
    }
}

void StreamBufferUser::onBufferFilled(_size_t index, _size_t size){
    if (mBufferNumber == 1){
        fill_CircularBuffer(size);
    }else{

    }
}

bool StreamBufferUser::isEOS(){
    return mEOS;
}

void StreamBufferUser::reset(){
    if (mCBMgr){
        mCBMgr->readIndex  = 0;
        mCBMgr->writeIndex = 0;
    }
}

void StreamBufferUser::issueCommand(Command cmd, bool synchronous){
    switch(cmd){
        case EOS:
            AGILE_LOGI("get EOS command");
            mEOS = true;
            break;
            
        default:
            break;
    }
}

_size_t StreamBufferUser::getDataSize(){
    i32 w;
    i32 r;

    Mag_AcquireMutex(mCBMgr->lock);
    r = mCBMgr->readIndex;
    w = mCBMgr->writeIndex;
    Mag_ReleaseMutex(mCBMgr->lock);

    if (w < r){
        return (_size_t)(mCBMgr->totalSize - (r - w));
    }else if (w > r){
        return (_size_t)(w - r);
    }
    return 0;
}

void StreamBufferUser::setReadPolicy(enum Read_Policy p){
    mReadPolicy = p;
}

_size_t StreamBufferUser::readData_CircularBuffer(void *data, _size_t size){
    i32 w;
    i32 total_read = 0;

restart:
    Mag_AcquireMutex(mCBMgr->lock);
    w = mCBMgr->writeIndex;
    Mag_ReleaseMutex(mCBMgr->lock);

    if (mCBMgr != NULL){
        AGILE_LOGV("readIndex=%d, writeIndex=%u, size=%u", mCBMgr->readIndex, w, size);
        if (w == mCBMgr->readIndex){
            /*if w == r: the buffer is empty*/
            return 0;
        }else if (w < mCBMgr->readIndex){
            if ((w + mCBMgr->totalSize - mCBMgr->readIndex) >= static_cast<i32>(size)){
                if (mCBMgr->totalSize >= mCBMgr->readIndex + static_cast<i32>(size)){
                    memcpy(data, static_cast<void *>(static_cast<ui8 *>(mCBMgr->pointer) + mCBMgr->readIndex), size);
                    mCBMgr->readIndex += size;
                }else{
                    ui32 firstPart = mCBMgr->totalSize - mCBMgr->readIndex;
                    ui32 secondPart = size - firstPart;
                    if (firstPart > 0)
                        memcpy(data, static_cast<void *>(static_cast<ui8 *>(mCBMgr->pointer) + mCBMgr->readIndex), firstPart);
                    memcpy(static_cast<void *>(static_cast<ui8 *>(data) + firstPart), 
                           static_cast<void *>(mCBMgr->pointer), secondPart);
                    mCBMgr->readIndex = secondPart;  
                }
                mStreamBuffer->onBufferEmpty(0, size);
                return size;
            }else{
                if (mReadPolicy == StreamBufferUser::READ_ANY){
                    ui32 firstPart = mCBMgr->totalSize - mCBMgr->readIndex;
                    ui32 secondPart = w;

                    if (firstPart > 0)
                        memcpy(data, static_cast<void *>(static_cast<ui8 *>(mCBMgr->pointer) + mCBMgr->readIndex), firstPart);
                    memcpy(static_cast<void *>(static_cast<ui8 *>(data) + firstPart), static_cast<void *>(mCBMgr->pointer), secondPart);

                    mCBMgr->readIndex = w;
                    total_read = firstPart + secondPart;
                    mStreamBuffer->onBufferEmpty(0, total_read);
                    return total_read;
                }else{
                    usleep(10000);
                    goto restart;
                }
            }
        }else{
            if (w - mCBMgr->readIndex >= static_cast<i32>(size)){
                memcpy(data, static_cast<void *>(static_cast<ui8 *>(mCBMgr->pointer) + mCBMgr->readIndex), size);
                mCBMgr->readIndex += size;
                mStreamBuffer->onBufferEmpty(0, size);
                return size;
            }else{
                if (mReadPolicy == StreamBufferUser::READ_ANY){
                    total_read = w - mCBMgr->readIndex;
                    memcpy(data, static_cast<void *>(static_cast<ui8 *>(mCBMgr->pointer) + mCBMgr->readIndex), total_read);
                    mCBMgr->readIndex += total_read;
                    mStreamBuffer->onBufferEmpty(0, total_read);
                    return total_read;
                }else{
                    usleep(10000);
                    goto restart;
                }
            }
        }
    }else{
        AGILE_LOGE("mCBMgr is NULL. invalid state!");
    }
    return 0;    
}

_size_t StreamBufferUser::read(void *data, _size_t size){
    if (mBufferNumber == 1){
        return readData_CircularBuffer(data, size);
    }else{
        return 0;
    }
}


