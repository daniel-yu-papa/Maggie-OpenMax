#ifndef __IMAGPLAYER_SERVICE_H__
#define __IMAGPLAYER_SERVICE_H__

#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "IMagPlayerClient.h"
#include "IMagPlayerNotifier.h"

using namespace android;

class IMagPlayerService: public IInterface
{
public:
    DECLARE_META_INTERFACE(MagPlayerService);

    virtual sp<IMagPlayerClient> create(pid_t pid, const sp<IMagPlayerNotifier>& client) = 0;
};

// ----------------------------------------------------------------------------

class BnMagPlayerService: public BnInterface<IMagPlayerService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

#endif
