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

#ifndef __MAGOMX_COMPONENT_FFMPEG_AREN_H__
#define __MAGOMX_COMPONENT_FFMPEG_AREN_H__

#include "MagOMX_Component_audio.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"

/*#define CAPTURE_PCM_DATA_TO_FILE*/
/*#define CAPTURE_PCM_DATA_TO_SDL*/

typedef struct{
    OMX_BUFFERHEADERTYPE *pOmxBufHeader;
    ui8 *buf;
    OMX_U32 nFilledLen;
    OMX_U32 nOffset;
}MagOmx_Aren_Buffer_t;

typedef struct{
    List_t Node;    
    MagOmx_Aren_Buffer_t *pBuf;
}MagOmx_Aren_BufferNode_t;

typedef struct AudioParams {
    int freq;
    int channels;
    i64 channel_layout;
    enum AVSampleFormat fmt;
} AudioParams;

DeclareClass(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio);

Virtuals(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio, \
    int (*openAudio)(MagOmxComponent_FFmpeg_Aren thiz);      \
	MagOmx_Aren_BufferNode_t *(*getBuffer)(MagOmxComponent_FFmpeg_Aren thiz);                 \
    void (*putBuffer)(MagOmxComponent_FFmpeg_Aren thiz, OMX_BUFFERHEADERTYPE *pOmxBufHeader, ui8 *buf);     \
    void (*resetBuf)(MagOmxComponent_FFmpeg_Aren thiz); \
)
    AVStream *mpAudioStream;
    struct SwrContext *mpSwrCtx;
    AudioParams mAudioParameters;

    MagMutexHandle mListMutex;
    List_t mBufFreeListHead;
    List_t mBufBusyListHead;

    MagEventHandle         mNewBufEvent;
    MagEventGroupHandle    mWaitBufEventGroup;

    OMX_BOOL mStopped;

#ifdef CAPTURE_PCM_DATA_TO_FILE
	FILE *mfPCMFile;
#endif

#ifdef CAPTURE_PCM_DATA_TO_SDL
    FILE *mfPCMSdl;
#endif
EndOfClassMembers;

#endif