#ifndef _MAG_LIST_H__
#define _MAG_LIST_H__

#include <stdlib.h>
#include <stdint.h>

#define list_offsetof(TYPE, MEMBER) ( (uintptr_t)(&((TYPE *)0)->MEMBER) ) 

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (const char *)__mptr - list_offsetof(type,member) );})

#define list_entry(ptr, type, member) container_of(ptr,type,member)

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
static inline void list_add_tail(List_t *new, List_t *head){
    new->next = head;
    head->prev->next = new;
    new->prev = head->prev;
    head->prev = new;
}

/** add a new entry after the node
 *
 * @param new
 *        new entry to be added
 * @param head
 *        list head
 */
static inline void list_add(List_t *new, List_t *head){
    new->prev = head;
    head->next->prev = new;
    new->next = head->next;
    head->next = new;
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

#endif