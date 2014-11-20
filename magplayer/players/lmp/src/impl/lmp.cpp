#include "lmp_priv.h"
#include "MagParameters.h"
#include "MagEventType.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "DonglePlayer"

enum{
    DONGLE_ERROR_WHAT_UNKNOWN = 0,
    DONGLE_ERROR_WHAT_DEMUXER,
    DONGLE_ERROR_WHAT_DATASOURCE,
    DONGLE_ERROR_WHAT_VIDEO_PIPELINE,
    DONGLE_ERROR_WHAT_AUDIO_PIPELINE,
    DONGLE_ERROR_WHAT_COMMAND,
};

MAG_SINGLETON_STATIC_INSTANCE(DonglePlayer)

LinuxMediaPlayer* GetMediaPlayer()
{
    AGILE_LOGD("enter");
    
    DonglePlayer& inst = DonglePlayer::getInstance();
    
    return dynamic_cast<LinuxMediaPlayer *>(&inst);
}


void DonglePlayer::eventNotify(void* cookie, int msg, int ext1, int ext2){
    DonglePlayer *pObject = static_cast<DonglePlayer *>(cookie);
    AGILE_LOGI("get message: %d (ext1:%d, ext2:%d)", msg, ext1, ext2);
    if (pObject->mAppEventCallback == NULL){
        AGILE_LOGE("callback is not registered!!");
        return;
    }

    if (msg == MEDIA_ERROR){
        if (ext1 == DONGLE_ERROR_WHAT_DEMUXER){
            AGILE_LOGI("send out stream_invalid event!");
            pObject->mAppEventCallback(LMP_PLAYER_EVT_STREAM_INVALID, pObject->mAppHandler, 0, 0);
        }else{
            AGILE_LOGI("send out playback_error event!");
            pObject->mAppEventCallback(LMP_PLAYER_EVT_PLAYBACK_ERROR, pObject->mAppHandler, 0, 0);
        }
    }else if (msg == MEDIA_INFO){
        if (ext1 == MEDIA_INFO_PLAY_COMPLETE){
            AGILE_LOGI("send out playback_complete event!");
            pObject->mAppEventCallback(LMP_PLAYER_EVT_PLAYBACK_COMPLETE, pObject->mAppHandler, 0, 0);
        } else if (ext1 == MEDIA_INFO_BUFFERING_REPORT){
            AGILE_LOGI("send out buffer status report event: %d%%!", ext2);
            pObject->mAppEventCallback(LMP_PLAYER_EVT_BUFFER_STATUS, pObject->mAppHandler, ext2, 0);
        }
    }else if(msg == MEDIA_SEEK_COMPLETE){
        AGILE_LOGI("send out seek_complete event!");
        pObject->mAppEventCallback(LMP_PLAYER_EVT_SEEK_COMPLETE, pObject->mAppHandler, 0, 0);
    }
}

void DonglePlayer::initialize(){
    AGILE_LOGV("enter!");

    mpMediaPlayer = new MagPlayerDriver(static_cast<void *>(this), DonglePlayer::eventNotify);
    if (mpMediaPlayer){
        mbInitialized = true;
    }else{
        AGILE_LOGE("failed to create Media Player!");
    }
}

void DonglePlayer::destroy(){
    AGILE_LOGV("enter!");
    
    if (mpMediaPlayer){
        delete mpMediaPlayer;
        mpMediaPlayer = NULL;
    }

    AGILE_LOGV("exit!");
    AGILE_LOG_DESTROY();
}

DonglePlayer::DonglePlayer():
          mbInitialized(false),
          mbError(false),
          mAppEventCallback(NULL),
          mAppHandler(NULL){
    initialize();
    mpMediaPlayer->getVersion();
}

DonglePlayer::~DonglePlayer(){
    destroy();
}

int DonglePlayer::setDataSource(const char *url){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setDataSource(url);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::setDataSource(unsigned int fd, signed long long offset, signed long long length){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setDataSource(fd, offset, length);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::prepare(){
    if ((mbInitialized) && (!mbError)){
        /*No need data source and using ffmpeg as the datasource + demuxer*/
        int hasDataSource = 0;
        mpMediaPlayer->setParameter(PARAM_KEY_CP_AVAIL, static_cast<void *>(&hasDataSource));
        return (int)mpMediaPlayer->prepare();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::prepareAsync(){
    if ((mbInitialized) && (!mbError)){
        /*No need data source and using ffmpeg as the datasource + demuxer*/
        int hasDataSource = 0;
        mpMediaPlayer->setParameter(PARAM_KEY_CP_AVAIL, static_cast<void *>(&hasDataSource));
        return (int)mpMediaPlayer->prepareAsync();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::start(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->start();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::stop(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->stop();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::pause(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->pause();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

bool DonglePlayer::isPlaying(){
    if ((mbInitialized) && (!mbError)){
        return mpMediaPlayer->isPlaying();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return false;
    }
}

int DonglePlayer::seekTo(int msec){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->seekTo(msec);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::flush(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->flush();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::fast(int speed){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->fast(speed);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::getCurrentPosition(int* msec){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->getCurrentPosition(msec);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::getDuration(int* msec){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->getDuration(msec);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::reset(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->reset();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::setVolume(float leftVolume, float rightVolume){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setVolume(leftVolume, rightVolume);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::setParameter(int key, void *request){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setParameter(key, request);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::getParameter(int key, void **reply){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->getParameter(key, reply);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::invoke(const unsigned int methodID, const void *request, void *reply){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->invoke(methodID, request, reply);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return LMP_INVALID_OPERATION;
    }
}

int DonglePlayer::registerEventCallback(lmp_event_callback_t cb, void *handler){
    mAppEventCallback = cb;
    mAppHandler       = handler;
}


