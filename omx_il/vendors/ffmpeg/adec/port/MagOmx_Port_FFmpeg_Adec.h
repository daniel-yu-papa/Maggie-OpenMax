#ifndef __MAGOMX_PORT_FFMPEG_ADEC_H__
#define __MAGOMX_PORT_FFMPEG_ADEC_H__

#include "MagOMX_Port_audio.h"

#define ADEC_PORT_NAME "Adec"

DeclareClass(MagOmxPort_FFmpeg_Adec, MagOmxPortAudio);

Virtuals(MagOmxPort_FFmpeg_Adec, MagOmxPortAudio) 

EndOfVirtuals;

ClassMembers(MagOmxPort_FFmpeg_Adec, MagOmxPortAudio, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif