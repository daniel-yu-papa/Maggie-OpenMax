#ifndef __MAGPLAYER_DEMUXER_FFMPEG_H__
#define __MAGPLAYER_DEMUXER_FFMPEG_H__

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavformat/avio.h"
#include "libavformat/internal.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"
}

#include "framework/MagFramework.h"
#include "MagDemuxerBaseImpl.h"
#include "MagContentPipe.h"
#include "MagStreamTrackManager.h"

/*#include "MrvlAMPESFormat_AAC.h"
#include "MrvlAMPESFormat_AVC.h"*/

#include <stdio.h>

#define TS_NOPROBE_NEW_PID_MASK      (0x8000)
#define TS_NOPROBE_REMOVE_PID_MASK   (0x10000)

class MagDemuxerFFMPEG : public MagDemuxerBaseImpl{
public:
    MagDemuxerFFMPEG();
    virtual ~MagDemuxerFFMPEG();

    virtual _status_t   prepare(MagContentPipe *contentPipe, MagBufferObserver *pObserver, MagMiniDBHandle paramDB);
    virtual _status_t   start();
    
    virtual _status_t   stopImpl();
    virtual _status_t   flushImpl();
    virtual _status_t   seekToImpl(i32 msec, i64 mediaTime, TrackInfo_t *track);
    virtual _status_t   readFrameImpl(Stream_Track *track, ui32 StreamID);
    
private:
    MagMutexHandle mSeekMutex;
    MagContentPipe *mDataSource;
    _status_t  stop_common();

    static void PrintLog_Callback(void* ptr, int level, const char* fmt, va_list vl);
    
    _status_t  create_track(TrackInfo_t *track, AVCodecContext* codec);
    
    bool checkSupportedStreams(AVStream *st);
    ui32 convertCodec_FFMPEGToOMX(enum AVCodecID ffmpegCodec, TrackType_t type);
    enum AVCodecID convertCodec_OMXToFFMPEG(ui32 omxCodec, TrackType_t type);
    void setMediaBufferFields(MagOmxMediaBuffer_t *mb, AVPacket *packet, TrackInfo_t* info);
    _status_t readMore(Stream_Track *track, ui32 StreamID);

    _status_t  probe_add_streams();
    _status_t  probe_prepare(const char *url);
    _status_t  probe_stop();
    
    int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);
    AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                    AVFormatContext *s, AVStream *st, AVCodec *codec);
    AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                               AVDictionary *codec_opts);

    void setOptionsByUrl(const char *url);

    AVFormatContext *mpAVFormat; /* av format context. */
    // AVInputFormat   *mpInputFormat;
    AVDictionary    *mpFormatOpts;
    AVDictionary    *mCodecOpts;
    i32        mTotalTrackNum;

    AVIOContext     *ffmpeg_utiles_CreateAVIO();
    // AVFormatContext *ffmpeg_utiles_CreateAVFormat(AVInputFormat *inputFormat, 
    //                                                        AVIOContext *context, 
    //                                                        bool seekable, 
    //                                                        bool probe);
    void            ffmpeg_utiles_SetOption(const char *opt, const char *arg);

    void            fillMediaBuffer(Stream_Track *track, AVPacket *pPacket);

    void            fillVideoMetaData(TrackInfo_t *track, AVStream *aStream);
    void            fillAudioMetaData(TrackInfo_t *track, AVStream *aStream);

    i32 AVIO_Read (ui8 *buf, int buf_size);
    i32 AVIO_Write(ui8* buf, int buf_size);
    i64 AVIO_Seek (i64 offset, i32 whence);
    
    static i32 AVIO_Static_Read (void * opaque, ui8 *buf, int buf_size);
    static i32 AVIO_Static_Write(void* opaque, ui8* buf, int buf_size);
    static i64 AVIO_Static_Seek (void * opaque, i64 offset, i32 whence);

    static int demux_interrupt_cb(void *ctx);
    
    /*dump ffmpeg frames to the file. for debugging purpose*/
    FILE *mVideoDumpFile; 
    FILE *mAudioDumpFile;

    /*dump stream buffer reading data*/
    FILE *mStreamBufFile;
};

#endif

