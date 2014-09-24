#ifndef __MAG_OMX_COMPONENT_DISP_TEST_H__
#define __MAG_OMX_COMPONENT_DISP_TEST_H__

#include "MagOMX_Component_video.h"

DeclareClass(MagOmxComponent_DispTest, MagOmxComponentVideo);

Virtuals(MagOmxComponent_DispTest, MagOmxComponentVideo) 

EndOfVirtuals;

ClassMembers(MagOmxComponent_DispTest, MagOmxComponentVideo, \
	void (*self)(void); \
)

EndOfClassMembers;

#endif