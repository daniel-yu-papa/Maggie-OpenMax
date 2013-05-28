#include "Mag_mempool.h"
#include "Mag_msg.h"
#include "agilelog.h"

struct msgMemPool{
    int count;
    void *buffer;
};

static MagMsgChannelHandle hMsgChanl;

void *fillBufferEntry(void *arg){
#define BASE_BYTES 128
    magMempoolHandle hmp = (magMempoolHandle)arg;
    int loop = 4;
    struct msgMemPool msg;
    int count = 0;
    void *pBuf;
    int i;
    int direction = 0;
    
    for (i = 0; i < 100; i++){
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
    usleep(100);
}



int main(){
    pthread_t fillBufThread;
    magMempoolHandle memPoolHandle;
    
    memPoolHandle = magMemPoolCreate(1*1024);

    Mag_MsgChannelCreate(&hMsgChanl);
    
    Mag_MsgChannelReceiverAttach(hMsgChanl, emptyBufferReceiver, memPoolHandle);
    
    pthread_create(&fillBufThread, NULL, fillBufferEntry, memPoolHandle);

    pthread_join(fillBufThread, NULL);  

    AGILE_LOGD("executing is complete");
    
    magMemPoolDestroy(memPoolHandle);
    AGILE_LOGD("after magMemPoolDestroy");
    Mag_MsgChannelDestroy(hMsgChanl);
}
