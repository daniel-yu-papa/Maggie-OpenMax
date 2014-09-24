#include "MagOMX_Component_video.h"

AllocateClass(MagOmxComponentVideo, MagOmxComponentImpl);

static MagOMX_Component_Type_t virtual_MagOmxComponentVideo_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Video;
}

static OMX_ERRORTYPE virtual_MagOmxComponentVideo_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentVideo_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	return OMX_ErrorNone;
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
    thiz->mParametersDB = createMagMiniDB(16);
}

static void MagOmxComponentVideo_destructor(MagOmxComponentVideo thiz, MagOmxComponentVideoVtable vtab){
	AGILE_LOGV("Enter!");

	destroyMagMiniDB(&thiz->mParametersDB);
    Mag_DestroyMutex(&thiz->mhMutex);
}