#include "MagOmx_Component_FFmpeg_Clk.h"
#include "MagOmx_Port_FFmpeg_Clk.h"
#include "MagOMX_IL.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompVendor"

#define COMPONENT_NAME   "OMX.Mag.clock.ffmpeg"
#define ROLE_NAME        OMX_ROLE_CLOCK_BINARY
#define START_PORT_INDEX kCompPortStartNumber
#define PORT_NUMBER      MAX_CLOCK_PORT_NUMBER

AllocateClass(MagOmxComponent_FFmpeg_Clk, MagOmxComponentClock);

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxComponent                clkComp;

	clkComp     = ooc_cast(hComponent, MagOmxComponent);
	clkComp->setName(clkComp, (OMX_U8 *)COMPONENT_NAME);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	OMX_U32 i;
	OMX_HANDLETYPE clkOutPort;
	MagOmxComponentImpl clkCompImpl;
	MagOmxComponent_FFmpeg_Clk clkComp;

	AGILE_LOGV("clock component enter!");

	clkCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	clkComp     = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Clk);
	
	for (i = 0; i < clkComp->mPortIndex; i++){
		clkOutPort = clkCompImpl->getPort(clkCompImpl, i);
		ooc_delete((Object)clkOutPort);
	}

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_Clock_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_HANDLETYPE hDestPort){
	
	AGILE_LOGV("Invalid operation for MagOmxComponent_FFmpeg_Clk class");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_DecideStartTime(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_TICKS* pStartTimeList,
                    OMX_IN  OMX_U32    startTimeMap,  /*((1 << n) & startTimeMap) == (1 << n): the array index nth has the start time*/
                    OMX_OUT OMX_TICKS* pFinalStartTime){
	OMX_U32 i = 0;
	OMX_TICKS startTime = -1;

	do{
		if ( (startTimeMap & (1 << i)) ){
            /*get the smallest time value as the playback kick-off time*/
			if ((startTime == -1) || (startTime > pStartTimeList[i])){
				startTime = pStartTimeList[i];
			}
            startTimeMap &= ~(1 << i);
		}
		i++;
	}while(startTimeMap);

	*pFinalStartTime = startTime;

	if (startTime == -1){
		AGILE_LOGE("No start time could be decided!");
		return OMX_ErrorUndefined;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_GetOffset(
    				OMX_IN  OMX_HANDLETYPE hComponent,
    				OMX_OUT OMX_TICKS *offset){
	/*in us unit*/
	*offset = 1000;
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_AddPort(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U32 *pPortIdx){
	MagOmxPort_FFmpeg_Clk          clkOutPort;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponent_FFmpeg_Clk     clkComp;
	MagOmxComponentImpl            clkCompImpl;

	AGILE_LOGV("enter!");

	clkComp     = ooc_cast(hComponent, MagOmxComponent_FFmpeg_Clk);
	clkCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);

	param.portIndex    = clkComp->mPortIndex;
	param.isInput      = OMX_FALSE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-Out-%d", CLOCK_PORT_NAME, param.portIndex + 1);

	ooc_init_class(MagOmxPort_FFmpeg_Clk);
	clkOutPort = ooc_new(MagOmxPort_FFmpeg_Clk, &param);
	MAG_ASSERT(clkOutPort);
	
	clkCompImpl->addPort(clkCompImpl, param.portIndex, clkOutPort);
	*pPortIdx = param.portIndex;
	clkComp->mPortIndex++;

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Clock_RemovePort(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_U32 portIdx){
	return OMX_ErrorNone;
}


/*Class Constructor/Destructor*/
static void MagOmxComponent_FFmpeg_Clk_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_Clock_GetComponentUUID;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Prepare           = virtual_Clock_Prepare;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Preroll           = virtual_Clock_Preroll;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Start             = virtual_Clock_Start;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Stop              = virtual_Clock_Stop;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Pause             = virtual_Clock_Pause;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Resume            = virtual_Clock_Resume;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Deinit            = virtual_Clock_Deinit;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_Reset             = virtual_Clock_Reset;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_Clock_ComponentRoleEnum;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_Clock_ProceedBuffer;

    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOMX_Clock_DecideStartTime = virtual_Clock_DecideStartTime;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOMX_Clock_GetOffset       = virtual_Clock_GetOffset;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOMX_Clock_AddPort         = virtual_Clock_AddPort;
    MagOmxComponent_FFmpeg_ClkVtableInstance.MagOmxComponentClock.MagOMX_Clock_RemovePort      = virtual_Clock_RemovePort;
}

static void MagOmxComponent_FFmpeg_Clk_constructor(MagOmxComponent_FFmpeg_Clk thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_FFmpeg_Clk));
    chain_constructor(MagOmxComponent_FFmpeg_Clk, thiz, params);

    thiz->mPortIndex = START_PORT_INDEX;
}

static void MagOmxComponent_FFmpeg_Clk_destructor(MagOmxComponent_FFmpeg_Clk thiz, MagOmxComponent_FFmpeg_ClkVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Clk_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_FFmpeg_Clk hClkComp;
	MagOmxComponentImpl        parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_FFmpeg_Clk);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hClkComp = (MagOmxComponent_FFmpeg_Clk) ooc_new( MagOmxComponent_FFmpeg_Clk, (void *)param);
    MAG_ASSERT(hClkComp);

    parent = ooc_cast(hClkComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hClkComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hClkComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_FFmpeg_Clk_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_FFmpeg_Clk hClkComp;

	AGILE_LOGV("MagOmxComponent_FFmpeg_Clk_DeInit enter!");
	hClkComp = (MagOmxComponent_FFmpeg_Clk)compType->pComponentPrivate;
	ooc_delete((Object)hClkComp);

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {ROLE_NAME, NULL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_FFmpeg_Clk_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_FFmpeg_Clk_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef ROLE_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER