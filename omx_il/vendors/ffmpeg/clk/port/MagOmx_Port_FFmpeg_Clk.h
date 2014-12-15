#ifndef __MAGOMX_PORT_FFMPEG_CLOCK_H__
#define __MAGOMX_PORT_FFMPEG_CLOCK_H__

#include "MagOMX_Port_clock.h"

#define CLOCK_PORT_NAME "Clk"

DeclareClass(MagOmxPort_FFmpeg_Clk, MagOmxPortClock);

Virtuals(MagOmxPort_FFmpeg_Clk, MagOmxPortClock) 

EndOfVirtuals;

ClassMembers(MagOmxPort_FFmpeg_Clk, MagOmxPortClock, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif