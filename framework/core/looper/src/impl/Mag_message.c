#include "Mag_looper.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-Looper"

static void freeItem(MagItem_t *item){
    switch(item->mType){
        case TypeString:
            mag_freep((void **)&item->u.stringValue);
            break;

        case TypePointer:
            if (item->mOwnedByMsg)
                mag_freep((void **)&item->u.ptrValue);
            break;

        case TypeMessage:
            if (item->mOwnedByMsg)
                destroyMagMessage(&item->u.messageValue);
            break;

        default:
            break;
    }
}

static MagItem_t *findItem(MagMessage_t *msg, const char *name, enum MsgPayloadType type){
    ui32 i = 0;

    for(i = 0; i < msg->mNumItems; i++){
        if (strcmp(msg->mItems[i].mName, name) == 0 ){
            return msg->mItems[i].mType == type ? &msg->mItems[i] : NULL;
        }
    }
    return NULL;
}

static MagItem_t *allocateItem(MagMessage_t *msg, const char *name){
    ui32 i = 0;
    MagItem_t *item;

    while(i < msg->mNumItems && strcmp(msg->mItems[i].mName, name) != 0){
        ++i;
    }

    if (i < msg->mNumItems){
        item = &msg->mItems[i];
        freeItem(item);
    }else{
        MAG_ASSERT(msg->mNumItems < MAX_MSG_PAYLOAD_NUM);
        i = msg->mNumItems++;
        item = &msg->mItems[i];
        
        item->mOwnedByMsg = MAG_TRUE;
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

static void MagMessage_setUInt32(MagMessage_t *msg, const char *name, ui32 value){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeSize;
    item->u.ui32Value = value;
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

static void MagMessage_setPointer(MagMessage_t *msg, const char *name, void *value, boolean owndedByMsg){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypePointer;
    item->mOwnedByMsg = owndedByMsg;
    item->u.ptrValue = value;
}

static void MagMessage_setString(MagMessage_t *msg, const char *name, const char *s){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeString;
    item->u.stringValue = mag_strdup(s);
}

static void MagMessage_setMessage(MagMessage_t *msg, const char *name, struct mag_message *message, boolean owndedByMsg){
    MagItem_t *item;

    item = allocateItem(msg, name);
    item->mType = TypeMessage;
    item->mOwnedByMsg = owndedByMsg;
    item->u.messageValue = message;
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

static boolean MagMessage_findUInt32(MagMessage_t *msg, const char *name, ui32 *value){
    MagItem_t *item;

    item = findItem(msg, name, TypeSize);
    if (item != NULL){
        *value = item->u.ui32Value;
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

static boolean MagMessage_findMessage(MagMessage_t *msg, const char *name, struct mag_message **message){
    MagItem_t *item;

    item = findItem(msg, name, TypeMessage);
    if (item != NULL){
        *message = item->u.messageValue;
        return MAG_TRUE;
    }
    return MAG_FALSE;
}


static ui32 MagMessage_what(MagMessage_t *msg){
    return msg->mWhat;
}

static boolean MagMessage_postMessage(MagMessage_t *msg, i64 delayMs){
    if (msg->mLooper != NULL){
        AGILE_LOGV("msg(%d) do post() in looper(0x%x)", msg->mWhat, msg->mLooper);
        msg->mLooper->postMessage(msg->mLooper, msg, delayMs);
        return MAG_TRUE;
    }else{
        return MAG_FALSE;
    }
}

MagMessageHandle createMagMessage(struct MagLooper *looper, ui32 what, ui32 target){
    MagMessage_t *msg;

    msg = mag_malloc(sizeof(MagMessage_t));

    if (msg != NULL){
        msg->mLooper = looper;
        msg->mWhat   = what;
        msg->mTarget = target;
        msg->mNumItems = 0;

        msg->what       = MagMessage_what;
        msg->setInt32   = MagMessage_setInt32;
        msg->setInt64   = MagMessage_setInt64;
        msg->setFloat   = MagMessage_setFloat;
        msg->setDouble  = MagMessage_setDouble;
        msg->setPointer = MagMessage_setPointer;
        msg->setString  = MagMessage_setString;
        msg->setUInt32  = MagMessage_setUInt32;
        msg->setMessage = MagMessage_setMessage;
        
        msg->findInt32   = MagMessage_findInt32;
        msg->findInt64   = MagMessage_findInt64;
        msg->findFloat   = MagMessage_findFloat;
        msg->findDouble  = MagMessage_findDouble;
        msg->findPointer = MagMessage_findPointer;
        msg->findString  = MagMessage_findString;
        msg->findUInt32  = MagMessage_findUInt32;
        msg->findMessage = MagMessage_findMessage;
        
        msg->postMessage = MagMessage_postMessage;
    }

    return msg;
}

void             destroyMagMessage(MagMessageHandle *pMsg){
    ui32 i;
    MagMessageHandle msg = *pMsg;

    if (msg == NULL)
        return;
    
    for (i = 0; i < msg->mNumItems; i++){
        if ((msg->mItems[i].mOwnedByMsg) && (msg->mItems[i].mType == TypePointer)){
            mag_freep((void **)&msg->mItems[i].u.ptrValue);
        }

        if (msg->mItems[i].mType == TypeString){
            mag_freep((void **)&msg->mItems[i].u.stringValue);
        }

        if ((msg->mItems[i].mOwnedByMsg) && (msg->mItems[i].mType == TypeMessage)){
            destroyMagMessage(&msg->mItems[i].u.messageValue);
        }
    }
    mag_freep((void **)pMsg);
}

