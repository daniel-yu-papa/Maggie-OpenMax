#include <stdio.h>
#include <string.h>

#include "hashTable.h"
#include "hashTableC.h"
#include "agilelog.h"

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
            AGILE_LOGD("remove node (%s)", tnode->nodeStr);
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
        AGILE_LOGE("the string[%s] has not been added into the Table!!", str);
        return NULL;
    }else{
        /*several nodes has the same hash key so traverse the link*/
        hlist_for_each_entry(tnode, pos, &mpTable[tableIndex].head, node){
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
void HashTable::printHashTable(void){

    int i;
    char buf[AGILELOG_PRINT_BUFFER_SIZE];
    char tmpBuf[256];
    
    LinkedKeyNode_t *tnode = NULL;
    struct hlist_node *pos;
    
    AGILE_LOGD("------Hash Table------");
    
    for(i = 0; i < mTableSize; i++){
        memset(buf, 0, AGILELOG_PRINT_BUFFER_SIZE);
        if (NULL == mpTable[i].head.first){
            AGILE_LOGD("%d - [NODE: NULL]", i);
        }else{
            sprintf(buf, "%d - ", i);
            hlist_for_each_entry(tnode, pos, &mpTable[i].head, node){
                sprintf(tmpBuf, "[NODE: %s] ->", tnode->nodeStr);
                strcat(buf, tmpBuf);
            }
            AGILE_LOGD("%s END", buf);
        }
    }

    AGILE_LOGD("----- Table END  ------");
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

/*the interfaces for the calling from c*/
void* StrHashTable_Create(int maxNum){
    HashTable *p = new HashTable(maxNum);
    return (void *)p;
}

void StrHashTable_Destroy(void *h){
    HashTable *pc = static_cast<HashTable *>(h);
    delete pc;
}


void StrHashTable_AddItem(void *obj, const char *str, void *item){
    HashTable *pc = static_cast<HashTable *>(obj);

    pc->buildHashTable(item, str);
}

void *StrHashTable_GetItem(void *obj, const char *str){
    HashTable *pc = static_cast<HashTable *>(obj);

    return (pc->getHashItem(str));
}

