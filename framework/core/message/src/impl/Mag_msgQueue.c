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

#include "Mag_msgQueue.h"
#include "Mag_mem.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Message"


static Mag_MsgQueueNode_t *getFreeMsg(Mag_MsgQueueHandle h){
    List_t *tmpNode;
    Mag_MsgQueueNode_t *pMsg = NULL;
    
    tmpNode = h->mFreeHead.next;

    if(tmpNode == &h->mFreeHead){
        AGILE_LOGV("no nodes in the free list. malloc one");
        pMsg = mag_malloc(sizeof(Mag_MsgQueueNode_t));
        if (pMsg != NULL)
            INIT_LIST(&pMsg->node);
    }else{
        list_del(tmpNode);
        pMsg = (Mag_MsgQueueNode_t *)list_entry(tmpNode, Mag_MsgQueueNode_t, node);    
    }
    return pMsg;
}

static void MagMsgQueue_get(Mag_MsgQueueHandle h, MagMessageHandle *msg){
    List_t *tmpNode;
    Mag_MsgQueueNode_t *pMsg = NULL;

    Mag_AcquireMutex(h->mLock);
    
    tmpNode = h->mQueueHead.next;
    if (tmpNode == &h->mQueueHead){
        *msg = NULL;
    }else{
        pMsg = (Mag_MsgQueueNode_t *)list_entry(tmpNode, Mag_MsgQueueNode_t, node); 
        *msg = pMsg->msg;
        AGILE_LOGD("get message: %d", pMsg->msg->what(pMsg->msg));
        list_del(tmpNode);
        list_add(tmpNode, &h->mFreeHead);
    }
    
    Mag_ReleaseMutex(h->mLock);
}

static void MagMsgQueue_put(Mag_MsgQueueHandle h, MagMessageHandle msg){
    Mag_MsgQueueNode_t *node;
    if (msg == NULL)
        return;
    
    Mag_AcquireMutex(h->mLock);
    
    node = getFreeMsg(h);
    node->msg = msg;
    AGILE_LOGD("put message: %d", msg->what(msg));
    list_add_tail(&node->node, &h->mQueueHead);

    Mag_ReleaseMutex(h->mLock);
}

Mag_MsgQueueHandle Mag_CreateMsgQueue(void){
    Mag_MsgQueueHandle h;
    ui32 i;
    Mag_MsgQueueNode_t *pMsg;

    h = mag_mallocz(sizeof(Mag_MsgQueue_t));
    if (h != NULL){
        INIT_LIST(&h->mQueueHead);
        INIT_LIST(&h->mFreeHead);
        h->get = MagMsgQueue_get;
        h->put = MagMsgQueue_put;

        Mag_CreateMutex(&h->mLock);

        for (i = 0; i < MAG_MQ_REALLOCATED_NODES; i++){
            pMsg = mag_malloc(sizeof(Mag_MsgQueueNode_t));
            INIT_LIST(&pMsg->node);
            list_add(&pMsg->node, &h->mFreeHead);
        }
    }else{
        AGILE_LOGE("failed to malloc Mag_MsgQueueHandle");
    }
    return h;
}

void Mag_DestroyMsgQueue(Mag_MsgQueueHandle *pHandle){
    List_t *tmpNode;
    Mag_MsgQueueNode_t *node;
    Mag_MsgQueueHandle h = *pHandle;

    Mag_AcquireMutex(h->mLock);
    
    tmpNode = h->mQueueHead.next;
    while (tmpNode != &h->mQueueHead){
        list_del(tmpNode);
        node = (Mag_MsgQueueNode_t *)list_entry(tmpNode, Mag_MsgQueueNode_t, node);   
        mag_free(node);
        tmpNode = h->mQueueHead.next;
    }

    tmpNode = h->mFreeHead.next;
    while (tmpNode != &h->mFreeHead){
        list_del(tmpNode);
        node = (Mag_MsgQueueNode_t *)list_entry(tmpNode, Mag_MsgQueueNode_t, node);   
        mag_free(node);
        tmpNode = h->mFreeHead.next;
    }

    Mag_ReleaseMutex(h->mLock);

    Mag_DestroyMutex(&h->mLock);
    
    mag_freep((void **)pHandle);

}

