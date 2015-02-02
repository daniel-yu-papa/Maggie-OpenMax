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

#ifndef __MAG_INVOKE_DEF_H__
#define __MAG_INVOKE_DEF_H__

typedef struct{
	int audio_buffer_time;
	int video_buffer_time;
	int loadingSpeed;
}BufferStatistic_t;

typedef struct{
	unsigned int width;
	unsigned int height;
	double       fps;
	unsigned int bps;     /*in kb/s*/
    char codec[128];
}VideoMetaData_t;

typedef struct{
    unsigned int hz;
    unsigned int bps;
    char codec[32];
}AudioMetaData_t;

typedef enum invoke_id_t{
    INVOKE_ID_GET_BUFFER_STATUS = 0x1000,
    INVOKE_ID_GET_VIDEO_META_DATA,
    INVOKE_ID_GET_AUDIO_META_DATA,
    INVOKE_ID_GET_DECODED_VIDEO_FRAME,
    INVOKE_ID_PUT_USED_VIDEO_FRAME,
    INVOKE_ID_GET_DECODED_AUDIO_FRAME,
    INVOKE_ID_PUT_USED_AUDIO_FRAME,
}INVOKE_ID_t;

#endif