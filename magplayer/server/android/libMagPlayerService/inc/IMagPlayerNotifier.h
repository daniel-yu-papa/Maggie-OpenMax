#ifndef __IMAGPLAYER_NOTIFIER_H__
#define __IMAGPLAYER_NOTIFIER_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

class IMagPlayerNotifier: public IInterface
{
public:
    DECLARE_META_INTERFACE(MagPlayerNotifier);

    virtual void notify(int msg, int ext1, int ext2) = 0;
};

// ----------------------------------------------------------------------------

class BnMagPlayerNotifier: public BnInterface<IMagPlayerNotifier>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};


#endif 
