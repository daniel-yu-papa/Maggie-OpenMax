#include "MagPlayer.h"
#include "MagEventType.h"
#include "MagVersion.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Main"


#define LOOPER_NAME "MagPlayerLooper"
#define SEEK_LOOPER_NAME "MagPlayerSeekLooper"

MagPlayer::MagPlayer(){
    initialize();
}

MagPlayer::~MagPlayer(){
    mLooper->clear(mLooper);
    reset();
    mLooper->waitOnAllDone(mLooper);
    cleanup();
}

char* MagPlayer::proceedUrl(char *url){
    mReportOutBufStatus = true;
    if ((strstr(url, "ipcamrtsp://") != NULL) ||
        (strstr(url, "ipcamhttp://") != NULL)){
        mParametersDB->setInt32(mParametersDB, kDemuxer_Disable_Buffering, 1);
        mParametersDB->setInt32(mParametersDB, kMediaPlayerConfig_AvSyncDisable, 1);
        mParametersDB->setInt32(mParametersDB, kMediaPlayerConfig_AudioDisable, 1);
        mReportOutBufStatus = false;
        return (url + 5);
    }else if(strstr(url, "rtsp://") != NULL){
        mParametersDB->setInt32(mParametersDB, kDemuxer_Disable_Buffering, 0);
        mParametersDB->setInt32(mParametersDB, kMediaPlayerConfig_AvSyncDisable, 0);
        mParametersDB->setInt32(mParametersDB, kMediaPlayerConfig_AudioDisable, 0);
    }else{
        mParametersDB->setInt32(mParametersDB, kDemuxer_Disable_Buffering, 0);
        mParametersDB->setInt32(mParametersDB, kMediaPlayerConfig_AvSyncDisable, 0);
        mParametersDB->setInt32(mParametersDB, kMediaPlayerConfig_AudioDisable, 0);
    }

    return url;
}

void MagPlayer::onSourceNotify(MagMessageHandle msg){
    char *type;
    boolean ret;

    AGILE_LOGV("enter!");
    
    if (mState != ST_IDLE){
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        mState = ST_ERROR;
        sendErrorEvent(E_WHAT_COMMAND, MagMsg_SourceNotify);
        return;
    }

    ret = msg->findString(msg, "type", &type);
    if (!ret){
        AGILE_LOGE("failed to find the type object!");
        return;
    }
    
    if (!strcmp(type, "path")){
        char *url;
        ret = msg->findString(msg, "url", &url);
        if (!ret){
            AGILE_LOGE("failed to find the url object!");
            return;
        }

        url = proceedUrl(url);
        AGILE_LOGD("proceeded url = %s!", url);

        if (mSource != NULL){
            mSource->Create(url);
        }else{
            AGILE_LOGE("failed to allocate the object: MagContentPipe");
            return;
        }
    }else if(!strcmp(type, "file")){

    }else if(!strcmp(type, "source")){
#ifdef INTER_PROCESSES
        void *hbuffer;
        StreamBufferUser *sbuffer;
        msg->findPointer(msg, "source", &hbuffer);
        sbuffer = static_cast<StreamBufferUser *>(hbuffer);
        if (mSource != NULL){
            mSource->Create(sbuffer);
        }else{
            AGILE_LOGE("failed to allocate the object: MagContentPipe");
            return;
        }
#endif
    }else{
        AGILE_LOGE("type: %s is wrong!", type);
        return;
    }

    if (mDemuxer == NULL){
        mDemuxer = new MagDemuxerFFMPEG();
    }

    if (mDemuxer)
        mState = ST_INITIALIZED;
    else
        AGILE_LOGE("failed to allocate the demuxer: MagDemuxerFFMPEG");
}

_status_t MagPlayer::setDataSource(const char *url){
    getLooper();

    AGILE_LOGD("url: %s", url);

    if (NULL == mSource)
        mSource = new MagContentPipe();
    
    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "type", "path");
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "url", url);
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        delete mSource;
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::setDataSource(i32 fd, i64 offset, i64 length){
    getLooper();

    if (NULL == mSource)
        mSource = new MagContentPipe();
    
    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "type", "file");
        mSourceNotifyMsg->setInt32(mSourceNotifyMsg, "fd", fd);
        mSourceNotifyMsg->setInt64(mSourceNotifyMsg, "offset", offset);
        mSourceNotifyMsg->setInt64(mSourceNotifyMsg, "length", length);
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        delete mSource;
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

#ifdef INTER_PROCESSES
_status_t MagPlayer::setDataSource(StreamBufferUser *buffer){
    MagContentPipe *cp;

    getLooper();
    
    if (NULL == mSource)
        mSource = new MagContentPipe();

    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "type", "buffer");
        mSourceNotifyMsg->setPointer(mSourceNotifyMsg, "source", static_cast<void *>(buffer), MAG_FALSE);
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        delete mSource;
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}
#endif

void MagPlayer::onErrorNotify(MagMessageHandle msg){
    i32 what;
    i32 extra;
    boolean ret;
    
    ret = msg->findInt32(msg, "what", &what);
    if (!ret){
        AGILE_LOGE("failed to find the what object!");
        what = E_WHAT_UNKNOWN;
    }

    ret = msg->findInt32(msg, "extra", &extra);
    if (!ret){
        AGILE_LOGE("failed to find the extra object!");
        extra = MAG_UNKNOWN_ERROR;
    }
    
    AGILE_LOGV("set the player to error status, (what:%d, extra:%d)",
                what, extra);

    Mag_SetEvent(mErrorEvt);
    
    if ((mErrorFn != NULL) && (mErrorPriv != NULL))
        mErrorFn(mErrorPriv, what, extra);
}

