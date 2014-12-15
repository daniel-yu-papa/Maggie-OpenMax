/*Yu Jun: Created in 2013-09-25*/

#include "Mag_rbtree.h"
#include "Mag_mem.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_RBTree"


static int gRepeatKeyNum = 0;

static RBTreeNodeHandle rbtree_get_priv(RBTreeNodeHandle root, i64 key){
    RBTreeNodeHandle x = root;

    while(NULL != x){
        if (x->key == key) 
            return x;
        else if (x->key < key)
            x = x->right;
        else
            x = x->left;
    }
    return NULL;
}

void *rbtree_get(RBTreeNodeHandle root, i64 key){
    RBTreeNodeHandle x = root;

    while(NULL != x){
        if (x->key == key) 
            return x->value;
        else if (x->key < key)
            x = x->right;
        else
            x = x->left;
    }
    return NULL;
}

static RBTreeNodeHandle rbtree_insert_priv(RBTreeNodeHandle root, i64 key, void *value){
    /*insert the node at the bottom*/
    if (NULL == root){
        RBTreeNodeHandle node = (RBTreeNodeHandle)mag_malloc(sizeof(RBTREE_NODE_t));
        if (NULL != node){
            node->color = RBTREE_TRUE;
            node->key   = key;
            node->value = value;
            node->left  = NULL;
            node->right = NULL;
            node->parent = NULL;
        }
        return node;
    }

    /*split 4-node on the way down*/
    if (isRed(root->left) && isRed(root->right))
        colorFlip(root);
    
    /*standard binary tree insert*/
    if (root->key == key){
        root->value = value;
        ++gRepeatKeyNum;
    }else if (root->key < key){
        root->right = rbtree_insert_priv(root->right, key, value);
        root->right->parent = root;
    }else{
        root->left = rbtree_insert_priv(root->left, key, value);
        root->left->parent = root;
    }
    
    /*fix right-leaning red node:  this will assure that a 3-node is the left child*/
    if (isRed(root->right) && !isRed(root->left))
        root = rotateLeft(root);

    /*fix two reds in a row: this will rebalance a 4-node.*/
    if (isRed(root->left) && isRed(root->left->left))
        root = rotateRight(root);
    
    return root;
}

RBTreeNodeHandle rbtree_insert(RBTreeNodeHandle root, i64 key, void *value){
    root = rbtree_insert_priv(root, key, value);
    root->color = RBTREE_FALSE;
    return root;
}

static RBTreeNodeHandle rbtree_delete_priv(RBTreeNodeHandle root, i64 key, RBTreeNodeHandle *deleteNode){
    if (NULL == root)
        return NULL;
    
    if (key < root->key){
        if (NULL == root->left)
            return NULL;
        
        /*delete minimal node*/
        if (!isRed(root->left) && !isRed(root->left->left))
            root = moveRedLeft(root);
        
        root->left = rbtree_delete_priv(root->left, key, deleteNode);
    }else{
        if (isRed(root->left))
            root = rotateRight(root);

        /*delete leaf node*/
        if ((root->right == NULL) && (root->key == key)){
            /*AGILE_LOGD("find the key: %lld", key);*/
            *deleteNode = root;
            return NULL;
        }

        if (NULL != root->right){
            if (!isRed(root->right) && !isRed(root->right->left))
                root = moveRedRight(root);
            
            if (root->key == key){
                RBTreeNodeHandle min_node = NULL;
                min_node = min(root->right);
                AGILE_LOGD("the min key: %lld", min_node->key);
                root->key = min_node->key;
                root->value = min_node->value;
                root->right = deleteMin(root->right, deleteNode);
                /**deleteNode = min_node;*/
            }else{
                root->right = rbtree_delete_priv(root->right, key, deleteNode);
            }
        }
    }

    return fixup(root);
}

i32 rbtree_delete(RBTreeNodeHandle *root, i64 key){
    RBTreeNodeHandle deleteNode;
    if (rbtree_get_priv(*root, key) != NULL){
        *root = rbtree_delete_priv(*root, key, &deleteNode);
        if (NULL != *root)
            (*root)->color = RBTREE_FALSE;
        
        if (NULL != deleteNode){
            mag_free(deleteNode);
        }
        return 0;
    }else{
        return -1;
    }
}

/*pre-order traverse the binary tree*/
static void rbtree_dump_priv(RBTreeNodeHandle root, i32 print_flag, i32 *num){
    if (NULL == root)
        return;

    rbtree_dump_priv(root->left, print_flag, num);
    *num = *num + 1;
    if (print_flag){
        AGILE_LOGI("node %d: [parent(key: %lld, color: %s) key: %lld - color: %s]", *num, root->parent ? root->parent->key : -1,
                    root->parent ? (isRed(root->parent) ? "RED" : "BLACK") : "NULL", root->key, (isRed(root) ? "RED" : "BLACK"));
    }
    rbtree_dump_priv(root->right, print_flag, num);
}

i32 rbtree_dump(RBTreeNodeHandle root, i32 print_flag){
    i32 node_num = 0;
    rbtree_dump_priv(root, print_flag, &node_num);
    return node_num;
}

i32 rbtree_debug_getRepeatNum(void){
    return gRepeatKeyNum;
}

void rbtree_getMinValue(RBTreeNodeHandle root, i64 *key, void **value){
    RBTreeNodeHandle minNode;
    minNode = min(root);
    *key = minNode->key;
    *value = minNode->value;
}

RBTreeNodeHandle rbtree_first(const RBTreeNodeHandle root){
    RBTreeNodeHandle n;

    n = root;
    if (!n)
        return NULL;
    while (n->left)
        n = n->left;
    return n;
}

RBTreeNodeHandle rbtree_next(RBTreeNodeHandle node){
    RBTreeNodeHandle parent;

    if (node == NULL)
        return NULL;

    /*
     * If we have a right-hand child, go down and then left as far
     * as we can.
     */
    if (node->right) {
        node = node->right;
        while (node->left)
            node = node->left;
        return node;
    }

    /*
     * No right-hand children. Everything down and left is smaller than us,
     * so any 'next' node must be in the general direction of our parent.
     * Go up the tree; any time the ancestor is a right-hand child of its
     * parent, keep going up. First time it's a left-hand child of its
     * parent, said parent is our 'next' node.
     */
    while ((parent = node->parent) && node == parent->right)
        node = parent;

    return parent;
}
