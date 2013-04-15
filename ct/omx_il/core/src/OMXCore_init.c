#include "OMX_Core.h"
#include "OMX_Priv.h"

#include "agilelog.h"

static Maggie_OMX_CompRegistration_t *ComponentsStaticRegTable[] = {
    NULL
};

static Maggie_OMX_t gMaggieOMX;

static OMX_U32 releaseComponentsStatic(void){
    List_t *tmpNode;

    tmpNode = gMaggieOMX.staticLoadCompListHead.next;
    while(tmpNode != &gMaggieOMX.staticLoadCompListHead){
        Maggie_OMX_Component_t *omxName = (Maggie_OMX_Component_t *)tmpNode;
        List_t *tmpRoleNode = omxName->roles_list_head.next;
        while(tmpRoleNode != &omxName->roles_list_head){
            Maggie_OMX_Roles_t *role = (Maggie_OMX_Roles_t *)tmpRoleNode;
            list_del(tmpRoleNode);
            free(role->name);
            free(role);
            tmpRoleNode = omxName->roles_list_head.next;
        }
        
        list_del(tmpNode);
        free(omxName->name);
        free(omxName->obj);
        free(omxName);
        tmpNode = gMaggieOMX.staticLoadCompListHead.next;
    }

    
}

static OMX_S32 loadComponentsStatic(void){
    OMX_U8 index = 0;
    OMX_U8 i = 0;
    Maggie_OMX_Component_t *newComp;
    Maggie_OMX_Roles_t *newRole;

    List_t *tmpNode;
    
    while (ComponentsStaticRegTable[index]){
        tmpNode = gMaggieOMX.staticLoadCompListHead.next;
        Maggie_OMX_Component_t *omxName = (Maggie_OMX_Component_t *)tmpNode;
        while(tmpNode != gMaggieOMX.staticLoadCompListHead.next){
            if (!strcmp(ComponentsStaticRegTable[index].name, omxName->name)){
                AGILE_LOGE("the duplicate component name(%s). stop to do loadComponentsStatic()", omxName->name);
                goto failure;
            }
        }
        newComp = (Maggie_OMX_Component_t *)malloc(sizeof(Maggie_OMX_Component_t));
        newComp->name = strdup(ComponentsStaticRegTable[index].name);
        
        INIT_LIST(&newComp->roles_list_head);
skip_dup_role:
        while(ComponentsStaticRegTable[index].roles[i]){
            tmpNode = newComp->roles_list_head.next;
            while (tmpNode != &newComp->roles_list_head){
                Maggie_OMX_Roles_t *omxRole = (Maggie_OMX_Roles_t *)tmpNode;
                if (!strcmp(ComponentsStaticRegTable[index].roles[i], omxRole->name)){
                    AGILE_LOGE("the duplicate role(%s). skip it!", omxRole->name);
                    i++;
                    goto skip_dup_role;
                }
            }
            newRole = (Maggie_OMX_Roles_t *)malloc(sizeof(Maggie_OMX_Roles_t));
            list_add(newRole, &newComp->roles_list_head);
            newRole->name = strdup(ComponentsStaticRegTable[index].roles[i]);
            i++;
        }

        newComp->obj = (Maggie_OMX_Component_Obj_t *)malloc(sizeof(Maggie_OMX_Component_Obj_t));
        newComp->obj->initFunc = ComponentsStaticRegTable[index].init;
        newComp->obj->so       = NULL;
        index++;
    }
    return index;
    
failure:
    releaseComponentsStatic();
    return -1;
}

static OMX_S32 loadComponentsDynamic(OMX_STRING searchPath){

}

OMX_ERRORTYPE OMX_Init (void){
    OMX_U32 components = 0;

    INIT_LIST(&gMaggieOMX.dynamicLoadCompListHead);
    INIT_LIST(&gMaggieOMX.staticLoadCompListHead);
    
    components += loadComponentsStatic();
    if(components < 0)
        return OMX_ErrorInvalidComponentName;
    
    loadComponentsDynamic();

    
}

OMX_ERRORTYPE OMX_Deinit (void){

}

OMX_ERRORTYPE OMX_ComponentNameEnum (
    OMX_STRING cComponentName,
    OMX_U32 nNameLength,
    OMX_U32 nIndex){

}



