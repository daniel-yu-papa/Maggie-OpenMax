#ifndef __MAGOMX_COMPONENT_FFMPEG_ADEC_H__
#define __MAGOMX_COMPONENT_FFMPEG_ADEC_H__

#include "MagOMX_Component_audio.h"

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"

DeclareClass(MagOmxComponent_FFmpeg_Adec, MagOmxComponentAudio);

Virtuals(MagOmxComponent_FFmpeg_Adec, MagOmxComponentAudio) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Adec, MagOmxComponentAudio, \
	void (*self)(void); \
)
	AVCodec *mpAudioCodec;
	AVStream *mpAudioStream;
	AVFormatContext *mpAVFormat;

EndOfClassMembers;

#endif