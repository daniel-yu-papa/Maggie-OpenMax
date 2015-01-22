#include "MagOmx_Port_FFmpeg_Clk.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

AllocateClass(MagOmxPort_FFmpeg_Clk, MagOmxPortClock);

static OMX_ERRORTYPE virtual_FFmpeg_Clk_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	if (nSizeBytes != 0){
		AGILE_LOGE("no buffer is allocated, the size[%d] should be 0", nSizeBytes);
	}

	/*don't pre-allocate the memory, it uses the ffmpeg decoded AVFrame*/
	*ppBuffer = NULL;
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Clk_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
    /*don't free any buffer*/
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Clk_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	AGILE_LOGD("[invalid action]proceed buffer header: 0x%x", pBufHeader);
	return OMX_ErrorNone;
}


/*Class Constructor/Destructor*/
static void MagOmxPort_FFmpeg_Clk_initialize(Class this){
	AGILE_LOGV("Enter!");

	MagOmxPort_FFmpeg_ClkVtableInstance.MagOmxPortClock.MagOmxPortImpl.MagOMX_AllocateBuffer        = virtual_FFmpeg_Clk_AllocateBuffer;
	MagOmxPort_FFmpeg_ClkVtableInstance.MagOmxPortClock.MagOmxPortImpl.MagOMX_FreeBuffer            = virtual_FFmpeg_Clk_FreeBuffer;
	MagOmxPort_FFmpeg_ClkVtableInstance.MagOmxPortClock.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer = virtual_FFmpeg_Clk_ProceedReturnedBuffer;
}

static void MagOmxPort_FFmpeg_Clk_constructor(MagOmxPort_FFmpeg_Clk thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxPort_FFmpeg_Clk));
    chain_constructor(MagOmxPort_FFmpeg_Clk, thiz, params);
}

static void MagOmxPort_FFmpeg_Clk_destructor(MagOmxPort_FFmpeg_Clk thiz, MagOmxPort_FFmpeg_ClkVtable vtab){
}