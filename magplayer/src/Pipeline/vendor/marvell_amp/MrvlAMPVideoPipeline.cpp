#include "MrvlAMPVideoPipeline.h"
#include "MagPlatform.h"

extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"
}

#define VDEC_INPUT_PORT_NUM    1
#define VDEC_OUTPUT_PORT_NUM   1
#define VOUT_INPUT_PORT_NUM    2
#define VOUT_OUTPUT_PORT_NUM   0

#define AMP_VIDEO_ES_BUFFER_SIZE  256*1024
#define AMP_VIDEO_STREAM_NUM      16

#define BUFFER_ALIGN 32

static char *amp_client_argv[] = {"client", "iiop:1.0//127.0.0.1:999/AMP::FACTORY/factory"};

MrvlAMPVideoPipeline::MrvlAMPVideoPipeline():
                            mhFactory(NULL),
                            mhDisp(NULL),
                            mhVdec(NULL),
                            mhVout(NULL),
                            mVdecEvtListener(NULL),
                            mpAMPVideoBuf(NULL),
                            mStreamPosition(0),
                            mpEOSBuffer(NULL),
                            mDisplayWidth(0),
                            mDisplayHeight(0),
                            mTrackInfo(NULL),
                            mOutputFrameCnt(0){
    AMP_GetFactory(&mhFactory);
    if (mhFactory == NULL) {
        AGILE_LOGD("Initialize amp client");
        MV_OSAL_Init();
        AMP_Initialize(2, amp_client_argv, &mhFactory);
    }

    mpRGBData[0]    = NULL;
    mpUYVYData[0]   = NULL;

#ifdef AMP_VIDEO_STREAM_DUMP
    mDumpVideoFile = NULL;
#endif
}

MrvlAMPVideoPipeline::~MrvlAMPVideoPipeline(){
    reset();
}

void MrvlAMPVideoPipeline::getDispResolution(){
    HRESULT ret;
    ui32 id = 0;

    AMP_RPC(ret, AMP_DISP_OUT_GetResolution, mhDisp, AMP_DISP_PLANE_MAIN, &id);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to do DISP_OUT_GetResolution(), ret = 0x%x", ret);
    }

    switch (id){
        case AMP_DISP_OUT_RES_525I60:
        case AMP_DISP_OUT_RES_525I5994:
        case AMP_DISP_OUT_RES_525P60:
        case AMP_DISP_OUT_RES_525P5994:
        {
            mDisplayWidth  = 720;
            mDisplayHeight = 480;
            break;
        }
        case AMP_DISP_OUT_RES_625P50:
        case AMP_DISP_OUT_RES_625I50:
        {
            mDisplayWidth  = 720;
            mDisplayHeight = 576;
            break;
        }
        case AMP_DISP_OUT_RES_720P30:
        case AMP_DISP_OUT_RES_720P2997:
        case AMP_DISP_OUT_RES_720P25:
        case AMP_DISP_OUT_RES_720P60:
        case AMP_DISP_OUT_RES_720P5994:
        case AMP_DISP_OUT_RES_720P50:
        {
            mDisplayWidth  = 1280;
            mDisplayHeight = 720;
            break;
        }
        default:
        {
            mDisplayWidth  = 1920;
            mDisplayHeight = 1080;
            break;
        }
    }
    AGILE_LOGD("To do DISP_OUT_GetResolution(w:%d, h:%d), id = %d", mDisplayWidth, mDisplayHeight, id);
}


ui32 MrvlAMPVideoPipeline::getAMPVideoFormat(ui32 OMXCodec){
    switch(OMXCodec){
        case OMX_VIDEO_CodingAVC:
            AGILE_LOGD("[AMPObject::getAMPVideoFormat]: H264");
            return MEDIA_VES_AVC;

        // case OMX_VIDEO_CodingMPEG4:
        //     AGILE_LOGD("[AMPObject::getAMPVideoFormat]: MPEG4");
        //     return MEDIA_VES_MPEG4;
            
        // case OMX_VIDEO_CodingMPEG2:
        //     AGILE_LOGD("[AMPObject::getAMPVideoFormat]: MPEG2");
        //     return MEDIA_VES_MPEG2;
            
        default:
            AGILE_LOGE("[AMPObject::getAMPVideoFormat]: unsupported video format: %d", OMXCodec);
            break;
    }
    return MEDIA_INVALIDATE;
}

bool MrvlAMPVideoPipeline::needData(){
    if (mpAMPVideoBuf){
        if (mpAMPVideoBuf->needPushBuffers()){
            return true;
        }else{
            return false;
        }
    }else{
        AGILE_LOGV("mpAMPVideoBuf is NULL!");
        return true;
    }
}

bool MrvlAMPVideoPipeline::isPlaying(){
    if (mState == ST_PLAY)
        return true;
    return false;
}

HRESULT MrvlAMPVideoPipeline::videoPushBufferDone(CORBA_Object hCompObj, AMP_PORT_IO ePortIo,
                                  UINT32 uiPortIdx, struct AMP_BD_ST *hBD,
                                  AMP_IN void *pUserData) {
    MrvlAMPVideoPipeline *driver = static_cast<MrvlAMPVideoPipeline *>(pUserData);
    AMPBuffer *ampbuf;

    ampbuf = driver->getEOSBuffer();
    driver->mpAMPVideoBuf->putAMPBuffer(hBD);

    if (ampbuf != NULL){
        if (ampbuf->getBD() == hBD){
            AGILE_LOGI("get the EOS frame back and notify APP the playback completion");
            driver->notifyPlaybackComplete();
        }
    }else{
        if (driver->getFillBufferFlag()){
            if (driver->isPlaying() && !driver->mIsFlushed){
                driver->postFillThisBuffer();
                AGILE_LOGD("retrigger the fillThisBuffer event!");
            }
            driver->setFillBufferFlag(false);
            
        }
    }
    return SUCCESS;
}

HRESULT MrvlAMPVideoPipeline::VdecEventHandle(HANDLE hListener, AMP_EVENT *pEvent, VOID *pUserData) {
    if (!pEvent) {
        AGILE_LOGE("pEvent is NULL!");
        return !SUCCESS;
    }

    if (AMP_EVENT_API_VDEC_CALLBACK == pEvent->stEventHead.eEventCode) {
        UINT32 *payload = static_cast<UINT32 *>(AMP_EVENT_PAYLOAD_PTR(pEvent));
        MrvlAMPVideoPipeline *pComp = static_cast<MrvlAMPVideoPipeline *>(pUserData);
        switch (pEvent->stEventHead.uiParam1) {
            case AMP_VDEC_EVENT_RES_CHANGE:
                pComp->mVideoInfoPQ.width = payload[0];
                pComp->mVideoInfoPQ.height = payload[1];
                AGILE_LOGD("[VdecEventHandle] resolution: w=%d, h=%d", payload[0], payload[1]);
                break;
            
            case AMP_VDEC_EVENT_FR_CHANGE:
                pComp->mVideoInfoPQ.framerate = payload[0];
                pComp->mVideoInfoPQ.framerate_den = payload[1];
                AGILE_LOGD("[VdecEventHandle] frame rate: FrameRateNum=%d, FrameRateDen=%d", payload[0], payload[1]);
                break;
            
            case AMP_VDEC_EVENT_AR_CHANGE:
                pComp->mVideoInfoPQ.ar_w = payload[0];
                pComp->mVideoInfoPQ.ar_h = payload[1];
                AGILE_LOGD("[VdecEventHandle] AR: AR_W=%d, AR_H=%d", payload[0], payload[1]);
                break;
            
            case AMP_VDEC_EVENT_1ST_I_DECODED:
                pComp->mVideoInfoPQ.width = payload[0];
                // pComp->mVideoWidth  = (i32)pComp->mVideoInfoPQ.width;
                pComp->mVideoInfoPQ.height = payload[1];
                // pComp->mVideoHeight = (i32)pComp->mVideoInfoPQ.height;
                
                pComp->mVideoInfoPQ.IsIntlSeq = static_cast<bool>(payload[2]);
                pComp->mVideoInfoPQ.framerate = payload[3];
                pComp->mVideoInfoPQ.framerate_den = payload[4];
                pComp->mVideoInfoPQ.ar_w = payload[5];
                pComp->mVideoInfoPQ.ar_h = payload[6];
                AGILE_LOGD("[VdecEventHandle] Res: w=%d, h=%d, is_interlace=%d, FR: Num=%d, Den=%d, AR: AR_W=%d, AR_H=%d", 
                           payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6]);
                // pComp->setVideoGeometry();
                pComp->calcPictureRGB();
                break;
            
            // case AMP_VDEC_EVENT_ONE_FRAME_DECODED:
            //     ui32 frameType;
            //     struct timezone zone;
                
            //     frameType = payload[0];
            //     gettimeofday(&pComp->mLastVideoFrameTime, &zone);
            //     pComp->mbJudgeVideoFrameTime = true;
            //     //AGILE_LOGD("[VdecEventHandle]: get video frame[%s]", 
            //     //          frameType == AMP_VDEC_FRAME_TYPE_I ? "I" : (frameType == AMP_VDEC_FRAME_TYPE_P ? "P" : "B"));
            //     break;
            
            default:
                return SUCCESS;
        }
    }
    return SUCCESS;
}

void MrvlAMPVideoPipeline::SwapDisplayPlane(){
    HRESULT result = SUCCESS;
    AMP_DISP_ZORDER Zorder;
    ui32 temp;
    char *enable = getenv("lmptest");
    
    if (enable != NULL && !strcmp(enable, "yes")){
        AMP_RPC(result, AMP_DISP_GetPlaneZOrder, mhDisp, 0, &Zorder);
        temp = Zorder.iGfx0;
        Zorder.iGfx0 = Zorder.iMain;
        Zorder.iMain = temp;
        AMP_RPC(result, AMP_DISP_SetPlaneZOrder, mhDisp, 0, &Zorder);
    }
}

void MrvlAMPVideoPipeline::setDisplayRect(i32 x, i32 y, i32 w, i32 h){
    HRESULT result;

    AMP_DISP_WIN src;
    AMP_DISP_WIN dst;
    
    src.iX      = 0;
    src.iY      = 0;
    src.iWidth  = 0;
    src.iHeight = 0;

    dst.iX      = x ;
    dst.iY      = y ;
    dst.iWidth  = w;
    dst.iHeight = h;
    
    if (mhDisp == NULL){
        AGILE_LOGE("mhDisp is NULL!");
        return;
    }

    AMP_RPC(result, AMP_DISP_SetScale, mhDisp, AMP_DISP_PLANE_MAIN, &src, &dst);
    if(result != SUCCESS){
        AGILE_LOGE("failed to do AMP_DISP_SetScale() [SRC][x:%d, y:%d, w:%d, h:%d] [DST][x:%d, y:%d, w:%d, h:%d]. ret = %d", 
                    x, y, w, h, dst.iX, dst.iY, dst.iWidth, dst.iHeight, result);
    }else{
        AGILE_LOGD("AMP_DISP_SetScale() [SRC][x:%d, y:%d, w:%d, h:%d] [DST][x:%d, y:%d, w:%d, h:%d]. - OK!", 
                    x, y, w, h, dst.iX, dst.iY, dst.iWidth, dst.iHeight);
    }
}

