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

#ifndef __MAG_OMX_BUFFER_H__
#define __MAG_OMX_BUFFER_H__

#include "framework/MagFramework.h"

typedef   enum {
  STREAM_FRAME_FLAG_NONE        = 0,
  STREAM_FRAME_FLAG_KEY_FRAME   = (1 << 0),
  STREAM_FRAME_FLAG_EOS         = (1 << 1),
} StreamFrameFlag_t;

typedef struct mag_omx_stream_frame_t{
  List_t            node;

  OMX_U32           size;
  OMX_PTR           *buffer;
  OMX_TICKS         pts;          /*in 90K*/
  OMX_TICKS         dts;          /*in 90K*/  

  OMX_S32           duration;     /*Duration of this packet in ns*/
  OMX_S64           position;     /*byte position in stream*/
  StreamFrameFlag_t flag;
}MagOmxStreamFrame_t;

#endif
