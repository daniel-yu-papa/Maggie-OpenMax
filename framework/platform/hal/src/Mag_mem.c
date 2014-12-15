#include <stdlib.h>
#include <string.h>
#include "Mag_mem.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Memory"

static size_t max_alloc_size= INT_MAX;

void *mag_malloc(size_t size){
    void *ptr = NULL;
    if (size > 0)
        ptr = malloc(size);
    return ptr;
}

void *mag_mallocz(size_t size){
    void *ptr = mag_malloc(size);

    if (ptr)
        memset(ptr, 0, size);

    return ptr;
}

void *mag_realloc(void *ptr, size_t size){
    /* let's disallow possible ambiguous cases */
    if (size > (max_alloc_size - 32))
        return NULL;

    return realloc(ptr, size + !size);
}

void mag_free(void *ptr){
    if(ptr)
        free(ptr);
}

void mag_freep(void **arg){
    mag_free(*arg);
    *arg = NULL;
}

char *mag_strdup(const char *s){
    char *ptr = NULL;
    if (s) {
        int len = strlen(s) + 1;
        ptr = mag_malloc(len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

void *mag_memdup(const void *p, size_t size){
    void *ptr = NULL;
    if (p) {
        ptr = mag_malloc(size);
        if (ptr)
            memcpy(ptr, p, size);
    }
    return ptr;
}


