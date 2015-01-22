#include "mmp_impl.h"
#include "MagParameters.h"
#include "MagEventType.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagPlayerImpl"

enum{
    MMPI_ERROR_WHAT_UNKNOWN = 0,
    MMPI_ERROR_WHAT_DEMUXER,
    MMPI_ERROR_WHAT_DATASOURCE,
    MMPI_ERROR_WHAT_VIDEO_PIPELINE,
    MMPI_ERROR_WHAT_AUDIO_PIPELINE,
    MMPI_ERROR_WHAT_COMMAND,
};

MAG_SINGLETON_STATIC_INSTANCE(MagMediaPlayerImpl)

MagMediaPlayer* GetMediaPlayer()
{
    MagMediaPlayerImpl& inst = MagMediaPlayerImpl::getInstance();
    
    return dynamic_cast<MagMediaPlayer *>(&inst);
}


void MagMediaPlayerImpl::eventNotify(void* cookie, int msg, int ext1, int ext2){
    MagMediaPlayerImpl *pObject = static_cast<MagMediaPlayerImpl *>(cookie);
    AGILE_LOGI("get message: %d (ext1:%d, ext2:%d)", msg, ext1, ext2);
    if (pObject->mAppEventCallback == NULL){
        AGILE_LOGE("callback is not registered!!");
        return;
    }

    if (msg == MEDIA_ERROR){
        if (ext1 == MMPI_ERROR_WHAT_DEMUXER){
            AGILE_LOGI("send out stream_invalid event!");
            pObject->mAppEventCallback(MMP_PLAYER_EVT_STREAM_INVALID, pObject->mAppHandler, 0, 0);
        }else{
            AGILE_LOGI("send out playback_error event!");
            pObject->mAppEventCallback(MMP_PLAYER_EVT_PLAYBACK_ERROR, pObject->mAppHandler, 0, 0);
        }
    }else if (msg == MEDIA_INFO){
        if (ext1 == MEDIA_INFO_PLAY_COMPLETE){
            AGILE_LOGI("send out playback_complete event!");
            pObject->mAppEventCallback(MMP_PLAYER_EVT_PLAYBACK_COMPLETE, pObject->mAppHandler, 0, 0);
        } else if (ext1 == MEDIA_INFO_BUFFERING_REPORT){
            AGILE_LOGI("send out buffer status report event: %d%%!", ext2);
            pObject->mAppEventCallback(MMP_PLAYER_EVT_BUFFER_STATUS, pObject->mAppHandler, ext2, 0);
        }
    }else if(msg == MEDIA_SEEK_COMPLETE){
        AGILE_LOGI("send out seek_complete event!");
        pObject->mAppEventCallback(MMP_PLAYER_EVT_SEEK_COMPLETE, pObject->mAppHandler, 0, 0);
    }else if(msg == MEDIA_PREPARED){
        AGILE_LOGI("send out prepare_complete event!");
        pObject->mAppEventCallback(MMP_PLAYER_EVT_PREPARE_COMPLETE, pObject->mAppHandler, 0, 0);
    }else if(msg == MEDIA_FLUSH_COMPLETE){
        AGILE_LOGI("send out flush_done event!");
        pObject->mAppEventCallback(MMP_PLAYER_EVT_FLUSH_DONE, pObject->mAppHandler, 0, 0);
    }
}

void MagMediaPlayerImpl::initialize(){
    AGILE_LOGV("enter!");

    mpMediaPlayer = new MagPlayerDriver(static_cast<void *>(this), MagMediaPlayerImpl::eventNotify);
    if (mpMediaPlayer){
        mbInitialized = true;
    }else{
        AGILE_LOGE("failed to create Media Player!");
    }
}

void MagMediaPlayerImpl::destroy(){
    AGILE_LOGV("enter!");
    
    if (mpMediaPlayer){
        delete mpMediaPlayer;
        mpMediaPlayer = NULL;
    }

    AGILE_LOGV("exit!");
    AGILE_LOG_DESTROY();
}

MagMediaPlayerImpl::MagMediaPlayerImpl():
          mAppEventCallback(NULL),
          mAppHandler(NULL),
          mbInitialized(false),
          mbError(false){
    AGILE_LOG_CREATE();
    initialize();
    mpMediaPlayer->getVersion();
}

MagMediaPlayerImpl::~MagMediaPlayerImpl(){
    destroy();
}

int MagMediaPlayerImpl::setDataSource(const char *url){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setDataSource(url);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::setDataSource(unsigned int fd, signed long long offset, signed long long length){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setDataSource(fd, offset, length);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::prepare(){
    if ((mbInitialized) && (!mbError)){
        /*No need data source and using ffmpeg as the datasource + demuxer*/
        int hasDataSource = 0;
        mpMediaPlayer->setParameter(PARAM_KEY_CP_AVAIL, static_cast<void *>(&hasDataSource));
        return (int)mpMediaPlayer->prepare();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::prepareAsync(){
    if ((mbInitialized) && (!mbError)){
        /*No need data source and using ffmpeg as the datasource + demuxer*/
        int hasDataSource = 0;
        mpMediaPlayer->setParameter(PARAM_KEY_CP_AVAIL, static_cast<void *>(&hasDataSource));
        return (int)mpMediaPlayer->prepareAsync();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::start(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->start();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::stop(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->stop();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::pause(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->pause();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

bool MagMediaPlayerImpl::isPlaying(){
    if ((mbInitialized) && (!mbError)){
        return mpMediaPlayer->isPlaying();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return false;
    }
}

int MagMediaPlayerImpl::seekTo(int msec){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->seekTo(msec);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::flush(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->flush();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::fast(int speed){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->fast(speed);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::getCurrentPosition(int* msec){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->getCurrentPosition(msec);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::getDuration(int* msec){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->getDuration(msec);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::reset(){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->reset();
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::setVolume(float leftVolume, float rightVolume){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setVolume(leftVolume, rightVolume);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::setParameter(int key, void *request){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->setParameter(key, request);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::getParameter(int key, void **reply){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->getParameter(key, reply);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::invoke(const unsigned int methodID, void *request, void **reply){
    if ((mbInitialized) && (!mbError)){
        return (int)mpMediaPlayer->invoke(methodID, request, reply);
    }else{
        AGILE_LOGE("invalid state to do (mbInitialized: %d, mbError: %d)", mbInitialized, mbError);
        return MMP_INVALID_OPERATION;
    }
}

int MagMediaPlayerImpl::registerEventCallback(mmp_event_callback_t cb, void *handler){
    mAppEventCallback = cb;
    mAppHandler       = handler;

    return 0;
}


