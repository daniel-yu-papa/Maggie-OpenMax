#include "MagDemuxerBaseImpl.h"
#include "MagPlatform.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Demuxer"

#define MAX_PARAM_DB_ITEMS_NUMBER  64

#define LOOPER_NAME "MagPlayerDemuxerLooper"



MagDemuxerBaseImpl::MagDemuxerBaseImpl():
                        mpStreamTrackManager(NULL),
                        mIsPrepared(MAG_FALSE),
                        mLooper(NULL),
                        mMsgHandler(NULL),
                        // mDataReadyMsg(NULL),
                        mIsEOS(false)
                        {
    /*
    mParamDB = createMagMiniDB(MAX_PARAM_DB_ITEMS_NUMBER);
    if (NULL == mParamDB){
        AGILE_LOGE("Failed to create the parameters db!");
    }*/
    AGILE_LOGV("Enter!");

    mParamDB = NULL;

    // mDataReadyMsg = createMessage(MagDemuxerMsg_PlayerNotify);
    // mDataReadyMsg->setInt32(mDataReadyMsg, "what", MagDemuxerBase::kWhatReadFrame);

    mDemuxStopMsg    = createMessage(MagDemuxerMsg_Stop);
    mDemuxFlushMsg   = createMessage(MagDemuxerMsg_Flush); 
}

MagDemuxerBaseImpl::~MagDemuxerBaseImpl(){
    AGILE_LOGV("Enter!");
    delete mpStreamTrackManager;
    destroyMagMessage(&mDemuxStopMsg);
    AGILE_LOGV("Exit!");
}

_status_t  MagDemuxerBaseImpl::setPlayingTrackID(ui32 index){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    return mpStreamTrackManager->setPlayingTrackID(index);
}

ui32       MagDemuxerBaseImpl::getPlayingTracksID(ui32 *index){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return 0;
    }

    return mpStreamTrackManager->getPlayingTracksID(index);
}

_status_t   MagDemuxerBaseImpl::readFrame(ui32 trackIndex, MagOmxMediaBuffer_t **buffer){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_NO_INIT;
    }

    return mpStreamTrackManager->readFrame(trackIndex, buffer);
}

MagMessageHandle MagDemuxerBaseImpl::createNotifyMsg(){
    MagMessageHandle msg;

    msg = createMessage(MagDemuxerMsg_PlayerNotify);
    return msg;
}

_status_t  MagDemuxerBaseImpl::prepare(MagContentPipe *contentPipe, MagBufferObserver *pObserver, MagMiniDBHandle paramDB){
    MagMessageHandle msg;
    _status_t res;
    Demuxer_BufferPolicy_t policy;

    AGILE_LOGV("Enter!");

    msg = createMessage(MagDemuxerMsg_ContentPipeNotify);
    contentPipe->SetDemuxerNotifier(msg);

    mParamDB = paramDB;
    if (NULL == mpStreamTrackManager)
        mpStreamTrackManager = new Stream_Track_Manager(static_cast<void *>(this), paramDB);

    if (NULL == mpStreamTrackManager){
        AGILE_LOGE("Failed to new Stream_Track_Manager!");
        return MAG_NO_MEMORY;
    }

    if (pObserver){
        res = pObserver->start(&policy);
    }else{
        AGILE_LOGE("The pObserver is NULL!");
        return MAG_BAD_VALUE;
    }

    if (res == MAG_NO_ERROR){
        mpStreamTrackManager->setBufferPolicy(&policy);
        mpStreamTrackManager->attachBufferObserver(pObserver);
    }else{
        AGILE_LOGE("failed to start the buffer observer! ret = %d", res);
    }

    return res;
}

