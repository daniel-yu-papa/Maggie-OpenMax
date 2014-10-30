#include "MagOMX_Component_audio.h"

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
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);

    switch (nParamIndex){
        case OMX_IndexConfigTimeRenderingDelay:
        {   
            OMX_TIME_CONFIG_RENDERINGDELAYTYPE *output = (OMX_TIME_CONFIG_RENDERINGDELAYTYPE *)pComponentParameterStructure;
            if (MagOmxComponentImplVirtual(base)->MagOMX_GetRenderDelay){
                ret = MagOmxComponentImplVirtual(base)->MagOMX_GetRenderDelay(hComponent, &output->nRenderingDelay);
            }else{
                COMP_LOGE(root, "The pure virtual function MagOMX_GetRenderDelay() is not overrided!");
                ret = OMX_ErrorNotImplemented;
            }
        }
            break;

        default:
            return OMX_ErrorUnsupportedIndex;
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
    	case OMX_IndexConfigTimeUpdate:
            OMX_TIME_MEDIATIMETYPE *mt = (OMX_TIME_MEDIATIMETYPE *)pComponentParameterStructure;

            if (mt->eUpdateType == OMX_TIME_UpdateClockStateChanged){
                if (MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus){
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus(hComponent, 
                                                                                   OMX_TIME_UpdateClockStateChanged, 
                                                                                   (OMX_S32)(mt->eState));
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(root, "Failed to do MagOMX_SetAVSyncStatus(OMX_TIME_UpdateClockStateChanged), ret = 0x%x", ret);
                    }
                }else{
                    COMP_LOGE(root, "pure virtual func: MagOMX_SetAVSyncStatus() is not overrided!!");
                }
            }else if (mt->eUpdateType == OMX_TIME_UpdateRequestFulfillment){
                if (MagOmxComponentImplVirtual(base)->MagOMX_DoAVSync){
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_DoAVSync(hComponent, mt);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(root, "Failed to do MagOMX_DoAVSync(), ret = 0x%x", ret);
                    }
                }else{
                    COMP_LOGE(root, "pure virtual func: MagOMX_DoAVSync() is not overrided!!");
                }
            }else if (mt->eUpdateType == OMX_TIME_UpdateScaleChanged){
                if (MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus){
                    ret = MagOmxComponentImplVirtual(base)->MagOMX_SetAVSyncStatus(hComponent, 
                                                                                   OMX_TIME_UpdateScaleChanged, 
                                                                                   mt->xScale);
                    if (ret != OMX_ErrorNone){
                        COMP_LOGE(root, "Failed to do MagOMX_SetAVSyncStatus(OMX_TIME_UpdateScaleChanged), ret = 0x%x", ret);
                    }
                }else{
                    COMP_LOGE(root, "pure virtual func: MagOMX_SetAVSyncStatus() is not overrided!!");
                }
            }else{
                COMP_LOGE(root, "OMX_IndexConfigTimeUpdate: UpdateType %d is not correct!", mt->eUpdateType);
            }
            
            break;

        case OMX_IndexConfigTimeActiveRefClockUpdate:
            OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *refClock = (OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *)pComponentParameterStructure;
            if (MagOmxComponentImplVirtual(base)->MagOMX_SetRefClock){
                ret = MagOmxComponentImplVirtual(base)->MagOMX_SetRefClock(hComponent, refClock);
                if (ret != OMX_ErrorNone){
                    COMP_LOGE(root, "Unable to be Reference Clock Provider, ret = 0x%x", ret);
                }
            }else{
                COMP_LOGE(root, "pure virtual func: MagOMX_SetRefClock() is not overrided!!");
            }
            break;

    	default:
    		break;
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
}

static void MagOmxComponentAudio_constructor(MagOmxComponentAudio thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentAudio));
    chain_constructor(MagOmxComponentAudio, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
    thiz->mParametersDB = createMagMiniDB(16);
}

static void MagOmxComponentAudio_destructor(MagOmxComponentAudio thiz, MagOmxComponentAudioVtable vtab){
	AGILE_LOGV("Enter!");

	destroyMagMiniDB(&thiz->mParametersDB);
    Mag_DestroyMutex(&thiz->mhMutex);
}