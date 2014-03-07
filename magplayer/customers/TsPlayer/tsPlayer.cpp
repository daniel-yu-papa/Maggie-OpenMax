#include "tsPlayerDriver.h"

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
    mPlayer = new MagPlayerClient();

    if (NULL != GET_POINTER(mPlayer)){
        mStreamBuf = new StreamBuffer();

        if (NULL == GET_POINTER(mStreamBuf)){
            AGILE_LOG_FATAL("failed to create StreamBuffer object!");
            return;
        }

        Parcel data;
        data.writeInt32(MediaTypeTS);
        mPlayer->setParameters(idsMediaType, data);

        mPlayer->setDataSource(mStreamBuf);
        mState = TSP_INITIALIZED;
        
        Mag_CreateEventGroup(&mPrepareEvtGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&mPrepareDoneEvt, 0))
            Mag_AddEventGroup(mPrepareEvtGroup, mPrepareDoneEvt);
        if (MAG_ErrNone == Mag_CreateEvent(&mPrepareErrorEvt, 0))
            Mag_AddEventGroup(mPrepareEvtGroup, mPrepareErrorEvt);
    }else{
        AGILE_LOG_FATAL("failed to create MagPlayerClient object!");
    }
}

TsPlayer::TsPlayer():
    mState(TSP_IDLE){
    initialize();
}

TsPlayer::~TsPlayer(){
    Mag_DestroyEvent(mPrepareDoneEvt);
    Mag_DestroyEvent(mPrepareErrorEvt);
    Mag_DestroyEventGroup(mPrepareEvtGroup);
    delete mPlayer;
}


int  TsPlayer::GetPlayMode(){
    return 1;
}

int  TsPlayer::SetVideoWindow(int x,int y,int width,int height){
    Parcel request;
    Parcel reply;

    if ((mState == TSP_IDLE) ||
        (mState == TSP_ERROR)){
        return 1;
    }
    
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

    if (mState == TSP_INITIALIZED){
        request.writeInt32(1);
        request.writeInt32(pVideoPara.pid);
        request.writeInt32(convertVideoCodecType(pVideoPara->vFmt));
        
        mPlayer->setParameters(idsVideo_TS_pidlist, request);
    }
}

void TsPlayer::InitAudio(PAUDIO_PARA_T pAudioPara){
    Parcel request;
    
    ui32 i = 0;
    ui32 num = 0;

    if (mState == TSP_INITIALIZED){
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
        }
        mPlayer->setParameters(idsAudio_TS_pidlist, request);
    }
}

bool TsPlayer::StartPlay(){
    if (mState == TSP_INITIALIZED){
        mPlayer->setPrepareCompleteListener(PrepareCompleteEvtListener, static_cast<void *>(this));
        Prepare();

        if (TSP_PREPARED == mState){
            mState = TSP_RUNNING;
            mPlayer->start();
            return true;
        }else{
            AGILE_LOGE("the state[%d] is wrong", mState);
            return false;
        }
    }
    return false;
}

int  TsPlayer::WriteData(unsigned char* pBuffer, unsigned int nSize){
    if ((TSP_RUNNING == mState) ||
        (TSP_FASTING == mState))
        mStreamBuf->WriteData(pBuffer, nSize, true);

    return 0;
}

bool TsPlayer::Pause(){
    if (TSP_RUNNING == mState){
        mState = TSP_PAUSED;
        mPlayer->pause();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::Resume(){
    if (TSP_PAUSED == mState){
        mState = TSP_RUNNING;
        mPlayer->resume();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::Fast(){
    if ((TSP_PREPARED == mState) ||
        (TSP_RUNNING  == mState)){
        mState = TSP_FASTING;
        mPlayer->fast();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::StopFast(){
    if (TSP_FASTING == mState){
        mState = TSP_RUNNING;
        mPlayer->stop();
        mPlayer->start();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::Stop(){
    if (TSP_RUNNING == mState){
        mState = TSP_STOPPED;
        mPlayer->stop();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::Seek(){
    if ((TSP_RUNNING == mState) ||
        (TSP_PREPARED == mState) ||
        (TSP_PAUSED == mState){
        mPlayer->setFlushCompleteListener(FlushCompleteEvtListener);
        mSeekBackState = mState;
        mState = TSP_FLUSHING;
        mPlayer->flush();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::SetVolume(int volume){

}

int  TsPlayer::GetVolume(){

}

bool TsPlayer::SetRatio(int nRatio){

}

int  TsPlayer::GetAudioBalance(){

}

bool TsPlayer::SetAudioBalance(int nAudioBalance){

}

void TsPlayer::GetVideoPixels(int& width, int& height){

}

bool TsPlayer::IsSoftFit(){

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

}

void TsPlayer::leaveChannel(){

}

void TsPlayer::Prepare(){
    MagErr_t ret;

    mPlayer->prepareAsync();
    mState = TSP_PREPARING;
    Mag_WaitForEventGroup(mPrepareEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");
}

void TsPlayer::playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *hander){

}

void TsPlayer::PrepareCompleteEvtListener(void *priv){
    TsPlayer *inst = static_cast<TsPlayer *>(priv);

    inst->mState = TSP_PREPARED;
    Mag_SetEvent(inst->mPrepareDoneEvt);
}

void TsPlayer::ErrorEvtListener(void *priv, i32 what, i32 extra){
    TsPlayer *inst = static_cast<TsPlayer *>(priv);
    if (TSP_PREPARING == inst->mState){
        inst->mState = TSP_ERROR;
        Mag_SetEvent(inst->mPrepareErrorEvt);
    }
}

void TsPlayer::FlushCompleteEvtListener(void *priv){
    TsPlayer *inst = static_cast<TsPlayer *>(priv);

    inst->mState = inst->mSeekBackState;
}

