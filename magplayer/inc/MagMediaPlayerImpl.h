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

#ifndef __MAG_PLAYER_H__
#define __MAG_PLAYER_H__

#include "framework/MagFramework.h"
#include "MagParameters.h"
#include "MagContentPipe.h"
#include "MagDemuxerFFMPEG.h"
#include "MagAudioPipeline.h"
#include "MagVideoPipeline.h"
#include "MagClock.h"
#include "MagInvokeDef.h"
#include "MagPipelineManager.h"

#define MOCK_OMX_IL

#define LOOPER_NAME "MagPlayerLooper"

#define PARAMETERS_DB_SIZE   256

typedef enum{
    E_WHAT_UNKNOWN = 0,
    E_WHAT_DATASOURCE,
    E_WHAT_PREPARE,
    E_WHAT_START,
    E_WHAT_PIPELINE
}ErrorWhatCode_t;

typedef enum{
    E_EXTRA_UNKNOWN = 0,
    E_EXTRA_WRONG_STATE,
    E_EXTRA_INVALID_PIPELINE,
    E_EXTRA_INVALID_PARAMETER,
}ErrorExtraCode_t;

typedef struct{
    List_t node;
    OMX_DEMUXER_STREAM_INFO *streamInfo;
    mmp_meta_data_t *streamMetaData;
    i32 streamTableId;
}StreamInfo_t;

class MagMediaPlayerImpl : public MagMediaPlayer, public MagSingleton<MagMediaPlayerImpl>{
    friend class MagSingleton<MagMediaPlayerImpl>;
    MagMediaPlayerImpl();
    virtual ~MagMediaPlayerImpl();

public:
    int  setDataSource(const char *url);
    int  setDataSource(unsigned int fd, signed long long offset, signed long long length);
    int  prepare();
    int  prepareAsync();
    int  start();
    int  stop();
    int  pause();
    bool isPlaying();
    int  seekTo(int msec);
    int  flush();
    int  fast(int speed);
    int  getCurrentPosition(int* msec);
    int  getDuration(int* msec);
    int  reset();
    int  setVolume(float leftVolume, float rightVolume);
    int  setParameters(int key, void *request);
    int  getParameters(int key, void **reply);
    int  invoke(const unsigned int methodID, void *request, void **reply);
    int  registerEventCallback(mmp_event_callback_t cb, void *handler);

private:
    enum{
        MagMsg_SourceNotify = 0x10,
        MagMsg_Prepare,
        MagMsg_Start,
        MagMsg_Play,
        MagMsg_Stop,
        MagMsg_Pause,
        MagMsg_Flush,
        MagMsg_Fast,
        MagMsg_Reset,
        MagMsg_SetVolume,
        MagMsg_ComponentNotify,
        MagMsg_SetParameters,
        MagMsg_BufferObserverNotify
    };

    enum State_t{
        ST_IDLE = 0,
        ST_INITIALIZED,
        ST_PREPARED,
        ST_FASTING,
        ST_BUFFERING,
        ST_PLAYING,
        ST_PAUSED,
        ST_STOPPED,
        ST_PLAYBACK_COMPLETED,
        ST_ERROR,
    };

    const char *state2String(ui32 state) {
        switch (state) {
            STRINGIFY(ST_IDLE);
            STRINGIFY(ST_INITIALIZED);
            STRINGIFY(ST_PREPARED);
            STRINGIFY(ST_FASTING);
            STRINGIFY(ST_BUFFERING);
            STRINGIFY(ST_PLAYING);
            STRINGIFY(ST_PAUSED);
            STRINGIFY(ST_STOPPED);
            STRINGIFY(ST_PLAYBACK_COMPLETED);
            STRINGIFY(ST_ERROR);
            default: return "state - unknown";
        }
    }

    static OMX_ERRORTYPE playbackPipelineEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);

    static OMX_ERRORTYPE playbackPipelineEmptyBufferDone(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE playbackPipelineFillBufferDone(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_PTR pAppData,
                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    void sendEvent(mmp_event_t evt, i32 what, i32 extra);
    void doPauseAction();
    void doRunAction();
    void doStopAction();
    void doFlushAction();
    void doSeekAction(i32 target);
    void doResetAction();
    void fillStreamMetaData(mmp_meta_data_t *md, OMX_DEMUXER_STREAM_INFO *streamInfo);

    ui32       getVersion();
    void       setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h);
    _status_t  getBufferStatus(BufferStatistic_t  *pBufSt);
    _status_t  getVideoMetaData(VideoMetaData_t *pVmd);
    _status_t  getAudioMetaData(AudioMetaData_t *pVmd);
    _status_t  getDecodedVideoFrame(void **ppGetFrame);
    _status_t  putUsedVideoFrame(void *pUsedFrame);
    _status_t  getDecodedAudioFrame(void **ppGetFrame);
    _status_t  putUsedAudioFrame(void *pUsedFrame);
    static void onCompletePrepareEvtCB(void *priv);
    
    void initialize();
    void cleanup();
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    static void onMessageReceived(const MagMessageHandle msg, void *priv);

    void onSourceNotify(MagMessageHandle msg);
    void onPrepared(MagMessageHandle msg);
    void onStart(MagMessageHandle msg);
    void onPlay(MagMessageHandle msg);
    void onStop(MagMessageHandle msg);
    void onPause(MagMessageHandle msg);
    void onFlush(MagMessageHandle msg);
    void onSeek(MagMessageHandle msg);
    void onFast(MagMessageHandle msg);
    void onReset(MagMessageHandle msg);
    void onSetVolume(MagMessageHandle msg);


    mmp_event_callback_t mAppEventCallback;
    void                 *mAppHandler;

    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
    
    OMX_HANDLETYPE   mhPlaybackPipeline;
    OMX_CALLBACKTYPE mPlaybackPipelineCb;

    OMX_PLAYBACK_PIPELINE_SETTING mPlaybackPipelineConfig;
    bool             mReturnDecodedVideo;
    bool             mPlaybackLooping;

    char             *mpSubtitleUrl;

    MagBufferManager *mpVDecodedBufferMgr;
    MagBufferManager *mpPipelineBufferMgr;

    MagEventHandle      mPipelineStateEvent;
    MagEventGroupHandle mPipelineStateEvtGrp;

    MagEventHandle      mPipelineFlushDoneEvent;
    MagEventGroupHandle mPipelineFlushDoneEvtGrp;

    ui32                mBufferLevel;
    ui32                mBufferTime; 

    MagMessageHandle mSourceNotifyMsg;
    MagMessageHandle mPrepareMsg;
    MagMessageHandle mStartMsg;
    MagMessageHandle mPlayMsg;
    MagMessageHandle mStopMsg;
    MagMessageHandle mPauseMsg;
    MagMessageHandle mFlushMsg;
    MagMessageHandle mFastMsg;
    MagMessageHandle mSetVolumeMsg;
    MagMessageHandle mSeekToMsg;
        
    State_t mState;

    MagEventHandle mCompletePrepareEvt;
    MagEventGroupHandle mEventGroup;
    MagEventSchedulerHandle mEventScheduler;

    fp32             mLeftVolume;
    fp32             mRightVolume;

    List_t           mStreamList;
    i32              mStreamTotal;
};
#endif
