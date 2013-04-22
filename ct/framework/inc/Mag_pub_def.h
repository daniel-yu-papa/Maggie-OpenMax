#ifndef _MAG_PUB_DEF_H__
#define _MAG_PUB_DEF_H__

typedef enum{
    MAG_ErrNone = 0,
    MAG_Failure,
    MAG_NoMemory,
    MAG_ErrMutexCreate,
    MAG_ErrCondCreate,
    MAG_ErrGetTime,
    MAG_TimeOut,
    MAG_BadParameter,
    MAG_BadEvtWait,
    MAG_ErrEvtSetOp,
    MAG_AssertFault,
    MAG_ErrMaxEventNum,
    MAG_EventStatusMeet,
    MAG_EventStatusErr,
    MAG_InvalidPointer,
}MagErr_t;

#define MAG_TIMEOUT_INFINITE 0xF0000000

typedef enum{
    MAG_FALSE = 0,
    MAG_TRUE = 1
}MAG_BOOL_t;

#endif
