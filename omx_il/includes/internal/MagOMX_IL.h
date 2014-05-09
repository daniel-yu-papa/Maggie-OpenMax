#ifndef __MAG_OMX_IL_H__
#define __MAG_OMX_IL_H__

#include "OMX_Core.h"

#define kInvalidCompPortNumber  0

#define kCompPortStartNumber    1


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
    OMX_STRING        name;      /*OMX IL standard component name*/
    OMX_STRING        *roles;    /*OMX IL standard component roles, end with NULL string*/
    OMX_U32           roles_num; /*the number of the component roles*/
    OMX_VERSIONTYPE   version;   /*OMX IL component version*/
    OMX_ComponentInit init;      /*OMX IL component constructor. */
} MagOMX_Component_Registration_t;

/*do component registration. 
 * it is called upon OMX_Init().
 */
MagOMX_Component_Registration_t *MagOMX_Component_Registration ();

/* revert the component registration
 * when implemented, this gets called upon OMX_Deinit().
 */
void MagOMX_Component_Deregistration (void);

#endif
