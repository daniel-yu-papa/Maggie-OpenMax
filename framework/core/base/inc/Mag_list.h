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

#ifndef _MAG_LIST_H__
#define _MAG_LIST_H__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif

#define list_offsetof(TYPE, MEMBER) ( (uintptr_t)(&((TYPE *)0)->MEMBER) ) 

#define container_of(ptr, type, member) __extension__({			\
	const __typeof__( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (const char *)__mptr - list_offsetof(type, member) );})

#define list_entry(ptr, type, member) container_of(ptr, type, member)

typedef struct list_t{
    struct list_t *next;
    struct list_t *prev; 
}List_t;

static inline void INIT_LIST(List_t *head){
    head->prev = head;
    head->next = head;
}

/** add a new entry to the list tail
 *
 * @param new
 *        new entry to be added
 * @param head
 *        list head
 */
static inline void list_add_tail(List_t *pnew, List_t *head){
    pnew->next = head;
    head->prev->next = pnew;
    pnew->prev = head->prev;
    head->prev = pnew;
}

/** add a new entry after the node
 *
 * @param new
 *        new entry to be added
 * @param head
 *        list head
 */
static inline void list_add(List_t *pnew, List_t *head){
    pnew->prev = head;
    head->next->prev = pnew;
    pnew->next = head->next;
    head->next = pnew;
}

static inline void list_del(List_t *node){
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->next = node;
    node->prev = node;
}

static inline int is_added_list(List_t *node){
    if ((node->next == node) && (node->prev == node))
        return 0;
    else
        return 1;
}

static inline int is_list_empty(List_t *listHead){
    if ((listHead->next == listHead) && (listHead->prev == listHead))
        return 1;
    else
        return 0;
}

#ifdef __cplusplus
}
#endif

#endif