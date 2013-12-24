#ifndef _HASHTABLE_C_H__
#define _HASHTABLE_C_H__

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void* StrHashTable_Create(int maxNum);

EXTERNC void StrHashTable_Destroy(void *h);

EXTERNC void StrHashTable_AddItem(void *obj, const char *str, void *item);

EXTERNC void *StrHashTable_GetItem(void *obj, const char *str);

#undef EXTERNC

#endif

