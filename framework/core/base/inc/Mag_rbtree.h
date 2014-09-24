#ifndef __MAG_RBTREE_H__
#define __MAG_RBTREE_H__

/*
* https://www.cs.princeton.edu/~rs/talks/LLRB/LLRB.pdf
* LLRB properties:
* 1) No path from the root to the bottom contains two consecutive red links.
* 2) The number of black links on every such path is the same.
*/

#include <stdlib.h>
#include "Mag_agilelog.h"
#include "Mag_pub_type.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    RBTREE_FALSE = 0,
    RBTREE_TRUE  = !RBTREE_FALSE
}RBTREE_BOOL;


typedef struct rbtree_node_t{
    i64          key;
    void*        value;
    /*the color of the node represents the color of the upper link, which points to it's parent.
       * the root node also has the upper link,which points to nothing.
       */
    RBTREE_BOOL  color;
    
    struct rbtree_node_t *parent;

    struct rbtree_node_t *left;
    struct rbtree_node_t *right;   
}RBTREE_NODE_t;

typedef RBTREE_NODE_t* RBTreeNodeHandle;

static inline RBTREE_BOOL isRed(RBTreeNodeHandle x){
    if (NULL != x)
        return x->color;
    else
        return RBTREE_FALSE;
}

static inline RBTreeNodeHandle rotateLeft(RBTreeNodeHandle h){
    RBTreeNodeHandle x = h->right;

    h->right = x->left;
    x->left = h;
    x->color = h->color;
    h->color = RBTREE_TRUE;

    x->parent = h->parent;
    h->parent = x;
    if (h->right)
        h->right->parent = h;

    return x;
}

static inline RBTreeNodeHandle rotateRight(RBTreeNodeHandle h){
    RBTreeNodeHandle x = h->left;

    h->left = x->right;
    x->right = h;
    x->color = h->color;
    h->color = RBTREE_TRUE;

    x->parent = h->parent;
    h->parent = x;
    if (h->left)
        h->left->parent = h;

    return x;
}

static inline void colorFlip(RBTreeNodeHandle h){  
    h->color = (RBTREE_BOOL)!h->color;
    
    if (NULL != h->left)
        h->left->color = (RBTREE_BOOL)!h->left->color;
    
    if (NULL != h->right)
        h->right->color = (RBTREE_BOOL)!h->right->color;
}

/*the basic operations for delete action*/
static inline RBTreeNodeHandle fixup(RBTreeNodeHandle h){
    if (NULL == h)
        return NULL;
    
    /*rotate-left for red-leaning reds*/
    if (isRed(h->right))
        h = rotateLeft(h);

    /*rotate right for red-red pairs*/
    if (isRed(h->left) && isRed(h->left->left))
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
    while (h->left != NULL)
        h = h->left;

    return h;
}


static inline RBTreeNodeHandle deleteMin(RBTreeNodeHandle h, RBTreeNodeHandle *deleteNode)
{ 
    if (h->left == NULL){
        *deleteNode = h;
        return NULL;
    }
    if (!isRed(h->left) && !isRed(h->left->left))
        h = moveRedLeft(h);

    h->left = deleteMin(h->left, deleteNode);

    return fixup(h);
}

void *rbtree_get(RBTreeNodeHandle root, i64 key);
void rbtree_getMinValue(RBTreeNodeHandle root, i64 *key, void **value);
RBTreeNodeHandle rbtree_insert(RBTreeNodeHandle root, i64 key, void *value);
i32 rbtree_delete(RBTreeNodeHandle *root, i64 key);
i32 rbtree_dump(RBTreeNodeHandle root, i32 print_flag);
RBTreeNodeHandle rbtree_first(const RBTreeNodeHandle root);
RBTreeNodeHandle rbtree_next(RBTreeNodeHandle node);
i32 rbtree_debug_getRepeatNum(void);

#ifdef __cplusplus
}
#endif

#endif
