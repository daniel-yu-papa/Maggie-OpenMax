#include "MagOMX_Component_audio.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompAudio"

AllocateClass(MagOmxComponentAudio, MagOmxComponentImpl);

static MagOMX_Component_Type_t virtual_MagOmxComponentAudio_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Audio;
}

static OMX_ERRORTYPE virtual_MagOmxComponentAudio_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	MagOmxComponent     root;
    MagOmxComponentImpl base;
    MagOmxComponentAudio thiz;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);
    thiz = ooc_cast(hComponent, MagOmxComponentAudio);
        
    if (MagOmxComponentAudioVirtual(thiz)->MagOmx_Audio_GetParameter){
        ret = MagOmxComponentAudioVirtual(thiz)->MagOmx_Audio_GetParameter(hComponent, nParamIndex, pComponentParameterStructure);
    }else{
        COMP_LOGE(root, "The pure virtual function MagOmx_Audio_GetParameter() is not overrided!");
        ret = OMX_ErrorUnsupportedIndex;
    }
        
    return ret;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentAudio_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	MagOmxComponent      root;
    MagOmxComponentAudio thiz;
    MagOmxComponentImpl  base;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentAudio);
    base = ooc_cast(hComponent, MagOmxComponentImpl);
    root = ooc_cast(hComponent, MagOmxComponent);

    Mag_AcquireMutex(thiz->mhMutex);

    switch (nIndex){
        case OMX_IndexConfigTimeActiveRefClockUpdate:
        {
            OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *refClock = (OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *)pComponentParameterStructure;
            if (MagOmxComponentImplVirtual(base)->MagOMX_SetRefClock){
                ret = MagOmxComponentImplVirtual(base)->MagOMX_SetRefClock(hComponent, refClock);
                if (ret != OMX_ErrorNone){
                    COMP_LOGE(root, "Unable to be Reference Clock Provider, ret = 0x%x", ret);
                }
            }else{
                COMP_LOGE(root, "pure virtual func: MagOMX_SetRefClock() is not overrided!!");
            }
        }
            break;

    	default:
    	{
            if (MagOmxComponentAudioVirtual(thiz)->MagOmx_Audio_SetParameter){
                ret = MagOmxComponentAudioVirtual(thiz)->MagOmx_Audio_SetParameter(hComponent, nIndex, pComponentParameterStructure);
            }else{
                COMP_LOGE(root, "The pure virtual function MagOmx_Audio_SetParameter() is not overrided!");
                ret = OMX_ErrorUnsupportedIndex;
            }
        }
            return ret;
    }

    Mag_ReleaseMutex(thiz->mhMutex);

	return ret;
}

/*Class Constructor/Destructor*/
static void MagOmxComponentAudio_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_getType      = virtual_MagOmxComponentAudio_getType;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter = virtual_MagOmxComponentAudio_GetParameter;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter = virtual_MagOmxComponentAudio_SetParameter;

    MagOmxComponentAudioVtableInstance.MagOmx_Audio_GetParameter = NULL;
    MagOmxComponentAudioVtableInstance.MagOmx_Audio_SetParameter = NULL;
}

static void MagOmxComponentAudio_constructor(MagOmxComponentAudio thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentAudio));
    chain_constructor(MagOmxComponentAudio, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
}

static void MagOmxComponentAudio_destructor(MagOmxComponentAudio thiz, MagOmxComponentAudioVtable vtab){
	AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
}