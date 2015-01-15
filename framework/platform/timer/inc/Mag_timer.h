#ifndef __MAG_TIMER_H__
#define __MAG_TIMER_H__

#include "Mag_pub_def.h"  
#include "Mag_pub_type.h"
#include "Mag_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MagTimer{
    i64 mTimeSuspend;
    i64 mTimeResume;
    MagMutexHandle mTimeLock;
    boolean mPaused;

    i64  (*get)(struct MagTimer *self);
    void (*pause)(struct MagTimer *self);
    void (*resume)(struct MagTimer *self);
}MagTimer_t;

typedef MagTimer_t* MagTimerHandle;

MagTimerHandle Mag_createTimer(void);
void Mag_destroyTimer(MagTimerHandle *phTimer);

#ifdef __cplusplus
}
#endif

#endif