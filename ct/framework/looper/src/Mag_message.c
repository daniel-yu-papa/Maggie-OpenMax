#include "Mag_message.h"

static void freeItem(MagMessage_t *msg, MagItem_t *item){
    switch(item->mType){
        case TypeString:
            mag_free(item->u.stringValue);
            break;
        default:
            break;
    }
}

static const MagItem_t *findItem(MagMessage_t *msg, const char *name, MsgPayloadType type) const{
    ui32 i = 0;

    for(i = 0; i < msg->mNumItems; i++){
        if (strcmp(msg->mItem[i].mName, name) == 0 ){
            return msg->mItem[i].mType == type ? &msg->mItem[i] : NULL;
        }
    }
    return NULL;
}

static MagItem_t *allocateItem(MagMessage_t *msg, const char *name){
    ui32 i = 0;

    while(i < msg->mNumItems && strcmp(msg->mItem[i].mName, name) != 0){
        ++i;
    }

    MagItem_t *item;

    if (i < msg->mNumItems){
        item = &msg->mItems[i];
        freeItem(item);
    }else{
        MAG_ASSERT(msg->mNumItems < MAX_MSG_PAYLOAD_NUM);
        i = msg->mNumItems++;
        item = &msg->mItems[i];

        item->mName = mag_strdup(name);
    }
    return item;
}

static void MagMessage_setInt32(MagMessage_t *msg, const char *name, i32 value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeInt32;
    item->u.int32Value = value;
}

static void MagMessage_setInt64(MagMessage_t *msg, const char *name, i64 value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeInt64;
    item->u.int64Value = value;
}

static void MagMessage_setSize(MagMessage_t *msg, const char *name, _size_t value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeSize;
    item->u.sizeValue = value;
}

static void MagMessage_setFloat(MagMessage_t *msg, const char *name, fp32 value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeFloat;
    item->u.floatValue = value;
}

static void MagMessage_setDouble(MagMessage_t *msg, const char *name, fp64 value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeDouble;
    item->u.doubleValue = value;
}

static void MagMessage_setPointer(MagMessage_t *msg, const char *name, void *value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypePointer;
    item->u.ptrValue = value;
}

static void MagMessage_setString(MagMessage_t *msg, const char *name, const char *s){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeString;
    item->u.stringValue = mag_strdup(s);
}

static boolean MagMessage_findInt32(MagMessage_t *msg, const char *name, i32 *value){
    MagItem_t *item;

    item = findItem(msg, name, TypeInt32);
    if (item != NULL){
        *value = item->u.int32Value;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

static boolean MagMessage_findInt64(MagMessage_t *msg, const char *name, i64 *value){
    MagItem_t *item;

    item = findItem(msg, name, TypeInt64);
    if (item != NULL){
        *value = item->u.int64Value;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

static boolean MagMessage_findSize(MagMessage_t *msg, const char *name, _size_t *value){
    MagItem_t *item;

    item = findItem(msg, name, TypeSize);
    if (item != NULL){
        *value = item->u.sizeValue;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

static boolean MagMessage_findFloat(MagMessage_t *msg, const char *name, fp32 *value){
    MagItem_t *item;

    item = findItem(msg, name, TypeFloat);
    if (item != NULL){
        *value = item->u.floatValue;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

static boolean MagMessage_findDouble(MagMessage_t *msg, const char *name, fp64 *value){
    MagItem_t *item;

    item = findItem(msg, name, TypeDouble);
    if (item != NULL){
        *value = item->u.doubleValue;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

static boolean MagMessage_findPointer(MagMessage_t *msg, const char *name, void **value){
    MagItem_t *item;

    item = findItem(msg, name, TypePointer);
    if (item != NULL){
        *value = item->u.ptrValue;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

static boolean MagMessage_findString(MagMessage_t *msg, const char *name, char **s){
    MagItem_t *item;

    item = findItem(msg, name, TypeString);
    if (item != NULL){
        *s = item->u.stringValue;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}

MagMessage_t *createMessage(ui32 what, ui32 target){
    MagMessage_t *msg;

    msg = mag_malloc(sizeof(MagMessage_t));

    if (msg != NULL){
        msg->mWhat   = what;
        msg->mTarget = target;
        msg->mNumItems = 0;

        msg->setInt32   = MagMessage_setInt32;
        msg->setInt64   = MagMessage_setInt64;
        msg->setFloat   = MagMessage_setFloat;
        msg->setDouble  = MagMessage_setDouble;
        msg->setPointer = MagMessage_setPointer;
        msg->setString  = MagMessage_setString;
        msg->setSize    = MagMessage_setSize;

        msg->findInt32   = MagMessage_findInt32;
        msg->findInt64   = MagMessage_findInt64;
        msg->findFloat   = MagMessage_findFloat;
        msg->findDouble  = MagMessage_findDouble;
        msg->findPointer = MagMessage_findPointer;
        msg->findString  = MagMessage_findString;
        msg->findSize    = MagMessage_findSize;
    }

    return msg;
}

