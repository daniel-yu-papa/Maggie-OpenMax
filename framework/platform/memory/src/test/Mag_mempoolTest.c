#include "Mag_mempool.h"
#include "Mag_msg.h"
#include <time.h>

struct msgMemPool{
    int count;
    void *buffer;
};

static MagMsgChannelHandle hMsgChanl;
static int gLoop = 0;

#define TESTING_LOOPS 10000

void *fillBufferEntry(void *arg){
#define BASE_BYTES 128
    magMempoolHandle hmp = (magMempoolHandle)arg;
    int loop = 4;
    struct msgMemPool msg;
    int count = 0;
    void *pBuf;
    int i;
    int direction = 0;
    
    for (i = 0; i < TESTING_LOOPS; i++){
        count++;
        pBuf = magMemPoolGetBuffer(hmp, BASE_BYTES + loop);
        msg.buffer = pBuf;
        msg.count = count;
        Mag_MsgChannelSend(hMsgChanl, &msg, sizeof(struct msgMemPool));
        if (0 == direction){
           if (loop < 512)
                loop *= 2;
           else
                direction = 1;
        }else{
            if (loop > 4)
                loop /= 2;
            else
                direction = 0;
        }
        usleep(5);
    }
    sleep(2);
}

void emptyBufferReceiver(void *msg, void *priv){
    struct msgMemPool *pMsg = (struct msgMemPool *)msg;
    magMempoolHandle hmp = (magMempoolHandle)priv;
    
    AGILE_LOGD("count = %d, buffer = 0x%x", pMsg->count, pMsg->buffer);
    magMemPoolPutBuffer(hmp, pMsg->buffer);
    gLoop++;
    //usleep(100);
}

int main(){
    pthread_t fillBufThread;
    magMempoolHandle memPoolHandle;
    
    memPoolHandle = magMemPoolCreate(8*1024, 1);

    Mag_MsgChannelCreate(&hMsgChanl);
    
    Mag_MsgChannelReceiverAttach(hMsgChanl, emptyBufferReceiver, memPoolHandle);
    
    pthread_create(&fillBufThread, NULL, fillBufferEntry, memPoolHandle);

    pthread_join(fillBufThread, NULL);  

    while(gLoop < TESTING_LOOPS){}
    sleep(1);
    magMemPoolDump(memPoolHandle);
    
    AGILE_LOGD("executing is complete");
    
    magMemPoolDestroy(memPoolHandle);
    AGILE_LOGD("after magMemPoolDestroy");
    Mag_MsgChannelDestroy(hMsgChanl);
}
