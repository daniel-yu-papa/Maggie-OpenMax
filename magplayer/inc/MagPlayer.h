#ifndef __MAG_PLAYER_H__
#define __MAG_PLAYER_H__

#include "framework/MagFramework.h"
#include "MagParameters.h"
#include "MagContentPipe.h"
#include "MagDataSource.h"
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
    E_WHAT_DEMUXER,
    E_WHAT_DATASOURCE,
    E_WHAT_VIDEO_PIPELINE,
    E_WHAT_AUDIO_PIPELINE,
    E_WHAT_COMMAND,
}ErrorWhatCode_t;

typedef void (*fnNotifyInfo)(void *priv, i32 what, i32 extra);
typedef void (*fnNotifySeekComplete)(void *priv);
typedef void (*fnNotifyPrepareComplete)(void *priv);
typedef void (*fnNotifyFlushComplete)(void *priv);
typedef void (*fnNotifyError)(void *priv, i32 what, i32 extra);

class MagPlayer {
public:
    enum {
        kWhatFillThisBuffer      = 'filb',
        kWhatPlayComplete        = 'plcm',
        kWhatError               = 'erro',
    };

    
    MagPlayer();
    virtual ~MagPlayer();

    _status_t        setDataSource(const char *url);
    _status_t        setDataSource(i32 fd, i64 offset, i64 length);
#ifdef ANDROID_OS
    _status_t        setDataSource(StreamBufferUser *buffer);
#endif

    _status_t        prepare();
    _status_t        prepareAsync();
    _status_t        start();
    _status_t        stop();
    _status_t        pause();
    bool             isPlaying();
    _status_t        seekTo(i32 msec);
    _status_t        flush();
    _status_t        fast(int speed);
    _status_t        getCurrentPosition(int* msec);
    _status_t        getDuration(int* msec);
    _status_t        reset();
    _status_t        setVolume(float leftVolume, float rightVolume);
    _status_t        setParameters(const char *name, MagParamType_t type, void *value);
    _status_t        getParameters(const char *name, MagParamType_t type, void **value);
    ui32             getVersion();
    
    void             setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h);
    _status_t        getBufferStatus(BufferStatistic_t  *pBufSt);
    _status_t        getVideoMetaData(VideoMetaData_t *pVmd);
    _status_t        getAudioMetaData(AudioMetaData_t *pVmd);

    _status_t        getDecodedVideoFrame(void **ppGetFrame);
    _status_t        putUsedVideoFrame(void *pUsedFrame);
    _status_t        getDecodedAudioFrame(void **ppGetFrame);
    _status_t        putUsedAudioFrame(void *pUsedFrame);

    void             setInfoListener(fnNotifyInfo fn, void *priv);
    void             setSeekCompleteListener(fnNotifySeekComplete fn, void *priv);
    void             setPrepareCompleteListener(fnNotifyPrepareComplete fn, void *priv);
    void             setFlushCompleteListener(fnNotifyFlushComplete fn, void *priv);
    void             setErrorListener(fnNotifyError fn, void *priv);

protected:
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
    
    MagLooperHandle  mSeekLooper;
    MagHandlerHandle mSeekMsgHandler;

    // Mag_MsgQueueHandle mFlushCompleteMQ;
    // Mag_MsgQueueHandle mSeekCompleteMQ;
    // Mag_MsgQueueHandle mPrepareCompleteMQ;
        
    fnNotifySeekComplete mNotifySeekCompleteFn;
    void *mSeekCompletePriv;
    fnNotifyPrepareComplete mNotifyPrepareCompleteFn;
    void *mPrepareCompletePriv;
    fnNotifyFlushComplete mNotifyFlushCompleteFn;
    void *mFlushCompletePriv;
    fnNotifyError mErrorFn;
    void *mErrorPriv;
    fnNotifyInfo mInfoFn;
    void *mInfoPriv;
    
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
        MagMsg_ErrorNotify,
        MagMsg_ComponentNotify,
        MagMsg_SetParameters,
        MagMsg_BufferObserverNotify
    };
    
    enum{
        MagMsg_SeekTo = 0x30
    };

    enum State_t{
        ST_IDLE = 0,
        ST_INITIALIZED,
        ST_PREPARING,
        ST_PREPARED,
        ST_FLUSHING,
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
            STRINGIFY(ST_FLUSHING);
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

    MagMessageHandle mSourceNotifyMsg;
    MagMessageHandle mPrepareMsg;
    MagMessageHandle mStartMsg;
    MagMessageHandle mPlayMsg;
    MagMessageHandle mStopMsg;
    MagMessageHandle mPauseMsg;
    MagMessageHandle mFlushMsg;
    MagMessageHandle mFastMsg;
    MagMessageHandle mResetMsg;
    MagMessageHandle mSetVolumeMsg;
    MagMessageHandle mErrorNotifyMsg;
    // MagMessageHandle mComponentNotifyMsg;
    MagMessageHandle mBufferObserverNotifyMsg;
    MagMessageHandle mSetParametersMsg;
    
    MagMessageHandle mSeekToMsg;

    MagMutexHandle   mGetParamLock;
        
    State_t mState;
    State_t mFlushBackState; /*the initial state where flush action should be back after it is complete*/
    State_t mSeekBackState;  /*the initial state where seekTo action should be back after it is complete*/
    State_t mFastBackState;  /*the initial state where fast action should be back after it is complete*/
    MagMutexHandle mStateLock;

    MagEventHandle mCompletePrepareEvt;
    MagEventHandle mCompleteSeekEvt;
    MagEventHandle mCompleteFlushEvt;
    MagEventHandle mErrorEvt;

    MagEventGroupHandle mEventGroup;
    MagEventGroupHandle mSeekEventGroup;

    MagEventSchedulerHandle mEventScheduler;

    MagMiniDBHandle mParametersDB;

    MagContentPipe   *mSource;
    MagDemuxerBase   *mDemuxer;
    MagMessageHandle mDemuxerNotify;
    
    TrackInfoTable_t *mTrackTable;
    
    MagBufferObserver *mpContentPipeObserver;
    MagBufferObserver *mpDemuxerStreamObserver;

    MagPipelineManager *mAVPipelineMgr;
    MagVideoPipeline *mVideoPipeline;
    MagAudioPipeline *mAudioPipeline;
    MagClock         *mClock;

    fp32             mLeftVolume;
    fp32             mRightVolume;

    i32              mPlayTimeInMs;
    i32              mPlayTimeBeforeSeek;

    bool             mbIsPlayed;
    bool             mReportOutBufStatus;
    
    static void onCompletePrepareEvtCB(void *priv);
    static void onCompleteSeekEvtCB(void *priv);
    static void onCompleteFlushEvtCB(void *priv);
    static void onErrorEvtCB(void *priv);
    
    void initialize();
    void cleanup();
    void doResetAction();
    
    _status_t getLooper();
    _status_t getSeekLooper();
    MagMessageHandle createMessage(ui32 what);
    MagMessageHandle createSeekMessage(ui32 what);
    static void onMessageReceived(const MagMessageHandle msg, void *priv);
    static void onSeekMessageReceived(const MagMessageHandle msg, void *priv);

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
    void onErrorNotify(MagMessageHandle msg);
    void onSetVolume(MagMessageHandle msg);
    void onComponentNotify(MagMessageHandle msg);
    void onSetParameters(MagMessageHandle msg);
    void onBufferObserverNotify(MagMessageHandle msg);

    bool isValidFSState(State_t st);

    void sendErrorEvent(i32 what, i32 extra);
    _status_t resumeTo();

    char* proceedUrl(char *url);
};
#endif
