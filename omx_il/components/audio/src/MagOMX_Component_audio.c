#include "MagOMX_Component_audio.h"

AllocateClass(MagOmxComponentAudio, MagOmxComponentImpl);

static MagOMX_Component_Type_t virtual_MagOMX_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Audio;
}

static OMX_ERRORTYPE virtual_MagOMX_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOMX_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){

	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponentAudio_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_getType      = virtual_MagOMX_getType;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter = virtual_MagOMX_GetParameter;
    MagOmxComponentAudioVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter = virtual_MagOMX_SetParameter;
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