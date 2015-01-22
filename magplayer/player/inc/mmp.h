#ifndef __MMP_H__
#define __MMP_H__

#include <errno.h>

enum {
    MMP_OK                  = 0,    // Everything's swell.
    MMP_NO_ERROR            = 0,    // No errors.
    
    MMP_UNKNOWN_ERROR       = 0x80000000,

    MMP_NO_MEMORY           = -ENOMEM,
    MMP_INVALID_OPERATION   = -ENOSYS,
    MMP_BAD_VALUE           = -EINVAL,
    MMP_BAD_TYPE            = 0x80000001,
    MMP_NAME_NOT_FOUND      = -ENOENT,
    MMP_PERMISSION_DENIED   = -EPERM,
    MMP_NO_INIT             = -ENODEV,
    MMP_ALREADY_EXISTS      = -EEXIST,
    MMP_DEAD_OBJECT         = -EPIPE,
    MMP_FAILED_TRANSACTION  = 0x80000002,

    MMP_BAD_INDEX           = -EOVERFLOW,
    MMP_NOT_ENOUGH_DATA     = -ENODATA,
    MMP_WOULD_BLOCK         = -EWOULDBLOCK, 
    MMP_TIMED_OUT           = -ETIMEDOUT,
    MMP_UNKNOWN_TRANSACTION = -EBADMSG,
};

typedef enum
{
    MMP_PLAYER_EVT_STREAM_INVALID = 0,   //invalid streams
    MMP_PLAYER_EVT_PLAYBACK_ERROR,       //playback error
    MMP_PLAYER_EVT_PLAYBACK_COMPLETE,    //playback is complete
    MMP_PLAYER_EVT_SEEK_COMPLETE,        //seek completes
    MMP_PLAYER_EVT_PREPARE_COMPLETE,     //prepare completes
    MMP_PLAYER_EVT_BUFFER_STATUS,        //buffer status report. event+parameter: 0 - playing stop, (10-90) - buffer percentage, 100 - playing resume
    MMP_PLAYER_EVT_FLUSH_DONE,           //flush video/audio pipeline is complete
    MMP_PLAYER_EVT_MAX
}mmp_event_t;

typedef void (*mmp_event_callback_t)(mmp_event_t evt, void *handler, unsigned int param1, unsigned int param2);

class MagMediaPlayer{
public:
    MagMediaPlayer(){};
    virtual ~MagMediaPlayer(){};

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
    virtual int        invoke(const unsigned int methodID, void *request, void **reply) = 0;
    virtual int        registerEventCallback(mmp_event_callback_t cb, void *handler) = 0;
};


MagMediaPlayer* GetMediaPlayer();

#endif