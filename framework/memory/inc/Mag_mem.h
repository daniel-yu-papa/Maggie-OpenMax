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
