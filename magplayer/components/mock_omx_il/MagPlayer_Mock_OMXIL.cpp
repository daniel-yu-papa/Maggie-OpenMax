#include "MagPlayer_Mock_OMXIL.h"
#include "MagPlayer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magPlayerMockOMXIL"


#define ES_DUMP_FILE_NAME "/data/data/magplayer_%s.es"

MagPlayer_Mock_OMX::MagPlayer_Mock_OMX(const char *type):
                                      mMagPlayerNotifier(NULL){ 
    mType = mag_strdup(type);
}

MagPlayer_Mock_OMX::~MagPlayer_Mock_OMX(){
    destroyMagMessage(mEmptyThisBufferMsg);
}

void MagPlayer_Mock_OMX::start(){
    char filename[128];
    mEmptyThisBufferMsg = createMessage(MagMockOMX_EmptyThisBuffer);

    sprintf(filename, ES_DUMP_FILE_NAME, mType);
    mDumpFile = fopen(filename, "wb+");
    if (NULL == mDumpFile){
        AGILE_LOGE("failed to open the file: %s", ES_DUMP_FILE_NAME);
    }
    
    mLooper->start(mLooper);
    
    postFillThisBuffer();
}

void MagPlayer_Mock_OMX::setMagPlayerNotifier(MagMessageHandle notifyMsg){
    mMagPlayerNotifier = notifyMsg;
}

void MagPlayer_Mock_OMX::proceedMediaBuffer(MediaBuffer_t *buf){
    if (NULL != mDumpFile){
        fwrite(buf->buffer, 1, buf->buffer_size, mDumpFile);
    }
    buf->release(buf);
}

void MagPlayer_Mock_OMX::onEmptyThisBuffer(MagMessageHandle msg){
    boolean ret;
    void *value;
    MediaBuffer_t *buf = NULL;
    
    ret = msg->findPointer(msg, "media-buffer", &value);
    if (!ret){
        AGILE_LOGE("failed to find the media-buffer pointer!");
        return;
    }  

    buf = static_cast<MediaBuffer_t *>(value);
    proceedMediaBuffer(buf);

    postFillThisBuffer();
}

void MagPlayer_Mock_OMX::postFillThisBuffer(){
    if (mMagPlayerNotifier != NULL){
        mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "what", MagPlayer::kWhatFillThisBuffer);
        mMagPlayerNotifier->setMessage(mMagPlayerNotifier, "reply", mEmptyThisBufferMsg);
        mMagPlayerNotifier->postMessage(mMagPlayerNotifier, 0);
    }else{
        AGILE_LOGE("mMagPlayerNotifier is not setting!");
    }
}

void MagPlayer_Mock_OMX::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer_Mock_OMX *thiz = (MagPlayer_Mock_OMX *)priv;
    
    switch (msg->what(msg)) {
        case MagMockOMX_EmptyThisBuffer:
            thiz->onEmptyThisBuffer(msg);
            break;
            
        default:
            break;
    }
}

MagMessageHandle MagPlayer_Mock_OMX::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagPlayer_Mock_OMX::getLooper(){
    if ((NULL != mLooper) && (NULL != mMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == mLooper){
        mLooper = createLooper(LOOPER_NAME);
    }
    
    if (NULL != mLooper){
        if (NULL == mMsgHandler){
            mMsgHandler = createHandler(mLooper, onMessageReceived, (void *)this);

            if (NULL != mMsgHandler){
                mLooper->registerHandler(mLooper, mMsgHandler);
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