void MagPlayer::onPrepared(MagMessageHandle msg){
    _status_t ret;
    
    if (mState == ST_INITIALIZED){
        i32 hasContentPipe = 1;
        i32 noAVSync = 0;
        ui32 setValue;
        void *p;

        mState = ST_PREPARING;

        p = static_cast<void *>(&hasContentPipe);
        if (MAG_NO_ERROR == getParameters(kMediaPlayerConfig_CPAvail, MagParamTypeInt32, &p)){
            AGILE_LOGV("Content Pipe - %s", hasContentPipe ? "Yes" : "No");
            if (!hasContentPipe){
                setValue = MPCP_MODE_DUMMY;
                mSource->SetConfig(kMPCPConfigName_WoringMode, MagParamTypeUInt32, &setValue);
                if (NULL == mpDemuxerStreamObserver){
                    mpDemuxerStreamObserver = new MagBufferObserver(true, kBufferObserver_DemuxerStream);
                    mpDemuxerStreamObserver->setMediaPlayerNotifier(mBufferObserverNotifyMsg);
                }
            }else{
                setValue = MPCP_MODE_NORMAL;
                mSource->SetConfig(kMPCPConfigName_WoringMode, MagParamTypeUInt32, &setValue);
                if (NULL == mpContentPipeObserver){
                    mpContentPipeObserver =  new MagBufferObserver(true, kBufferObserver_ContentPipe);
                    mpContentPipeObserver->setMediaPlayerNotifier(mBufferObserverNotifyMsg);
                }

                if (NULL == mpDemuxerStreamObserver){
                    mpDemuxerStreamObserver = new MagBufferObserver(false, kBufferObserver_DemuxerStream);
                }
            }
        }
        mSource->Open();

        ret = mDemuxer->prepare(mSource, mpDemuxerStreamObserver, mParametersDB);

        if (ret == MAG_NO_ERROR){
            MagMessageHandle msg;
            if (NULL == mDemuxerNotify)
                mDemuxerNotify = mDemuxer->createNotifyMsg();
            mTrackTable = mDemuxer->getTrackInfoList();
            MAG_ASSERT(NULL != mTrackTable);
            if (mTrackTable->videoTrackNum > 0){
                mDemuxer->setPlayingTrackID(0);
                /*construct the video pipeline*/
                if (mVideoPipeline == NULL){
                    mVideoPipeline = new MagVideoPipeline(MAG_OMX_PIPELINE);
                    msg = createMessage(MagMsg_ComponentNotify);
                    msg->setInt32(msg, "track-idx", 0);
                    mVideoPipeline->setMagPlayerNotifier(msg);
                }else{
                    msg = mVideoPipeline->getMagPlayerNotifier();
                    msg->setInt32(msg, "track-idx", 0);
                }
                
                mVideoPipeline->init(0, mTrackTable->trackTableList[0]);
            }

            if (mTrackTable->audioTrackNum > 0){
                mDemuxer->setPlayingTrackID(mTrackTable->videoTrackNum);

                /*construct the audio pipeline*/
                if (mAudioPipeline == NULL){
                    mAudioPipeline = new MagAudioPipeline(MAG_OMX_PIPELINE); 
                    msg = createMessage(MagMsg_ComponentNotify);
                    msg->setInt32(msg, "track-idx", mTrackTable->videoTrackNum);
                    mAudioPipeline->setMagPlayerNotifier(msg);
                }else{
                    msg = mAudioPipeline->getMagPlayerNotifier();
                    msg->setInt32(msg, "track-idx", mTrackTable->videoTrackNum);
                }
                mAudioPipeline->init(mTrackTable->videoTrackNum, 
                                      mTrackTable->trackTableList[mTrackTable->videoTrackNum]);
            }

            if (mTrackTable->subtitleTrackNum > 0){
                mDemuxer->setPlayingTrackID(mTrackTable->videoTrackNum + mTrackTable->audioTrackNum);
            }
            
            p = static_cast<void *>(&noAVSync);
            getParameters(kMediaPlayerConfig_AvSyncDisable, MagParamTypeInt32, &p);
            AGILE_LOGV("Disable AV Sync - %s", noAVSync ? "Yes" : "No");

            if (mClock == NULL){
                mClock = new MagClock(MAG_OMX_CLOCK); 
            }

            if (mAVPipelineMgr == NULL)
                mAVPipelineMgr = new MagPipelineManager(mClock);

            if (mVideoPipeline)
                mAVPipelineMgr->addVideoPipeline(mVideoPipeline, !noAVSync ? MAG_TRUE : MAG_FALSE);

            if (mAudioPipeline)
                mAVPipelineMgr->addAudioPipeline(mAudioPipeline, !noAVSync ? MAG_TRUE : MAG_FALSE);

            mAVPipelineMgr->setup();

            mState = ST_PREPARED;
            if (mLeftVolume >= 0.0 && mRightVolume >= 0.0)
                setVolume(mLeftVolume, mRightVolume);
            
            Mag_SetEvent(mCompletePrepareEvt); 
        }else{
            if (ret != MAG_DEMUXER_ABORT){
                AGILE_LOGE("failed to prepare the demuxer module. ret = 0x%x", ret);
                mState = ST_ERROR;
                sendErrorEvent(E_WHAT_DEMUXER, ret);
            }
        }
    }else if (ST_STOPPED == mState){
        mSeekToMsg->setInt32(mSeekToMsg, "seek_to", 0);
        mSeekToMsg->setInt32(mSeekToMsg, "do_flush", 0);
        mSeekToMsg->postMessage(mSeekToMsg, 0);

        AGILE_LOGV("see if the seeking is complete or not?");
        Mag_WaitForEventGroup(mSeekEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGV("seeking completes");

        Mag_SetEvent(mCompletePrepareEvt);
        mState = ST_PREPARED;
    }else{
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        mState = ST_ERROR;
        sendErrorEvent(E_WHAT_COMMAND, MagMsg_Prepare);
    }  
}

_status_t MagPlayer::prepareAsync(){
    AGILE_LOGD("Enter!");
    if (mPrepareMsg != NULL){
        mPrepareMsg->postMessage(mPrepareMsg, 0);
    }else{
        AGILE_LOGE("the mPrepareMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::prepare(){
    prepareAsync();
    AGILE_LOGD("before prepare waiting!");
    Mag_WaitForEventGroup(mEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("after prepare waiting!");
    
    return MAG_NO_ERROR;
}

void MagPlayer::onPlay(MagMessageHandle msg){
    i32 buffer_play = 0;
    boolean ret;

    if (ST_BUFFERING == mState){
        if (!mbIsPlayed){
            /*first playing after the buffering is complete*/
            if (NULL != mAVPipelineMgr){
                mAVPipelineMgr->start();
            }
            
            mbIsPlayed = true;
        }else{
            /*pause/play sequence while the buffer is in low level*/
            if (NULL != mAVPipelineMgr){
                mAVPipelineMgr->resume();
            }
        }

        mState = ST_PLAYING;
    }else if (ST_PAUSED == mState){
        ret = msg->findInt32(msg, "buffering_play", &buffer_play);
        if (ret == MAG_FALSE){
            AGILE_LOGE("failed to find the key: buffering_play!! do resume() anyway");
            // mDemuxer->resume();
        }else{
            // if (!buffer_play){
            //     mDemuxer->resume();
            // }
        }
        if (!buffer_play){
            if (NULL != mAVPipelineMgr){
                mAVPipelineMgr->resume();
            }

            mState = ST_PLAYING;   
        }
    }else{
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        return;
    }
}

void MagPlayer::onStart(MagMessageHandle msg){
    AGILE_LOGV("enter!");
    if ((ST_PREPARED == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
        mDemuxer->start();
        mState = ST_BUFFERING;
    }else if (ST_PAUSED == mState){
        mPlayMsg->setInt32(mPlayMsg, "buffering_play", 0);
        mPlayMsg->postMessage(mPlayMsg, 0);;
    }else{
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        // mState = ST_ERROR;
        // sendErrorEvent(E_WHAT_COMMAND, MagMsg_Start);
        return;
    }
}


_status_t MagPlayer::start(){
    AGILE_LOGV("enter!");
    if (mStartMsg != NULL){
        mStartMsg->postMessage(mStartMsg, 0);
    }else{
        AGILE_LOGE("the mStartMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::doResetAction(){
    AGILE_LOGD("do reset()");

    mPlayTimeInMs = 0;

    mDemuxer->readyToStop();

    if ((ST_PLAYING == mState) ||
        (ST_PAUSED == mState) ||
        (ST_BUFFERING == mState)){
        if (NULL != mAVPipelineMgr){
            mAVPipelineMgr->stop();
        }
    }

    if (NULL != mAVPipelineMgr){
        mAVPipelineMgr->reset();
    }

    mSource->Close();
    if (mpContentPipeObserver)
        mpContentPipeObserver->reset();
    if (mpDemuxerStreamObserver)
        mpDemuxerStreamObserver->reset();
    mDemuxer->stop();

    Mag_ClearEvent(mCompletePrepareEvt);
    Mag_ClearEvent(mCompleteSeekEvt);
    Mag_ClearEvent(mCompleteFlushEvt);
    Mag_ClearEvent(mErrorEvt);

    AGILE_LOGD("exit!");
}

void MagPlayer::onStop(MagMessageHandle msg){
    AGILE_LOGD("enter! state=%d", mState);

    mPlayTimeInMs = 0;
    if ((ST_PLAYING == mState)   ||
        (ST_PAUSED == mState)    ||
        (ST_BUFFERING == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
        if (mDemuxer)
            mDemuxer->readyToFlush();

        if (NULL != mAVPipelineMgr){
            mAVPipelineMgr->stop();
        }

        if (mDemuxer)
            mDemuxer->flush();

        if (mSource)
            mSource->Flush();

        if (mpContentPipeObserver)
            mpContentPipeObserver->stop();
        if (mpDemuxerStreamObserver)
            mpDemuxerStreamObserver->stop();

        mbIsPlayed = false;
        mState = ST_STOPPED;
    }else{
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        // mState = ST_ERROR;
        // sendErrorEvent(E_WHAT_COMMAND, MagMsg_Stop);
        return;
    }
}

_status_t MagPlayer::stop(){
    if (mStopMsg != NULL){
        AGILE_LOGD("enter!");
        mStopMsg->postMessage(mStopMsg, 0);
    }else{
        AGILE_LOGE("the mStopMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onPause(MagMessageHandle msg){
    i32 buffer_pause = 0;
    boolean ret;
    ui8 flag;

    AGILE_LOGD("Enter!");
    if (ST_PLAYING == mState){
        ret = msg->findInt32(msg, "buffering_pause", &buffer_pause);
        if (ret == MAG_FALSE){
            AGILE_LOGE("failed to find the key: buffering_pause!! do pause() anyway");
            // mDemuxer->pause();
        }else{
            /*In pause state: continue the buffering till the buffer is full*/
            // if (!buffer_pause){
            //     mDemuxer->pause();
            // }
        }

        if (NULL != mAVPipelineMgr){
            flag = PAUSE_CLOCK_FLAG;
            if (!buffer_pause){
                flag |= PAUSE_AV_FLAG;
            }
            mAVPipelineMgr->pause(flag);
        }

        if (buffer_pause){
            mState = ST_BUFFERING;
        }else{
            mState = ST_PAUSED;
        }
    }else if (ST_BUFFERING == mState){
        AGILE_LOGV("Do pause action in buffering state!");
        if (NULL != mAVPipelineMgr){
            flag = PAUSE_AV_FLAG;
            mAVPipelineMgr->pause(flag);
        }

        mState = ST_PAUSED;
    }else if (ST_PAUSED == mState){
        AGILE_LOGV("ignore the pause action in paused state!");
    }else{
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        // mState = ST_ERROR;
        // sendErrorEvent(E_WHAT_COMMAND, MagMsg_Pause);
        return;
    }
}

_status_t MagPlayer::pause(){
    if (mPauseMsg != NULL){
        mPauseMsg->setInt32(mPauseMsg, "buffering_pause", 0);
        mPauseMsg->postMessage(mPauseMsg, 0);
    }else{
        AGILE_LOGE("the mPauseMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

bool MagPlayer::isPlaying(){
    if ((mState == ST_PLAYING) || (mState == ST_PAUSED) || (mState == ST_BUFFERING)){
        return true;
    }
    return false;
}

_status_t MagPlayer::getCurrentPosition(int* msec){
    i64 pos = 0;
    i64 startTime;

    *msec = 0;

    if ((mState >= ST_PREPARED) && (mState <= ST_PAUSED)){
        if ((mDemuxer != NULL) && (mTrackTable != NULL)){
            TrackInfo_t *ti = mTrackTable->trackTableList[0];
            startTime = ti->start_time;
        }else{
            AGILE_LOGE("demuxer[0x%p] or mTrackTable[0x%p] is invalid!", mDemuxer, mTrackTable);
            return MAG_NO_INIT;
        }

        if (mClock){
            pos = mClock->getPlayingTime();

            // AGILE_LOGV("[1] pos: %lld", pos);
            if ((pos > 0) && (pos >= startTime)){
                *msec = (int)(pos - startTime);
                mPlayTimeInMs = *msec;
            }else{
                *msec = mPlayTimeInMs;
            }
            // AGILE_LOGV("[2] pos: %d ms, start_time: %lld", *msec, startTime);
        }
    }

    if (pos < 0)
        return MAG_UNKNOWN_ERROR;
    else
        return MAG_NO_ERROR;
}

_status_t MagPlayer::getDuration(int* msec){
    if ((mState >= ST_PREPARED) && (mState <= ST_PAUSED)){
        if ((mDemuxer != NULL) && (mTrackTable != NULL)){
            TrackInfo_t *ti = mTrackTable->trackTableList[0];
            *msec = (int)(ti->duration);
            // AGILE_LOGD("duration: %d ms", *msec);
        }else{
            AGILE_LOGE("demuxer[0x%p] or mTrackTable[0x%p] is invalid!", mDemuxer, mTrackTable);
            *msec = 0;
            return MAG_NO_INIT;
        }
    }else{
        // AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        // return MAG_INVALID_OPERATION;
        *msec = 0;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onFlush(MagMessageHandle msg){
    boolean ret;
    i32 seek_flush;

    if (isValidFSState(mState)){
        mFlushBackState = mState;
        mState          = ST_FLUSHING;
        
        ret = msg->findInt32(msg, "seek_flush", &seek_flush);
        if (!ret){
            AGILE_LOGV("Failed to find the seek_flush object!");
            seek_flush = 0;
        }else{
            AGILE_LOGV("It is %s action!", seek_flush ? "seek_flush" : "flush");
        }

        if (mDemuxer)
            mDemuxer->readyToFlush();

        if (NULL != mAVPipelineMgr){
            mAVPipelineMgr->flush();
        }

        if (mDemuxer)
            mDemuxer->flush();

        if (mSource)
            mSource->Flush();

        if (seek_flush){
            AGILE_LOGV("see if the seeking is complete or not?");
            Mag_WaitForEventGroup(mSeekEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            AGILE_LOGV("seeking completes");
        }else{
            Mag_SetEvent(mCompleteFlushEvt);
        }

        resumeTo();
        mState = mFlushBackState;
    }else{
        if ((ST_FLUSHING == mState) || (ST_BUFFERING == mState)){
            AGILE_LOGW("in state: %s. ignore the flush action!", state2String(mState));
        }else{
            AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
            // mState = ST_ERROR;
            // sendErrorEvent(E_WHAT_COMMAND, MagMsg_Flush);
        }
        return;
    }
}

_status_t MagPlayer::flush(){
    if (mFlushMsg != NULL){
        mFlushMsg->setInt32(mFlushMsg, "seek_flush", 0);
        mFlushMsg->postMessage(mFlushMsg, 0);
    }else{
        AGILE_LOGE("the mFlushMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onSeek(MagMessageHandle msg){
    boolean ret;
    i32 seekTarget;
    i32 do_flush;

    if (isValidFSState(mState)){
        ret = msg->findInt32(msg, "seek_to", &seekTarget);
        if (!ret){
            AGILE_LOGE("failed to find the seek_to object!");
            return;
        }else{
            AGILE_LOGV("seek to %d ms", seekTarget);
        }

        if (ST_PREPARED == mState){
            /*don't need to flush the player in prepared state!*/
            do_flush = 0;
        }else{
            ret = msg->findInt32(msg, "do_flush", &do_flush);
            if (!ret){
                AGILE_LOGE("failed to find the do_flush object!");
                return;
            }else{
                AGILE_LOGV("%s flushing before seeking", do_flush ? "Do" : "No");
            }
        }

        if (do_flush){
            mFlushMsg->setInt32(mFlushMsg, "seek_flush", 1);
            mFlushMsg->postMessage(mFlushMsg, 0);
        }

        if (mDemuxer){
            AGILE_LOGV("Before mDemuxer->seekTo(%d)", seekTarget);
            if (mDemuxer->seekTo(seekTarget) == MAG_NO_ERROR){
                AGILE_LOGI("seek to %d ms -- OK!", seekTarget);
            }else{
                AGILE_LOGE("Failed to do seekTo %d, ret = 0x%x. playtime(%d) back to %d",
                            seekTarget, mPlayTimeInMs, mPlayTimeBeforeSeek);
                mPlayTimeInMs = mPlayTimeBeforeSeek;
                mPlayTimeBeforeSeek = 0;
            }
        }else{
            AGILE_LOGE("no demuxer is found, quit the seekTo(%d)", seekTarget);
        }

        Mag_SetEvent(mCompleteSeekEvt);
    }else{
        if ((ST_FLUSHING == mState) || (ST_BUFFERING == mState)){
            AGILE_LOGW("in state: %s. ignore the seek action!", state2String(mState));
        }else{
            AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
            // mState = ST_ERROR;
            // sendErrorEvent(E_WHAT_COMMAND, MagMsg_SeekTo);
        }
    }
}

_status_t MagPlayer::seekTo(i32 msec){
    if (mSeekToMsg != NULL){
        AGILE_LOGV("enter: %d ms", msec);

        mPlayTimeBeforeSeek = mPlayTimeInMs;
        mPlayTimeInMs = msec;

        mSeekToMsg->setInt32(mSeekToMsg, "seek_to", msec);
        mSeekToMsg->setInt32(mSeekToMsg, "do_flush", 1);
        mSeekToMsg->postMessage(mSeekToMsg, 0);
    }else{
        AGILE_LOGE("the mFlushMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onFast(MagMessageHandle msg){
    if ((ST_PREPARED == mState) ||
        (ST_PLAYING == mState)){
        mFastBackState = mState;
        /*todo*/
        mState = ST_FASTING;
    }else{
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        mState = ST_ERROR;
        sendErrorEvent(E_WHAT_COMMAND, MagMsg_Fast);
        return;
    }
}

_status_t MagPlayer::fast(i32 speed){
    if (mFastMsg != NULL){
        mFastMsg->setInt32(mFastMsg, "speed", speed);
        mFastMsg->postMessage(mFastMsg, 0);
    }else{
        AGILE_LOGE("the mFastMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onReset(MagMessageHandle msg){
    doResetAction();

    mLeftVolume     = -1.0;
    mRightVolume    = -1.0;

    mbIsPlayed      = false;
    mState          = ST_IDLE;
    mFlushBackState = ST_IDLE;
    mSeekBackState  = ST_IDLE;
}

_status_t MagPlayer::reset(){
    AGILE_LOGD("enter!");
    /*
     * In preparing state, the low level demuxing is block calls and might be time consuming.
     * If user reset the player in the middle of the preparing stage, we need to have way to abort the 
     * on-going demuxing action now. 
     */
    if (mState == ST_PREPARING){
        mLooper->clear(mLooper);
        mDemuxer->abort();
    }

    if (mResetMsg != NULL){
        mResetMsg->postMessage(mResetMsg, 0);
    }else{
        AGILE_LOGE("the mResetMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagPlayer::onSetVolume(MagMessageHandle msg){

}

_status_t MagPlayer::setVolume(float leftVolume, float rightVolume){
    // if (mSetVolumeMsg != NULL){
    //     mSetVolumeMsg->setFloat(mSetVolumeMsg, "left", leftVolume);
    //     mSetVolumeMsg->setFloat(mSetVolumeMsg, "right", rightVolume);
    //     mSetVolumeMsg->postMessage(mSetVolumeMsg, 0);
    // }else{
    //     AGILE_LOGE("the mResetMsg is NULL");
    //     return MAG_BAD_VALUE;
    // }
    if ((mState >= ST_PREPARED ) &&
         (mState != ST_ERROR)){
        AGILE_LOGV("enter[lv=%f, rv=%f]!", leftVolume, rightVolume);
        if (mAVPipelineMgr)
            mAVPipelineMgr->setVolume(leftVolume, rightVolume);
    }else{
        mLeftVolume = leftVolume;
        mRightVolume = rightVolume;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::resumeTo(){
    _status_t ret;

    AGILE_LOGV("enter!");
    ret = mDemuxer->resume();

    if (ret == MAG_NO_ERROR){

        if (mAVPipelineMgr != NULL){
            mAVPipelineMgr->resume();
        }
    }
    AGILE_LOGV("exit!");
    return MAG_NO_ERROR;
}


void MagPlayer::setSeekCompleteListener(fnNotifySeekComplete fn, void *priv){
    if (NULL != fn){
        mNotifySeekCompleteFn = fn;
        mSeekCompletePriv     = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setPrepareCompleteListener(fnNotifyPrepareComplete fn, void *priv){
    if (NULL != fn){
        mNotifyPrepareCompleteFn = fn;
        mPrepareCompletePriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setFlushCompleteListener(fnNotifyFlushComplete fn, void *priv){
    if (NULL != fn){
        mNotifyFlushCompleteFn = fn;
        mFlushCompletePriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setInfoListener(fnNotifyInfo fn, void *priv){
    if (NULL != fn){
        mInfoFn   = fn;
        mInfoPriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::setErrorListener(fnNotifyError fn, void *priv){
    if (NULL != fn){
        mErrorFn   = fn;
        mErrorPriv = priv;
    }else{
        AGILE_LOGE("fn is NULL!");
    }
}

void MagPlayer::onSetParameters(MagMessageHandle msg){
    if ((mState == ST_IDLE) || (mState == ST_ERROR)){
        AGILE_LOGE("MagPlayer is at wrong state:%d. QUIT!", mState);
        return;
    }else{
        char *name;
        i32 t;
        MagParamType_t type;
        
        msg->findString(msg, "name", &name);
        msg->findInt32(msg, "type", &t);
        type = static_cast<MagParamType_t>(t);

        if (NULL != mParametersDB){
            switch (type){
                case MagParamTypeInt32:
                    {
                    i32 v;
                    msg->findInt32(msg, "value", &v);
                    AGILE_LOGV("name = %s, value = %d", name, v);
                    mParametersDB->setInt32(mParametersDB, name, v);

                    }
                    break;

                case MagParamTypeInt64:
                    {
                    i64 v;
                    msg->findInt64(msg, "value", &v);
                    mParametersDB->setInt64(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeUInt32:
                    {
                    ui32 v;
                    msg->findUInt32(msg, "value", &v);
                    mParametersDB->setUInt32(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeFloat:
                    {
                    fp32 v;
                    msg->findFloat(msg, "value", &v);
                    mParametersDB->setFloat(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypeDouble:
                    {
                    fp64 v;
                    msg->findDouble(msg, "value", &v);
                    mParametersDB->setDouble(mParametersDB, name, v);
                    }
                    break;

                case MagParamTypePointer:
                {
                    void *value;
                    msg->findPointer(msg, "value", &value);
                    AGILE_LOGV("[type: pointer]: name = %s, value = 0x%x", name, value);
                    mParametersDB->setPointer(mParametersDB, name, value);
                }
                    break;

                case MagParamTypeString:
                {
                    char *v;
                    msg->findString(msg, "value", &v);
                    AGILE_LOGV("[type: string]: name = %s, value = %s", name, v);
                    mParametersDB->setString(mParametersDB, name, v);

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
}

_status_t MagPlayer::setParameters(const char *name, MagParamType_t type, void *value){
    if (NULL != mSetParametersMsg){
        mSetParametersMsg->setString(mSetParametersMsg, "name", name);
        mSetParametersMsg->setInt32(mSetParametersMsg, "type", static_cast<i32>(type));
        switch (type){
            case MagParamTypeInt32:
            {
                i32 v = *(static_cast<i32 *>(value));
                mSetParametersMsg->setInt32(mSetParametersMsg, "value", v);
            }
                break;
                
            case MagParamTypeInt64:
            {
                i64 v = *(static_cast<i64 *>(value));
                mSetParametersMsg->setInt64(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypeUInt32:
            {
                ui32 v = *(static_cast<ui32 *>(value));
                mSetParametersMsg->setUInt32(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypeFloat:
            {
                fp32 v = *(static_cast<fp32 *>(value));
                mSetParametersMsg->setFloat(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypeDouble:
            {
                fp64 v = *(static_cast<fp64 *>(value));
                mSetParametersMsg->setDouble(mSetParametersMsg, "value", v);
            }
                break;

            case MagParamTypePointer:
            {
                mSetParametersMsg->setPointer(mSetParametersMsg, "value", value, MAG_TRUE);
            }
                break;

            case MagParamTypeString:
            {
                char *v = static_cast<char *>(value);
                mSetParametersMsg->setString(mSetParametersMsg, "value", v);
            }
                break;

             default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                break;
        }
        mSetParametersMsg->postMessage(mSetParametersMsg, 0);
    }else{
        AGILE_LOGE("the mSetParametersMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

_status_t MagPlayer::getParameters(const char *name, MagParamType_t type, void **value){
    boolean result;
    _status_t ret = MAG_NO_ERROR;
    
   if (NULL != mParametersDB){
        switch (type){
            case MagParamTypeInt32:
            {
                i32 v;
                result = mParametersDB->findInt32(mParametersDB, name, &v);
                if (result)
                    *(i32 *)(*value) = v;
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeInt64:
            {
                i64 v;
                result = mParametersDB->findInt64(mParametersDB, name, &v);
                if (result)
                    *(i64 *)(*value) = v;
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeUInt32:
            {
                ui32 v;
                result = mParametersDB->findUInt32(mParametersDB, name, &v);
                if (result)
                    *(ui32 *)(*value) = v;
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeFloat:
            {
                fp32 v;
                result = mParametersDB->findFloat(mParametersDB, name, &v);
                if (result)
                    *(fp32 *)(*value) = v;
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeDouble:
            {
                fp64 v;
                result = mParametersDB->findDouble(mParametersDB, name, &v);
                if (result)
                    *(fp64 *)(*value) = v;
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypePointer:
            {
                result = mParametersDB->findPointer(mParametersDB, name, value);
                if (!result)
                    ret = MAG_BAD_VALUE;
            }
                break;

            case MagParamTypeString:
            {
                char *v = NULL;
                result = mParametersDB->findString(mParametersDB, name, &v);
                AGILE_LOGD("string: %s", v);
                if (result)
                    *value = static_cast<void *>(v);
                else
                    ret = MAG_BAD_VALUE;
            }
                break;

            default:
                AGILE_LOGE("the parameter type(%d) is unrecognized!", type);
                ret = MAG_BAD_VALUE;
                break;
        }
    }else{
        AGILE_LOGE("the parameter db is NOT initialized!");
    }
    return ret;
}

bool MagPlayer::isValidFSState(State_t st){
    if ((ST_PREPARED == st) ||
        (ST_PLAYING  == st) ||
        (ST_PAUSED   == st) ||
        (ST_STOPPED  == st) ||
        (ST_PLAYBACK_COMPLETED == st)){
        return true;
    }else{
        return false;
    }
}

ui32 MagPlayer::getVersion(){
    Mag_getFrameWorkVer();
    AGILE_LOGI("%s", LIBMAGPLAYER_IDENT);
    return LIBMAGPLAYER_BUILD;
}

void MagPlayer::onComponentNotify(MagMessageHandle msg){
    boolean ret;
    i32 ivalue;
    MagMessageHandle mvalue;
    i32 what;
    i32 trackid;

    if ((ST_PREPARED >= mState) ||
        (ST_PAUSED <= mState )){
        AGILE_LOGD("discard the events while in the state: %d", mState);
        return;
    }
    
    ret = msg->findInt32(msg, "what", &what);
    if (!ret){
        AGILE_LOGE("failed to find the what int32!");
        return;
    }

    if (what == MagPlayer::kWhatFillThisBuffer){
        ret = msg->findInt32(msg, "track-idx", &ivalue);
        if (!ret){
            AGILE_LOGE("failed to find the track-idx int32!");
            return;
        }

        ret = msg->findMessage(msg, "reply", &mvalue);
        if (!ret){
            AGILE_LOGE("failed to find the reply message!");
            return;
        }
        
        if (NULL != mDemuxerNotify){
            AGILE_LOGV("what=kWhatFillThisBuffer, track-idx=%d", ivalue);
            mDemuxerNotify->setInt32(mDemuxerNotify, "what", MagDemuxerBase::kWhatReadFrame);
            mDemuxerNotify->setInt32(mDemuxerNotify, "track-idx", ivalue);
            mDemuxerNotify->setMessage(mDemuxerNotify, "reply", mvalue, MAG_FALSE);
            mDemuxerNotify->postMessage(mDemuxerNotify, 0);
        }      
    }else if (what == MagPlayer::kWhatPlayComplete){
        ret = msg->findInt32(msg, "track-id", &trackid);
        if (!ret){
            AGILE_LOGE("failed to find the track-id int32!");
            return;
        }

        if (mTrackTable->trackTableList[trackid]->status == TRACK_PLAY){
            mTrackTable->trackTableList[trackid]->status = TRACK_PLAY_COMPLETE;
        }else{
            AGILE_LOGE("the track[st: %d] is not in playing status, but get playback complete msg!", 
                        mTrackTable->trackTableList[trackid]->status);
        }


        i32 totalTrack = mTrackTable->totalTrackNum;
        i32 i;
#if 1        
        for (i = 0; i < totalTrack; i++){
            if (mTrackTable->trackTableList[i]->status == TRACK_PLAY){
                AGILE_LOGD("track %d is still playing", i);
                break;
            }
        }
#else
        /*If any of the track is complete, it indicates the whole stream playback is complete.*/
        i = totalTrack;
#endif
        if (i == totalTrack){
            AGILE_LOGD("******All tracks playback are complete!******");
            mState = ST_PLAYBACK_COMPLETED;
            for (i = 0; i < totalTrack; i++){
                if (mTrackTable->trackTableList[i]->status == TRACK_PLAY_COMPLETE){
                    mTrackTable->trackTableList[i]->status = TRACK_PLAY;
                }
            }

            stop();

            if (( NULL != mInfoFn) && (NULL != mInfoPriv)){
                mInfoFn(mInfoPriv, MEDIA_INFO_PLAY_COMPLETE, 0);
            }
        }
    }
}

void MagPlayer::onBufferObserverNotify(MagMessageHandle msg){
    char *command;
    boolean ret;

    ret = msg->findString(msg, "command", &command);
    if (!ret){
        AGILE_LOGE("failed to find the command string!");
        return;
    }

    AGILE_LOGD("get the command %s", command);
    if (!strcmp(command, "play")){
        mPlayMsg->setInt32(mPlayMsg, "buffering_play", 1);
        mPlayMsg->postMessage(mPlayMsg, 0);
        if (( NULL != mInfoFn) && (NULL != mInfoPriv)){
            mInfoFn(mInfoPriv, MEDIA_INFO_BUFFERING_REPORT, 100);
        }
    }else if(!strcmp(command, "pause")){
        mPauseMsg->setInt32(mPauseMsg, "buffering_pause", 1);
        mPauseMsg->postMessage(mPauseMsg, 0);
        if ((mReportOutBufStatus) && ( NULL != mInfoFn) && (NULL != mInfoPriv)){
            mInfoFn(mInfoPriv, MEDIA_INFO_BUFFERING_REPORT, 0);
        }
    }else if(!strcmp(command, "buffer_status")){
        ui32 percentage;
        ret = msg->findUInt32(msg, "percentage", &percentage);
        if (!ret){
            AGILE_LOGE("failed to find the percentage value!");
            return;
        }
        if ((mReportOutBufStatus) && ( NULL != mInfoFn) && (NULL != mInfoPriv)){
            mInfoFn(mInfoPriv, MEDIA_INFO_BUFFERING_REPORT, percentage);
        }
    }else{
        AGILE_LOGE("The unsupported command %s", command);
    }
}

void MagPlayer::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    
    switch (msg->what(msg)) {
        case MagMsg_SourceNotify:
            thiz->onSourceNotify(msg);
            break;
            
        case MagMsg_Prepare:
            thiz->onPrepared(msg);
            break;

        case MagMsg_Start:
            thiz->onStart(msg);
            break;

        case MagMsg_Play:
            thiz->onPlay(msg);
            break;

        case MagMsg_Stop:
            thiz->onStop(msg);
            break;

        case MagMsg_Pause:
            thiz->onPause(msg);
            break;

        case MagMsg_Flush:
            thiz->onFlush(msg);
            break;

        case MagMsg_SeekTo:
            thiz->onSeek(msg);
            break;
            
        case MagMsg_Fast:
            thiz->onFast(msg);
            break;
            
        case MagMsg_SetVolume:
            thiz->onSetVolume(msg);
            break;

        case MagMsg_Reset:
            thiz->onReset(msg);
            break;
            
        case MagMsg_ErrorNotify:
            thiz->onErrorNotify(msg);
            break;

        case MagMsg_ComponentNotify:
            thiz->onComponentNotify(msg);
            break;
            
        case MagMsg_SetParameters:
            thiz->onSetParameters(msg);
            break;
        
        case MagMsg_BufferObserverNotify:
            thiz->onBufferObserverNotify(msg);
            break;

        default:
            break;
    }
}

void MagPlayer::onSeekMessageReceived(const MagMessageHandle msg, void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    
    switch (msg->what(msg)) {
        case MagMsg_SeekTo:
            thiz->onSeek(msg);
            break;

        default:
            break;
    }
}

void MagPlayer::onCompletePrepareEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    // MagMessageHandle msg = NULL;

    if (NULL == thiz)
        return;

    AGILE_LOGI("enter!");
#if 0
    /*must set the state firstly and then proceed the messages in the waiting queue. Otherwise it might cause dead lock*/
    thiz->mState = ST_PREPARED;
    do {
        msg = NULL;
        thiz->mPrepareCompleteMQ->get(thiz->mPrepareCompleteMQ, &msg);
        if (NULL != msg){
            AGILE_LOGI("send out message: %d", msg->what(msg));
            msg->postMessage(msg, 0);
        }
    }while(msg);
#endif    
    if ((thiz->mNotifyPrepareCompleteFn != NULL) && (thiz->mPrepareCompletePriv != NULL))
        thiz->mNotifyPrepareCompleteFn(thiz->mPrepareCompletePriv);
}

void MagPlayer::onCompleteSeekEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    // MagMessageHandle msg = NULL;

    if (NULL == thiz)
        return;
    
    AGILE_LOGI("enter!");
    
    if ((thiz->mNotifySeekCompleteFn != NULL) && (thiz->mSeekCompletePriv != NULL))
        thiz->mNotifySeekCompleteFn(thiz->mSeekCompletePriv);

#if 0    
    do {
        msg = NULL;
        thiz->mSeekCompleteMQ->get(thiz->mSeekCompleteMQ, &msg);
        if (NULL != msg){
            AGILE_LOGI("send out message: %d", msg->what(msg));
            msg->postMessage(msg, 0);
        }
    }while(msg);
#endif
}

void MagPlayer::onCompleteFlushEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;
    // MagMessageHandle msg = NULL;

    if (NULL == thiz)
        return;

    AGILE_LOGI("enter!");
    // if (thiz->isValidFSState(thiz->mFlushBackState)){
    //     thiz->mState = thiz->mFlushBackState;
    //     thiz->resumeTo();
        
    if ((thiz->mNotifyFlushCompleteFn != NULL) && (thiz->mFlushCompletePriv != NULL))
        thiz->mNotifyFlushCompleteFn(thiz->mFlushCompletePriv);

    // }else{
    //     AGILE_LOGE("backstate:%d is not valid for Flush operation. QUIT!", thiz->mFlushBackState);
    // }

#if 0
    do {
        msg = NULL;
        thiz->mFlushCompleteMQ->get(thiz->mFlushCompleteMQ, &msg);
        if (NULL != msg){
            AGILE_LOGI("send out message: %d", msg->what(msg));
            msg->postMessage(msg, 0);
        }
    }while(msg);
#endif
}

void MagPlayer::onErrorEvtCB(void *priv){
    MagPlayer *thiz = (MagPlayer *)priv;

    if (NULL == thiz)
        return;
    
    thiz->mState = ST_ERROR;
}

void MagPlayer::initialize(){
    AGILE_LOGV("Enter!");

    mbIsPlayed      = false;
    mState          = ST_IDLE;
    mFlushBackState = ST_IDLE;
    mSeekBackState  = ST_IDLE;

    mReportOutBufStatus = true;

    mLooper         = NULL;
    mMsgHandler     = NULL;
    mSeekLooper     = NULL;
    mSeekMsgHandler = NULL;
    mSource         = NULL;
    mDemuxer        = NULL;
    mTrackTable     = NULL;

    mNotifySeekCompleteFn    = NULL;
    mSeekCompletePriv        = NULL;
    mNotifyPrepareCompleteFn = NULL;
    mPrepareCompletePriv     = NULL;
    mNotifyFlushCompleteFn   = NULL;
    mFlushCompletePriv       = NULL;
    mErrorFn                 = NULL;
    mErrorPriv               = NULL;
    mInfoFn                  = NULL;
    mInfoPriv                = NULL;
    mVideoPipeline           = NULL;
    mAudioPipeline           = NULL;
    mAVPipelineMgr           = NULL;
    mClock                   = NULL;
    mpContentPipeObserver    = NULL;
    mpDemuxerStreamObserver  = NULL;

    mDemuxerNotify           = NULL;

    mLeftVolume              = -1.0;
    mRightVolume             = -1.0;

    mPlayTimeInMs            = 0;

    mParametersDB = createMagMiniDB(PARAMETERS_DB_SIZE);
        
    // mFlushCompleteMQ   = Mag_CreateMsgQueue();
    // mSeekCompleteMQ    = Mag_CreateMsgQueue();
    // mPrepareCompleteMQ = Mag_CreateMsgQueue();

    Mag_CreateMutex(&mGetParamLock);
    Mag_CreateMutex(&mStateLock);

    Mag_CreateEventGroup(&mEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mCompletePrepareEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mCompletePrepareEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mCompleteFlushEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mCompleteFlushEvt);

    if (MAG_ErrNone == Mag_CreateEvent(&mErrorEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mErrorEvt);
    
    Mag_CreateEventGroup(&mSeekEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mCompleteSeekEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mSeekEventGroup, mCompleteSeekEvt);
    
    Mag_CreateEventScheduler(&mEventScheduler, MAG_EVT_SCHED_NORMAL);
    
    Mag_RegisterEventCallback(mEventScheduler, mCompletePrepareEvt, onCompletePrepareEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mCompleteSeekEvt, onCompleteSeekEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mCompleteFlushEvt, onCompleteFlushEvtCB, (void *)this);
    Mag_RegisterEventCallback(mEventScheduler, mErrorEvt, onErrorEvtCB, (void *)this);

    AGILE_LOGV("mCompleteFlushEvt = 0x%p, mCompleteSeekEvt = 0x%p", 
                mCompleteFlushEvt, mCompleteSeekEvt);

    mSourceNotifyMsg         = createMessage(MagMsg_SourceNotify);
    mPrepareMsg              = createMessage(MagMsg_Prepare);
    mStartMsg                = createMessage(MagMsg_Start);
    mPlayMsg                 = createMessage(MagMsg_Play);
    mStopMsg                 = createMessage(MagMsg_Stop);
    mPauseMsg                = createMessage(MagMsg_Pause);
    mFlushMsg                = createMessage(MagMsg_Flush);
    mFastMsg                 = createMessage(MagMsg_Fast);
    mResetMsg                = createMessage(MagMsg_Reset);
    mSetVolumeMsg            = createMessage(MagMsg_SetVolume);
    mErrorNotifyMsg          = createMessage(MagMsg_ErrorNotify);
    // mComponentNotifyMsg      = createMessage(MagMsg_ComponentNotify);
    mSetParametersMsg        = createMessage(MagMsg_SetParameters);
    mBufferObserverNotifyMsg = createMessage(MagMsg_BufferObserverNotify);

    mSeekToMsg               = createSeekMessage(MagMsg_SeekTo);
}

void MagPlayer::sendErrorEvent(i32 what, i32 extra){
    if (mErrorNotifyMsg){
        mErrorNotifyMsg->setInt32(mErrorNotifyMsg, "what", what);
        mErrorNotifyMsg->setInt32(mErrorNotifyMsg, "extra", extra);
        mErrorNotifyMsg->postMessage(mErrorNotifyMsg, 0);
    }else{
        AGILE_LOGE("failure because mErrorNotifyMsg is null");
    }
}

MagMessageHandle MagPlayer::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

MagMessageHandle MagPlayer::createSeekMessage(ui32 what) {
    getSeekLooper();
    
    MagMessageHandle msg = createMagMessage(mSeekLooper, what, mSeekMsgHandler->id(mSeekMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

void MagPlayer::cleanup(){
    AGILE_LOGV("enter!");

    if (mVideoPipeline){
        delete mVideoPipeline;
        mVideoPipeline = NULL;
    }

    if (mAudioPipeline){
        delete mAudioPipeline;
        mAudioPipeline = NULL;
    }

    if (mClock){
        delete mClock;
        mClock = NULL;
    }

    if (mSource){
        delete mSource;
        mSource = NULL;
    }

    mbIsPlayed      = false;
    mState          = ST_IDLE;
    mFlushBackState = ST_IDLE;
    mSeekBackState  = ST_IDLE;

    delete mDemuxer;
    mDemuxer = NULL;

    if (mpContentPipeObserver){
        delete mpContentPipeObserver;
        mpContentPipeObserver = NULL;
    }

    if (mpDemuxerStreamObserver){
        delete mpDemuxerStreamObserver;
        mpDemuxerStreamObserver = NULL;
    }

    MagPipelineFactory &factory = MagPipelineFactory::getInstance();
    delete dynamic_cast<MagPipelineFactory *>(&factory);

    AGILE_LOGV("step 1");
    destroyMagMiniDB(&mParametersDB);
    // Mag_DestroyMsgQueue(mFlushCompleteMQ);
    // Mag_DestroyMsgQueue(mSeekCompleteMQ);
    // Mag_DestroyMsgQueue(mPrepareCompleteMQ);
    AGILE_LOGV("step 2");
    Mag_DestroyMutex(&mStateLock);

    Mag_DestroyEvent(&mCompletePrepareEvt);
    Mag_DestroyEvent(&mCompleteSeekEvt);
    Mag_DestroyEvent(&mCompleteFlushEvt);
    Mag_DestroyEvent(&mErrorEvt);

    Mag_DestroyEventGroup(&mEventGroup);
    Mag_DestroyEventGroup(&mSeekEventGroup);

    Mag_DestroyEventScheduler(&mEventScheduler);
    AGILE_LOGV("step 3");
    mSourceNotifyMsg->setPointer(mSourceNotifyMsg, "source", NULL, MAG_FALSE);
    destroyMagMessage(&mSourceNotifyMsg);
    destroyMagMessage(&mPrepareMsg);
    destroyMagMessage(&mStartMsg);
    destroyMagMessage(&mStopMsg);
    destroyMagMessage(&mPauseMsg);
    destroyMagMessage(&mFlushMsg);
    destroyMagMessage(&mFastMsg);
    destroyMagMessage(&mResetMsg);
    destroyMagMessage(&mSetVolumeMsg);
    destroyMagMessage(&mErrorNotifyMsg);
    // destroyMagMessage(mComponentNotifyMsg);
    destroyMagMessage(&mDemuxerNotify);
    destroyMagMessage(&mSetParametersMsg);
    AGILE_LOGV("step 4");
    destroyMagMessage(&mSeekToMsg);
    AGILE_LOGV("step 5");
    destroyLooper(&mLooper);
    destroyHandler(&mMsgHandler);
    AGILE_LOGV("step 6");
    destroyLooper(&mSeekLooper);
    destroyHandler(&mSeekMsgHandler);
    AGILE_LOGV("exit!");
}

_status_t MagPlayer::getLooper(){
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

_status_t MagPlayer::getSeekLooper(){
    if ((NULL != mSeekLooper) && (NULL != mSeekMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == mSeekLooper){
        mSeekLooper = createLooper(SEEK_LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", mSeekLooper);
    }
    
    if (NULL != mSeekLooper){
        if (NULL == mSeekMsgHandler){
            mSeekMsgHandler = createHandler(mSeekLooper, onSeekMessageReceived, (void *)this);

            if (NULL != mSeekMsgHandler){
                mSeekLooper->registerHandler(mSeekLooper, mSeekMsgHandler);
                mSeekLooper->setMergeMsg(mSeekLooper);
                mSeekLooper->start(mSeekLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", SEEK_LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}


void MagPlayer::setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h){
    AGILE_LOGD("x = %d, y = %d, w = %d, h = %d", x, y, w, h);
}

void MagPlayer::getBufferStatus(BufferStatistic_t  *pBufSt){
    ui32 videoBufSt = 0;
    ui32 audioBufSt = 0;
    ui32 loadingSpeed = 0;

    if ((ST_PLAYING == mState) ||
        (ST_BUFFERING == mState) ||
        (ST_PAUSED    == mState)){
        if (mDemuxer){
           mDemuxer->getAVBufferStatus(&videoBufSt, &audioBufSt, &loadingSpeed);
        }
    }

    pBufSt->video_buffer_time = videoBufSt;
    pBufSt->audio_buffer_time = audioBufSt;
    pBufSt->loadingSpeed      = loadingSpeed;
}

_status_t MagPlayer::buildAudioPipeline(){
    return MAG_NO_ERROR;
}
_status_t MagPlayer::buildVideoPipeline(){
    return MAG_NO_ERROR;
}

#undef LOOPER_NAME