_status_t MrvlAMPVideoPipeline::setup(i32 trackID, TrackInfo_t *sInfo){
    AMP_COMPONENT_CONFIG amp_config;
    HRESULT ret;
    ui32 cnt;
    AMP_PORT_INFO PortInfo;
    ui32 ampVideoFormat;

    i32 VdecPortIn = -1;
    i32 VdecPort2Vout = -1;
    i32 VoutPort2Vdec = -1;
    i32 ClkPort2VOut = -1;

    AGILE_LOGD("Enter!");
    MagVideoPipelineImpl::setup(trackID, sInfo);

    mTrackInfo = sInfo;
    if ((NULL == mhVdec) && (NULL == mhVout)){
        ampVideoFormat = getAMPVideoFormat(sInfo->codec);
        if (MEDIA_INVALIDATE == ampVideoFormat){
            return MAG_NO_INIT;
        }

        AMP_RPC(ret, AMP_FACTORY_CreateDisplayService, mhFactory, &mhDisp);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to start display service. ret = %d", ret);
            mhDisp = NULL;
            return ret;
        }else{
            AGILE_LOGD("AMP create component Display Service -- OK!");
        }
        getDispResolution();

        AMP_RPC(ret, AMP_FACTORY_CreateComponent, mhFactory, AMP_COMPONENT_VDEC, 1, &mhVdec);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to create component: vdec. ret = %d", ret);
            mhVdec = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("AMP create component VDEC -- OK!");
        }

        AMP_RPC(ret, AMP_FACTORY_CreateComponent, mhFactory, AMP_COMPONENT_VOUT, 1, &mhVout);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to create component: vout. ret = %d", ret);
            mhVout = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD("AMP create component VOUT -- OK!");
        }

        AmpMemClear(&amp_config, sizeof(AMP_COMPONENT_CONFIG));
        amp_config._d = AMP_COMPONENT_VDEC;
        amp_config._u.pVDEC.mode = AMP_SECURE_TUNNEL; /*AMP_SECURE_TUNNEL, AMP_TUNNEL*/
        amp_config._u.pVDEC.uiType = ampVideoFormat;
        amp_config._u.pVDEC.uiFlag |= 1 << 9; /*Must be set for bg3cd-A0 and optional for bg3cd-z1/2*/

        AMP_RPC(ret, AMP_VDEC_Open, mhVdec, &amp_config);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_VDEC_Open(). ret = %d", ret);
            AMP_RPC(ret, AMP_VDEC_Destroy, mhVdec);
            mhVdec = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD(" AMP_VDEC_Open() - OK!");
        }

        if (NULL != mhVdec){
            // Register a callback so that it can receive event - for example resolution changed -from VDEC.
            mVdecEvtListener = AMP_Event_CreateListener(AMP_EVENT_TYPE_MAX, 0);
            if (mVdecEvtListener) {
                HRESULT err = SUCCESS;
                err = AMP_Event_RegisterCallback(mVdecEvtListener, AMP_EVENT_API_VDEC_CALLBACK, 
                                                 VdecEventHandle, static_cast<void *>(this));
                if (SUCCESS != err){
                    AGILE_LOGE("failed to register VDEC notify. err=0x%x", err);
                }else{
                    AGILE_LOGD("register VDEC notify -- OK!");

                    AMP_RPC(err, AMP_VDEC_RegisterNotify, mhVdec,
                            AMP_Event_GetServiceID(mVdecEvtListener), AMP_EVENT_API_VDEC_CALLBACK);

                    if (SUCCESS != err){
                        AGILE_LOGE("failed to register VDEC callback. err=0x%x", err);
                    }else{
                        AGILE_LOGD("register VDEC callback -- OK!");
                    }
                }
            }
        }

        for (cnt = 0; cnt < VDEC_INPUT_PORT_NUM; cnt++) {
            AMP_RPC(ret, AMP_VDEC_QueryPort, mhVdec, AMP_PORT_INPUT,
                    cnt, &PortInfo);
            if (PortInfo.ePortType == AMP_PORT_VDEC_IN_ES_MEM) {
               VdecPortIn = cnt;
               break;
            }
        }

        for (cnt = 0; cnt < VDEC_OUTPUT_PORT_NUM; cnt++) {
            AMP_RPC(ret, AMP_VDEC_QueryPort, mhVdec, AMP_PORT_OUTPUT,
                    cnt, &PortInfo);
            if (PortInfo.ePortType == AMP_PORT_VDEC_OUT_FRAMES) {
               VdecPort2Vout = cnt;
               break;
            }
        }

        if (VdecPortIn != -1){
            ret = AMP_ConnectApp(mhVdec, AMP_PORT_INPUT, VdecPortIn, videoPushBufferDone, static_cast<void *>(this));
            if(ret != SUCCESS){
                AGILE_LOGE("failed to do AMP_ConnectApp() for vdec input. ret = %d", ret);
                return MAG_NO_INIT;
            }
        }else{
            AGILE_LOGE("[AMPObject::InitVideoSetting]: failed to query port: VdecPortIn = %d", VdecPortIn);
            return MAG_NO_INIT;
        }

        AmpMemClear(&amp_config, sizeof(AMP_COMPONENT_CONFIG));
        amp_config._d                       = AMP_COMPONENT_VOUT;
        amp_config._u.pVOUT.mode            = AMP_TUNNEL;
        amp_config._u.pVOUT.uiInputPortNum  = VOUT_INPUT_PORT_NUM;
        amp_config._u.pVOUT.uiOutputPortNum = VOUT_OUTPUT_PORT_NUM;
        AMP_RPC(ret, AMP_VOUT_Open, mhVout, &amp_config);
        if(ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_VOUT_Open(). ret = %d", ret);
            AMP_RPC(ret, AMP_VOUT_Destroy, mhVout);
            mhVout = NULL;
            return MAG_NO_INIT;
        }else{
            AGILE_LOGD(" AMP_VOUT_Open() - OK!");
        }

        for (cnt = 0; cnt < VOUT_INPUT_PORT_NUM; cnt++) {
            AMP_RPC(ret, AMP_VOUT_QueryPort, mhVout, AMP_PORT_INPUT,
                    cnt, &PortInfo);
            if (PortInfo.ePortType == AMP_PORT_VOUT_IN_TO_MAIN) {
               VoutPort2Vdec = cnt;
               break;
            }
        }

        if ((VdecPort2Vout != -1) && (VoutPort2Vdec != -1)){
            ret = AMP_ConnectComp(mhVdec, VdecPort2Vout, mhVout, VoutPort2Vdec);
            if(ret != SUCCESS){
                AGILE_LOGE("failed to do AMP_ConnectComp() between vdec[%d] and vout[%d]. ret = %d", 
                          VdecPort2Vout, VoutPort2Vdec, ret);
                return MAG_NO_INIT;
            }else{
                AGILE_LOGD("AMP Connect component: Vdec[%d] and Vout[%d] - OK!", VdecPort2Vout, VoutPort2Vdec);
            }
        }else{
            AGILE_LOGE("[AMPObject::InitVideoSetting]: failed to query port: VdecPort2Vout = %d, VoutPort2Vdec = %d", 
                       VdecPort2Vout, VoutPort2Vdec);
            return MAG_NO_INIT;
        }

        for (cnt = 0; cnt < VOUT_INPUT_PORT_NUM; cnt++) {
            AMP_RPC(ret, AMP_VOUT_QueryPort, mhVout, AMP_PORT_INPUT,
                    cnt, &PortInfo);
            if (PortInfo.ePortType == AMP_PORT_VOUT_IN_CLOCK) {
               mVoutPort2Clk = cnt;
               break;
            }
        }

        if (NULL == mpAMPVideoBuf){
            mpAMPVideoBuf = new AMPBufferMgr(&mhVdec, AMP_COMPONENT_VDEC_PIPELINE, AMP_VIDEO_ES_BUFFER_SIZE, AMP_VIDEO_STREAM_NUM);
            
            if (mpAMPVideoBuf->Create() == SUCCESS){
                AGILE_LOGD("create the Video AMP buffers -- OK!");
            }else{
                return MAG_NO_MEMORY;
            }
        }
    }
    AMP_RPC(ret, AMP_VDEC_SetState, mhVdec, AMP_IDLE);
    AMP_RPC(ret, AMP_VOUT_SetState, mhVout, AMP_IDLE);
    
    setDisplayRect(0, 0, mDisplayWidth, mDisplayHeight);
    return MAG_NO_ERROR;
}

