#include "hashTable.h"

//using namespace AGILELOG;

int main(){
    HashTable *pTable = new HashTable(10);

    pTable->buildHashTable(NULL, "YuJun");
    pTable->buildHashTable(NULL, "PengQianlan");
    pTable->buildHashTable(NULL, "XiaoXiao");
    pTable->buildHashTable(NULL, "Mother");
    pTable->buildHashTable(NULL, "Father");
    pTable->buildHashTable(NULL, "Ridiculous");
    pTable->buildHashTable(NULL, "Please");
    pTable->buildHashTable(NULL, "helloworld");
    pTable->buildHashTable(NULL, "sorry");
    pTable->buildHashTable(NULL, "happy");

    pTable->getHashItem("XiaoXiao");
    pTable->getHashItem("Please");
    pTable->getHashItem("Ridiculous");
    pTable->getHashItem("TestT");
    
    pTable->printHashTable();

    delete pTable;
}
