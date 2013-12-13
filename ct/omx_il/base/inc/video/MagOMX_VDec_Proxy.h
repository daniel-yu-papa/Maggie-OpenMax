#ifndef __MAG_OMX_VDEC_PROXY_H__
#define __MAG_OMX_VDEC_PROXY_H__

#include "MagOMX_VDec_Base.h"

DeclareClass(MagOmx_VDec_Proxy, MagOmx_VDec_Base);

Virtuals(MagOmx_VDec_Proxy, MagOmx_VDec_Base)
    
EndOfVirtuals;

ClassMembers(MagOmx_VDec_Proxy, MagOmx_VDec_Base, \
    void (*loadVdec)(MagOmx_VDec_Proxy thiz); \  
)
    MagOmx_VDec_Base mVDecObj; //the real vdec object
EndOfClassMembers;

#endif

