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

#ifndef __MAGOMX_COMPONENT_FFMPEG_DATA_SOURCE_H__
#define __MAGOMX_COMPONENT_FFMPEG_DATA_SOURCE_H__

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavformat/internal.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"

#include "MagOMX_Component_DataSource.h"

DeclareClass(MagOmxComponent_FFmpeg_DataSource, MagOmxComponentDataSource);

Virtuals(MagOmxComponent_FFmpeg_DataSource, MagOmxComponentDataSource) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_DataSource, MagOmxComponentDataSource, \
	int  (*demux_interrupt_cb)(void *ctx); \
    void (*fillAVFrame)(MagOmxComponent_FFmpeg_DataSource thiz, AVPacket *pPacket, MAG_DEMUXER_AVFRAME *avFrame, OMX_IN MAG_DEMUXER_DATA_SOURCE *pDataSource); \
    void (*releaseAVFrame)(OMX_HANDLETYPE hComponent, OMX_PTR frame); \
)
    AVIOContext *mAVIO;
    
EndOfClassMembers;

#endif