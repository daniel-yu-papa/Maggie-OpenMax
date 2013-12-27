#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "OMX_Core.h"
#include "OMX_Priv.h"

#include "agilelog.h"

#define DEFAULT_OMX_COMP_LOAD_PATH "/system/lib/openmax" 

static Maggie_OMX_CompRegistration_t *ComponentsStaticRegTable[] = {
    NULL
};

Maggie_OMX_t gMaggieOMX;

static OMX_U32 releaseComponents(List_t *head){
    List_t *tmpNode;

    tmpNode = head->next;
    while(tmpNode != head){
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
        if (omxName->obj->so){
            free(omxName->obj->so->name);
            free(omxName->obj->so);
        }
        free(omxName->obj);
        free(omxName);
        tmpNode = head->next;
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
            tmpNode = tmpNode->next;
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
                tmpNode = tmpNode->next;
            }
            newRole = (Maggie_OMX_Roles_t *)malloc(sizeof(Maggie_OMX_Roles_t));
            list_add(newRole, &newComp->roles_list_head);
            newRole->name = strdup(ComponentsStaticRegTable[index].roles[i]);
            i++;
        }

        newComp->obj = (Maggie_OMX_Component_Obj_t *)malloc(sizeof(Maggie_OMX_Component_Obj_t));
        newComp->obj->initFunc = ComponentsStaticRegTable[index].init;
        INIT_LIST(&newComp->obj->instanceListHead);
        newComp->obj->so       = NULL;
        index++;
    }
    return index;
    
failure:
    releaseComponents(&gMaggieOMX.staticLoadCompListHead);
    return -1;
}

static OMX_S32 addComponentList(const OMX_STRING fileName,
                                      comp_reg_func_t regFunc,
                                      comp_dereg_func_t deregFunc){
    Maggie_OMX_CompRegistration_t **regInfo;
    OMX_U8 compNum;
    List_t *tmpNode;
    OMX_U8 i = 0;
    OMX_U8 loop = 0;
    Maggie_OMX_Component_t *newComp;
    Maggie_OMX_Roles_t *newRole;
    
    regInfo = regFunc(&compNum);

    for (i = 0; i < compNum; i++){
        /*Scan the Dynamic loaded component list to make sure the component name is unique*/
        tmpNode = gMaggieOMX.dynamicLoadCompListHead.next;

        while(tmpNode != &gMaggieOMX.dynamicLoadCompListHead){
            Maggie_OMX_Component_t *omxComp = (Maggie_OMX_Component_t *)tmpNode;
            if (!strcmp(omxComp->name, regInfo[i]->name)){
                AGILE_LOGE("the duplicate component name(%s). stop to do loadComponentsDynamic", omxComp->name);
                goto failure;
            }
            tmpNode = tmpNode->next;
        }
        newComp = (Maggie_OMX_Component_t *)malloc(sizeof(Maggie_OMX_Component_t));
        newComp->name = strdup(regInfo[i]->name);

        INIT_LIST(&newComp->roles_list_head);
        loop = 0;
skip_dup_role:
        while(regInfo[i]->roles[loop]){
            tmpNode = newComp->roles_list_head.next;
            while (tmpNode != &newComp->roles_list_head){
                Maggie_OMX_Roles_t *omxRole = (Maggie_OMX_Roles_t *)tmpNode;
                if (!strcmp(regInfo[i]->roles[loop], omxRole->name)){
                    AGILE_LOGE("the duplicate role(%s). skip it!", omxRole->name);
                    loop++;
                    goto skip_dup_role;
                }
                tmpNode = tmpNode->next;
            }
            newRole = (Maggie_OMX_Roles_t *)malloc(sizeof(Maggie_OMX_Roles_t));
            list_add(newRole, &newComp->roles_list_head);
            newRole->name = strdup(regInfo[i]->roles[loop]);
            loop++;
        }

        newComp->obj = (Maggie_OMX_Component_Obj_t *)malloc(sizeof(Maggie_OMX_Component_Obj_t));
        newComp->obj->initFunc = regInfo[i]->init;
        INIT_LIST(&newComp->obj->instanceListHead);
        newComp->obj->so       = (Maggie_OMX_Component_so_t *)malloc(sizeof(Maggie_OMX_Component_so_t));
        newComp->obj->so->name = strdup(fileName);
        newComp->obj->so->refCount   = 0;
        newComp->obj->so->registFunc = regFunc;
        newComp->obj->so->deregistFunc = deregFunc;
    }
    return compNum;
    
failure:
    releaseComponents(&gMaggieOMX.dynamicLoadCompListHead);
    return -1;
}

