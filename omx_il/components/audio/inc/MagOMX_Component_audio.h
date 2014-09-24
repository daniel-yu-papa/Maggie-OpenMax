#ifndef __MAGOMX_COMPONENT_AUDIO_H__
#define __MAGOMX_COMPONENT_AUDIO_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

DeclareClass(MagOmxComponentAudio, MagOmxComponentImpl);

Virtuals(MagOmxComponentAudio, MagOmxComponentImpl) 
   
EndOfVirtuals;

ClassMembers(MagOmxComponentAudio, MagOmxComponentImpl, \
	void (*self)(void); \
)
    MagMutexHandle         mhMutex;
    MagMiniDBHandle        mParametersDB;

EndOfClassMembers;

#endif