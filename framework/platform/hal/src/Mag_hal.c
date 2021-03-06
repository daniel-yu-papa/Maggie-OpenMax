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

#include <errno.h>
#include <stdlib.h>

#include "Mag_hal.h"
#include "Mag_agilelog.h"
#include "Mag_mem.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Hal"


void Mag_AssertFailed(const char *expr, const char *file, unsigned int line){
    AGILE_LOGE("!!! Assert '%s' Failed at %s:%d (err code: %s)\n", expr, file, line, strerror(errno));
#ifdef MAG_DEBUG
    /* Derefering 0 will cause a SIGSEGV will usually produce a core dump. */
    *(volatile unsigned char *)0;
#else
#endif
}

MagErr_t Mag_CreateMutex(MagMutexHandle *handler){
    *handler = (MagMutexHandle)mag_mallocz(sizeof(**handler));

    if(NULL == *handler){
        return MAG_NoMemory;
    }

    if (pthread_mutex_init(&(*handler)->mutex, NULL)) {
        AGILE_LOGE("failed to create the MagMutexHandle. err code(%s)", strerror(errno));
        free(*handler);
        *handler = NULL;
        return MAG_ErrMutexCreate;
    }else{
        return MAG_ErrNone;
    }
}

MagErr_t Mag_DestroyMutex(MagMutexHandle *pHandler){
    MagMutexHandle handler = *pHandler;

    if (NULL == handler){
        return MAG_InvalidPointer;
    }

    pthread_mutex_destroy(&handler->mutex);
    mag_freep((void **)pHandler);
    return MAG_ErrNone;
}

MagErr_t Mag_TryAcquireMutex(MagMutexHandle handler){
    int rc;

    if (handler == NULL){
        return MAG_InvalidPointer;
    }

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

    if (handler == NULL){
        return MAG_InvalidPointer;
    }

    rc = pthread_mutex_lock(&handler->mutex);

    if (0 == rc){
        return MAG_ErrNone;
    }else{
        AGILE_LOGE("failed to lock the mutex(0x%lx)", (unsigned long)handler);
        return MAG_Failure;
    }
}

MagErr_t Mag_ReleaseMutex(MagMutexHandle handler){
    int rc;

    if (handler == NULL){
        return MAG_InvalidPointer;
    }

    rc = pthread_mutex_unlock(&handler->mutex);

    if (0 == rc){
        return MAG_ErrNone;
    }else{
        AGILE_LOGE("failed to unlock the mutex(0x%lx)", (unsigned long)handler);
        return MAG_Failure;
    }
}

/*return current system time in ns*/
ui64 Mag_GetSystemTime(i32 clock)
{
#if defined(HAVE_POSIX_CLOCKS)
    static const clockid_t clocks[] = {
            CLOCK_REALTIME,
            CLOCK_MONOTONIC,
            CLOCK_PROCESS_CPUTIME_ID,
            CLOCK_THREAD_CPUTIME_ID
    };
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(clocks[clock], &t);
    return (ui64)(t.tv_sec)*1000000000LL + t.tv_nsec;
#else
    /*we don't support the clocks here.*/
    struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    return (ui64)(t.tv_sec)*1000000000LL + (ui64)(t.tv_usec)*1000LL;
#endif
}

void Mag_TimeTakenStatistic(boolean start, const char *func, const char *spec){
    static ui64 start_time;

    if (start){
        start_time = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000000ll;
    }else{
        AGILE_LOGD("%s: %s -- %lld ms", func, spec ? spec : "", 
                    Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000000ll - start_time);
    }
}

