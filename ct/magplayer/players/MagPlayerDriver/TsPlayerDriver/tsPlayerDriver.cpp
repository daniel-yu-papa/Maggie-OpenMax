#include "tsPlayerDriver.h"

MAG_SINGLETON_STATIC_INSTANCE(TsPlayerDriver)

CTC_MediaProcessor* GetMediaProcessor()
{
    AGILE_LOGD("enter");
    
    TsPlayerDriver& inst = TsPlayerDriver::getInstance();
    
    return dynamic_cast<CTC_MediaProcessor *>(&inst);
}

int GetMediaProcessorVersion()
{
    AGILE_LOGD("enter");    
    return 2; //uses the interfaces after and including SwitchAudioTrack()
    //return 1; //uses the interfaces before SwitchAudioTrack()
}

TsPlayerDriver::TsPlayerDriver():
    mState(TSP_IDLE){
    mpPlayer = new MagPlayer();

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

TsPlayerDriver::~TsPlayerDriver(){
    Mag_DestroyEvent(mPrepareDoneEvt);
    Mag_DestroyEvent(mPrepareErrorEvt);
    Mag_DestroyEventGroup(mPrepareEvtGroup);
    delete mpPlayer;
}


int  TsPlayerDriver::GetPlayMode(){
    return 1;
}

int  TsPlayerDriver::SetVideoWindow(int x,int y,int width,int height){
    MagVideoDisplay_t *window = (MagVideoDisplay_t *)mag_malloc(sizeof(MagVideoDisplay_t));
    mpPlayer->setParameters(kVideo_Display, MagParamTypePointer, (void *)window);

    return 0;
}

int  TsPlayerDriver::VideoShow(void){
    
}

int  TsPlayerDriver::VideoHide(void){

}

ui32 TsPlayerDriver::convertVideoCodecType(vformat_t vcodec){
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

ui32 TsPlayerDriver::convertAudioCodecType(aformat_t acodec){
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

void TsPlayerDriver::InitVideo(PVIDEO_PARA_T pVideoPara){
    MagTsPIDList_t *pid = (MagTsPIDList_t *)mag_mallocz(sizeof(PVIDEO_PARA_T));
    
    pid->pidTable[0] = pVideoPara.pid;
    pid->num         = 1;
    pid->codec[0]    = convertVideoCodecType(pVideoPara->vFmt);
    mpPlayer->setParameters(kVideo_TS_pidlist, MagParamTypePointer, (void *)pid);
}

void TsPlayerDriver::InitAudio(PAUDIO_PARA_T pAudioPara){
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

bool TsPlayerDriver::StartPlay(){
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

int  TsPlayerDriver::WriteData(unsigned char* pBuffer, unsigned int nSize){

}

bool TsPlayerDriver::Pause(){
    if (TSP_RUNNING == mState){
        mState = TSP_PAUSED;
        mpPlayer->pause();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayerDriver::Resume(){
    if (TSP_PAUSED == mState){
        mState = TSP_RUNNING;
        mpPlayer->resume();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayerDriver::Fast(){
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

bool TsPlayerDriver::StopFast(){
    if (TSP_FASTING == mState){
        mState = TSP_RUNNING;
        mpPlayer->stop();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayerDriver::Stop(){
    if (TSP_RUNNING == mState){
        mState = TSP_STOPPED;
        mpPlayer->stop();
        return true;
    }else{
        AGILE_LOGE("the state[%d] is wrong", mState);
        return false;
    }
}

bool TsPlayerDriver::Seek(){
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

bool TsPlayerDriver::SetVolume(int volume){

}

int  TsPlayerDriver::GetVolume(){

}

bool TsPlayerDriver::SetRatio(int nRatio){

}

int  TsPlayerDriver::GetAudioBalance(){

}

bool TsPlayerDriver::SetAudioBalance(int nAudioBalance){

}

void TsPlayerDriver::GetVideoPixels(int& width, int& height){

}

bool TsPlayerDriver::IsSoftFit(){

}

void TsPlayerDriver::SetEPGSize(int w, int h){

}

void TsPlayerDriver::SetSurface(Surface* pSurface){

}

void TsPlayerDriver::SwitchAudioTrack(int pid){

}

void TsPlayerDriver::SwitchSubtitle(int pid){

}

void TsPlayerDriver::SetProperty(int nType, int nSub, int nValue){

}

long TsPlayerDriver::GetCurrentPlayTime(){

}

void TsPlayerDriver::leaveChannel(){

}

void TsPlayerDriver::Prepare(){
    MagErr_t ret;
    
    mpPlayer->prepareAsync();
    mState = TSP_PREPARING;
    Mag_WaitForEventGroup(mPrepareEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGD("exit!");
}

void TsPlayerDriver::playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *hander){

}

void TsPlayerDriver::PrepareCompleteEvtListener(void *priv){
    TsPlayerDriver *inst = static_cast<TsPlayerDriver *>(priv);

    inst->mState = TSP_PREPARED;
    Mag_SetEvent(inst->mPrepareDoneEvt);
}

void TsPlayerDriver::ErrorEvtListener(void *priv, i32 what, i32 extra){
    TsPlayerDriver *inst = static_cast<TsPlayerDriver *>(priv);
    if (TSP_PREPARING == inst->mState){
        inst->mState = TSP_ERROR;
        Mag_SetEvent(inst->mPrepareErrorEvt);
    }
}

void TsPlayerDriver::FlushCompleteEvtListener(void *priv){
    TsPlayerDriver *inst = static_cast<TsPlayerDriver *>(priv);

    inst->mState = inst->mSeekBackState;
}

























