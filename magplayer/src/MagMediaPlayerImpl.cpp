/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MagMediaPlayerImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MMP_Main"


#define LOOPER_NAME "MagPlayerLooper"
#define SEEK_LOOPER_NAME "MagPlayerSeekLooper"

#define DEFAULT_BUFFER_SIZE (2*1024*1024)
#define DEFAULT_BUFFER_TIME (30*1000)
#define DEFAULT_BUFFER_HIGH_LEVEL (99)
#define DEFAULT_BUFFER_LOW_LEVEL  (20)

#define VIDEO_PORT_BUFFER_NUMBER 16

MAG_SINGLETON_STATIC_INSTANCE(MagMediaPlayerImpl)

MagMediaPlayer* GetMediaPlayer()
{
    MagMediaPlayerImpl& inst = MagMediaPlayerImpl::getInstance();

    return dynamic_cast<MagMediaPlayer *>(&inst);
}


MagMediaPlayerImpl::MagMediaPlayerImpl(){
    AGILE_LOG_CREATE();
    initialize();
}

MagMediaPlayerImpl::~MagMediaPlayerImpl(){
    mLooper->clear(mLooper);
    reset();
    mLooper->waitOnAllDone(mLooper);
    cleanup();

    AGILE_LOGD("exit!");
    AGILE_LOG_DESTROY();
}

