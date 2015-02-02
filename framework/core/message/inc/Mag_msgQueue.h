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

#ifndef __MAG_MESSAGE_QUEUE_H__
#define __MAG_MESSAGE_QUEUE_H__

#include "Mag_looper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAG_MQ_REALLOCATED_NODES 32

typedef struct{
    List_t node;
    MagMessageHandle msg;
}Mag_MsgQueueNode_t;

typedef struct mag_msg_queue{
    List_t mQueueHead;
    List_t mFreeHead;

    MagMutexHandle mLock;
    
    void (*put)(struct mag_msg_queue *h, MagMessageHandle msg);
    void (*get)(struct mag_msg_queue *h, MagMessageHandle *msg);
}Mag_MsgQueue_t;

typedef Mag_MsgQueue_t* Mag_MsgQueueHandle;

Mag_MsgQueueHandle Mag_CreateMsgQueue(void);
void Mag_DestroyMsgQueue(Mag_MsgQueueHandle *pHandle);

#ifdef __cplusplus
}
#endif

#endif