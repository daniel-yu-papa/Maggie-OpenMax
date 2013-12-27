#ifndef __IMAGPLAYER_DEATH_NOTIFIER_H__
#define __IMAGPLAYER_DEATH_NOTIFIER_H__


#include <utils/threads.h>
#include <media/IMediaPlayerService.h>
#include <utils/SortedVector.h>

namespace android {

class IMagPlayerDeathNotifier: virtual public RefBase
{
public:
    IMagDeathNotifier() { addObitRecipient(this); }
    virtual ~IMagDeathNotifier() { removeObitRecipient(this); }

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
    static  sp<IMagPlayerService>                 sMediaPlayerService;
    static  sp<DeathNotifier>                       sDeathNotifier;
    static  SortedVector< wp<IMagPlayerDeathNotifier> > sObitRecipients;
};

}; // namespace android

#endif // ANDROID_IMEDIADEATHNOTIFIER_H
#endif
