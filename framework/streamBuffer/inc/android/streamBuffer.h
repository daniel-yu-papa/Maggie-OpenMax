#ifndef __STREAM_BUFFER_CLIENT_ANDROID_H__
#define __STREAM_BUFFER_CLIENT_ANDROID_H__

#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <binder/MemoryDealer.h>

#include "Mag_list.h"
#include "Mag_pub_type.h"


typedef struct{
    List_t node;
    android::sp<android::IMemory> buffer;
    ui64   pts;
    ui32   flag;
}BufferNode_t;

using namespace android;

struct IStreamBufferUser;

struct IStreamBuffer : public IInterface {
    DECLARE_META_INTERFACE(StreamBuffer);

    virtual void setUser(const sp<IStreamBufferUser> &user) = 0;
    virtual void setBuffers(List_t *bufListHead) = 0;

    virtual void onBufferEmpty(_size_t index, _size_t size) = 0;
    
protected:
    sp<IStreamBufferUser> mUser;
};

struct IStreamBufferUser : public IInterface {
    DECLARE_META_INTERFACE(StreamBufferUser);

    enum Command {
        EOS,
        DISCONTINUITY,
    };

    virtual void onBufferFilled(_size_t index, _size_t size) = 0;
    
    virtual void issueCommand(
            Command cmd, bool synchronous) = 0;
};

struct StreamBuffer_Server : public BnInterface<IStreamBuffer> {
    virtual status_t onTransact(
            uint32_t code, const Parcel &data, Parcel *reply,
            uint32_t flags = 0);
};

struct StreamBufferUser_Server : public BnInterface<IStreamBufferUser> {
    virtual status_t onTransact(
            uint32_t code, const Parcel &data, Parcel *reply,
            uint32_t flags = 0);
};

typedef sp<IStreamBuffer>  streamBuf_t;

#endif