_status_t MrvlAMPVideoPipeline::start(){
    _status_t ret;
    HRESULT res = SUCCESS;

    if ((NULL == mhVdec) && (NULL == mhVout)){
        AGILE_LOGE("Video Pipeline doesn't setup, exit!");
        return ret;
    }

    SwapDisplayPlane();
    mGetFirstIFrame = false;

#ifdef AMP_VIDEO_STREAM_DUMP
    mDumpVideoFile = fopen("/data/lmp/ampvideo.es","wb+");
    if (NULL == mDumpVideoFile)
        AGILE_LOGE("To create the file: /data/lmp/ampvideo.es -- Fail");
    else
        AGILE_LOGD("To create the file: /data/lmp/ampvideo.es -- OK");
#endif

    mpAMPVideoBuf->waitForAllBufFree(1);
    AMP_RPC(ret, AMP_VOUT_SetLastFrameMode, mhVout, AMP_VOUT_SHOWBLACKSCREEN);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to do AMP_VOUT_SetLastFrameMode(): AMP_VOUT_SHOWBLACKSCREEN. ret = %d", ret);
        return -1;
    }else{
        AGILE_LOGD("AMP_VOUT_SetLastFrameMode(): AMP_VOUT_SHOWBLACKSCREEN - OK!");
    }

    AMP_RPC(res, AMP_VOUT_SetState, mhVout, AMP_EXECUTING);
    if (res != SUCCESS){
        AGILE_LOGE("failed to AMP_VOUT_SetState: AMP_EXECUTING. ret = %d", res);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_VOUT_SetState: AMP_EXECUTING - OK!");
    }

    AMP_RPC(res, AMP_VDEC_SetState, mhVdec, AMP_EXECUTING);
    if (res != SUCCESS){
        AGILE_LOGE("failed to AMP_VDEC_SetState: AMP_EXECUTING. ret = %d", res);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_VDEC_SetState: AMP_EXECUTING - OK!");
    }

    ret = MagVideoPipelineImpl::start();
    return ret;
}

_status_t MrvlAMPVideoPipeline::stop(){
    HRESULT ret = SUCCESS;

    if ((NULL == mhVdec) && (NULL == mhVout)){
        AGILE_LOGE("Video Pipeline doesn't setup, exit!");
        return ret;
    }

#ifdef AMP_VIDEO_STREAM_DUMP
    if (mDumpVideoFile){
        fclose(mDumpVideoFile);
        mDumpVideoFile = NULL;
    }
#endif
    SwapDisplayPlane();

    mStreamPosition = 0;
    mpEOSBuffer     = NULL;
    MagVideoPipelineImpl::stop();

    if (NULL != mhVdec){
        AMP_RPC(ret, AMP_VDEC_SetState, mhVdec, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_VDEC_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_VDEC_SetState: AMP_IDLE - OK!");
        }
    }

    if (NULL != mhVout){
        AMP_RPC(ret, AMP_VOUT_SetState, mhVout, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_VOUT_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_VOUT_SetState: AMP_IDLE - OK!");
        }
    }
    return MAG_NO_ERROR;
}

