#ifndef _MAG_BASE_H__
#define _MAG_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "Mag_pub_def.h"
#include <pthread.h>

struct MAG_MutexObj{
    pthread_mutex_t mutex;
};

typedef struct MAG_MutexObj *MagMutexHandle;

#ifdef MAG_DEBUG
#define MAG_ASSERT(expr) (expr) ? (void) 0 : Mag_AssertFailed(#expr, __FILE__, __LINE__)
#else
#define MAG_ASSERT(expr) ret = MAG_AssertFault
#endif

void Mag_AssertFailed(const char *expr, const char *file, unsigned int line);

MagErr_t Mag_CreateMutex(MagMutexHandle *handler);
MagErr_t Mag_DestroyMutex(MagMutexHandle handler);
MagErr_t Mag_TryAcquireMutex(MagMutexHandle handler);
MagErr_t Mag_AcquireMutex(MagMutexHandle handler);
MagErr_t Mag_ReleaseMutex(MagMutexHandle handler);

#ifdef __cplusplus
}
#endif

#endif