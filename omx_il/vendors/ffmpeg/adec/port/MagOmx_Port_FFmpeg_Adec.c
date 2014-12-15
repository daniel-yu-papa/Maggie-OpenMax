#include "MagOmx_Port_FFmpeg_Adec.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

AllocateClass(MagOmxPort_FFmpeg_Adec, MagOmxPortAudio);

static OMX_ERRORTYPE virtual_FFmpeg_Adec_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	if (nSizeBytes != 0){
		AGILE_LOGE("no buffer is allocated, the size[%d] should be 0", nSizeBytes);
	}

	/*don't pre-allocate the memory, it uses the ffmpeg decoded AVFrame*/
	*ppBuffer = NULL;
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
    /*don't free any buffer*/
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Adec_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	AGILE_LOGD("[invalid action]proceed buffer header: 0x%x", pBufHeader);
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_FFmpeg_Adec_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	MagOmxPort_FFmpeg_AdecVtableInstance.MagOmxPortAudio.MagOmxPortImpl.MagOMX_AllocateBuffer        = virtual_FFmpeg_Adec_AllocateBuffer;
	MagOmxPort_FFmpeg_AdecVtableInstance.MagOmxPortAudio.MagOmxPortImpl.MagOMX_FreeBuffer            = virtual_FFmpeg_Adec_FreeBuffer;
	MagOmxPort_FFmpeg_AdecVtableInstance.MagOmxPortAudio.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer = virtual_FFmpeg_Adec_ProceedReturnedBuffer;
}

static void MagOmxPort_FFmpeg_Adec_constructor(MagOmxPort_FFmpeg_Adec thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxPort_FFmpeg_Adec));
    chain_constructor(MagOmxPort_FFmpeg_Adec, thiz, params);
}

static void MagOmxPort_FFmpeg_Adec_destructor(MagOmxPort_FFmpeg_Adec thiz, MagOmxPort_FFmpeg_AdecVtable vtab){
}