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

#ifndef __MAGPLAYER_AUDIO_PIPELINE_IMPL_H__
#define __MAGPLAYER_AUDIO_PIPELINE_IMPL_H__

#include <stdio.h>

#include "MagAudioPipelineImplBase.h"
#include "MagOmx_Buffer.h"
#include "MagPlayerCommon.h"

// #define DUMP_AUDIO_ES_FILE

class MagAudioPipelineImpl : public MagAudioPipelineImplBase{
public:
    MagAudioPipelineImpl();
    virtual ~MagAudioPipelineImpl();

    virtual _status_t init(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t setup();
    virtual _status_t start();
	virtual _status_t stop();  
	virtual _status_t pause();
	virtual _status_t resume();
	virtual _status_t flush(); 
    virtual _status_t reset();
    virtual _status_t getClkConnectedComp(i32 *port, void **ppComp);
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume);
    virtual _status_t getDecodedFrame(void **ppAudioFrame);
    virtual _status_t putUsedFrame(void *pAudioFrame);

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    virtual MagMessageHandle getMagPlayerNotifier();

    virtual _status_t     pushEsPackets(MagOmxMediaBuffer_t *buf) = 0;
    virtual bool          needData() = 0;

    const char *state2String(ui32 state) {
        switch (state) {
            STRINGIFY(ST_INIT);
            STRINGIFY(ST_PLAY);
            STRINGIFY(ST_STOP);
            STRINGIFY(ST_PAUSE);
            default: return "state - unknown";
        }
    }

protected:
    void postFillThisBuffer();
    void setFillBufferFlag();
    i32  getFillBufferFlag();
    void notifyPlaybackComplete();
    i32 mState;
    bool mIsFlushed;
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
    MagMessageHandle mMagPlayerNotifier;

    MagMessageHandle mEmptyThisBufferMsg;

    i32  mTrackID;

    void onEmptyThisBuffer(MagMessageHandle msg);
    void onDestroy(MagMessageHandle msg);
    
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    void proceedMediaBuffer(MagOmxMediaBuffer_t *buf);
    static void onMessageReceived(const MagMessageHandle msg, void *priv);

    MagMessageHandle mDestroyMsg;
    i32 mPostFillBufCnt;
    MagMutexHandle mhPostFillBufMutex;

#ifdef DUMP_AUDIO_ES_FILE
    FILE *mDumpFile;
#endif
};

#endif