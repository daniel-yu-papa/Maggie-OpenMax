#include "MagOMX_Component_Vdec_Test.h"
#include "MagOMX_Component_Disp_Test.h"

#define OMX_IL_INPUT_BUFFER_NUMBER 4
#define OMX_IL_INPUT_BUFFER_SIZE   512

typedef struct {
	OMX_BUFFERHEADERTYPE *bufHeader;
	OMX_U32 bufIndex;
	OMX_BOOL isBusy;
}Buffer_t;

static Buffer_t inBuffer[OMX_IL_INPUT_BUFFER_NUMBER];

static OMX_BOOL gIsStopped = OMX_FALSE;

OMX_CALLBACKTYPE vDecCallbacks = { .EventHandler    = vDecEventHandler,
                                   .EmptyBufferDone = vDecEmptyBufferDone,
                                   .FillBufferDone  = vDecFillBufferDone,
};

OMX_CALLBACKTYPE vDidplayCallbacks = { .EventHandler    = vDisplayEventHandler,
                                       .EmptyBufferDone = vDisplayEmptyBufferDone,
                                       .FillBufferDone  = vDisplayFillBufferDone,
};

/* VDEC Callbacks implementation */
OMX_ERRORTYPE vDecEventHandler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 Data1,
    OMX_U32 Data2,
    OMX_PTR pEventData) ,

    AGILE_LOGE("Vdec event: %d", eEvent);
    if(eEvent == OMX_EventCmdComplete) {
    	if (Data1 == OMX_CommandStateSet) {
        	AGILE_LOGD("OMX_CommandStateSet");
        	switch ((int)Data2) {
	        	case OMX_StateMax:
	        	    AGILE_LOGD("OMX_StateMax\n");
	        	    break;
	        	case OMX_StateLoaded:
	        	    AGILE_LOGD("OMX_StateLoaded\n");
	        	    break;
	        	case OMX_StateIdle:
	        	    AGILE_LOGD("OMX_StateIdle\n");
	        	    break;
	        	case OMX_StateExecuting:
	        	    AGILE_LOGD("OMX_StateExecuting\n");
	        	    break;
	        	case OMX_StatePause:
	        	    AGILE_LOGD("OMX_StatePause\n");
	        	    break;
	        	case OMX_StateWaitForResources:
	        	    AGILE_LOGD("OMX_StateWaitForResources\n");
	        	    break;
        	}
        }else if (Data1 == OMX_CommandPortEnable){
		    AGILE_LOGD("Vdec component enables port %d is done!", Data2);
	    }else if (Data1 == OMX_CommandPortDisable){
      		AGILE_LOGD("Vdec component disables port %d is done!", Data2);
    	}else if (Data1 == OMX_CommandFlush){
    		AGILE_LOGD("Vdec component flushes port %d is done!", Data2);
    	}
    }else{
    	AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE vDecEmptyBufferDone(
	OMX_HANDLETYPE hComponent,
	OMX_PTR pAppData,
    OMX_BUFFERHEADERTYPE* pBuffer) {

	OMX_U32 index = *(OMX_U32 *)(pAppData);

	AGILE_LOGD("get buffer %d returned, buffer header: 0x%x", index, pBuffer);
	inBuffer[index].isBusy = OMX_FALSE;

	if (!gIsStopped){
		inBuffer[index].isBusy = OMX_TRUE;
		OMX_EmptyThisBuffer(hComponent, pBuffer);
	}
}

OMX_ERRORTYPE vDecFillBufferDone(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_BUFFERHEADERTYPE* pBuffer){

	OMX_U32 index = *(OMX_U32 *)(pAppData);

	AGILE_LOGD("get buffer %d returned, buffer header: 0x%x", index, pBuffer);
	inBuffer[index].isBusy = OMX_FALSE;

	if (!gIsFlushing){
		inBuffer[index].isBusy = OMX_TRUE;
		OMX_FillThisBuffer(hComponent, pBuffer);
	}
}

/* VDisplay Callbacks implementation */
OMX_ERRORTYPE vDisplayEventHandler(
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 Data1,
    OMX_U32 Data2,
    OMX_PTR pEventData) ,

    AGILE_LOGE("Vdisplay event: %d", eEvent);
    if(eEvent == OMX_EventCmdComplete) {
    	if (Data1 == OMX_CommandStateSet) {
        	AGILE_LOGD("OMX_CommandStateSet");
        	switch ((int)Data2) {
	        	case OMX_StateMax:
	        	    AGILE_LOGD("OMX_StateMax\n");
	        	    break;
	        	case OMX_StateLoaded:
	        	    AGILE_LOGD("OMX_StateLoaded\n");
	        	    break;
	        	case OMX_StateIdle:
	        	    AGILE_LOGD("OMX_StateIdle\n");
	        	    break;
	        	case OMX_StateExecuting:
	        	    AGILE_LOGD("OMX_StateExecuting\n");
	        	    break;
	        	case OMX_StatePause:
	        	    AGILE_LOGD("OMX_StatePause\n");
	        	    break;
	        	case OMX_StateWaitForResources:
	        	    AGILE_LOGD("OMX_StateWaitForResources\n");
	        	    break;
        	}
        }else if (Data1 == OMX_CommandPortEnable){
		    AGILE_LOGD("Vdisplay component enables port %d is done!", Data2);
	    }else if (Data1 == OMX_CommandPortDisable){
      		AGILE_LOGD("Vdisplay component disables port %d is done!", Data2);
    	}else if (Data1 == OMX_CommandFlush){
    		AGILE_LOGD("Vdisplay component flushes port %d is done!", Data2);
    	}
    }else{
    	AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

int main(int argc, char** argv){
	OMX_ERRORTYPE err;
	OMX_HANDLETYPE *hVdec;
	OMX_HANDLETYPE *hDisplay;
	OMX_PORT_PARAM_TYPE vDecPortParam;
	OMX_PORT_PARAM_TYPE vDispPortParam;
	OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
	OMX_U32 i;

    initHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

	err = OMX_Init();
    if(err != OMX_ErrorNone) {
    	AGILE_LOGE("OMX_Init() failed");
	    exit(1);
    }

    err = OMX_GetHandle(&hVdec, "OMX.Mag.vdec.test", NULL /*appPriv */, &vDecCallbacks);
    if(err != OMX_ErrorNone) {
	    AGILE_LOGE("OMX_GetHandle OMX.Mag.vdec.test failed");
	    exit(1);
    }

    err = OMX_GetHandle(&hDisplay, "OMX.Mag.display.test", NULL /*appPriv */, &vDidplayCallbacks);
    if(err != OMX_ErrorNone) {
	    AGILE_LOGE("OMX_GetHandle OMX.Mag.display.test failed");
	    exit(1);
    }

    initHeader(&vDecPortParam, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(hVdec, OMX_IndexParamVideoInit, &vDecPortParam);
    if(err != OMX_ErrorNone){
	    AGILE_LOGE("Error in getting vDec OMX_PORT_PARAM_TYPE parameter");
	    exit(1);
    }

    for (i = vDecPortParam.nStartPortNumber; i < vDecPortParam.nPorts; i++){
    	sPortDef.nPortIndex = i;
    	err = OMX_GetParameter(hVdec, OMX_IndexParamPortDefinition, &sPortDef);
    	if(err != OMX_ErrorNone){
		    AGILE_LOGE("Error in getting vDec OMX_PARAM_PORTDEFINITIONTYPE %d parameter", i);
		    exit(1);
	    }

	    if (sPortDef.eDir == OMX_DirInput){
	    	sPortDef.nBufferCountActual = OMX_IL_INPUT_BUFFER_NUMBER;
	    	sPortDef.nBufferSize = OMX_IL_INPUT_BUFFER_SIZE;
	    }else{
	    	sPortDef.nBufferCountActual = 2;
	    	sPortDef.nBufferSize = 1024;
	    }

	    err = OMX_SetParameter(hVdec, OMX_IndexParamPortDefinition, &sPortDef);
	    if(err != OMX_ErrorNone){
		    AGILE_LOGE("Error in setting OMX_PARAM_PORTDEFINITIONTYPE %d parameter", i);
		    exit(1);
	    }
    }

    initHeader(&vDispPortParam, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(hDisplay, OMX_IndexParamVideoInit, &vDispPortParam);
    if(err != OMX_ErrorNone){
	    AGILE_LOGE("Error in getting vDisplay OMX_PORT_PARAM_TYPE parameter");
	    exit(1);
    }

    for (i = vDispPortParam.nStartPortNumber; i < vDispPortParam.nPorts; i++){
    	sPortDef.nPortIndex = i;
    	err = OMX_GetParameter(hDisplay, OMX_IndexParamPortDefinition, &sPortDef);
    	if(err != OMX_ErrorNone){
		    AGILE_LOGE("Error in getting vDisplay OMX_PARAM_PORTDEFINITIONTYPE %d parameter", i);
		    exit(1);
	    }

	    if (sPortDef.eDir == OMX_DirInput){
	    	sPortDef.nBufferCountActual = 2;
	    	sPortDef.nBufferSize = 512;
	    }

	    err = OMX_SetParameter(hDisplay, OMX_IndexParamPortDefinition, &sPortDef);
	    if(err != OMX_ErrorNone){
		    AGILE_LOGE("Error in setting OMX_PARAM_PORTDEFINITIONTYPE %d parameter", i);
		    exit(1);
	    }
    }

    err = OMX_SetupTunnel(hVdec, vDecPortParam.nStartPortNumber + 1, 
    	                  hDisplay, vDispPortParam.nStartPortNumber);
    if(err != OMX_ErrorNone){
    	AGILE_LOGE("Error in setting up tunnel between vDec[port: %d] and vDisplay[port: %d]",
    		        vDecPortParam.nStartPortNumber + 1, vDispPortParam.nStartPortNumber);
	    exit(1);
    }

    for (i = 0; i < OMX_IL_INPUT_BUFFER_NUMBER; i++){
    	inBuffer[i].bufIndex = i;
    	inBuffer[i].isBusy   = OMX_FALSE;
    	err = OMX_AllocateBuffer(hVdec, &inBuffer[i].bufHeader, vDecPortParam.nStartPortNumber, &inBuffer[i].bufIndex, OMX_IL_INPUT_BUFFER_SIZE);
	    if (err != OMX_ErrorNone) {
		    AGILE_LOGE("Error on Allocate %dth Buffer", i);
		    exit(1);
	    }
    }

    OMX_SendCommand(hVdec, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_SendCommand(hDisplay, OMX_CommandStateSet, OMX_StateIdle, NULL);

    while (1){
        printf("p: play, s: stop, q: exit the program.\n");
        scanf("%c",&c);
        printf("\ninput is '%c'\n", c);

        if (c == 'p'){
        	gIsStopped = OMX_FALSE;
        	OMX_SendCommand(hVdec, OMX_CommandStateSet, OMX_StateExecuting, NULL);
		    OMX_SendCommand(hDisplay, OMX_CommandStateSet, OMX_StateExecuting, NULL);

		    for (i = 0; i < OMX_IL_INPUT_BUFFER_NUMBER; i++){
		    	inBuffer[i].isBusy = OMX_TRUE;
				OMX_EmptyThisBuffer(hVdec, inBuffer[i].bufHeader);
				usleep(10000);
		    }
        }else if (c == 's'){
        	gIsStopped = OMX_TRUE;
        	OMX_SendCommand(hVdec, OMX_CommandStateSet, OMX_StateIdle, NULL);
		    OMX_SendCommand(hDisplay, OMX_CommandStateSet, OMX_StateIdle, NULL);
        }else if (c == 'q'){
        	if (!gIsStopped){
        		OMX_SendCommand(hVdec, OMX_CommandStateSet, OMX_StateIdle, NULL);
			    OMX_SendCommand(hDisplay, OMX_CommandStateSet, OMX_StateIdle, NULL);  
        	}

        	OMX_SendCommand(hVdec, OMX_CommandStateSet, OMX_StateLoaded, NULL);
		    OMX_SendCommand(hDisplay, OMX_CommandStateSet, OMX_StateLoaded, NULL);

		    for (i = 0; i < OMX_IL_INPUT_BUFFER_NUMBER; i++){
			    OMX_FreeBuffer(hVdec, vDecPortParam.nStartPortNumber, inBuffer[i].bufHeader);
		    }

		    OMX_FreeHandle(hVdec);
		    OMX_FreeHandle(hDisplay);

		    OMX_Deinit();
		    break;
        }
    }
}