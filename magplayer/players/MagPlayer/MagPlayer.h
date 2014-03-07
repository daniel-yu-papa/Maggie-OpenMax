#ifndef __MAG_PLAYER_H__
#define __MAG_PLAYER_H__

#include "Mag_base.h"

#define LOOPER_NAME "MagPlayerLooper"

#define PARAMETERS_DB_SIZE   256

typedef enum{
    E_WHAT_UNKNOWN = 0,
    E_WHAT_DEMUXER,
}ErrorWhatCode_t;


typedef void (*fnNotifyResetComplete)(void *priv);
typedef void (*fnNotifyPrepareComplete)(void *priv);
typedef void (*fnNotifyFlushComplete)(void *priv);
typedef void (*fnNotifyError)(void *priv, i32 what, i32 extra);

class MagPlayer {
public:
    MagPlayer();
    virtual ~MagPlayer();
    
    _status_t        setDataSource(const char *url, const MagMiniDBHandle settings);
    _status_t        setDataSource(i32 fd, i64 offset, i64 length);
    /*the source is the memory buffer*/
    _status_t        setDataSource(StreamBufferUser *buffer);
    _status_t        setDataSource(MAG_EXTERNAL_SOURCE source);

    void prepareAsync();
    void start();
    void stop();

    void pause();
    void resume();

    void flush();
    void seekTo(ui32 msec);
    void resetAsync();
    
    void setResetCompleteListener(fnNotifyResetComplete fn, void *priv);
    void setPrepareCompleteListener(fnNotifyPrepareComplete fn, void *priv);
    void setFlushCompleteListener(fnNotifyFlushComplete fn, void *priv);
    void setErrorListener(fnNotifyError fn, void *priv);

    void setParameters(const char *name, MagParamType_t type, void *value);
    _status_t getParameters(const char *name, MagParamType_t type, void **value);

    void fast(i32 speed);

    void setDisplayWindow(ui32 x, ui32 y, ui32 w, ui32 h);
    
protected:
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    Mag_MsgQueueHandle mFlushCompleteMQ;
    Mag_MsgQueueHandle mSeekCompleteMQ;
    Mag_MsgQueueHandle mPrepareCompleteMQ;

    MagPlayerDriver *mpDriver;
        
    fnNotifyResetComplete mNotifyResetCompleteFn;
    void *mResetCompletePriv;
    fnNotifyPrepareComplete mNotifyPrepareCompleteFn;
    void *mPrepareCompletePriv;
    fnNotifyFlushComplete mNotifyFlushCompleteFn;
    void *mFlushCompletePriv;
    fnNotifyError mErrorFn;
    void *mErrorPriv;
    
    enum{
        MagMsg_SourceNotify,
        MagMsg_Prepare,
        MagMsg_Start,
        MagMsg_Stop,
        MagMsg_Pause,
        MagMsg_Resume,
        MagMsg_Flush,
        MagMsg_SeekTo,
        MagMsg_Fast,
        MagMsg_Reset,
        MagMsg_ErrorNotify,
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

    State_t mState;
    State_t mFlushBackState; /*the initial state where flush action should be back after it is complete*/
    State_t mSeekBackState;  /*the initial state where seekTo action should be back after it is complete*/
    State_t mFastBackState;  /*the initial state where fast action should be back after it is complete*/
    MagMutexHandle mStateLock;

    MagEventHandle mCompletePrepareEvt;
    MagEventHandle mCompleteSeekEvt;
    MagEventHandle mCompleteFlushEvt;
    MagEventGroupHandle mEventGroup;
    MagEventSchedulerHandle mEventScheduler;

    MagMiniDBHandle mParametersDB;

    MagPlayer_Component_CP *mSource;
    MagPlayer_Demuxer_Base *mDemuxer;

    TrackInfoTable_t *mTrackTable;
    
    static void onCompletePrepareEvtCB(void *priv);
    static void onCompleteSeekEvtCB(void *priv);
    static void onCompleteFlushEvtCB(void *priv);
    
    void initialize();
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    static void onMessageReceived(const void *msg, void *priv);
    
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
    void onError(MagMessageHandle msg);
    
    bool isValidFSState(State_t st);
};
#endif
