#include "MagPlayer_ContentPipe.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magPlayerContentPipe"

#define LOOPER_NAME "ComponentPipeLooper"

#define DATA_OBSERVER_RUNNING_DELAY 40

MagPlayer_Component_CP::MagPlayer_Component_CP(){
    mStreamBuffer = NULL;
    mSourceType   = MPCP_INVALID;

    mDemuxerNotifier = NULL;
    mLooper          = NULL;
    mMsgHandler      = NULL;
    
    mIsOpened        = false;
    
    mDataObserverMsg = createMessage(MagCPMsg_DataObserver);
}

MagPlayer_Component_CP::~MagPlayer_Component_CP(){

}

MPCP_RESULTTYPE MagPlayer_Component_CP::Create( MPCP_IN ui8 *szURI ){
    return MPCP_OK;
}


MPCP_RESULTTYPE MagPlayer_Component_CP::Create( MPCP_IN StreamBufferUser *buffer ){
    mStreamBuffer = buffer; 
    mSourceType   = MPCP_MEMORY;
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::Open( ){
    AGILE_LOGV("enter!");
    if (mSourceType == MPCP_MEMORY){
        AGILE_LOGV("set read policy: full!");
        mStreamBuffer->start(StreamBufferUser::READ_FULL);
    }
    mIsOpened = true;
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::Close( ){
    mLooper->clear(mLooper);
    if (mSourceType == MPCP_MEMORY){
        if (NULL != mStreamBuffer)
            mStreamBuffer->reset();
    }
    mIsOpened = false;
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::SetPosition( MPCP_IN MPCP_POSITIONTYPE nOffset, MPCP_IN MPCP_ORIGINTYPE eOrigin ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::GetCurrentPosition( MPCP_OUT MPCP_POSITIONTYPE* pPosition ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::GetSize( MPCP_OUT MPCP_POSITIONTYPE* pSize ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagPlayer_Component_CP::Read( MPCP_OUT ui8* pData, MPCP_INOUT ui32* pSize ){
    if (mSourceType == MPCP_MEMORY){
        ui32 readSize = *pSize;
        if (mStreamBuffer){
            *pSize = mStreamBuffer->read(static_cast<void *>(pData), readSize);
            if ((*pSize < readSize) && (mIsOpened)){
                /*start up the buffer observer to wait for the required data to be ready*/
                if (mStreamBuffer->isEOS()){
                    AGILE_LOGV("send mDemuxerNotifier message for EOS!");
                    mDemuxerNotifier->setString(mDemuxerNotifier, "eos", "yes");
                    mDemuxerNotifier->postMessage(mDemuxerNotifier, 0);
                }else{
                    AGILE_LOGV("Try to read %d bytes but available data is not enough!", readSize);
                    mDataObserverMsg->setInt32(mDataObserverMsg, "size", readSize);
                    mDataObserverMsg->postMessage(mDataObserverMsg, DATA_OBSERVER_RUNNING_DELAY);
                }
            }
        }else{
            return MPCP_EINVAL;
        }
    }else{

    }
    return MPCP_OK;
}

void MagPlayer_Component_CP::SetDemuxerNotifier(MagMessageHandle msg){
    mDemuxerNotifier = msg;
}

void MagPlayer_Component_CP::onDataObserver(MagMessageHandle msg){
    i32 size;
    boolean ret;
    _size_t dataSize;
    
    ret = msg->findInt32(msg, "size", &size);
    if (!ret){
        AGILE_LOGE("failed to find the size key!");
        return;
    }

    dataSize = mStreamBuffer->getDataSize();
    if ((i32)dataSize > size){
        AGILE_LOGV("data[%d bytes] are ready and notify app for actions", (i32)dataSize);
        mDemuxerNotifier->setString(mDemuxerNotifier, "eos", "no");
        mDemuxerNotifier->setInt32(mDemuxerNotifier, "data_size", dataSize);
        mDemuxerNotifier->postMessage(mDemuxerNotifier, 0);
    }else{
        msg->postMessage(msg, DATA_OBSERVER_RUNNING_DELAY);
    }
}

void MagPlayer_Component_CP::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer_Component_CP *thiz = (MagPlayer_Component_CP *)priv;
    
    switch (msg->what(msg)) {
        case MagCPMsg_DataObserver:
            thiz->onDataObserver(msg);
            break;

        default:
            break;
    }
}

MagMessageHandle MagPlayer_Component_CP::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagPlayer_Component_CP::getLooper(){
    if ((NULL != mLooper) && (NULL != mMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == mLooper){
        mLooper = createLooper(LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", mLooper);
    }
    
    if (NULL != mLooper){
        if (NULL == mMsgHandler){
            mMsgHandler = createHandler(mLooper, onMessageReceived, (void *)this);

            if (NULL != mMsgHandler){
                mLooper->registerHandler(mLooper, mMsgHandler);
                mLooper->start(mLooper);
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

#undef LOOPER_NAME


