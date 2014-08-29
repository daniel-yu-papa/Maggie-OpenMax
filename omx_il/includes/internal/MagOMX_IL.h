#ifndef __MAG_OMX_IL_H__
#define __MAG_OMX_IL_H__

#include "OMX_Core.h"

/*Mag OMX IL Version definitions*/
#define kVersionMajor           1
#define kVersionMinor           2
#define kVersionRevision        0
#define kVersionStep            0

#define kComponentVersion       0x00000001

/*Mag OMX IL port number range definitions*/
#define kInvalidCompPortNumber  0
#define kCompPortStartNumber    1

#define STRINGIFY(x) case x: return #x


/** all parameters/configs structures start with this structure. */
typedef struct
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
} MagOMX_Param_Header_t;

typedef struct  {
    OMX_U32 nGroupPriority;
    OMX_U32 nGroupID;
} MagOMX_Param_PRIORITYMGMTTYPE_t;

typedef struct{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
}MagOMX_Port_Header_t;

/** construct and initialize component
 * This is called only upon @ref OMX_GetHandle(), to instantiate component.
 *
 * @param hComponent
 *        component handle allocated by core, freed upon @ref OMX_FreeHandle().
 * @param pAppData
 *        The private data input from IL client.
 * @param pCallBacks
 *        The callbacks to IL client.
 *
 * @return
 *        OpenMAX IL return code.
 */
typedef OMX_ERRORTYPE (*MagOMX_Component_Init)(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                               OMX_IN  OMX_PTR pAppData,
                                               OMX_IN  OMX_CALLBACKTYPE* pCallBacks);


/** MagOMX component registration data structure. */
typedef struct
{
    OMX_STRING            name;      /*OMX IL standard component name*/
    OMX_STRING            *roles;    /*OMX IL standard component roles*/
    OMX_U32               roles_num; /*the number of the component roles*/
    MagOMX_Component_Init init;      /*OMX IL component constructor. */
} MagOMX_Component_Registration_t;

/*do component registration. 
 * it is called upon OMX_Init().
 */
MagOMX_Component_Registration_t *MagOMX_Component_Registration (void);

/* revert the component registration
 * when implemented, this gets called upon OMX_Deinit().
 */
void MagOMX_Component_Deregistration (void);

static inline void setSpecVersion(OMX_VERSIONTYPE* pSpecVersion) {
    pSpecVersion->s.nVersionMajor = kVersionMajor;
    pSpecVersion->s.nVersionMinor = kVersionMinor;
    pSpecVersion->s.nRevision     = kVersionRevision;
    pSpecVersion->s.nStep         = kVersionStep;
}

static inline void setComponentVersion(OMX_VERSIONTYPE* pComponentVersion) {
    pComponentVersion->nVersion   = kComponentVersion;
}

static inline void initHeader(void *obj, OMX_U32 size){
    MagOMX_Param_Header_t *header = (MagOMX_Param_Header_t *obj);

    header->nSize = size;
    header->s.nVersionMajor = kVersionMajor;
    header->s.nVersionMinor = kVersionMinor;
    header->s.nRevision     = kVersionRevision;
    header->s.nStep         = kVersionStep;
}

static inline OMX_U32 getPortIndex(OMX_PTR *pStruct){
    MagOMX_Port_Header_t *h = (MagOMX_Port_Header_t *)pStruct;

    return h->nPortIndex;
}

static inline OMX_STRING OmxState2String(OMX_STATETYPE state) {
    switch (state) {
        STRINGIFY(OMX_StateReserved_0x00000000);
        STRINGIFY(OMX_StateLoaded);
        STRINGIFY(OMX_StateIdle);
        STRINGIFY(OMX_StateExecuting);
        STRINGIFY(OMX_StatePause);
        STRINGIFY(OMX_StateWaitForResources);
        STRINGIFY(OMX_StateKhronosExtensions);
        STRINGIFY(OMX_StateVendorStartUnused);
        STRINGIFY(OMX_StateMax);
        default: return "state - unknown";
    }
}

static inline OMX_STRING OmxParameter2String(OMX_INDEXTYPE nIndex) {
    switch (nIndex) {
        STRINGIFY(OMX_IndexParamAudioInit);
        STRINGIFY(OMX_IndexParamImageInit);
        STRINGIFY(OMX_IndexParamVideoInit);
        STRINGIFY(OMX_IndexParamOtherInit);
        STRINGIFY(OMX_IndexParamNumAvailableStreams);
        STRINGIFY(OMX_IndexConfigTunneledPortStatus);
        STRINGIFY(OMX_IndexParamActiveStream);
        STRINGIFY(OMX_StateVendorStartUnused);
        STRINGIFY(OMX_StateMax);
        default: return "parameter - unknown";
    }
}

#endif
