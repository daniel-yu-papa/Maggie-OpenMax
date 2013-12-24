#ifndef __MAG_MESSAGE_H__
#define __MAG_MESSAGE_H__

#include "Mag_pub_type.h"
#include "Mag_pub_def.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    MsgPayloadType mType;
}MagItem_t;

typedef struct mag_message{
    ui32 mTarget;
    ui32 mWhat;
    MagItem_t mItem[MAX_MSG_PAYLOAD_NUM];

    ui32 mNumItems;

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
}MagMessage_t;

typedef MagMessage_t* MagMessageHandle;

#ifdef __cplusplus
}
#endif

#endif