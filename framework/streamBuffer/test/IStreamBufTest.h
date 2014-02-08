#ifndef __I_STREAM_BUFFER_TEST_H__
#define __I_STREAM_BUFFER_TEST_H__

#include <binder/IInterface.h>

#include "streamBuffer.h"

using namespace android;

struct ISBTestService : public IInterface {
    DECLARE_META_INTERFACE(SBTestService);

    virtual void setStreamBuffer(const sp<IStreamBuffer> &buffer) = 0;
    virtual void start() = 0;
};

struct BnSBTestService : public BnInterface<ISBTestService> {
    virtual status_t onTransact(
            uint32_t code, const Parcel &data, Parcel *reply,
            uint32_t flags = 0);
};

#endif

