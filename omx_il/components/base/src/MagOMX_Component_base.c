#include "MagOMX_Component_base.h"

AllocateClass(MagOmxComponent, Base);

static MagOmxComponent getBase(OMX_HANDLETYPE hComponent) {

    MagOmxComponent base;
    base = ooc_cast(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, MagOmxComponent);
    return base;
}

static OMX_ERRORTYPE GetComponentVersionWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STRING pComponentName,
                OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                OMX_OUT OMX_UUIDTYPE* pComponentUUID){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->MagOMX_GetComponentVersion ){
        AGILE_LOGE("The component(0x%p - %s): GetComponentVersion() is not implemented", hComponent, pComponentName);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->GetComponentVersion(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                                pComponentName,
                                                                                pComponentVersion,
                                                                                pSpecVersion,
                                                                                pComponentUUID);
    }
}

static OMX_ERRORTYPE SendCommandWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_COMMANDTYPE Cmd,
                OMX_IN  OMX_U32 nParam1,
                OMX_IN  OMX_PTR pCmdData){
    if ( !MagOmxComponentVirtual(getBase(hComponent))->SendCommand ){
        AGILE_LOGE("The component(0x%p): SendCommand() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->SendCommand(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                        Cmd,
                                                                        nParam1,
                                                                        pCmdData);
    }
}

static OMX_ERRORTYPE GetParameterWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nParamIndex,  
                OMX_INOUT OMX_PTR pComponentParameterStructure){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->GetParameter ){
        AGILE_LOGE("The component(0x%p): GetParameter() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->GetParameter(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                         nParamIndex,
                                                                         pComponentParameterStructure);
    }
}

static OMX_ERRORTYPE SetParameterWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent, 
                OMX_IN  OMX_INDEXTYPE nIndex,
                OMX_IN  OMX_PTR pComponentParameterStructure){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->SetParameter ){
        AGILE_LOGE("The component(0x%p): SetParameter() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->SetParameter(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                         nIndex,
                                                                         pComponentParameterStructure);
    }
}

static OMX_ERRORTYPE GetConfigWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_INOUT OMX_PTR pComponentConfigStructure){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->GetConfig ){
        AGILE_LOGE("The component(0x%p): GetConfig() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->GetConfig(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                      nIndex,
                                                                      pComponentConfigStructure);
    }
}

static OMX_ERRORTYPE SetConfigWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_INDEXTYPE nIndex, 
                OMX_IN  OMX_PTR pComponentConfigStructure){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->SetConfig ){
        AGILE_LOGE("The component(0x%p): SetConfig() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->SetConfig(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                      nIndex,
                                                                      pComponentConfigStructure);
    }
}

static OMX_ERRORTYPE GetExtensionIndexWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_STRING cParameterName,
                OMX_OUT OMX_INDEXTYPE* pIndexType){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->GetExtensionIndex ){
        AGILE_LOGE("The component(0x%p): GetExtensionIndex() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->GetExtensionIndex(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                              cParameterName,
                                                                              pIndexType);
    }
}


static OMX_ERRORTYPE GetStateWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_STATETYPE* pState){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->GetState ){
        AGILE_LOGE("The component(0x%p): GetState() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->GetState(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                     pState);
    }
}

static OMX_ERRORTYPE ComponentTunnelRequestWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPort,
                OMX_IN  OMX_HANDLETYPE hTunneledComp,
                OMX_IN  OMX_U32 nTunneledPort,
                OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->ComponentTunnelRequest ){
        AGILE_LOGE("The component(0x%p): ComponentTunnelRequest() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->ComponentTunnelRequest(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                                   nPort,
                                                                                   hTunneledComp,
                                                                                   nTunneledPort,
                                                                                   pTunnelSetup);
    }
}

