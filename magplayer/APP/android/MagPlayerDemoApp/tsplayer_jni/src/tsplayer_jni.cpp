/**
 * @file 		tsplayer_jni.cpp
 * @author    	Yu Jun
 * @date      	2012/10/18
 * @version   	ver 1.0
 * @brief     	jni wrapper layer for Java code calling
 * @attention
*/

#include "tsPlayer.h"
#include <utils/threads.h>

using namespace android;

#include "android/log.h"
#include "jni.h"
#include "stdio.h"
#include <string.h>

#define  LOG_TAG    "CTC_MediaProcessor"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define  BUFF_SIZE (40*188)
#define  FEED_DATA_CYCLE (40 * 1000) //40ms

class ReadFileThread :public Thread
{
public:
        ReadFileThread(int numTsPacket, int wCycle);
        ~ReadFileThread();

        virtual bool  threadLoop();

        bool Start(const char *source);
        void Stop();

private:
        bool mRunning;
        FILE *mFile;
        CTC_MediaProcessor *mPlayer;
        char* mBuffer;
        int mNumTsPackets;
        int mWCycle;
};

ReadFileThread::ReadFileThread(int numTsPacket, int wCycle):
                mNumTsPackets(numTsPacket),
                mWCycle(wCycle),
                mRunning(false),
                mFile(NULL){

}

ReadFileThread::~ReadFileThread(){

}

bool ReadFileThread::threadLoop()
{
    int rd_result = 0;
    if (mRunning){
        if (NULL != mFile){
    		rd_result = fread(mBuffer, (numTsPacket*188), 1, mFile);
    		if (rd_result <= 0)	
			{
				LOGE("read the end of file");
				return false;
			}
    		
    		int wd_result = mPlayer->WriteData(mBuffer, rd_result);
    		usleep(wCycle*1000);
	    }
    }
}

bool ReadFileThread::Start(CTC_MediaProcessor *p, const char *fileName)
{
    mFile = fopen(fileName, "rb+");
    if (NULL != mFile){
        if (run("readFileThread", ANDROID_PRIORITY_NORMAL) != 0){
            LOGE("failed to run readFileThread");
            return false;
        }
    }else{
        LOGE("failed to open the file: %s", fileName);
        return false;
    }
    
    mPlayer  = p;
    mRunning = true;
    mBuffer = (char* )malloc(BUFF_SIZE);
    return ture;
}

void CExeThread::Stop()
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
ReadFileThread *gThread;

void Java_com_magplayer_MagPlayerDemo_nativeTsPlayer_Init(JNIEnv* env, jobject thiz, jint vpid, jint vcodec, jint apid, jint acodec, jint numTSPackets, jint wCycle, jstring url)
{
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
	
	AUDIO_PARA_T audioPara;
	audioPara.pid = apid;
    if (acodec == 0){
	    audioPara.aFmt = FORMAT_MPEG;
	}else if (acodec == 1){
	    audioPara.aFmt = FORMAT_AAC;
	}else if (acodec == 2){
	    audioPara.aFmt = FORMAT_DDPlus;
	}else{
	    LOGE("failed to get the correct audio codec(%d)", acodec);
	    return;
	}
	
	strcpy(gFileName, (*env).GetStringUTFChars(url, NULL));
	LOGI("tsplayer file name is %s", fileName);
	
	gThread = new ReadFileThread(numTSPackets, wCycle);
	ctc_MediaProcessor = GetMediaProcessor();
	
	if (ctc_MediaProcessor != NULL){
    	ctc_MediaProcessor->InitVideo(&videoPara);
    	ctc_MediaProcessor->InitAudio(&audioPara);
    }
}


void Java_com_magplayer_MagPlayerDemo_nativeTsPlayer_Play(JNIEnv* env, jobject thiz)
{
    gThread->Start(ctc_MediaProcessor, gFileName);
	jboolean result = ctc_MediaProcessor->StartPlay();
}

void Java_com_magplayer_MagPlayerDemo_nativeTsPlayer_Stop(JNIEnv* env, jobject thiz)
{
    gThread->Stop();
	jboolean result = ctc_MediaProcessor->Stop();
}

void Java_com_magplayer_MagPlayerDemo_nativeTsPlayer_Destroy(JNIEnv* env, jobject thiz)
{
	delete ctc_MediaProcessor;
	ctc_MediaProcessor = NULL;
}

#ifdef __cplusplus
}
#endif
