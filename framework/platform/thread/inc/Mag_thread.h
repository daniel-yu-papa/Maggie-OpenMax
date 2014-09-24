#ifndef __MAG_THREAD_H__
#define __MAG_THREAD_H__

#include "Mag_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MAGTHREAD_PRIORITY_LOWEST         = 19,
    MAGTHREAD_PRIORITY_LOW            = 10,
    MAGTHREAD_PRIORITY_NORMAL         = 0,
    MAGTHREAD_PRIORITY_HIGH           = -16,
    /* should never be used in practice. regular process might not 
     * be allowed to use this level */
    MAGTHREAD_PRIORITY_HIGHEST        = -20
}MagThread_Priority_t;

typedef boolean (*fnThreadLoop)(void *priv);
typedef boolean (*fnReadyToRun)(void *priv);

typedef struct mag_thread{
    fnThreadLoop mfnTheadLoop;
    fnReadyToRun mfnReadyToRun;
    void *mPriv;
    char *mName;
    ui32 mStackSize;
    i32  mPriority;
    
    boolean   mRunning;
    boolean   mExitPending;
    void      *mThread;

    MagMutexHandle mLock;

    MagEventGroupHandle mExitEvtGroup;
    MagEventHandle      mExitEvt;

    boolean             mSuspendRequest;
    MagEventGroupHandle mSuspendEvtGroup;
    MagEventHandle      mSuspendEvt;
    
    _status_t (*run)(struct mag_thread *self);
    _status_t (*setFunc_readyToRun)(struct mag_thread *self, fnReadyToRun fn);
    _status_t (*setParm_Priority)(struct mag_thread *self, MagThread_Priority_t priority);
    _status_t (*setParm_StackSize)(struct mag_thread *self, _size_t stackSize);

    _status_t (*suspend)(struct mag_thread *self);
    _status_t (*resume)(struct mag_thread *self);

    _status_t (*requestExit)(struct mag_thread *self);
    _status_t (*requestExitAndWait)(struct mag_thread *self, i32 timeout);
}MagThread_t;

typedef MagThread_t*   MagThreadHandle;

MagThreadHandle Mag_CreateThread(const char* name, fnThreadLoop fn, void *priv);
void        Mag_DestroyThread(MagThreadHandle *pSelf);

#ifdef __cplusplus
}
#endif

#endif
