#ifndef __MAG_OMX_IL_CORE_H__
#define __MAG_OMX_IL_CORE_H__

#include "framework/MagFramework.h"
#include "MagOMX_IL.h"

typedef MagOMX_Component_Registration_t *(*comp_reg_func_t) ();
typedef void (*comp_dereg_func_t) (OMX_HANDLETYPE hComponent);

typedef struct {
    List_t            node;
    OMX_PTR           libHandle;
    OMX_HANDLETYPE    compHandle;
    MagOMX_Component_Registration_t *regInfo;
    comp_dereg_func_t deregFunc;
    OMX_BOOL          initialized;
}Component_Entry_t;

typedef struct{
    List_t  LoadedCompListHead;
    OMX_U32 LoadedCompNumber;

    HashTableHandle roleToComponentTable;
    HashTableHandle componentToRoleTable;

    MagMutexHandle  lock;
}MagOMX_IL_Core_t;

#endif