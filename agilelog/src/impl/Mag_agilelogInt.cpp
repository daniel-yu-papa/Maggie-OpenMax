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

#include "Mag_agilelogImpl.h"
#include <stdarg.h>
#include <stdio.h>

using namespace MAGAGILELOG;

#define LOG_BUF_SIZE 1024

static MagAgileLog *gpAgile = NULL;

#ifdef __cplusplus
extern "C" {
#endif

void Mag_agilelogPrint(int prio, const char *module, const char *caller, int line, const char *fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    if (gpAgile)
        gpAgile->printLog(prio, module, caller, line, buf);
}

void Mag_agilelogCreate(){
    gpAgile = MagAgileLog::getInstance();
}

void Mag_agilelogDestroy(){
	MagAgileLog::destroy();
    gpAgile = NULL;
}

#ifdef __cplusplus
}
#endif


