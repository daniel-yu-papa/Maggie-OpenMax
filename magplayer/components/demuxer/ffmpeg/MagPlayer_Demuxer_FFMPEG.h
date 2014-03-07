#ifndef __MAGPLAYER_DEMUXER_FFMPEG_H__
#define __MAGPLAYER_DEMUXER_FFMPEG_H__

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavformat/internal.h"
}

#define TS_NOPROBE_NEW_PID_MASK      (0x8000)
#define TS_NOPROBE_REMOVE_PID_MASK   (0x10000)


class MagPlayer_Demuxer_FFMPEG : public MagPlayer_Demuxer_Base{
public:
    MagPlayer_Demuxer_FFMPEG();
    virtual ~MagPlayer_Demuxer_FFMPEG();

    virtual _status_t   readFrameInternal(ui32 StreamID, MediaBuffer_t **buffer);
    virtual _status_t   start(MagPlayer_Component_CP *contentPipe, ui32 flags);
    virtual _status_t   stop();
    
private:
    MagPlayer_Component_CP *mDataSource;

    _status_t  ts_noprobe_start();
    _status_t  ts_noprobe_stop();
    _status_t  ts_noprobe_add_streams();
    bool       ts_noprobe_decide_adding(enum CodecID codec, ui32 pid);

    _status_t  create_track(TrackInfo_t *track);
    
    enum CodecID convertCodec_OMXToFFMPEG(ui32 omxCodec);
    void setMediaBufferFields(MediaBuffer_t *mb, AVPacket *packet);
    _status_t readMore(Stream_Track *track, ui32 StreamID);
    
    _status_t  probe_start();
    _status_t  probe_stop();
    
    AVFormatContext *mpAVFormat; /* av format context. */
    AVDictionary    *mpFormatOpts;

    bool       mInitialized;
    i32        mTotalTrackNum;
    
    AVIOContext     *ffmpeg_utiles_CreateAVIO();
    AVFormatContext *ffmpeg_utiles_CreateAVFormat();
    void            ffmpeg_utiles_SetOption(const char *opt, const char *arg);
    
    i32 AVIO_Read (ui8 *buf, int buf_size);
    i32 AVIO_Write(ui8* buf, int buf_size);
    i64 AVIO_Seek (i64 offset, i32 whence);
    
    static i32 AVIO_Static_Read (void * opaque, ui8 *buf, int buf_size);
    static i32 AVIO_Static_Write(void* opaque, ui8* buf, int buf_size);
    static i64 AVIO_Static_Seek (void * opaque, i64 offset, i32 whence);

    RBTreeNodeHandle mStreamIDRoot;
};

#endif

