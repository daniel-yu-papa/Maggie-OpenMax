#include "MagPlayerService.h"


void MagPlayerService::instantiate() {
    defaultServiceManager()->addService(String16("mag.player"), new MagPlayerService());
}

MagPlayerService::MagPlayerService()
{
    AGILE_LOGV("MagPlayerService created");
    mNextConnId = 1;
}

MagPlayerService::~MagPlayerService()
{
    AGILE_LOGV("MagPlayerService destroyed");
}

sp<IMagPlayerClient> MagPlayerService::create(pid_t pid, const sp<IMagPlayerClient>& client)
{
    i32 connId = android_atomic_inc(&mNextConnId);

    sp<Client> c = new Client(
            this, pid, connId, client,
            IPCThreadState::self()->getCallingUid());

    AGILE_LOGV("Create new client(%d) from pid %d, uid %d, ", connId, pid,
                IPCThreadState::self()->getCallingUid());

    wp<Client> w = c;
    {
        Mutex::Autolock lock(mLock);
        mClients.add(w);
    }
    return c;
}

void MagPlayerService::removeClient(wp<Client> client)
{
    Mutex::Autolock lock(mLock);
    mClients.remove(client);
}

/*************************************/
/*     MagPlayerService::Client Implementation    */
/*************************************/
MagPlayerService::Client::Client(
        const sp<MagPlayerService>& service, pid_t pid,
        i32 connId, const sp<IMagPlayerClient>& client,
        uid_t uid)
{
    AGILE_LOGV("Client(%d) constructor", connId);
    mPid = pid;
    mConnId = connId;
    mService = service;
    mClient = client;
    mLoop = false;
    mUID = uid;
}

MagPlayerService::Client::~Client()
{
    AGILE_LOGV("Client(%d) destructor pid = %d", mConnId, mPid);
    wp<Client> client(this);
    disconnect();
    mService->removeClient(client);
}

void MagPlayerService::Client::disconnect()
{
    AGILE_LOGV("disconnect(%d) from pid %d", mConnId, mPid);
    
    mClient.clear();

    if (NULL != mPlayer){
        mPlayer->setNotifyCallback(0, 0);
        delete mPlayer;
        mPlayer = NULL;
    }

    IPCThreadState::self()->flushCommands();
}

MagPlayerDriver* MagPlayerService::Client::createPlayer()
{
    // determine if we have the right player type
    MagPlayerDriver *p = mPlayer;
    
    if (p == NULL) {
        p = new MagPlayerDriver(this, notify);

        if (p != NULL) {
            p->setUID(mUID);
        }else{
            AGILE_LOGE("Failed to create MagPlayerDriver!!!");
        }
    }
    return p;
}

_status_t MagPlayerService::Client::setDataSource(const char *url)
{
    _status_t status;
    
    AGILE_LOGV("setDataSource(%s)", url);
    if (url == NULL)
        return MAG_UNKNOWN_ERROR;
 
    MagPlayerDriver *p = createPlayer();
    if (p == NULL){
        status = MAG_NO_INIT;
    }else{
        status = p->setDataSource(url);
        if (status == MAG_OK){
            mPlayer = p;
        }else{
            AGILE_LOGE("error: %d", status);
        }  
    }
    return status;
}

_status_t MagPlayerService::Client::setDataSource(int fd, int64_t offset, int64_t length)
{
    AGILE_LOGV("setDataSource fd=%d, offset=%lld, length=%lld", fd, offset, length);
    struct stat sb;
    int ret = fstat(fd, &sb);
    if (ret != 0) {
        AGILE_LOGE("fstat(%d) failed: %d, %s", fd, ret, strerror(errno));
        return MAG_UNKNOWN_ERROR;
    }

    AGILE_LOGV("st_dev  = %llu", sb.st_dev);
    AGILE_LOGV("st_mode = %u", sb.st_mode);
    AGILE_LOGV("st_uid  = %lu", sb.st_uid);
    AGILE_LOGV("st_gid  = %lu", sb.st_gid);
    AGILE_LOGV("st_size = %llu", sb.st_size);

    if (offset >= sb.st_size) {
        AGILE_LOGE("offset error");
        ::close(fd);
        return MAG_UNKNOWN_ERROR;
    }
    if (offset + length > sb.st_size) {
        length = sb.st_size - offset;
        AGILE_LOGV("calculated length = %lld", length);
    }

    _status_t status;
    MagPlayerDriver *p = createPlayer();
    if (p == NULL){
        status = MAG_NO_INIT;
    }else{
        status = p->setDataSource(fd, offset, length);
        if (status == MAG_OK){
            mPlayer = p;
        }else{
            AGILE_LOGE("error: %d", status);
        }
    }
    return status;
}

