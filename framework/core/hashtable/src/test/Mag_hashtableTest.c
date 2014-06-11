#include "Mag_hashTable.h"
#include <stdio.h>

int main(){
    HashTableHandle pTable = createMagStrHashTable(10);

    pTable->addItem(pTable, NULL, "YuJun");
    pTable->addItem(pTable, NULL, "PengQianlan");
    pTable->addItem(pTable, NULL, "XiaoXiao");
    pTable->addItem(pTable, NULL, "Mother");
    pTable->addItem(pTable, NULL, "Father");
    pTable->addItem(pTable, NULL, "Ridiculous");
    pTable->addItem(pTable, NULL, "Please");
    pTable->addItem(pTable, NULL, "helloworld");
    pTable->addItem(pTable, NULL, "sorry");
    pTable->addItem(pTable, NULL, "happy");

    pTable->getItem(pTable, "XiaoXiao");
    pTable->getItem(pTable, "Please");
    pTable->getItem(pTable, "Ridiculous");
    pTable->getItem(pTable, "TestT");

    pTable->print(pTable);

    destroyMagStrHashTable(pTable);
}
