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

#ifndef __OMXIL_VIDEO_PIPELINE_H__
#define __OMXIL_VIDEO_PIPELINE_H__

#include "MagVideoPipelineImpl.h"
#include "Omxil_BufferMgr.h"


// #define OMXIL_VIDEO_STREAM_DUMP

#define VIDEO_PORT_BUFFER_NUMBER 16

class OmxilVideoPipeline : public MagVideoPipelineImpl{
public:
    OmxilVideoPipeline();
    virtual ~OmxilVideoPipeline();
    
	virtual _status_t init(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();  
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush(); 
    virtual _status_t reset();

    virtual _status_t pushEsPackets(MagOmxMediaBuffer_t *buf);
    virtual bool      needData();
    void              setDisplayRect(i32 x, i32 y, i32 w, i32 h);
    bool              isPlaying();
    _status_t         getClkConnectedComp(i32 *port, void **ppComp);

    _status_t         getDecodedFrame(void **ppVideoFrame);
    _status_t         putUsedFrame(void *pVideoFrame);

private:
    OMX_STRING    getCompNameByCodecId(ui32 OMXCodec);

    TrackInfo_t   *mTrackInfo;

    OMX_HANDLETYPE mhVideoDecoder;
    OMX_HANDLETYPE mhVideoScheduler;
    OMX_HANDLETYPE mhVideoRender;

    OMX_CALLBACKTYPE mVideoDecCallbacks;
    OMX_CALLBACKTYPE mVideoSchCallbacks;
    OMX_CALLBACKTYPE mVideoRenCallbacks;

    OmxilBufferMgr *mpBufferMgr;
    
    OmxilBufferMgr *mpDecodedBufferMgr;
    OmxilBufferMgr *mpFeedVrenBufMgr;

    MagEventHandle         mVDecStIdleEvent;
    MagEventHandle         mVSchStIdleEvent;
    MagEventHandle         mVRenStIdleEvent;
    MagEventGroupHandle    mStIdleEventGroup;

    MagEventHandle         mVDecStLoadedEvent;
    MagEventHandle         mVSchStLoadedEvent;
    MagEventHandle         mVRenStLoadedEvent;
    MagEventGroupHandle    mStLoadedEventGroup;

    MagEventHandle         mVDecStExecutingEvent;
    MagEventHandle         mVSchStExecutingEvent;
    MagEventHandle         mVRenStExecutingEvent;
    MagEventGroupHandle    mStExecutingEventGroup;

    MagEventHandle         mVDecStPauseEvent;
    MagEventHandle         mVSchStPauseEvent;
    MagEventHandle         mVRenStPauseEvent;
    MagEventGroupHandle    mStPauseEventGroup;

    MagEventHandle         mVDecFlushDoneEvent;
    MagEventHandle         mVSchFlushDoneEvent;
    MagEventHandle         mVRenFlushDoneEvent;
    MagEventGroupHandle    mFlushDoneEventGroup;
    
    i32 mVSchClockPortIdx;

    i32 mFeedVrenPending;
    MagMutexHandle mhFeedPendingMutex;

    OMX_U32 mVSchTunnelInPortIdx;
    OMX_U32 mVSchTunnelOutPortIdx;
    OMX_U32 mVRenTunnelInPortIdx;
    OMX_U32 mVDecTunnelOutPortIdx;

#ifdef OMXIL_VIDEO_STREAM_DUMP
    FILE *mDumpVideoFile;
#endif
    const char *getRoleByCodecId(ui32 OMXCodec);

    static OMX_ERRORTYPE VideoDecoderEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);

    static OMX_ERRORTYPE VideoDecoderEmptyBufferDone(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE VideoDecoderFillBufferDone(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_PTR pAppData,
                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE VideoScheduleEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);

    static OMX_ERRORTYPE VideoRenderEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);

    static OMX_ERRORTYPE  VideoRenderFillBufferDone(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_PTR pAppData,
                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
};

#endif