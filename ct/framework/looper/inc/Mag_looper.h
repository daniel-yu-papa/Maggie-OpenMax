#ifndef __MAG_LOOPER_H__
#define __MAG_LOOPER_H__

#include "Mag_pub_type.h"
#include "Mag_pub_def.h"
#include "Mag_list.h"
#include "Mag_thread.h"

#define NUM_PRE_ALLOCATED_EVENTS 64

typedef struct Event {
    list_t node;
    i64 mWhenMs;
    MagMessage_t *mMessage;
}MagLooperEvent_t;

typedef struct MagHandler{
    ui32 uiID;
    void (*onMessageReceived)(const void *msg, void *priv);    
    void (*setID)(i32 id);
    void *priv; /*the pointer to the concrete handler object*/
}MagHandler_t;

typedef MagHandler_t* MagHandlerHandle;

typedef struct MagLooper{
    ui8 *mpName;
    
    RBTreeNodeHandle mHandlerTreeRoot;

    list_t mNoDelayEvtQueue;
    RBTreeNodeHandle mDelayEvtTreeRoot;
    list_t mFreeEvtQueue;

    MagMutexHandle mLock;

    MagThreadHandle mLooperThread;
    MagEventGroupHandle mMQPushEvtGroup;
    MagEventHandle      mMQEmptyPushEvt;
    
    void (*registerHandler)(struct MagHandler *self, const MagHandler_t *handler);
    MagErr_t (*unregisterHandler)(struct MagHandler *self, i32 handlerID);
    MagErr_t (*start)(struct MagHandler *self);
    MagErr_t (*stop)(struct MagHandler *self);
    void (*postMessage)(const void *msg, i64 delayUs);
}MagLooper_t;

typedef MagLooper_t* MagLooperHandle;

MagLooperHandle createLooper(const ui8 *pName);
void destroyLooper(MagLooperHandle hlooper);

#endif