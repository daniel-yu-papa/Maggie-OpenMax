#include "MagPlayer_Demuxer_Base.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magPlayerDemuxer"

#define MAX_PARAM_DB_ITEMS_NUMBER  64

#define LOOPER_NAME "MagPlayerDemuxerLooper"

_status_t releaseMediaBuffer(MediaBuffer_t *mb){
    Stream_Track *strack = static_cast<Stream_Track *>(mb->track_obj);
    return strack->releaseFrame(mb);
}

Stream_Track::Stream_Track(TrackInfo_t *info, ui32 buffer_pool_size):
                               mInfo(info),
                               mBufferPoolSize(buffer_pool_size),
                               mBufPoolHandle(NULL){
    INIT_LIST(&mMBufBusyListHead);
    INIT_LIST(&mMBufFreeListHead);
    Mag_CreateMutex(&mMutex);
}

Stream_Track::~Stream_Track(){
    Mag_DestroyMutex(mMutex);
    magMemPoolDestroy(mBufPoolHandle);   
}

_status_t Stream_Track::start(){    
    if (mBufferPoolSize > 0){
        mBufPoolHandle = magMemPoolCreate(mBufferPoolSize);
        if (mBufPoolHandle != NULL){
            AGILE_LOGI("The buffer pool 0x%x is created!", mBufPoolHandle);
        }else{
            AGILE_LOGE("Failed to create the buffer pool!");
            return MAG_NO_MEMORY;
        }
    }
    return MAG_NO_ERROR;
}

TrackInfo_t* Stream_Track::getInfo(){
    return mInfo;
}


MediaBuffer_t* Stream_Track::dequeueFrame(){
    List_t *next;
    MediaBuffer_t *item = NULL;
    
    Mag_AcquireMutex(mMutex);
    
    next = mMBufBusyListHead.next;
    if (next != &mMBufBusyListHead){
        item = (MediaBuffer_t *)list_entry(next, MediaBuffer_t, node);
        list_del(next);
    }
    
    Mag_ReleaseMutex(mMutex);
    return item;
}

_status_t     Stream_Track::enqueueFrame(MediaBuffer_t *mb){
    void *buf;
    
    if (NULL == mBufPoolHandle){
        if (MAG_NO_ERROR == start()){
            buf = magMemPoolGetBuffer(mBufPoolHandle, mb->buffer_size);
            if (NULL != buf){
                memcpy(buf, mb->buffer, mb->buffer_size);
                mb->buffer = buf;
                Mag_AcquireMutex(mMutex);
                putMediaBuffer(&mMBufBusyListHead, mb);
                Mag_ReleaseMutex(mMutex);
                return MAG_NO_ERROR;
            }
        }
    }
    return MAG_NO_MEMORY;
}

_status_t     Stream_Track::releaseFrame(MediaBuffer_t *mb){
    if (NULL == mb){
        magMemPoolPutBuffer(mBufPoolHandle, mb->buffer);

        Mag_AcquireMutex(mMutex);
        putMediaBuffer(&mMBufFreeListHead, mb);
        Mag_ReleaseMutex(mMutex);
        return MAG_NO_ERROR;
    }else{
        return MAG_NO_MEMORY;
    }
}

MediaBuffer_t *Stream_Track::getMediaBuffer(){
    List_t *next = mMBufFreeListHead.next;
    MediaBuffer_t *item;

    Mag_AcquireMutex(mMutex);
    if (next != &mMBufFreeListHead){
        item = (MediaBuffer_t *)list_entry(next, MediaBuffer_t, node);
        list_del(next);
    }else{
        item = (MediaBuffer_t *)mag_mallocz(sizeof(MediaBuffer_t));
        if (NULL != item){
            INIT_LIST(&item->node);
            item->track_obj = static_cast<void *>(this);
            item->release   = releaseMediaBuffer;
        }
    }
    Mag_ReleaseMutex(mMutex);
    
    return item;
}

_status_t Stream_Track::putMediaBuffer(List_t *list_head, MediaBuffer_t *mb){
    if ((mb != NULL) && (list_head != NULL)){
        list_add_tail(&mb->node, list_head);
        return MAG_NO_ERROR;
    }else{
        return MAG_BAD_VALUE;
    }
}

_status_t Stream_Track::reset(){
    MediaBuffer_t* item;
    
    do{
        item = dequeueFrame();
        if (item != NULL){
            releaseFrame(item);
        }
    }while(item != NULL);

    return MAG_NO_ERROR;
}