_status_t  MagDemuxerBaseImpl::start(){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    if (mpStreamTrackManager){
        mPlayState.abort_request = 0;
        mpStreamTrackManager->start();
    }else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

_status_t  MagDemuxerBaseImpl::pause(){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    if (mpStreamTrackManager)
        mpStreamTrackManager->pause();
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

_status_t  MagDemuxerBaseImpl::resume(){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    if (mpStreamTrackManager){
        // mPlayState.abort_request = 0;
        mpStreamTrackManager->resume();
    }else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

void MagDemuxerBaseImpl::onStop(MagMessageHandle msg){
    AGILE_LOGV("enter!");

    if (mpStreamTrackManager)
        mpStreamTrackManager->stop();
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
    }
    stopImpl();
}

_status_t MagDemuxerBaseImpl::stop(){
    AGILE_LOGV("enter!");
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    mPlayState.abort_request = 1;
    mIsEOS = false;
    
    mLooper->clear(mLooper);
    mDemuxStopMsg->postMessage(mDemuxStopMsg, 0);
    mLooper->waitOnAllDone(mLooper);
    AGILE_LOGV("exit!");
    return MAG_NO_ERROR;
}

void MagDemuxerBaseImpl::readyToStop(){
    AGILE_LOGV("enter!");
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return;
    }

    if (mpStreamTrackManager)
        mpStreamTrackManager->readyToStop();
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
    }
}

void MagDemuxerBaseImpl::readyToFlush(){
    AGILE_LOGV("enter!");
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return;
    }

    if (mpStreamTrackManager)
        mpStreamTrackManager->readyToFlush();
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
    }
}

void MagDemuxerBaseImpl::onFlush(MagMessageHandle msg){
    AGILE_LOGV("enter!");

    if (mpStreamTrackManager)
        mpStreamTrackManager->flush();
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
    }
    flushImpl();
}

/*flush() and resume() command pair*/
_status_t MagDemuxerBaseImpl::flush(){
    AGILE_LOGV("enter!");
    // mPlayState.abort_request = 1;
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    mLooper->clear(mLooper);
    mDemuxFlushMsg->postMessage(mDemuxFlushMsg, 0);
    mLooper->waitOnAllDone(mLooper);
    AGILE_LOGV("exit!");
    return MAG_NO_ERROR;
}

_status_t MagDemuxerBaseImpl::seekTo(i32 msec, i64 mediaTime){
    /*TrackInfoTable_t *tb = NULL;
    ui32 i;
    TrackInfo_t *ti;

    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return MAG_INVALID_OPERATION;
    }

    tb = getTrackInfoList();

    if (tb->videoTrackNum > 0){
        for (i = 0; i < tb->videoTrackNum; i++){
            ti = tb->trackTableList[i];
            if (ti->status == TRACK_PLAY){
                return seekToImpl(msec, ti);
            }
        }
    }

    if (tb->audioTrackNum > 0){
        for (i = tb->videoTrackNum; i < tb->videoTrackNum + tb->audioTrackNum; i++){
            ti = tb->trackTableList[i];
            if (ti->status == TRACK_PLAY){
                return seekToImpl(msec, ti);
            }
        }
    }
    AGILE_LOGE("no stream track is in playing status. Quit the seek!");*/
    return seekToImpl(msec, mediaTime, NULL);
}

TrackInfoTable_t *MagDemuxerBaseImpl::getTrackInfoList(){
    if (!mIsPrepared){
        AGILE_LOGE("The demuxer has NOT been started! Quit!");
        return NULL;
    }

    if (mpStreamTrackManager)
        return mpStreamTrackManager->getTrackInfoList();
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
    }
    return NULL;
}

void MagDemuxerBaseImpl::getAVBufferStatus(ui32 *videoBuf, ui32 *audioBuf, ui32 *loadingSpeed){
    if (mpStreamTrackManager)
        return mpStreamTrackManager->getAVBufferStatus(videoBuf, audioBuf, loadingSpeed);
    else{
        AGILE_LOGE("the mpStreamTrackManager is NOT initialized!");
    }
}

void MagDemuxerBaseImpl::abort(){
    mPlayState.abort_request = 1;
}

void      MagDemuxerBaseImpl::setEOS(){
    mIsEOS = true;
}

_status_t   MagDemuxerBaseImpl::dettachBufferObserver(MagBufferObserver *pObserver){
    return mpStreamTrackManager->dettachBufferObserver(pObserver);
}


