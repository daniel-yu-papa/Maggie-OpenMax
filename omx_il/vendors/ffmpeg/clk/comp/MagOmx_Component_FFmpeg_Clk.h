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

#ifndef __MAGOMX_COMPONENT_FFMPEG_CLOCK_H__
#define __MAGOMX_COMPONENT_FFMPEG_CLOCK_H__

#include "MagOMX_Component_clock.h"

DeclareClass(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock);

Virtuals(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock, \
	void (*self)(void); \
)
	OMX_U32 mPortIndex;
	
EndOfClassMembers;

#endif