#ifndef __IMAGPLAYER_DEATH_NOTIFIER_H__
#define __IMAGPLAYER_DEATH_NOTIFIER_H__


#include <utils/threads.h>
#include "IMagPlayerService.h"
#include <utils/SortedVector.h>

using namespace android;

class IMagPlayerDeathNotifier: virtual public RefBase
{
public:
    IMagPlayerDeathNotifier() { addObitRecipient(this); }
    virtual ~IMagPlayerDeathNotifier() { removeObitRecipient(this); }

    virtual void died() = 0;
    static const sp<IMagPlayerService>& getMagPlayerService();

private:
    IMagPlayerDeathNotifier &operator=(const IMagPlayerDeathNotifier &);
    IMagPlayerDeathNotifier(const IMagPlayerDeathNotifier &);

    static void addObitRecipient(const wp<IMagPlayerDeathNotifier>& recipient);
    static void removeObitRecipient(const wp<IMagPlayerDeathNotifier>& recipient);

    class DeathNotifier: public IBinder::DeathRecipient
    {
    public:
                DeathNotifier() {}
        virtual ~DeathNotifier();

        virtual void binderDied(const wp<IBinder>& who);
    };

    friend class DeathNotifier;

    static  Mutex                                   sServiceLock;
    static  sp<IMagPlayerService>                   sMagPlayerService;
    static  sp<DeathNotifier>                       sDeathNotifier;
    static  SortedVector< wp<IMagPlayerDeathNotifier> > sObitRecipients;
};

#endif
