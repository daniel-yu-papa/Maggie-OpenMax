
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

#ifndef __MAG_VERSION_H__
#define __MAG_VERSION_H__

#include "framework/Mag_macros.h"
/*z2: 0.4.x
 *a0: 0.5.x
*/
#define LIBMAGPLAYER_VERSION_MAJOR 0
#define LIBMAGPLAYER_VERSION_MINOR 5
#define LIBMAGPLAYER_VERSION_MICRO 10

#define LIBMAGPLAYER_VERSION_INT MAG_VERSION_INT(LIBMAGPLAYER_VERSION_MAJOR, \
                                                 LIBMAGPLAYER_VERSION_MINOR, \
                                                 LIBMAGPLAYER_VERSION_MICRO)
#define LIBMAGPLAYER_VERSION     MAG_VERSION(LIBMAGPLAYER_VERSION_MAJOR,   \
                                            LIBMAGPLAYER_VERSION_MINOR,   \
                                            LIBMAGPLAYER_VERSION_MICRO)
#define LIBMAGPLAYER_BUILD       LIBMAGPLAYER_VERSION_INT

#define LIBMAGPLAYER_IDENT       "Lmagplayer" MAG_STRINGIFY(LIBMAGPLAYER_VERSION)

#endif