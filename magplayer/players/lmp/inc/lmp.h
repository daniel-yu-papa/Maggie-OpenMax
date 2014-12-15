#ifndef __LMP_H__
#define __LMP_H__

#include <errno.h>

enum {
    LMP_OK                  = 0,    // Everything's swell.
    LMP_NO_ERROR            = 0,    // No errors.
    
    LMP_UNKNOWN_ERROR       = 0x80000000,

    LMP_NO_MEMORY           = -ENOMEM,
    LMP_INVALID_OPERATION   = -ENOSYS,
    LMP_BAD_VALUE           = -EINVAL,
    LMP_BAD_TYPE            = 0x80000001,
    LMP_NAME_NOT_FOUND      = -ENOENT,
    LMP_PERMISSION_DENIED   = -EPERM,
    LMP_NO_INIT             = -ENODEV,
    LMP_ALREADY_EXISTS      = -EEXIST,
    LMP_DEAD_OBJECT         = -EPIPE,
    LMP_FAILED_TRANSACTION  = 0x80000002,

    LMP_BAD_INDEX           = -EOVERFLOW,
    LMP_NOT_ENOUGH_DATA     = -ENODATA,
    LMP_WOULD_BLOCK         = -EWOULDBLOCK, 
    LMP_TIMED_OUT           = -ETIMEDOUT,
    LMP_UNKNOWN_TRANSACTION = -EBADMSG,
};

typedef enum
{
    LMP_PLAYER_EVT_STREAM_INVALID = 0,   //invalid streams
    LMP_PLAYER_EVT_PLAYBACK_ERROR,       //playback error
    LMP_PLAYER_EVT_PLAYBACK_COMPLETE,    //playback is complete
    LMP_PLAYER_EVT_SEEK_COMPLETE,        //seek completes
    LMP_PLAYER_EVT_BUFFER_STATUS,        //buffer status report. event+parameter: 0 - playing stop, (10-90) - buffer percentage, 100 - playing resume
    LMP_PLAYER_EVT_MAX
}lmp_event_t;

typedef void (*lmp_event_callback_t)(lmp_event_t evt, void *handler, unsigned int param1, unsigned int param2);

class LinuxMediaPlayer{
public:
    LinuxMediaPlayer(){};
    virtual ~LinuxMediaPlayer(){};

    virtual int        setDataSource(const char *url) = 0;
    virtual int        setDataSource(unsigned int fd, signed long long offset, signed long long length) = 0;
    virtual int        prepare() = 0;
    virtual int        prepareAsync() = 0;
    virtual int        start() = 0;
    virtual int        stop() = 0;
    virtual int        pause() = 0;
    virtual bool       isPlaying() = 0;
    virtual int        seekTo(int msec) = 0;
    virtual int        flush() = 0;
    virtual int        fast(int speed) = 0;
    virtual int        getCurrentPosition(int* msec) = 0;
    virtual int        getDuration(int* msec) = 0;
    virtual int        reset() = 0;
    virtual int        setVolume(float leftVolume, float rightVolume) = 0;
    virtual int        setParameter(int key, void *request) = 0;
    virtual int        getParameter(int key, void **reply) = 0;
    virtual int        invoke(const unsigned int methodID, const void *request, void *reply) = 0;
    virtual int        registerEventCallback(lmp_event_callback_t cb, void *handler) = 0;
};


LinuxMediaPlayer* GetMediaPlayer();

#endif