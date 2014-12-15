#ifndef __MAG_MEM_POOL_H__
#define __MAG_MEM_POOL_H__

#include <pthread.h>
#include "Mag_pub_type.h"
#include "Mag_pub_def.h"
#include "Mag_list.h"
#include "Mag_agilelog.h"
#include "Mag_mem.h"

#ifdef __cplusplus
extern "C" {
#endif
#define BUF_ALIGN(size, align) (((unsigned int)((size)/(align)) + 1)*align)

typedef enum{
    NO_AUTO_EXPANDING,  /*if the mempool is used up, it would not create another mempool automatically*/
    AUTO_EXPANDING      /*if the mempool is used up, it would create another mempool automatically*/
}magMemPoolCtrl_t;

typedef struct{
    pthread_mutex_t mutex;
    List_t memPoolListHead;     /*link the magMempoolInternal_t nodes*/
    List_t allocatedMBListHead; /*organized as FIFO*/
    List_t unusedMBListHead;    /*no order. used to list the unused magMemBlock_t for later using*/
    unsigned int MemPoolTotal;  /* the total number of the mempools */
    magMemPoolCtrl_t ctrlFlag;
    ui32 totalBufSize;          /*total allocated buffer size*/
    ui32 bufferLimit;            /*in Bytes. 0: no limitation*/
}magMemPoolMgr_t;

typedef struct magMempoolInternal_t{
    List_t node;
    unsigned int memPoolSize;
    unsigned char *pMemPoolBuf;
    List_t freeMBListHead;      /*the free MB nearest to the Mem Pool is always the first node in the list*/   
    List_t *activeMBNode;       /*point to the MB that hit the allocation in the nearest time*/
    unsigned int index;         /* the index of the mempool in terms of the creation time*/
    boolean empty;              /* show the empty or not*/
}magMempoolInternal_t;

typedef struct{
    List_t node;
    unsigned int start;
    unsigned int end;
    unsigned char *pBuf;
    magMempoolInternal_t *pMemPool;
}magMemBlock_t;

typedef magMemPoolMgr_t* magMempoolHandle;

magMempoolHandle magMemPoolCreate(unsigned int bytes, ui32 flags);
void magMemPoolSetBufLimit(magMempoolHandle hMemPool, ui32 top_size);
MagErr_t magMemPoolGetBuffer(magMempoolHandle hMemPool, unsigned int bytes, void **ppBuf);
MagErr_t magMemPoolPutBuffer(magMempoolHandle hMemPool, void *pBuf);
MagErr_t magMemPoolReset(magMempoolHandle hMemPool);
void magMemPoolDestroy(magMempoolHandle *phMemPool);
void magMemPoolDump(magMempoolHandle hMemPool);

#ifdef __cplusplus
}
#endif

#endif