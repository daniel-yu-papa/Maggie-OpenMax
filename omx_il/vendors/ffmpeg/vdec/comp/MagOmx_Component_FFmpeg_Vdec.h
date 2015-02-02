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

#ifndef __MAGOMX_COMPONENT_FFMPEG_VDEC_H__
#define __MAGOMX_COMPONENT_FFMPEG_VDEC_H__

#include "MagOMX_Component_video.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"

// #define CAPTURE_ES_DATA
// #define CAPTURE_DECODED_YUV_DATA

DeclareClass(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo);

Virtuals(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo, \
	void (*self)(void); \
)
	AVCodec *mpVideoCodec;
	AVStream *mpVideoStream;
	AVFormatContext *mpAVFormat;

    OMX_TICKS mPrePTS;

    MagMutexHandle mhFFMpegMutex;

#ifdef CAPTURE_ES_DATA
    FILE *mfEsData;
#endif

#ifdef CAPTURE_DECODED_YUV_DATA
    FILE *mfDecodedYUV;
#endif
EndOfClassMembers;

#endif