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

TsPlayerDriver::TsPlayerDriver(){
    mpPlayer = new MagPlayer();
}

TsPlayerDriver::~TsPlayerDriver(){

}


int  TsPlayerDriver::GetPlayMode(){

}

int  TsPlayerDriver::SetVideoWindow(int x,int y,int width,int height){

}

int  TsPlayerDriver::VideoShow(void){

}

int  TsPlayerDriver::VideoHide(void){

}

void TsPlayerDriver::InitVideo(PVIDEO_PARA_T pVideoPara){

}

void TsPlayerDriver::InitAudio(PAUDIO_PARA_T pAudioPara){

}

bool TsPlayerDriver::StartPlay(){

}

int  TsPlayerDriver::WriteData(unsigned char* pBuffer, unsigned int nSize){

}

bool TsPlayerDriver::Pause(){

}

bool TsPlayerDriver::Resume(){

}

bool TsPlayerDriver::Fast(){

}

bool TsPlayerDriver::StopFast(){

}

bool TsPlayerDriver::Stop(){

}

bool TsPlayerDriver::Seek(){

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

void TsPlayerDriver::playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *hander){

}



























