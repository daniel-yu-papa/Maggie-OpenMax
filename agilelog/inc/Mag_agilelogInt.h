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

#ifndef __MAG_AGILE_LOG_INTERFACE_H__
#define __MAG_AGILE_LOG_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum agilelog_LogPriority {
    AGILE_LOG_UNKNOWN = 0,
    AGILE_LOG_DEFAULT = 1,
    AGILE_LOG_VERBOSE = 2,
    AGILE_LOG_DEBUG   = 3,
    AGILE_LOG_INFO    = 4,
    AGILE_LOG_WARN    = 5,
    AGILE_LOG_ERROR   = 6,
    AGILE_LOG_FATAL   = 7
}agilelog_LogPriority;

void Mag_agilelogPrint(int prio, const char *module, const char *fmt, ...);
void Mag_agilelogDestroy();
void Mag_agilelogCreate();

#ifdef __cplusplus
}
#endif

#endif