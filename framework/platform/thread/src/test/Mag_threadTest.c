#include <stdio.h>
#include <unistd.h>
#include "Mag_thread.h"
#include "Mag_agilelog.h"
#include "Mag_mem.h"

MagThreadHandle gThreads_eat;
MagThreadHandle gThreads_work;
MagThreadHandle gThreads_sleep;

struct person father;
struct person mother;
struct person child;

struct person{
    i32 id;
    char *name;
    i32  age;
    i32 times;
};

static boolean EatThreadEntry(void *priv){
    if (priv == NULL)
        AGILE_LOGE("priv is NULL");
    else{
        struct person *p = (struct person *)priv;

        AGILE_LOGD("thread eating: times:%d, id=%d, name=%s, age=%d", ++p->times, p->id, p->name, p->age);
        usleep(1000);
    }
    return MAG_TRUE;
}

static boolean EatThread_ReadyToRun(void *priv){
    struct person *p = (struct person *)priv;
    
    AGILE_LOGD("%s is ready to eat!", p->name);

    return MAG_TRUE;
}

static boolean WorkThreadEntry(void *priv){
    struct person *p = (struct person *)priv;

    AGILE_LOGD("thread working: times:%d, id=%d, name=%s, age=%d", ++p->times, p->id, p->name, p->age);
    usleep(1000);
    return MAG_TRUE;
}

static boolean SleepThreadEntry(void *priv){
    struct person *p = (struct person *)priv;

    AGILE_LOGD("thread sleeping: times:%d, id=%d, name=%s, age=%d", ++p->times, p->id, p->name, p->age);
    usleep(3000);
    return MAG_TRUE;
}


int main(){
    
    father.name = mag_strdup("father");
    father.age  = 40;
    father.id   = 1;
    father.times= 0;

    mother.name = mag_strdup("mother");
    mother.age  = 30;
    mother.id   = 2;
    mother.times= 0;

    child.name = mag_strdup("child");
    child.age  = 10;
    child.id   = 3;
    child.times= 0;
    
    gThreads_eat = Mag_CreateThread("eating", EatThreadEntry, (void *)&father);
    gThreads_eat->setFunc_readyToRun(gThreads_eat, EatThread_ReadyToRun);
    gThreads_eat->run(gThreads_eat);
    gThreads_eat->setParm_Priority(gThreads_eat, MAGTHREAD_PRIORITY_HIGH);

    gThreads_work = Mag_CreateThread("working", WorkThreadEntry, (void *)&mother);
    gThreads_work->run(gThreads_work);
    
    gThreads_sleep = Mag_CreateThread("sleeping", SleepThreadEntry, (void *)&child);
    gThreads_sleep->run(gThreads_sleep);
    
    sleep(2);

    gThreads_eat->requestExitAndWait(gThreads_eat, MAG_TIMEOUT_INFINITE);
    gThreads_work->requestExitAndWait(gThreads_work, MAG_TIMEOUT_INFINITE);
    gThreads_sleep->requestExitAndWait(gThreads_sleep, MAG_TIMEOUT_INFINITE);

    
    Mag_DestroyThread(&gThreads_eat);
    Mag_DestroyThread(&gThreads_work);
    Mag_DestroyThread(&gThreads_sleep);

    return 0;
}

