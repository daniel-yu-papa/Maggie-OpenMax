/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

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
    
    Mag_AcquireMutex(thiz->mhMutex);

    if (MagOmxComponentAudioVirtual(thiz)->MagOmx_Audio_GetParameter){
        ret = MagOmxComponentAudioVirtual(thiz)->MagOmx_Audio_GetParameter(hComponent, nParamIndex, pComponentParameterStructure);
    }else{
        COMP_LOGE(root, "The pure virtual function MagOmx_Audio_GetParameter() is not overrided!");
        ret = OMX_ErrorUnsupportedIndex;
    }
    
    Mag_ReleaseMutex(thiz->mhMutex);
     
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
            break;
    }

    Mag_ReleaseMutex(thiz->mhMutex);

	return ret;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentAudio_SetRefClock(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *rc){

    MagOmxComponentAudio thiz;

    thiz = ooc_cast(hComponent, MagOmxComponentAudio);

    thiz->mEnableRefClockUpdate  = rc->bEnableRefClockUpdates;
    thiz->mRefTimeUpdateInterval = rc->nRefTimeUpdateInterval / 1000;  /*in ms*/
    thiz->mAllFrameDuration      = 0;
    COMP_LOGD(ooc_cast(hComponent, MagOmxComponent), "mRefTimeUpdateInterval: %d ms", thiz->mRefTimeUpdateInterval);
}

static void MagOmxComponentAudio_updateRefTime(MagOmxComponentAudio thiz, OMX_TICKS timeStamp, OMX_U32 frame_duration){
    MagOmxComponentImpl audioCompImpl;
    MagOmxPort clockPort;
    OMX_TIME_CONFIG_TIMESTAMPTYPE refTime;

    audioCompImpl = ooc_cast(thiz, MagOmxComponentImpl);

    COMP_LOGD(ooc_cast(thiz, MagOmxComponent), "mAllFrameDuration: %d, frame_duration: %d", thiz->mAllFrameDuration, frame_duration);

    thiz->mAllFrameDuration += frame_duration;
    if (thiz->mEnableRefClockUpdate && thiz->mAllFrameDuration >= thiz->mRefTimeUpdateInterval){
        clockPort = ooc_cast(audioCompImpl->mhClockPort, MagOmxPort);

        initHeader(&refTime, sizeof(OMX_TIME_CONFIG_TIMESTAMPTYPE));
        refTime.nTimestamp = timeStamp;
        MagOmxPortVirtual(clockPort)->SetParameter(clockPort, 
                                                   OMX_IndexConfigTimeCurrentReference, 
                                                   &refTime);
        thiz->mAllFrameDuration = 0;
    }
}


/*Class Constructor/Destructor*/
static void MagOmxComponentAudio_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_getType      = virtual_MagOmxComponentAudio_getType;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter = virtual_MagOmxComponentAudio_GetParameter;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter = virtual_MagOmxComponentAudio_SetParameter;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_SetRefClock  = virtual_MagOmxComponentAudio_SetRefClock;

    MagOmxComponentAudioVtableInstance.MagOmx_Audio_GetParameter = NULL;
    MagOmxComponentAudioVtableInstance.MagOmx_Audio_SetParameter = NULL;
}

static void MagOmxComponentAudio_constructor(MagOmxComponentAudio thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentAudio));
    chain_constructor(MagOmxComponentAudio, thiz, params);

    thiz->updateRefTime = MagOmxComponentAudio_updateRefTime;

    Mag_CreateMutex(&thiz->mhMutex);

    thiz->mEnableRefClockUpdate  = OMX_FALSE;
    thiz->mRefTimeUpdateInterval = 0;
    thiz->mAllFrameDuration      = 0;
}

static void MagOmxComponentAudio_destructor(MagOmxComponentAudio thiz, MagOmxComponentAudioVtable vtab){
	AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
}