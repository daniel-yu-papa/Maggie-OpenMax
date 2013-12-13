/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <media/IStreamSource.h>

#include <gui/ISurfaceTexture.h>
#include <utils/String8.h>

#include "IMagPlayerClient.h"

namespace android {

enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    SET_DATA_SOURCE_URL,
    SET_DATA_SOURCE_FD,
    SET_DATA_SOURCE_STREAM,
    PREPARE_ASYNC,
    START,
    STOP,
    IS_PLAYING,
    PAUSE,
    SEEK_TO,
    GET_CURRENT_POSITION,
    GET_DURATION,
    RESET,
    SET_LOOPING,
    SET_VOLUME,
    INVOKE,
    SET_VIDEO_SURFACETEXTURE,
    SET_PARAMETER,
    GET_PARAMETER,
};

class BpMagPlayerClient: public BpInterface<IMagPlayerClient>
{
public:
    BpMagPlayerClient(const sp<IBinder>& impl)
        : BpInterface<IMagPlayerClient>(impl)
    {
    }

    // disconnect from media player service
    void disconnect()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
    }

    status_t setDataSource(const char* url,
            const KeyedVector<String8, String8>* headers)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeCString(url);
        if (headers == NULL) {
            data.writeInt32(0);
        } else {
            // serialize the headers
            data.writeInt32(headers->size());
            for (size_t i = 0; i < headers->size(); ++i) {
                data.writeString8(headers->keyAt(i));
                data.writeString8(headers->valueAt(i));
            }
        }
        remote()->transact(SET_DATA_SOURCE_URL, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(int fd, int64_t offset, int64_t length) {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeFileDescriptor(fd);
        data.writeInt64(offset);
        data.writeInt64(length);
        remote()->transact(SET_DATA_SOURCE_FD, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(const sp<IStreamSource> &source) {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeStrongBinder(source->asBinder());
        remote()->transact(SET_DATA_SOURCE_STREAM, data, &reply);
        return reply.readInt32();
    }

    // pass the buffered ISurfaceTexture to the media player service
    status_t setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        sp<IBinder> b(surfaceTexture->asBinder());
        data.writeStrongBinder(b);
        remote()->transact(SET_VIDEO_SURFACETEXTURE, data, &reply);
        return reply.readInt32();
    }

    status_t prepareAsync()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(PREPARE_ASYNC, data, &reply);
        return reply.readInt32();
    }

    status_t start()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(START, data, &reply);
        return reply.readInt32();
    }

    status_t stop()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(STOP, data, &reply);
        return reply.readInt32();
    }

    status_t isPlaying(bool* state)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(IS_PLAYING, data, &reply);
        *state = reply.readInt32();
        return reply.readInt32();
    }

    status_t pause()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(PAUSE, data, &reply);
        return reply.readInt32();
    }

    status_t seekTo(int msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(msec);
        remote()->transact(SEEK_TO, data, &reply);
        return reply.readInt32();
    }

    status_t getCurrentPosition(int* msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(GET_CURRENT_POSITION, data, &reply);
        *msec = reply.readInt32();
        return reply.readInt32();
    }

    status_t getDuration(int* msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(GET_DURATION, data, &reply);
        *msec = reply.readInt32();
        return reply.readInt32();
    }

    status_t reset()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(RESET, data, &reply);
        return reply.readInt32();
    }
    status_t setLooping(int loop)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(loop);
        remote()->transact(SET_LOOPING, data, &reply);
        return reply.readInt32();
    }

    status_t setVolume(float leftVolume, float rightVolume)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeFloat(leftVolume);
        data.writeFloat(rightVolume);
        remote()->transact(SET_VOLUME, data, &reply);
        return reply.readInt32();
    }

    status_t invoke(const Parcel& request, Parcel *reply)
    {
        // Avoid doing any extra copy. The interface descriptor should
        // have been set by MediaPlayer.java.
        return remote()->transact(INVOKE, request, reply);
    }
    
    status_t setParameter(int key, const Parcel& request)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(key);
        if (request.dataSize() > 0) {
            data.appendFrom(const_cast<Parcel *>(&request), 0, request.dataSize());
        }
        remote()->transact(SET_PARAMETER, data, &reply);
        return reply.readInt32();
    }

    status_t getParameter(int key, Parcel *reply)
    {
        Parcel data;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(key);
        return remote()->transact(GET_PARAMETER, data, reply);
    }
};

IMPLEMENT_META_INTERFACE(MagPlayerClient, "android.media.IMagPlayerClient");

// ----------------------------------------------------------------------

status_t BnMagPlayerClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case DISCONNECT: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            disconnect();
            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_URL: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            const char* url = data.readCString();
            KeyedVector<String8, String8> headers;
            int32_t numHeaders = data.readInt32();
            for (int i = 0; i < numHeaders; ++i) {
                String8 key = data.readString8();
                String8 value = data.readString8();
                headers.add(key, value);
            }
            reply->writeInt32(setDataSource(url, numHeaders > 0 ? &headers : NULL));
            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_FD: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int fd = data.readFileDescriptor();
            int64_t offset = data.readInt64();
            int64_t length = data.readInt64();
            reply->writeInt32(setDataSource(fd, offset, length));
            return NO_ERROR;
        }
        case SET_DATA_SOURCE_STREAM: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            sp<IStreamSource> source =
                interface_cast<IStreamSource>(data.readStrongBinder());
            reply->writeInt32(setDataSource(source));
            return NO_ERROR;
        }
        case SET_VIDEO_SURFACETEXTURE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            sp<ISurfaceTexture> surfaceTexture =
                    interface_cast<ISurfaceTexture>(data.readStrongBinder());
            reply->writeInt32(setVideoSurfaceTexture(surfaceTexture));
            return NO_ERROR;
        } break;
        case PREPARE_ASYNC: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(prepareAsync());
            return NO_ERROR;
        } break;
        case START: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(start());
            return NO_ERROR;
        } break;
        case STOP: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(stop());
            return NO_ERROR;
        } break;
        case IS_PLAYING: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            bool state;
            status_t ret = isPlaying(&state);
            reply->writeInt32(state);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case PAUSE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(pause());
            return NO_ERROR;
        } break;
        case SEEK_TO: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(seekTo(data.readInt32()));
            return NO_ERROR;
        } break;
        case GET_CURRENT_POSITION: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int msec;
            status_t ret = getCurrentPosition(&msec);
            reply->writeInt32(msec);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case GET_DURATION: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int msec;
            status_t ret = getDuration(&msec);
            reply->writeInt32(msec);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case RESET: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(reset());
            return NO_ERROR;
        } break;
        case SET_LOOPING: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(setLooping(data.readInt32()));
            return NO_ERROR;
        } break;
        case SET_VOLUME: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            float leftVolume = data.readFloat();
            float rightVolume = data.readFloat();
            reply->writeInt32(setVolume(leftVolume, rightVolume));
            return NO_ERROR;
        } break;
        case INVOKE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            status_t result = invoke(data, reply);
            return result;
        } break;
        case SET_PARAMETER: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int key = data.readInt32();

            Parcel request;
            if (data.dataAvail() > 0) {
                request.appendFrom(
                        const_cast<Parcel *>(&data), data.dataPosition(), data.dataAvail());
            }
            request.setDataPosition(0);
            reply->writeInt32(setParameter(key, request));
            return NO_ERROR;
        } break;
        case GET_PARAMETER: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            return getParameter(data.readInt32(), reply);
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android