OMX_ERRORTYPE MagMediaPlayerImpl::playbackPipelineEventHandler(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 Data1,
                                            OMX_U32 Data2,
                                            OMX_PTR pEventData){
    MagMediaPlayerImpl *thiz;

    AGILE_LOGV("playbackPipeline event: %d", eEvent);
    thiz = static_cast<MagMediaPlayerImpl *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                case OMX_StateIdle:
                case OMX_StateExecuting:
                case OMX_StatePause:
                case OMX_StateWaitForResources:
                    Mag_SetEvent(thiz->mPipelineStateEvent);
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Vdec component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Vdec component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Vdec component flushes port %d is done!", Data2);
            Mag_SetEvent(thiz->mPipelineFlushDoneEvent);
        }
    }else if (eEvent == OMX_EventBufferFlag){
        thiz->mBufferLevel = Data1;
        thiz->mBufferTime  = Data2;
        if (thiz->mBufferLevel >= thiz->mPlaybackPipelineConfig.buffer_play_threshold){
            if (thiz->mState == ST_BUFFERING){
                thiz->mPlayMsg->postMessage(thiz->mPlayMsg, 0);
            }
        }else if (thiz->mBufferLevel < thiz->mPlaybackPipelineConfig.buffer_low_threshold){
            if (thiz->mState == ST_PLAYING){
                thiz->mState = ST_BUFFERING;
                thiz->doPauseAction();
            }
        }

        thiz->sendEvent(MMP_EVENT_BUFFER_STATUS, thiz->mBufferLevel, thiz->mBufferTime);
    }else if (eEvent == (OMX_EVENTTYPE)OMX_EventExtAVStreamInfo){
        OMX_DEMUXER_STREAM_INFO *streamInfo;
        StreamInfo_t *item;

        streamInfo = static_cast<OMX_DEMUXER_STREAM_INFO *>(pEventData);
        item = (StreamInfo_t *)mag_mallocz(sizeof(StreamInfo_t));
        item->streamMetaData = (mmp_meta_data_t *)mag_mallocz(sizeof(mmp_meta_data_t));
        INIT_LIST(&item->node);

        thiz->fillStreamMetaData(item->streamMetaData, streamInfo);
        item->streamInfo = streamInfo;
        item->streamTableId = thiz->mStreamTotal++;
        list_add_tail(&item->node, &thiz->mStreamList);

        thiz->sendEvent(MMP_EVENT_NEW_STREAM_REPORT, item->streamTableId, 0);
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MagMediaPlayerImpl::playbackPipelineEmptyBufferDone(
                                            OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_BUFFERHEADERTYPE* pBuffer){
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MagMediaPlayerImpl::playbackPipelineFillBufferDone(
                                            OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_PTR pAppData,
                                            OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){
    MagMediaPlayerImpl *thiz;
    OMX_BUFFERHEADERTYPE *pBufHeader;
    OMX_ERRORTYPE ret;

    thiz = static_cast<MagMediaPlayerImpl *>(pAppData);

    if (pBuffer->pBuffer){
        thiz->mpVDecodedBufferMgr->put(pBuffer);
        pBufHeader = thiz->mpPipelineBufferMgr->get();
        if (pBufHeader){
            pBufHeader->pBuffer = NULL;
            ret = OMX_FillThisBuffer(thiz->mhPlaybackPipeline, pBufHeader);
            if (ret != OMX_ErrorNone){
                AGILE_LOGE("Do OMX_FillThisBuffer() - Failed!");
            }else{
                AGILE_LOGV("Do OMX_FillThisBuffer() - OK!");
            }
        }
    }else{
        /*it is EOS frame, directly put it into the feedVren queue*/
        thiz->mpPipelineBufferMgr->put(pBuffer);
    }

    AGILE_LOGI("put buffer header: %p, buffer: %p", pBuffer, pBuffer->pBuffer);

    return OMX_ErrorNone;
}

void MagMediaPlayerImpl::fillStreamMetaData(mmp_meta_data_t *md, 
                                            OMX_DEMUXER_STREAM_INFO *streamInfo){
    if (streamInfo->type == OMX_DynamicPort_Video){
        md->metadata.vMetaData.width  = streamInfo->format.video.width;
        md->metadata.vMetaData.height = streamInfo->format.video.height;
        md->metadata.vMetaData.fps = streamInfo->format.video.fps;
        md->metadata.vMetaData.bps = streamInfo->format.video.bps;
        strcpy(md->metadata.vMetaData.codec, (char *)streamInfo->codec_info);
    }else if (streamInfo->type == OMX_DynamicPort_Audio){
        md->metadata.aMetaData.hz  = streamInfo->format.audio.hz;
        md->metadata.aMetaData.bps = streamInfo->format.audio.bps;
        strcpy(md->metadata.aMetaData.codec, (char *)streamInfo->codec_info);
    }else if (streamInfo->type == OMX_DynamicPort_Subtitle){

    }
}

void MagMediaPlayerImpl::doPauseAction(){
    OMX_SendCommand(mhPlaybackPipeline, OMX_CommandStateSet, OMX_StatePause, NULL);
    Mag_WaitForEventGroup(mPipelineStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
}

void MagMediaPlayerImpl::doRunAction(){
    OMX_SendCommand(mhPlaybackPipeline, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    Mag_WaitForEventGroup(mPipelineStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
}

void MagMediaPlayerImpl::doStopAction(){
    OMX_SendCommand(mhPlaybackPipeline, OMX_CommandStateSet, OMX_StateIdle, NULL);
    Mag_WaitForEventGroup(mPipelineStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
}

void MagMediaPlayerImpl::doFlushAction(){
    OMX_SendCommand(mhPlaybackPipeline, OMX_CommandFlush, OMX_ALL, NULL);
    Mag_WaitForEventGroup(mPipelineFlushDoneEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
}

void MagMediaPlayerImpl::doSeekAction(i32 target){

}

void MagMediaPlayerImpl::doResetAction(){
    AGILE_LOGD("enter!");

    if ( ST_PLAYING   == mState ||
         ST_PAUSED    == mState ||
         ST_BUFFERING == mState ||
         ST_FASTING   == mState ){
        doStopAction();
    }

    OMX_SendCommand(mhPlaybackPipeline, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    Mag_WaitForEventGroup(mPipelineStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);

    OMX_FreeHandle(mhPlaybackPipeline);
    mhPlaybackPipeline = NULL;

    Mag_ClearEvent(mCompletePrepareEvt);
    Mag_ClearEvent(mPipelineStateEvent);
    Mag_ClearEvent(mPipelineFlushDoneEvent);

    AGILE_LOGD("exit!");
}

void MagMediaPlayerImpl::onSourceNotify(MagMessageHandle msg){
    char *type;
    boolean ret;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    char compName[128];

    AGILE_LOGV("enter!");
    
    if (mState != ST_IDLE){
        AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        sendEvent(MMP_EVENT_ERROR, E_WHAT_DATASOURCE, E_EXTRA_WRONG_STATE);
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

        err = OMX_ComponentOfRoleEnum((OMX_STRING)compName, OMX_ROLE_PIPELINE_PLAYBACK, 1);
        if (err == OMX_ErrorNone){
            err = OMX_GetHandle(&mhPlaybackPipeline, compName, (OMX_PTR)this, &mPlaybackPipelineCb);
            if(err != OMX_ErrorNone) {
                AGILE_LOGE("OMX_GetHandle(%s) is failure", compName);
                sendEvent(MMP_EVENT_ERROR, E_WHAT_DATASOURCE, E_EXTRA_INVALID_PIPELINE);
                return;
            }
        }else{
            AGILE_LOGE("OMX_ComponentOfRoleEnum(role: %s) failed", OMX_ROLE_PIPELINE_PLAYBACK);
            sendEvent(MMP_EVENT_ERROR, E_WHAT_DATASOURCE, E_EXTRA_INVALID_PIPELINE);
            return;
        }

        mPlaybackPipelineConfig.url = mag_strdup(url);
    }else if(!strcmp(type, "file")){

    }else if(!strcmp(type, "source")){

    }else{
        AGILE_LOGE("type: %s is wrong!", type);
        return;
    }

    mState = ST_INITIALIZED;
}

int MagMediaPlayerImpl::setDataSource(const char *url){
    getLooper();

    AGILE_LOGD("url: %s", url);

    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "type", "path");
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "url", url);
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::setDataSource(unsigned int fd, signed long long offset, signed long long length){
    getLooper();

    if (mSourceNotifyMsg != NULL){
        mSourceNotifyMsg->setString(mSourceNotifyMsg, "type", "file");
        mSourceNotifyMsg->setInt32(mSourceNotifyMsg, "fd", fd);
        mSourceNotifyMsg->setInt64(mSourceNotifyMsg, "offset", offset);
        mSourceNotifyMsg->setInt64(mSourceNotifyMsg, "length", length);
        mSourceNotifyMsg->postMessage(mSourceNotifyMsg, 0);
    }else{
        AGILE_LOGE("the mSourceNotifyMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onPrepared(MagMessageHandle msg){
    OMX_PORT_PARAM_TYPE plPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    i32 voutportIdx;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_U32 i;

    if (mState == ST_INITIALIZED){
        if ( mPlaybackPipelineConfig.url == NULL ){
            AGILE_LOGE("the datasource url is not set");
            sendEvent(MMP_EVENT_ERROR, E_WHAT_PREPARE, E_EXTRA_INVALID_PARAMETER);
            return;
        }

        err = OMX_SetParameter(mhPlaybackPipeline, (OMX_INDEXTYPE)OMX_IndexParamPlaybackPipelineSetting, &mPlaybackPipelineConfig);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("Error in setting playback pipeline OMX_IndexParamPlaybackPipelineSetting parameter");
            sendEvent(MMP_EVENT_ERROR, E_WHAT_PREPARE, E_EXTRA_UNKNOWN);
            return;
        }

        if (mReturnDecodedVideo){
            /*it is configured to get back the decoded video frame*/
            initHeader(&portDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            initHeader(&plPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            err = OMX_GetParameter(mhPlaybackPipeline, OMX_IndexParamOtherInit, &plPortParam);
            if(err != OMX_ErrorNone){
                AGILE_LOGE("Error in getting playback pipeline OMX_PORT_PARAM_TYPE parameter");
                sendEvent(MMP_EVENT_ERROR, E_WHAT_PREPARE, E_EXTRA_UNKNOWN);
                return;
            }

            AGILE_LOGD("get playback pipeline param(OMX_IndexParamOtherInit): StartPortNumber-%d, Ports-%d",
                        plPortParam.nStartPortNumber, plPortParam.nPorts);

            for (i = plPortParam.nStartPortNumber; i < plPortParam.nStartPortNumber + plPortParam.nPorts; i++){
                portDef.nPortIndex = i;
            
                if (portDef.eDir == OMX_DirOutput && portDef.eDomain == OMX_PortDomainVideo){
                    err = OMX_GetParameter(mhPlaybackPipeline, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in getting playback pipeline output port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                        sendEvent(MMP_EVENT_ERROR, E_WHAT_PREPARE, E_EXTRA_UNKNOWN);
                        return;
                    }

                    portDef.nBufferCountActual = VIDEO_PORT_BUFFER_NUMBER;
                    portDef.nBufferSize        = 0; 

                    err = OMX_SetParameter(mhPlaybackPipeline, OMX_IndexParamPortDefinition, &portDef);
                    if(err != OMX_ErrorNone){
                        AGILE_LOGE("Error in setting playback pipeline output port[%d] OMX_PARAM_PORTDEFINITIONTYPE parameter", i);
                        sendEvent(MMP_EVENT_ERROR, E_WHAT_PREPARE, E_EXTRA_UNKNOWN);
                        return;
                    }

                    voutportIdx = i;
                }
            }

            if (mpVDecodedBufferMgr == NULL) {
                mpVDecodedBufferMgr = new MagBufferManager(0, VIDEO_PORT_BUFFER_NUMBER, true);
                mpVDecodedBufferMgr->create(NULL, voutportIdx, static_cast<void *>(this));
            }

            if (mpPipelineBufferMgr == NULL) {
                mpPipelineBufferMgr = new MagBufferManager(0, VIDEO_PORT_BUFFER_NUMBER, false);
                mpPipelineBufferMgr->create(mhPlaybackPipeline, voutportIdx, static_cast<void *>(this));
            }
        }
        
        OMX_SendCommand(mhPlaybackPipeline, OMX_CommandStateSet, OMX_StateIdle, NULL);

        AGILE_LOGV("Waiting for pipeline state to IDLE!");
        Mag_WaitForEventGroup(mPipelineStateEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGV("Pipeline state is IDLE!");

        mState = ST_PREPARED;
        Mag_SetEvent(mCompletePrepareEvt); 
    }else if (ST_STOPPED == mState){
        doSeekAction(0);

        mState = ST_PREPARED;
        Mag_SetEvent(mCompletePrepareEvt);

    }else{
        AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
        sendEvent(MMP_EVENT_ERROR, E_WHAT_PREPARE, E_EXTRA_WRONG_STATE);
    }  
}

int MagMediaPlayerImpl::prepareAsync(){
    AGILE_LOGD("Enter!");

    if (mPrepareMsg != NULL){
        mPrepareMsg->postMessage(mPrepareMsg, 0);
    }else{
        AGILE_LOGE("the mPrepareMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::prepare(){
    prepareAsync();

    AGILE_LOGD("before prepare waiting!");
    Mag_WaitForEventGroup(mEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("after prepare waiting!");
    
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onStart(MagMessageHandle msg){
    AGILE_LOGV("enter!");

    mState = ST_BUFFERING;
    if (ST_PLAYBACK_COMPLETED == mState){
        seekTo(0);
    }else{
        if ( !(ST_PREPARED == mState ||
               ST_PAUSED == mState) ){
            AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
            sendEvent(MMP_EVENT_ERROR, E_WHAT_START, E_EXTRA_WRONG_STATE);
        }
    }
}

int MagMediaPlayerImpl::start(){
    AGILE_LOGV("enter!");

    if (mStartMsg != NULL){
        mStartMsg->postMessage(mStartMsg, 0);
    }else{
        AGILE_LOGE("the mStartMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onPlay(MagMessageHandle msg){
    OMX_BUFFERHEADERTYPE *pBufHeader;
    OMX_ERRORTYPE ret;

    if (ST_BUFFERING == mState){
        while ( (pBufHeader = mpPipelineBufferMgr->get()) != NULL ){
            pBufHeader->pBuffer = NULL;
            ret = OMX_FillThisBuffer(mhPlaybackPipeline, pBufHeader);
            if (ret != OMX_ErrorNone){
                AGILE_LOGE("Do OMX_FillThisBuffer() - Failed!");
            }else{
                AGILE_LOGV("Do OMX_FillThisBuffer() - OK!");
            }
        }

        doRunAction();
        mState = ST_PLAYING;
    }else{
        AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
        return;
    }
}

void MagMediaPlayerImpl::onStop(MagMessageHandle msg){
    AGILE_LOGD("Enter! state=%s", state2String(mState));

    if ((ST_PLAYING == mState)   ||
        (ST_PAUSED == mState)    ||
        (ST_BUFFERING == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){
        doStopAction();
        mState = ST_STOPPED;
    }else{
        AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
    }
}

int MagMediaPlayerImpl::stop(){
    AGILE_LOGD("enter!");

    if (mStopMsg != NULL){
        mStopMsg->postMessage(mStopMsg, 0);
    }else{
        AGILE_LOGE("the mStopMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onPause(MagMessageHandle msg){
    AGILE_LOGD("Enter! state=%s", state2String(mState));

    if (ST_PLAYING == mState){
        doPauseAction();
        mState = ST_PAUSED;
    }else if (ST_BUFFERING == mState){
        mState = ST_PAUSED;
    }else if (ST_PAUSED == mState){
        AGILE_LOGV("ignore the pause action in paused state!");
    }else{
        AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
    }
}

int MagMediaPlayerImpl::pause(){
    AGILE_LOGD("enter!");

    if (mPauseMsg != NULL){
        mPauseMsg->postMessage(mPauseMsg, 0);
    }else{
        AGILE_LOGE("the mPauseMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

bool MagMediaPlayerImpl::isPlaying(){
    if ((mState == ST_PLAYING) || (mState == ST_PAUSED) || (mState == ST_BUFFERING)){
        return true;
    }
    return false;
}

int MagMediaPlayerImpl::getCurrentPosition(int* msec){
    i64 pos = 0;

    if ((mState >= ST_PREPARED) && (mState <= ST_PAUSED)){
        
    }

    if (pos < 0)
        return MAG_UNKNOWN_ERROR;
    else
        return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::getDuration(int* msec){
    if ((mState >= ST_PREPARED) && (mState <= ST_PAUSED)){
        
    }else{
        // AGILE_LOGE("MagPlayer is at wrong state: %s. QUIT!", state2String(mState));
        // return MAG_INVALID_OPERATION;
        *msec = 0;
    }
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onFlush(MagMessageHandle msg){
    if ( (ST_PLAYING  == mState) ||
         (ST_PAUSED   == mState) ||
         (ST_PLAYBACK_COMPLETED == mState) ){
        doFlushAction();
    }else if(ST_PREPARED == mState ||
             ST_STOPPED  == mState){
        AGILE_LOGV("Do nothing at the state: %s!", state2String(mState));
    }else{
        AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
    }

    sendEvent(MMP_EVENT_FLUSH_COMPLETE, 0, 0);
}

int MagMediaPlayerImpl::flush(){
    AGILE_LOGD("enter!");

    if (mFlushMsg != NULL){
        mFlushMsg->postMessage(mFlushMsg, 0);
    }else{
        AGILE_LOGE("the mFlushMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onSeek(MagMessageHandle msg){
    boolean ret;
    i32 seekTarget;

    if ((ST_PREPARED == mState) ||
        (ST_PLAYING  == mState) ||
        (ST_PAUSED   == mState) ||
        (ST_PLAYBACK_COMPLETED == mState)){

        ret = msg->findInt32(msg, "seek_to", &seekTarget);
        if (!ret){
            AGILE_LOGE("failed to find the seek_to object!");
            return;
        }else{
            AGILE_LOGV("seek to %d ms", seekTarget);
        }

        doSeekAction(seekTarget);

        sendEvent(MMP_EVENT_SEEK_COMPLETE, 0, 0);
    }
}

int MagMediaPlayerImpl::seekTo(i32 msec){
    AGILE_LOGD("%d ms", msec);

    if (mSeekToMsg != NULL){
        mFlushMsg->postMessage(mFlushMsg, 0);
        mSeekToMsg->setInt32(mSeekToMsg, "seek_to", msec);
        mSeekToMsg->postMessage(mSeekToMsg, 0);
    }else{
        AGILE_LOGE("the mFlushMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

void MagMediaPlayerImpl::onFast(MagMessageHandle msg){
    if ((ST_PREPARED == mState) ||
        (ST_PLAYING == mState)){
        /*todo*/
        mState = ST_FASTING;
    }else{
        AGILE_LOGE("At wrong state: %s. QUIT!", state2String(mState));
    }
}

int MagMediaPlayerImpl::fast(i32 speed){
    if (mFastMsg != NULL){
        mFastMsg->setInt32(mFastMsg, "speed", speed);
        mFastMsg->postMessage(mFastMsg, 0);
    }else{
        AGILE_LOGE("the mFastMsg is NULL");
        return MAG_BAD_VALUE;
    }
    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::setVolume(float leftVolume, float rightVolume){
    if ((mState >= ST_PREPARED) &&
        (mState != ST_ERROR)){
        AGILE_LOGV("enter[lv=%f, rv=%f]!", leftVolume, rightVolume);
    }else{
        mLeftVolume = leftVolume;
        mRightVolume = rightVolume;
    }
    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::reset(){
    AGILE_LOGD("enter!");
    
    mLooper->clear(mLooper);

    doResetAction();
    mState = ST_IDLE;

    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::setParameter(int key, void *request){
    switch (key){
        case MMP_PARAMETER_BUFFER_SETTING:
        {
            mmp_buffer_setting_t *config = static_cast<mmp_buffer_setting_t *>(request);

            if (config->buffer_type == 1){
                mPlaybackPipelineConfig.buffer_type = OMX_BUFFER_TYPE_FRAME;
            }else if (config->buffer_type == 0){
                mPlaybackPipelineConfig.buffer_type = OMX_BUFFER_TYPE_BYTE;
            }else{
                mPlaybackPipelineConfig.buffer_type = OMX_BUFFER_TYPE_NONE;
            }

            mPlaybackPipelineConfig.buffer_size = config->buffer_size;
            mPlaybackPipelineConfig.buffer_time = config->buffer_time;
            mPlaybackPipelineConfig.buffer_low_threshold  = config->buffer_low_level;
            mPlaybackPipelineConfig.buffer_play_threshold = config->buffer_play_level;
            mPlaybackPipelineConfig.buffer_high_threshold = config->buffer_high_level;
        }
            break;

        case MMP_PARAMETER_AVSYNC_ENABLE:
        {
            i32 *free_run = static_cast<i32 *>(request);

            mPlaybackPipelineConfig.free_run = *free_run ? OMX_TRUE : OMX_FALSE;
        }
            break;

        case MMP_PARAMETER_RETURN_VIDEO_FRAME:
        {
            i32 *config = static_cast<i32 *>(request);

            if (*config)
                mReturnDecodedVideo = true;
            else
                mReturnDecodedVideo = false;
        }
            break;

        case MMP_PARAMETER_SUBTITLE_FILE_PATH:
        {
            char *url = static_cast<char *>(request);

            mpSubtitleUrl = mag_strdup(url);
        }
            break;

        case MMP_PARAMETER_LOOPING_PLAYBACK:
        {
            i32 *loopback = static_cast<i32 *>(request);

            if (*loopback)
                mPlaybackLooping = true;
            else
                mPlaybackLooping = false;
        }
            break;

        default:
            AGILE_LOGE("Unsupported parameter[%d] setting!", key);
            break;
    }
    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::getParameter(int key, void **reply){
    return MAG_NO_ERROR;
}

int MagMediaPlayerImpl::invoke(const unsigned int methodID, void *request, void **reply){
    switch (methodID){
        case MMP_INVOKE_GET_NETWORK_BANDWIDTH:
        {

        }
            break;

        case MMP_INVOKE_READ_STREAM_META_DATA:
        {
            List_t *next;
            StreamInfo_t *si;
            i32 *streamid = static_cast<i32 *>(request);

            *reply = NULL;
            next = mStreamList.next;
            while (next != &mStreamList){
                si = (StreamInfo_t *)list_entry( next, 
                                                 StreamInfo_t, 
                                                 node);

                if (si->streamTableId == *streamid){
                    *reply = static_cast<void *>(si->streamMetaData);
                    break;
                }
            }
        }
            break;

        case MMP_INVOKE_SET_AUDIO_TRACK:
        {

        }
            break;

        case MMP_INVOKE_SET_SUBTITLE_TRACK:
        {

        }
            break;

        case MMP_INVOKE_GET_DECODED_VIDEO_FRAME:
        {
            getDecodedVideoFrame(reply);
        }
            break;

        case MMP_INVOKE_PUT_USED_VIDEO_FRAME:
        {
            putUsedVideoFrame(request);
        }
            break;
    }

    return 0;
}

ui32 MagMediaPlayerImpl::getVersion(){
    Mag_getFrameWorkVer();
    AGILE_LOGI("%s", LIBMAGPLAYER_IDENT);
    return LIBMAGPLAYER_BUILD;
}

void MagMediaPlayerImpl::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagMediaPlayerImpl *thiz = (MagMediaPlayerImpl *)priv;
    
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

        default:
            break;
    }
}

void MagMediaPlayerImpl::sendEvent(mmp_event_t evt, i32 what, i32 extra){
    if (mAppEventCallback){
        if (evt == MMP_EVENT_ERROR)
            mState = ST_ERROR;

        mAppEventCallback(evt, mAppHandler, what, extra);
    }else{
        AGILE_LOGE("Event callback is not registered");
    }
}

int MagMediaPlayerImpl::registerEventCallback(mmp_event_callback_t cb, void *handler){
    mAppEventCallback = cb;
    mAppHandler       = handler;

    return 0;
}

void MagMediaPlayerImpl::onCompletePrepareEvtCB(void *priv){
    MagMediaPlayerImpl *thiz = (MagMediaPlayerImpl *)priv;

    if (NULL == thiz)
        return;

    AGILE_LOGI("enter!");

    thiz->sendEvent(MMP_EVENT_PREPARE_COMPLETE, 0, 0);
}

MagMessageHandle MagMediaPlayerImpl::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagMediaPlayerImpl::getLooper(){
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

void MagMediaPlayerImpl::initialize(){
    OMX_ERRORTYPE err;

    AGILE_LOGV("Enter!");

    mState          = ST_IDLE;
    mLooper         = NULL;
    mMsgHandler     = NULL;
    mLeftVolume     = -1.0;
    mRightVolume    = -1.0;

    mhPlaybackPipeline = NULL;
    mAppHandler        = NULL;

    mPlaybackPipelineCb.EventHandler    = MagMediaPlayerImpl::playbackPipelineEventHandler;
    mPlaybackPipelineCb.EmptyBufferDone = MagMediaPlayerImpl::playbackPipelineEmptyBufferDone;
    mPlaybackPipelineCb.FillBufferDone  = MagMediaPlayerImpl::playbackPipelineFillBufferDone;

    mpSubtitleUrl      = NULL;

    initHeader(&mPlaybackPipelineConfig, sizeof(OMX_PLAYBACK_PIPELINE_SETTING));
    mPlaybackPipelineConfig.url         = NULL;
    mPlaybackPipelineConfig.free_run    = OMX_FALSE;
    mPlaybackPipelineConfig.buffer_type = OMX_BUFFER_TYPE_BYTE;
    mPlaybackPipelineConfig.buffer_size = DEFAULT_BUFFER_SIZE;
    mPlaybackPipelineConfig.buffer_time = -1;
    mPlaybackPipelineConfig.buffer_low_threshold  = DEFAULT_BUFFER_LOW_LEVEL;
    mPlaybackPipelineConfig.buffer_high_threshold = DEFAULT_BUFFER_HIGH_LEVEL;

    mReturnDecodedVideo  = false;

    Mag_CreateEventGroup(&mPipelineStateEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&mPipelineStateEvent, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mPipelineStateEvtGrp, mPipelineStateEvent);

    Mag_CreateEventGroup(&mPipelineFlushDoneEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&mPipelineFlushDoneEvent, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mPipelineFlushDoneEvtGrp, mPipelineFlushDoneEvent);

    Mag_CreateEventScheduler(&mEventScheduler, MAG_EVT_SCHED_NORMAL);

    Mag_CreateEventGroup(&mEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mCompletePrepareEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(mEventGroup, mCompletePrepareEvt);

    Mag_RegisterEventCallback(mEventScheduler, mCompletePrepareEvt, onCompletePrepareEvtCB, (void *)this);

    mSourceNotifyMsg         = createMessage(MagMsg_SourceNotify);
    mPrepareMsg              = createMessage(MagMsg_Prepare);
    mStartMsg                = createMessage(MagMsg_Start);
    mPlayMsg                 = createMessage(MagMsg_Play);
    mStopMsg                 = createMessage(MagMsg_Stop);
    mPauseMsg                = createMessage(MagMsg_Pause);
    mFlushMsg                = createMessage(MagMsg_Flush);
    mFastMsg                 = createMessage(MagMsg_Fast);
    mSeekToMsg               = createMessage(MagMsg_SeekTo);

    INIT_LIST(&mStreamList);

    err = OMX_Init();
    if(err != OMX_ErrorNone) {
        AGILE_LOGE("OMX_Init() failed");
    }
}

void MagMediaPlayerImpl::cleanup(){
    AGILE_LOGV("enter!");

    mState          = ST_IDLE;

    Mag_DestroyEvent(&mCompletePrepareEvt);

    Mag_DestroyEventGroup(&mEventGroup);

    Mag_DestroyEventScheduler(&mEventScheduler);
    destroyMagMessage(&mSourceNotifyMsg);
    destroyMagMessage(&mPrepareMsg);
    destroyMagMessage(&mStartMsg);
    destroyMagMessage(&mStopMsg);
    destroyMagMessage(&mPauseMsg);
    destroyMagMessage(&mFlushMsg);
    destroyMagMessage(&mFastMsg);
    destroyMagMessage(&mSeekToMsg);

    destroyLooper(&mLooper);
    destroyHandler(&mMsgHandler);
    AGILE_LOGV("exit!");
    
    OMX_Deinit();
}

void MagMediaPlayerImpl::setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h){
    AGILE_LOGD("x = %d, y = %d, w = %d, h = %d", x, y, w, h);
}

_status_t MagMediaPlayerImpl::getVideoMetaData(VideoMetaData_t *pVmd){
    return MAG_NO_ERROR;
}

_status_t MagMediaPlayerImpl::getAudioMetaData(AudioMetaData_t *pAmd){
    return MAG_NO_ERROR;
}

_status_t MagMediaPlayerImpl::getDecodedVideoFrame(void **ppVideoFrame){
    OMX_BUFFERHEADERTYPE* pBuffer;

    Mag_TimeTakenStatistic(MAG_TRUE, __FUNCTION__, NULL);
    pBuffer = mpVDecodedBufferMgr->get();
    if (pBuffer){
        Mag_TimeTakenStatistic(MAG_FALSE, __FUNCTION__, NULL);
        *ppVideoFrame = static_cast<void *>(pBuffer);
        return MAG_NO_ERROR;
    }
    AGILE_LOGV("Don't get decoded frame!");
    *ppVideoFrame = NULL;
    return MAG_NAME_NOT_FOUND;
}  

_status_t MagMediaPlayerImpl::putUsedVideoFrame(void *pVideoFrame){
    OMX_BUFFERHEADERTYPE* pBuffer;

    pBuffer = static_cast<OMX_BUFFERHEADERTYPE *>(pVideoFrame);
    AGILE_LOGV("Put used frame %p!", pBuffer);
    mpPipelineBufferMgr->put(pBuffer);

    return MAG_NO_ERROR;
}

_status_t MagMediaPlayerImpl::getDecodedAudioFrame(void **ppGetFrame){
    return MAG_NO_ERROR;
}

_status_t MagMediaPlayerImpl::putUsedAudioFrame(void *pUsedFrame){
    return MAG_NO_ERROR;
}

#undef LOOPER_NAME


