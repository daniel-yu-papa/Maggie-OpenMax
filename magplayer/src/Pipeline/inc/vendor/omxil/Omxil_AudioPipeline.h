
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

#ifndef __OMXIL_AUDIO_PIPELINE_H__
#define __OMXIL_AUDIO_PIPELINE_H__

#include "MagAudioPipelineImpl.h"
#include "Omxil_BufferMgr.h"

// #define OMXIL_AUDIO_STREAM_DUMP

#define AUIDO_PORT_BUFFER_NUMBER 16

class OmxilAudioPipeline : public MagAudioPipelineImpl{
public:
    OmxilAudioPipeline();
    virtual ~OmxilAudioPipeline();
    
	virtual _status_t init(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();  
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush(); 
    virtual _status_t reset();
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume);
    virtual _status_t getDecodedFrame(void **ppAudioFrame);
    virtual _status_t putUsedFrame(void *pAudioFrame);
    
    virtual _status_t pushEsPackets(MagOmxMediaBuffer_t *buf);
    virtual bool      needData();

    bool              isPlaying();
    _status_t         getClkConnectedComp(i32 *port, void **ppComp);

private:
    const char *getRoleByCodecId(ui32 OMXCodec);
    OMX_STRING    getCompNameByCodecId(ui32 OMXCodec);

    TrackInfo_t   *mTrackInfo;

    OMX_HANDLETYPE mhAudioDecoder;
    OMX_HANDLETYPE mhAudioRender;

    OMX_CALLBACKTYPE mAudioDecCallbacks;
    OMX_CALLBACKTYPE mAudioRenCallbacks;

    OmxilBufferMgr *mpBufferMgr;
    
    MagEventHandle         mADecStIdleEvent;
    MagEventHandle         mARenStIdleEvent;
    MagEventGroupHandle    mStIdleEventGroup;

    MagEventHandle         mADecStLoadedEvent;
    MagEventHandle         mARenStLoadedEvent;
    MagEventGroupHandle    mStLoadedEventGroup;

    MagEventHandle         mADecStExecutingEvent;
    MagEventHandle         mARenStExecutingEvent;
    MagEventGroupHandle    mStExecutingEventGroup;

    MagEventHandle         mADecStPauseEvent;
    MagEventHandle         mARenStPauseEvent;
    MagEventGroupHandle    mStPauseEventGroup;

    MagEventHandle         mADecFlushDoneEvent;
    MagEventHandle         mARenFlushDoneEvent;
    MagEventGroupHandle    mFlushDoneEventGroup;
    
    i32 mARenClockPortIdx;
    
    OMX_U32 mADecTunnelPortIdx;
    OMX_U32 mARenTunnelPortIdx;

#ifdef AMP_AUDIO_STREAM_DUMP
    FILE *mDumpAudioFile;
#endif

    static OMX_ERRORTYPE AudioDecoderEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);

    static OMX_ERRORTYPE AudioDecoderEmptyBufferDone(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE AudioDecoderFillBufferDone(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_PTR pAppData,
                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE AudioRenderEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);
};

#endif