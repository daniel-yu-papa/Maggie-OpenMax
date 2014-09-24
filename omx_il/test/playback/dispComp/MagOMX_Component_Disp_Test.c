#include "MagOMX_Component_Disp_Test.h"
#include "MagOMX_Port_Disp_Test.h"
#include "MagOMX_IL.h"

#define COMPONENT_NAME "OMX.Mag.display.test"
#define ROLE_NAME      "video_display.all"
#define START_PORT_INDEX 0
#define PORT_NUMBER      1

AllocateClass(MagOmxComponent_DispTest, MagOmxComponentVideo);

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_DispTest dispInPort;
	MagOmxPort dispInPortRoot;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponentImpl dispCompImpl;
	MagOmxComponent     dispComp;

	AGILE_LOGV("enter!");

	param.portIndex    = START_PORT_INDEX;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", DISP_PORT_NAME);
	
	ooc_init_class(MagOmxPort_DispTest);
	dispInPort = ooc_new(MagOmxPort_DispTest, &param);
	MAG_ASSERT(dispInPort);
	dispInPortRoot = ooc_cast(dispInPort, MagOmxPort);

	dispCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	dispComp = ooc_cast(hComponent, MagOmxComponent);

	dispComp->setName(dispComp, (OMX_U8 *)COMPONENT_NAME);
	dispCompImpl->addPort(dispCompImpl, 0, dispInPort);
	dispCompImpl->setupPortDataFlow(dispCompImpl, dispInPort, NULL);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_GetComponentUUID(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_UUIDTYPE* pComponentUUID){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Prepare(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Preroll(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Start(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Stop(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Deinit(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	OMX_HANDLETYPE dispInPort;
	MagOmxComponentImpl dispCompImpl;

	AGILE_LOGV("Disp enter!");
	dispCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	dispInPort  = dispCompImpl->getPort(dispCompImpl, START_PORT_INDEX + 0);

	ooc_delete((Object)dispInPort);
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_Reset(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	AGILE_LOGV("Enter!");
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_ComponentRoleEnum(
                    OMX_IN OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U8 *cRole,
                    OMX_IN OMX_U32 nIndex){
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE virtual_ProceedBuffer(
                    OMX_IN  OMX_HANDLETYPE hComponent, 
                    OMX_IN  OMX_BUFFERHEADERTYPE *srcbufHeader,
                    OMX_IN  OMX_BUFFERHEADERTYPE *destbufHeader){
	AGILE_LOGV("display buffer: 0x%x", srcbufHeader);
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_DispTest_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_GetComponentUUID;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Prepare           = virtual_Prepare;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Preroll           = virtual_Preroll;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Start             = virtual_Start;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Stop              = virtual_Stop;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Pause             = virtual_Pause;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Resume            = virtual_Resume;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Deinit            = virtual_Deinit;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Reset             = virtual_Reset;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_ComponentRoleEnum;
    MagOmxComponent_DispTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_ProceedBuffer;
}

static void MagOmxComponent_DispTest_constructor(MagOmxComponent_DispTest thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_DispTest));
    chain_constructor(MagOmxComponent_DispTest, thiz, params);
}

static void MagOmxComponent_DispTest_destructor(MagOmxComponent_DispTest thiz, MagOmxComponent_DispTestVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_DispTest_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_DispTest hVDispComp;
	MagOmxComponentImpl      parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_DispTest);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hVDispComp = (MagOmxComponent_DispTest) ooc_new( MagOmxComponent_DispTest, (void *)param);
    MAG_ASSERT(hVDispComp);
    AGILE_LOGV("get hVDispComp=0x%x", hVDispComp);

    parent = ooc_cast(hVDispComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hVDispComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hVDispComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_DispTest_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_DispTest hVdecComp;

	AGILE_LOGV("MagOmxComponent_DispTest_DeInit enter!");
	hVdecComp = (MagOmxComponent_DispTest)compType->pComponentPrivate;
	ooc_delete((Object)hVdecComp);

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {ROLE_NAME, NULL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_DispTest_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_DispTest_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef ROLE_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER