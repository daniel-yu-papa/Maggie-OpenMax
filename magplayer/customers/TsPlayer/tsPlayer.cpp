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
    mpPlayer = new MagPlayerClient();

    if (NULL != mpPlayer){
        MagMediaType_t t = MediaTypeTS;
        mpPlayer->setParameters(kMediaType, MagParamTypeInt32, (void *)&t);

        mpPlayer->setDataSource(MAG_EXTERNAL_SOURCE_TSMemory);
        mState = TSP_INITIALIZED;
        
        Mag_CreateEventGroup(&mPrepareEvtGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&mPrepareDoneEvt, 0))
            Mag_AddEventGroup(mPrepareEvtGroup, mPrepareDoneEvt);
        if (MAG_ErrNone == Mag_CreateEvent(&mPrepareErrorEvt, 0))
            Mag_AddEventGroup(mPrepareEvtGroup, mPrepareErrorEvt);
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
    delete mpPlayer;
}


int  TsPlayer::GetPlayMode(){
    return 1;
}

int  TsPlayer::SetVideoWindow(int x,int y,int width,int height){
    MagVideoDisplay_t *window = (MagVideoDisplay_t *)mag_malloc(sizeof(MagVideoDisplay_t));
    mpPlayer->setParameters(kVideo_Display, MagParamTypePointer, (void *)window);

    return 0;
}

int  TsPlayer::VideoShow(void){
    
}

int  TsPlayer::VideoHide(void){

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
    MagTsPIDList_t *pid = (MagTsPIDList_t *)mag_mallocz(sizeof(PVIDEO_PARA_T));
    
    pid->pidTable[0] = pVideoPara.pid;
    pid->num         = 1;
    pid->codec[0]    = convertVideoCodecType(pVideoPara->vFmt);
    mpPlayer->setParameters(kVideo_TS_pidlist, MagParamTypePointer, (void *)pid);
}

void TsPlayer::InitAudio(PAUDIO_PARA_T pAudioPara){
    MagTsPIDList_t *pid = (MagTsPIDList_t *)mag_mallocz(sizeof(PVIDEO_PARA_T));
    ui32 i = 0;
    
    while(pAudioPara[i].pid){
        pid->pidTable[i] = pAudioPara[i].pid;
        pid->codec[i]    = convertAudioCodecType(pAudioPara[i].aFmt);
        ++i;
    }
    pid->num = i;
    mpPlayer->setParameters(kAudio_TS_pidlist, MagParamTypePointer, (void *)pid);
}

bool TsPlayer::StartPlay(){
    mpPlayer->setPrepareCompleteListener(PrepareCompleteEvtListener, static_cast<void *>(this));
    Prepare();

    if (TSP_PREPARED == mState){
        mState = TSP_RUNNING;
        mpPlayer->start();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

int  TsPlayer::WriteData(unsigned char* pBuffer, unsigned int nSize){

}

bool TsPlayer::Pause(){
    if (TSP_RUNNING == mState){
        mState = TSP_PAUSED;
        mpPlayer->pause();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::Resume(){
    if (TSP_PAUSED == mState){
        mState = TSP_RUNNING;
        mpPlayer->resume();
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
        mpPlayer->fast();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::StopFast(){
    if (TSP_FASTING == mState){
        mState = TSP_RUNNING;
        mpPlayer->stop();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayer::Stop(){
    if (TSP_RUNNING == mState){
        mState = TSP_STOPPED;
        mpPlayer->stop();
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
        mpPlayer->setFlushCompleteListener(FlushCompleteEvtListener);
        mSeekBackState = mState;
        mState = TSP_FLUSHING;
        mpPlayer->flush();
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
    
    mpPlayer->prepareAsync();
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

























