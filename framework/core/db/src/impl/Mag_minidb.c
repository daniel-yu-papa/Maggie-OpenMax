#include "Mag_minidb.h"
#include "Mag_agilelog.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-MiniDB"


void MagMiniDB_setInt32(struct mag_minidb *db, const char *name, i32 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.int32Value = value;
        hItem->mType = MagMiniDB_TypeInt32;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.int32Value = value;
            hItem->mType = MagMiniDB_TypeInt32;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setInt64(struct mag_minidb *db, const char *name, i64 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.int64Value = value;
        hItem->mType = MagMiniDB_TypeInt64;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.int64Value = value;
            hItem->mType = MagMiniDB_TypeInt64;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setUInt32(struct mag_minidb *db, const char *name, ui32 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.uint32Value = value;
        hItem->mType = MagMiniDB_TypeUInt32;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.uint32Value = value;
            hItem->mType = MagMiniDB_TypeUInt32;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setFloat(struct mag_minidb *db, const char *name, fp32 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.floatValue = value;
        hItem->mType = MagMiniDB_TypeFloat;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.floatValue = value;
            hItem->mType = MagMiniDB_TypeFloat;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setDouble(struct mag_minidb *db, const char *name, fp64 value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        hItem->u.doubleValue = value;
        hItem->mType = MagMiniDB_TypeDouble;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.doubleValue = value;
            hItem->mType = MagMiniDB_TypeDouble;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setPointer(struct mag_minidb *db, const char *name, void *value){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        AGILE_LOGV("name: %s, the old pointer: 0x%x", name, hItem->u.ptrValue);
        mag_free(hItem->u.ptrValue);
        hItem->u.ptrValue = value;
        hItem->mType = MagMiniDB_TypePointer;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.ptrValue = value;
            hItem->mType = MagMiniDB_TypePointer;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

void MagMiniDB_setString(struct mag_minidb *db, const char *name, const char *s){
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        mag_free(hItem->u.stringValue);
        hItem->u.stringValue = mag_strdup(s);
        hItem->mType = MagMiniDB_TypeString;
    }else{
        hItem = (MagMiniDBItemHandle)mag_malloc(sizeof(MagMiniDBItem_t));
        if (NULL != hItem){
            hItem->u.stringValue = mag_strdup(s);
            hItem->mType = MagMiniDB_TypeString;
            hItem->mName = mag_strdup(name);
            INIT_LIST(&hItem->node);
            list_add(&hItem->node, &db->mItemListHead);
            db->mhHashTable->addItem(db->mhHashTable, (void *)hItem, name);
        }
    }
    Mag_ReleaseMutex(db->mLock);
}

boolean MagMiniDB_findInt32(struct mag_minidb *db, const char *name, i32 *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeInt32){
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
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeInt64){
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

boolean MagMiniDB_findUInt32(struct mag_minidb *db, const char *name, ui32 *value){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeUInt32){
            *value = hItem->u.uint32Value;
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
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeFloat){
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
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeDouble){
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
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypePointer){
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
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeString){
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

/* once the string/pointer type items are copied to other mag_minidb object. 
  * the items must be deferenced cause the copied mag_minidb object takes responsibility to free them.*/
void    MagMiniDB_derefItem(struct mag_minidb *db, const char *name){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeString){
            AGILE_LOGV("set [%s] string pointer to NULL");
            hItem->u.stringValue = NULL;
        }
        
        if (hItem->mType == MagMiniDB_TypePointer){
            AGILE_LOGV("set [%s] pointer to NULL");
            hItem->u.ptrValue = NULL;
        } 
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
    }
    Mag_ReleaseMutex(db->mLock);
}

void    MagMiniDB_deleteItem(struct mag_minidb *db, const char *name){
    MagMiniDBItemHandle hItem;
    boolean ret;

    Mag_AcquireMutex(db->mLock);
    hItem = (MagMiniDBItemHandle)db->mhHashTable->getItem(db->mhHashTable, name);
    if (NULL != hItem){
        if (hItem->mType == MagMiniDB_TypeString){
            mag_free(hItem->u.stringValue);
            hItem->u.stringValue = NULL;
        }
        
        if (hItem->mType == MagMiniDB_TypePointer){
            mag_free(hItem->u.ptrValue);
            hItem->u.ptrValue = NULL;
        } 
        
        mag_free(hItem->mName);
        list_del(&hItem->node);
        mag_free(hItem);
        db->mhHashTable->delItem(db->mhHashTable, name);
    }else{
        AGILE_LOGE("failed to find the key: %s", name);
    }
    Mag_ReleaseMutex(db->mLock);
}

MagMiniDBHandle createMagMiniDB(i32 maxItemsNum){
    MagMiniDBHandle h;

    h = mag_malloc(sizeof(MagMiniDB_t));

    if (h != NULL){
        h->mhHashTable = createMagStrHashTable(maxItemsNum);
        h->mItemNum    = 0;
        INIT_LIST(&h->mItemListHead);
        Mag_CreateMutex(&h->mLock);
        
        h->setInt32   = MagMiniDB_setInt32;
        h->setInt64   = MagMiniDB_setInt64;
        h->setFloat   = MagMiniDB_setFloat;
        h->setDouble  = MagMiniDB_setDouble;
        h->setPointer = MagMiniDB_setPointer;
        h->setString  = MagMiniDB_setString;
        h->setUInt32  = MagMiniDB_setUInt32;

        h->findInt32   = MagMiniDB_findInt32;
        h->findInt64   = MagMiniDB_findInt64;
        h->findFloat   = MagMiniDB_findFloat;
        h->findDouble  = MagMiniDB_findDouble;
        h->findPointer = MagMiniDB_findPointer;
        h->findString  = MagMiniDB_findString;
        h->findUInt32  = MagMiniDB_findUInt32;

        h->deleteItem  = MagMiniDB_deleteItem;
        h->derefItem   = MagMiniDB_derefItem;
    }

    return h;
}

void destroyMagMiniDB(MagMiniDBHandle db){
    List_t *tmpNode;
    MagMiniDBItemHandle hItem;

    Mag_AcquireMutex(db->mLock);
    
    destroyMagStrHashTable(db->mhHashTable);

    tmpNode = db->mItemListHead.next;

    while (tmpNode != &db->mItemListHead){
        hItem = (MagMiniDBItemHandle)list_entry(tmpNode, MagMiniDBItem_t, node);
        if (hItem->mType == MagMiniDB_TypeString)
            mag_free(hItem->u.stringValue);
        if (hItem->mType == MagMiniDB_TypePointer)
            mag_free(hItem->u.ptrValue);

        mag_free(hItem->mName);
        list_del(&hItem->node);
        mag_free(hItem);
        tmpNode = db->mItemListHead.next;
    }
    Mag_ReleaseMutex(db->mLock);
    
    Mag_DestroyMutex(db->mLock);
    mag_free(db);
}

