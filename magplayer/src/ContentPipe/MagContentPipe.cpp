#include "MagContentPipe.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magPlayerContentPipe"

#define LOOPER_NAME "ComponentPipeLooper"

#define DATA_OBSERVER_RUNNING_DELAY 40
#define CP_CONFIGS_DB_SIZE          32

MagContentPipe::MagContentPipe(){
#ifdef INTER_PROCESSES
    mStreamBuffer = NULL;
#endif
    mSourceType   = MPCP_INVALID;

    mDemuxerNotifier = NULL;
    mLooper          = NULL;
    mMsgHandler      = NULL;
    
    mIsOpened        = false;
    
    mDataObserverMsg = createMessage(MagCPMsg_DataObserver);

    mConfigDB = createMagMiniDB(CP_CONFIGS_DB_SIZE);

    //set as Normal working mode in default
    mConfigDB->setUInt32(mConfigDB, kMPCPConfigName_WoringMode, MPCP_MODE_NORMAL);
}

MagContentPipe::~MagContentPipe(){
    if (mDemuxerNotifier)
        destroyMagMessage(&mDemuxerNotifier);
    destroyMagMessage(&mDataObserverMsg);
    destroyMagMiniDB(&mConfigDB);
}

MPCP_RESULTTYPE MagContentPipe::SetConfig(MPCP_IN const char *name, MagParamType_t type, MPCP_IN void *value){
    if (!mConfigDB){
        AGILE_LOGE("mConfigDB is NULL!!");
        return MPCP_ENOMEM;
    }

    switch (type){
        case MagParamTypeInt32:
            {
            i32 v = *((i32 *)value);
            AGILE_LOGV("name = %s, value = %d", name, v);
            mConfigDB->setInt32(mConfigDB, name, v);
            }
            break;

        case MagParamTypeInt64:
            {
            i64 v = *((i64 *)value);
            mConfigDB->setInt64(mConfigDB, name, v);
            }
            break;

        case MagParamTypeUInt32:
            {
            ui32 v = *((ui32 *)value);
            mConfigDB->setUInt32(mConfigDB, name, v);
            }
            break;

        case MagParamTypeFloat:
            {
            fp32 v = *((fp32 *)value);
            mConfigDB->setFloat(mConfigDB, name, v);
            }
            break;

        case MagParamTypeDouble:
            {
            fp64 v = *((fp64 *)value);
            mConfigDB->setDouble(mConfigDB, name, v);
            }
            break;

        case MagParamTypePointer:
            {
            AGILE_LOGV("[type: pointer]: name = %s, value = 0x%x", name, value);
            mConfigDB->setPointer(mConfigDB, name, value);
            }
            break;

        case MagParamTypeString:
            {
            char *v = (char *)value;
            AGILE_LOGV("[type: string]: name = %s, value = %s", name, v);
            mConfigDB->setString(mConfigDB, name, v);
            }
            break;

        default:
            AGILE_LOGE("the config type(%d) is unrecognized!", type);
            break;
    }
    return MPCP_OK;
}

