#ifndef __MAG_RBTREE_H__
#define __MAG_RBTREE_H__

#include <stdlib.h>
#include "agilelog.h"
#include "Mag_pub_type.h"

typedef enum{
    RBTREE_FALSE = 0,
    RBTREE_TRUE  = !RBTREE_FALSE,
}RBTREE_BOOL;

typedef enum{
    NODE_RED = 0,
    NODE_BLACK = 1,
}NODE_COLOR_t;

typedef struct rbtree_node_t{
    i64          key;
    void*        value;
    NODE_COLOR_t color;
    
    struct rbtree_node_t *left;
    struct rbtree_node_t *right;   
}RBTREE_NODE_t;

typedef RBTREE_NODE_t* RBTreeNodeHandle;

static inline RBTREE_BOOL isRed(RBTreeNodeHandle x){
    if (NULL == x) return RBTREE_FALSE;
    if (x->color == NODE_RED) return RBTREE_TRUE;
    return RBTREE_FALSE;
}

static inline void reverseColor(NODE_COLOR_t *color){
    if (*color == NODE_RED)
        *color = NODE_BLACK;
    else
        *color = NODE_RED;
}

static inline RBTreeNodeHandle rotateLeft(RBTreeNodeHandle h){
    RBTreeNodeHandle x = h->right;

    h->right = x->left;
    x->left = h;

    x->color = x->left->color;
    x->left->color = NODE_RED;
    return x;
}

static inline RBTreeNodeHandle rotateRight(RBTreeNodeHandle h){
    RBTreeNodeHandle x = h->left;

    h->left = x->right;
    x->right = h;

    x->color = x->right->color;
    x->right->color = NODE_RED;
    return x;
}

static inline void colorFlip(RBTreeNodeHandle h){   
    if ((NULL == h->left) ||
        (NULL == h->right)){
        AGILE_LOGE("Should not be here!! left(0x%x) right(0x%x)", h->left, h->right);
        return;
    }
    reverseColor(&h->color);
    reverseColor(&h->left->color);
    reverseColor(&h->right->color);
}

/*the basic operations for delete action*/
static inline RBTreeNodeHandle fixup(RBTreeNodeHandle h){
    if (NULL == h)
        return NULL;
    
    /*rotate-left for red-leaning reds*/
    if (isRed(h->right))
        h = rotateLeft(h);

    /*rotate right for red-red pairs*/
    if (isRed(h->left) && (NULL != h->left) && isRed(h->left->left))
        h = rotateRight(h);

    /*split 4-nodes*/
    if (isRed(h->left) && isRed(h->right))
        colorFlip(h);

    return h;
}

static inline RBTreeNodeHandle moveRedRight(RBTreeNodeHandle h){
    colorFlip(h);   
    if ((NULL != h->left) && isRed(h->left->left)){
        h = rotateRight(h);
        colorFlip(h);
    }
    return h;
}

static inline RBTreeNodeHandle moveRedLeft(RBTreeNodeHandle h){
    colorFlip(h);
    if ((NULL != h->right) && isRed(h->right->left)){
        h->right = rotateRight(h->right);
        h = rotateLeft(h);
        colorFlip(h);
    }
    return h;
}

static inline RBTreeNodeHandle min(RBTreeNodeHandle h)
{
    if (h->left == NULL) 
        return h;
    else 
        min(h->left);
}


static inline RBTreeNodeHandle deleteMin(RBTreeNodeHandle h)
{ 
    if ((h == NULL) || (h->left == NULL))
     return NULL;

    if (!isRed(h->left) && !isRed(h->left->left))
     h = moveRedLeft(h);

    h->left = deleteMin(h->left);

    return fixup(h);
}

void *rbtree_get(RBTreeNodeHandle root, i64 key);
void rbtree_getMinValue(RBTreeNodeHandle root, i64 *key, void **value);
RBTreeNodeHandle rbtree_insert(RBTreeNodeHandle root, i64 key, void *value);
int rbtree_delete(RBTreeNodeHandle *root, i64 key);
int rbtree_dump(RBTreeNodeHandle root, i32 print_flag);

int rbtree_debug_getRepeatNum(void);


#endif
