#include "IMagPlayerNotifier.h"

enum {
    NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
};

class BpMagPlayerNotifier: public BpInterface<IMagPlayerNotifier>
{
public:
    BpMagPlayerNotifier(const sp<IBinder>& impl)
        : BpInterface<IMagPlayerNotifier>(impl)
    {
    }

    virtual void notify(int msg, int ext1, int ext2)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerNotifier::getInterfaceDescriptor());
        data.writeInt32(msg);
        data.writeInt32(ext1);
        data.writeInt32(ext2);
        
        remote()->transact(NOTIFY, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(MagPlayerNotifier, "android.media.IMagPlayerNotifier");

// ----------------------------------------------------------------------

status_t BnMagPlayerNotifier::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case NOTIFY: {
            CHECK_INTERFACE(IMagPlayerNotifier, data, reply);
            int msg = data.readInt32();
            int ext1 = data.readInt32();
            int ext2 = data.readInt32();
            Parcel obj;

            notify(msg, ext1, ext2);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

