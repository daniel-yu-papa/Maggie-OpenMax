#ifndef __OMX_CPA_BASE_H__
#define __OMX_CPA_BASE_H__

#include "CPA/CPA_ContentPipe.h"

#define CPA_VERSION_MAJOR    1
#define CPA_VERSION_MINOR    0
#define CPA_VERSION_REVISION 0

/*it is the pure virtual class and could only be used after the inheritance*/
CLASS(OMXCPABase_t)
#define OMXCPABase_t_FIELDS \
    CPA_PIPETYPE *pCPA; \
    CPA_STRING   szURI;
ENDCLASS(OMXCPABase_t)

CPA_RESULTTYPE OMXCPA_base_Constructor(CPA_INOUT OMXCPABase_t **hPipe);
#endif