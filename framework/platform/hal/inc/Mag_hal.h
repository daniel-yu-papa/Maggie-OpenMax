#ifndef _MAG_BASE_H__
#define _MAG_BASE_H__

#include "Mag_pub_def.h"
#include "Mag_pub_type.h"
#include <pthread.h>
#include <time.h>

#define HAVE_POSIX_CLOCKS
#ifdef __cplusplus
extern "C" {
#endif

enum {
    MAG_SYSTEM_TIME_REALTIME = 0,  // system-wide realtime clock
    MAG_SYSTEM_TIME_MONOTONIC = 1, // monotonic time since unspecified starting point
    MAG_SYSTEM_TIME_PROCESS = 2,   // high-resolution per-process clock
    MAG_SYSTEM_TIME_THREAD = 3,    // high-resolution per-thread clock
    MAG_SYSTEM_TIME_BOOTTIME = 4   // same as SYSTEM_TIME_MONOTONIC, but including CPU suspend time
};

struct MAG_MutexObj{
    pthread_mutex_t mutex;
};

typedef struct MAG_MutexObj *MagMutexHandle;

#define MAG_ASSERT(expr) (expr) ? (void) 0 : Mag_AssertFailed(#expr, __FILE__, __LINE__)

void Mag_AssertFailed(const char *expr, const char *file, unsigned int line);

MagErr_t Mag_CreateMutex(MagMutexHandle *handler);
MagErr_t Mag_DestroyMutex(MagMutexHandle handler);
MagErr_t Mag_TryAcquireMutex(MagMutexHandle handler);
MagErr_t Mag_AcquireMutex(MagMutexHandle handler);
MagErr_t Mag_ReleaseMutex(MagMutexHandle handler);

/*for C++ static mutexes using*/
typedef pthread_mutex_t MagStaticMutex;
    
#define MAG_STATIC_MUTEX_INITIALIZER(lock) \
        lock = PTHREAD_MUTEX_INITIALIZER

#define MAG_STATIC_MUTEX_Acquire(lock) \
        pthread_mutex_lock(&lock)

#define MAG_STATIC_MUTEX_Release(lock) \
        pthread_mutex_unlock(&lock)
        
ui64 Mag_GetSystemTime(i32 clock);
#ifdef __cplusplus
}
#endif

#endif