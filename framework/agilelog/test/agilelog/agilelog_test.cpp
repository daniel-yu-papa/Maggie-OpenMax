#include "agilelog.h"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif 
#define MODULE_TAG "AGILELOG_TEST"


static void loadComponentLib(const char *file, void *arg){
    AGILE_LOGI("load file %s.", file);
    *(int *)arg = *(int *)arg + 1;
}
#if 1
static void loadComponentRecursive(char *loadPath,
                                              void (*loader)(const char *file, void *arg),
                                              void *arg){
    DIR *dir;
    struct dirent *fileInfo;
    char dirPathFull[PATH_MAX];
    
    dir = opendir(loadPath);

    if (NULL == dir){
        AGILE_LOGE("failed to open the dir: %s (err = %s)", loadPath, strerror(errno));
        return;
    }

    do{
        fileInfo = readdir(dir);

        if (NULL == fileInfo){
            //AGILE_LOGE("failed to read the dir: %s (err = %s)", loadPath, strerror(errno));
            continue;
        }
        
        if( (fileInfo->d_type == DT_DIR) && 
            !(!strcmp(fileInfo->d_name, ".") || !strcmp(fileInfo->d_name, ".."))){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            AGILE_LOGI("find dir: %s", dirPathFull);
            loadComponentRecursive(dirPathFull, loader, arg);
        }else if ((fileInfo->d_type == DT_REG) || (fileInfo->d_type == DT_LNK)){
            sprintf(dirPathFull, "%s%s", loadPath, fileInfo->d_name);
            AGILE_LOGI("find file: %s", dirPathFull);
            (*loader)(dirPathFull, arg);
        }
    }while (fileInfo != NULL);

    closedir(dir);
}
#endif
int main(){
    int i = 9;
    char str[30] = "Hello world - AgileLog";
    char dir[30] = "/home/yujun/testdir/";
    
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

    int num = 0;
    //loadComponentRecursive(dir, loadComponentLib, (void *)&num);
    AGILE_LOGI("%d files are found", num);
    return 0;
}
