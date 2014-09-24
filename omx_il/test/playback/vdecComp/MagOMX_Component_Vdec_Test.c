#include "MagOMX_Component_Vdec_Test.h"
#include "MagOMX_Port_Vdec_Test.h"
#include "MagOMX_Port_Disp_Test.h"
#include "MagOMX_IL.h"

#define COMPONENT_NAME "OMX.Mag.vdec.test"
#define ROLE_NAME      "video_decoder.all"
#define START_PORT_INDEX 0
#define PORT_NUMBER      2

AllocateClass(MagOmxComponent_VdecTest, MagOmxComponentVideo);

static OMX_ERRORTYPE localSetupComponent(
                    OMX_IN  OMX_HANDLETYPE hComponent){
	MagOmxPort_VdecTest vdecInPort;
	MagOmxPort_DispTest dispOutPort;
	MagOmxPort_Constructor_Param_t param;
	MagOmxComponentImpl vdecCompImpl;
	MagOmxComponent     vdecComp;

	AGILE_LOGV("enter!");

	param.portIndex    = START_PORT_INDEX + 0;
	param.isInput      = OMX_TRUE;
	param.bufSupplier  = OMX_BufferSupplyUnspecified;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-In", VDEC_PORT_NAME);

	ooc_init_class(MagOmxPort_VdecTest);
	vdecInPort = ooc_new(MagOmxPort_VdecTest, &param);
	MAG_ASSERT(vdecInPort);

	param.portIndex    = START_PORT_INDEX + 1;
	param.isInput      = OMX_FALSE;
	param.bufSupplier  = OMX_BufferSupplyOutput;
	param.formatStruct = 0;
	sprintf((char *)param.name, "%s-Out", DISP_PORT_NAME);

	ooc_init_class(MagOmxPort_DispTest);
	dispOutPort = ooc_new(MagOmxPort_DispTest, &param);
	MAG_ASSERT(dispOutPort);

	vdecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	vdecComp = ooc_cast(hComponent, MagOmxComponent);
	
	vdecComp->setName(vdecComp, (OMX_U8 *)COMPONENT_NAME);
	vdecCompImpl->addPort(vdecCompImpl, START_PORT_INDEX + 0, vdecInPort);
	vdecCompImpl->addPort(vdecCompImpl, START_PORT_INDEX + 1, dispOutPort);

	vdecCompImpl->setupPortDataFlow(vdecCompImpl, vdecInPort, dispOutPort);

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
	OMX_HANDLETYPE vdecInPort;
	OMX_HANDLETYPE dispOutPort;
	MagOmxComponentImpl vdecCompImpl;

	AGILE_LOGV("vdec enter!");
	vdecCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
	vdecInPort  = vdecCompImpl->getPort(vdecCompImpl, START_PORT_INDEX + 0);
	dispOutPort = vdecCompImpl->getPort(vdecCompImpl, START_PORT_INDEX + 1);

	ooc_delete((Object)vdecInPort);
	ooc_delete((Object)dispOutPort);

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
	AGILE_LOGV("src buffer: 0x%x, dest buffer: 0x%x", srcbufHeader, destbufHeader);
	return OMX_ErrorNone;
}

/*Class Constructor/Destructor*/
static void MagOmxComponent_VdecTest_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_GetComponentUUID  = virtual_GetComponentUUID;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Prepare           = virtual_Prepare;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Preroll           = virtual_Preroll;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Start             = virtual_Start;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Stop              = virtual_Stop;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Pause             = virtual_Pause;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Resume            = virtual_Resume;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Deinit            = virtual_Deinit;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_Reset             = virtual_Reset;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ComponentRoleEnum = virtual_ComponentRoleEnum;
    MagOmxComponent_VdecTestVtableInstance.MagOmxComponentVideo.MagOmxComponentImpl.MagOMX_ProceedBuffer     = virtual_ProceedBuffer;
}

static void MagOmxComponent_VdecTest_constructor(MagOmxComponent_VdecTest thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponent_VdecTest));
    chain_constructor(MagOmxComponent_VdecTest, thiz, params);
}

static void MagOmxComponent_VdecTest_destructor(MagOmxComponent_VdecTest thiz, MagOmxComponent_VdecTestVtable vtab){
	AGILE_LOGV("Enter!");
}

static OMX_ERRORTYPE MagOmxComponent_VdecTest_Init(OMX_OUT OMX_HANDLETYPE *hComponent,  
                                   				  OMX_IN  OMX_PTR pAppData,
				                                  OMX_IN  OMX_CALLBACKTYPE* pCallBacks){
	MagOmxComponent_VdecTest hVdecComp;
	MagOmxComponentImpl     parent;
    OMX_U32 param[2];

    AGILE_LOGV("enter!");

    ooc_init_class(MagOmxComponent_VdecTest);

    param[0] = START_PORT_INDEX;
    param[1] = PORT_NUMBER;

    hVdecComp = (MagOmxComponent_VdecTest) ooc_new( MagOmxComponent_VdecTest, (void *)param);
    MAG_ASSERT(hVdecComp);

    parent = ooc_cast(hVdecComp, MagOmxComponentImpl);
    *hComponent = MagOmxComponentImplVirtual(parent)->Create(hVdecComp, pAppData, pCallBacks);
    if (*hComponent){
    	return localSetupComponent(hVdecComp);
    }else{
    	return OMX_ErrorInsufficientResources;
    }
}

static OMX_ERRORTYPE MagOmxComponent_VdecTest_DeInit(OMX_IN OMX_HANDLETYPE hComponent){
	OMX_COMPONENTTYPE *compType = (OMX_COMPONENTTYPE *)hComponent;
	MagOmxComponent_VdecTest hVdecComp;

	AGILE_LOGV("MagOmxComponent_VdecTest_DeInit enter!");
	hVdecComp = (MagOmxComponent_VdecTest)compType->pComponentPrivate;
	ooc_delete((Object)hVdecComp);

	return OMX_ErrorNone;
}

MagOMX_Component_Registration_t *MagOMX_Component_Registration(){
	static char * roles[] = {ROLE_NAME, NULL};
    static MagOMX_Component_Registration_t comp_reg = {
        COMPONENT_NAME, roles, 1, MagOmxComponent_VdecTest_Init
    };

    return &comp_reg;
}

void MagOMX_Component_Deregistration(OMX_HANDLETYPE hComponent){
	MagOmxComponent_VdecTest_DeInit(hComponent);
}

#undef COMPONENT_NAME
#undef ROLE_NAME
#undef START_PORT_INDEX
#undef PORT_NUMBER