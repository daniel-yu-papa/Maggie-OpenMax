#ifndef __MAG_MESSAGE_H__
#define __MAG_MESSAGE_H__

#include "Mag_pub_type.h"
#include "Mag_pub_def.h"

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

    void (*setInt32)(const char *name, i32 value);
    void (*setInt64)(const char *name, i64 value);
    void (*setSize)(const char *name, _size_t value);
    void (*setFloat)(const char *name, fp32 value);
    void (*setDouble)(const char *name, fp64 value);
    void (*setPointer)(const char *name, void *value);
    void (*setString)(const char *name, const char *s);

    boolean (*findInt32)(const char *name, i32 *value);
    boolean (*findInt64)(const char *name, i64 *value);
    boolean (*findSize)(const char *name, _size_t *value);
    boolean (*findFloat)(const char *name, fp32 *value);
    boolean (*findDouble)(const char *name, fp64 *value);
    boolean (*findPointer)(const char *name, void **value);
    boolean (*findString)(const char *name, char **s);
}MagMessage_t;

#endif