MPCP_RESULTTYPE MagContentPipe::GetConfig(MPCP_IN const char *name, MagParamType_t type, MPCP_OUT void **value){
    boolean result;
    MPCP_RESULTTYPE ret = MPCP_OK;
    
   if (NULL != mConfigDB){
        switch (type){
            case MagParamTypeInt32:
            {
                i32 v;
                result = mConfigDB->findInt32(mConfigDB, name, &v);
                if (result)
                    *(i32 *)(*value) = v;
                else
                    ret = MPCP_ENOENT;
            }
                break;

            case MagParamTypeInt64:
            {
                i64 v;
                result = mConfigDB->findInt64(mConfigDB, name, &v);
                if (result)
                    *(i64 *)(*value) = v;
                else
                    ret = MPCP_ENOENT;
            }
                break;

            case MagParamTypeUInt32:
            {
                ui32 v;
                result = mConfigDB->findUInt32(mConfigDB, name, &v);
                if (result)
                    *(ui32 *)(*value) = v;
                else
                    ret = MPCP_ENOENT;
            }
                break;

            case MagParamTypeFloat:
            {
                fp32 v;
                result = mConfigDB->findFloat(mConfigDB, name, &v);
                if (result)
                    *(fp32 *)(*value) = v;
                else
                    ret = MPCP_ENOENT;
            }
                break;

            case MagParamTypeDouble:
            {
                fp64 v;
                result = mConfigDB->findDouble(mConfigDB, name, &v);
                if (result)
                    *(fp64 *)(*value) = v;
                else
                    ret = MPCP_ENOENT;
            }
                break;

            case MagParamTypePointer:
            {
                result = mConfigDB->findPointer(mConfigDB, name, value);
                if (!result)
                    ret = MPCP_ENOENT;
            }
                break;

            case MagParamTypeString:
            {
                char *v = NULL;
                result = mConfigDB->findString(mConfigDB, name, &v);
                AGILE_LOGD("string: %s", v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MPCP_ENOENT;
            }
                break;

            default:
                AGILE_LOGE("the config type(%d) is unrecognized!", type);
                ret = MPCP_ENOENT;
                break;
        }
    }else{
        AGILE_LOGE("the config db is NOT initialized!");
    }
    return ret;
}

MPCP_RESULTTYPE MagContentPipe::Create( MPCP_IN const char *szURI ){
    SetConfig(kMPCPConfigName_URI, MagParamTypeString, const_cast<char*>(szURI));
    //TODO:
    //create the data source to manage the data retrieving.
    return MPCP_OK;
}

#ifdef INTER_PROCESSES
MPCP_RESULTTYPE MagContentPipe::Create( MPCP_IN StreamBufferUser *buffer ){
    mStreamBuffer = buffer; 
    mSourceType   = MPCP_MEMORY;
    return MPCP_OK;
}
#endif

MPCP_RESULTTYPE MagContentPipe::Open( ){
    AGILE_LOGV("enter!");
#ifdef INTER_PROCESSES
    if (mSourceType == MPCP_MEMORY){
        AGILE_LOGV("set read policy: full!");
        mStreamBuffer->start(StreamBufferUser::READ_FULL);
    }
#endif
    mIsOpened = true;
    return MPCP_OK;
}

MPCP_RESULTTYPE MagContentPipe::Close( ){
    mLooper->clear(mLooper);
    if (mDemuxerNotifier)
        destroyMagMessage(&mDemuxerNotifier);
    mDemuxerNotifier = NULL;

#ifdef INTER_PROCESSES
    if (mSourceType == MPCP_MEMORY){
        if (NULL != mStreamBuffer)
            mStreamBuffer->reset();
    }
#endif
    mIsOpened = false;
    return MPCP_OK;
}

MPCP_RESULTTYPE MagContentPipe::SetPosition( MPCP_IN MPCP_POSITIONTYPE nOffset, MPCP_IN MPCP_ORIGINTYPE eOrigin ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagContentPipe::GetCurrentPosition( MPCP_OUT MPCP_POSITIONTYPE* pPosition ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagContentPipe::GetSize( MPCP_OUT MPCP_POSITIONTYPE* pSize ){
    return MPCP_OK;
}

MPCP_RESULTTYPE MagContentPipe::Read( MPCP_OUT ui8* pData, MPCP_INOUT ui32* pSize ){
    if (mSourceType == MPCP_MEMORY){
#ifdef INTER_PROCESSES
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
#endif
    }else{

    }
    return MPCP_OK;
}

void MagContentPipe::SetDemuxerNotifier(MagMessageHandle msg){
    mDemuxerNotifier = msg;
}

MPCP_RESULTTYPE MagContentPipe::Flush(){
    return MPCP_OK;
}

void MagContentPipe::onDataObserver(MagMessageHandle msg){
    i32 size;
    boolean ret;
    _size_t dataSize = 0;
    
    ret = msg->findInt32(msg, "size", &size);
    if (!ret){
        AGILE_LOGE("failed to find the size key!");
        return;
    }

    if (mSourceType == MPCP_MEMORY){
#ifdef INTER_PROCESSES
        dataSize = mStreamBuffer->getDataSize();
#endif
    }else{
        /*TODO*/
    }

    if ((i32)dataSize > size){
        AGILE_LOGV("data[%d bytes] are ready and notify app for actions", (i32)dataSize);
        mDemuxerNotifier->setString(mDemuxerNotifier, "eos", "no");
        mDemuxerNotifier->setInt32(mDemuxerNotifier, "data_size", dataSize);
        mDemuxerNotifier->postMessage(mDemuxerNotifier, 0);
    }else{
        msg->postMessage(msg, DATA_OBSERVER_RUNNING_DELAY);
    }
}

void MagContentPipe::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagContentPipe *thiz = (MagContentPipe *)priv;
    
    switch (msg->what(msg)) {
        case MagCPMsg_DataObserver:
            thiz->onDataObserver(msg);
            break;

        default:
            break;
    }
}

MagMessageHandle MagContentPipe::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagContentPipe::getLooper(){
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