static OMX_ERRORTYPE UseBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes,
                OMX_IN OMX_U8* pBuffer){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->UseBuffer ){
        AGILE_LOGE("The component(0x%p): UseBuffer() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->UseBuffer(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                      ppBufferHdr,
                                                                      nPortIndex,
                                                                      pAppPrivate,
                                                                      nSizeBytes,
                                                                      pBuffer);
    }
}

static OMX_ERRORTYPE AllocateBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN OMX_U32 nSizeBytes){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->AllocateBuffer ){
        AGILE_LOGE("The component(0x%p): AllocateBuffer() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->AllocateBuffer(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                           ppBuffer,
                                                                           nPortIndex,
                                                                           pAppPrivate,
                                                                           nSizeBytes);
    }
}

static OMX_ERRORTYPE FreeBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_U32 nPortIndex,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->FreeBuffer ){
        AGILE_LOGE("The component(0x%p): FreeBuffer() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->FreeBuffer(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                       nPortIndex,
                                                                       pBuffer);
    }
}

static OMX_ERRORTYPE EmptyThisBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->EmptyThisBuffer ){
        AGILE_LOGE("The component(0x%p): EmptyThisBuffer() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->EmptyThisBuffer(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                            pBuffer);
    }
}

static OMX_ERRORTYPE FillThisBufferWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->FillThisBuffer ){
        AGILE_LOGE("The component(0x%p): FillThisBuffer() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->FillThisBuffer(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                           pBuffer);
    }
}

static OMX_ERRORTYPE SetCallbacksWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
                OMX_IN  OMX_PTR pAppData){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->SetCallbacks ){
        AGILE_LOGE("The component(0x%p): SetCallbacks() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->SetCallbacks(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate, 
                                                                         pCallbacks,
                                                                         pAppData);
    }
}

static OMX_ERRORTYPE ComponentDeInitWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->ComponentDeInit ){
        AGILE_LOGE("The component(0x%p): ComponentDeInit() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->ComponentDeInit(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate);
    }
}

static OMX_ERRORTYPE UseEGLImageWrapper(
                OMX_IN  OMX_HANDLETYPE hComponent,
                OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                OMX_IN OMX_U32 nPortIndex,
                OMX_IN OMX_PTR pAppPrivate,
                OMX_IN void* eglImage){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->UseEGLImage ){
        AGILE_LOGE("The component(0x%p): UseEGLImage() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->UseEGLImage(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate,
                                                                        ppBufferHdr,
                                                                        nPortIndex,
                                                                        pAppPrivate,
                                                                        eglImage);
    }
}

