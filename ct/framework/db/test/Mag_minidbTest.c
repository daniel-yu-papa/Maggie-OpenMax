#include "Mag_minidb.h"
#include "Mag_pub_type.h"
#include "Mag_pub_def.h"
#include "agilelog.h"

int main(){
    i32 value;
    char *s;
    i64 l;
    boolean ret;
    
    MagMiniDBHandle h = createMagMiniDB(32);

    h->setInt32(h, "testDB_item_Int32_1", 11);
    h->setString(h, "testDB_item_String_1", "Yu Jun");
    h->setInt64(h, "testDB_item_Int64_1", 3333l);
    h->setInt32(h, "testDB_item_Int32_1", 22);
    h->setString(h, "testDB_item_String_1", "Xiaoxiao");

    ret = h->findInt32(h, "testDB_item_Int32_1", &value);
    if (MAG_TRUE == ret)
        AGILE_LOGI("testDB_item_Int32_1: %d", value);

    ret = h->findString(h, "testDB_item_String_1", &s);
    if (MAG_TRUE == ret)
        AGILE_LOGI("testDB_item_String_1: %s", s);

    ret = h->findInt64(h, "testDB_item_Int64_1", &l);
    if (MAG_TRUE == ret)
        AGILE_LOGI("testDB_item_Int64_1: %lld", l);

    destroyMagMiniDB(h);
    
    return 0;
}
