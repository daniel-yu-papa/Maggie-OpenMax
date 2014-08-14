/*
 * Author: YJ
 * Date: 2014-05-09
 * Implemention the OMX IL Spec Version 1.2.0 - chapter: 3.2.3 
 */
 
#include "MagOMX_IL_Core.h"

#define DEFAULT_OMX_COMP_LOAD_PATH "/system/lib/openmax" 

static MagOMX_IL_Core_t *gOmxCore = NULL;

static OMX_S32 addComponentList(const OMX_STRING  fileName,
                                      OMX_PTR           hLib,
                                      comp_reg_func_t   regFunc,
                                      comp_dereg_func_t deregFunc){
    MagOMX_Component_Registration_t *regInfo;
    List_t *tmpNode;
    Component_Entry_t *entry;
    OMX_U32 u = 0;
    
    regInfo = regFunc();
    if (NULL != regInfo){
        entry = (Component_Entry_t *)mag_mallocz(sizeof(Component_Entry_t));
        MAG_ASSERT(entry != NULL);
        
        INIT_LIST(&entry->node);
        entry->regInfo   = regInfo;
        entry->deregFunc = deregFunc;
        entry->libHandle = hLib;
        
        list_add_tail(&entry->node, &gOmxCore->LoadedCompListHead);

        for (u = 0; u < regInfo->roles_num; u++){
            gOmxCore->roleToComponentTable->addItem(gOmxCore->roleToComponentTable, entry, regInfo->roles[u]);
        }

        gOmxCore->componentToRoleTable->addItem(gOmxCore->componentToRoleTable, entry, regInfo->name);
        
        return 0;
    }else{
        AGILE_LOGE("failed to do regFunc[%p]()", regFunc);
        return -1;
    }
    
}

static void loadComponentLib(const OMX_STRING file, OMX_PTR arg){
    void *so;
    void *reg;
    void *dereg;
    OMX_S32 ret;
    
    so = dlopen(file, RTLD_NOW);

    if(NULL == so){
        AGILE_LOGE("failed to load the component lib: %s (error code: %s)", file, dlerror());
        return;
    }

    AGILE_LOGI("Loading the component lib: %s", file);
    
    reg   = dlsym(so, "MagOMX_Component_Registration");
    dereg = dlsym(so, "MagOMX_Component_Deregistration");

    ret = addComponentList(file, so, reg, dereg);
    if(ret == 0)
        *(int *)arg = *(int *)arg + 1;
}

static void loadComponentRecursive(OMX_STRING loadPath,
                                    void (*loader)(const OMX_STRING file, OMX_PTR arg),
                                    OMX_PTR arg){
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
            AGILE_LOGE("failed to read the dir: %s (err = %s)", loadPath, strerror(errno));
            continue;
        }
        
        if( (fileInfo->d_type == DT_DIR) && 
            !(!strcmp(fileInfo->d_name, ".") || !strcmp(fileInfo->d_name, ".."))){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            AGILE_LOGI("find dir: %s", dirPathFull);
            loadComponentRecursive(dirPathFull, loader, arg);
        }else if ((fileInfo->d_type == DT_REG) || (fileInfo->d_type == DT_LNK)){
            sprintf(dirPathFull, "%s/%s", loadPath, fileInfo->d_name);
            AGILE_LOGI("find file: %s", dirPathFull);
            (*loader)(dirPathFull, arg);
        }
    }while (fileInfo != NULL);

    closedir(dir);
}

