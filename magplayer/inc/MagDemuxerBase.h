#ifndef __MAGPLAYER_DEMUXER_BASE_H__
#define __MAGPLAYER_DEMUXER_BASE_H__

#include "framework/MagFramework.h"
#include "MagMediaBuffer.h"
#include "MagContentPipe.h"
#include "MagBufferObserver.h"
#include "MagStreamTrackManager.h"

/*parameter keys*/
#define kDemuxer_Video_Track_Number       "demuxer.video.track.num"          /*Int32*/
#define kDemuxer_Audio_Track_Number       "demuxer.audio.track.num"          /*Int32*/
#define kDemuxer_Subtitle_Track_Number    "demuxer.subtitle.track.num"       /*Int32*/
#define kDemuxer_Track_Info               "demuxer.track.info.%i"            /*Pointer: TrackInfo_t*/

#define kDemuxer_Bit_Rate                 "demuxer.bit.rate"                 /*Int32 kbps*/
#define kDemuxer_Video_Width              "demuxer.video.width"              /*Int32 pixels*/
#define kDemuxer_Video_Height             "demuxer.video.height"             /*Int32 pixels*/
#define kDemuxer_Stream_Duration          "demuxer.stream.duration"          /*Int64 ms*/

#define kDemuxer_Probe_Stream             "demuxer.stream.probe"             /*String: no/yes*/
#define kDemuxer_Container_Type           "demuxer.container.type"           /*String: ts/unknown*/

#define kDemuxer_Disable_Buffering        "demuxer.buffering.disable"        /*Int32 1-disable 0-enable*/

class MagDemuxerBase{
public:
    enum {
        kWhatReadFrame      = 'redf',
        kWhatError          = 'erro',
    };
    
    enum{
        MagDemuxerMsg_PlayerNotify,
        MagDemuxerMsg_ContentPipeNotify,
        MagDemuxerMsg_Stop,
        MagDemuxerMsg_Flush
    };

    MagDemuxerBase(){};
    virtual ~MagDemuxerBase(){};
    
    virtual _status_t   setPlayingTrackID(ui32 index) = 0;
    virtual ui32        getPlayingTracksID(ui32 *index) = 0;
    virtual _status_t   readFrame(ui32 trackIndex, MediaBuffer_t **buffer) = 0;
    virtual MagMessageHandle createNotifyMsg() = 0;
    virtual _status_t   prepare(MagContentPipe *contentPipe, MagBufferObserver *pObserver, MagMiniDBHandle paramDB) = 0;
    virtual _status_t   start() = 0;
    virtual _status_t   stop()  = 0;
    virtual void        readyToStop() = 0;
    virtual _status_t   pause()  = 0;
    virtual _status_t   resume()  = 0;
    virtual _status_t   flush()  = 0;
    virtual void        readyToFlush() = 0;
    virtual _status_t   seekTo(i32 msec) = 0;
    virtual _status_t   dettachBufferObserver(MagBufferObserver *pObserver) = 0;

    virtual TrackInfoTable_t *getTrackInfoList() = 0;
    virtual void getAVBufferStatus(ui32 *videoBuf, ui32 *audioBuf, ui32 *loadingSpeed) = 0;
    virtual void abort() = 0;
};

#endif