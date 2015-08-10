#ifndef OMX_Core_Ext_h
#define OMX_Core_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum OMX_DYNAMIC_PORT_TYPE
{
    OMX_DynamicPort_Unknown = 0,
    OMX_DynamicPort_Buffer,
    OMX_DynamicPort_Video,
    OMX_DynamicPort_Audio,
    OMX_DynamicPort_Subtitle
} OMX_DYNAMIC_PORT_TYPE;

typedef struct OMX_VIDEO_STREAM_INFO
{
    OMX_U32    width;
    OMX_U32    height;
    OMX_U32    fps;
    OMX_U32    bps;     /*in kb/s*/
}OMX_VIDEO_STREAM_INFO;

typedef struct OMX_AUDIO_STREAM_INFO
{
    OMX_U32    hz;
    OMX_U32    bps;     /*in kb/s*/
}OMX_AUDIO_STREAM_INFO;

typedef struct OMX_SUBTITLE_STREAM_INFO
{
    OMX_U32    hz;
}OMX_SUBTITLE_STREAM_INFO;

typedef struct OMX_DEMUXER_STREAM_INFO
{   
    OMX_DYNAMIC_PORT_TYPE type;
    
    union{
        OMX_VIDEO_STREAM_INFO    video;
        OMX_AUDIO_STREAM_INFO    audio;
        OMX_SUBTITLE_STREAM_INFO subtitle;
    } format;

    OMX_S32    time_base_num;
    OMX_S32    time_base_den;
    OMX_BOOL   url_data_source;   /*if true, the demuxer directly retrieve the data from URL. Otherwise, it gets data from bytes buffer*/
    OMX_S64    duration;
    OMX_U8     codec_info[128];
    OMX_U32    codec_id;
    OMX_U32    stream_id;        /*the stream id used by low-level demuxer and identifies the frame belonging to which stream*/  
    OMX_PTR    stream_hanlde;    /*point to low-level demuxer handler*/
}OMX_DEMUXER_STREAM_INFO;

typedef enum OMX_EVENTTYPE_EXT
{
    OMX_EventExtExtension = OMX_EventKhronosExtensions, /**< Reserved region for introducing Khronos Standard Extensions */ 
    OMX_EventExtDynamicPortAdding,
    OMX_EventExtAVStreamInfo
} OMX_EVENTTYPE_EXT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */