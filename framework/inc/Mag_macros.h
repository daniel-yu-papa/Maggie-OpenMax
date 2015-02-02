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

#ifndef __MAG_MACROS_H__
#define __MAG_MACROS_H__

#define MAG_TOSTRING(s)      #s
#define MAG_STRINGIFY(s)     MAG_TOSTRING(s)

#define MAG_VERSION_INT(a, b, c) (a<<16 | b<<8 | c)
#define MAG_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define MAG_VERSION(a, b, c) MAG_VERSION_DOT(a, b, c)


#endif