#ifndef __MAG_LOOPER_H__
#define __MAG_LOOPER_H__

#include "Mag_pub_type.h"
#include "Mag_pub_def.h"
#include "Mag_list.h"
#include "Mag_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_PRE_ALLOCATED_EVENTS 64

typedef struct Event {
    list_t node;
    i64 mWhenMs;
    MagMessage_t *mMessage;
}MagLooperEvent_t;

typedef void (*fnOnMessageReceived)(const MagMessageHandle msg, void *priv);

typedef struct MagHandler{
    ui32 uiID;
    fnOnMessageReceived mMsgProcessor;
    void *priv; /*the pointer to the concrete handler object*/

    ui32 id(struct MagHandler *h);
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

    ui32 mLooperID;
    ui32 mHandlerID;
    
    void (*registerHandler)(struct MagLooper *self, const MagHandler_t *handler);
    MagErr_t (*unregisterHandler)(struct MagLooper *self, i32 handlerID);
    MagErr_t (*start)(struct MagLooper *self);
    MagErr_t (*stop)(struct MagLooper *self);
    void (*postMessage)(const void *msg, i64 delayUs);

    ui32 (*getHandlerID)(struct MagLooper *self);
}MagLooper_t;

typedef MagLooper_t* MagLooperHandle;

MagLooperHandle createLooper(const ui8 *pName);
void destroyLooper(MagLooperHandle hlooper);

MagHandlerHandle createHandler(ui32 id, fnOnMessageReceived cb);
void destroyHandler(MagHandlerHandle hHandler);

#ifdef __cplusplus
}
#endif

#endif