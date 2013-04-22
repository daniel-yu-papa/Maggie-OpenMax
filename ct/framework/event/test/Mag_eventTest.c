#include "Mag_event.h"
#include <pthread.h>
#include <stdio.h>
#include "agilelog.h"

#define MODULE_TAG "MAG_EVENT_TEST"

int  thread_ids[4] = {0,1,2,3};
MagEventGroupHandle evtGrp;
MagEventGroupElementHandle evtGrpElem[3];


void *inc_count(void *arg){
    int count = 1;
    int index = *((int *)arg);
    
    while (count <= 10000){
        usleep((index + 1)*count);
        Mag_SetEventGroupElement(evtGrpElem[index]);
        AGILE_LOGI("send the %d event from thread(%d)", count, index);
        count++;
    }
}

void *watch_count(void *arg){
    int count = 1;
    
    while(count < 30000){
        Mag_WaitForEventGroup(evtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGD("the event is triggered!! number: %d", count);
        count++;
    }
}

int main(){
    
    int      i;  
    pthread_t threads[4]; 

    Mag_CreateEventGroup(&evtGrp);
    for (i = 0; i < 3; i++){
        if (MAG_ErrNone == Mag_CreateEventGroupElement(&evtGrpElem[i]))
            Mag_AddEventGroup(evtGrp, evtGrpElem[i]);
        
    }
    pthread_create(&threads[3],NULL,watch_count, &thread_ids[3]);
    
    pthread_create(&threads[0],NULL,inc_count, &thread_ids[0]);  
    pthread_create(&threads[1],NULL,inc_count, &thread_ids[1]);
    pthread_create(&threads[2],NULL,inc_count, &thread_ids[2]); 
      
    
    for (i = 0; i < 4; i++) {  
        pthread_join(threads[i], NULL);  
    }  

    Mag_DestroyEventGroup(evtGrp);
    return 0; 
    
}
