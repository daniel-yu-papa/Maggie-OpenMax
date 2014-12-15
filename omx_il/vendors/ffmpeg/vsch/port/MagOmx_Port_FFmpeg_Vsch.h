#ifndef __MAGOMX_PORT_FFMPEG_VSCH_H__
#define __MAGOMX_PORT_FFMPEG_VSCH_H__

#include "MagOMX_Port_video.h"

#define VSCH_PORT_NAME "Vsch"

DeclareClass(MagOmxPort_FFmpeg_Vsch, MagOmxPortVideo);

Virtuals(MagOmxPort_FFmpeg_Vsch, MagOmxPortVideo) 

EndOfVirtuals;

ClassMembers(MagOmxPort_FFmpeg_Vsch, MagOmxPortVideo, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif