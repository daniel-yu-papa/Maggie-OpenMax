#include "tsPlayer.h"
#include "platform.h"
#include "Parameters.h"
#include "OMX_Video.h"
#include "OMX_Audio.h"
#include "OMX_AudioExt.h"

#include "EventType.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagTsPlayer"


MAG_SINGLETON_STATIC_INSTANCE(TsPlayer)

CTC_MediaProcessor* GetMediaProcessor()
{
    AGILE_LOGD("enter");
    
    TsPlayer& inst = TsPlayer::getInstance();
    
    return dynamic_cast<CTC_MediaProcessor *>(&inst);
}

int GetMediaProcessorVersion()
{
    AGILE_LOGD("enter");    
    return 2; //uses the interfaces after and including SwitchAudioTrack()
    //return 1; //uses the interfaces before SwitchAudioTrack()
}

void TsPlayer::initialize(){
    _status_t ret;

    mPlayer = new MagPlayerClient();
    if (NULL != GET_POINTER(mPlayer)){
        mStreamBuf = new StreamBuffer();

        if (NULL == GET_POINTER(mStreamBuf)){
            AGILE_LOG_FATAL("failed to create StreamBuffer object!");
            return;
        }
        StreamBuffer *sb = static_cast<StreamBuffer *>(GET_POINTER(mStreamBuf));
        sb->setType(IStreamBuffer::TS);

        ret = mPlayer->setDataSource(mStreamBuf);
        if (ret != MAG_NO_ERROR){
            AGILE_LOGE("failed to do setDataSource(ret = 0x%x)", ret);
            return;
        }else{
            AGILE_LOGV("To do setDataSource successfully!");
        }
    }else{
        AGILE_LOG_FATAL("failed to create MagPlayerClient object!");
        return;
    }

    mpListener = new TsPlayerListener(this);
    if (mpListener != NULL)
        mPlayer->setListener(mpListener);

    mbInitialized = true;
    mbError       = false;
}

void TsPlayer::destroy(){
    AGILE_LOGV("enter!");
    
    if (mpListener)
        delete mpListener;

    //if (mStreamBuf)
    //    delete mStreamBuf;

    //if (mPlayer)
    //    delete mPlayer;
}

TsPlayer::TsPlayer():
          mbInitialized(false){
    initialize();
}

TsPlayer::~TsPlayer(){
    destroy();
}


int  TsPlayer::GetPlayMode(){
    return 1;
}

int  TsPlayer::SetVideoWindow(int x,int y,int width,int height){
    Parcel request;
    Parcel reply;

    request.writeInt32(MAG_INVOKE_ID_SET_WINDOW_SIZE);
    request.writeInt32(x);
    request.writeInt32(y);
    request.writeInt32(width);
    request.writeInt32(height);
    mPlayer->invoke(request, &reply);

    return 0;
}

int  TsPlayer::VideoShow(void){
    return 0;
}

int  TsPlayer::VideoHide(void){
    return 0;
}

ui32 TsPlayer::convertVideoCodecType(vformat_t vcodec){
    switch(vcodec){
        case VFORMAT_MPEG12:
            return (ui32)OMX_VIDEO_CodingMPEG2;

        case VFORMAT_MPEG4:
            return (ui32)OMX_VIDEO_CodingMPEG4;

        case VFORMAT_H264:
            return (ui32)OMX_VIDEO_CodingAVC;

        case VFORMAT_MJPEG:
            return (ui32)OMX_VIDEO_CodingMJPEG;

        case VFORMAT_REAL:
            return (ui32)OMX_VIDEO_CodingRV;

        default:
            return (ui32)OMX_VIDEO_CodingUnused;
    };
}

ui32 TsPlayer::convertAudioCodecType(aformat_t acodec){
    switch(acodec){
        case FORMAT_MPEG:
            return (ui32)OMX_AUDIO_CodingMP3;

        case FORMAT_AAC:
            return (ui32)OMX_AUDIO_CodingAAC;

        case FORMAT_AC3:
            return (ui32)OMX_AUDIO_CodingAC3;

        case FORMAT_DTS:
            return (ui32)OMX_AUDIO_CodingDTS;

        case FORMAT_DDPlus:
            return (ui32)OMX_AUDIO_CodingDDPlus;

        default:
            return (ui32)OMX_AUDIO_CodingUnused;
    };
}

void TsPlayer::InitVideo(PVIDEO_PARA_T pVideoPara){
    Parcel request;
      
    if (!mbInitialized){
        AGILE_LOGE("The player is not initialized. Quit!");
        return;
    }
    
    request.writeInt32(1);
    request.writeInt32(pVideoPara->pid);
    request.writeInt32(convertVideoCodecType(pVideoPara->vFmt));
    AGILE_LOGV("video track: pid = %d, codec = %d", pVideoPara->pid, pVideoPara->vFmt);
    
    mPlayer->setParameter(idsVideo_TS_pidlist, request);
}

void TsPlayer::InitAudio(PAUDIO_PARA_T pAudioPara){
    Parcel request;
    
    ui32 i = 0;
    ui32 num = 0;
  
    if (!mbInitialized){
        AGILE_LOGE("The player is not initialized. Quit!");
        return;
    }
    
    while(pAudioPara[i].pid){
        ++i;
    }

    num = i;

    if (num > 32){
        AGILE_LOGE("the number(%d) exceeds 32 and set to 32", num);
        num = 32;
    }else{
        AGILE_LOGD("total number is %d", num);
    }
    
    request.writeInt32(num);

    for (i = 0; i < num; i++){
        request.writeInt32(pAudioPara[i].pid);
        request.writeInt32(convertAudioCodecType(pAudioPara[i].aFmt));
        AGILE_LOGV("audio track: pid = %d, codec = %d", pAudioPara[i].pid, pAudioPara[i].aFmt);
    }
    mPlayer->setParameter(idsAudio_TS_pidlist, request);
}

bool TsPlayer::StartPlay(){
    _status_t ret;

    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }
    
    ret = mPlayer->prepare();
    if (ret == MAG_NO_ERROR){
        mPlayer->start();
    }else{
        AGILE_LOGE("failed to do the preparation, ret = 0x%x", ret);
        mbError = true;
        destroy();
        return false;
    }    
    return true;
}

int  TsPlayer::WriteData(unsigned char* pBuffer, unsigned int nSize){
    int w = 0;
      
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return 0;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    //AGILE_LOGV("write %d", nSize);
    
    StreamBuffer *sb = static_cast<StreamBuffer *>(GET_POINTER(mStreamBuf));
    if (nSize != 0){
        w = sb->WriteData(pBuffer, nSize, true);
    }else{
        /*if nSize == 0, to notify the EOS*/
        sb->getUser()->issueCommand(IStreamBufferUser::EOS, true);
    }
    return w;
}

bool TsPlayer::Pause(){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    
    mPlayer->pause();
    return true;
}

bool TsPlayer::Resume(){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    
    mPlayer->start();
    return true;
}

bool TsPlayer::Fast(){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    
    mPlayer->fast(-1);
    return true;
}

bool TsPlayer::StopFast(){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    
    mPlayer->stop();
    mPlayer->start();
    return true;
}

bool TsPlayer::Stop(){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    mPlayer->stop();
    return true;
}

bool TsPlayer::Seek(){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    
    mPlayer->flush();
    return true;
}

bool TsPlayer::SetVolume(int volume){
    AGILE_LOGV("enter");   
    if (!mbInitialized){
        AGILE_LOGE("the player is not initialized, quit!");
        return false;
    }

    if (mbError){
        AGILE_LOGE("it is in error state, quit!");
        return 0;
    }
    
    mPlayer->setVolume(volume, volume);
    return true;
}

int  TsPlayer::GetVolume(){
    return 0;
}

bool TsPlayer::SetRatio(int nRatio){
    return true;
}

int  TsPlayer::GetAudioBalance(){
    return 0;
}

bool TsPlayer::SetAudioBalance(int nAudioBalance){
    return true;
}

void TsPlayer::GetVideoPixels(int& width, int& height){

}

bool TsPlayer::IsSoftFit(){
    return true;
}

void TsPlayer::SetEPGSize(int w, int h){

}

void TsPlayer::SetSurface(Surface* pSurface){

}

void TsPlayer::SwitchAudioTrack(int pid){

}

void TsPlayer::SwitchSubtitle(int pid){

}

void TsPlayer::SetProperty(int nType, int nSub, int nValue){

}

long TsPlayer::GetCurrentPlayTime(){
    return 0;
}

void TsPlayer::leaveChannel(){

}

void TsPlayer::playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *hander){

}

TsPlayerListener::TsPlayerListener(TsPlayer *obj){
    mHost = obj;
}

void TsPlayerListener::notify(int msg, int ext1, int ext2){
    AGILE_LOGD("get message: %d(ext1:%d, ext2:%d)", msg, ext1, ext2);
    switch (msg){
        case MEDIA_INFO:
            if (ext1 == MEDIA_INFO_PLAY_COMPLETE){
                mHost->Stop();
            }
            break;

        default:
            break;
    }
}

