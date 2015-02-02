/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __MAG_MINIDB_H__
#define __MAG_MINIDB_H__

#include "Mag_list.h"
#include "Mag_pub_def.h"  
#include "Mag_pub_type.h"
#include "Mag_hashTable.h"
#include "Mag_hal.h"

/*
* the pointers that setPointer() and setString() use must be malloced, they would be freed while the mini db is destroyed. 
*/

#ifdef __cplusplus
extern "C" {
#endif

enum MagMiniDBItemType {
    MagMiniDB_TypeInt32,
    MagMiniDB_TypeInt64,
    MagMiniDB_TypeUInt32,
    MagMiniDB_TypeFloat,
    MagMiniDB_TypeDouble,
    MagMiniDB_TypePointer,
    MagMiniDB_TypeString
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
    char *mName;
    enum MagMiniDBItemType mType;
}MagMiniDBItem_t;

typedef MagMiniDBItem_t* MagMiniDBItemHandle;

typedef struct mag_minidb{
    HashTableHandle mhHashTable;
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
    boolean (*findUInt32)(struct mag_minidb *db, const char *name, ui32 *value);
    boolean (*findFloat)(struct mag_minidb *db, const char *name, fp32 *value);
    boolean (*findDouble)(struct mag_minidb *db, const char *name, fp64 *value);
    boolean (*findPointer)(struct mag_minidb *db, const char *name, void **value);
    boolean (*findString)(struct mag_minidb *db, const char *name, char **s);

    void    (*derefItem)(struct mag_minidb *db, const char *name);
    void    (*deleteItem)(struct mag_minidb *db, const char *name);
}MagMiniDB_t;

typedef MagMiniDB_t* MagMiniDBHandle;

MagMiniDBHandle createMagMiniDB(i32 maxItemsNum);
void destroyMagMiniDB(MagMiniDBHandle *pDb);

#ifdef __cplusplus
}
#endif
#endif