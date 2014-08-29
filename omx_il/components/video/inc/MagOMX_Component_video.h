#ifndef __MAGOMX_COMPONENT_VIDEO_H__
#define __MAGOMX_COMPONENT_VIDEO_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

DeclareClass(MagOmxComponentVideo, MagOmxComponentImpl);

ClassMembers(MagOmxComponentVideo, MagOmxComponentImpl, \
	
)
    MagMutexHandle         mhMutex;
    MagMiniDBHandle        mParametersDB;

EndOfClassMembers;

#endif