MagPlayer_Demuxer_Base::MagPlayer_Demuxer_Base():
                        mIsStarted(MAG_FALSE),
                        mTrackList(NULL),
                        mLooper(NULL),
                        mMsgHandler(NULL){
    mParamDB = createMagMiniDB(MAX_PARAM_DB_ITEMS_NUMBER);
    if (NULL == mParamDB){
        AGILE_LOGE("Failed to create the parameters db!");
    }
}

MagPlayer_Demuxer_Base::~MagPlayer_Demuxer_Base(){
    destroyMagMiniDB(mParamDB);

    if (NULL != mTrackList){
        if (mTrackList->trackTableList)
            mag_free(mTrackList->trackTableList);
        
        mag_free(mTrackList);
    }
}

void MagPlayer_Demuxer_Base::setParameters(const char *name, MagParamType_t type, void *value){
    if (NULL != mParamDB){
        switch (type){
            case MagParamTypeInt32:
                {
                i32 v = *(static_cast<i32 *>(value));
                mParamDB->setInt32(mParamDB, name, v);
                }
                break;

            case MagParamTypeInt64:
                {
                i64 v = *(static_cast<i64 *>(value));
                mParamDB->setInt64(mParamDB, name, v);
                }
                break;

            case MagParamTypeUInt32:
                {
                ui32 v = *(static_cast<ui32 *>(value));
                mParamDB->setUInt32(mParamDB, name, v);
                }
                break;

            case MagParamTypeFloat:
                {
                fp32 v = *(static_cast<fp32 *>(value));
                mParamDB->setFloat(mParamDB, name, v);
                }
                break;

            case MagParamTypeDouble:
                {
                fp64 v = *(static_cast<fp64 *>(value));
                mParamDB->setDouble(mParamDB, name, v);
                }
                break;

            case MagParamTypePointer:
                mParamDB->setPointer(mParamDB, name, value);
                break;

            case MagParamTypeString:
                {
                char *v = static_cast<char *>(value);
                mParamDB->setString(mParamDB, name, v);
                }
                break;

            default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                break;
        }
    }else{
        AGILE_LOGE("the parameter db is NOT initialized!");
    }
}


TrackInfoTable_t *MagPlayer_Demuxer_Base::getTrackInfoList(){
    void *value;
    i32 vnumber = 0;
    i32 anumber = 0;
    i32 snumber = 0;
    boolean ret;
    i32 i;
    char buf[128];
    ui32 trackIndex = 0;
    i32 totalTrackNum = 0;
    TrackInfo_t *track;
    
    if (!mIsStarted){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return NULL;
    }
    
    if (mTrackList != NULL){
        return mTrackList;
    }

    mTrackList = (TrackInfoTable_t *)mag_mallocz(sizeof(TrackInfoTable_t));
    if (NULL == mTrackList){
        AGILE_LOGE("Failed to create TrackInfoTable_t!");
        return NULL;
    }

    ret = mParamDB->findInt32(mParamDB, kDemuxer_Video_Track_Number, &vnumber);
    if (ret == MAG_TRUE){
        mTrackList->videoTrackNum = vnumber;
        totalTrackNum = totalTrackNum + vnumber;
        AGILE_LOGI("video track number is %d", vnumber);
    }

    ret = mParamDB->findInt32(mParamDB, kDemuxer_Audio_Track_Number, &anumber);
    if (ret == MAG_TRUE){
        mTrackList->audioTrackNum = anumber;
        totalTrackNum = totalTrackNum + anumber;
        AGILE_LOGI("audio track number is %d", anumber);
    }

    ret = mParamDB->findInt32(mParamDB, kDemuxer_Subtitle_Track_Number, &snumber);
    if (ret == MAG_TRUE){
        mTrackList->subtitleTrackNum = snumber;
        totalTrackNum = totalTrackNum + snumber;
        AGILE_LOGI("subtitle track number is %d", snumber);
    }
    AGILE_LOGI("total track number is %d", totalTrackNum);
    
    if (totalTrackNum > 0){
        mTrackList->trackTableList = (TrackInfo_t **)mag_mallocz(totalTrackNum * sizeof(TrackInfo_t *));
        if (NULL == mTrackList->trackTableList){
            AGILE_LOGE("Failed to create mTrackList->trackTableList!");
            mag_free(mTrackList);
            mTrackList = NULL;
            return NULL;
        }

        i32 v=0;
        i32 a=0;
        i32 s=0;
        TrackInfo_t *ti;
        
        for (i = 0; i < totalTrackNum; i++){
            sprintf(buf, kDemuxer_Track_Info, i);
            ret = mParamDB->findPointer(mParamDB, buf, &value);
            if (ret == MAG_TRUE){
                track = static_cast<TrackInfo_t *>(value);
                AGILE_LOGI("index %d: track %s[0x%x], pid = %d, codec = %d", i, track->type == TRACK_VIDEO ? "video" : "audio",
                            track, track->pid, track->codec);
                if (track->type == TRACK_VIDEO){
                    mTrackList->trackTableList[v] = track;
                    v++;
                }else if (track->type == TRACK_AUDIO){
                    mTrackList->trackTableList[vnumber + a] = track;
                    a++;
                }else if (track->type == TRACK_SUBTITLE){
                    mTrackList->trackTableList[vnumber + anumber + s] = track;
                    s++;
                }else{
                    AGILE_LOGE("unknown track type: %d", track->type);
                }
            }
        }
    }else{
        mTrackList->trackTableList = NULL;
    }
    
    return mTrackList;
}

