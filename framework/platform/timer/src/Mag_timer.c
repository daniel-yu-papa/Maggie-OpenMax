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

#include "Mag_mem.h"
#include "Mag_agilelog.h"
#include "Mag_hal.h"
#include "Mag_timer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Timer"

static i64       MagTimer_get(MagTimerHandle handler){
    i64 getTime = 0;

    Mag_AcquireMutex(handler->mTimeLock);
    if (handler->mTimeResume == 0){
        /*first time to get the time*/
        if (!handler->mPaused){
            handler->mTimeResume = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
        }
    }else{
        if (!handler->mPaused){
            getTime = (Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll - handler->mTimeResume) + handler->mTimeSuspend;
        }else{
            getTime = handler->mTimeSuspend;
        }
    }
    Mag_ReleaseMutex(handler->mTimeLock);

    return getTime;
}

static void MagTimer_pause(MagTimerHandle handler){
    Mag_AcquireMutex(handler->mTimeLock);
    if (!handler->mPaused){
        if (handler->mTimeResume != 0){
            handler->mTimeSuspend = (Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll - handler->mTimeResume) + handler->mTimeSuspend;
        }
        handler->mPaused = MAG_TRUE;
    }
    Mag_ReleaseMutex(handler->mTimeLock);
}

static void MagTimer_resume(MagTimerHandle handler){
    Mag_AcquireMutex(handler->mTimeLock);
    if (handler->mPaused){
        if (handler->mTimeResume != 0){
            handler->mTimeResume  = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
        }
        handler->mPaused = MAG_FALSE;
    }
    Mag_ReleaseMutex(handler->mTimeLock);
}

MagTimerHandle Mag_createTimer(void){
    MagTimerHandle handle;

    handle = (MagTimerHandle)mag_mallocz(sizeof(MagTimer_t));
    if (NULL != handle){
        handle->mTimeSuspend = 0;
        handle->mTimeResume  = 0;
        handle->mPaused      = MAG_FALSE;
        Mag_CreateMutex(&handle->mTimeLock);

        handle->get    = MagTimer_get;
        handle->pause  = MagTimer_pause;
        handle->resume = MagTimer_resume;
    }else{
        AGILE_LOGE("failed to malloc MagTimer_t");
    }
    return handle;
}

void Mag_destroyTimer(MagTimerHandle *phTimer){
    if (*phTimer){
        Mag_DestroyMutex(&(*phTimer)->mTimeLock);
        mag_freep((void **)phTimer);
    }
}