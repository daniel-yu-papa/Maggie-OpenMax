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

#ifndef __MAG_MEM_H__
#define __MAG_MEM_H__

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#    define MAG_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > x || __GNUC__ == x && __GNUC_MINOR__ >= y)
#else
#    define MAG_GCC_VERSION_AT_LEAST(x,y) 0
#endif


#if MAG_GCC_VERSION_AT_LEAST(3,1)
    #define mag_malloc_attrib __attribute__((__malloc__))
#else
    #define mag_malloc_attrib
#endif

#if MAG_GCC_VERSION_AT_LEAST(4,3)
    #define mag_alloc_size(...) __attribute__((alloc_size(__VA_ARGS__)))
#else
    #define mag_alloc_size(...)
#endif

void *mag_malloc(size_t size) mag_malloc_attrib mag_alloc_size(1);

void *mag_mallocz(size_t size) mag_malloc_attrib mag_alloc_size(1);

void *mag_realloc(void *ptr, size_t size) mag_alloc_size(2);

void mag_free(void *ptr);

void mag_freep(void **arg);

char *mag_strdup(const char *s) mag_malloc_attrib;

void *mag_memdup(const void *p, size_t size);

#ifdef __cplusplus
}
#endif


#endif
