#ifndef __MAGOMX_PORT_FFMPEG_VREN_H__
#define __MAGOMX_PORT_FFMPEG_VREN_H__

#include "MagOMX_Port_video.h"

#define VREN_PORT_NAME "Vren"

DeclareClass(MagOmxPort_FFmpeg_Vren, MagOmxPortVideo);

Virtuals(MagOmxPort_FFmpeg_Vren, MagOmxPortVideo) 

EndOfVirtuals;

ClassMembers(MagOmxPort_FFmpeg_Vren, MagOmxPortVideo, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif