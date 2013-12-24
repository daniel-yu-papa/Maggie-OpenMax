#include "Mag_minidb.h"
#include "hashTableC.h"
#include "Mag_mem.h"
#include "agilelog.h"

void MagMiniDB_setInt32(struct mag_minidb *db, const char *name, i32 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.int32Value = value;
        hItem->mType = TypeInt32;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.int32Value = value;
            hItem->mType = TypeInt32;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setInt64(struct mag_minidb *db, const char *name, i64 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.int64Value = value;
        hItem->mType = TypeInt64;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.int64Value = value;
            hItem->mType = TypeInt64;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setSize(struct mag_minidb *db, const char *name, _size_t value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.sizeValue = value;
        hItem->mType = TypeSize;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.sizeValue = value;
            hItem->mType = TypeSize;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setFloat(struct mag_minidb *db, const char *name, fp32 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.floatValue = value;
        hItem->mType = TypeFloat;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.floatValue = value;
            hItem->mType = TypeFloat;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setDouble(struct mag_minidb *db, const char *name, fp64 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.doubleValue = value;
        hItem->mType = TypeDouble;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.doubleValue = value;
            hItem->mType = TypeDouble;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setPointer(struct mag_minidb *db, const char *name, void *value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.ptrValue = value;
        hItem->mType = TypePointer;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.ptrValue = value;
            hItem->mType = TypePointer;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setString(struct mag_minidb *db, const char *name, const char *s){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        mag_free(hItem->u.stringValue);
        hItem->u.stringValue = mag_strdup(s);
        hItem->mType = TypeString;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.stringValue = mag_strdup(s);
            hItem->mType = TypeString;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            StrHashTable_AddItem(db->mhHashTable, name, (void *)hItem);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

boolean MagMiniDB_findInt32(struct mag_minidb *db, const char *name, i32 *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypeInt32){
            *value = hItem->u.int32Value;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not Int32", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

boolean MagMiniDB_findInt64(struct mag_minidb *db, const char *name, i64 *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypeInt64){
            *value = hItem->u.int64Value;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not Int64", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

boolean MagMiniDB_findSize(struct mag_minidb *db, const char *name, _size_t *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypeSize){
            *value = hItem->u.sizeValue;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not _size_t", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

boolean MagMiniDB_findFloat(struct mag_minidb *db, const char *name, fp32 *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypeFloat){
            *value = hItem->u.floatValue;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not float", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

boolean MagMiniDB_findDouble(struct mag_minidb *db, const char *name, fp64 *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypeDouble){
            *value = hItem->u.doubleValue;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not double", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

boolean MagMiniDB_findPointer(struct mag_minidb *db, const char *name, void **value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypePointer){
            *value = hItem->u.ptrValue;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not pointer", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

boolean MagMiniDB_findString(struct mag_minidb *db, const char *name, char **s){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)StrHashTable_GetItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == TypeString){
            *s = hItem->u.stringValue;
            ret = MAG_TRUE;
        }else{
            AGILE_LOGE("the value type of the key:%s is %d, not pointer", name, hItem->mType);
            ret = MAG_FALSE;
        }     
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
        ret = MAG_FALSE;
    }
    Mag_ReleaseMutex(db->mLock);
    return ret;
}

MagMiniDBHandle createMagMiniDB(i32 maxItemsNum){
    MagMiniDBHandle h;

    h = mag_malloc(sizeof(MagMiniDB_t));

    if (h != NULL){
        h->mhHashTable = StrHashTable_Create(maxItemsNum);
        h->mItemNum    = 0;
        INIT_LIST(&h->mItemListHead);
        Mag_CreateMutex(&h->mLock);
        
        h->setInt32   = MagMiniDB_setInt32;
        h->setInt64   = MagMiniDB_setInt64;
        h->setFloat   = MagMiniDB_setFloat;
        h->setDouble  = MagMiniDB_setDouble;
        h->setPointer = MagMiniDB_setPointer;
        h->setString  = MagMiniDB_setString;
        h->setSize    = MagMiniDB_setSize;

        h->findInt32   = MagMiniDB_findInt32;
        h->findInt64   = MagMiniDB_findInt64;
        h->findFloat   = MagMiniDB_findFloat;
        h->findDouble  = MagMiniDB_findDouble;
        h->findPointer = MagMiniDB_findPointer;
        h->findString  = MagMiniDB_findString;
        h->findSize    = MagMiniDB_findSize;
    }

    return h;
}

void destroyMagMiniDB(MagMiniDBHandle db){
    List_t *tmpNode;
    MagMiniDBItemHandle hItem;
    
    StrHashTable_Destroy(db->mhHashTable);

    tmpNode = db->mItemListHead.next;

    while (tmpNode != &db->mItemListHead){
        hItem = (MagMiniDBItemHandle)list_entry(tmpNode, MagMiniDBItem_t, node);
        if (hItem->mType == TypeString)
            mag_free(hItem->u.stringValue);
        list_del(&hItem->node);
        mag_free(hItem);
        tmpNode = db->mItemListHead.next;
    }

    Mag_DestroyMutex(db->mLock);
}

