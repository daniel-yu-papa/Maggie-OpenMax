#include "hashTable.h"

#include <stdio.h>
#include <string.h>

namespace AGILELOG {

HashTable::HashTable(int num){
    mpTable = (HashTable_t *)malloc(sizeof(HashTable_t) * num);
    memset(mpTable, 0, sizeof(HashTable_t) * num);
    mTableSize = num;
}

HashTable::~HashTable(){
    int i;
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    struct hlist_node *prev;
    
    for(i = 0; i < mTableSize; i++){
        while(NULL != mpTable[i].head.first){
            hlist_for_each_entry_to_tail(tnode, pos, prev, &mpTable[i].head, node){
            }
            printf("%s: remove node (%s)\n", __FUNCTION__, tnode->nodeStr);
            hlist_del(prev);
            free((void *)tnode);
        }
    }
    free((void *)mpTable);
}

void HashTable::buildHashTable(void *item, const char *str){
    unsigned int tableIndex = 0;

    tableIndex = calcDJBHashValue(str, strlen(str)) % mTableSize;

    LinkedKeyNode_t *node = (LinkedKeyNode_t *)malloc(sizeof(LinkedKeyNode_t));
    INIT_HLIST_NODE(&node->node);
    node->nodeValue = item;
    strcpy(node->nodeStr, str);
    hlist_add_head(&node->node, &mpTable[tableIndex].head);
}

void *HashTable::getHashItem(const char *str){
    unsigned int tableIndex = 0;
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    
    tableIndex = calcDJBHashValue(str, strlen(str)) % mTableSize;

    if (NULL == mpTable[tableIndex].head.first){
        printf("%s: the string[%s] has not been added into the Table!!\n", __FUNCTION__, str);
        return NULL;
    }else{
        /*several nodes has the same hash key so traverse the link*/
        hlist_for_each_entry(tnode, pos, &mpTable[tableIndex].head, node){
            if(!strcmp(str, tnode->nodeStr)){
                printf("%s: get the node (%s)\n", __FUNCTION__, tnode->nodeStr);
                return tnode->nodeValue;
            }
        }
    }
    
    printf("%s: failed to get the node (%s)\n", __FUNCTION__, str);
    return NULL;
}

#define AGILELOG_PRINT_BUFFER_SIZE 2048
void HashTable::printHashTable(void){

    int i;
    char buf[AGILELOG_PRINT_BUFFER_SIZE];
    char tmpBuf[128];
    
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    
    printf("\n\t------Hash Table------\n");
    
    for(i = 0; i < mTableSize; i++){
        memset(buf, 0, AGILELOG_PRINT_BUFFER_SIZE);
        if (NULL == mpTable[i].head.first){
            printf("\t%d - [NODE: NULL]\n", i);
        }else{
            sprintf(buf, "\t%d - ", i);
            hlist_for_each_entry(tnode, pos, &mpTable[i].head, node){
                memset(tmpBuf, 0, 128);
                sprintf(tmpBuf, "[NODE: %s] ->", tnode->nodeStr);
                strcat(buf, tmpBuf);
            }
            printf("%s\n", buf);
        }
    }

    printf("\t-------  END  --------\n");
}

unsigned int HashTable::calcDJBHashValue(const char* str, unsigned int len){
   unsigned int hash = 5381;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {   
      hash = ((hash << 5) + hash) + (*str);
   }   

   return hash;
}

}

