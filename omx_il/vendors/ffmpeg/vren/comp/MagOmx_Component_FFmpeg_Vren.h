#ifndef __MAGOMX_COMPONENT_FFMPEG_VREN_H__
#define __MAGOMX_COMPONENT_FFMPEG_VREN_H__

#include "MagOMX_Component_video.h"

DeclareClass(MagOmxComponent_FFmpeg_Vren, MagOmxComponentVideo);

Virtuals(MagOmxComponent_FFmpeg_Vren, MagOmxComponentVideo) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Vren, MagOmxComponentVideo, \
	void (*self)(void); \
)
	
EndOfClassMembers;

#endif