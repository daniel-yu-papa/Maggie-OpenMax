#ifndef __MAGOMX_PORT_OTHER_CLOCK_H__
#define __MAGOMX_PORT_OTHER_CLOCK_H__

#include "MagOMX_Port_base.h"
#include "MagOMX_Port_baseImpl.h"

DeclareClass(MagOmxPortClock, MagOmxPortImpl);

Virtuals(MagOmxPortClock, MagOmxPortImpl) 
   
EndOfVirtuals;

ClassMembers(MagOmxPortClock, MagOmxPortImpl, \
	void (*addFormat)(MagOmxPortAudio hPort, MagOMX_Audio_PortFormat_t *pEncoding); \
)
    MagMutexHandle         mhMutex;
    MagMiniDBHandle        mParametersDB;

EndOfClassMembers;

#endif