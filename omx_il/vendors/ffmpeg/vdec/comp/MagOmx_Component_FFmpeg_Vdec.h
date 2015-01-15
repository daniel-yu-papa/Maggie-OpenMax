#ifndef __MAGOMX_COMPONENT_FFMPEG_VDEC_H__
#define __MAGOMX_COMPONENT_FFMPEG_VDEC_H__

#include "MagOMX_Component_video.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"

// #define CAPTURE_ES_DATA
// #define CAPTURE_DECODED_YUV_DATA

DeclareClass(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo);

Virtuals(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Vdec, MagOmxComponentVideo, \
	void (*self)(void); \
)
	AVCodec *mpVideoCodec;
	AVStream *mpVideoStream;
	AVFormatContext *mpAVFormat;

    OMX_TICKS mPrePTS;
    
#ifdef CAPTURE_ES_DATA
    FILE *mfEsData;
#endif

#ifdef CAPTURE_DECODED_YUV_DATA
    FILE *mfDecodedYUV;
#endif
EndOfClassMembers;

#endif