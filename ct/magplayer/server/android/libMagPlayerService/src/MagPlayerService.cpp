#include "MagPlayerService.h"

namespace android {

void MagPlayerService::instantiate() {
    defaultServiceManager()->addService(String16("mag.player"), new MagPlayerService());
}

MagPlayerService::MagPlayerService()
{
    AGILE_LOGV("MagPlayerService created");
    mNextConnId = 1;

    //MediaPlayerFactory::registerBuiltinFactories();
}

MagPlayerService::~MagPlayerService()
{
    AGILE_LOGV("MagPlayerService destroyed");
}

sp<IMagPlayerClient> MagPlayerService::create(pid_t pid, const sp<IMagPlayerClient>& client)
{
    int32_t connId = android_atomic_inc(&mNextConnId);

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
        int32_t connId, const sp<IMagPlayerClient>& client,
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
    // grab local reference and clear main reference to prevent future
    // access to object
    sp<MediaPlayerBase> p;
    {
        Mutex::Autolock l(mLock);
        p = mPlayer;
    }
    mClient.clear();

    mPlayer.clear();

    // clear the notification to prevent callbacks to dead client
    // and reset the player. We assume the player will serialize
    // access to itself if necessary.
    if (p != 0) {
        p->setNotifyCallback(0, 0);
        p->reset();
    }

    //disconnectNativeWindow();

    IPCThreadState::self()->flushCommands();
}

sp<MagPlayer> MagPlayerService::Client::createPlayer()
{
    // determine if we have the right player type
    sp<MagPlayer> p = mPlayer;
    
    if (p == NULL) {
        p = new MagPlayer(this, notify);

        if (p != NULL) {
            p->setUID(mUID);
        }else{
            AGILE_LOGE("Failed to create MagPlayer!!!");
        }
    }
    return p;
}

status_t MagPlayerService::Client::setDataSource(
        const char *url, const KeyedVector<String8, String8> *headers)
{
    status_t status;
    
    AGILE_LOGV("setDataSource(%s)", url);
    if (url == NULL)
        return UNKNOWN_ERROR;

    if (strncmp(url, "content://", 10) == 0) {
        // get a filedescriptor for the content Uri and
        // pass it to the setDataSource(fd) method

        String16 url16(url);
        int fd = android::openContentProviderFile(url16);
        if (fd < 0)
        {
            ALOGE("Couldn't open fd for %s", url);
            return UNKNOWN_ERROR;
        }
        status = setDataSource(fd, 0, 0x7fffffffffLL); // this sets mStatus
        close(fd);
    } else {
        sp<MagPlayer> p = createPlayer();
        if (p == NULL){
            status = NO_INIT;
        }else{
            status = p->setDataSource(url, headers);
            if (status == OK){
                mPlayer = p;
            }else{
                AGILE_LOGE("error: %d", status);
            }  
        }
    }
    return status;
}

status_t MagPlayerService::Client::setDataSource(int fd, int64_t offset, int64_t length)
{
    AGILE_LOGV("setDataSource fd=%d, offset=%lld, length=%lld", fd, offset, length);
    struct stat sb;
    int ret = fstat(fd, &sb);
    if (ret != 0) {
        AGILE_LOGE("fstat(%d) failed: %d, %s", fd, ret, strerror(errno));
        return UNKNOWN_ERROR;
    }

    AGILE_LOGV("st_dev  = %llu", sb.st_dev);
    AGILE_LOGV("st_mode = %u", sb.st_mode);
    AGILE_LOGV("st_uid  = %lu", sb.st_uid);
    AGILE_LOGV("st_gid  = %lu", sb.st_gid);
    AGILE_LOGV("st_size = %llu", sb.st_size);

    if (offset >= sb.st_size) {
        AGILE_LOGE("offset error");
        ::close(fd);
        return UNKNOWN_ERROR;
    }
    if (offset + length > sb.st_size) {
        length = sb.st_size - offset;
        AGILE_LOGV("calculated length = %lld", length);
    }

    status_t status;
    sp<MagPlayer> p = createPlayer();
    if (p == NULL){
        status = NO_INIT;
    }else{
        status = p->setDataSource(fd, offset, length);
        if (status == OK){
            mPlayer = p;
        }else{
            AGILE_LOGE("error: %d", status);
        }
    }
    return status;
}

status_t MagPlayerService::Client::setDataSource(
        const sp<IStreamSource> &source) {
    status_t status;
    sp<MagPlayer> p = createPlayer();
    if (p == NULL){
        status = NO_INIT;
    }else{
        status = p->setDataSource(source);
        if (status == OK){
            mPlayer = p;
        }else{
            AGILE_LOGE("error: %d", status);
        }
    }
    return status;
}

status_t MagPlayerService::Client::setVideoSurfaceTexture(
        const sp<ISurfaceTexture>& surfaceTexture)
{
    return OK;
}

status_t MagPlayerService::Client::invoke(const Parcel& request,
                                            Parcel *reply)
{
    sp<MagPlayer> p = getPlayer();
    if (p == NULL) return UNKNOWN_ERROR;
    return p->invoke(request, reply);
}

status_t MagPlayerService::Client::prepareAsync()
{
    AGILE_LOGV("[%d] prepareAsync", mConnId);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    status_t ret = p->prepareAsync();
    return ret;
}

status_t MagPlayerService::Client::start()
{
    AGILE_LOGV("[%d] start", mConnId);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    p->setLooping(mLoop);
    return p->start();
}

status_t MagPlayerService::Client::stop()
{
    AGILE_LOGV("[%d] stop", mConnId);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    return p->stop();
}

status_t MagPlayerService::Client::pause()
{
    AGILE_LOGV("[%d] pause", mConnId);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    return p->pause();
}

status_t MagPlayerService::Client::isPlaying(bool* state)
{
    *state = false;
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    *state = p->isPlaying();
    AGILE_LOGV("[%d] isPlaying: %d", mConnId, *state);
    return NO_ERROR;
}

status_t MagPlayerService::Client::getCurrentPosition(int *msec)
{
    AGILE_LOGV("getCurrentPosition");
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    status_t ret = p->getCurrentPosition(msec);
    if (ret == NO_ERROR) {
        AGILE_LOGV("[%d] getCurrentPosition = %d", mConnId, *msec);
    } else {
        AGILE_LOGE("getCurrentPosition returned %d", ret);
    }
    return ret;
}

status_t MagPlayerService::Client::getDuration(int *msec)
{
    AGILE_LOGV("getDuration");
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    status_t ret = p->getDuration(msec);
    if (ret == NO_ERROR) {
        AGILE_LOGV("[%d] getDuration = %d", mConnId, *msec);
    } else {
        AGILE_LOGV("getDuration returned %d", ret);
    }
    return ret;
}

status_t MagPlayerService::Client::seekTo(int msec)
{
    AGILE_LOGV("[%d] seekTo(%d)", mConnId, msec);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    return p->seekTo(msec);
}

status_t MagPlayerService::Client::reset()
{
    AGILE_LOGV("[%d] reset", mConnId);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    return p->reset();
}

status_t MagPlayerService::Client::setLooping(int loop)
{
    AGILE_LOGV("[%d] setLooping(%d)", mConnId, loop);
    mLoop = loop;
    sp<MagPlayer> p = getPlayer();
    if (p != 0) return p->setLooping(loop);
    return NO_ERROR;
}

status_t MagPlayerService::Client::setVolume(float leftVolume, float rightVolume)
{
    AGILE_LOGV("[%d] setVolume(L:%f - R:%f)", mConnId, leftVolume, rightVolume);
    sp<MagPlayer> p = getPlayer();
    if (p != 0) return p->setVolume(leftVolume, rightVolume);
    return NO_ERROR;
}

status_t MagPlayerService::Client::setParameter(int key, const Parcel &request) {
    AGILE_LOGV("[%d] setParameter(%d)", mConnId, key);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    return p->setParameter(key, request);
}

status_t MagPlayerService::Client::getParameter(int key, Parcel *reply) {
    AGILE_LOGV("[%d] getParameter(%d)", mConnId, key);
    sp<MagPlayer> p = getPlayer();
    if (p == 0) return UNKNOWN_ERROR;
    return p->getParameter(key, reply);
}

void MagPlayerService::Client::notify(
        void* cookie, int msg, int ext1, int ext2, const Parcel *obj)
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
        c->notify(msg, ext1, ext2, obj);
    }
}

};

