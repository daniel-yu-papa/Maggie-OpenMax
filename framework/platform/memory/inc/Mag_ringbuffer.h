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

#ifndef __MAG_RING_BUFFER_H__
#define __MAG_RING_BUFFER_H__

#include <pthread.h>
#include "Mag_pub_type.h"
#include "Mag_pub_def.h"
#include "Mag_list.h"
#include "Mag_agilelog.h"
#include "Mag_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kInvalidSeekOffset       ((int64_t)UINT64_C(0x8FFFFFFFFFFFFFFF))

typedef struct MagRingBuffer{
    i32 rb_size;
    i32 rb_reading_pos;
    i32 rb_writing_pos;
    i32 rb_total_data_size;       /*the size of the total data in rb, including the read and unread data*/
    i32 rb_unread_data_size;      /*the size of the unread data in rb. 0: Empty, rb_size: Full*/
    i64 source_offset;
    pthread_mutex_t  mutex;

    ui8 *pRingBuffer;

    i32 (*read)(struct MagRingBuffer *self, i32 bytes, ui8 *pBuf);
    i32 (*write)(struct MagRingBuffer *self, i32 bytes, ui8 *pBuf);
    i64 (*seek)(struct MagRingBuffer *self, i64 offset);
    void (*flush)(struct MagRingBuffer *self);
    void (*setSourcePos)(struct MagRingBuffer *self, i64 source_offset);
    void (*getSourceRange)(struct MagRingBuffer *self, i64 *start, i64 *end);
}MagRingBuffer_t;

typedef MagRingBuffer_t* MagRingBufferHandle;

MagRingBufferHandle Mag_createRingBuffer(i32 bytes, ui32 flags);
void Mag_destroyRingBuffer(MagRingBufferHandle *phRingBuffer);

#ifdef __cplusplus
}
#endif

#endif