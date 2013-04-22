#include <errno.h>
#include <stdlib.h>

#include "Mag_base.h"
#include "agilelog.h"

void Mag_AssertFailed(const char *expr, const char *file, unsigned int line){
    AGILE_LOGE("!!! Assert '%s' Failed at %s:%d (err code: %s)\n", expr, file, line, strerror(errno));
    /* Derefering 0 will cause a SIGSEGV will usually produce a core dump. */
    *(volatile unsigned char *)0;
}


MagErr_t Mag_CreateMutex(MagMutexHandle *handler){
    *handler = (MagMutexHandle)malloc(sizeof(**handler));

    if(NULL == *handler)
        return MAG_NoMemory;

    if (pthread_mutex_init(&(*handler)->mutex, NULL)) {
        AGILE_LOGE("failed to create the MagMutexHandle. err code(%s)", strerror(errno));
        free(*handler);
        *handler = NULL;
        return MAG_ErrMutexCreate;
    }else{
        return MAG_ErrNone;
    }
}

MagErr_t Mag_DestroyMutex(MagMutexHandle handler){
    if (NULL == handler)
        return MAG_InvalidPointer;

    pthread_mutex_destroy(&handler->mutex);
    free(handler);
}

MagErr_t Mag_TryAcquireMutex(MagMutexHandle handler){
    int rc;

    rc = pthread_mutex_trylock(&handler->mutex);

    if (0 == rc){
        return MAG_ErrNone;
    }else if (EBUSY == rc){
        return MAG_TimeOut;
    }else{
        return MAG_Failure;
    }
}

MagErr_t Mag_AcquireMutex(MagMutexHandle handler){
    int rc;

    rc = pthread_mutex_lock(&handler->mutex);

    if (0 == rc){
        return MAG_ErrNone;
    }else{
        AGILE_LOGE("faile to lock the mutex(0x%lx)", (unsigned long)handler);
        return MAG_Failure;
    }
}

MagErr_t Mag_ReleaseMutex(MagMutexHandle handler){
    int rc;

    rc = pthread_mutex_unlock(&handler->mutex);

    if (0 == rc){
        return MAG_ErrNone;
    }else{
        AGILE_LOGE("faile to unlock the mutex(0x%lx)", (unsigned long)handler);
        return MAG_Failure;
    }
}


