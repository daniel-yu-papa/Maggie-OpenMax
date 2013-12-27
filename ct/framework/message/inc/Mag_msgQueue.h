#ifndef __MAG_MESSAGE_QUEUE_H__
#define __MAG_MESSAGE_QUEUE_H__

#include "Mag_pub_def.h"
#include "Mag_pub_type.h"
#include "Mag_base.h"
#include "Mag_message.h"

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

Mag_MsgQueueHandle Mag_CreateMsgQueue();
void Mag_DestroyMsgQueue(Mag_MsgQueueHandle h);

#ifdef __cplusplus
}
#endif

#endif