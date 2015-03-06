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

typedef struct MagMemoryPool{
    List_t           memPoolListHead;     /*link the magMempoolInternal_t nodes*/
    List_t           allocatedMBListHead; /*organized as FIFO*/
    List_t           unusedMBListHead;    /*no order. used to list the unused magMemBlock_t for later using*/
    unsigned int     MemPoolTotal;        /* the total number of the mempools */
    magMemPoolCtrl_t ctrlFlag;
    ui32             totalBufSize;        /*total allocated buffer size*/
    ui32             bufferLimit;         /*in Bytes. 0: no limitation*/
    pthread_mutex_t  mutex;

    MagErr_t (*get)(struct MagMemoryPool *self, unsigned int bytes, void **ppBuf);
    MagErr_t (*put)(struct MagMemoryPool *self, void *pBuf);
    MagErr_t (*reset)(struct MagMemoryPool *self);
    void     (*setLimit)(struct MagMemoryPool *self, ui32 top_size);
    void     (*dump)(struct MagMemoryPool *self);
}MagMemoryPool_t;

typedef MagMemoryPool_t* MagMemoryPoolHandle;

MagMemoryPoolHandle Mag_createMemoryPool(unsigned int bytes, ui32 flags);
void Mag_destroyMemoryPool(MagMemoryPoolHandle *phMemPool);

#ifdef __cplusplus
}
#endif

#endif