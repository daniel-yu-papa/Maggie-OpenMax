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

#include "MagAudioPipelineImpl.h"
#include "MagPlayerCommon.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline"

#define LOOPER_NAME "MagAudioPipeline"

#define AUDIO_ES_DUMP_FILE_NAME "/lmp/magplayer_audio.es"

MagAudioPipelineImpl::MagAudioPipelineImpl():
                                      mLooper(NULL),
                                      mMsgHandler(NULL),
                                      mMagPlayerNotifier(NULL),
                                      mPostFillBufCnt(0){ 
    AGILE_LOGV("Constructor!");
    mState = ST_INIT;
    mIsFlushed = false;
    mEmptyThisBufferMsg = createMessage(MagAudioPipeline_EmptyThisBuffer);
    mDestroyMsg = createMessage(MagAudioPipeline_Destroy);
    Mag_CreateMutex(&mhPostFillBufMutex);
}

MagAudioPipelineImpl::~MagAudioPipelineImpl(){
    AGILE_LOGV("enter!");
    mLooper->clear(mLooper);
    mDestroyMsg->postMessage(mDestroyMsg, 0);
    mLooper->waitOnAllDone(mLooper);
    destroyHandler(&mMsgHandler);
    destroyLooper(&mLooper);
    Mag_DestroyMutex(&mhPostFillBufMutex);
    AGILE_LOGV("exit!");
}

void MagAudioPipelineImpl::setMagPlayerNotifier(MagMessageHandle notifyMsg){
    mMagPlayerNotifier = notifyMsg;
}

MagMessageHandle MagAudioPipelineImpl::getMagPlayerNotifier(){
    return mMagPlayerNotifier;
}

_status_t MagAudioPipelineImpl::init(i32 trackID, TrackInfo_t *sInfo){
    if (mState != ST_INIT){
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return MAG_INVALID_OPERATION;
    }
    mState = ST_STOP;
    mTrackID = trackID;

#ifdef DUMP_AUDIO_ES_FILE
    mDumpFile = fopen(AUDIO_ES_DUMP_FILE_NAME, "wb+");
    if (NULL == mDumpFile){
        AGILE_LOGE("failed to open the file: %s", AUDIO_ES_DUMP_FILE_NAME);
    }else{
        AGILE_LOGD("Create the file: %s -- OK!", AUDIO_ES_DUMP_FILE_NAME);
    }
#endif
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::setup(){
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::getDecodedFrame(void **ppAudioFrame){
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::putUsedFrame(void *pAudioFrame){
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::start(){
    AGILE_LOGV("enter!");

    if ((mState == ST_STOP) ||
        (mState == ST_PAUSE)){
        mState = ST_PLAY;
        postFillThisBuffer();
    }else{
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return MAG_INVALID_OPERATION;
    }
    AGILE_LOGV("exit!");
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::stop(){
    _status_t ret = MAG_NO_ERROR;

    if ((mState == ST_PLAY) ||
        (mState == ST_PAUSE)){
        mLooper->clear(mLooper);
        
#ifdef DUMP_AUDIO_ES_FILE
        if (mDumpFile){
            AGILE_LOGD("Close the file -- OK!");
            fclose(mDumpFile);
            mDumpFile = NULL;
        }
#endif
    }else{
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        ret = MAG_INVALID_OPERATION;
    }
    mState = ST_STOP;
    return ret;
}

_status_t MagAudioPipelineImpl::pause(){
    if (mState == ST_PLAY){
        AGILE_LOGD("do pause!");
        mState = ST_PAUSE;
    }else{
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return MAG_INVALID_OPERATION;
    }
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::resume(){
    /*if (mIsFlushed){
        mIsFlushed = false;
        if (mState == ST_PLAY){
            postFillThisBuffer();
        }else{
            AGILE_LOGD("[flush to resume] in state: %s, keep it!", state2String(mState));
        }
    }else{*/
        if ((mState == ST_PAUSE) || (mState == ST_PLAY)){
            mState = ST_PLAY;
            postFillThisBuffer();
        }else{
            AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
            return MAG_INVALID_OPERATION;
        }
    /*}*/
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::flush(){
    if ((mState == ST_PLAY) ||
        (mState == ST_PAUSE)){
        mIsFlushed = true;
        mLooper->clear(mLooper);
        mLooper->waitOnAllDone(mLooper);
    }
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::reset(){
    mState = ST_INIT;
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::getClkConnectedComp(i32 *port, void **ppComp){
    return MAG_NO_ERROR;
}

_status_t MagAudioPipelineImpl::setVolume(fp32 leftVolume, fp32 rightVolume){
    return MAG_NO_ERROR;
}

void MagAudioPipelineImpl::proceedMediaBuffer(MagOmxMediaBuffer_t *buf){
#ifdef DUMP_AUDIO_ES_FILE
    if (NULL != mDumpFile){
        fwrite(buf->buffer, 1, buf->buffer_size, mDumpFile);
    }
#endif
    pushEsPackets(buf);
    /*buf->release(buf);*/
}

void MagAudioPipelineImpl::notifyPlaybackComplete(){
    /*simulate the video playback complete and then notify the APP*/
    mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "what", PIPELINE_NOTIFY_PlaybackComplete);
    mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "track-id", mTrackID);
    mMagPlayerNotifier->setMessage(mMagPlayerNotifier, "reply", mEmptyThisBufferMsg, MAG_FALSE);
    mMagPlayerNotifier->postMessage(mMagPlayerNotifier, 0);
}

void MagAudioPipelineImpl::onEmptyThisBuffer(MagMessageHandle msg){
    boolean ret;
    void *value;
    MagOmxMediaBuffer_t *buf = NULL;
    char *eos = false;
    
    if ((mState == ST_STOP) ||
        (mState == ST_INIT)){
        /*even if the buffer is not NULL, do not release the buffer here and 
        leave it to the owner handling it*/
        AGILE_LOGE("it is in invalid state: (%s)! quit ...", state2String(mState));
        return;
    }

    ret = msg->findPointer(msg, "media-buffer", &value);
    if (!ret){
        AGILE_LOGE("failed to find the media-buffer pointer!");
        return;
    }  

    buf = static_cast<MagOmxMediaBuffer_t *>(value);

    if (NULL != buf){
        proceedMediaBuffer(buf);
        postFillThisBuffer();
    }else{
        ret = msg->findString(msg, "eos", &eos);
        if (!ret){
            AGILE_LOGE("failed to find the eos string!");
        }  

        if (!strcmp(eos, "yes")){
            AGILE_LOGD("get EOS. finished!");
        }else{
            postFillThisBuffer();
        }
    }
}

void MagAudioPipelineImpl::onDestroy(MagMessageHandle msg){
    AGILE_LOGV("enter!");
    if (mEmptyThisBufferMsg)
        destroyMagMessage(&mEmptyThisBufferMsg);
    if (mMagPlayerNotifier)
        destroyMagMessage(&mMagPlayerNotifier);
    mEmptyThisBufferMsg = NULL;
    mMagPlayerNotifier  = NULL;
}

void MagAudioPipelineImpl::setFillBufferFlag(){
    Mag_AcquireMutex(mhPostFillBufMutex);
    mPostFillBufCnt++;
    Mag_ReleaseMutex(mhPostFillBufMutex);
}

i32 MagAudioPipelineImpl::getFillBufferFlag(){
    i32 ret;

    Mag_AcquireMutex(mhPostFillBufMutex);
    ret = mPostFillBufCnt;

    if (mPostFillBufCnt)
        mPostFillBufCnt--;
    Mag_ReleaseMutex(mhPostFillBufMutex);

    return ret;
}

void MagAudioPipelineImpl::postFillThisBuffer(){
    if (mMagPlayerNotifier != NULL){
        if ((mState == ST_PLAY) && (!mIsFlushed)){
            if (needData()){
                mMagPlayerNotifier->setInt32(mMagPlayerNotifier, "what", PIPELINE_NOTIFY_FillThisBuffer);
                mMagPlayerNotifier->setMessage(mMagPlayerNotifier, "reply", mEmptyThisBufferMsg, MAG_FALSE);
                mMagPlayerNotifier->postMessage(mMagPlayerNotifier, 0);
            }else{
                AGILE_LOGD("driver buffer is full");
                setFillBufferFlag();
            }
        }else{
            AGILE_LOGV("don't send out the fillThisBuffer event in %s state: (%s)", 
                        mIsFlushed ? "flushed" : "none-Play",
                        state2String(mState));
        }
    }else{
        AGILE_LOGE("mMagPlayerNotifier is not setting!");
    }
}

void MagAudioPipelineImpl::onMessageReceived(const MagMessageHandle msg, void *priv){
    MagAudioPipelineImpl *thiz = (MagAudioPipelineImpl *)priv;
    
    switch (msg->what(msg)) {
        case MagAudioPipeline_EmptyThisBuffer:
            thiz->onEmptyThisBuffer(msg);
            break;

        case MagAudioPipeline_Destroy:
            thiz->onDestroy(msg);
            break;

        default:
            break;
    }
}

MagMessageHandle MagAudioPipelineImpl::createMessage(ui32 what) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t MagAudioPipelineImpl::getLooper(){
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