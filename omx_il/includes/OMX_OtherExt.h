#ifndef OMX_Other_Ext_h
#define OMX_Other_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Each OMX header must include all required header files to allow the
 *  header to compile without errors.  The includes below are required
 *  for this header file to compile successfully 
 */

#include <OMX_Core.h>

typedef enum OMX_BUFFER_MODE{
    OMX_BUFFER_MODE_NONE = 0,
    OMX_BUFFER_MODE_PUSH,
    OMX_BUFFER_MODE_PULL
}OMX_BUFFER_MODE;

typedef enum OMX_BUFFER_TYPE{
    OMX_BUFFER_TYPE_NONE = 0,   /*no buffer needed*/
    OMX_BUFFER_TYPE_BYTE,
    OMX_BUFFER_TYPE_FRAME
}OMX_BUFFER_TYPE;

typedef enum OMX_SEEK_WHENCE{
    OMX_SEEK_SET = 0,
    OMX_SEEK_CUR,
    OMX_SEEK_END
}OMX_SEEK_WHENCE;

typedef struct OMX_BUFFER_PARAM {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_BUFFER_MODE mode;         /*push or pull*/
    OMX_U32 uHighPercent;         /*High threshold for buffering to finish. Allowed [0, 100]. default: 99*/
    OMX_U32 uLowPercent;          /*Low threshold for buffering to start. Allowed [0, 100]. default: 10*/
    OMX_U32 uMaxSizeBuffers;      /*Max. number of buffers in the queue*/
    OMX_U32 uMaxSizeBytes;        /*Max. amount of data in the queue in Bytes*/
    OMX_U32 uMaxSizeTime;         /*Max. amount of data in the queue in ns*/
    OMX_U32 uRingBufferMaxSize;   /*The maximum size of the ring buffer in bytes. If it is 0, the ring buffer is disabled*/
    OMX_U32 uUseRateEstimate;     /*Estimate the bitrate of the stream to calculate time level.*/
} OMX_BUFFER_PARAM;

typedef struct OMX_BUFFER_STATUS {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 uCurrentLevelFrames;         /*Current number of buffers in the queue*/
    OMX_U32 uCurrentLevelBytes;          /*Current amount of data in the queue (bytes).*/
    OMX_U32 uCurrentLevelTime;           /*Current amount of data in the queue (in ns).*/
} OMX_BUFFER_STATUS;

typedef struct OMX_CONFIG_DATABUFFER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U8* pBuffer;
    OMX_U32 nLen;
    OMX_U32 nDoneLen;          /*Returned: actual read/write data length*/
} OMX_CONFIG_DATABUFFER;

typedef struct OMX_CONFIG_SEEKDATABUFFER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_S64 sOffset;
    OMX_SEEK_WHENCE sWhence;
    OMX_S64 sCurPos;            /*Returned: current position in the source after the seeking*/
} OMX_CONFIG_SEEKDATABUFFER;

typedef struct OMX_DEMUXER_SETTING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_BUFFER_TYPE eBufferType;
    OMX_STRING      cStreamUrl;
} OMX_DEMUXER_SETTING;

typedef struct OMX_DEMUXER_KICKOFF {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_STRING      url;
    OMX_S32         *pnStreamId;    /*the stream ids(>= 0) to be started. -1: not running */
    OMX_U32         nStreamNum;     /*Total number of the started streams*/
} OMX_DEMUXER_KICKOFF;

typedef struct OMX_DATA_SOURCE_SETTING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_STRING      url; 
}OMX_DATA_SOURCE_SETTING;

typedef enum OMX_COMPONENT_TYPE{
    OMX_COMPONENT_VIDEO = 0,
    OMX_COMPONENT_AUDIO,
    OMX_COMPONENT_OTHER
}OMX_COMPONENT_TYPE;

typedef struct OMX_CODEC_PIPELINE_COMP_PARAM {
    OMX_STRING role;
    OMX_U32    buffer_number;
    OMX_U32    buffer_size;
    OMX_COMPONENT_TYPE type;
    OMX_U32    codec_id;
    OMX_PTR    stream_handle;
}OMX_CODEC_PIPELINE_COMP_PARAM;

typedef struct OMX_CODEC_PIPELINE_SETTING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;

    OMX_PORTDOMAINTYPE domain;  /*the pipeline is only for one of domain, can't be mixture*/
    OMX_CODEC_PIPELINE_COMP_PARAM *compList;
    OMX_U32 compNum;
}OMX_CODEC_PIPELINE_SETTING;

typedef struct OMX_PLAYBACK_PIPELINE_SETTING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;

    OMX_STRING      url;
    OMX_BOOL        free_run;              /*OMX_TRUE: no clock component connected.*/
    
    OMX_BUFFER_TYPE buffer_type;
    OMX_S32         buffer_size;           /*in KB*/  
    OMX_S32         buffer_time;           /*in ms units*/
    OMX_S32         buffer_low_threshold;  /*< n%: pause*/
    OMX_S32         buffer_play_threshold; /*> n%. play*/
    OMX_S32         buffer_high_threshold; /*> n%. stop buffering*/
}OMX_PLAYBACK_PIPELINE_SETTING;

typedef enum OMX_DEMUXER_AVFRAME_FLAG{
  OMX_AVFRAME_FLAG_NONE        = 0,
  OMX_AVFRAME_FLAG_KEY_FRAME   = (1 << 0),
  OMX_AVFRAME_FLAG_EOS         = (1 << 1)
} OMX_DEMUXER_AVFRAME_FLAG;

typedef struct OMX_DEMUXER_AVFRAME{
    OMX_U32                  stream_id;    /*the id of the stream that the frame belongs to*/
    OMX_U32                  size;         /*the size of frame buffer*/
    OMX_U8                   *buffer;      /*point to frame buffer*/
    OMX_TICKS                pts;          /*in 90K*/
    OMX_TICKS                dts;          /*in 90K*/  

    OMX_S32                  duration;     /*Duration of this packet in ns*/
    OMX_S64                  position;     /*byte position in stream*/
    OMX_DEMUXER_AVFRAME_FLAG flag;

    OMX_PTR                   priv;        /*point to low-level avpacket*/
    OMX_PTR                   opaque;      
    void                      (*releaseFrame)(OMX_HANDLETYPE hComponent, OMX_PTR frame);
}OMX_DEMUXER_AVFRAME;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif