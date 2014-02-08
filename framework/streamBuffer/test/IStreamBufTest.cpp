#include "IStreamBufTest.h"

#include <binder/Parcel.h>

enum {
    // IStreamBuffer
    SET_STREAM_BUFFER = IBinder::FIRST_CALL_TRANSACTION,
    START
};

struct BpSBTestService : public BpInterface<ISBTestService> {
    BpSBTestService(const sp<IBinder> &impl)
        : BpInterface<ISBTestService>(impl) {
    }

    virtual void setStreamBuffer(const sp<IStreamBuffer> &buffer) {
        Parcel data, reply;
        data.writeInterfaceToken(ISBTestService::getInterfaceDescriptor());
        data.writeStrongBinder(buffer->asBinder());
        remote()->transact(SET_STREAM_BUFFER, data, &reply/*, IBinder::FLAG_ONEWAY*/);
    }

    virtual void start(){
        Parcel data, reply;
        data.writeInterfaceToken(ISBTestService::getInterfaceDescriptor());

        remote()->transact(START, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(SBTestService, "android.test.ISBTestService");

status_t BnSBTestService::onTransact(
        uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    switch (code) {
        case SET_STREAM_BUFFER:
        {
            CHECK_INTERFACE(ISBTestService, data, reply);
            setStreamBuffer(interface_cast<IStreamBuffer>(data.readStrongBinder()));
            break;
        }

        case START:
        {
            CHECK_INTERFACE(ISBTestService, data, reply);
            
            start();
            break;
        }
        
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return OK;
}

