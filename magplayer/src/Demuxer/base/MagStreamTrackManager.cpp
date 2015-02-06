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

#include "MagStreamTrackManager.h"
#include "MagDemuxerBase.h"
#include "MagDemuxerBaseImpl.h"
#include "MagPlayerCommon.h"

#include <unistd.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_BuffMgr"

#define MAX_TRACK_NUMBER 64

_status_t releaseMediaBuffer(MagOmxMediaBuffer_t *mb){
    Stream_Track *strack = static_cast<Stream_Track *>(mb->track_obj);
    _status_t ret;

    Mag_AcquireMutex(strack->mMutex);
    ret = strack->releaseFrame(mb);
    Mag_ReleaseMutex(strack->mMutex);
    return ret;
}

Stream_Track::Stream_Track(TrackInfo_t *info):
                               mInfo(info),
                               mBufferPoolSize(0),
                               mBufPoolHandle(NULL),
                               mStartPTS(0),
                               mEndPTS(0),
                               mFrameNum(0),
                               mBufferStatus(kEmpty),
                               mEsFormatter(NULL),
                               mBufLevelChange(false),
                               mIsFull(false),
                               mIsRunning(true),
                               mBufferLimit(0){
    INIT_LIST(&mMBufBusyListHead);
    INIT_LIST(&mMBufFreeListHead);
    INIT_LIST(&mFrameStatList);
    Mag_CreateMutex(&mMutex);
    Mag_CreateMutex(&mBufferStatMutex);
}

Stream_Track::~Stream_Track(){
    AGILE_LOGV("track %d: enter!", mInfo->streamID);
    stop();
    if (mEsFormatter)
        delete mEsFormatter;
    Mag_DestroyMutex(&mMutex);
    Mag_DestroyMutex(&mBufferStatMutex);
    magMemPoolDestroy(&mBufPoolHandle);   
}

void Stream_Track::setBufferLimit(ui32 limit){
    mBufferLimit = limit;
}

bool Stream_Track::getSetBufLevelChange(){
    bool ret = mBufLevelChange;
    mBufLevelChange = false;
    return ret;
}

_status_t Stream_Track::start(){  
    _status_t ret = MAG_NO_ERROR;

    Mag_AcquireMutex(mMutex);

    if (mBufferPoolSize > 0){
        if (mBufPoolHandle == NULL){
            mBufPoolHandle = magMemPoolCreate(mBufferPoolSize, 1);
            if (mBufPoolHandle != NULL){
                AGILE_LOGI("The buffer pool 0x%x is created for %s track!", 
                            mBufPoolHandle, mInfo->type == TRACK_VIDEO ? "video" : mInfo->type == TRACK_AUDIO ? "audio" : "subtitle");
                magMemPoolSetBufLimit(mBufPoolHandle, mBufferLimit);
            }else{
                AGILE_LOGE("Failed to create the buffer pool!");
                ret = MAG_NO_MEMORY;
            }
        }else{
            AGILE_LOGV("%s: set to running", mInfo->name);
            mIsRunning = true;
        }
    }else{
    	AGILE_LOGE("buffer pool size: %d is wrong!", mBufferPoolSize);
    	ret = MAG_NO_INIT;
    }

    Mag_ReleaseMutex(mMutex);
    return ret;
}

_status_t Stream_Track::stop(){
    AGILE_LOGI("stop stream track: %s", mInfo->name);

    Mag_AcquireMutex(mMutex);
    {
        MagOmxMediaBuffer_t* item;
        do{
            item = dequeueFrame(false);
            /*if (item != NULL){         
                releaseFrame(item);
            }*/
        }while(item != NULL);
        
        magMemPoolReset(mBufPoolHandle);
        mStartPTS = 0;
        mEndPTS   = 0;
        mFrameNum = 0;

        if (mEsFormatter)
            mEsFormatter->reset();

        mIsRunning = false;
    }
    Mag_ReleaseMutex(mMutex);

    Mag_AcquireMutex(mBufferStatMutex);
    {   
        List_t *next;
        FrameStatistics_t *item = NULL;
        next = mFrameStatList.next;
        while (next != &mFrameStatList){
            item = (FrameStatistics_t *)list_entry(next, FrameStatistics_t, node);
            list_del(&item->node);
            mag_free(item);
            next = mFrameStatList.next;
        }
    }
    Mag_ReleaseMutex(mBufferStatMutex);

    mBufferStatus = kEmpty;
    return MAG_NO_ERROR;
}

TrackInfo_t* Stream_Track::getInfo(){
    return mInfo;
}


