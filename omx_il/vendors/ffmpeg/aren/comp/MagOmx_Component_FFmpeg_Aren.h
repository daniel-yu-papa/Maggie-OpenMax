#ifndef __MAGOMX_COMPONENT_FFMPEG_AREN_H__
#define __MAGOMX_COMPONENT_FFMPEG_AREN_H__

#include "MagOMX_Component_audio.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"

/*#define CAPTURE_PCM_DATA_TO_FILE*/
/*#define CAPTURE_PCM_DATA_TO_SDL*/

typedef struct{
    OMX_BUFFERHEADERTYPE *pOmxBufHeader;
    ui8 *buf;
    OMX_U32 nFilledLen;
    OMX_U32 nOffset;
}MagOmx_Aren_Buffer_t;

typedef struct{
    List_t Node;    
    MagOmx_Aren_Buffer_t *pBuf;
}MagOmx_Aren_BufferNode_t;

typedef struct AudioParams {
    int freq;
    int channels;
    i64 channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

DeclareClass(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio);

Virtuals(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio, \
    int (*openAudio)(MagOmxComponent_FFmpeg_Aren thiz);      \
	MagOmx_Aren_BufferNode_t *(*getBuffer)(MagOmxComponent_FFmpeg_Aren thiz);                 \
    void (*putBuffer)(MagOmxComponent_FFmpeg_Aren thiz, OMX_BUFFERHEADERTYPE *pOmxBufHeader, ui8 *buf);     \
    void (*resetBuf)(MagOmxComponent_FFmpeg_Aren thiz); \
)
    AVStream *mpAudioStream;
    AVFormatContext *mpAVFormat;
    struct SwrContext *mpSwrCtx;
    AudioParams mAudioParameters;

    MagMutexHandle mListMutex;
    List_t mBufFreeListHead;
    List_t mBufBusyListHead;

    MagEventHandle         mNewBufEvent;
    MagEventGroupHandle    mWaitBufEventGroup;

#ifdef CAPTURE_PCM_DATA_TO_FILE
	FILE *mfPCMFile;
#endif

#ifdef CAPTURE_PCM_DATA_TO_SDL
    FILE *mfPCMSdl;
#endif
EndOfClassMembers;

#endif