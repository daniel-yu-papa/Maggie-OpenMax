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

#ifndef __MAG_PLAYER_COMMON_H__
#define __MAG_PLAYER_COMMON_H__

#include "MagOMX_IL.h"

#define STRINGIFY(x) case x: return #x

static inline const char *OmxCodec2String(ui32 omxCodec) {
    switch (omxCodec) {
        STRINGIFY(OMX_VIDEO_CodingAVC);
        STRINGIFY(OMX_VIDEO_CodingMPEG4);
        STRINGIFY(OMX_VIDEO_CodingMPEG2);
        STRINGIFY(OMX_AUDIO_CodingMP3);
        STRINGIFY(OMX_AUDIO_CodingMP2);
        STRINGIFY(OMX_AUDIO_CodingAAC);
        STRINGIFY(OMX_AUDIO_CodingAC3);
        STRINGIFY(OMX_AUDIO_CodingDDPlus);
        default: return "codec - unknown";
    }
}

#define ALIGNTO(value, alignment) (((value / alignment) + 1) * alignment)

enum {
    MAG_NO_MORE_DATA = (MAG_STATUS_EXTENSION + 1),
    MAG_PREPARE_FAILURE,
    MAG_READ_ABORT,
    MAG_DEMUXER_ABORT
};

enum {
    PIPELINE_NOTIFY_FillThisBuffer      = 'filb',
    PIPELINE_NOTIFY_PlaybackComplete    = 'plcm'
};

#endif