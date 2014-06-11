#include "StreamBufferDef.h"
#include "Mag_thread.h"

#include "IStreamBufTest.h"

#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

char gFileName[256];
boolean firstLoopDone = MAG_FALSE;

boolean writeFromFileLooper(void *priv){
    StreamBuffer *sb = static_cast<StreamBuffer *>(priv);
    FILE *inputFile = NULL;
    char buf[2048];
    ui32 size = 100;
    ui32 readings;

    if (!firstLoopDone){
        inputFile = fopen(gFileName, "r");
        if (NULL == inputFile){
            AGILE_LOGE("failed to open the file(%s) for reading", gFileName);
            return MAG_FALSE;
        }else{
            AGILE_LOGI("To open the file(%s) for reading -- OK!", gFileName);
        }
        
        do{
            size = (size + 1) % 1000;
            readings = fread(buf, 1, size, inputFile);
            sb->WriteData(buf, readings, true);
            usleep(5000);
        }while(readings == size);

        AGILE_LOGI("reach to the end of file, readings=%d", readings);
        sb->getUser()->issueCommand(IStreamBufferUser::EOS, true);

        if (inputFile)
            fclose(inputFile);
        
        firstLoopDone = MAG_TRUE;
    }else{
        sleep(10);
    }
    return MAG_TRUE;
}

int main(int argc, char **argv) {
    MagThreadHandle wThread;
    
    android::ProcessState::self()->startThreadPool();
    
    sp<IStreamBuffer> sb = new StreamBuffer();

    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }
    
    strcpy(gFileName, argv[1]);
    
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("streamBuffer.test"));
    sp<ISBTestService> service = interface_cast<ISBTestService>(binder);

    if(service.get() == NULL){
        AGILE_LOGE("failed to get the service: streamBuffer.test");
        return 1;
    }
    
    service->setStreamBuffer(sb);
 
    wThread = Mag_CreateThread("streamBufClientTest", writeFromFileLooper, static_cast<void *>(sb.get()));
    
    service->start();

    wThread->run(wThread);

    sleep(2);
    
    wThread->requestExitAndWait(wThread, MAG_TIMEOUT_INFINITE);

    sleep(10);
    
    Mag_DestroyThread(wThread);
    
    AGILE_LOGI("Quit the streamBuf_client_test app");
}