MagOmxMediaBuffer_t* Stream_Track::dequeueFrame(bool lock){
    List_t *next;
    MagOmxMediaBuffer_t *item = NULL;
    
    if (lock)
        Mag_AcquireMutex(mMutex);
    
    if (mIsRunning){
        next = mMBufBusyListHead.next;
        if (next != &mMBufBusyListHead){
            item = (MagOmxMediaBuffer_t *)list_entry(next, MagOmxMediaBuffer_t, node);
            list_del(next);
            mStartPTS = item->pts;
            mFrameNum--;
            /*AGILE_LOGV("%s: delete buffer(0x%x) from stream track queue", getInfo()->name, item->buffer);*/
        }
    }else{
        AGILE_LOGE("%s stream track is stopped, ignore the request!", mInfo->name);
    }

    if (lock)
        Mag_ReleaseMutex(mMutex);
    return item;
}

ui32  Stream_Track::doFrameStat(ui32 frame_size){
    List_t *next = NULL;
    FrameStatistics_t *item = NULL;
    bool isAdded = false;
    ui32 total_size = 0;

    i64 time;
    i32 diff;

    Mag_AcquireMutex(mBufferStatMutex);

    time = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000000ll;
    next = mFrameStatList.next;
    if (next != &mFrameStatList){
        item = (FrameStatistics_t *)list_entry(next, FrameStatistics_t, node);
        diff = (i32)(time - item->enqueue_time);
        while (diff > 1000){
            // AGILE_LOGV("diff = %d, time_now = %lld, start_time = %lld", diff, time, item->enqueue_time);
            list_del(&item->node);
            if (frame_size > 0){
                if (!isAdded){
                    item->enqueue_time = time;
                    item->size         = frame_size;
                    list_add_tail(&item->node, &mFrameStatList);
                    isAdded = true;
                }else{
                    mag_free(item);
                }
            }else{
                mag_free(item);
            }

            next = mFrameStatList.next;
            if (next != &mFrameStatList){
                item = (FrameStatistics_t *)list_entry(next, FrameStatistics_t, node);
                diff = (i32)(time - item->enqueue_time);
            }else{
                diff = 0;
            }
        }

        if (frame_size > 0){
            if (!isAdded){
                item = (FrameStatistics_t *)mag_mallocz(sizeof(FrameStatistics_t));
                if (NULL != item){
                    INIT_LIST(&item->node);
                    item->enqueue_time = time;
                    item->size         = frame_size;
                }
                list_add_tail(&item->node, &mFrameStatList);
            }
        }
    }else{
        if (frame_size > 0){
            item = (FrameStatistics_t *)mag_mallocz(sizeof(FrameStatistics_t));
            if (NULL != item){
                INIT_LIST(&item->node);
                item->enqueue_time = time;
                item->size         = frame_size;
                list_add_tail(&item->node, &mFrameStatList);
            }
        }
    }

    if (frame_size == 0){
        next = mFrameStatList.next;
        while (next != &mFrameStatList){
            item = (FrameStatistics_t *)list_entry(next, FrameStatistics_t, node);
            total_size += item->size;
            next = next->next;
        }
    }

    Mag_ReleaseMutex(mBufferStatMutex);
    return total_size;
}

_status_t     Stream_Track::enqueueFrame(MagOmxMediaBuffer_t *mb){
    void *buf;
    _status_t ret = MAG_NO_ERROR;
    MagErr_t err;

    if (NULL == mBufPoolHandle){
        if (MAG_NO_ERROR != (ret = start())){
            return ret;
        }
    }

    Mag_AcquireMutex(mMutex);
    if (mIsRunning){
        if (!mb->eosFrame){
            err = magMemPoolGetBuffer(mBufPoolHandle, mb->buffer_size, &buf);
            if (NULL != buf){
                memcpy(buf, mb->buffer, mb->buffer_size);
                mb->buffer = buf;
                if (mMBufBusyListHead.next == &mMBufBusyListHead){
                	mStartPTS = mb->pts;
                }
                mEndPTS = mb->pts;
                mFrameNum++;
                putMediaBuffer(&mMBufBusyListHead, mb);
                mIsFull = false;
                if (err == MAG_NoMore){
                    AGILE_LOGI("send out buffer level change request!!");
                    mBufLevelChange = true;
                }
                /*AGILE_LOGV("%s: add buffer(0x%x) to stream track queue", mInfo->name, buf);*/
            }else{
                mIsFull = true;
                AGILE_LOGE("the stream track %s is FULL!!", mInfo->name);
                ret = MAG_NO_MEMORY;
            }
            doFrameStat(mb->buffer_size);
        }else{
            AGILE_LOGV("%s: push back the EOS frame!", mInfo->name);
            putMediaBuffer(&mMBufBusyListHead, mb);
        }
    }else{
        AGILE_LOGE("%s stream track is stopped, ignore the request!", mInfo->name);
        ret = MAG_INVALID_OPERATION;
    }
    Mag_ReleaseMutex(mMutex);

    return ret;
}

