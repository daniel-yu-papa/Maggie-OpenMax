#ifndef __MAG_LOOPER_H__
#define __MAG_LOOPER_H__

#include "Mag_pub_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* definitions of Looper Message
*/

#define MAX_MSG_PAYLOAD_NUM 32

enum MsgPayloadType {
    TypeInt32,
    TypeInt64,
    TypeSize,
    TypeFloat,
    TypeDouble,
    TypePointer,
    TypeString,
};

typedef struct mag_item{
    union {
        i32     int32Value;
        i64     int64Value;
        _size_t sizeValue;
        fp32    floatValue;
        fp64    doubleValue;
        void    *ptrValue;
        char    *stringValue;
    } u;
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
    void (*setSize)(struct mag_message *msg, const char *name, _size_t value);
    void (*setFloat)(struct mag_message *msg, const char *name, fp32 value);
    void (*setDouble)(struct mag_message *msg, const char *name, fp64 value);
    void (*setPointer)(struct mag_message *msg, const char *name, void *value);
    void (*setString)(struct mag_message *msg, const char *name, const char *s);

    boolean (*findInt32)(struct mag_message *msg, const char *name, i32 *value);
    boolean (*findInt64)(struct mag_message *msg, const char *name, i64 *value);
    boolean (*findSize)(struct mag_message *msg, const char *name, _size_t *value);
    boolean (*findFloat)(struct mag_message *msg, const char *name, fp32 *value);
    boolean (*findDouble)(struct mag_message *msg, const char *name, fp64 *value);
    boolean (*findPointer)(struct mag_message *msg, const char *name, void **value);
    boolean (*findString)(struct mag_message *msg, const char *name, char **s);

    boolean (*postMessage)(struct mag_message *msg, i64 delayMs);
}MagMessage_t;

typedef MagMessage_t* MagMessageHandle;

MagMessageHandle createMagMessage(struct MagLooper *looper, ui32 what, ui32 target);
void             destroyMagMessage(MagMessageHandle msg);

/*
* definitions of Looper
*/

#define NUM_PRE_ALLOCATED_EVENTS 64

typedef struct Event {
    List_t node;
    i64 mWhenMs;
    struct mag_message *mMessage;
}MagLooperEvent_t;

typedef void (*fnOnMessageReceived)(const MagMessageHandle msg, void *priv);

typedef struct MagHandler{
    ui32 uiID;
    fnOnMessageReceived mMsgProcessor;
    void *priv; /*the pointer to the concrete handler object*/

    ui32 (*id)(struct MagHandler *h);
}MagHandler_t;

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

    i64 mDelayEvtWhenMS;
    
    void (*registerHandler)(struct MagLooper *self, const MagHandler_t *handler);
    _status_t (*unregisterHandler)(struct MagLooper *self, i32 handlerID);
    _status_t (*start)(struct MagLooper *self);
    _status_t (*stop)(struct MagLooper *self);
    void (*postMessage)(struct MagLooper *self, MagMessage_t *msg, i64 delayUs);

    ui32 (*getHandlerID)(struct MagLooper *self);
    _status_t (*waitOnAllDone)(struct MagLooper *self);
}MagLooper_t;

typedef MagLooper_t* MagLooperHandle;

MagLooperHandle createLooper(const char *pName);
void destroyLooper(MagLooperHandle hlooper);

MagHandlerHandle createHandler(MagLooperHandle hLooper, fnOnMessageReceived cb, void *priv);
void destroyHandler(MagHandlerHandle hHandler);

#ifdef __cplusplus
}
#endif

#endif