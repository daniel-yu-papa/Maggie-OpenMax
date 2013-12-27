#include "Mag_rbtree.h"
#include <stdlib.h>
#include "agilelog.h"

static int deleteNum = 0;
int main()  
{  
    int i, count = 500000;  
    key_t key;  
    RBTreeNodeHandle root = NULL, node = NULL;  
    int test_addr = 0;

    key_t search_key = -1;
    key_t delete_key = -1;
    
    srand(1103515245);  
    for (i = 1; i < count; ++i)  
    {  
        key = rand() % count;  
        test_addr = key;

        root = rbtree_insert(root, key, (void *)&test_addr); 
        
        if (!(i % 188)){
            AGILE_LOGI("[i = %d] set search key %d", i, key);
            search_key = key;
        }

        if (!(i % 373)){
            AGILE_LOGI("[i = %d] set delete key %d", i, key);
            delete_key = key;
        }

        if (search_key > 0){
            if (rbtree_get(root, search_key))  
            {  
                AGILE_LOGI("[i = %d] search key %d success!", i, search_key);  
                search_key = -1;
            }  
            else  
            {  
                AGILE_LOGI("[i = %d] search key %d error!", i, search_key);  
                return (-1);  
            } 
        }
        
        if (delete_key > 0)  
        {  
            AGILE_LOGI("****** Before delete: node number: %d, repeatKeyNum: %d", rbtree_dump(root, 0), rbtree_debug_getRepeatNum());
            if (rbtree_delete(&root, delete_key) == 0)  
            {  
                AGILE_LOGI("[i = %d] delete key %d success (%d)", i, delete_key, ++deleteNum);  
                delete_key = -1;
            }  
            else  
            {  
                AGILE_LOGI("[i = %d] delete key %d error", i, delete_key);  
                return -1;
            }  
            AGILE_LOGI("****** After delete: node number: %d", rbtree_dump(root, 0));
            //return 0;
        }  
    }  
   
    return 0;  
}  

