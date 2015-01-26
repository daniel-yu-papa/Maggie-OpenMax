#ifndef __MAGOMX_COMPONENT_FFMPEG_AREN_H__
#define __MAGOMX_COMPONENT_FFMPEG_AREN_H__

#include "MagOMX_Component_audio.h"

/*#define CAPTURE_PCM_DATA_TO_FILE*/

typedef struct{
    List_t Node;    
    OMX_BUFFERHEADERTYPE *pBufHeader;
}MagOmx_Aren_BufferNode_t;

DeclareClass(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio);

Virtuals(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio, \
	OMX_BUFFERHEADERTYPE *getBuffer(MagOmxComponent_FFmpeg_Aren thiz);                 \
    void putBuffer(MagOmxComponent_FFmpeg_Aren thiz, OMX_BUFFERHEADERTYPE *bufHeader); \
)
    AVStream *mpAudioStream;
    AVFormatContext *mpAVFormat;

    MagMutexHandle mListMutex;
    List_t mBufFreeListHead;
    List_t mBufBusyListHead;

    MagEventHandle         mNewBufEvent;
    MagEventGroupHandle    mWaitBufEventGroup;

#ifdef CAPTURE_PCM_DATA_TO_FILE
	FILE *mfPCMFile;
#endif
EndOfClassMembers;

#endif