void MagDemuxerBaseImpl::onPlayerNotify(MagMessageHandle msg){
    boolean ret;
    i32 idx;
    _status_t res;
    
    MagOmxMediaBuffer_t *buf = NULL;
    MagMessageHandle reply = NULL;
    i32 what;

    ret = msg->findInt32(msg, "what", &what);
    if (!ret){
        AGILE_LOGE("failed to find the what int32!");
        return;
    }

    if (what == MagDemuxerBase::kWhatReadFrame){
        ret = msg->findInt32(msg, "track-idx", &idx);
        if (!ret){
            AGILE_LOGE("failed to find the track-idx!");
            return;
        }   
        
        ret = msg->findMessage(msg, "reply", &reply);
        if (!ret){
            AGILE_LOGE("failed to find the reply message!");
        } 
        
        AGILE_LOGV("message: what = kWhatReadFrame, track-idx = %d", idx);

        if (NULL == reply){
            AGILE_LOGE("the reply message is NULL");
            return;
        }

        res = readFrame(idx, &buf);
        if (MAG_NO_ERROR == res){
            if (NULL != buf){
                reply->setString(reply, "eos", "no");
                reply->setPointer(reply, "media-buffer", static_cast<void *>(buf), MAG_FALSE);
                /*send message to the pipeline that owns the media track processing*/
                reply->postMessage(reply, 0);
            }else{
                AGILE_LOGE("the media buffer is NULL!");
            }
        }else if (MAG_NAME_NOT_FOUND == res){
            reply->setString(reply, "eos", "no");
            reply->setPointer(reply, "media-buffer", NULL, MAG_FALSE);
            /*send message to the pipeline that owns the media track processing*/
            reply->postMessage(reply, 0);
        }else{
            if (mIsEOS){
                reply->setString(reply, "eos", "yes");
                reply->setPointer(reply, "media-buffer", NULL, MAG_FALSE);
                /*send message to the pipeline that owns the media track processing*/
                reply->postMessage(reply, 0);
            }else{
                TrackInfoTable_t *trackList;
                TrackInfo_t *ti;

                trackList = mpStreamTrackManager->getTrackInfoList();

                if (trackList){
                    ti = trackList->trackTableList[idx];
                    ATOMIC_INC(&ti->pendingRead);
                    ti->message->setMessage(ti->message, "reply", reply, MAG_FALSE);
                    
                    AGILE_LOGV("track: %d, pending read number: %d]", idx, ti->pendingRead);
                }
            }
        }
    }
}

void MagDemuxerBaseImpl::onContentPipeNotify(MagMessageHandle msg){
    char *eos;
    boolean ret;
    
    ret = msg->findString(msg, "eos", &eos);
    if (!ret){
        AGILE_LOGE("failed to find the eos string!");
        return;
    }

    if (!strcmp(eos, "yes")){
        /*get EOS notification*/
        setEOS();
        AGILE_LOGV("get EOS!");
    }else{
        i32 i;
        TrackInfo_t *ti;
        i32 pendings;
        i32 totalTracks;
        TrackInfoTable_t *trackList;

        trackList = mpStreamTrackManager->getTrackInfoList();
        if (!trackList)
            return;
        
        totalTracks = trackList->totalTrackNum;
        
        for (i = 0; i < totalTracks; i++){
            ti = trackList->trackTableList[i];
            pendings = ti->pendingRead;
            if (pendings > 0){
                i32 j;
                for (j = 0; j < pendings; j++){
                   ti->message->postMessage(ti->message, 0);
                   ATOMIC_DEC(&ti->pendingRead);
                   AGILE_LOGV("track %d: send the pending readframe message!", i);
                } 
            }
        }   
    }
}

void MagDemuxerBaseImpl::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagDemuxerBaseImpl *thiz = (MagDemuxerBaseImpl *)priv;
    
    switch (msg->what(msg)) {
        case MagDemuxerMsg_PlayerNotify:
            thiz->onPlayerNotify(msg);
            break;

        case MagDemuxerMsg_ContentPipeNotify:
            thiz->onContentPipeNotify(msg);
            break;

        case MagDemuxerMsg_Stop:
            thiz->onStop(msg);
            break;
        
        case MagDemuxerMsg_Flush:
            thiz->onFlush(msg);
            break;

        default:
            break;
    }
}

MagMessageHandle MagDemuxerBaseImpl::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagDemuxerBaseImpl::getLooper(){
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