#include <stdint.h>
#include <sys/types.h>
#include <utils/Errors.h>  // for status_t
#include <utils/String8.h>

#include "IMagPlayerService.h"

enum {
    CREATE = IBinder::FIRST_CALL_TRANSACTION,
};

class BpMagPlayerService: public BpInterface<IMagPlayerService>
{
public:
    BpMagPlayerService(const sp<IBinder>& impl)
        : BpInterface<IMagPlayerService>(impl)
    {
    }

    virtual sp<IMagPlayerClient> create(
            pid_t pid, const sp<IMagPlayerNotifier>& client) {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeStrongBinder(client->asBinder());

        remote()->transact(CREATE, data, &reply);
        return interface_cast<IMagPlayerClient>(reply.readStrongBinder());
    }
};

IMPLEMENT_META_INTERFACE(MagPlayerService, "android.media.IMagPlayerService");

status_t BnMagPlayerService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case CREATE: {
            CHECK_INTERFACE(IMagPlayerService, data, reply);
            pid_t pid = data.readInt32();
            sp<IMagPlayerNotifier> client =
                interface_cast<IMagPlayerNotifier>(data.readStrongBinder());
            sp<IMagPlayerClient> player = create(pid, client);
            reply->writeStrongBinder(player->asBinder());
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

