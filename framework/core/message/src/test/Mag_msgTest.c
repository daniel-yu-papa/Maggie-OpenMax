#include "Mag_msg.h"
#include "Mag_agilelog.h"
#include <stdio.h>

static int  thread_ids[3] = {1,2,3};
static unsigned int loops = 100;
static MagMsgChannelHandle handle;

struct msgTest{
    int msgID;
    char msgContent[64];
};

void MessageChannelReceiver(void *msg, void *priv){
    struct msgTest *msgGet = (struct msgTest *)msg;
    
    AGILE_LOGD("msgID = %d, msgContent = %s", msgGet->msgID, msgGet->msgContent);
}

void *post_msg_thread(void *arg){
    int count = 1;
    int index = *((int *)arg);
    struct msgTest msg;
    
    while (count <= loops){
        msg.msgID = count;
        sprintf(msg.msgContent, "test msg %d from thread[%d]", count, index);
        Mag_MsgChannelSend(handle, &msg, sizeof(struct msgTest));
        usleep(index * count);
        
        AGILE_LOGI("send the %d message from thread(%d)", count, index);
        count++;
    }
}

int main(){
    int i = 0;
    
    pthread_t threads[3]; 
    
    Mag_MsgChannelCreate(&handle);
    
    Mag_MsgChannelReceiverAttach(handle, MessageChannelReceiver, NULL);

    AGILE_LOGD("enter ...");

    pthread_create(&threads[0], NULL, post_msg_thread, &thread_ids[0]);  
    pthread_create(&threads[1], NULL, post_msg_thread, &thread_ids[1]);
    pthread_create(&threads[2], NULL, post_msg_thread, &thread_ids[2]); 
    
    for (i = 0; i < 3; i++) {  
        pthread_join(threads[i], NULL);  
    }

    Mag_MsgChannelDestroy(handle);
}

