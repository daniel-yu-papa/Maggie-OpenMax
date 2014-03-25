#ifndef __IMAGPLAYERCLIENT_H__
#define __IMAGPLAYERCLIENT_H__


#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <gui/ISurfaceTexture.h>

#include "MagFramework.h"

using namespace android;

class IMagPlayerClient: public IInterface
{
public:
    DECLARE_META_INTERFACE(MagPlayerClient);

    virtual void             disconnect() = 0;

    virtual _status_t        setDataSource(const char *url) = 0;
    virtual _status_t        setDataSource(i32 fd, i64 offset, i64 length) = 0;
    virtual _status_t        setDataSource(const sp<IStreamBuffer>& source) = 0;
    virtual _status_t        setVideoSurfaceTexture(
                                    const sp<ISurfaceTexture>& surfaceTexture) = 0;

    virtual _status_t        prepare() = 0;
    virtual _status_t        prepareAsync() = 0;
    virtual _status_t        start() = 0;
    virtual _status_t        stop() = 0;
    virtual _status_t        pause() = 0;
    virtual _status_t        isPlaying(bool* state) = 0;
    virtual _status_t        seekTo(int msec) = 0;
    virtual _status_t        fast(int multiple) = 0;
    virtual _status_t        getCurrentPosition(int* msec) = 0;
    virtual _status_t        getDuration(int* msec) = 0;
    virtual _status_t        reset() = 0;
    virtual _status_t        setVolume(float leftVolume, float rightVolume) = 0;
    virtual _status_t        setParameter(int key, const Parcel& request) = 0;
    virtual _status_t        getParameter(int key, Parcel* reply) = 0;

    // Invoke a generic method on the player by using opaque parcels
    // for the request and reply.
    // @param request Parcel that must start with the media player
    // interface token.
    // @param[out] reply Parcel to hold the reply data. Cannot be null.
    // @return OK if the invocation was made successfully.
    virtual _status_t        invoke(const Parcel& request, Parcel *reply) = 0;
    virtual _status_t        flush() = 0;
};

// ----------------------------------------------------------------------------

class BnMagPlayerClient: public BnInterface<IMagPlayerClient>
{
public:
    virtual _status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

#endif