_status_t MagPlayerService::Client::setDataSource(const sp<IStreamBuffer> &source) {
    _status_t status;
    MagPlayerDriver *p = createPlayer();
    if (p == NULL){
        status = MAG_NO_INIT;
    }else{
        status = p->setDataSource(source);
        if (status == MAG_OK){
            mPlayer = p;
        }else{
            AGILE_LOGE("error: %d", status);
        }
    }
    return status;
}

_status_t MagPlayerService::Client::setVideoSurfaceTexture(
        const sp<ISurfaceTexture>& surfaceTexture)
{
    return MAG_OK;
}

_status_t MagPlayerService::Client::invoke(const Parcel& request,
                                            Parcel *reply)
{
    MagPlayerDriver *p = getPlayer();
    if (p == NULL) return MAG_UNKNOWN_ERROR;
    return p->invoke(request, reply);
}

_status_t MagPlayerService::Client::prepare()
{
    AGILE_LOGV("[%d] prepare", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    _status_t ret = p->prepare();
    return ret;
}

_status_t MagPlayerService::Client::prepareAsync()
{
    AGILE_LOGV("[%d] prepareAsync", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    _status_t ret = p->prepareAsync();
    return ret;
}

_status_t MagPlayerService::Client::start()
{
    AGILE_LOGV("[%d] start", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->start();
}

_status_t MagPlayerService::Client::stop()
{
    AGILE_LOGV("[%d] stop", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->stop();
}

_status_t MagPlayerService::Client::pause()
{
    AGILE_LOGV("[%d] pause", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->pause();
}

_status_t MagPlayerService::Client::isPlaying(bool* state)
{
    *state = false;
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    *state = p->isPlaying();
    AGILE_LOGV("[%d] isPlaying: %d", mConnId, *state);
    return MAG_NO_ERROR;
}

_status_t MagPlayerService::Client::getCurrentPosition(int *msec)
{
    AGILE_LOGV("getCurrentPosition");
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    _status_t ret = p->getCurrentPosition(msec);
    if (ret == MAG_NO_ERROR) {
        AGILE_LOGV("[%d] getCurrentPosition = %d", mConnId, *msec);
    } else {
        AGILE_LOGE("getCurrentPosition returned %d", ret);
    }
    return ret;
}

_status_t MagPlayerService::Client::getDuration(int *msec)
{
    AGILE_LOGV("getDuration");
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    _status_t ret = p->getDuration(msec);
    if (ret == MAG_NO_ERROR) {
        AGILE_LOGV("[%d] getDuration = %d", mConnId, *msec);
    } else {
        AGILE_LOGV("getDuration returned %d", ret);
    }
    return ret;
}

_status_t MagPlayerService::Client::seekTo(int msec)
{
    AGILE_LOGV("[%d] seekTo(%d)", mConnId, msec);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->seekTo(msec);
}

_status_t MagPlayerService::Client::flush()
{
    AGILE_LOGV("[%d] flush()", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->flush();
}

_status_t MagPlayerService::Client::reset()
{
    AGILE_LOGV("[%d] reset", mConnId);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->reset();
}

_status_t MagPlayerService::Client::setVolume(float leftVolume, float rightVolume)
{
    AGILE_LOGV("[%d] setVolume(L:%f - R:%f)", mConnId, leftVolume, rightVolume);
    MagPlayerDriver *p = getPlayer();
    if (p != 0) return p->setVolume(leftVolume, rightVolume);
    return MAG_NO_ERROR;
}

_status_t MagPlayerService::Client::setParameter(int key, const Parcel &request) {
    AGILE_LOGV("[%d] setParameter(%d)", mConnId, key);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->setParameter(key, request);
}

_status_t MagPlayerService::Client::getParameter(int key, Parcel *reply) {
    AGILE_LOGV("[%d] getParameter(%d)", mConnId, key);
    MagPlayerDriver *p = getPlayer();
    if (p == 0) return MAG_UNKNOWN_ERROR;
    return p->getParameter(key, reply);
}

void MagPlayerService::Client::notify(
        void* cookie, int msg, int ext1, int ext2)
{
    Client* client = static_cast<Client*>(cookie);
    if (client == NULL) {
        return;
    }

    sp<IMagPlayerClient> c;
    {
        Mutex::Autolock l(client->mLock);
        c = client->mClient;
    }

    if (c != NULL) {
        AGILE_LOGV("[%d] notify (%p, %d, %d, %d)", client->mConnId, cookie, msg, ext1, ext2);
        c->notify(msg, ext1, ext2);
    }
}

