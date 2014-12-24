#ifndef __MAGOMX_COMPONENT_FFMPEG_VSCH_H__
#define __MAGOMX_COMPONENT_FFMPEG_VSCH_H__

#include "MagOMX_Component_video.h"

DeclareClass(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo);

Virtuals(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Vsch, MagOmxComponentVideo, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif