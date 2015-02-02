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

#ifndef __MRVL_AMP_ES_FORMAT_H__
#define __MRVL_AMP_ES_FORMAT_H__

#include "framework/MagFramework.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavformat/internal.h"
}

class MagESFormat{
public:
	MagESFormat(){};
	virtual ~MagESFormat(){};

	virtual ui32 formatES(ui8 *pHeader, ui8 *esPacket, ui32 esLength) = 0;
	virtual ui32 addESHeader(ui8 *pHeader) = 0;
	virtual void reset() = 0;
};

static inline ui16 ReadU16At(const ui8 *ptr){
	return ptr[0] << 8 | ptr[1];
}

static inline ui32 ReadU24At(const ui8 *ptr){
	return ptr[0] << 16 | ptr[1] << 8 | ptr[2];
}

static inline ui32 ReadU32At(const ui8 *ptr){
	return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static inline void WriteU16At(ui8 *ptr, ui16 v){
	ptr[0] = (ui8)((v >> 8) & 0xFF);
	ptr[1] = (ui8)((v) & 0xFF);
}

static inline void WriteU24At(ui8 *ptr, ui32 v){
	ptr[0] = (ui8)((v >> 16) & 0xFF);
	ptr[1] = (ui8)((v >> 8)  & 0xFF);
	ptr[2] = (ui8)((v)       & 0xFF);
}

static inline void WriteU32At(ui8 *ptr, ui32 v){
	ptr[0] = (ui8)((v >> 24) & 0xFF);
	ptr[1] = (ui8)((v >> 16) & 0xFF);
	ptr[2] = (ui8)((v >> 8)  & 0xFF);
	ptr[3] = (ui8)((v)       & 0xFF);
}

#endif