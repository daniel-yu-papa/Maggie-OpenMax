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

#include "MagPlayerDriverImpl.h"
#ifdef INTER_PROCESSES
#include "MagPlayerClient.h"
#endif
#include "MagEventType.h"
#include "MagInvokeDef.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Driver"

MagPlayerDriverImpl::MagPlayerDriverImpl(void *client, notify_client_callback_f cb):
                                                                mpClient(client),
                                                                mClientNotifyFn(cb){

    mState = MPD_ST_IDLE;
    mSetTrackIndex = 0;

    mpPlayer = new MagPlayer();
    if (NULL != mpPlayer){
        mpPlayer->setPrepareCompleteListener(PrepareCompleteEvtListener, static_cast<void *>(this));
        mpPlayer->setFlushCompleteListener(FlushCompleteEvtListener, static_cast<void *>(this));
        mpPlayer->setErrorListener(ErrorEvtListener, static_cast<void *>(this));
        mpPlayer->setInfoListener(InfoEvtListener, static_cast<void *>(this));
        mpPlayer->setSeekCompleteListener(SeekCompleteEvtListener, static_cast<void *>(this));
    }
}

MagPlayerDriverImpl::~MagPlayerDriverImpl(){
    if (mpPlayer)
        delete mpPlayer;
}

_status_t MagPlayerDriverImpl::setDataSource(const char *url){
    AGILE_LOGV("enter! url=%s", url);
    
    if (mpPlayer != NULL){
        mState = MPD_ST_INITIALIZED;
        return mpPlayer->setDataSource(url);
    }else{
        return MAG_NO_INIT;
    }
}

_status_t MagPlayerDriverImpl::setDataSource(ui32 fd, i64 offset, i64 length){
    AGILE_LOGV("enter! fd=%d, offset=%lld, length=%lld", fd, offset, length);
    
    if (mpPlayer != NULL){
        mState = MPD_ST_INITIALIZED;
        return mpPlayer->setDataSource(fd, offset, length);
    }else{
        return MAG_NO_INIT;
    }
}

#ifdef INTER_PROCESSES
_status_t MagPlayerDriverImpl::setDataSource(const sp<IStreamBuffer> &source){
    Parcel param;
    _status_t ret;

    if (mpPlayer ==  NULL){
        return MAG_NO_INIT;
    }
    AGILE_LOGV("Enter!");
    
    IStreamBuffer::Type sbt = source->getType();
    
    if (sbt == IStreamBuffer::TS){
        AGILE_LOGV("it is TS stream buffer!");
        mpStreamBufUser = new StreamBufferUser(source, 1*1024*1024, 1);
    }else if (sbt == IStreamBuffer::ES){

    }else{
       AGILE_LOGE("unsupported source type: %d", sbt);
       return MAG_BAD_TYPE;
    }
    
    mState = MPD_ST_INITIALIZED;
    return mpPlayer->setDataSource(mpStreamBufUser);
}
#endif

