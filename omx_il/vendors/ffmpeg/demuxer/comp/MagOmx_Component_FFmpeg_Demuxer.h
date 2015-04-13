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

#ifndef __MAGOMX_COMPONENT_FFMPEG_DEMUXER_H__
#define __MAGOMX_COMPONENT_FFMPEG_DEMUXER_H__

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavformat/internal.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"

#include "MagOMX_Component_demuxer.h"

#define kAVIOBufferSize  (32 * 1024)

typedef struct MAG_FFMPEG_DEMUXER_PLAY_STATE {
    OMX_S32 abort_request;
}MAG_FFMPEG_DEMUXER_PLAY_STATE;

DeclareClass(MagOmxComponent_FFmpeg_Demuxer, MagOmxComponentDemuxer);

Virtuals(MagOmxComponent_FFmpeg_Demuxer, MagOmxComponentDemuxer) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Demuxer, MagOmxComponentDemuxer, \
	int  (*demux_interrupt_cb)(void *ctx); \
    void (*fillAVFrame)(MagOmxComponent_FFmpeg_Demuxer thiz, AVPacket *pPacket, OMX_DEMUXER_AVFRAME *avFrame, AVRational timeBase); \
    void (*releaseAVFrame)(OMX_HANDLETYPE hComponent, OMX_PTR frame); \
)
    MAG_FFMPEG_DEMUXER_PLAY_STATE mPlayState;

    OMX_U32 mStreamIndex;
    OMX_DEMUXER_STREAM_INFO mStreamInfo[MAX_STREAMS_NUMBER];

    MagMutexHandle   mhFFMpegMutex;
    
EndOfClassMembers;

#endif