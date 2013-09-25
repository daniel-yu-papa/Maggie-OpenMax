/*Yu Jun: Created in 2013-09-25*/

#include "Mag_rbtree.h"
#include "Mag_mem.h"

static int gLoopNum = 0;
static int gRepeatKeyNum = 0;

static RBTreeNodeHandle rbtree_get_priv(RBTreeNodeHandle root, int key){
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

void *rbtree_get(RBTreeNodeHandle root, int key){
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

RBTreeNodeHandle rbtree_insert(RBTreeNodeHandle root, int key, void *value){
    /*insert the node at the bottom*/
    if (NULL == root){
        RBTreeNodeHandle node = (RBTreeNodeHandle)mag_malloc(sizeof(RBTREE_NODE_t));
        if (NULL != node){
            node->color = NODE_RED;
            node->key   = key;
            node->value = value;
            node->left  = NULL;
            node->right = NULL;
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
        root->right = rbtree_insert(root->right, key, value);
    }else{
        root->left = rbtree_insert(root->left, key, value);
    }
    
    /*fix right-leaning reds on the way up*/
    if (isRed(root->right))
        root = rotateLeft(root);

    /*fix two reds in a row on the way up*/
    if (isRed(root->left) && isRed(root->left->left))
        root = rotateRight(root);
    
    return root;
}

static RBTreeNodeHandle rbtree_delete_priv(RBTreeNodeHandle root, int key, RBTreeNodeHandle *deleteNode){
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
        if (root->right == NULL){
            if (root->key == key){
                AGILE_LOGD("find the key: %d", key);
                *deleteNode = root;
                return NULL;
            }
        }

        if (!isRed(root->right) && !isRed(root->right->left))
            root = moveRedRight(root);
        
        if (root->key == key){
            RBTreeNodeHandle min_node;
            min_node = min(root->right);
            AGILE_LOGD("the min key: %d", min_node->key);
            root->key = min_node->key;
            root->value = min_node->value;
            root->right = deleteMin(root->right);
            *deleteNode = min_node;
        }else{
            root->right = rbtree_delete_priv(root->right, key, deleteNode);
        }
    }

    return fixup(root);
}

int rbtree_delete(RBTreeNodeHandle *root, int key){
    RBTreeNodeHandle deleteNode;
    if (rbtree_get_priv(*root, key) != NULL){
        *root = rbtree_delete_priv(*root, key, &deleteNode);
        if (NULL != deleteNode){
            mag_free(deleteNode);
        }
        return 0;
    }else{
        return -1;
    }
}

/*in-order traverse the binary tree*/
static void rbtree_dump_priv(RBTreeNodeHandle root, int print_flag){
    if (NULL == root)
        return;

    rbtree_dump_priv(root->left, print_flag);
    if (print_flag)
        AGILE_LOGI("node %d: [key:%d - value:%d - color:%s]", ++gLoopNum, root->key, *((int *)root->value), root->color == NODE_RED ? "RED" : "BLACK");
    else
        ++gLoopNum;
    rbtree_dump_priv(root->right, print_flag);
}

int rbtree_dump(RBTreeNodeHandle root, int print_flag){
    gLoopNum = 0;
    rbtree_dump_priv(root, print_flag);
    return gLoopNum;
}

int rbtree_debug_getRepeatNum(void){
    return gRepeatKeyNum;
}


