#ifndef __MAGPLAYER_DEMUXER_BASE_H__
#define __MAGPLAYER_DEMUXER_BASE_H__

#include "Parameters.h"
#include "MediaBuffer.h"

/*parameter keys*/
#define kDemuxer_Video_Track_Number       "demuxer.video.track.num"          /*Int32*/
#define kDemuxer_Audio_Track_Number       "demuxer.audio.track.num"          /*Int32*/
#define kDemuxer_Subtitle_Track_Number    "demuxer.subtitle.track.num"       /*Int32*/
#define kDemuxer_Track_Info               "demuxer.track.info%i"             /*Pointer: TrackInfo_t*/

#define kDemuxer_Probe_Stream             "demuxer.stream.probe"             /*String: no/yes*/
#define kDemuxer_Container_Type           "demuxer.container.type"           /*String: ts/unknown*/

enum TrackType_t{
    TRACK_UNKNOWN = 0,
    TRACK_VIDEO,
    TRACK_AUDIO,
    TRACK_SUBTITLE,
};

enum TrackStatus_t{
    TRACK_STOP = 0,
    TRACK_PAUSE,
    TRACK_PLAY,
};

typedef struct{
    enum TrackType_t type;
    enum TrackStatus_t status;
    
    ui32 index;                   /*the incremental index of all tracks in the stream in align with the structure TrackInfoTable_t*/
    ui8  *name;                   /*the description information acquired from the stream*/
    ui32 streamID;                /*the stream id used in low-level demuxer*/
    ui32 codec;                   /*codec id*/
    ui32 pid;                     /*pid for ts*/
}TrackInfo_t;

typedef struct{
    ui32 videoTrackNum;
    ui32 audioTrackNum;
    ui32 subtitleTrackNum;

    /*
        *index (0)                                           - (videoTrackNum - 1):                                              video tracks
        *index (videoTrackNum)                         - (videoTrackNum + audioTrackNum -1):                       audio tracks
        *index (videoTrackNum + audioTrackNum) - (videoTrackNum + audioTrackNum subtitleTrackNum-1): subtitle tracks
        */
    TrackInfo_t *trackTableList;
}TrackInfoTable_t;


class Stream_Track{
public:
    Stream_Track(TrackInfo_t *info);
    ~Stream_Track();
    
    TrackInfo_t *getInfo();

    _status_t start();
    MediaBuffer_t *dequeueFrame();
    _status_t     enqueueFrame(MediaBuffer_t *buffer);
    _status_t     releaseFrame(MediaBuffer_t *mb);

    MediaBuffer_t *getMediaBuffer();
    _status_t     reset();
    
private:
    _status_t putMediaBuffer(MediaBuffer_t *mb);
        
    TrackInfo_t *mInfo;
    ui32 mBufferPoolSize;
    magMempoolHandle mBufPoolHandle;
    List_t mMBufBusyListHead;
    List_t mMBufFreeListHead;
    MagMutexHandle mMutex;
};

class MagPlayer_Demuxer_Base{
public:
    MagPlayer_Demuxer_Base();
    virtual ~MagPlayer_Demuxer_Base();
    
    void setParameters(const char *name, MagParamType_t type, void *value);
    TrackInfoTable_t *getTrackInfoList();
    _status_t   setPlayingTrackID(ui32 index);
    ui32        getPlayingTracksID(ui32 **index);
    _status_t   readFrame(ui32 trackIndex, MediaBuffer_t **buffer);

    _status_t   getLooper();
    
    virtual _status_t   readFrameInternal(ui32 StreamID, MediaBuffer_t **buffer) = 0;
    virtual _status_t   start(MagPlayer_Component_CP *contentPipe, ui32 flags) = 0;
    virtual _status_t   stop() = 0;
    
protected:
    MagMiniDBHandle mParamDB;
    boolean mIsStarted;
    
private:
    enum{
        MagDemuxerMsg_ReadFrame,
    };

    void onReadFrame(MagMessageHandle msg);

    void             onMessageReceived(const MagMessageHandle msg, void *priv);
    MagMessageHandle createMessage(ui32 what);
    _status_t        postMessage(MagMessageHandle msg, ui64 delay);
    
    TrackInfoTable_t *mTrackList;

    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
};

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _status_t releaseMediaBuffer(void* thiz, MediaBuffer_t *mb);

#undef EXTERNC

#endif