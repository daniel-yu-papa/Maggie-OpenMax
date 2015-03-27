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

#ifndef __MAGPIPELINE_COMMON_H__
#define __MAGPIPELINE_COMMON_H__

#include "framework/MagFramework.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    MAGPIPELINE_DECODE_AUDIO,
    MAGPIPELINE_DECODE_VIDEO,
    MAGPIPELINE_DECODE_SUBTITLE,
    MAGPIPELINE_ENCODE_AUDIO,
    MAGPIPELINE_ENCODE_VIDEO,
    MAGPIPELINE_ENCODE_SUBTITLE,
    MAGPIPELINE_CLOCK
}MAGPIPELINE_TYPE_T;

typedef OMX_ERRORTYPE (*MagPipeline_Init)();

typedef struct
{
    i32                major;
    i32                minor;
    char               *name;      
    char               *desc; 
    MAGPIPELINE_TYPE_T type;     
    MagPipeline_Init   init; 
} MagPipeline_Registration_t;

typedef MagPipeline_Registration_t *(*pipeline_reg_func_t) ();
typedef void (*pipeline_dereg_func_t) (void *handler);

typedef struct {
    List_t                     node;
    void                       *hLib;
    void                       *hPipeline;
    MagPipeline_Registration_t *regInfo;
    pipeline_dereg_func_t      deregFunc;
    boolean                    initialized;
}MagPipeline_Entry_t;

#ifdef __cplusplus
}
#endif
#endif