static OMX_U32 loadComponents(const OMX_STRING searchPath){
    OMX_U32 components;

    loadComponentRecursive(searchPath, loadComponentLib, &components);
    return components;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Init(void){
    char    loadPath[PATH_MAX];
    OMX_U32 num;
    
    if (gOmxCore != NULL){
        AGILE_LOGW("MagOMX IL Core has been initialized!");
        return OMX_ErrorNone;
    }

    gOmxCore = mag_mallocz(sizeof(MagOMX_IL_Core_t));
    if (NULL != gOmxCore){
        INIT_LIST(&gOmxCore->LoadedCompListHead);
        gOmxCore->roleToComponentTable = createMagStrHashTable(128);
        if (NULL == gOmxCore->roleToComponentTable){
            AGILE_LOGE("failed to create hastable: roleToComponentTable!");
            mag_freep(&gOmxCore);
            return OMX_ErrorInsufficientResources;
        }
        gOmxCore->componentToRoleTable = createMagStrHashTable(128);
        if (NULL == gOmxCore->componentToRoleTable){
            AGILE_LOGE("failed to create hastable: componentToRoleTable!");
            mag_freep(&gOmxCore);
            return OMX_ErrorInsufficientResources;
        }
        return OMX_ErrorNone;
    }else{
        AGILE_LOGE("failed to malloc the MagOMX_IL_Core_t!");
        return OMX_ErrorInsufficientResources;
    }

    sprintf(loadPath, "%s", getenv("OMX_LOAD_PATH") ? getenv("OMX_LOAD_PATH") : DEFAULT_OMX_COMP_LOAD_PATH);

    num = loadComponents(loadPath);

    gOmxCore->LoadedCompNumber = num;
    Mag_CreateMutex(&gOmxCore->lock);
    
    return OMX_ErrorNone;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit(void){
    List_t *tmpNode;
    Component_Entry_t *comp;

    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }
    
    tmpNode = gOmxCore->LoadedCompListHead.next;

    while (tmpNode != &gOmxCore->LoadedCompListHead){
        comp = (Component_Entry_t *)list_entry(tmpNode, Component_Entry_t, node);
        ((OMX_COMPONENTTYPE *)comp->compHandle)->ComponentDeInit(comp->compHandle);
        list_del(tmpNode);
        if (NULL != comp->deregFunc){
            comp->deregFunc();
        }
        dlclose(comp->libHandle);
        mag_freep(&comp);
        
        tmpNode = gOmxCore->LoadedCompListHead.next;
    }

    destroyMagStrHashTable(gOmxCore->roleToComponentTable);
    destroyMagStrHashTable(gOmxCore->componentToRoleTable);
    Mag_DestroyMutex(gOmxCore->lock);
    
    mag_freep(&gOmxCore);
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex){
    
    OMX_U32 i;
    List_t *tmpNode;
    Component_Entry_t *comp;

    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() has not been done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }

    tmpNode = &gOmxCore->LoadedCompListHead;
    do {
        tmpNode = tmpNode->next;
        if (tmpNode == &gOmxCore->LoadedCompListHead){
            return OMX_ErrorNoMore;
        }
    }while((nIndex > 0) && nIndex--);
    
    comp = (Component_Entry_t *)list_entry(tmpNode, Component_Entry_t, node);
    strncpy(cComponentName, comp->regInfo->name, nNameLength);
    
    return OMX_ErrorNone;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(
    OMX_OUT OMX_HANDLETYPE* pHandle, 
    OMX_IN  OMX_STRING cComponentName,
    OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE* pCallBacks){

    List_t *tmpNode;
    Component_Entry_t *comp;
    OMX_ERRORTYPE ret;

    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }
    
    Mag_AcquireMutex(gOmxCore->lock);
    
    tmpNode = gOmxCore->LoadedCompListHead.next;

    while (tmpNode != &gOmxCore->LoadedCompListHead){
        comp = (Component_Entry_t *)list_entry(tmpNode, Component_Entry_t, node);

        if (!strcmp(comp->regInfo->name, cComponentName)){
            if (comp->regInfo->init){
                ret = comp->regInfo->init(pHandle, pAppData, pCallBacks);

                if (ret == OMX_ErrorNone){
                    comp->compHandle = *pHandle;
                    ret = OMX_SendCommand(*pHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
                    if (ret == OMX_ErrorNone){
                        AGILE_LOGI("Successful!");
                    }else{
                        AGILE_LOGE("Failed to set the state to OMX_StateLoaded, ret = 0x%x", ret);
                    }
                }else{
                    AGILE_LOGE("Failed to initialize the component(%s)", cComponentName);
                }

                Mag_ReleaseMutex(gOmxCore->lock);
                return ret;
            }
        }
    }
    AGILE_LOGE("Failed to find the component: %s", cComponentName);
    Mag_ReleaseMutex(gOmxCore->lock);
    
    return OMX_ErrorComponentNotFound;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(
    OMX_IN  OMX_HANDLETYPE hComponent){

    List_t *tmpNode;
    Component_Entry_t *comp;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }
    
    Mag_AcquireMutex(gOmxCore->lock);
    
    tmpNode = gOmxCore->LoadedCompListHead.next;
    
    while (tmpNode != &gOmxCore->LoadedCompListHead){
        comp = (Component_Entry_t *)list_entry(tmpNode, Component_Entry_t, node);

        if (comp->compHandle == hComponent){
            ret = ((OMX_COMPONENTTYPE *)hComponent)->ComponentDeInit(hComponent);

            if (ret != OMX_ErrorNone){
                AGILE_LOGE("Failed to do (0x%p)->ComponentDeInit(), ret = 0x%x", hComponent, ret);
                goto out;
            }

            list_del(tmpNode);
            if (NULL != comp->deregFunc){
                comp->deregFunc();
            }
            dlclose(comp->libHandle);
            mag_freep(&comp);
            goto out;
        }
    }
    ret = OMX_ErrorComponentNotFound;
    AGILE_LOGE("Failed to find the component: 0x%p", hComponent);

out:
    Mag_ReleaseMutex(gOmxCore->lock);
    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_SetupTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput){

    OMX_COMPONENTTYPE *comp_in;
    OMX_COMPONENTTYPE *comp_out;
    OMX_TUNNELSETUPTYPE tunnelSetup;
    OMX_ERRORTYPE ret;

    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }
    
    if ((NULL == hOutput) || (NULL == hInput)){
        AGILE_LOGE("invalid component: in_comp = 0x%p, out_comp = 0x%p", hInput, hOutput);
        return OMX_ErrorBadParameter;
    }

    Mag_AcquireMutex(gOmxCore->lock);
    
    comp_in  = (OMX_COMPONENTTYPE *)hInput;
    comp_out = (OMX_COMPONENTTYPE *)hOutput;

    tunnelSetup.eSupplier    = OMX_BufferSupplyUnspecified;
    tunnelSetup.nTunnelFlags = 0;

    ret = comp_out->ComponentTunnelRequest(comp_out, nPortOutput, comp_in, nPortInput, &tunnelSetup);
    if (ret == OMX_ErrorNone){
        ret = comp_in->ComponentTunnelRequest(comp_in, nPortInput, comp_out, nPortOutput, &tunnelSetup);
        if (ret == OMX_ErrorNone){
            AGILE_LOGD("connect the in_comp[0x%p:%d] <--> out_comp[0x%p:%d] OK!!!", comp_in, nPortInput, comp_out, nPortOutput);
        }else{
            AGILE_LOGE("failed to do ComponentTunnelRequest() of comp_out (connect the in_comp[0x%p:%d] --> out_comp[0x%p:%d])",
                        comp_in, nPortInput, comp_out, nPortOutput);

            /*Tear down the tunnel on Output Port*/
            ret = comp_out->ComponentTunnelRequest(comp_out, nPortOutput, NULL, kInvalidCompPortNumber, NULL);
            if (ret == OMX_ErrorNone){
                AGILE_LOGD("To tear down the component[0x%p, %d] tunnel OK!!!", comp_out, nPortOutput);
            }else{
                AGILE_LOGE("Failed to tear down the component[0x%p, %d] tunnel)", comp_out, nPortOutput);
            }
            Mag_ReleaseMutex(gOmxCore->lock);
            return ret;
        }
    }else{
        AGILE_LOGE("failed to do ComponentTunnelRequest() of comp_in (connect the in_comp[0x%p:%d] --> out_comp[0x%p:%d])",
                    comp_in, nPortInput, comp_out, nPortOutput);
        Mag_ReleaseMutex(gOmxCore->lock);
        return ret;
    }
    
    Mag_ReleaseMutex(gOmxCore->lock);
    return OMX_ErrorNone;
}
    
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_TeardownTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput){

    OMX_COMPONENTTYPE *comp_in;
    OMX_COMPONENTTYPE *comp_out;
    OMX_TUNNELSETUPTYPE tunnelSetup;
    OMX_ERRORTYPE ret;

    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }
    
    if ((NULL == hOutput) || (NULL == hInput)){
        AGILE_LOGE("invalid component: in_comp = 0x%p, out_comp = 0x%p", hInput, hOutput);
        return OMX_ErrorBadParameter;
    }
    
    Mag_AcquireMutex(gOmxCore->lock);
    
    comp_in  = (OMX_COMPONENTTYPE *)hInput;
    comp_out = (OMX_COMPONENTTYPE *)hOutput;

    tunnelSetup.eSupplier    = OMX_BufferSupplyUnspecified;
    tunnelSetup.nTunnelFlags = 0;

    ret = comp_in->ComponentTunnelRequest(comp_in, nPortInput, NULL, kInvalidCompPortNumber, &tunnelSetup);
    if (ret == OMX_ErrorNone){
        ret = comp_out->ComponentTunnelRequest(comp_out, nPortOutput, NULL, kInvalidCompPortNumber, &tunnelSetup);

        if (ret == OMX_ErrorNone){
            AGILE_LOGD("Tear down the in_comp[0x%p:%d] -/-> out_comp[0x%p:%d] OK!!!", comp_in, nPortInput, comp_out, nPortOutput);
        }else{
            AGILE_LOGE("failed to do ComponentTunnelRequest() of comp_out (Tear down the in_comp[0x%p:%d] -/-> out_comp[0x%p:%d])",
                        comp_in, nPortInput, comp_out, nPortOutput);
            Mag_ReleaseMutex(gOmxCore->lock);
            return ret;
        }
    }else{
        AGILE_LOGE("failed to do ComponentTunnelRequest() of comp_in (Tear down the in_comp[0x%p:%d] -/-> out_comp[0x%p:%d])",
                    comp_in, nPortInput, comp_out, nPortOutput);
        Mag_ReleaseMutex(gOmxCore->lock);
        return ret;
    }

    Mag_ReleaseMutex(gOmxCore->lock);
    return OMX_ErrorNone;
}
    
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentOfRoleEnum(
    OMX_OUT OMX_STRING compName,
    OMX_IN  OMX_STRING role,
    OMX_IN  OMX_U32 nIndex){

    Component_Entry_t *comp;
    
    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }

    comp = (Component_Entry_t *)gOmxCore->componentToRoleTable->getItem(gOmxCore->componentToRoleTable, compName);
    
    if (comp){
        if (nIndex < comp->regInfo->roles_num){
            strcpy(role, comp->regInfo->roles[nIndex]);
        }else{
            AGILE_LOGD("the component: %s has %d roles. the query index:%d is out of range.", 
                        compName, 
                        comp->regInfo->roles_num,
                        nIndex);
            return OMX_ErrorNoMore;
        }
    }else{
        AGILE_LOGE("failed to find out the component: %s", compName);
        return OMX_ErrorComponentNotFound;
    }
    return OMX_ErrorNone;
}

