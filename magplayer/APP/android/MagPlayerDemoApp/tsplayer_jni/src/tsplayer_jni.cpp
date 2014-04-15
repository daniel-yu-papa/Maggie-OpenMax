/**
 * @file 		tsplayer_jni.cpp
 * @author    	Yu Jun
 * @date      	2012/10/18
 * @version   	ver 1.0
 * @brief     	jni wrapper layer for Java code calling
 * @attention
*/

#include <utils/threads.h>
#include "android/log.h"
#include "jni.h"
#include "stdio.h"
#include <string.h>
#include "tsPlayer.h"

#define  LOG_TAG    "magplayerdemo"

using namespace android;

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


class ReadFileThread :public Thread
{
public:
        ReadFileThread(int numTsPacket, int wCycle);
        ~ReadFileThread();

        virtual bool  threadLoop();

        bool Start(CTC_MediaProcessor *p, const char *fileName);
        void Stop();

private:
        bool mRunning;
        FILE *mFile;
        unsigned char* mBuffer;
        int mNumTsPackets;
        int mWCycle;
        CTC_MediaProcessor *mPlayer;
};

ReadFileThread::ReadFileThread(int numTsPacket, int wCycle):
                mRunning(false),
                mNumTsPackets(numTsPacket),
                mWCycle(wCycle){

}

ReadFileThread::~ReadFileThread(){

}

bool ReadFileThread::threadLoop()
{
    int rd_result = 0;
    int expected = mNumTsPackets*188;
    
    if (mRunning){
        if (NULL != mFile){
    		rd_result = fread(mBuffer, 1, expected, mFile);
            if (rd_result < expected){
                LOGE("Read to the end of file(remaining: %d bytes)", rd_result);
                mPlayer->WriteData(mBuffer, rd_result);
                mPlayer->WriteData(mBuffer, 0);
				return false;
            }else{
                mPlayer->WriteData(mBuffer, rd_result);
            }
    		
    		usleep(mWCycle*1000);
	    }
    }
    return true;
}

bool ReadFileThread::Start(CTC_MediaProcessor *p, const char *fileName)
{
    mFile = fopen(fileName, "rb+");
    if (NULL != mFile){
        LOGI("open the file: %s", fileName);
        mPlayer  = p;
        mRunning = true;
        mBuffer = (unsigned char *)malloc(mNumTsPackets*188);
        if (run("readFileThread", ANDROID_PRIORITY_NORMAL) != 0){
            LOGE("failed to run readFileThread");
            return false;
        }
    }else{
        LOGE("failed to open the file: %s", fileName);
        return false;
    }
    return true;
}

void ReadFileThread::Stop()
{
    mRunning = false;
    //requestExitAndWait();
}

#ifdef __cplusplus
extern "C" {
#endif

CTC_MediaProcessor* ctc_MediaProcessor = NULL;
FILE *fp = NULL;
int isPause = 0;
char gFileName[128];
/*YJ comment: the thread only get running once if using ReadFileThread *gThread. we must use below definition to get it working as expected!*/
sp<ReadFileThread> gThread;

void Java_com_magplayer_MagPlayerDemo_nativeInit(JNIEnv* env, jobject thiz, jint vpid, jint vcodec, jint apid, jint acodec, jint numTSPackets, jint wCycle, jstring url)
{
    LOGI("Enter Java_com_magplayer_MagPlayerDemo_nativeInit");
	VIDEO_PARA_T  videoPara;
	videoPara.pid = vpid;
	if (vcodec == 0){
	    videoPara.vFmt = VFORMAT_H264;
	}else if (vcodec == 1){
	    videoPara.vFmt = VFORMAT_MPEG4;
	}else if (vcodec == 2){
	    videoPara.vFmt = VFORMAT_MPEG12;
	}else{
	    LOGE("failed to get the correct video codec(%d)", vcodec);
	    return;
	}
	
	AUDIO_PARA_T audioPara[2];
	audioPara[0].pid = apid;
        if (acodec == 0){
	    audioPara[0].aFmt = FORMAT_MPEG;
	}else if (acodec == 1){
	    audioPara[0].aFmt = FORMAT_AAC;
	}else if (acodec == 2){
            audioPara[0].aFmt = FORMAT_AC3;  	    
        }else if (acodec == 3){
	    audioPara[0].aFmt = FORMAT_DDPlus;
	}else{
	    LOGE("failed to get the correct audio codec(%d)", acodec);
	    return;
	}
	audioPara[1].pid = 0;
	
	strcpy(gFileName, (*env).GetStringUTFChars(url, NULL));
	LOGI("tsplayer file name is %s", gFileName);
	
	gThread = new ReadFileThread(numTSPackets, wCycle);
	ctc_MediaProcessor = GetMediaProcessor();
	
	if (ctc_MediaProcessor != NULL){
    	ctc_MediaProcessor->InitVideo(&videoPara);
    	ctc_MediaProcessor->InitAudio(audioPara);
    }
}


void Java_com_magplayer_MagPlayerDemo_nativePlay(JNIEnv* env, jobject thiz)
{
    LOGI("Enter Java_com_magplayer_MagPlayerDemo_nativePlay");
    gThread->Start(ctc_MediaProcessor, gFileName);
	jboolean result = ctc_MediaProcessor->StartPlay();
}

void Java_com_magplayer_MagPlayerDemo_nativeStop(JNIEnv* env, jobject thiz)
{
    LOGI("Enter Java_com_magplayer_MagPlayerDemo_nativeStop");

    gThread->Stop();
	jboolean result = ctc_MediaProcessor->Stop();
}

void Java_com_magplayer_MagPlayerDemo_nativeDestroy(JNIEnv* env, jobject thiz)
{
    LOGI("Enter Java_com_magplayer_MagPlayerDemo_nativeDestroy");

	delete ctc_MediaProcessor;
	ctc_MediaProcessor = NULL;
}

#ifdef __cplusplus
}
#endif
