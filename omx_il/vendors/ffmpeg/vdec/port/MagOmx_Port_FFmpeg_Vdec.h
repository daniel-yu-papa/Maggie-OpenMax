#ifndef __MAGOMX_PORT_FFMPEG_VDEC_H__
#define __MAGOMX_PORT_FFMPEG_VDEC_H__

#include "MagOMX_Port_video.h"

#define VDEC_PORT_NAME "Vdec"

DeclareClass(MagOmxPort_FFmpeg_Vdec, MagOmxPortVideo);

Virtuals(MagOmxPort_FFmpeg_Vdec, MagOmxPortVideo) 

EndOfVirtuals;

ClassMembers(MagOmxPort_FFmpeg_Vdec, MagOmxPortVideo, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif