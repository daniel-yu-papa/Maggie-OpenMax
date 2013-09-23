#ifndef __MAGOMX_COMMON_H__
#define __MAGOMX_COMMON_H__

#include "Mag_mem.h"

static const OMX_U8  kSpecVersionMajor = 1;
static const OMX_U8  kSpecVersionMinor = 1;
static const OMX_U8  kSpecRevision = 2;
static const OMX_U8  kSpecStep = 0;
static const OMX_U32 kSpecVersion = 0x00020101;

inline void MagOmx_Common_InitHeader(OMX_U8 *pOmxStruct, OMX_U32 size){
    OMX_U32 *p = (OMX_U32 *)pOmxStruct;
    *(p + 0) = size;
    pOmxStruct[sizeof(OMX_U32)] = kSpecVersionMajor;
    pOmxStruct[sizeof(OMX_U32) + 1] = kSpecVersionMinor;
    pOmxStruct[sizeof(OMX_U32) + 2] = kSpecRevision;
    pOmxStruct[sizeof(OMX_U32) + 3] = kSpecStep;
}

#endif