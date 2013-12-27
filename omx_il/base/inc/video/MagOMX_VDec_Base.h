#ifndef __MAG_OMX_VDEC_BASE_H__
#define __MAG_OMX_VDEC_BASE_H__

#include "ooc.h"

DeclareClass(MagOmx_VDec_Base, Base);

Virtuals(MagOmx_VDec_Base, Base)
    /*pure virtual functions and must be overrided*/
    OMX_ERRORTYPE (*Start)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*Stop)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*Resume)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*Pause)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*Prepare)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*Preroll)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*AllResourcesReady)(OMX_IN  MagOmx_VDec_Base hVdec);

    OMX_ERRORTYPE (*FreeResources)(OMX_IN  MagOmx_VDec_Base hVdec);
EndOfVirtuals;

ClassMembers(MagOmx_VDec_Base, Base \
)

EndOfClassMembers;

#endif
