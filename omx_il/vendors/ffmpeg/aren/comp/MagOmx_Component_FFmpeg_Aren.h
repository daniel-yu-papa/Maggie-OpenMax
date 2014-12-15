#ifndef __MAGOMX_COMPONENT_FFMPEG_AREN_H__
#define __MAGOMX_COMPONENT_FFMPEG_AREN_H__

#include "MagOMX_Component_audio.h"

#define CAPTURE_PCM_DATA_TO_FILE

DeclareClass(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio);

Virtuals(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_FFmpeg_Aren, MagOmxComponentAudio, \
	void (*self)(void); \
)

#ifdef CAPTURE_PCM_DATA_TO_FILE
	FILE *mfPCMFile;
#endif
EndOfClassMembers;

#endif