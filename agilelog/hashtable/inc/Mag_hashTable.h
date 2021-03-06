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

#ifndef _HASHTABLE_H__
#define _HASHTABLE_H__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*BEGIN: The Macros for hlist operations*/
/*uintptr_t is an unsigned int that is guaranteed to be the same size as a pointer. in <stdint.h>*/
#define ht_offsetof(TYPE, MEMBER) ( (uintptr_t) (&((TYPE *)0)->MEMBER) ) 

#define hlist_container_of(ptr, type, member)  \
	__extension__({			\
	const __typeof__( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (const char *)__mptr - ht_offsetof(type,member) );})

#define hlist_entry(ptr, type, member) hlist_container_of(ptr, type, member)

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(tpos, pos, head, member)   \
	for (pos = (head)->first;					 \
	     pos &&							 \
		 __extension__({ tpos = hlist_entry(pos, __typeof__(*tpos), member); 1;}); \
	     pos = pos->next) 

#define hlist_for_each_entry_to_tail(tpos, pos, prev, head, member)    \
	for (pos = (head)->first;					 \
	     pos &&							 \
	    __extension__({ prev = pos; 1;}) &&          \
		__extension__({ tpos = hlist_entry(pos, __typeof__(*tpos), member); 1;}); \
	     pos = pos->next)
	     
/*END: The Macros for hlist operations*/
    
struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

typedef struct hash_table{
    struct hlist_head head;
}HashTable_t;

typedef struct linked_key_node{
    struct hlist_node node;
    void *nodeValue;
    char nodeStr[64]; /*the length of the key should be less than 64*/
}LinkedKeyNode_t;


/*handle the hlist.*/
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h) 
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

static inline void hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
	n->next  = NULL;
	n->pprev = NULL;
}

static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline unsigned int calcDJBHashValue(const char* str, unsigned int len)
{
    unsigned int hash = 5381;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {   
       hash = ((hash << 5) + hash) + (*str);
    }   

    return hash;
}

typedef struct str_hash_table{
    HashTable_t *mpTable;
    int mTableSize;

    void  (*addItem)(struct str_hash_table *ht, void *item, const char *str);
    void* (*getItem)(struct str_hash_table *ht, const char *str);
    void  (*delItem)(struct str_hash_table *ht, const char *str);
    void  (*print)(struct str_hash_table *ht);
}StrHashTable_t;

typedef StrHashTable_t* HashTableHandle;

HashTableHandle  createMagStrHashTable(int num);
void             destroyMagStrHashTable(HashTableHandle *pHt);

#ifdef __cplusplus
}
#endif

#endif
