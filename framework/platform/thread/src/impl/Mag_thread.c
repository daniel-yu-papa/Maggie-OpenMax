#include <pthread.h>
#include <errno.h>
#include <sys/prctl.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "Mag_thread.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-Thread"

static _status_t loopEntryWrapper(void *userData){
    MagThread_t *user =  (MagThread_t *)userData;
    boolean ret = MAG_TRUE;
    
    boolean first = MAG_TRUE;
    do {
        if (first){
            first = MAG_FALSE;
            if (user->mfnReadyToRun)
                ret = user->mfnReadyToRun(user->mPriv);
        }else{
            ret = user->mfnTheadLoop(user->mPriv);
        }

        if (ret != MAG_TRUE || user->mExitPending){
            user->mExitPending = MAG_TRUE;
            user->mRunning     = MAG_FALSE;
            AGILE_LOGD("exit thread %s!", user->mName);
            break;
        }
    }while(1);

    Mag_SetEvent(user->mExitEvt);
    
    return 0;
}

static void *loopEntry(void *priv){
    MagThread_t *userData;

    if (priv != NULL){
        userData = (MagThread_t *)priv;
        setpriority(PRIO_PROCESS, 0, userData->mPriority);   
        prctl(PR_SET_NAME, (unsigned long)userData->mName, 0, 0, 0);
        // mag_free(userData->mName);
        loopEntryWrapper(priv);
    }

    return priv;
}

static _status_t createThread(MagThread_t *self){
    pthread_attr_t attr; 

    self->mExitPending = MAG_FALSE;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (self->mStackSize) {
        pthread_attr_setstacksize(&attr, self->mStackSize);
    }

    errno = 0;
    pthread_t thread;
    int result = pthread_create(&thread, &attr, loopEntry, (void *)self);
    pthread_attr_destroy(&attr);
    if (result != 0) {
        AGILE_LOGE("createThread failed (res=%d, errno=%d)(threadPriority=%d)\n",
                   result, errno, self->mPriority);
        return MAG_UNKNOWN_ERROR;
    }

    self->mThread = (void *)thread;
    return MAG_NO_ERROR;
}

static _status_t MagThread_Run(MagThread_t *self){
    _status_t ret;

    Mag_AcquireMutex(self->mLock);
    
    if (self->mRunning == MAG_TRUE){
        Mag_ReleaseMutex(self->mLock);
        AGILE_LOGE("the thread %s is running. ignore the run() command!", self->mName);
        // thread already started
        return MAG_INVALID_OPERATION; 
    }
    
    ret = createThread(self);
    if (ret == MAG_NO_ERROR){
        AGILE_LOGV("thread %s start to run!", self->mName);
        self->mRunning = MAG_TRUE;
    }

    Mag_ReleaseMutex(self->mLock);
    return ret;
}

_status_t MagThread_readyToRun(MagThread_t *self, fnReadyToRun fn){
    _status_t ret;
    
    Mag_AcquireMutex(self->mLock);
    if (self){
        self->mfnReadyToRun = fn;
        ret = MAG_NO_ERROR;
    }else{
        ret = MAG_NO_INIT;
    }
    Mag_ReleaseMutex(self->mLock);
    return ret;
}

_status_t MagThread_setParm_Priority(MagThread_t *self, MagThread_Priority_t priority){
    _status_t ret;
    
    Mag_AcquireMutex(self->mLock);
    if (self){
        self->mPriority = priority;
        ret = MAG_NO_ERROR;
    }else{
        ret = MAG_NO_INIT;
    }
    Mag_ReleaseMutex(self->mLock);
    return ret;
}

_status_t MagThread_setParm_StackSize(MagThread_t *self, _size_t stackSize){
    _status_t ret;
    
    Mag_AcquireMutex(self->mLock);
    if (self){
        self->mStackSize = stackSize;
        ret = MAG_NO_ERROR;
    }else{
        ret = MAG_NO_INIT;
    }
    Mag_ReleaseMutex(self->mLock);
    return ret;
}

_status_t  MagThread_requestExit(MagThread_t *self){
    _status_t ret = MAG_NO_ERROR;

    if (self->mRunning){
        Mag_AcquireMutex(self->mLock);
        if (self){
            self->mExitPending= MAG_TRUE;
        }else{
            ret = MAG_NO_INIT;
        }
        Mag_ReleaseMutex(self->mLock);
    }
    return ret;
}

_status_t MagThread_requestExitAndWait(MagThread_t *self, i32 timeout){
    _status_t ret = MAG_NO_ERROR;

    if (self->mRunning){
        Mag_AcquireMutex(self->mLock);
        if (self){
            self->mExitPending= MAG_TRUE;
        }else{
            ret = MAG_NO_INIT;
        }
        Mag_ReleaseMutex(self->mLock);

        Mag_WaitForEventGroup(self->mExitEvtGroup, MAG_EG_OR, timeout);
    }
    return ret;
}

MagThreadHandle Mag_CreateThread(const char* name, fnThreadLoop fn, void *priv){
    MagThread_t *thread;

    thread = (MagThread_t *)mag_mallocz(sizeof(MagThread_t));

    if (thread != NULL){
        thread->mfnTheadLoop = fn;
        thread->mPriv = priv;
        thread->mPriority = MAGTHREAD_PRIORITY_NORMAL;
        thread->mName = mag_strdup(name);

        Mag_CreateMutex(&thread->mLock);

        Mag_CreateEventGroup(&thread->mExitEvtGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&thread->mExitEvt, 0))
            Mag_AddEventGroup(thread->mExitEvtGroup, thread->mExitEvt);

        thread->run = MagThread_Run;
        thread->setFunc_readyToRun = MagThread_readyToRun;
        thread->setParm_Priority   = MagThread_setParm_Priority;
        thread->setParm_StackSize  = MagThread_setParm_StackSize;
        thread->requestExit        = MagThread_requestExit;
        thread->requestExitAndWait = MagThread_requestExitAndWait;
    }
    return thread;
}

void Mag_DestroyThread(MagThreadHandle self){
    if (self->mRunning){
        self->requestExitAndWait(self, MAG_TIMEOUT_INFINITE);
    }
    
    Mag_DestroyMutex(self->mLock);

    Mag_DestroyEvent(self->mExitEvt);
    Mag_DestroyEventGroup(self->mExitEvtGroup);
    mag_free(self->mName);
    mag_free(self);
}


