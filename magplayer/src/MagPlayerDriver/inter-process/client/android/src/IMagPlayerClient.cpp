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

#include <gui/ISurfaceTexture.h>
#include <utils/String8.h>

#include "IMagPlayerClient.h"

enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    SET_DATA_SOURCE_URL,
    SET_DATA_SOURCE_FD,
    SET_DATA_SOURCE_STREAM,
    PREPARE,
    PREPARE_ASYNC,
    START,
    STOP,
    IS_PLAYING,
    PAUSE,
    SEEK_TO,
    FAST,
    GET_CURRENT_POSITION,
    GET_DURATION,
    RESET,
    SET_VOLUME,
    INVOKE,
    SET_VIDEO_SURFACETEXTURE,
    SET_PARAMETER,
    GET_PARAMETER,
    FLUSH,
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

    _status_t setDataSource(const char* url)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeCString(url);
        remote()->transact(SET_DATA_SOURCE_URL, data, &reply);
        return reply.readInt32();
    }

    _status_t setDataSource(i32 fd, i64 offset, i64 length) {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeFileDescriptor(fd);
        data.writeInt64(offset);
        data.writeInt64(length);
        remote()->transact(SET_DATA_SOURCE_FD, data, &reply);
        return reply.readInt32();
    }

    _status_t setDataSource(const sp<IStreamBuffer> &source) {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeStrongBinder(source->asBinder());
        remote()->transact(SET_DATA_SOURCE_STREAM, data, &reply);
        return reply.readInt32();
    }

    // pass the buffered ISurfaceTexture to the media player service
    _status_t setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        sp<IBinder> b(surfaceTexture->asBinder());
        data.writeStrongBinder(b);
        remote()->transact(SET_VIDEO_SURFACETEXTURE, data, &reply);
        return reply.readInt32();
    }

    _status_t prepare()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(PREPARE, data, &reply);
        return reply.readInt32();
    }
    
    _status_t prepareAsync()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(PREPARE_ASYNC, data, &reply);
        return reply.readInt32();
    }

    _status_t start()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(START, data, &reply);
        return reply.readInt32();
    }

    _status_t stop()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(STOP, data, &reply);
        return reply.readInt32();
    }

    _status_t pause()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(PAUSE, data, &reply);
        return reply.readInt32();
    }
    
    _status_t isPlaying(bool* state)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(IS_PLAYING, data, &reply);
        *state = reply.readInt32();
        return reply.readInt32();
    }
    _status_t seekTo(int msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(msec);
        remote()->transact(SEEK_TO, data, &reply);
        return reply.readInt32();
    }

    _status_t fast(int multiple)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(multiple);
        remote()->transact(FAST, data, &reply);
        return reply.readInt32();
    }
        
    _status_t getCurrentPosition(int* msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(GET_CURRENT_POSITION, data, &reply);
        *msec = reply.readInt32();
        return reply.readInt32();
    }

    _status_t getDuration(int* msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(GET_DURATION, data, &reply);
        *msec = reply.readInt32();
        return reply.readInt32();
    }

    _status_t reset()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(RESET, data, &reply);
        return reply.readInt32();
    }

    _status_t setVolume(float leftVolume, float rightVolume)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeFloat(leftVolume);
        data.writeFloat(rightVolume);
        remote()->transact(SET_VOLUME, data, &reply);
        return reply.readInt32();
    }
    _status_t setParameter(int key, const Parcel& request)
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

    _status_t getParameter(int key, Parcel *reply)
    {
        Parcel data;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        data.writeInt32(key);
        return remote()->transact(GET_PARAMETER, data, reply);
    }
    
    _status_t invoke(const Parcel& request, Parcel *reply)
    {
        // Avoid doing any extra copy. The interface descriptor should
        // have been set by MediaPlayer.java.
        return remote()->transact(INVOKE, request, reply);
    }
    
    _status_t flush()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMagPlayerClient::getInterfaceDescriptor());
        remote()->transact(FLUSH, data, &reply);
        return reply.readInt32();
    }
    
};

IMPLEMENT_META_INTERFACE(MagPlayerClient, "android.media.IMagPlayerClient");

// ----------------------------------------------------------------------

_status_t BnMagPlayerClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case DISCONNECT: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            disconnect();
            return MAG_NO_ERROR;
        } break;
        case SET_DATA_SOURCE_URL: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            const char* url = data.readCString();
            reply->writeInt32(setDataSource(url));
            return MAG_NO_ERROR;
        } break;
        case SET_DATA_SOURCE_FD: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int fd = data.readFileDescriptor();
            int64_t offset = data.readInt64();
            int64_t length = data.readInt64();
            reply->writeInt32(setDataSource(fd, offset, length));
            return MAG_NO_ERROR;
        }
        case SET_DATA_SOURCE_STREAM: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            sp<IStreamBuffer> source =
                interface_cast<IStreamBuffer>(data.readStrongBinder());
            reply->writeInt32(setDataSource(source));
            return MAG_NO_ERROR;
        }
        case SET_VIDEO_SURFACETEXTURE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            sp<ISurfaceTexture> surfaceTexture =
                    interface_cast<ISurfaceTexture>(data.readStrongBinder());
            reply->writeInt32(setVideoSurfaceTexture(surfaceTexture));
            return MAG_NO_ERROR;
        } break;
        case PREPARE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(prepare());
            return MAG_NO_ERROR;
        } break;
        case PREPARE_ASYNC: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(prepareAsync());
            return MAG_NO_ERROR;
        } break;
        case START: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(start());
            return MAG_NO_ERROR;
        } break;
        case STOP: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(stop());
            return MAG_NO_ERROR;
        } break;
        case IS_PLAYING: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            bool state;
            _status_t ret = isPlaying(&state);
            reply->writeInt32(state);
            reply->writeInt32(ret);
            return MAG_NO_ERROR;
        } break;
        case PAUSE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(pause());
            return MAG_NO_ERROR;
        } break;
        case SEEK_TO: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(seekTo(data.readInt32()));
            return MAG_NO_ERROR;
        } break;
        case FLUSH: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(flush());
            return MAG_NO_ERROR;
        } break;
        case FAST: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(fast(data.readInt32()));
            return MAG_NO_ERROR;
        } break;
        case GET_CURRENT_POSITION: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int msec;
            _status_t ret = getCurrentPosition(&msec);
            reply->writeInt32(msec);
            reply->writeInt32(ret);
            return MAG_NO_ERROR;
        } break;
        case GET_DURATION: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            int msec;
            _status_t ret = getDuration(&msec);
            reply->writeInt32(msec);
            reply->writeInt32(ret);
            return MAG_NO_ERROR;
        } break;
        case RESET: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            reply->writeInt32(reset());
            return MAG_NO_ERROR;
        } break;
        case SET_VOLUME: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            float leftVolume = data.readFloat();
            float rightVolume = data.readFloat();
            reply->writeInt32(setVolume(leftVolume, rightVolume));
            return MAG_NO_ERROR;
        } break;
        case INVOKE: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            _status_t result = invoke(data, reply);
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
            return MAG_NO_ERROR;
        } break;
        case GET_PARAMETER: {
            CHECK_INTERFACE(IMagPlayerClient, data, reply);
            return getParameter(data.readInt32(), reply);
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}