/* 
 * The role only has one component name depending on the adding time during OMX_Init(). 
 * It only keep the latest one.
 */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_RoleOfComponentEnum(
    OMX_OUT OMX_STRING role,
    OMX_IN  OMX_STRING compName,
    OMX_IN  OMX_U32 nIndex){

    Component_Entry_t *comp;
    
    if (!gOmxCore){
        AGILE_LOGE("the OMX_Init() is not done yet! quit...");
        return OMX_ErrorInsufficientResources;
    }

    comp = (Component_Entry_t *)gOmxCore->roleToComponentTable->getItem(gOmxCore->roleToComponentTable, role);
    
    if (comp){
        if (nIndex < 1){
            strcpy(compName, comp->regInfo->name);
        }else{
            AGILE_LOGD("the role: %s has only ONE component. the query index:%d is out of range.", 
                        role, 
                        nIndex);
            return OMX_ErrorNoMore;
        }
    }else{
        AGILE_LOGE("failed to find out the role: %s", role);
        return OMX_ErrorBadParameter;
    }
    return OMX_ErrorNone;
}

/*
 *returns a new core extension interface by an extension name
 */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetCoreInterface(
    OMX_OUT void ** ppItf,
    OMX_IN OMX_STRING cExtensionName){
    
    return OMX_ErrorNotImplemented;
}

OMX_API void OMX_APIENTRY OMX_FreeCoreInterface(
    OMX_IN void * pItf){

    return OMX_ErrorNotImplemented;
}


