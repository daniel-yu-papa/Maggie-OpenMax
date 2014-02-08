#include "streamBuffer.h"

#include <binder/Parcel.h>

enum {
    ON_BUFFER_FILLED = IBinder::FIRST_CALL_TRANSACTION,
    ISSUE_COMMAND,
    START,
    SETUP,
};

struct BpStreamBufferUser : public BpInterface<IStreamBufferUser> {
    BpStreamBufferUser(const sp<IBinder> &impl)
        : BpInterface<IStreamBufferUser>(impl) {
    }

    virtual void onBufferFilled(_size_t index, _size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBufferUser::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(index));
        data.writeInt32(static_cast<int32_t>(size));

        remote()->transact(ON_BUFFER_FILLED, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual void issueCommand(
            Command cmd, bool synchronous) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBufferUser::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(cmd));
        data.writeInt32(static_cast<int32_t>(synchronous));

        remote()->transact(ISSUE_COMMAND, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(StreamBufferUser, "android.hardware.IStreamBufferUser");

status_t StreamBufferUser_Server::onTransact(
        uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    switch (code) {
        case ON_BUFFER_FILLED:
        {
            CHECK_INTERFACE(IStreamBufferUser, data, reply);
            _size_t index = static_cast<_size_t>(data.readInt32());
            _size_t size = static_cast<_size_t>(data.readInt32());

            onBufferFilled(index, size);
            break;
        }

        case ISSUE_COMMAND:
        {
            CHECK_INTERFACE(IStreamBufferUser, data, reply);
            Command cmd = static_cast<Command>(data.readInt32());

            bool synchronous = static_cast<bool>(data.readInt32());

            issueCommand(cmd, synchronous);
            break;
        }
        
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return OK;
}

