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

#ifndef _MAG_PUB_DEF_H__
#define _MAG_PUB_DEF_H__

#include <errno.h>

#ifdef __GNUC__
#    define MAG_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > x || __GNUC__ == x && __GNUC_MINOR__ >= y)
#else
#    define MAG_GCC_VERSION_AT_LEAST(x,y) 0
#endif

typedef enum{
    MAG_ErrNone = 0,
    MAG_NoMore,
    MAG_Failure,
    MAG_NoMemory,
    MAG_ErrMutexCreate,
    MAG_ErrCondCreate,
    MAG_ErrThreadCreate,
    MAG_ErrGetTime,
    MAG_TimeOut,
    MAG_BadParameter,
    MAG_BadEvtWait,
    MAG_ErrEvtSetOp,
    MAG_AssertFault,
    MAG_ErrMaxEventNum,
    MAG_EventStatusMeet,
    MAG_EventStatusErr,
    MAG_InvalidPointer,
    MAG_InvalidOperation
}MagErr_t;

enum {
    MAG_OK                  = 0,    /*// Everything's swell.*/
    MAG_NO_ERROR            = 0,    /*// No errors.*/

    MAG_UNKNOWN_ERROR       = 0x8000,

    MAG_NO_MEMORY           = -ENOMEM,
    MAG_INVALID_OPERATION   = -ENOSYS,
    MAG_BAD_VALUE           = -EINVAL,
    MAG_BAD_TYPE            = 0x8001,
    MAG_NAME_NOT_FOUND      = -ENOENT,
    MAG_PERMISSION_DENIED   = -EPERM,
    MAG_NO_INIT             = -ENODEV,
    MAG_ALREADY_EXISTS      = -EEXIST,
    MAG_DEAD_OBJECT         = -EPIPE,
    MAG_FAILED_TRANSACTION  = 0x8002,

    MAG_BAD_INDEX           = -EOVERFLOW,
    MAG_NOT_ENOUGH_DATA     = -ENODATA,
    MAG_WOULD_BLOCK         = -EWOULDBLOCK, 
    MAG_TIMED_OUT           = -ETIMEDOUT,
    MAG_UNKNOWN_TRANSACTION = -EBADMSG,
    MAG_STATUS_EXTENSION    = 0x8010
};

#define MAG_TIMEOUT_INFINITE 0xF0000000

#define MAG_MAX_INTEGER      INT_MAX

#endif