MagOmxMediaBuffer_t *Stream_Track::getMediaBuffer(){
    List_t *next = NULL;
    MagOmxMediaBuffer_t *item = NULL;

    Mag_AcquireMutex(mMutex);
    if (mIsRunning){
        next = mMBufFreeListHead.next;
        if (next != &mMBufFreeListHead){
            item = (MagOmxMediaBuffer_t *)list_entry(next, MagOmxMediaBuffer_t, node);
            list_del(next);
        }else{
            item = (MagOmxMediaBuffer_t *)mag_mallocz(sizeof(MagOmxMediaBuffer_t));
            if (NULL != item){
                INIT_LIST(&item->node);
                item->track_obj = static_cast<void *>(this);
                item->release   = releaseMediaBuffer;
            }
        }
    }
    Mag_ReleaseMutex(mMutex);
    
    return item;
}

_status_t Stream_Track::putMediaBuffer(List_t *list_head, MagOmxMediaBuffer_t *mb){
    _status_t ret = MAG_NO_ERROR;
    i32 i;

    if ((mb != NULL) && (list_head != NULL)){
        if (mb->side_data_elems > 0){
            for (i = 0; i < mb->side_data_elems; i++){
                mag_freep((void **)&mb->side_data[i].data);
                mb->side_data[i].size = 0;
            }
            mag_freep((void **)&mb->side_data);
            mb->side_data_elems = 0;
        }
        list_add_tail(&mb->node, list_head);
    }else{
        ret = MAG_BAD_VALUE;
    }
    return ret;
}

_status_t     Stream_Track::releaseFrame(MagOmxMediaBuffer_t *mb){
    if (NULL != mb){
        magMemPoolPutBuffer(mBufPoolHandle, mb->buffer);
        putMediaBuffer(&mMBufFreeListHead, mb);
        return MAG_NO_ERROR;
    }else{
        return MAG_NO_MEMORY;
    }
}

void  Stream_Track::setBufferPoolSize(ui32 size){
	mBufferPoolSize = size;
}

/*return in ms units*/
ui32 Stream_Track::getBufferingDataTime(){
    ui32 duration;

	if ((mEndPTS >= 0) &&
        (mStartPTS >= 0) &&
        (mEndPTS >= mStartPTS)){
		return ((ui32)(mEndPTS - mStartPTS)/(90));
	}else{
        duration = mFrameNum*30;
        AGILE_LOGE("Wrong PTS pair: startPTS=%lld, endPTS=%lld. Use frame number(%d) to get duration(%d)", 
                    mStartPTS, mEndPTS, mFrameNum, duration);
        return duration;
	}
}

bool Stream_Track::getFullness(){
    return mIsFull;
}

BufferStatus_t Stream_Track::getBufferStatus(){
    return mBufferStatus;
}

void Stream_Track::setBufferStatus(BufferStatus_t st){
    mBufferStatus = st;
}

void Stream_Track::setFormatter(MagESFormat *fmt){
    mEsFormatter = fmt;
}

void *Stream_Track::getFormatter(){
    return static_cast<void *>(mEsFormatter);
}

Stream_Track_Manager::Stream_Track_Manager(void *pDemuxer, MagMiniDBHandle hParamDB) :
														mParamDB(hParamDB),
														mpDemuxer(pDemuxer),
														mTrackList(NULL),
                                                        mStreamIDRoot(NULL),
                                                        mpObserver(NULL),
                                                        mIsPaused(false),
                                                        mIsFlushed(false),
                                                        mAbortReading(false),
                                                        mDisableBufferMgr(-1),
                                                        mBufferingType(BUFFER_POLICY_NORMAL){
	memset(&mBufManagePolicy, 0, sizeof(Demuxer_BufferPolicy_t));
	mhReadingFramesEntry = Mag_CreateThread("ReadingFramesThread", 
		                                     Stream_Track_Manager::ReadingFramesEntry,
		                                     static_cast<void *>(this));

    Mag_CreateEventGroup(&mPlayingEvtGroup);
    Mag_CreateEvent(&mEventResume, MAG_EVT_PRIO_DEFAULT);
    Mag_AddEventGroup(mPlayingEvtGroup, mEventResume);
}

Stream_Track_Manager::~Stream_Track_Manager(){
    AGILE_LOGV("enter!");
    mAbortReading = true;
    Mag_SetEvent(mEventResume);
    Mag_DestroyThread(&mhReadingFramesEntry);
    Mag_DestroyEvent(&mEventResume);
    Mag_DestroyEventGroup(&mPlayingEvtGroup); 
    AGILE_LOGV("exit!");
}

