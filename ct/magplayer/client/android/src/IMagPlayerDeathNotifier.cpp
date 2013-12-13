#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include "IMagPlayerDeathNotifier.h"

// client singleton for binder interface to services
Mutex IMagPlayerDeathNotifier::sServiceLock;
sp<IMediaPlayerService> IMagPlayerDeathNotifier::sMediaPlayerService;
sp<IMagPlayerDeathNotifier::DeathNotifier> IMagPlayerDeathNotifier::sDeathNotifier;
SortedVector< wp<IMagPlayerDeathNotifier> > IMagPlayerDeathNotifier::sObitRecipients;

// establish binder interface to MediaPlayerService
/*static*/const sp<IMediaPlayerService>&
IMagPlayerDeathNotifier::getMagPlayerService()
{
    AGILE_LOGV("getMediaPlayerService");
    Mutex::Autolock _l(sServiceLock);
    if (sMediaPlayerService == 0) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16("maggie.player"));
            if (binder != 0) {
                break;
            }
            AGILE_LOGW("Maggie player service not published, waiting...");
            usleep(500000); // 0.5 s
        } while (true);

        if (sDeathNotifier == NULL) {
        sDeathNotifier = new DeathNotifier();
    }
    binder->linkToDeath(sDeathNotifier);
    sMediaPlayerService = interface_cast<IMediaPlayerService>(binder);
    }
    
    if (sMediaPlayerService == 0)
        AGILE_LOGE("no maggie player service!?");
    
    return sMediaPlayerService;
}

/*static*/ void
IMagPlayerDeathNotifier::addObitRecipient(const wp<IMagPlayerDeathNotifier>& recipient)
{
    AGILE_LOGV("addObitRecipient");
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.add(recipient);
}

/*static*/ void
IMagPlayerDeathNotifier::removeObitRecipient(const wp<IMagPlayerDeathNotifier>& recipient)
{
    AGILE_LOGV("removeObitRecipient");
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.remove(recipient);
}

void
IMagPlayerDeathNotifier::DeathNotifier::binderDied(const wp<IBinder>& who) {
    AGILE_LOGW("Maggie media server died");

    // Need to do this with the lock held
    SortedVector< wp<IMagPlayerDeathNotifier> > list;
    {
        Mutex::Autolock _l(sServiceLock);
        sMediaPlayerService.clear();
        list = sObitRecipients;
    }

    // Notify application when media server dies.
    // Don't hold the static lock during callback in case app
    // makes a call that needs the lock.
    size_t count = list.size();
    for (size_t iter = 0; iter < count; ++iter) {
        sp<IMagPlayerDeathNotifier> notifier = list[iter].promote();
        if (notifier != 0) {
            notifier->died();
        }
    }
}

IMagPlayerDeathNotifier::DeathNotifier::~DeathNotifier()
{
    AGILE_LOGV("DeathNotifier::~DeathNotifier");
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.clear();
    if (sMediaPlayerService != 0) {
        sMediaPlayerService->asBinder()->unlinkToDeath(this);
    }
}


