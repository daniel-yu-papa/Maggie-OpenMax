#ifndef __OMX_PRIV_H__
#define __OMX_PRIV_H__

#include "OMX_Core.h"
#include "maggie_list.h"

typedef Maggie_OMX_CompRegistration_t ** (*comp_reg_func_t) (int *);
typedef void (*comp_dereg_func_t) (void);

typedef struct{
    OMX_STRING name;
    OMX_U32    refCount;
    
    comp_reg_func_t   registFunc;
    comp_dereg_func_t deregistFunc;
}Maggie_OMX_Component_so_t;

typedef struct{
    Maggie_OMX_Component_so_t *so;
    OMX_ComponentInit initFunc;

    List_t instanceListHead;
}Maggie_OMX_Component_Obj_t;

typedef struct{
    List_t list;
    const OMX_STRING name;
}Maggie_OMX_Roles_t;

typedef struct{
    List_t list;
    const OMX_STRING name;
    List_t roles_list_head;
    Maggie_OMX_Component_Obj_t *obj; /*the component object body*/
}Maggie_OMX_Component_t;

typedef struct{
    List_t staticLoadCompListHead;
    OMX_U32 staticCompNumber;
    List_t dynamicLoadCompListHead;
    OMX_U32 dynamicCompNumber;
}Maggie_OMX_t;
/** construct and initialize component
 * This is called only upon @ref OMX_GetHandle(), to instantiate component.
 *
 * @param hComponent
 *        component handle allocated by core, freed upon @ref OMX_FreeHandle().
 * @param name
 *        unique OpenMAX IL standard component name.
 * @return
 *        OpenMAX IL return code.
 */
typedef OMX_ERRORTYPE (*OMX_ComponentInit)
(OMX_HANDLETYPE * hComponent, const OMX_STRING name);

typedef struct {
    OMX_STRING name;
    OMX_STRING *roles; /*end with NULL string*/
    OMX_ComponentInit init;
}Maggie_OMX_CompRegistration_t;

typedef struct comp_instance_obj_t{
    List_t node;
    void *compHandle;
}MagCompInstance_t;
OMX_API Maggie_OMX_CompRegistration_t * OMX_APIENTRY OMX_ComponentRegistration(void);

extern Maggie_OMX_t gMaggieOMX;

#endif