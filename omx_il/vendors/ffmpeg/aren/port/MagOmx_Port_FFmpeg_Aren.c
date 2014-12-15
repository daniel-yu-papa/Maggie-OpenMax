#include "MagOmx_Port_FFmpeg_Aren.h"

#include "libavutil/frame.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

AllocateClass(MagOmxPort_FFmpeg_Aren, MagOmxPortAudio);

static OMX_ERRORTYPE virtual_FFmpeg_Aren_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	if (nSizeBytes != 0){
		AGILE_LOGE("no buffer is allocated, the size[%d] should be 0", nSizeBytes);
	}

	/*don't pre-allocate the memory, it uses the ffmpeg decoded AVFrame*/
	*ppBuffer = NULL;
	
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
    /*don't free any buffer*/
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_FFmpeg_Aren_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	AVFrame *frame;

	AGILE_LOGD("proceed buffer header: 0x%x", pBufHeader);
	if (pBufHeader == NULL){
		AGILE_LOGE("wrong pBufHeader is NULL!");
		return OMX_ErrorBadParameter;
	}

	/*release the used audio frame buffer*/
	frame = (AVFrame *)pBufHeader->pBuffer;
	av_frame_unref(frame);

	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_FFmpeg_Aren_initialize(Class this){
	AGILE_LOGV("Enter!");

	MagOmxPort_FFmpeg_ArenVtableInstance.MagOmxPortAudio.MagOmxPortImpl.MagOMX_AllocateBuffer        = virtual_FFmpeg_Aren_AllocateBuffer;
	MagOmxPort_FFmpeg_ArenVtableInstance.MagOmxPortAudio.MagOmxPortImpl.MagOMX_FreeBuffer            = virtual_FFmpeg_Aren_FreeBuffer;
	MagOmxPort_FFmpeg_ArenVtableInstance.MagOmxPortAudio.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer = virtual_FFmpeg_Aren_ProceedReturnedBuffer;
}

static void MagOmxPort_FFmpeg_Aren_constructor(MagOmxPort_FFmpeg_Aren thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxPort_FFmpeg_Aren));
    chain_constructor(MagOmxPort_FFmpeg_Aren, thiz, params);
}

static void MagOmxPort_FFmpeg_Aren_destructor(MagOmxPort_FFmpeg_Aren thiz, MagOmxPort_FFmpeg_ArenVtable vtab){
}