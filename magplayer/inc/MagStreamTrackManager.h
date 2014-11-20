#ifndef __MAG_PLAYER_STREAM_TRACK_MANAGER_H__
#define __MAG_PLAYER_STREAM_TRACK_MANAGER_H__

#include "framework/MagFramework.h"
#include "MagMediaBuffer.h"
#include "MagBufferObserver.h"
#include "MagESFormat.h"

#define DEFAULT_VIDEO_BUFFER_SIZE (16 * 1024 * 1024)
#define MAX_VIDEO_BUFFER_SIZE     (64 * 1024 * 1024)

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
    TRACK_PLAY_COMPLETE,
};

typedef struct{
    enum TrackType_t type;
    enum TrackStatus_t status;
    
    ui32 index;                   /*the incremental index of all tracks in the stream in align with the structure TrackInfoTable_t*/
    char name[32];                /*the description information acquired from the stream*/
    ui32 streamID;                /*the stream id used in low-level demuxer*/
    ui32 codec;                   /*codec id*/
    ui32 pid;                     /*pid for ts*/

    i32  pendingRead;             /*the pending read number when no data happens while try to read frame*/
    MagMessageHandle message;     /*To send the message for reading frame*/

    void *stream_track;           /*point to the class Stream_Track*/
    i32 time_base_num;
    i32 time_base_den;
    i64 duration;                 /*stream duration in ms*/
    i64 start_time;               /*stream start time in ms*/

    void *avCodecContext;         /*avCodec context used for decoding the video/auido stream*/
    void *avFrame;                /*the decoded video/audio frame*/

    ui32 top_rgb;                 /*the rgb value of the top half video frame*/
    ui32 bottom_rgb;              /*the rgb value of the bottom half video frame*/

    i64 last_pts;                 /*the pts of previous video/audio packet*/
    MAG_BOOL_t generatePTS;       /*MAG_TRUE: to generate the pts by itself, MAG_FALSE: use stream PTS*/
}TrackInfo_t;

typedef struct{
    ui32 videoTrackNum;
    ui32 audioTrackNum;
    ui32 subtitleTrackNum;
    ui32 totalTrackNum;
    
    /*
        *index (0)                                           - (videoTrackNum - 1):                                              video tracks
        *index (videoTrackNum)                         - (videoTrackNum + audioTrackNum -1):                       audio tracks
        *index (videoTrackNum + audioTrackNum) - (videoTrackNum + audioTrackNum subtitleTrackNum-1): subtitle tracks
        */
    TrackInfo_t **trackTableList;
}TrackInfoTable_t;

typedef struct{
    List_t node;
    ui64 enqueue_time;
    ui32 size;
}FrameStatistics_t;

class Stream_Track{
public:
    Stream_Track(TrackInfo_t *info);
    virtual ~Stream_Track();
    
    TrackInfo_t    *getInfo();

    _status_t      start();
    _status_t      stop();

    MediaBuffer_t  *dequeueFrame(bool lock = true);
    _status_t      enqueueFrame(MediaBuffer_t *buffer);
    _status_t      releaseFrame(MediaBuffer_t *mb);

    MediaBuffer_t  *getMediaBuffer();
    void           setBufferPoolSize(ui32 size);

    ui32           getBufferingDataTime();
    bool           getFullness();
    BufferStatus_t getBufferStatus();
    void           setBufferStatus(BufferStatus_t st);
    
    void           setFormatter(MagESFormat *fmt);
    void           *getFormatter();

    ui32           doFrameStat(ui32 frame_size);
    
    MagMutexHandle mMutex;
private:
    _status_t putMediaBuffer(List_t *list_head, MediaBuffer_t *mb);
    
    MagMutexHandle mBufferStatMutex;

    TrackInfo_t *mInfo;
    ui32 mBufferPoolSize;
    magMempoolHandle mBufPoolHandle;
    List_t mMBufBusyListHead;
    List_t mMBufFreeListHead;

    i64 mStartPTS;  //the PTS of the 1st frame in the queue
    i64 mEndPTS;    //the PTS of the last frame in the queue
    ui32 mFrameNum; //record total frame number in the track

    BufferStatus_t mBufferStatus;
    MagESFormat *mEsFormatter;

    bool mIsFull;
    bool mIsRunning;

    List_t mFrameStatList;
};

class Stream_Track_Manager{
public:
	Stream_Track_Manager(void *pDemuxer, MagMiniDBHandle hParamDB);
	virtual ~Stream_Track_Manager();

	TrackInfoTable_t *getTrackInfoList();
    void             destroyTrackInfoList();
    
    _status_t   setPlayingTrackID(ui32 index);
    ui32        getPlayingTracksID(ui32 *index);
    ui32        getPlayingStreamsID(ui32 *index);

    _status_t   addStreamTrack(Stream_Track *track);
    _status_t   deleteStreamTrack(i32 streamID);
    Stream_Track *getStreamTrack(ui32 StreamID);
    
    _status_t   readFrame(ui32 trackIndex, MediaBuffer_t **buffer);

    _status_t   attachBufferObserver(MagBufferObserver *pObserver);
	_status_t   dettachBufferObserver(MagBufferObserver *pObserver);
	_status_t   setBufferPolicy(BufferPolicy_t *pPolicy);
    
    _status_t   start();
    _status_t   stop();
    _status_t   pause();
    _status_t   resume();
    _status_t   flush();
    void        readyToFlush();
    void        readyToStop();

    void        setDemuxerNotifier(MagMessageHandle msg);

    bool        interruptReading();
    void        getAVBufferStatus(ui32 *videoBuf, ui32 *audioBuf, ui32 *loadingSpeed);

private:
	_status_t   readFrameFromQueue(ui32 trackIndex, MediaBuffer_t **buffer);
	_status_t   readFramesMore(Stream_Track *track, ui32 StreamID);
    ui32        getBufferPoolSize(enum TrackType_t type);
    bool        handleAllPlayingTracksBuffer();

    static boolean ReadingFramesEntry(void *priv);

    MagMiniDBHandle mParamDB;
    void *mpDemuxer;
	TrackInfoTable_t *mTrackList;
	RBTreeNodeHandle mStreamIDRoot;

    // BufferStatus_t mBufferStatus;

	MagBufferObserver *mpObserver;
	BufferPolicy_t mBufManagePolicy;
	MagThreadHandle mhReadingFramesEntry;

    MagEventGroupHandle mPlayingEvtGroup;
    MagEventHandle      mEventResume;
    bool mIsPaused;
    bool mIsFlushed;
    bool mAbortReading;

    /*for the live stream, such as rtsp etc, the stream should be played ASAP. So the buffering logic should be disabled*/
    i32 mDisableBufferMgr;
};
#endif