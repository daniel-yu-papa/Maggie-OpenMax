#ifndef __MAGOMX_PORT_OTHER_CLOCK_H__
#define __MAGOMX_PORT_OTHER_CLOCK_H__

#include "MagOMX_Port_base.h"
#include "MagOMX_Port_baseImpl.h"

DeclareClass(MagOmxPortClock, MagOmxPortImpl);

Virtuals(MagOmxPortClock, MagOmxPortImpl) 
   
EndOfVirtuals;

ClassMembers(MagOmxPortClock, MagOmxPortImpl, \
	void (*self)(void); \
)
    MagMutexHandle                mhMutex;
    OMX_OTHER_PORTDEFINITIONTYPE  mPortDefinition;

EndOfClassMembers;

#endif