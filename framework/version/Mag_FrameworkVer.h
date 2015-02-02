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

#ifndef __MAG_FRAMEWORK_VERSION_H__
#define __MAG_FRAMEWORK_VERSION_H__

#include "Mag_pub_type.h"
#include "Mag_macros.h"
#include "Mag_agilelog.h"

#ifdef __cplusplus
extern "C" {
#endif
#define LIBMAGFW_VERSION_MAJOR 0
#define LIBMAGFW_VERSION_MINOR 2
#define LIBMAGFW_VERSION_MICRO 5

#define LIBMAGFW_VERSION_INT MAG_VERSION_INT(LIBMAGFW_VERSION_MAJOR, \
                                                 LIBMAGFW_VERSION_MINOR, \
                                                 LIBMAGFW_VERSION_MICRO)
#define LIBMAGFW_VERSION     MAG_VERSION(LIBMAGFW_VERSION_MAJOR,   \
                                            LIBMAGFW_VERSION_MINOR,   \
                                            LIBMAGFW_VERSION_MICRO)
#define LIBMAGFW_BUILD       LIBMAGFW_VERSION_INT

#define LIBMAGFW_IDENT       "Lmagfw" MAG_STRINGIFY(LIBMAGFW_VERSION)

ui32 Mag_getFrameWorkVer(void);

#ifdef __cplusplus
}
#endif

#endif