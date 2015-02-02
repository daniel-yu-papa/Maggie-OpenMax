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

#ifndef __MAG_OMX_IL_CORE_H__
#define __MAG_OMX_IL_CORE_H__

#include "framework/MagFramework.h"
#include "MagOMX_IL.h"

typedef MagOMX_Component_Registration_t *(*comp_reg_func_t) ();
typedef void (*comp_dereg_func_t) (OMX_HANDLETYPE hComponent);

typedef struct {
    List_t            node;
    OMX_PTR           libHandle;
    OMX_HANDLETYPE    compHandle;
    MagOMX_Component_Registration_t *regInfo;
    comp_dereg_func_t deregFunc;
    OMX_BOOL          initialized;
}Component_Entry_t;

typedef struct{
    List_t  LoadedCompListHead;
    OMX_U32 LoadedCompNumber;

    HashTableHandle roleToComponentTable;
    HashTableHandle componentToRoleTable;

    MagMutexHandle  lock;
}MagOMX_IL_Core_t;

#endif