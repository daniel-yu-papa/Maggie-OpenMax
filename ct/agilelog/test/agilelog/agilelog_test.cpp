#include "agilelog.h"
#include <unistd.h>

#define MODULE_TAG "AGILELOG_TEST"

int main(){
    int i = 9;
    char str[30] = "Hello world - AgileLog";
    
    AGILE_LOGD("debug log: %s - %d", str, i);
    usleep(10000);
    AGILE_LOGV("verbose log: %s - %d", str, i);
    usleep(20000);
    AGILE_LOGE("error log: %s - %d", str, i);
    usleep(40000);
    AGILE_LOGW("warning log: %s - %d", str, i);
    usleep(60000);
    AGILE_LOGI("info log: %s - %d", str, i);
    usleep(80000);
    AGILE_LOG_FATAL("fatal log: %s - %d", str, i);

    return 0;
}