_status_t MagPlayerDriverImpl::prepare(){
    AGILE_LOGV("Enter! state = %d", mState);
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->prepare();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::prepareAsync(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->prepareAsync();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::start(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->start();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::stop(){
    if (MPD_ST_INITIALIZED == mState){
        mSetTrackIndex = 0;
        return mpPlayer->stop();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::pause(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->pause();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

bool MagPlayerDriverImpl::isPlaying(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->isPlaying();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

_status_t MagPlayerDriverImpl::seekTo(int msec){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->seekTo(msec);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::fast(int speed){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->fast(speed);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::flush(){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->flush();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::getCurrentPosition(int *msec){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->getCurrentPosition(msec);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::getDuration(int *msec){
    if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->getDuration(msec);
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::reset(){
    if ( (MPD_ST_INITIALIZED == mState) || (MPD_ST_ERROR == mState) ){
        return mpPlayer->reset();
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return MAG_INVALID_OPERATION;
    }
}

_status_t MagPlayerDriverImpl::setParameter(int key, void *request){
    _status_t ret = MAG_NO_ERROR;

    if (MPD_ST_INITIALIZED != mState)
        return MAG_INVALID_OPERATION;

    switch(key){
        case PARAM_KEY_None:
        
            break;

        case PARAM_KEY_CP_AVAIL:
            mpPlayer->setParameters(kMediaPlayerConfig_CPAvail, MagParamTypeInt32, request);
            break;

        default:
            ret = MAG_BAD_VALUE;
            AGILE_LOGE("unsupported parameter key: %d", key);
            break;
    }
    return ret;
}

_status_t MagPlayerDriverImpl::getParameter(int key, void **reply){
    _status_t ret = MAG_NO_ERROR;

    if (MPD_ST_INITIALIZED != mState)
        return MAG_INVALID_OPERATION;

    switch(key){
        case PARAM_KEY_None:
        
            break;

        case PARAM_KEY_CP_AVAIL:
            ret = mpPlayer->getParameters(kMediaPlayerConfig_CPAvail, MagParamTypeInt32, reply);
            break;

        default:
            ret = MAG_BAD_VALUE;
            AGILE_LOGE("unsupported parameter key: %d", key);
            break;
    }
    return ret;
}

_status_t MagPlayerDriverImpl::invoke(const ui32 methodID, void *request, void **reply){
    _status_t ret = MAG_NO_ERROR;

    if (MPD_ST_INITIALIZED != mState)
        return MAG_INVALID_OPERATION;
    
    switch(methodID){
        case INVOKE_ID_GET_BUFFER_STATUS:
            if (reply != NULL){
                BufferStatistic_t *pBufSt = static_cast<BufferStatistic_t *>(*reply);
                ret = mpPlayer->getBufferStatus(pBufSt);
            }else{
                AGILE_LOGE("Failed to do INVOKE_ID_GET_BUFFER_STATUS. reply=NULL");
                ret = MAG_BAD_VALUE;
            }
            break;

        case INVOKE_ID_GET_VIDEO_META_DATA:
            if (reply != NULL){
                VideoMetaData_t *pVmd = static_cast<VideoMetaData_t *>(*reply);
                ret = mpPlayer->getVideoMetaData(pVmd);
            }else{
                AGILE_LOGE("Failed to do INVOKE_ID_GET_VIDEO_META_DATA. reply=NULL");
                ret = MAG_BAD_VALUE;
            }
            break;

        case INVOKE_ID_GET_AUDIO_META_DATA:
            if (reply != NULL){
                AudioMetaData_t *pAmd = static_cast<AudioMetaData_t *>(*reply);
                ret = mpPlayer->getAudioMetaData(pAmd);
            }else{
                AGILE_LOGE("Failed to do INVOKE_ID_GET_AUDIO_META_DATA. reply=NULL");
                ret = MAG_BAD_VALUE;
            }
            break;

        case INVOKE_ID_GET_DECODED_VIDEO_FRAME:
            ret = mpPlayer->getDecodedVideoFrame(reply);
            break;

        case INVOKE_ID_PUT_USED_VIDEO_FRAME:
            if (request != NULL){
                ret = mpPlayer->putUsedVideoFrame(request);
            }else{
                AGILE_LOGE("Failed to do INVOKE_ID_PUT_USED_VIDEO_FRAME. request=NULL");
                ret = MAG_BAD_VALUE;
            }
            break;

        case INVOKE_ID_GET_DECODED_AUDIO_FRAME:
            ret = mpPlayer->getDecodedAudioFrame(reply);
            break;

        case INVOKE_ID_PUT_USED_AUDIO_FRAME:
            if (request != NULL){
                ret = mpPlayer->putUsedAudioFrame(request);
            }else{
                AGILE_LOGE("Failed to do INVOKE_ID_PUT_USED_AUDIO_FRAME. request=NULL");
                ret = MAG_BAD_VALUE;
            }
            break;

        default:
            break;
    }
    return ret;
}

ui32     MagPlayerDriverImpl::getVersion(){
    if (mpPlayer)
        return mpPlayer->getVersion();
    
    return 0;
}

_status_t MagPlayerDriverImpl::setVolume(float leftVolume, float rightVolume){
    // if (MPD_ST_INITIALIZED == mState){
        return mpPlayer->setVolume(leftVolume, rightVolume);
    // }else{
    //     AGILE_LOGE("the state[%d] is wrong", mState);
    //     return MAG_INVALID_OPERATION;
    // }
}

void MagPlayerDriverImpl::PrepareCompleteEvtListener(void *priv){
    if (priv != NULL){
        MagPlayerDriverImpl *obj = static_cast<MagPlayerDriverImpl *>(priv);
        obj->sendEvent(MEDIA_PREPARED);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriverImpl::SeekCompleteEvtListener(void *priv){
    if (priv != NULL){
        MagPlayerDriverImpl *obj = static_cast<MagPlayerDriverImpl *>(priv);
        obj->sendEvent(MEDIA_SEEK_COMPLETE);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriverImpl::ErrorEvtListener(void *priv, i32 what, i32 extra){
    if (priv != NULL){
        MagPlayerDriverImpl *obj = static_cast<MagPlayerDriverImpl *>(priv);
        obj->mState = MPD_ST_ERROR;
        obj->sendEvent(MEDIA_ERROR, what, extra);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriverImpl::InfoEvtListener(void *priv, i32 what, i32 extra){
    if (priv != NULL){
        MagPlayerDriverImpl *obj = static_cast<MagPlayerDriverImpl *>(priv);
        obj->sendEvent(MEDIA_INFO, what, extra);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriverImpl::FlushCompleteEvtListener(void *priv){
    if (priv != NULL){
        MagPlayerDriverImpl *obj = static_cast<MagPlayerDriverImpl *>(priv);
        obj->sendEvent(MEDIA_FLUSH_COMPLETE);
    }else{
        AGILE_LOGE("priv is NULL, quit!");
    }
}

void MagPlayerDriverImpl::sendEvent(int msg, int ext1, int ext2) {
    if ((NULL != mClientNotifyFn) && (NULL != mpClient)){
        mClientNotifyFn(mpClient, msg, ext1, ext2);
    }else{
        AGILE_LOGE("failed to do sendEvent! (mClientNotifyFn=0x%p, mpClient=0x%p)", mClientNotifyFn, mpClient);
    }
}

