#ifndef __MAG_LOOPER_H__
#define __MAG_LOOPER_H__

#include "Mag_list.h"
#include "Mag_rbtree.h"
#include "Mag_pub_def.h"  
#include "Mag_pub_type.h"
#include "Mag_agilelog.h"
#include "Mag_hal.h"
#include "Mag_thread.h"
#include "Mag_mem.h"
#include "Mag_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* definitions of Looper Message
*/

#define MAX_MSG_PAYLOAD_NUM 32

struct mag_message;

enum MsgPayloadType {
    TypeInt32,
    TypeInt64,
    TypeSize,
    TypeFloat,
    TypeDouble,
    TypePointer,
    TypeString,
    TypeMessage
};

typedef struct mag_item{
    union {
        i32     int32Value;
        i64     int64Value;
        ui32    ui32Value;
        fp32    floatValue;
        fp64    doubleValue;
        void    *ptrValue;
        char    *stringValue;
        struct mag_message *messageValue;
    } u;
    /*
     *MAG_TRUE[default]: the ptrValue/stringValue/messageValue is freed by message
     *MAG_FALSE: the ptrValue/stringValue/messageValue is freed by allocator. Do nothing in message destroy.
     */
    boolean mOwnedByMsg;
    const char *mName;
    enum MsgPayloadType mType;
}MagItem_t;

struct MagLooper;

typedef struct mag_message{
    ui32 mTarget;
    ui32 mWhat;
    MagItem_t mItems[MAX_MSG_PAYLOAD_NUM];

    ui32 mNumItems;

    struct MagLooper *mLooper;
    
    ui32 (*what)(struct mag_message *msg);
    
    void (*setInt32)(struct mag_message *msg, const char *name, i32 value);
    void (*setInt64)(struct mag_message *msg, const char *name, i64 value);
    void (*setUInt32)(struct mag_message *msg, const char *name, ui32 value);
    void (*setFloat)(struct mag_message *msg, const char *name, fp32 value);
    void (*setDouble)(struct mag_message *msg, const char *name, fp64 value);
    void (*setPointer)(struct mag_message *msg, const char *name, void *value, boolean owndedByMsg);
    void (*setString)(struct mag_message *msg, const char *name, const char *s);
    void (*setMessage)(struct mag_message *msg, const char *name, struct mag_message *message, boolean owndedByMsg);
    
    boolean (*findInt32)(struct mag_message *msg, const char *name, i32 *value);
    boolean (*findInt64)(struct mag_message *msg, const char *name, i64 *value);
    boolean (*findUInt32)(struct mag_message *msg, const char *name, ui32 *value);
    boolean (*findFloat)(struct mag_message *msg, const char *name, fp32 *value);
    boolean (*findDouble)(struct mag_message *msg, const char *name, fp64 *value);
    boolean (*findPointer)(struct mag_message *msg, const char *name, void **value);
    boolean (*findString)(struct mag_message *msg, const char *name, char **s);
    boolean (*findMessage)(struct mag_message *msg, const char *name, struct mag_message **message);

    boolean (*postMessage)(struct mag_message *msg, i64 delayUs);
}MagMessage_t;

typedef MagMessage_t* MagMessageHandle;

MagMessageHandle createMagMessage(struct MagLooper *looper, ui32 what, ui32 target);
void             destroyMagMessage(MagMessageHandle *pMsg);

/*
* definitions of Looper
*/

#define NUM_PRE_ALLOCATED_EVENTS 32

typedef struct Event {
    List_t node;
    i64 mWhenUS;
    struct mag_message *mMessage;
}MagLooperEvent_t;

typedef void (*fnOnMessageReceived)(const MagMessageHandle msg, void *priv);

typedef struct MagHandler{
    ui32 uiID;
    fnOnMessageReceived mMsgProcessor;
    void *priv; /*the pointer to the concrete handler object*/

    ui32 (*id)(struct MagHandler *h);
}MagHandler_t;

typedef enum{
    MagLooper_Priority_Normal,
    MagLooper_Priority_Low,
    MagLooper_Priority_High
}MagLooperPriority_t;

typedef MagHandler_t* MagHandlerHandle;

typedef struct MagLooper{
    ui8 *mpName;
    
    RBTreeNodeHandle mHandlerTreeRoot;

    List_t mNoDelayEvtQueue;
    RBTreeNodeHandle mDelayEvtTreeRoot;
    List_t mFreeEvtQueue;

    MagMutexHandle mLock;

    MagThreadHandle mLooperThread;
    MagEventGroupHandle mMQPushEvtGroup;
    MagEventHandle      mMQEmptyPushEvt;

    ui32 mLooperID;
    ui32 mHandlerID;

    i64 mDelayEvtWhenUS;

    i32 mFreeNodeNum;
    boolean mEventInExecuting;
    boolean mMergeSameTypeMsg;

    boolean mForceOut;
    boolean mReportPolicy;

    MagTimerHandle mTimer;

    void      (*registerHandler)(struct MagLooper *self, const MagHandler_t *handler);
    _status_t (*unregisterHandler)(struct MagLooper *self, i32 handlerID);
    _status_t (*start)(struct MagLooper *self);
    _status_t (*stop)(struct MagLooper *self);
    _status_t (*suspend)(struct MagLooper *self);
    _status_t (*resume)(struct MagLooper *self);
    void      (*clear)(struct MagLooper *self);
    void      (*forceOut)(struct MagLooper *self); /*force all delay message released immediately*/
    void      (*postMessage)(struct MagLooper *self, MagMessage_t *msg, i64 delayUS);

    ui32      (*getHandlerID)(struct MagLooper *self);
    _status_t (*waitOnAllDone)(struct MagLooper *self);
    void      (*setMergeMsg)(struct MagLooper *self);
    void      (*setPriority)(struct MagLooper *self, MagLooperPriority_t pri);
    void      (*setTimer)(struct MagLooper *self, MagTimerHandle hTimer);
}MagLooper_t;

typedef MagLooper_t* MagLooperHandle;

MagLooperHandle createLooper(const char *pName);
void destroyLooper(MagLooperHandle *phlooper);

MagHandlerHandle createHandler(MagLooperHandle hLooper, fnOnMessageReceived cb, void *priv);
void destroyHandler(MagHandlerHandle *phHandler);

#ifdef __cplusplus
}
#endif

#endif