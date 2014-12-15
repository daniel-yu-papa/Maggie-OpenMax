#ifndef __MAGOMX_PORT_FFMPEG_AREN_H__
#define __MAGOMX_PORT_FFMPEG_AREN_H__

#include "MagOMX_Port_audio.h"

#define AREN_PORT_NAME "Aren"

DeclareClass(MagOmxPort_FFmpeg_Aren, MagOmxPortAudio);

Virtuals(MagOmxPort_FFmpeg_Aren, MagOmxPortAudio) 

EndOfVirtuals;

ClassMembers(MagOmxPort_FFmpeg_Aren, MagOmxPortAudio, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif