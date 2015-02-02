/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __STREAM_BUFFER_CLIENT_ANDROID_H__
#define __STREAM_BUFFER_CLIENT_ANDROID_H__

#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <binder/MemoryDealer.h>

#include "Mag_pub_common.h"

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

    enum Type{
        INVALID,
        ES,
        TS,
    };
    
    virtual void setUser(const sp<IStreamBufferUser> &user) = 0;
    virtual void setBuffers(List_t *bufListHead) = 0;

    virtual void onBufferEmpty(_size_t index, _size_t size) = 0;
    virtual Type getType(void) = 0;
    virtual void reset(void) = 0;
    
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
