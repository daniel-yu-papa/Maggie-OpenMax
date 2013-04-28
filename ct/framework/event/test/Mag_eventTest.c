#include "Mag_event.h"
#include <pthread.h>
#include <stdio.h>
#include "agilelog.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif 
#define MODULE_TAG "MAG_EVENT_TEST"

int  thread_ids[4] = {0,1,2,3};
MagEventGroupHandle evtGrp;
MagEventHandle evtGrpElem[3];
MagEventHandle highPrioEvt[3];
MagEventSchedulerHandle hEvtSched;

static int all_count = 0;
static unsigned int loops = 10000;
void *inc_count(void *arg){
    int count = 1;
    int index = *((int *)arg);
    
    while (count <= loops){
        usleep((index + 1)*count);
        Mag_SetEvent(evtGrpElem[index]);
        usleep((index + 1)*count);
        Mag_SetEvent(highPrioEvt[index]);
        
        AGILE_LOGI("send the %d event from thread(%d)", count, index);
        count++;
    }
    all_count += count;
}

void *watch_count(void *arg){
    int count = 1;
    
    while(all_count <= (loops*3)){
        Mag_WaitForEventGroup(evtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("the event is triggered!! number: %d", count);
        count++;
    }
}
static void evtGrpElem1_callback(void *arg){
    AGILE_LOGD("do evtGrpElem[%d]_callback\n", *((int *)arg));
}

static void evtGrpElem2_callback(void *arg){
    AGILE_LOGD("do evtGrpElem[%d]_callback\n", *((int *)arg));
}

static void evtGrpElem3_callback(void *arg){
    AGILE_LOGD("do evtGrpElem[%d]_callback\n", *((int *)arg));
}

static void hPrioEvtGrpElem1_callback(void *arg){
    AGILE_LOGD("do hPrioEvtGrpElem1_callback\n");
}

static void hPrioEvtGrpElem2_callback(void *arg){
    AGILE_LOGD("do hPrioEvtGrpElem2_callback\n");
}

static void hPrioEvtGrpElem3_callback(void *arg){
    AGILE_LOGD("do hPrioEvtGrpElem3_callback\n");
}




int main(){
    
    int      i;  
    pthread_t threads[4]; 

    AGILE_LOGD("start ...");
    
    Mag_CreateEventGroup(&evtGrp);
    for (i = 0; i < 3; i++){
        if (MAG_ErrNone == Mag_CreateEvent(&evtGrpElem[i], 0)){}
            Mag_AddEventGroup(evtGrp, evtGrpElem[i]);
        
    }

    for (i = 0; i < 3; i++){
        Mag_CreateEvent(&highPrioEvt[i], MAG_EVT_PRIO_HIGH);
    }
    
    Mag_CreateEventScheduler(&hEvtSched, MAG_EVT_SCHED_NORMAL);

    Mag_RegisterEventCallback(hEvtSched, evtGrpElem[0], evtGrpElem1_callback, (void *)&thread_ids[0]);
    Mag_RegisterEventCallback(hEvtSched, evtGrpElem[1], evtGrpElem2_callback, (void *)&thread_ids[1]);
    Mag_RegisterEventCallback(hEvtSched, evtGrpElem[2], evtGrpElem3_callback, (void *)&thread_ids[2]);

    Mag_RegisterEventCallback(hEvtSched, highPrioEvt[0], hPrioEvtGrpElem1_callback, NULL);
    Mag_RegisterEventCallback(hEvtSched, highPrioEvt[1], hPrioEvtGrpElem2_callback, NULL);
    Mag_RegisterEventCallback(hEvtSched, highPrioEvt[2], hPrioEvtGrpElem3_callback, NULL);

    pthread_create(&threads[3],NULL,watch_count, &thread_ids[3]);
    
    pthread_create(&threads[0],NULL,inc_count, &thread_ids[0]);  
    pthread_create(&threads[1],NULL,inc_count, &thread_ids[1]);
    pthread_create(&threads[2],NULL,inc_count, &thread_ids[2]); 
      
    
    for (i = 0; i < 4; i++) {  
        pthread_join(threads[i], NULL);  
    }  

    Mag_DestroyEventGroup(evtGrp);
    AGILE_LOGD("test: 222");
    for (i = 0; i < 3; i++){
        //Mag_DestroyEvent(evtGrpElem[i]);
        Mag_UnregisterEventCallback(evtGrpElem[i]);
    }

    return 0; 
    
}
