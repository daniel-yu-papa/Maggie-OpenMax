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

#ifndef __MAGPLAYER_DEMUXER_BASE_IMPL_H__
#define __MAGPLAYER_DEMUXER_BASE_IMPL_H__

#include "MagDemuxerBase.h"
#include "MagStreamTrackManager.h"

typedef struct PlayState {
    i32 abort_request;
}PlayState_t;

class MagDemuxerBaseImpl : public MagDemuxerBase{
public:
    MagDemuxerBaseImpl();
    virtual ~MagDemuxerBaseImpl();

    virtual _status_t   setPlayingTrackID(ui32 index);
    virtual ui32        getPlayingTracksID(ui32 *index);
    virtual _status_t   readFrame(ui32 trackIndex, MagOmxMediaBuffer_t **buffer);
    virtual MagMessageHandle createNotifyMsg();
    virtual _status_t   prepare(MagContentPipe *contentPipe, MagBufferObserver *pObserver, MagMiniDBHandle paramDB);
    virtual _status_t   start();
    virtual _status_t   stop();
    virtual void        readyToStop();
    virtual _status_t   pause();
    virtual _status_t   resume();
    virtual _status_t   flush();
    virtual void        readyToFlush();
    virtual _status_t   seekTo(i32 msec, i64 mediaTime);
    virtual _status_t   dettachBufferObserver(MagBufferObserver *pObserver);
    virtual TrackInfoTable_t *getTrackInfoList();
    virtual void getAVBufferStatus(ui32 *videoBuf, ui32 *audioBuf, ui32 *loadingSpeed);
    virtual void        abort();
    
    virtual void        setEOS();

    virtual _status_t   stopImpl() = 0;
    virtual _status_t   flushImpl() = 0;
    virtual _status_t   seekToImpl(i32 msec, i64 mediaTime, TrackInfo_t *track) = 0;
    virtual _status_t   readFrameImpl(Stream_Track *track, ui32 StreamID) = 0;
    
protected:
    MagMiniDBHandle mParamDB;
    Stream_Track_Manager *mpStreamTrackManager;
    boolean mIsPrepared;
    PlayState_t mPlayState;
    MagMessageHandle createMessage(ui32 what);

private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
    // MagMessageHandle mDataReadyMsg;
    bool mIsEOS;

    void onPlayerNotify(MagMessageHandle msg);
    void onContentPipeNotify(MagMessageHandle msg);
    void onStop(MagMessageHandle msg);
    void onFlush(MagMessageHandle msg);

    static void      onMessageReceived(const MagMessageHandle msg, void *priv);
    _status_t        getLooper();

    MagMessageHandle mDemuxStopMsg;
    MagMessageHandle mDemuxFlushMsg;
};

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _status_t releaseMediaBuffer(MagOmxMediaBuffer_t *mb);

#undef EXTERNC

#endif