static OMX_ERRORTYPE ComponentRoleEnumWrapper(
                OMX_IN OMX_HANDLETYPE hComponent,
                OMX_OUT OMX_U8 *cRole,
                OMX_IN OMX_U32 nIndex){

    if ( !MagOmxComponentVirtual(getBase(hComponent))->ComponentRoleEnum ){
        AGILE_LOGE("The component(0x%p): ComponentRoleEnum() is not implemented", hComponent);
        return OMX_ErrorNotImplemented;
    }else{
        return MagOmxComponentVirtual(getBase(hComponent))->ComponentRoleEnum(((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate,
                                                                                cRole,
                                                                                nIndex);
    }
}

static OMX_COMPONENTTYPE *MagOmxComponent_getComponentObj(MagOmxComponent self){
    return self->mpComponentObject;
}

/****************************
 *Member function implementation
 ****************************/
static OMX_COMPONENTTYPE *virtual_MagOmxComponent_Create(
                OMX_IN MagOmxComponent pBase, 
                OMX_IN OMX_PTR pAppData){

    OMX_COMPONENTTYPE* comp = NULL;
    if (NULL == pBase){
        AGILE_LOGE("the MagOmxComponent parameter is NULL!");
        return NULL;
    }
    
    comp = pBase->mpComponentObject;

    comp->nSize                    = sizeof(OMX_COMPONENTTYPE);
    comp->nVersion.s.nVersionMajor = kVersionMajor;
    comp->nVersion.s.nVersionMinor = kVersionMinor;
    comp->nVersion.s.nRevision     = kVersionRevision;
    comp->nVersion.s.nStep         = kVersionStep;
    comp->pApplicationPrivate      = pAppData;

    comp->GetComponentVersion      = GetComponentVersionWrapper;
    comp->SendCommand              = SendCommandWrapper;
    comp->GetParameter             = GetParameterWrapper;
    comp->SetParameter             = SetParameterWrapper;
    comp->GetConfig                = GetConfigWrapper;
    comp->SetConfig                = SetConfigWrapper;
    comp->GetExtensionIndex        = GetExtensionIndexWrapper;
    comp->GetState                 = GetStateWrapper;
    comp->ComponentTunnelRequest   = ComponentTunnelRequestWrapper;
    comp->UseBuffer                = UseBufferWrapper;
    comp->AllocateBuffer           = AllocateBufferWrapper;
    comp->FreeBuffer               = FreeBufferWrapper;
    comp->EmptyThisBuffer          = EmptyThisBufferWrapper;
    comp->FillThisBuffer           = FillThisBufferWrapper;
    comp->SetCallbacks             = SetCallbacksWrapper;
    comp->ComponentDeInit          = ComponentDeInitWrapper;
    comp->UseEGLImage              = UseEGLImageWrapper;
    comp->ComponentRoleEnum        = ComponentRoleEnumWrapper;
    
    return comp;
}


/*Class Constructor/Destructor*/
static void MagOmxComponent_initialize(Class this){
    /*Pure virtual functions: must be overrided by sub-components*/
    MagOmxComponentVtableInstance.GetComponentVersion    = NULL;
    MagOmxComponentVtableInstance.SendCommand            = NULL;
    MagOmxComponentVtableInstance.GetParameter           = NULL;
    MagOmxComponentVtableInstance.SetParameter           = NULL;
    MagOmxComponentVtableInstance.GetConfig              = NULL;
    MagOmxComponentVtableInstance.SetConfig              = NULL;
    MagOmxComponentVtableInstance.GetExtensionIndex      = NULL;
    MagOmxComponentVtableInstance.GetState               = NULL;
    MagOmxComponentVtableInstance.ComponentTunnelRequest = NULL;
    MagOmxComponentVtableInstance.UseBuffer              = NULL;
    MagOmxComponentVtableInstance.AllocateBuffer         = NULL;
    MagOmxComponentVtableInstance.FreeBuffer             = NULL;
    MagOmxComponentVtableInstance.EmptyThisBuffer        = NULL;
    MagOmxComponentVtableInstance.FillThisBuffer         = NULL;
    MagOmxComponentVtableInstance.SetCallbacks           = NULL;
    MagOmxComponentVtableInstance.ComponentDeInit        = NULL;
    MagOmxComponentVtableInstance.UseEGLImage            = NULL;
    MagOmxComponentVtableInstance.ComponentRoleEnum      = NULL;

    MagOmxComponentVtableInstance.Create                 = virtual_MagOmxComponent_Create;
}

static void MagOmxComponent_constructor(MagOmxComponent thiz, const void *params){
    MagErr_t mc_ret;
    
    AGILE_LOGV("Enter!");
    
    MAG_ASSERT(ooc_isInitialized(MagOmxComponent));
    chain_constructor(MagOmxComponent, thiz, params);
    
    thiz->getComponentObj   = MagOmxComponent_getComponentObj;

    thiz->mpComponentObject = (OMX_COMPONENTTYPE *)mag_mallocz(sizeof(OMX_COMPONENTTYPE));
    MAG_ASSERT(thiz->mpComponentObject);
}

static void MagOmxComponent_destructor(MagOmxComponent thiz, MagOmxComponentVtable vtab){
    AGILE_LOGV("Enter!");
    mag_freep(&thiz->mpComponentObject);
}


