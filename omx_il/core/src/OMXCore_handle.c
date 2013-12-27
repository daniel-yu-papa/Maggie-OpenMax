#include <string.h>

Maggie_OMX_Component_t *getRegCompByName(OMX_IN OMX_STRING name){
    List_t *tmpNode;
    Maggie_OMX_Component_t *pComp = NULL;
    OMX_S32 ret;
    
    tmpNode = gMaggieOMX.staticLoadCompListHead.next;
    
    while (tmpNode != &gMaggieOMX.staticLoadCompListHead){
        pComp = (Maggie_OMX_Component_t *)list_entry(tmpNode, Maggie_OMX_Component_t, list);
        ret = strncmp(pComp->name, name, OMX_MAX_STRINGNAME_SIZE);

        if(0 == ret)
            return pComp;

        tmpNode = tmpNode->next;
    }

    tmpNode = gMaggieOMX.dynamicLoadCompListHead.next;

    while (tmpNode != &gMaggieOMX.dynamicLoadCompListHead){
        pComp = (Maggie_OMX_Component_t *)list_entry(tmpNode, Maggie_OMX_Component_t, list);
        ret = strncmp(pComp->name, name, OMX_MAX_STRINGNAME_SIZE);

        if(0 == ret)
            return pComp;

        tmpNode = tmpNode->next;
    }

    return NULL;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(
        OMX_OUT OMX_HANDLETYPE * pHandle,
        OMX_IN OMX_STRING cComponentName,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_CALLBACKTYPE * pCallBacks
        ){
    OMX_U8 i = 0;
    Maggie_OMX_Component_t *pComp;
    Maggie_OMX_CompRegistration_t *pRegComp;
    OMX_ERRORTYPE omx_ret = OMX_ErrorNone;
        
    pComp = getRegCompByName(cComponentName);

    if (NULL = pComp){
        AGILE_LOGD("failed to find the component as the name is %s", cComponentName);
        * pHandle = NULL;
        
        return OMX_ErrorInvalidComponentName;
    }
    
    if (NULL != pComp->obj->so){
        if (pComp->obj->so->registFunc){
            pRegComp = pComp->obj->so->registFunc();

            if(NULL != pRegComp){
                pComp->obj->so->refCount++;
                omx_ret = pRegComp->init(pHandle, cComponentName);
            }
        }else{
            AGILE_LOGE("no registration func is found for component lib: %s", pComp->obj->so->name);
            omx_ret = OMX_ErrorNotImplemented;
        }
    }else{
        if (pComp->obj->initFunc){
            omx_ret = pComp->obj->initFunc(pHandle, cComponentName);
        }else{
            AGILE_LOGE("no init func is found for static component: %s", pComp->name);
            omx_ret = OMX_ErrorNotImplemented;
        }
    }

    if (OMX_ErrorNone == omx_ret){
        MagCompInstance_t *inst;
        ((OMX_COMPONENTTYPE *)*pHandle)->SetCallbacks(*pHandle, pCallBacks, pAppData);

        inst = (MagCompInstance_t *)malloc(sizeof(MagCompInstance_t));
        if (NULL == inst){
            AGILE_LOGE("no memory for malloc(MagCompInstance_t)");
            omx_ret = OMX_ErrorInsufficientResources;
            goto err_nomem;
        }
        INIT_LIST(&inst->node);
        list_add_tail(&inst->node, &pComp->obj->instanceListHead);
    }

err_nomem:
    return omx_ret;
}

