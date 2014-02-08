/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "streamBufferService"
//#define LOG_NDEBUG 0

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include "StreamBufferDef.h"
#include "agilelog.h"
#include "Mag_pub_type.h"
#include "IStreamBufTest.h"

using namespace android;

#define OUTPUT_FILENAMR "/data/streamBufTest.data"

struct SBTestService : public BnSBTestService{
    SBTestService();
    static void instantiate();
    virtual void setStreamBuffer(const sp<IStreamBuffer> &buffer);
    virtual void start();
    
private:
    void writeToFile();

    FILE *mOutputFile;
    sp<StreamBufferUser> mStreamBufUser;
}; 

SBTestService::SBTestService(){

}

void SBTestService::instantiate(){
    defaultServiceManager()->addService(String16("streamBuffer.test"), new SBTestService());
}

void SBTestService::setStreamBuffer(const sp<IStreamBuffer> &buffer){
    mStreamBufUser = new StreamBufferUser(buffer, 1*1024*1024, 1);
}

void SBTestService::writeToFile(){
    ui8 buf[1024];
    _size_t readings;
    _size_t size = 218;
    
    while(!mStreamBufUser->isEOS()){
        size = (size + 1) % 1000;
        readings = mStreamBufUser->read(buf, size);
        if (readings > 0)
            fwrite(buf, 1, readings, mOutputFile);
        usleep(1000);
    }

    readings = 0;
    do{
        size = (size + 1) % 1000;
        readings = mStreamBufUser->read(buf, size);
        fwrite(buf, 1, readings, mOutputFile);
    }while(readings);

    mStreamBufUser->reset();
    fclose(mOutputFile);
}

void SBTestService::start(){
    AGILE_LOGI("enter!");
    mOutputFile = fopen(OUTPUT_FILENAMR,"wb+");
    if (NULL != mOutputFile){
        writeToFile();
    }else{
        AGILE_LOGE("failed to open the output file: %s", OUTPUT_FILENAMR);
    }
    AGILE_LOGI("exit!");
}


int main(int argc, char** argv)
{
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    AGILE_LOGI("ServiceManager: %p", sm.get());
    SBTestService::instantiate();
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
}

