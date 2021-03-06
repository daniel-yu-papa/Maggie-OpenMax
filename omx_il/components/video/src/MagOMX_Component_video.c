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

#include "MagOMX_Component_video.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVideo"

AllocateClass(MagOmxComponentVideo, MagOmxComponentImpl);

static MagOMX_Component_Type_t virtual_MagOmxComponentVideo_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Video;
}

static OMX_ERRORTYPE virtual_MagOmxComponentVideo_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
    MagOmxComponent     root;
    MagOmxComponentImpl base;
    MagOmxComponentVideo thiz;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);
    thiz = ooc_cast(hComponent, MagOmxComponentVideo);

    Mag_AcquireMutex(thiz->mhMutex);

    if (MagOmxComponentVideoVirtual(thiz)->MagOmx_Video_GetParameter){
        ret = MagOmxComponentVideoVirtual(thiz)->MagOmx_Video_GetParameter(hComponent, nParamIndex, pComponentParameterStructure);
    }else{
        COMP_LOGE(root, "The pure virtual function MagOmx_Video_GetParameter() is not overrided!");
        ret = OMX_ErrorUnsupportedIndex;
    }
    
    Mag_ReleaseMutex(thiz->mhMutex);
      
    return ret;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentVideo_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	MagOmxComponentVideo thiz;
    MagOmxComponent      root;
	MagOmxComponentImpl  base;
	OMX_ERRORTYPE ret = OMX_ErrorNone;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentVideo);
    root = ooc_cast(hComponent, MagOmxComponent);
    base = ooc_cast(hComponent, MagOmxComponentImpl);
    
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
            break;
        }

    	default:
        {
            if (MagOmxComponentVideoVirtual(thiz)->MagOmx_Video_SetParameter){
                ret = MagOmxComponentVideoVirtual(thiz)->MagOmx_Video_SetParameter(hComponent, nIndex, pComponentParameterStructure);
            }else{
                COMP_LOGE(root, "The pure virtual function MagOmx_Video_SetParameter() is not overrided!");
                ret = OMX_ErrorUnsupportedIndex;
            }
        }
    		break;
    }

    Mag_ReleaseMutex(thiz->mhMutex);

	return ret;
}

/*Class Constructor/Destructor*/
static void MagOmxComponentVideo_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentVideoVtableInstance.MagOmxComponentImpl.MagOMX_getType      = virtual_MagOmxComponentVideo_getType;
    MagOmxComponentVideoVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter = virtual_MagOmxComponentVideo_GetParameter;
    MagOmxComponentVideoVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter = virtual_MagOmxComponentVideo_SetParameter;
}

static void MagOmxComponentVideo_constructor(MagOmxComponentVideo thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentVideo));
    chain_constructor(MagOmxComponentVideo, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
}

static void MagOmxComponentVideo_destructor(MagOmxComponentVideo thiz, MagOmxComponentVideoVtable vtab){
	AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
}