_status_t MrvlAMPVideoPipeline::pause(){
    if ((NULL == mhVdec) && (NULL == mhVout)){
        AGILE_LOGE("Video Pipeline doesn't setup, exit!");
        return MAG_NO_ERROR;
    }

    MagVideoPipelineImpl::pause();
    return MAG_NO_ERROR;
}

_status_t MrvlAMPVideoPipeline::resume(){
    if ((NULL == mhVdec) && (NULL == mhVout)){
        AGILE_LOGE("Video Pipeline doesn't setup, exit!");
        return MAG_NO_ERROR;
    }

    MagVideoPipelineImpl::resume();
    return MAG_NO_ERROR;
}

_status_t MrvlAMPVideoPipeline::flush(){
    HRESULT ret = SUCCESS;

    AGILE_LOGV("enter!");

    if ((NULL == mhVdec) && (NULL == mhVout)){
        AGILE_LOGE("Video Pipeline doesn't setup, exit!");
        return ret;
    }

    MagVideoPipelineImpl::flush();

    if ((mState == ST_INIT) || (mState == ST_STOP)){
        AGILE_LOGV("do nothing while the state is in INIT/STOP");
        return MAG_NO_ERROR;
    }

    if (NULL != mhVdec){
        AMP_RPC(ret, AMP_VDEC_SetState, mhVdec, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_VDEC_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_VDEC_SetState: AMP_IDLE - OK!");
        }
    }

    if (NULL != mhVout){
        AMP_RPC(ret, AMP_VOUT_SetState, mhVout, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_VOUT_SetState: AMP_IDLE. ret = %d", ret);
            return MAG_INVALID_OPERATION;
        }else{
            AGILE_LOGD("AMP_VOUT_SetState: AMP_IDLE - OK!");
        }
    }

    mpAMPVideoBuf->waitForAllBufFree(1);

    AMP_RPC(ret, AMP_VOUT_SetState, mhVout, AMP_EXECUTING);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to AMP_VOUT_SetState: AMP_EXECUTING. ret = %d", ret);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_VOUT_SetState: AMP_EXECUTING - OK!");
    }

    AMP_RPC(ret, AMP_VDEC_SetState, mhVdec, AMP_EXECUTING);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to AMP_VDEC_SetState: AMP_EXECUTING. ret = %d", ret);
        return MAG_INVALID_OPERATION;
    }else{
        AGILE_LOGD("AMP_VDEC_SetState: AMP_EXECUTING - OK!");
    }

    mStreamPosition = 0;

    AGILE_LOGV("exit!");

    return MAG_NO_ERROR;
}

_status_t MrvlAMPVideoPipeline::reset(){
    HRESULT ret;

    AGILE_LOGV("enter!");

    if ((NULL == mhVdec) && (NULL == mhVout)){
        AGILE_LOGE("Video Pipeline doesn't setup, exit!");
        return ret;
    }
    
    if ((mState == ST_PLAY) || (mState == ST_PAUSE)){
        stop();
        mpAMPVideoBuf->waitForAllBufFree(1);
    }

    MagVideoPipelineImpl::reset();

    if (NULL != mpAMPVideoBuf){
        mpAMPVideoBuf->unregisterBDs();
    }
    
    if (NULL != mVdecEvtListener){
        AMP_RPC(ret, AMP_VDEC_UnregisterNotify, mhVdec, 
                AMP_Event_GetServiceID(mVdecEvtListener), AMP_EVENT_API_VDEC_CALLBACK);
        if (SUCCESS == ret)
            AMP_Event_UnregisterCallback(mVdecEvtListener, AMP_EVENT_API_VDEC_CALLBACK, VdecEventHandle);

        AMP_Event_DestroyListener(mVdecEvtListener);
        mVdecEvtListener = NULL;
    }

    if ((NULL != mhVdec) && (NULL != mhVout)){
        ret = AMP_DisconnectComp(mhVdec, 0, mhVout, 0);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_DisconnectComp (Vdec to Vout). ret = %d", ret);
        }
    }

    if (NULL != mhVdec){
        ret = AMP_DisconnectApp(mhVdec, AMP_PORT_INPUT, 0, videoPushBufferDone);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_DisconnectApp (video input). ret = %d", ret);
        }
    }

    if (NULL != mhVout){
        AMP_RPC(ret, AMP_VOUT_Close, mhVout);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_VOUT_Close. ret = %d", ret);
        }
    }

    if (NULL != mhVdec){
        AMP_RPC(ret, AMP_VDEC_Close, mhVdec);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_VDEC_Close. ret = %d", ret);
        }
    }

    if (NULL != mhVout){
        AMP_RPC(ret, AMP_VOUT_Destroy, mhVout);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_VOUT_Destroy. ret = %d", ret);
        }

        AMP_FACTORY_Release(mhVout);
        mhVout = NULL;
    }

    if (NULL != mhVdec){
        AMP_RPC(ret, AMP_VDEC_Destroy, mhVdec);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_VDEC_Destroy. ret = %d", ret);
        }
        AMP_FACTORY_Release(mhVdec);
        mhVdec = NULL;
    } 

    if (NULL != mpAMPVideoBuf){
        delete mpAMPVideoBuf;
        mpAMPVideoBuf = NULL;
    }
    
    AGILE_LOGV("111");
    if (mpRGBData[0]){
        AGILE_LOGV("222");
        mag_free(mpRGBData[0]);
        mpRGBData[0] = NULL;
    }
    AGILE_LOGV("333");
    if (mpUYVYData[0]){
        AGILE_LOGV("444");
        mag_free(mpUYVYData[0]);
        mpUYVYData[0] = NULL;
    }
    mOutputFrameCnt = 0;

    AGILE_LOGV("exit!");

    return MAG_NO_ERROR;
}

void *MrvlAMPVideoPipeline::getClkConnectedComp(i32 *port){
    *port = mVoutPort2Clk;
    AGILE_LOGV("vout = 0x%p", mhVout);
    return static_cast<void *>(mhVout);
}

