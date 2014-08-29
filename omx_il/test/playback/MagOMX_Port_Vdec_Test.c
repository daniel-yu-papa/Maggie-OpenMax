#include "MagOMX_Port_Vdec_Test.h"

AllocateClass(MagOmxPort_VdecTest, MagOmxPortVideo);

static OMX_ERRORTYPE virtual_VdecTest_AllocateBuffer(OMX_HANDLETYPE port, OMX_U8 **ppBuffer, OMX_U32 nSizeBytes){
	OMX_U8 *buf;

	buf = (OMX_U8 *)mag_mallocz(nSizeBytes);
	if (buf){
		*ppBuffer = buf;
		return OMX_ErrorNone;
	}else{
		return OMX_ErrorInsufficientResources;
	}
}

static OMX_ERRORTYPE virtual_VdecTest_FreeBuffer(OMX_HANDLETYPE port, OMX_U8 *pBuffer){
	mag_free(pBuffer);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_VdecTest_ProceedReturnedBuffer(OMX_HANDLETYPE port, OMX_BUFFERHEADERTYPE* pBufHeader){
	AGILE_LOGD("proceed buffer header: 0x%x", pBufHeader);
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxPort_VdecTest_initialize(Class this){
	MagOmxPort_VdecTestVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_AllocateBuffer        = virtual_VdecTest_AllocateBuffer;
	MagOmxPort_VdecTestVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_FreeBuffer            = virtual_VdecTest_FreeBuffer;
	MagOmxPort_VdecTestVtableInstance.MagOmxPortVideo.MagOmxPortImpl.MagOMX_ProceedReturnedBuffer = virtual_VdecTest_ProceedReturnedBuffer;
}

static void MagOmxPort_VdecTest_constructor(MagOmxPort_VdecTest thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxPort_VdecTest));
    chain_constructor(MagOmxPort_VdecTest, thiz, params);
}

static void MagOmxPort_VdecTest_destructor(MagOmxPort_VdecTest thiz, MagOmxPort_VdecTestVtable vtab){
}