TrackInfoTable_t *Stream_Track_Manager::getTrackInfoList(){
    void *value;
    i32 vnumber = 0;
    i32 anumber = 0;
    i32 snumber = 0;
    boolean ret;
    i32 i;
    char buf[128];
    i32 totalTrackNum = 0;
    TrackInfo_t *track;
    
    if (mTrackList != NULL){
        AGILE_LOGE("The Track List has been created!");
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
        
        for (i = 0; i < totalTrackNum; i++){
            sprintf(buf, kDemuxer_Track_Info, i);
            ret = mParamDB->findPointer(mParamDB, buf, &value);
            if (ret == MAG_TRUE){
                track = static_cast<TrackInfo_t *>(value);
                AGILE_LOGI("index %d: track %s[0x%x], pid = %d, codec = %d", i, track->type == TRACK_VIDEO ? "video" : "audio",
                            track, track->pid, track->codec);
                // track->message = createMessage(MagDemuxerBase::MagDemuxerMsg_PlayerNotify);
                // track->message->setInt32(track->message, "what", MagDemuxerBase::kWhatReadFrame);
                track->pendingRead = 0;
                if (track->type == TRACK_VIDEO){
                    track->index = v;
                    mTrackList->trackTableList[v] = track;
                    track->message->setInt32(track->message, "track-idx", v);
                    v++;
                }else if (track->type == TRACK_AUDIO){
                    track->index = vnumber + a;
                    mTrackList->trackTableList[vnumber + a] = track;
                    track->message->setInt32(track->message, "track-idx", vnumber + a);
                    a++;
                }else if (track->type == TRACK_SUBTITLE){
                    track->index = vnumber + anumber + s;
                    mTrackList->trackTableList[vnumber + anumber + s] = track;
                    track->message->setInt32(track->message, "track-idx", vnumber + anumber + s);
                    s++;
                }else{
                    AGILE_LOGE("unknown track type: %d", track->type);
                }
            }
        }
    }else{
        mTrackList->trackTableList = NULL;
    }
    mTrackList->totalTrackNum = totalTrackNum;

    return mTrackList;
}

void Stream_Track_Manager::destroyTrackInfoList(){
    i32 totalTrackNum = 0;
    i32 i;
    TrackInfo_t *track;
    Stream_Track *st = NULL;
    
    if (mTrackList == NULL)
        return;

    if (mTrackList->trackTableList == NULL){
        mag_free(mTrackList);
        mTrackList = NULL;
        return;
    }
    
    totalTrackNum = mTrackList->totalTrackNum;

    for (i = 0; i < totalTrackNum; i++){
        track = mTrackList->trackTableList[i];
        // destroyMagMessage(&track->message);
        deleteStreamTrack(track->streamID);
        st = static_cast<Stream_Track *>(track->stream_track);
        if (st)
            delete st;
    }
    mag_free(mTrackList->trackTableList);
    mag_free(mTrackList);
    mTrackList = NULL;
}

_status_t  Stream_Track_Manager::setPlayingTrackID(ui32 index){
    TrackInfo_t *ti;

    if (mTrackList == NULL){
        mTrackList = getTrackInfoList();
        AGILE_LOGE("failed to do getTrackInfoList(), quit!");
        return 0;
    }

    ti  = mTrackList->trackTableList[index];
    if (NULL != ti){
        AGILE_LOGD("[index:%d]: To set %s track[0x%x] -- name = %s, pid = %d as Playing", 
                    index, ti->type == TRACK_VIDEO ? "video" : "audio", ti, 
                    ti->name, ti->pid);
        ti->status = TRACK_PLAY;
    }
    return MAG_NO_ERROR;
}

ui32 Stream_Track_Manager::getPlayingTracksID(ui32 *index){
    i32 i;
    TrackInfo_t * ti;
    i32 j = 0;
    i32 total;

    if (mTrackList == NULL){
        mTrackList = getTrackInfoList();
        AGILE_LOGE("failed to do getTrackInfoList(), quit!");
        return 0;
    }

    total = mTrackList->totalTrackNum;
    for (i = 0; i < total; i++){
        ti = mTrackList->trackTableList[i];
        if (ti->status == TRACK_PLAY){
            index[j] = ti->index;
            j++;
        }
    }

    return j;
}

ui32 Stream_Track_Manager::getPlayingStreamsID(ui32 *index){
    i32 i;
    TrackInfo_t * ti;
    i32 j = 0;
    i32 total;

    if (mTrackList == NULL){
        mTrackList = getTrackInfoList();
        AGILE_LOGE("failed to do getTrackInfoList(), quit!");
        return 0;
    }

    total = mTrackList->totalTrackNum;
    for (i = 0; i < total; i++){
        ti = mTrackList->trackTableList[i];
        if (ti->status == TRACK_PLAY){
            index[j] = ti->streamID;
            j++;
        }
    }

    return j;
}


