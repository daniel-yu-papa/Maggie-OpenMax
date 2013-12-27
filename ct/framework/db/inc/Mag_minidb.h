#ifndef __MAG_MINIDB_H__
#define __MAG_MINIDB_H__

#include "Mag_pub_type.h"
#include "Mag_pub_def.h"
#include "Mag_list.h"
#include "Mag_base.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MagMiniDBItemType {
    TypeInt32,
    TypeInt64,
    TypeUInt32,
    TypeFloat,
    TypeDouble,
    TypePointer,
    TypeString,
};

typedef struct mag_minidb_item{
    List_t node;
    
    union {
        i32     int32Value;
        i64     int64Value;
        ui32    uint32Value;
        fp32    floatValue;
        fp64    doubleValue;
        void    *ptrValue;
        char    *stringValue;
    } u;
    const char *mName;
    enum MagMiniDBItemType mType;
}MagMiniDBItem_t;

typedef MagMiniDBItem_t* MagMiniDBItemHandle;

typedef struct mag_minidb{
    void *mhHashTable;
    List_t mItemListHead;
    ui32 mItemNum;
    MagMutexHandle mLock;
    
    void (*setInt32)(struct mag_minidb *db, const char *name, i32 value);
    void (*setInt64)(struct mag_minidb *db, const char *name, i64 value);
    void (*setUInt32)(struct mag_minidb *db, const char *name, ui32 value);
    void (*setFloat)(struct mag_minidb *db, const char *name, fp32 value);
    void (*setDouble)(struct mag_minidb *db, const char *name, fp64 value);
    void (*setPointer)(struct mag_minidb *db, const char *name, void *value);
    void (*setString)(struct mag_minidb *db, const char *name, const char *s);

    boolean (*findInt32)(struct mag_minidb *db, const char *name, i32 *value);
    boolean (*findInt64)(struct mag_minidb *db, const char *name, i64 *value);
    boolean (*findUInt32)(struct mag_minidb *db, const char *name, _size_t *value);
    boolean (*findFloat)(struct mag_minidb *db, const char *name, fp32 *value);
    boolean (*findDouble)(struct mag_minidb *db, const char *name, fp64 *value);
    boolean (*findPointer)(struct mag_minidb *db, const char *name, void **value);
    boolean (*findString)(struct mag_minidb *db, const char *name, char **s);
}MagMiniDB_t;

typedef MagMiniDB_t* MagMiniDBHandle;

MagMiniDBHandle createMagMiniDB(i32 maxItemsNum);
void destroyMagMiniDB(MagMiniDBHandle db);

#ifdef __cplusplus
}
#endif
#endif