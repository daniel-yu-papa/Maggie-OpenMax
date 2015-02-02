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

#include "MagStreamBuffer.h"

#include <binder/Parcel.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-StreamBuffer"


enum {
    ON_BUFFER_FILLED = IBinder::FIRST_CALL_TRANSACTION,
    ISSUE_COMMAND,
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

        remote()->transact(ON_BUFFER_FILLED, data, &reply);
    }

    virtual void issueCommand(
            Command cmd, bool synchronous) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBufferUser::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(cmd));
        data.writeInt32(static_cast<int32_t>(synchronous));

        remote()->transact(ISSUE_COMMAND, data, &reply);
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