_status_t  MagPlayer_Demuxer_Base::setPlayingTrackID(ui32 index){
    TrackInfo_t *ti;
    
    if (!mIsStarted){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    if (mTrackList == NULL){
        getTrackInfoList();
    }

    ti  = mTrackList->trackTableList[index];
    AGILE_LOGD("index track: 0x%x", ti);
    if (NULL != ti){
        AGILE_LOGD("[index:%d]: To set %s track[0x%x] -- name = %s, pid = %d as Playing", 
                    index, ti->type == TRACK_VIDEO ? "video" : "audio", ti, 
                    ti->name, ti->pid);
        ti->status = TRACK_PLAY;
    }
    return MAG_NO_ERROR;
}

ui32       MagPlayer_Demuxer_Base::getPlayingTracksID(ui32 **index){
    i32 i;
    TrackInfo_t * ti;
    i32 j = 0;
    i32 total;
    
    if (!mIsStarted){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return 0;
    }

    if (mTrackList == NULL){
        getTrackInfoList();
    }

    total = mTrackList->videoTrackNum + mTrackList->audioTrackNum + mTrackList->subtitleTrackNum;
    for (i = 0; i < total; i++){
        ti = mTrackList->trackTableList[i];
        if (ti->status == TRACK_PLAY){
            *(index + j) = &ti->index;
            j++;
        }
    }

    return j;
}

_status_t   MagPlayer_Demuxer_Base::readFrame(ui32 trackIndex, MediaBuffer_t **buffer){
    TrackInfo_t * ti;

    AGILE_LOGD("trackid = %d", trackIndex);
    ti = mTrackList->trackTableList[trackIndex];

    if (NULL != ti){
        if (ti->status != TRACK_PLAY){
            AGILE_LOGD("the track id(%d) is not in play state. try to set it as play!", trackIndex);
            ti->status = TRACK_PLAY;
        }

        return readFrameInternal(ti->streamID, buffer);
    }else{
        return MAG_BAD_INDEX;
    }
}

MagMessageHandle MagPlayer_Demuxer_Base::createNotifyMsg(){
    MagMessageHandle msg;

    msg = createMessage(MagDemuxerMsg_PlayerNotify);
    return msg;
}

void MagPlayer_Demuxer_Base::onPlayerNotify(MagMessageHandle msg){
    boolean ret;
    i32 idx;

    MediaBuffer_t *buf = NULL;
    MagMessageHandle reply;
    i32 what;

    ret = msg->findInt32(msg, "what", &what);
    if (!ret){
        AGILE_LOGE("failed to find the what int32!");
        return;
    }

    if (what == MagPlayer_Demuxer_Base::kWhatReadFrame){
        ret = msg->findInt32(msg, "track-idx", &idx);
        if (!ret){
            AGILE_LOGE("failed to find the track-idx!");
            return;
        }   
        
        AGILE_LOGV("message: what = kWhatReadFrame, track-idx = %d", idx);
        if (MAG_NO_ERROR == readFrame(idx, &buf)){
            if (NULL != buf){
                ret = msg->findMessage(msg, "reply", &reply);
                if (!ret){
                    AGILE_LOGE("failed to find the reply message!");
                } 
                
                msg->setPointer(msg, "media-buffer", static_cast<void *>(buf));
                /*send message to OMX component that owns the media track processing*/
                reply->postMessage(reply, 0);
            }
        }
    }
}

void MagPlayer_Demuxer_Base::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer_Demuxer_Base *thiz = (MagPlayer_Demuxer_Base *)priv;
    
    switch (msg->what(msg)) {
        case MagDemuxerMsg_PlayerNotify:
            thiz->onPlayerNotify(msg);
            break;

        default:
            break;
    }
}

MagMessageHandle MagPlayer_Demuxer_Base::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagPlayer_Demuxer_Base::getLooper(){
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