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

#ifndef __MAGPIPELINE_CODEC_H__
#define __MAGPIPELINE_CODEC_H__

#include "framework/MagFramework.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MagPipeline_Codec{
    List_t mPipelineList;
    i32    mPipelineNum;

    i32 (*init)(struct MagPipeline_Codec *hdpl);
    i32 (*uninit)(struct MagPipeline_Codec *hdpl);
    MagPipeline_Base (*getHandle)(struct MagPipeline_Codec *hdpl, MAGPIPELINE_TYPE_T type);
}MagPipeline_Codec_t;

typedef MagPipeline_Codec_t* MagCodecPipelineHandle;

MagPipelineCodecHandle createMagCodecPipeline(void);
void destroyMagCodecPipeline(MagPipelineCodecHandle *pdpl);

#ifdef __cplusplus
}
#endif
#endif