ui32 Stream_Track_Manager::getBufferPoolSize(enum TrackType_t type){
	if (type == TRACK_VIDEO){
        i32 bit_rate;
        ui32 buffer_size;
        boolean ret;

        ret = mParamDB->findInt32(mParamDB, kDemuxer_Bit_Rate, &bit_rate);
        if (!ret){
            AGILE_LOGE("failed to find the kDemuxer_Bit_Rate. To use default video buffer size: %d!",
                        DEFAULT_VIDEO_BUFFER_SIZE);
            buffer_size = DEFAULT_VIDEO_BUFFER_SIZE;
        }else{
            if (bit_rate){
                buffer_size = ALIGNTO( (mBufManagePolicy.videoBufferPoolSize * (ui32)bit_rate * 125), 1024);
            }else{
                i32 width;
                i32 heigth;
                boolean w_ret;
                boolean h_ret;

                w_ret = mParamDB->findInt32(mParamDB, kDemuxer_Video_Width, &width);
                h_ret = mParamDB->findInt32(mParamDB, kDemuxer_Video_Height, &heigth);

                if (w_ret & h_ret){
                    AGILE_LOGD("the stream bitrate is 0, to use video width(%d) and heigth(%d) to create the buffer",
                                width, heigth);
                    if (width > 0 && heigth > 0){
                        buffer_size = ALIGNTO( (mBufManagePolicy.videoBufferPoolSize * width * heigth * 25), 1024);
                    }else{
                        buffer_size = DEFAULT_VIDEO_BUFFER_SIZE;
                    }
                }else{
                    AGILE_LOGE("failed to find kDemuxer_Video_Width or kDemuxer_Video_Height. To use default video buffer size: %d!",
                                DEFAULT_VIDEO_BUFFER_SIZE);
                }
            }
            AGILE_LOGD("Created video buffer size is %d [setting = %d seconds, bit_rate = %d kbps]", 
                            buffer_size, mBufManagePolicy.videoBufferPoolSize, bit_rate);
            if (buffer_size > MAX_VIDEO_BUFFER_SIZE){
                AGILE_LOGD("To truncate the video buffer size(%d) to %d.", 
                            buffer_size, 
                            MAX_VIDEO_BUFFER_SIZE);
                buffer_size = MAX_VIDEO_BUFFER_SIZE;
            }
        }
		return buffer_size;
	}else if (type == TRACK_AUDIO){
		return mBufManagePolicy.audioBufferPoolSize;
	}else if (type == TRACK_SUBTITLE){
		return mBufManagePolicy.otherBufferPoolSize;
	}else{
		AGILE_LOGE("wrong type: %d", type);
	}
	return 0;
}

_status_t   Stream_Track_Manager::addStreamTrack(Stream_Track *pTrack){
	TrackInfo_t *ti;

	if (!pTrack){
		AGILE_LOGE("pTrack is NULL, QUIT!");
		return MAG_BAD_VALUE;
	}

	ti = pTrack->getInfo();
	pTrack->setBufferPoolSize(getBufferPoolSize(ti->type));
	
	mStreamIDRoot = rbtree_insert(mStreamIDRoot, static_cast<i64>(ti->streamID), static_cast<void *>(pTrack));
    ti->stream_track = static_cast<void *>(pTrack);

    if (ti->type == TRACK_VIDEO)
        pTrack->setBufferLimit(mBufManagePolicy.memPoolSizeLimit);

    return MAG_NO_ERROR;
}

_status_t   Stream_Track_Manager::deleteStreamTrack(i32 streamID){
    rbtree_delete(&mStreamIDRoot, streamID);
    return MAG_NO_ERROR;
}

Stream_Track *Stream_Track_Manager::getStreamTrack(ui32 StreamID){
	void *value = NULL;
    Stream_Track *track = NULL;

    value = rbtree_get(mStreamIDRoot, static_cast<i64>(StreamID));
    if (NULL != value){
        track =  static_cast<Stream_Track *>(value);
    }else{
    	AGILE_LOGE("failed to get the stream track with ID: %d", StreamID);
    }
    return track;
}

bool Stream_Track_Manager::interruptReading(){
    return mAbortReading;
}

_status_t   Stream_Track_Manager::readFrame(ui32 trackIndex, MagOmxMediaBuffer_t **buffer){
    return readFrameFromQueue(trackIndex, buffer);
}

