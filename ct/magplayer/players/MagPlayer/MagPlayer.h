#ifndef __MAG_PLAYER_H__
#define __MAG_PLAYER_H__

#include "Mag_base.h"

#define LOOPER_NAME "MagPlayerLooper"

#define PARAMETERS_DB_SIZE   256

typedef void (*fnNotifyResetComplete)(void *priv);
typedef void (*fnNotifyPrepareComplete)(void *priv);
typedef void (*fnNotifyFlushComplete)(void *priv);
typedef void (*fnNotifyError)(void *priv, i32 what, i32 extra);

typedef enum MAG_EXTERNAL_SOURCE{
    MAG_EXTERNAL_SOURCE_Unused,
    MAG_EXTERNAL_SOURCE_TSMemory, /*the continuous memory block filled with ts data*/
    MAG_EXTERNAL_SOURCE_ESMemory, /*the memory block filled with audio/video es data attached with PTS&DTS*/
    MAG_EXTERNAL_SOURCE_VendorStartUnused = 0x7F000000, /**< Reserved region for introducing External Source Extensions */
    MAG_EXTERNAL_SOURCE_Max = 0x7FFFFFFF
}MAG_EXTERNAL_SOURCE;

class MagPlayer {
public:
    MagPlayer();
    virtual ~MagPlayer();
    
    _status_t        setDataSource(const char *url, const void* parameters);
    _status_t        setDataSource(int fd, i64 offset, i64 length);
    /*the source is the memory buffer*/
    _status_t        setDataSource(MagBufferHandle buffer);
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
    void getParameters(const char *name, MagParamType_t type, void **value);

    void fast(i32 speed);
    
protected:
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    Mag_MsgQueueHandle mFlushCompleteMQ;
    Mag_MsgQueueHandle mSeekCompleteMQ;
    Mag_MsgQueueHandle mPrepareCompleteMQ;

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

    MAG_EXTERNAL_SOURCE mExtSource;
    
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

    bool isValidFSState(State_t st);
};
#endif
