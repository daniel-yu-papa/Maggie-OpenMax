#ifndef __MAG_PLAYER_H__
#define __MAG_PLAYER_H__

#include "MagFramework.h"
#include "Parameters.h"
#include "MagPlayer_ContentPipe.h"
#include "MagPlayer_DataSource.h"
#include "MagPlayer_Demuxer_FFMPEG.h"
#include "MagPlayer_Mock_OMXIL.h"

#define MOCK_OMX_IL

#define LOOPER_NAME "MagPlayerLooper"

#define PARAMETERS_DB_SIZE   256

typedef enum{
    E_WHAT_UNKNOWN = 0,
    E_WHAT_DEMUXER,
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
    _status_t        setDataSource(StreamBufferUser *buffer);

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

    void             setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h);
    
    void             setInfoListener(fnNotifyInfo fn, void *priv);
    void             setSeekCompleteListener(fnNotifySeekComplete fn, void *priv);
    void             setPrepareCompleteListener(fnNotifyPrepareComplete fn, void *priv);
    void             setFlushCompleteListener(fnNotifyFlushComplete fn, void *priv);
    void             setErrorListener(fnNotifyError fn, void *priv);

protected:
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    Mag_MsgQueueHandle mFlushCompleteMQ;
    Mag_MsgQueueHandle mSeekCompleteMQ;
    Mag_MsgQueueHandle mPrepareCompleteMQ;
        
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
        MagMsg_Stop,
        MagMsg_Pause,
        MagMsg_Flush,
        MagMsg_SeekTo,
        MagMsg_Fast,
        MagMsg_Reset,
        MagMsg_SetVolume,
        MagMsg_ErrorNotify,
        MagMsg_ComponentNotify,
        MagMsg_SetParameters,
    };
    
    enum State_t{
        ST_IDLE = 0,
        ST_INITIALIZED,
        ST_PREPARING,
        ST_PREPARED,
        ST_FLUSHING,
        ST_SEEKING,
        ST_FASTING,
        ST_RUNNING,
        ST_PAUSED,
        ST_STOPPED,
        ST_PLAYBACK_COMPLETED,
        ST_ERROR,
    };

    MagMessageHandle mSourceNotifyMsg;
    MagMessageHandle mPrepareMsg;
    MagMessageHandle mStartMsg;
    MagMessageHandle mStopMsg;
    MagMessageHandle mPauseMsg;
    MagMessageHandle mFlushMsg;
    MagMessageHandle mSeekToMsg;
    MagMessageHandle mFastMsg;
    MagMessageHandle mResetMsg;
    MagMessageHandle mSetVolumeMsg;
    MagMessageHandle mErrorNotifyMsg;
    MagMessageHandle mComponentNotifyMsg;
    MagMessageHandle mSetParametersMsg;
    
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
    
    MagEventSchedulerHandle mEventScheduler;

    MagMiniDBHandle mParametersDB;

    MagPlayer_Component_CP *mSource;
    MagPlayer_Demuxer_Base *mDemuxer;
    MagMessageHandle       mDemuxerNotify;
    
    TrackInfoTable_t *mTrackTable;
    
    static void onCompletePrepareEvtCB(void *priv);
    static void onCompleteSeekEvtCB(void *priv);
    static void onCompleteFlushEvtCB(void *priv);
    static void onErrorEvtCB(void *priv);
    
    void initialize();
    void cleanup();
    
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    static void onMessageReceived(const MagMessageHandle msg, void *priv);

    void onSourceNotify(MagMessageHandle msg);
    void onPrepared(MagMessageHandle msg);
    void onStart(MagMessageHandle msg);
    void onStop(MagMessageHandle msg);
    void onPause(MagMessageHandle msg);
    void onResume(MagMessageHandle msg);
    void onFlush(MagMessageHandle msg);
    void onSeek(MagMessageHandle msg);
    void onFast(MagMessageHandle msg);
    void onReset(MagMessageHandle msg);
    void onErrorNotify(MagMessageHandle msg);
    void onSetVolume(MagMessageHandle msg);
    void onComponentNotify(MagMessageHandle msg);
    void onSetParameters(MagMessageHandle msg);
    
    bool isValidFSState(State_t st);

    _status_t buildAudioPipeline();
    _status_t buildVideoPipeline();
    
    #ifdef MOCK_OMX_IL
    MagPlayer_Mock_OMX *mVOmxComponent;
    MagPlayer_Mock_OMX *mAOmxComponent;
    #endif
};
#endif
