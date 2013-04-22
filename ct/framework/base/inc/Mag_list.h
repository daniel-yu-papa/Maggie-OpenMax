#ifndef _MAG_LIST_H__
#define _MAG_LIST_H__

#include <stdlib.h>

typedef struct list_t{
    struct list_t *next;
    struct list_t *prev; 
}List_t;

static inline void INIT_LIST(List_t *head){
    head->prev = head;
    head->next = head;
}

/** add a new entry before the node
 *
 * @param new
 *        new entry to be added
 * @param node
 *        list node to add it before
 */
static inline void list_add_before(List_t *new, List_t *node){
    new->next = node;
    node->prev->next = new;
    new->prev = node->prev;
    node->prev = new;
}

/** add a new entry after the node
 *
 * @param new
 *        new entry to be added
 * @param node
 *        list node to add it after
 */
static inline void list_add(List_t *new, List_t *node){
    new->prev = node;
    node->next->prev = new;
    new->next = node->next;
    node->next = new;
}

static inline void list_del(List_t *node){
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->next = NULL;
    node->prev = NULL;
}


#endif