ui32 MrvlAMPVideoPipeline::getPaddingSize(ui32 data_size){
    if (data_size < (64 * 1024)) {
        return ((64 * 1024) - data_size);
    } else if (data_size & (BUFFER_ALIGN - 1)) {
        return (BUFFER_ALIGN - (data_size & (BUFFER_ALIGN - 1)));
    } else {
        return (0);
    }
}

_status_t MrvlAMPVideoPipeline::pushEsPackets(MediaBuffer_t *buf){
    ui8  *pEsPacket   = static_cast<ui8 *>(buf->buffer);
    ui32 sizeEsPacket = buf->buffer_size;
    i64  iPts90k      = buf->pts;
    AMPBufInter_t *pGetBufDesc = NULL;
    AMPBuffer *ampCodecBuf;
    ui8 *vBuf;
    ui32 offset;
    ui32 free_size;
    ui32 padding_size = 0;
    bool formatted = false;
    bool pts_streamPos_pushed = false;
    ui32 header_size = 0;

    if ((mState != ST_PLAY) && (mState != ST_PAUSE)){
        AGILE_LOGD("in non-playing state, exit!");
        return MAG_NO_ERROR;
    }

    AGILE_LOGD("packet size: %d, pts = 0x%llx, streamPOS = %d", sizeEsPacket, iPts90k, mStreamPosition);

    if (!mGetFirstIFrame){
        if (buf->flag != STREAM_FRAME_FLAG_KEY_FRAME){
            AGILE_LOGD("drop the P/B video frames before getting the first I frame!");
            return MAG_NO_ERROR;
        }else{
            mGetFirstIFrame = true;
        }
    }

    while ((sizeEsPacket > 0) || (padding_size > 0)){
        header_size = 0;

        if ((pGetBufDesc = mpAMPVideoBuf->getAMPBuffer()) == NULL){
            AGILE_LOGE("no Video AMP buffer available! [should not be here]");
            return MAG_UNKNOWN_ERROR;
        } 

        ampCodecBuf = reinterpret_cast<AMPBuffer *>(pGetBufDesc->pAMPBuf);
        vBuf = (ui8 *)ampCodecBuf->getBufferStatus(&offset, &free_size);
        
        AGILE_LOGD("get vBuf=0x%p, offset=%d, free_size=%d", vBuf, offset, free_size);

        if (!formatted){
            ui8 header[1024];
            MagESFormat *formatter = static_cast<MagESFormat *>(buf->esFormatter);

            {
                ui8 *src = pEsPacket;
                
                if (formatter){
                    if (pEsPacket[0] != 0x00 || pEsPacket[1] != 0x00 ||
                        pEsPacket[2] != 0x00 || pEsPacket[3] != 0x01){
                        header_size = formatter->formatES(header, pEsPacket, sizeEsPacket);
                    }else{
                        header_size = formatter->addESHeader(header);
                    }
                    if (header_size <= free_size){
                        if (header_size > 0){
                            memcpy(vBuf + offset, header, header_size);
                        #ifdef AMP_VIDEO_STREAM_DUMP
                            if (mDumpVideoFile)
                                fwrite(vBuf, 1, header_size, mDumpVideoFile);
                        #endif
                            offset    += header_size;
                            free_size -= header_size;
                        }
                        // WriteU32At(pEsPacket, 1);
                    }else{
                        AGILE_LOGE("failed to format the video es data!");
                    }
                }
            }
            formatted = true;
        }

        /*should not adding paddings for bg3-cd amp*/
        // padding_size = getPaddingSize(sizeEsPacket + header_size);
        padding_size = 0;
        if ((sizeEsPacket + padding_size) > free_size){
            if (sizeEsPacket > 0){
                if (sizeEsPacket <= free_size){
                    ampCodecBuf->updateUnitStartPtsTag(iPts90k, mStreamPosition);   
                    ampCodecBuf->updateDataSize(sizeEsPacket + header_size);
                    memcpy(vBuf + offset, pEsPacket, sizeEsPacket);
                #ifdef AMP_VIDEO_STREAM_DUMP
                    if (mDumpVideoFile)
                        fwrite(vBuf + offset, 1, sizeEsPacket, mDumpVideoFile);
                #endif
                    if (padding_size > 0)
                        ampCodecBuf->updateDataPaddingSize(free_size - sizeEsPacket);

                    if (padding_size > 0)
                        padding_size = padding_size - (free_size - sizeEsPacket);
                    
                    if (ampCodecBuf->updateMemInfo(&mhVdec, 0) != 1){
                        AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[1] - NOT pushing BD!");
                        // return MAG_NO_MEMORY;
                    }

                    mOutputFrameCnt++;
                    mStreamPosition += (sizeEsPacket + header_size);
                    mStreamPosition = mStreamPosition & 0x1fffffff; 
                    sizeEsPacket = 0;
                }else{
                    ampCodecBuf->updateUnitStartPtsTag(iPts90k, mStreamPosition);
                    ampCodecBuf->updateDataSize(free_size + header_size);
                    memcpy(vBuf + offset, pEsPacket, free_size);
                #ifdef AMP_VIDEO_STREAM_DUMP
                    if (mDumpVideoFile)
                        fwrite(vBuf + offset, 1, free_size, mDumpVideoFile);
                #endif
                    if (ampCodecBuf->updateMemInfo(&mhVdec, 0) != 1){
                        AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[2] - NOT pushing BD!");
                        // return MAG_NO_MEMORY;
                    }
                    
                    mOutputFrameCnt++;
                    pEsPacket += free_size;
                    sizeEsPacket -= free_size;  
                    
                    mStreamPosition += (free_size + header_size);
                    mStreamPosition = mStreamPosition & 0x1fffffff;
                }
                
            }else if (padding_size > 0){
                if (padding_size <= free_size){
                    ampCodecBuf->updateDataPaddingSize(padding_size);
                    
                    if (ampCodecBuf->updateMemInfo(&mhVdec, 0) != 1){
                        AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[3] - NOT pushing BD!");
                    }

                    mOutputFrameCnt++;
                    mStreamPosition += padding_size;
                    mStreamPosition = mStreamPosition & 0x1fffffff; 
                    padding_size = 0;
                }else{
                    ampCodecBuf->updateDataPaddingSize(free_size);
                    
                    if (ampCodecBuf->updateMemInfo(&mhVdec, 0) != 1){
                        AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[4] - NOT pushing BD!");
                        // return MAG_NO_MEMORY;
                    }

                    mOutputFrameCnt++;
                    padding_size -= free_size;

                    mStreamPosition += free_size;
                    mStreamPosition = mStreamPosition & 0x1fffffff; 
                }
            }
        }else{
            ampCodecBuf->updateUnitStartPtsTag(iPts90k, mStreamPosition);
            if (sizeEsPacket > 0){
                ampCodecBuf->updateDataSize(sizeEsPacket + header_size);
                memcpy(vBuf + offset, pEsPacket, sizeEsPacket);
            #ifdef AMP_VIDEO_STREAM_DUMP
                if (mDumpVideoFile)
                    fwrite(vBuf + offset, 1, sizeEsPacket, mDumpVideoFile);
            #endif
            }

            if (padding_size > 0)
                ampCodecBuf->updateDataPaddingSize(padding_size);
            
            if (ampCodecBuf->updateMemInfo(&mhVdec, 0) != 1){
                AGILE_LOGE("[AMPObject::onPlayingEvent]: should not be here[5] - NOT pushing BD!");
            }

            mOutputFrameCnt++;
            mStreamPosition += (sizeEsPacket + header_size + padding_size);
            mStreamPosition = mStreamPosition & 0x1fffffff;
            sizeEsPacket = 0;
            padding_size = 0; 
        }
    }  

    if (buf->eosFrame){
        AGILE_LOGV("proceed EOS frame!");
        if ((pGetBufDesc = mpAMPVideoBuf->getAMPBuffer()) == NULL){
            AGILE_LOGE("no Video AMP buffer available! [should not be here]");
            return MAG_UNKNOWN_ERROR;
        }          
        ampCodecBuf = reinterpret_cast<AMPBuffer *>(pGetBufDesc->pAMPBuf);
        padding_size = getPaddingSize(0);
        ampCodecBuf->updateDataPaddingSize(padding_size);
        ampCodecBuf->updateMemInfo(&mhVdec, 0, true);
        mpEOSBuffer = ampCodecBuf;
        AGILE_LOGV("EOS bd = 0x%p", ampCodecBuf->getBD());
    }

    if (mOutputFrameCnt > 50){
        calcPictureRGB();
        mOutputFrameCnt = 0;
    }

    AGILE_LOGD("exit!");
    return MAG_NO_ERROR;
}