_status_t   Stream_Track_Manager::readFrameFromQueue(ui32 trackIndex, MagOmxMediaBuffer_t **buffer){
    Stream_Track *track;
    MagOmxMediaBuffer_t* mb = NULL;
    _status_t ret = MAG_NO_ERROR;
    TrackInfo_t *ti;

    if (mTrackList == NULL){
        AGILE_LOGE("mTrackList is NULL, quit!");
        return MAG_NO_INIT;
    }

    ti = mTrackList->trackTableList[trackIndex];
    track = getStreamTrack(ti->streamID);
    if (NULL != track){
        mb = track->dequeueFrame();
        if (NULL == mb){
            AGILE_LOGE("The stream track %s is empty!", ti->name);
            /*command the mapplayer to pause*/
            // mpObserver->update(static_cast<BufferringEvent_t>(kEmpty));
            ret = MAG_NOT_ENOUGH_DATA;
        }else{
            mb->esFormatter = track->getFormatter();
        }
    }else{
    	ret = MAG_NAME_NOT_FOUND;
    }
    *buffer = mb;
    return ret;
}

/*Called by buffer monitor looper*/
_status_t   Stream_Track_Manager::readFramesMore(Stream_Track *track, ui32 StreamID){
	MagDemuxerBaseImpl *demuxer = NULL;

	if (!mpDemuxer){
		AGILE_LOGE("mpDemuxer is NULL!");
		return MAG_BAD_VALUE;
	}

	demuxer = static_cast<MagDemuxerBaseImpl *>(mpDemuxer);

	return demuxer->readFrameImpl(track, StreamID);
}

_status_t   Stream_Track_Manager::attachBufferObserver(MagBufferObserver *pObserver){
	mpObserver = pObserver;
	return MAG_NO_ERROR;
}

_status_t   Stream_Track_Manager::dettachBufferObserver(MagBufferObserver *pObserver){
	mpObserver = NULL;
	return MAG_NO_ERROR;
}

_status_t   Stream_Track_Manager::setBufferPolicy(Demuxer_BufferPolicy_t *pPolicy){
	if (!pPolicy){
		AGILE_LOGE("pPolicy is NULL!");
		return MAG_BAD_VALUE;
	}

	memcpy(&mBufManagePolicy, pPolicy, sizeof(Demuxer_BufferPolicy_t));

    mSBufLowThreshold  = mBufManagePolicy.normalBitRate.bufferLowThreshold;
    mSBufPlayThreshold = mBufManagePolicy.normalBitRate.bufferPlayThreshold;
    mSBufHighThreshold = mBufManagePolicy.normalBitRate.bufferHighThreshold;

	return MAG_NO_ERROR;
}

_status_t   Stream_Track_Manager::start(){
    if (mIsFlushed){
        mIsPaused = false;
        resume();
    }else{
        mAbortReading = false;
        if (mTrackList == NULL)
            mTrackList = getTrackInfoList();

    	mhReadingFramesEntry->run(mhReadingFramesEntry);
    }
    return MAG_NO_ERROR;
}

_status_t   Stream_Track_Manager::stop(){
    AGILE_LOGI("Enter. mIsPaused=%d, mIsFlushed=%d", mIsPaused, mIsFlushed);
    mAbortReading = true;

    if (mIsPaused){
        AGILE_LOGI("in paused state");
        resume();
    }

    Mag_SetEvent(mEventResume);
    mhReadingFramesEntry->requestExitAndWait(mhReadingFramesEntry, MAG_TIMEOUT_INFINITE);
    Mag_ClearEvent(mEventResume);
    mIsFlushed        = false;
    mIsPaused         = false;
    mAbortReading     = false;
    mDisableBufferMgr = -1;
    AGILE_LOGI("frame reading thread exited!!!");

    if (mTrackList)
        destroyTrackInfoList();
    
    mBufferingType = BUFFER_POLICY_NORMAL;
    mSBufLowThreshold  = mBufManagePolicy.normalBitRate.bufferLowThreshold;
    mSBufPlayThreshold = mBufManagePolicy.normalBitRate.bufferPlayThreshold;
    mSBufHighThreshold = mBufManagePolicy.normalBitRate.bufferHighThreshold;

    AGILE_LOGD("Exit!");
    return MAG_NO_ERROR;
}

/*pause() and resume()*/
_status_t   Stream_Track_Manager::pause(){
    AGILE_LOGV("enter!");
    mIsPaused = true;
    return MAG_NO_ERROR;
}

_status_t   Stream_Track_Manager::resume(){
    AGILE_LOGV("enter!");
    mAbortReading = false;
    if (mIsFlushed){
        i32 totalTrackNum = 0;
        i32 i;
        TrackInfo_t *track;
        Stream_Track *st = NULL;
        
        totalTrackNum = mTrackList->totalTrackNum;

        for (i = 0; i < totalTrackNum; i++){
            track = mTrackList->trackTableList[i];
            if (track->status == TRACK_PLAY){
                st = static_cast<Stream_Track *>(track->stream_track);
                if (st){
                    st->start();
                }
            }  
        }
        mIsFlushed = false;
        if (!mIsPaused)
            Mag_SetEvent(mEventResume);
    }else{
        mIsPaused = false;
        Mag_SetEvent(mEventResume);
    }
    return MAG_NO_ERROR;
}

