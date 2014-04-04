#include <stdio.h>
#include <string.h>

#include "hashTable.h"
//#include "agilelog.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-Hashtable"


static void StrHashTable_addItem(struct str_hash_table *ht, void *item, const char *str){
    unsigned int tableIndex = 0;

    tableIndex = calcDJBHashValue(str, strlen(str)) % ht->mTableSize;
    LinkedKeyNode_t *node = (LinkedKeyNode_t *)malloc(sizeof(LinkedKeyNode_t));
    if (node != NULL){
        INIT_HLIST_NODE(&node->node);
        node->nodeValue = item;
        strcpy(node->nodeStr, str);
        hlist_add_head(&node->node, &ht->mpTable[tableIndex].head);
    }else{
        printf("StrHashTable_addItem -- node is NULL\n");
    }
    
}

static void *StrHashTable_getItem(struct str_hash_table *ht, const char *str){
    unsigned int tableIndex = 0;
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    
    tableIndex = calcDJBHashValue(str, strlen(str)) % ht->mTableSize;

    if (NULL == ht->mpTable[tableIndex].head.first){
        //printf("the string[%s] has not been added into the Table!!\n", str);
        return NULL;
    }else{
        /*several nodes has the same hash key so traverse the link*/
        hlist_for_each_entry(tnode, pos, &ht->mpTable[tableIndex].head, node){
            if(!strcmp(str, tnode->nodeStr)){
                //printf("get the node (%s)\n", tnode->nodeStr);
                return tnode->nodeValue;
            }
        }
    }
    
    //printf("failed to get the node (%s)\n", str);
    return NULL;
}

#define AGILELOG_PRINT_BUFFER_SIZE 2048
static void StrHashTable_print(struct str_hash_table *ht){

    int i;
    char buf[AGILELOG_PRINT_BUFFER_SIZE];
    char tmpBuf[256];
    
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    
    printf("------Hash Table------\n");
    
    for(i = 0; i < ht->mTableSize; i++){
        memset(buf, 0, AGILELOG_PRINT_BUFFER_SIZE);
        if (NULL == ht->mpTable[i].head.first){
            printf("%d - [NODE: NULL]\n", i);
        }else{
            sprintf(buf, "%d - ", i);
            hlist_for_each_entry(tnode, pos, &ht->mpTable[i].head, node){
                sprintf(tmpBuf, "[NODE: %s] ->", tnode->nodeStr);
                strcat(buf, tmpBuf);
            }
            printf("%s END\n", buf);
        }
    }

    printf("----- Table END  ------\n");
}

HashTableHandle  createMagStrHashTable(int num){
    HashTableHandle ht;

    ht = (HashTableHandle)malloc(sizeof(StrHashTable_t));
    if (NULL != ht){
        ht->mpTable = (HashTable_t *)malloc(sizeof(HashTable_t) * num);
        if (NULL != ht->mpTable){
            memset(ht->mpTable, 0, sizeof(HashTable_t) * num);
            ht->mTableSize = num;
            ht->addItem = StrHashTable_addItem;
            ht->getItem = StrHashTable_getItem;
            ht->print   = StrHashTable_print;
        }
    }
    return ht;
}

void             destroyMagStrHashTable(HashTableHandle ht){
    int i;
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    struct hlist_node *prev;

    if (NULL == ht)
        return;
    
    for(i = 0; i < ht->mTableSize; i++){
        while(NULL != ht->mpTable[i].head.first){
            hlist_for_each_entry_to_tail(tnode, pos, prev, &ht->mpTable[i].head, node){
            }
            printf("remove node (%s)\n", tnode->nodeStr);
            hlist_del(prev);
            free((void *)tnode);
        }
    }
    free((void *)ht->mpTable);
    free(ht);
}