AMPBuffer *MrvlAMPVideoPipeline::getEOSBuffer(){
    return mpEOSBuffer;
}

#define AMP_PIC_CAPTURE_ENABLE "/tmp/pic_capture_enable"
#define AMP_PIC_CAPTURE_CMD    "/tmp/pic_capture_cmd"
#define AMP_PIC_CAPTURE_DUMP   "/tmp/pic_capture_dump.yuv"
#define PIXEL_SAMPLE_INTERVAL  10

void MrvlAMPVideoPipeline::calcPictureRGB(){
    FILE *fEnable = NULL;
    FILE *fCmd = NULL;
    FILE *fYUV = NULL;
    /*static FILE *RGBFrameFile = NULL;*/

    int command = 8;
    int sws_flags = SWS_BICUBIC;
    struct SwsContext *swsConvertCtx;
    int rgb_linesize[1];
    int uyvy_linesize[1];

    int r_top = 0;
    int g_top = 0;
    int b_top = 0;
    int rgb_top_num = 0;
    int r_bottom = 0;
    int g_bottom = 0;
    int b_bottom = 0;
    int rgb_bottom_num = 0;

    int y_top = 0;
    int u_top = 0;
    int v_top = 0;
    int yuv_top_num = 0;
    int r_yuv_top = 0;
    int g_yuv_top = 0;
    int b_yuv_top = 0;

    int y_bottom = 0;
    int u_bottom = 0;
    int v_bottom = 0;
    int yuv_bottom_num = 0;
    int r_yuv_bottom = 0;
    int g_yuv_bottom = 0;
    int b_yuv_bottom = 0;

    int i;
    i32 width;
    i32 height;
    i32 timeout = 0;

    fEnable = fopen(AMP_PIC_CAPTURE_ENABLE, "r");
    if (fEnable){
        fCmd = fopen(AMP_PIC_CAPTURE_CMD, "wb+");
        if (fCmd){
            fwrite((ui8 *)&command, 1, sizeof(int), fCmd);
            fclose(fCmd);
            fCmd = NULL;
            usleep(1000);

            AGILE_LOGV("before %s file is deleted", AMP_PIC_CAPTURE_CMD);
            while (((fCmd = fopen(AMP_PIC_CAPTURE_CMD, "r")) != NULL) &&
                   (timeout++ < 10)){
                fclose(fCmd);
                usleep(100);
                fCmd = NULL;
            }
            AGILE_LOGV("%s file is deleted", AMP_PIC_CAPTURE_CMD);
        }

        fYUV = fopen(AMP_PIC_CAPTURE_DUMP, "r");
        if (fYUV){
            width  = (i32)((mVideoInfoPQ.width - 1) / PIXEL_SAMPLE_INTERVAL) + 1;
            height = (i32)((mVideoInfoPQ.height - 1) / PIXEL_SAMPLE_INTERVAL) + 1;

            uyvy_linesize[0] = width*2;
            if (mpUYVYData[0] == NULL){
                mpUYVYData[0] = (ui8 *)mag_mallocz(uyvy_linesize[0]*height);
            }
            
            fread(mpUYVYData[0], 1, uyvy_linesize[0]*height, fYUV);

            if (mpRGBData[0] == NULL){
                mpRGBData[0] = (ui8 *)mag_mallocz(width * height * 3);
            }
            rgb_linesize[0] = width * 3;

            swsConvertCtx = sws_getContext(width, height, AV_PIX_FMT_UYVY422, width, height,
                                             AV_PIX_FMT_RGB24, sws_flags, NULL, NULL, NULL);
        
            if (swsConvertCtx == NULL) {
                AGILE_LOGE("Cannot initialize the conversion context\n");
                return;
            }

            AGILE_LOGV("convert the w:%d - h:%d frame to RGB24", width, height);

            sws_scale(swsConvertCtx, mpUYVYData, uyvy_linesize,
                      0, height, mpRGBData, rgb_linesize);

            sws_freeContext(swsConvertCtx);

            /*if (RGBFrameFile == NULL){
                RGBFrameFile = fopen("/tmp/picture_rgb.data","wb+");
                fwrite(mpRGBData[0], 1, rgb_linesize[0] * height, RGBFrameFile);
                fclose(RGBFrameFile);
            }*/

            for (i = 0; i < rgb_linesize[0] * height; i = i + 3){
                if (i < rgb_linesize[0] * height / 2){
                    r_top += mpRGBData[0][i];
                    g_top += mpRGBData[0][i + 1];
                    b_top += mpRGBData[0][i + 2];
                    rgb_top_num++;
                }else{
                    r_bottom += mpRGBData[0][i];
                    g_bottom += mpRGBData[0][i + 1];
                    b_bottom += mpRGBData[0][i + 2];
                    rgb_bottom_num++;
                }
            }

#if 0
            for (i = 0; i < uyvy_linesize[0] * height; i = i + 4){
                if (i < uyvy_linesize[0] * height / 2){
                    u_top += mpUYVYData[0][i];
                    y_top += mpUYVYData[0][i + 1];
                    v_top += mpUYVYData[0][i + 2];
                    y_top += mpUYVYData[0][i + 3];
                    yuv_top_num++;
                }else{
                    u_bottom += mpUYVYData[0][i];
                    y_bottom += mpUYVYData[0][i + 1];
                    v_bottom += mpUYVYData[0][i + 2];
                    y_bottom += mpUYVYData[0][i + 3];
                    yuv_bottom_num++;
                }
            }
            y_top = y_top / (yuv_top_num *2);
            u_top = u_top / yuv_top_num;
            v_top = v_top / yuv_top_num;

            y_bottom = y_bottom / (yuv_bottom_num*2);
            u_bottom = u_bottom / yuv_bottom_num;
            v_bottom = v_bottom / yuv_bottom_num;

            r_yuv_top = y_top + (1.370705 * (v_top - 128));
            g_yuv_top = y_top - (0.698001 * (v_top - 128)) - (0.337633 * (u_top - 128));
            b_yuv_top = y_top + (1.732446 * (u_top - 128));
            if (r_yuv_top > 255) r_yuv_top = 255;
            if (g_yuv_top > 255) g_yuv_top = 255;
            if (b_yuv_top > 255) b_yuv_top = 255;
            if (r_yuv_top < 0) r_yuv_top = 0;
            if (g_yuv_top < 0) g_yuv_top = 0;
            if (b_yuv_top < 0) b_yuv_top = 0;

            r_yuv_bottom = y_bottom + (1.370705 * (v_bottom - 128));
            g_yuv_bottom = y_bottom - (0.698001 * (v_bottom - 128)) - (0.337633 * (u_bottom - 128));
            b_yuv_bottom = y_bottom + (1.732446 * (u_bottom - 128));
            if (r_yuv_bottom > 255) r_yuv_bottom = 255;
            if (g_yuv_bottom > 255) g_yuv_bottom = 255;
            if (b_yuv_bottom > 255) b_yuv_bottom = 255;
            if (r_yuv_bottom < 0) r_yuv_bottom = 0;
            if (g_yuv_bottom < 0) g_yuv_bottom = 0;
            if (b_yuv_bottom < 0) b_yuv_bottom = 0;   
#endif

            mTrackInfo->top_rgb    = (r_top / rgb_top_num) << 16 | (g_top / rgb_top_num) << 8 | (b_top / rgb_top_num);
            mTrackInfo->bottom_rgb = (r_bottom / rgb_bottom_num) << 16 | (g_bottom / rgb_bottom_num) << 8 | (b_bottom / rgb_bottom_num);
            // mTrackInfo->top_rgb    = r_yuv_top << 16 | g_yuv_top << 8 | b_yuv_top;
            // mTrackInfo->bottom_rgb = r_yuv_bottom << 16 | g_yuv_bottom << 8 | b_yuv_bottom;

            AGILE_LOGV("top r(%d-%d) g(%d-%d) b(%d-%d), value(0x%x) - bottom r(%d-%d) g(%d-%d) b(%d-%d), value(0x%x)",
                       r_top / rgb_top_num, r_yuv_top,
                       g_top / rgb_top_num, g_yuv_top,
                       b_top / rgb_top_num, b_yuv_top,
                       mTrackInfo->top_rgb,
                       r_bottom / rgb_bottom_num, r_yuv_bottom,
                       g_bottom / rgb_bottom_num, g_yuv_bottom,
                       b_bottom / rgb_bottom_num, b_yuv_bottom,
                       mTrackInfo->bottom_rgb);
            fclose(fYUV);
        }

        fclose(fEnable);
    }
}
