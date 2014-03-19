#include <stdio.h>
#include <string.h>

#include "hashTable.h"
#include "agilelog.h"
#include <stdio.h>

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
        AGILE_LOGE("the string[%s] has not been added into the Table!!", str);
        return NULL;
    }else{
        /*several nodes has the same hash key so traverse the link*/
        hlist_for_each_entry(tnode, pos, &ht->mpTable[tableIndex].head, node){
            if(!strcmp(str, tnode->nodeStr)){
                AGILE_LOGD("get the node (%s)", tnode->nodeStr);
                return tnode->nodeValue;
            }
        }
    }
    
    AGILE_LOGE("failed to get the node (%s)", str);
    return NULL;
}

#define AGILELOG_PRINT_BUFFER_SIZE 2048
static void StrHashTable_print(struct str_hash_table *ht){

    int i;
    char buf[AGILELOG_PRINT_BUFFER_SIZE];
    char tmpBuf[256];
    
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    
    AGILE_LOGD("------Hash Table------");
    
    for(i = 0; i < ht->mTableSize; i++){
        memset(buf, 0, AGILELOG_PRINT_BUFFER_SIZE);
        if (NULL == ht->mpTable[i].head.first){
            AGILE_LOGD("%d - [NODE: NULL]", i);
        }else{
            sprintf(buf, "%d - ", i);
            hlist_for_each_entry(tnode, pos, &ht->mpTable[i].head, node){
                sprintf(tmpBuf, "[NODE: %s] ->", tnode->nodeStr);
                strcat(buf, tmpBuf);
            }
            AGILE_LOGD("%s END", buf);
        }
    }

    AGILE_LOGD("----- Table END  ------");
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
            AGILE_LOGD("remove node (%s)", tnode->nodeStr);
            hlist_del(prev);
            free((void *)tnode);
        }
    }
    free((void *)ht->mpTable);
    free(ht);
}

