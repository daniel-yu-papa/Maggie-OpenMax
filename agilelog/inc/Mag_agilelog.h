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

#ifndef __MAG_AGILE_LOG_H__
#define __MAG_AGILE_LOG_H__

#include "Mag_agilelogInt.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#ifndef MODULE_TAG
#define MODULE_TAG "NULL"
#endif

#ifdef __cplusplus
#if 0
    inline char *CPPFuncName(const char *prettyFunction, const char *func){
        char *str = strdup(prettyFunction);
        char *token = NULL; 
        char *result = NULL;

        result = strrchr(str, ':');
        if (result == NULL)
            return const_cast<char*>(func);
        
        token = strtok(str, ":");
        result = strrchr(token, ' ');
        
        if (NULL == result)
            result = token;
        
        sprintf(result, "%s::%s", result, func);
        return result;
    } 
#endif
    #undef __FUNCTION_NAME__
    #define __FUNCTION_NAME__  __PRETTY_FUNCTION__/*CPPFuncName(__PRETTY_FUNCTION__, __FUNCTION__)*/
#else
#if 0
    inline char *CFuncName(const char *func){
        char *str = strdup(func);
        return str;
    }
#endif
    #undef __FUNCTION_NAME__
    #define __FUNCTION_NAME__  __FUNCTION__
#endif

#ifndef AGILE_LOGV
#define AGILE_LOGV(...) (AGILE_LOG(LOG_VERBOSE, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGD
#define AGILE_LOGD(...) (AGILE_LOG(LOG_DEBUG, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGI
#define AGILE_LOGI(...) (AGILE_LOG(LOG_INFO, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGW
#define AGILE_LOGW(...) (AGILE_LOG(LOG_WARN, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOGE
#define AGILE_LOGE(...) (AGILE_LOG(LOG_ERROR, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOG_FATAL
#define AGILE_LOG_FATAL(...) (AGILE_LOG(LOG_FATAL, MODULE_TAG, __VA_ARGS__))
#endif

#ifndef AGILE_LOG
#define AGILE_LOG(priority, module, ...) \
    Mag_agilelogPrint(AGILE_##priority, module, __FUNCTION_NAME__, __LINE__, __VA_ARGS__)
#endif

#ifndef AGILE_LOG_DESTROY
#define AGILE_LOG_DESTROY() \
    Mag_agilelogDestroy()
#endif

#ifndef AGILE_LOG_CREATE
#define AGILE_LOG_CREATE() \
    Mag_agilelogCreate()
#endif

#if 0
#define MODULE_TAG_D(tag) \
    "#ifdef MODULE_TAG"  \
    "#undef MODULE_TAG"  \
    "#endif"             \
    "#define MODULE_TAG ##tag"
#endif

#endif