void Stream_Track_Manager::readyToFlush(){
    mIsFlushed = true;
    // mAbortReading = true;
}

void Stream_Track_Manager::readyToStop(){
    mIsFlushed = false;
    mAbortReading = true;
}

/*flush() and resume()*/
_status_t   Stream_Track_Manager::flush(){
    i32 totalTrackNum = 0;
    i32 i;
    TrackInfo_t *track;
    Stream_Track *st = NULL;
    
    if (mTrackList == NULL){
        AGILE_LOGE("mTrackList is NULL!");
        return MAG_NO_ERROR;
    }

    if (mTrackList->trackTableList == NULL){
        AGILE_LOGE("No stream tracks are added, do nothing!");
        return MAG_NO_ERROR;
    }

    totalTrackNum = mTrackList->totalTrackNum;

    for (i = 0; i < totalTrackNum; i++){
        track = mTrackList->trackTableList[i];
        if (track->status == TRACK_PLAY){
            st = static_cast<Stream_Track *>(track->stream_track);
            if (st){
                st->stop();
            }
        }  
    }
    mpObserver->flush();
    
    return MAG_NO_ERROR;
}

bool Stream_Track_Manager::handleAllPlayingTracksBuffer(){
    /*ui32 totalTrackNum;*/
    ui32 pTrackIdList[MAX_TRACK_NUMBER];
    ui32 playingTrackNum;
    ui32 i;
    TrackInfo_t *retrieveDataTrack   = NULL;
    BufferStatus_t finalBufferStatus = kInvalid;
    i32 buffer_percentage = -1;
    bool statusChanged = false;

    if ((mIsPaused) || (mIsFlushed)){
        AGILE_LOGI("wait on the mPlayingEvtGroup! mIsPaused(%d), mIsFlushed(%d)", mIsPaused, mIsFlushed);
        Mag_WaitForEventGroup(mPlayingEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGI("after waitting on the mPlayingEvtGroup!");
        if (mAbortReading)
            return false;
    }

    

	// totalTrackNum = mTrackList->totalTrackNum;
	// pTrackIdList = (ui32 *)mag_mallocz(sizeof(ui32) * totalTrackNum);
	playingTrackNum = getPlayingTracksID(pTrackIdList);
    // AGILE_LOGV("Total playing track number is %d", playingTrackNum);

    for(i = 0; i < playingTrackNum; i++){
		ui32 trackID;
		TrackInfo_t *track;
		Stream_Track *st;
		ui32 bufferingTime;
        BufferStatus_t keyBufSt;
        BufferStatus_t streamTrackBufSt;

		trackID = *(pTrackIdList + i);
		track = mTrackList->trackTableList[trackID];
		st = static_cast<Stream_Track *>(track->stream_track);

        if (track->type == TRACK_VIDEO){
            if (st->getSetBufLevelChange()){
                if (mBufferingType == BUFFER_POLICY_NORMAL){
                    mSBufLowThreshold  = mBufManagePolicy.highBitRate.bufferLowThreshold;
                    mSBufPlayThreshold = mBufManagePolicy.highBitRate.bufferPlayThreshold;
                    mSBufHighThreshold = mBufManagePolicy.highBitRate.bufferHighThreshold;
                    mBufferingType = BUFFER_POLICY_LOW;
                    AGILE_LOGI("change to low buffer policy!");
                }else if (mBufferingType == BUFFER_POLICY_LOW){
                    mSBufLowThreshold  = mBufManagePolicy.highestBitRate.bufferLowThreshold;
                    mSBufPlayThreshold = mBufManagePolicy.highestBitRate.bufferPlayThreshold;
                    mSBufHighThreshold = mBufManagePolicy.highestBitRate.bufferHighThreshold;
                    mBufferingType = BUFFER_POLICY_LOWEST;
                    AGILE_LOGI("change to lowest buffer policy!");
                }else{
                    AGILE_LOGE("[should not be here]: It has been in the lowest buffer policy!");
                }
            }
        }

		bufferingTime = st->getBufferingDataTime();
        keyBufSt = st->getBufferStatus();
        if (bufferingTime > 0){
            if (st->getFullness()){
                AGILE_LOGI("buffer pool is full in buffering time %d ms!", bufferingTime);
                keyBufSt = kFull;
            }else{
                if (bufferingTime < mSBufLowThreshold){
                    keyBufSt = kBelowLow; 
                }else if(bufferingTime < mSBufPlayThreshold){
                    keyBufSt = kBetweenLow_Play;
                }else if(bufferingTime < mSBufHighThreshold){
                    keyBufSt = kBetweenPlay_High;
                }else{
                    keyBufSt = kAboveHigh;
                } 
            }
        }else{
            keyBufSt = kEmpty;
        }
        streamTrackBufSt = st->getBufferStatus();

		if (streamTrackBufSt != keyBufSt){
            AGILE_LOGI("stream track %s: buffer status from %s to %s", 
                        track->name,
                        BufferStatus2String(streamTrackBufSt),
                        BufferStatus2String(keyBufSt));
            st->setBufferStatus(keyBufSt);
            statusChanged = true;
        }

        if (finalBufferStatus > keyBufSt){
            finalBufferStatus = keyBufSt;
            retrieveDataTrack =  track;
        }
        
        /*report out the buffering percentage in low buffer playing pause status*/
        if (bufferingTime < mSBufPlayThreshold){
            i32 ratio = (bufferingTime * 100) / mSBufPlayThreshold;
            if ((buffer_percentage == -1) || (buffer_percentage > ratio)){
                buffer_percentage = ratio;
            }
        }
    }

    /*TODO: send out the event to the observer*/
    if (mpObserver){
        boolean ret;
        if (mDisableBufferMgr == -1){
            ret = mParamDB->findInt32(mParamDB, kDemuxer_Disable_Buffering, &mDisableBufferMgr);
            if (ret != MAG_TRUE){
                /*enable buffering if the key does not present*/
                mDisableBufferMgr = 0;
            }
        }
        
        if (!mDisableBufferMgr){
            if (statusChanged){
                mpObserver->update(static_cast<BufferringEvent_t>(finalBufferStatus));
            }

            if (buffer_percentage >= 0){
                mpObserver->update(kEvent_BufferStatus, buffer_percentage);
            }
        }else{
            /*always play once the buffer is not empty.*/
            if (statusChanged){
                if (finalBufferStatus == kEmpty){
                    mpObserver->update(static_cast<BufferringEvent_t>(kEmpty));
                }else{
                    mpObserver->update(static_cast<BufferringEvent_t>(kBetweenPlay_High));
                }
            }
        }
    }else{
        AGILE_LOGE("buffer observer is NULL!");
    }

    if (finalBufferStatus >= kAboveHigh){
        AGILE_LOGI("stop reading frames since the buffer level of stream track is HIGH!");
        usleep(10000);
    }else{
        _status_t res;
        res = readFramesMore(static_cast<Stream_Track *>(retrieveDataTrack->stream_track), 
                             retrieveDataTrack->streamID);
        
        if (res == MAG_NO_MORE_DATA){
            AGILE_LOGI("quit the reading frames thread because of EOS or error happening on data source!");
            mpObserver->update(kEvent_NoMoreData);
            mIsFlushed = true;
        }
        if ((res == MAG_READ_ABORT) && (!mIsFlushed)){
            AGILE_LOGI("Abort the frames reading");
            return false;
        }
    }
    return true;
}

boolean Stream_Track_Manager::ReadingFramesEntry(void *priv){
	Stream_Track_Manager *stm;

	if (!priv){
		AGILE_LOGE("priv is NULL!");
		return MAG_FALSE;
	}
    
    stm = static_cast<Stream_Track_Manager *>(priv);

    if (stm->handleAllPlayingTracksBuffer())
        return MAG_TRUE;
    else
        return MAG_FALSE;
}

void Stream_Track_Manager::getAVBufferStatus(ui32 *videoBuf, ui32 *audioBuf, ui32 *loadingSpeed){
    ui32 pTrackIdList[MAX_TRACK_NUMBER];
    ui32 playingTrackNum;
    ui32 i;
    ui32 totalBytes = 0;

    playingTrackNum = getPlayingTracksID(pTrackIdList);

    for(i = 0; i < playingTrackNum; i++){
        ui32 trackID;
        TrackInfo_t *track;
        Stream_Track *st;
        ui32 bufferingTime;

        trackID = *(pTrackIdList + i);
        track = mTrackList->trackTableList[trackID];
        st = static_cast<Stream_Track *>(track->stream_track);
        bufferingTime = st->getBufferingDataTime();
        if (track->type == TRACK_VIDEO){
            *videoBuf = bufferingTime;
            totalBytes = totalBytes + st->doFrameStat(0);
        }else if(track->type == TRACK_AUDIO){
            *audioBuf = bufferingTime;
            totalBytes = totalBytes + st->doFrameStat(0);
        }
    }
    *loadingSpeed = totalBytes;
}