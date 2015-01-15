#ifndef __MAGOMX_COMPONENT_FFMPEG_VSCH_H__
#define __MAGOMX_COMPONENT_FFMPEG_VSCH_H__

#include "MagOMX_Component_video.h"

// #define CAPTURE_YUV_FRAME_BEFORE_AVSYNC

DeclareClass(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo);

Virtuals(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo, \
	void (*self)(void); \
)
#ifdef CAPTURE_YUV_FRAME_BEFORE_AVSYNC
    FILE *mfYUVPreAvsync;
#endif
EndOfClassMembers;

#endif