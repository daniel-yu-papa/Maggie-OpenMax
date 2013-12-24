#ifndef __MAG_PLAYER_H__
#define __MAG_PLAYER_H__

#include "Mag_base.h"

#define LOOPER_NAME "MagPlayerLooper"

typedef void (*fnNotifyResetComplete)();
typedef void (*fnNotifyPrepareComplete)();

class MagPlayer {
public:
    MagPlayer();
    virtual ~MagPlayer();
    
    _status_t        setDataSource(const char *url, const void* parameters);
    _status_t        setDataSource(int fd, i64 offset, i64 length);
    /*the source is the memory buffer*/
    _status_t        setDataSource(MagBufferHandle buffer);

    void prepareAsync();
    void start();
    void stop();

    void pause();
    void resume();

    void flush();
    void seekTo(ui32 msec);
    void resetAsync();
    
    void setResetCompleteListener(fnNotifyResetComplete fn);
    void setPrepareCompleteListener(fnNotifyPrepareComplete fn);
    

protected:
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    Mag_MsgQueueHandle mFlushCompleteMQ;
    Mag_MsgQueueHandle mSeekCompleteMQ;
    Mag_MsgQueueHandle mPrepareCompleteMQ;

    fnNotifyResetComplete mNotifyResetCompleteFn;
    fnNotifyPrepareComplete mNotifyPrepareCompleteFn;
    
    enum{
        MagMsg_SourceNotify,
        MagMsg_Prepare,
        MagMsg_Start,
        MagMsg_Stop,
        MagMsg_Pause,
        MagMsg_Resume,
        MagMsg_Flush,
        MagMsg_SeekTo,
        MagMsg_Reset,
    };
    enum State_t{
        ST_IDLE,
        ST_INITIALIZED,
        ST_PREPARING,
        ST_PREPARED,
        ST_FLUSHING,
        ST_SEEKING,
        ST_RUNNING,
        ST_PAUSED,
        ST_STOPPED,
        ST_PLAYBACK_COMPLETED,
        ST_ERROR,
    };

    State_t mState;
    State_t mFlushBackState; /*the initial state where flush action should be back after it is complete*/
    State_t mSeekBackState; /*the initial state where seekTo action should be back after it is complete*/
    MagMutexHandle mStateLock;

    MagEventHandle mCompletePrepareEvt;
    MagEventHandle mCompleteSeekEvt;
    MagEventHandle mCompleteFlushEvt;
    MagEventGroupHandle mEventGroup;
    MagEventSchedulerHandle mEventScheduler;
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
    void onReset(MagMessageHandle msg);

    bool isValidFSState(State_t st);
};
#endif
