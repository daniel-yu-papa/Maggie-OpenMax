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