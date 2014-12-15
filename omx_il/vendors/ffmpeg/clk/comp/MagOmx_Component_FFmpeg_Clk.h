#ifndef __MAGOMX_COMPONENT_FFMPEG_CLOCK_H__
#define __MAGOMX_COMPONENT_FFMPEG_CLOCK_H__

#include "MagOMX_Component_clock.h"

DeclareClass(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock);

Virtuals(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock, \
	void (*self)(void); \
)
	OMX_U32 mPortIndex;
	
EndOfClassMembers;

#endif