static void loadComponentLib(const OMX_STRING file, void *arg){
    void *so;
    void *reg;
    void *dereg;
    OMX_S32 ret;
    
    so = dlopen(file, RTLD_LAZY);

    if(NULL == so){
        AGILE_LOGE("failed to load the component lib: %s (error code: %s)", file, dlerror());
        return;
    }

    AGILE_LOGI("Loading the component lib: %s", file);
    
    reg   = dlsym(so, "OMX_ComponentRegistration");
    dereg = dlsym(so, "OMX_ComponentDeregistration");

    ret = addComponentList(file, reg, dereg);
    if(ret > 0)
        *(int *)arg = *(int *)arg + ret;

    dlclose(file);
}

static void loadComponentRecursive(char *loadPath,
                                              void (*loader)(const char *file, void *arg),
                                              void *arg){
    DIR *dir;
    struct dirent *fileInfo;
    char dirPathFull[PATH_MAX];
    
    dir = opendir(loadPath);

    if (NULL == dir){
        AGILE_LOGE("failed to open the dir: %s (err = %s)", loadPath, strerror(errno));
        return;
    }

    do{
        fileInfo = readdir(dir);

        if (NULL == fileInfo){
            //AGILE_LOGE("failed to read the dir: %s (err = %s)", loadPath, strerror(errno));
            continue;
        }
        
        if( (fileInfo->d_type == DT_DIR) && 
            !(!strcmp(fileInfo->d_name, ".") || !strcmp(fileInfo->d_name, ".."))){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            //AGILE_LOGI("find dir: %s", dirPathFull);
            loadComponentRecursive(dirPathFull, loader, arg);
        }else if ((fileInfo->d_type == DT_REG) || (fileInfo->d_type == DT_LNK)){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            //AGILE_LOGI("find file: %s", dirPathFull);
            (*loader)(dirPathFull, arg);
        }
    }while (fileInfo != NULL);

    closedir(dir);
}

static OMX_S32 loadComponentsDynamic(const OMX_STRING searchPath){
    OMX_U8 components;

    loadComponentRecursive(searchPath, loadComponentLib, &components);
    return components;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Init(void){
    OMX_U32 components = 0;
    OMX_U32 num;
    OMX_U8  loadPath[PATH_MAX];
    
    INIT_LIST(&gMaggieOMX.dynamicLoadCompListHead);
    INIT_LIST(&gMaggieOMX.staticLoadCompListHead);
    
    num = loadComponentsStatic();
    if(num < 0)
        return OMX_ErrorInvalidComponentName;
    gMaggieOMX.staticCompNumber = num;
    components += num;
    
    sprintf(loadPath, "%s", 
             getenv("OMX_LOAD_PATH") ? getenv("OMX_LOAD_PATH") : DEFAULT_OMX_COMP_LOAD_PATH);
    num = loadComponentsDynamic(loadPath);
    gMaggieOMX.dynamicCompNumber = num;
    components += num;
    
    AGILE_LOGI("total %d components are loaded", components);

    return OMX_ErrorNone;  
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit(void){

}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex){

    List_t *listHead;
    OMX_U32 lindex = 0;
    List_t *tmpNode;

    if (nNameLength > OMX_MAX_STRINGNAME_SIZE || NULL == cComponentName)
    {
        return OMX_ErrorBadParameter;
    }
    
    if ((gMaggieOMX.staticCompNumber + gMaggieOMX.dynamicCompNumber) < (nIndex + 1)){
        return OMX_ErrorNoMore;
    }
    
    if(nIndex < gMaggieOMX.staticCompNumber){
        listHead = &gMaggieOMX.staticLoadCompListHead; 
    }else{        
        nIndex -= gMaggieOMX.staticCompNumber;
        listHead = &gMaggieOMX.dynamicLoadCompListHead;
    }

    tmpNode = listHead->next;

    while(tmpNode != listHead){
        if (lindex == nIndex){
            Maggie_OMX_Component_t *omxComp = (Maggie_OMX_Component_t *)tmpNode;
            /*Only copy "nNameLength" chars to the "cComponentName". 
                     *if the length of "cComponentName" is larger than "nNameLength", the "cComponentName" would be truncated*/
            strncpy(cComponentName, omxComp->name, nNameLength);
            return OMX_ErrorNone;
        }
        tmpNode = tmpNode->next;
        ++lindex;
    }
    return OMX_ErrorNoMore;
}



