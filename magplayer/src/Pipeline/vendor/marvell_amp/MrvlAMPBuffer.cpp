#include "MrvlAMPBuffer.h"

static const unsigned int eof_padding[] =
{
    0xff010000, 0x04000081,
    0x88888801, 0x88888888,
    0x88888888, 0x88888888,
    0x88888888, 0x88888888,
};

AMPBufferMgr::AMPBufferMgr(AMP_COMPONENT *comp, AMPComponent_type_t type, ui32 size, ui32 num):
                                                                                mBufSize(size),
                                                                                mBufNum(num),
                                                                                mAMPComp(comp),
                                                                                mAMPCompType(type),
                                                                                mFreeNodeNum(num){
    Mag_CreateMutex(&mListMutex);
    INIT_LIST(&mAMPbufFreeListHead);
    INIT_LIST(&mAMPbufBusyListHead);

    INIT_LIST(&mVDCOutputBufFreeListHead);
    INIT_LIST(&mVDCOutputBufBusyListHead);
    
#ifdef VIDEO_NONE_TUNNEL_MODE
    if (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE)
        mVDECOutputFreeNodeNum = VDEC_OUTPUT_BUFFER_NUM;
    else
        mVDECOutputFreeNodeNum = 0;
#endif
}

AMPBufferMgr::~AMPBufferMgr(){
    List_t *tmpNode;
    AMPBufInter_t *bdNode;
    AMPBuffer *aBuf;
    
    tmpNode = mAMPbufFreeListHead.next;
    while(tmpNode != &mAMPbufFreeListHead){
        bdNode = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
        aBuf = reinterpret_cast<AMPBuffer *>(bdNode->pAMPBuf);
        list_del(tmpNode);
        delete aBuf;
        tmpNode = mAMPbufFreeListHead.next;
    }

#ifdef VIDEO_NONE_TUNNEL_MODE
    if (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE){
        tmpNode = mVDCOutputBufFreeListHead.next;
        while(tmpNode != &mVDCOutputBufFreeListHead){
            bdNode = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
            aBuf = reinterpret_cast<AMPBuffer *>(bdNode->pAMPBuf);
            list_del(tmpNode);
            delete aBuf;
            tmpNode = mVDCOutputBufFreeListHead.next;
        }
    }
#endif
    Mag_DestroyMutex(&mListMutex);
    AGILE_LOGD("Cleaned up the %s AMP buffers", (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE) ? "video" : "audio");
}

void AMPBufferMgr::unregisterBDs(){
    List_t *tmpNode;
    AMPBufInter_t *bdNode;
    AMPBuffer *aBuf;
    
    tmpNode = mAMPbufFreeListHead.next;
    while(tmpNode != &mAMPbufFreeListHead){
        bdNode = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
        aBuf = reinterpret_cast<AMPBuffer *>(bdNode->pAMPBuf);
        aBuf->unregisterBD();
        tmpNode = tmpNode->next;
    }

    AGILE_LOGD("Unregister BDs of %s decoder - DONE!", (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE) ? "video" : "audio");
}

HRESULT AMPBufferMgr::Create(){
    i32 i = 0;
    AMPBuffer *node;
    AMPBufInter_t *abi;
    HRESULT ret = SUCCESS;
    
    Mag_AcquireMutex(mListMutex);
    
    for (i = 0; i < mBufNum; i++){
        node = new AMPBuffer(mAMPComp, mAMPCompType, mBufSize, AMP_COMPONENT_PORT_INPUT);
        abi = (AMPBufInter_t *)malloc(sizeof(AMPBufInter_t));
        memset(abi, 0, sizeof(AMPBufInter_t));
        if ((ret = node->Allocate()) == SUCCESS){
            abi->pAMPBuf = reinterpret_cast<void *>(node);
            INIT_LIST(&abi->Node);
            list_add_tail(&abi->Node, &mAMPbufFreeListHead);
        }else{
            delete node;
            node = NULL;
        }
    }

#ifdef VIDEO_NONE_TUNNEL_MODE
    if (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE){
        for (i = 0; i < VDEC_OUTPUT_BUFFER_NUM; i++){
            node = new AMPBuffer(mAMPComp, mAMPCompType, (4 * 1024 * 1024)/*1920 * 1088 * 2*/, AMP_COMPONENT_PORT_OUTPUT);
            abi = (AMPBufInter_t *)malloc(sizeof(AMPBufInter_t));
            memset(abi, 0, sizeof(AMPBufInter_t));
            if ((ret = node->Allocate()) == SUCCESS){
                abi->pAMPBuf = reinterpret_cast<void *>(node);
                INIT_LIST(&abi->Node);
                list_add_tail(&abi->Node, &mVDCOutputBufFreeListHead);
            }else{
                delete node;
                node = NULL;
            }
        }
    }
#endif

    Mag_ReleaseMutex(mListMutex);
    
    return ret;
}

AMPBufInter_t *AMPBufferMgr::getAMPBuffer(){
    List_t *tmpNode = NULL;
    AMPBufInter_t *bd = NULL;
    
    Mag_AcquireMutex(mListMutex);

    tmpNode = mAMPbufFreeListHead.next;
    if (tmpNode != &mAMPbufFreeListHead){
        list_del(tmpNode);
        bd = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
        list_add_tail(tmpNode, &mAMPbufBusyListHead);
        mFreeNodeNum--;

        /*if ((mFreeNodeNum > (mBufNum  - 20)) || (mFreeNodeNum < 20))
            AGILE_LOGD("[getAMPBuffer(H:%d - L:%d)]: %s free nodes = %d", 
                      (mBufNum  - 20), 20,
                      (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE ? "Video" : "Audio"), mFreeNodeNum);*/
    }else{
        AGILE_LOGE("[getAMPBuffer(%s)]: no node in the mAMPbufFreeListHead! mFreeNodeNum = %d", 
                  (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE ? "video" : "audio"), mFreeNodeNum);
    }
    
    Mag_ReleaseMutex(mListMutex);
    return bd;
}

i64 AMPBufferMgr::putAMPBuffer(struct AMP_BD_ST *hBD){
    List_t *tmpNode;
    AMPBufInter_t *bdNode;
    AMPBuffer *aBuf;
    
    Mag_AcquireMutex(mListMutex);
    
    tmpNode = mAMPbufBusyListHead.next;
    while(tmpNode != &mAMPbufBusyListHead){
        bdNode = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
        aBuf = reinterpret_cast<AMPBuffer *>(bdNode->pAMPBuf);
        if (hBD == aBuf->getBD()){
            aBuf->Reset();
            list_del(tmpNode);
            list_add_tail(tmpNode, &mAMPbufFreeListHead);
            mFreeNodeNum++;
            AGILE_LOGD("[putAMPBuffer(%s)]: mFreeNodeNum = %d", (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE ? "video" : "audio"), mFreeNodeNum);
            Mag_ReleaseMutex(mListMutex);
            return 0;
        }
        tmpNode = tmpNode->next;
    }
    AGILE_LOGE("[AMPBufferMgr::putAMPBuffer]: failed to find the BD[0x%x] in the busy list", (ui32)hBD);
    
    Mag_ReleaseMutex(mListMutex);
    return 0;
}

#ifdef VIDEO_NONE_TUNNEL_MODE
AMPBufInter_t *AMPBufferMgr::getVDECOutputAMPBuffer(){
    List_t *tmpNode = NULL;
    AMPBufInter_t *bd = NULL;
    
    Mag_AcquireMutex(mListMutex);

    tmpNode = mVDCOutputBufFreeListHead.next;
    if (tmpNode != &mVDCOutputBufFreeListHead){
        list_del(tmpNode);
        bd = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
        list_add_tail(tmpNode, &mVDCOutputBufBusyListHead);
        mVDECOutputFreeNodeNum--;
    }else{
        AGILE_LOGE("[getAMPBuffer(video output)]: no node in the mAMPbufFreeListHead! mFreeNodeNum = %d", 
                  mVDECOutputFreeNodeNum);
    }
    
    Mag_ReleaseMutex(mListMutex);
    return bd;
}

i64 AMPBufferMgr::putVDECOutputAMPBuffer(struct AMP_BD_ST *hBD){
    List_t *tmpNode;
    AMPBufInter_t *bdNode;
    AMPBuffer *aBuf;
    i64 playingTime;
    
    Mag_AcquireMutex(mListMutex);
    
    tmpNode = mVDCOutputBufBusyListHead.next;
    while(tmpNode != &mVDCOutputBufBusyListHead){
        bdNode = (AMPBufInter_t *)list_entry(tmpNode, AMPBufInter_t, Node);
        aBuf = reinterpret_cast<AMPBuffer *>(bdNode->pAMPBuf);
        if (hBD == aBuf->getBD()){
            playingTime = aBuf->getPlayingTimeInMS();
            aBuf->Reset();
            list_del(tmpNode);
            list_add_tail(tmpNode, &mVDCOutputBufFreeListHead);
            mVDECOutputFreeNodeNum++;
            AGILE_LOGD("[putAMPBuffer(video output)]: mFreeNodeNum = %d",  mVDECOutputFreeNodeNum);
            Mag_ReleaseMutex(mListMutex);
            return playingTime;
        }
        tmpNode = tmpNode->next;
    }
    AGILE_LOGE("[AMPBufferMgr::putVDECOutputAMPBuffer]: failed to find the BD[0x%x] in the busy list", (ui32)hBD);
    
    Mag_ReleaseMutex(mListMutex);
    return 0;
}
#endif

bool AMPBufferMgr::needPushBuffers(void){
    if (mFreeNodeNum > 0){
        AGILE_LOGV("%s, free bds = %d", (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE ? "video" : "audio"), mFreeNodeNum);
        return true; 
    }else{
        return false;
    }
}
void AMPBufferMgr::waitForAllBufFree(bool long_wait){
    List_t *tmpNode;
    ui32 timeout = 0;
    ui32 loop = 0;
    
    AGILE_LOGD("enter!");

    if (long_wait){
        timeout = 1000; /* long waiting: 1s*/
    }else{
        timeout = 400; /* short waiting: 400ms*/
    }
restart:
    Mag_AcquireMutex(mListMutex);
    if (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE){
        if ((mFreeNodeNum < mBufNum) && (loop < timeout)){
            Mag_ReleaseMutex(mListMutex);
            loop++;
            usleep(1000);
            goto restart;
        }

#ifdef VIDEO_NONE_TUNNEL_MODE
        if ((mVDECOutputFreeNodeNum < VDEC_OUTPUT_BUFFER_NUM) && (loop < timeout)){
            Mag_ReleaseMutex(mListMutex);
            loop++;
            usleep(1000);
            goto restart;
        }
#endif
    }else{
        if ((mFreeNodeNum < mBufNum) && (loop < timeout)){
            Mag_ReleaseMutex(mListMutex);
            loop++;
            usleep(1000);
            goto restart;
        }
    }

#ifdef VIDEO_NONE_TUNNEL_MODE
    AGILE_LOGD("[waitForAllBufFree(%s)]: mFreeNodeNum = %d, mVDECOutputFreeNodeNum = %d", (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE ? "video" : "audio"), 
              mFreeNodeNum, mVDECOutputFreeNodeNum);
#else
    AGILE_LOGD("[waitForAllBufFree(%s)]: mFreeNodeNum = %d", (mAMPCompType == AMP_COMPONENT_VDEC_PIPELINE ? "video" : "audio"), 
              mFreeNodeNum);

#endif
    Mag_ReleaseMutex(mListMutex);
}

int AMPBufferMgr::getPushedDataSize(){
    return ((mBufNum - mFreeNodeNum) * mBufSize);
}

AMPBuffer::AMPBuffer(AMP_COMPONENT *comp, AMPComponent_type_t type, ui32 size, AMPComp_port_dir_t port_dir):
    mBufSize(size),
    mAMPComp(comp),
    mAMPCompType(type),
    mPtsIndex(0),
    mMemInfoIndex(0),
    mBufFilledSize(0),
    mBufOffset(0),
    mPortDir(port_dir){
}

AMPBuffer::~AMPBuffer(){
    HRESULT ret = SUCCESS;
    
    ret = AMP_BDTAG_Clear(mBD);
    if (ret != SUCCESS){
        AGILE_LOGE("faile to clear bd");
    }
    
    ret = AMP_BD_Free(mBD);        
    if (ret != SUCCESS){
        AGILE_LOGE("faile to release bd");
    }
    
    ret = AMP_SHM_Release(mShm);        
    if (ret != SUCCESS){
        AGILE_LOGE("faile to release shm");
    }  
}

HRESULT AMPBuffer::unregisterBD(){
    HRESULT ret;

    switch (mAMPCompType){
        case AMP_COMPONENT_VDEC_PIPELINE:
            if (mPortDir == AMP_COMPONENT_PORT_INPUT){
                AMP_RPC(ret, AMP_VDEC_UnregisterBD, *mAMPComp, AMP_PORT_INPUT, 0, mBD);
                if (SUCCESS != ret){
                    AGILE_LOGE("failed to AMP_VDEC_UnregisterBD(video input), ret = 0x%x", ret);
                }
            }else{
                AMP_RPC(ret, AMP_VDEC_UnregisterBD, *mAMPComp, AMP_PORT_OUTPUT, 0, mBD);
                if (SUCCESS != ret){
                    AGILE_LOGE("failed to AMP_VDEC_UnregisterBD(video output), ret = 0x%x", ret);
                }
            }
            break;

        case AMP_COMPONENT_ADEC_PIPELINE:
            AMP_RPC(ret, AMP_ADEC_UnregisterBD, *mAMPComp, AMP_PORT_INPUT, 0, mBD);
            if (SUCCESS != ret){
                AGILE_LOGE("failed to AMP_ADEC_UnregisterBD(audio), ret = 0x%x", ret);
            }
            break;

         default:
            break;
    }
    return ret;
}

HRESULT AMPBuffer::registerBD(){
    HRESULT ret = SUCCESS; 
    
    switch (mAMPCompType){
        case AMP_COMPONENT_VDEC_PIPELINE:
            AMP_RPC(ret, AMP_VDEC_RegisterBD, *mAMPComp, AMP_PORT_INPUT, 0, mBD);
            if (SUCCESS != ret){
                AGILE_LOGE("[registerBD]: failed to AMP_VDEC_RegisterBD, ret = 0x%x", ret);
                return ret;
            }
            
            break;

        case AMP_COMPONENT_ADEC_PIPELINE:   
            AMP_RPC(ret, AMP_ADEC_RegisterBD, *mAMPComp, AMP_PORT_INPUT, 0, mBD);
            if (SUCCESS != ret){
                AGILE_LOGE("[registerBD]: failed to AMP_ADEC_RegisterBD, ret = 0x%x", ret);
                return ret;
            }
            break;

         default:
            break;
    }
    return ret;
}

HRESULT AMPBuffer::Allocate(){
    HRESULT ret;
    ui32 space = 0;

    AMP_BDTAG_MEMINFO   memInfo;

    memset(&memInfo, 0, sizeof(AMP_BDTAG_MEMINFO));
    
    memInfo.Header.eType = AMP_BDTAG_ASSOCIATE_MEM_INFO;
    memInfo.Header.uLength = sizeof(AMP_BDTAG_MEMINFO);
    memInfo.uSize = mBufSize;
    memInfo.uMemOffset = 0;
    
    ret = AMP_BD_Allocate(&mBD);
    if (SUCCESS != ret){
        AGILE_LOGE("failed to allocate BD");
        return ERR_NOMEM;
    }
    
    ret = AMP_SHM_Allocate(AMP_SHM_DYNAMIC/*AMP_SHM_STATIC*/, mBufSize, BUFFER_ALIGN, &mShm);
    if (SUCCESS != ret){
        AMP_BD_Free(mBD);
        AGILE_LOGE("failed to allocate BD");
        return ERR_NOMEM;
    }

    ret = AMP_SHM_GetVirtualAddress(mShm, 0, &mpUserBuf);
    if (SUCCESS != ret){
        AMP_BD_Free(mBD);
        AMP_SHM_Release(mShm);
        AGILE_LOGE("failed to do AMP_SHM_GetVirtualAddress()");
        return ERR_NOMEM;
    }else{
        //AGILE_LOGD("get virtual address 0x%x for SHM", reinterpret_cast<ui32>(mpUserBuf));
    }
    
    memInfo.uMemHandle = mShm;

    ret = AMP_BDTAG_Append(mBD, (ui8 *)&memInfo, &mMemInfoIndex, &space);
    if (SUCCESS != ret){
        AMP_BD_Free(mBD);
        AMP_SHM_Release(mShm);
        AGILE_LOGE("failed to do AMP_BDTAG_Append()");
        return ERR_NOMEM;
    }
    
    switch (mAMPCompType){
        case AMP_COMPONENT_VDEC_PIPELINE:
            if (mPortDir == AMP_COMPONENT_PORT_INPUT){
                ret = addPtsTag();
                if (SUCCESS != ret){
                    AMP_BD_Free(mBD);
                    AMP_SHM_Release(mShm);
                    AGILE_LOGE("failed to do addPtsTag()");
                    return ERR_NOMEM;
                }
    
                AMP_RPC(ret, AMP_VDEC_RegisterBD, *mAMPComp, AMP_PORT_INPUT, 0, mBD);
                if (SUCCESS != ret){
                    AMP_BD_Free(mBD);
                    AGILE_LOGE("failed to AMP_VDEC_RegisterBD, ret = 0x%x", ret);
                    return ret;
                }
            }else{
                ret = addFrameInfoTag();
                if (SUCCESS != ret){
                    AMP_BD_Free(mBD);
                    AMP_SHM_Release(mShm);
                    AGILE_LOGE("failed to do addFrameInfoTag()");
                    return ERR_NOMEM;
                }
    
                AMP_RPC(ret, AMP_VDEC_RegisterBD, *mAMPComp, AMP_PORT_OUTPUT, 0, mBD);
                if (SUCCESS != ret){
                    AMP_BD_Free(mBD);
                    AGILE_LOGE("failed to AMP_VDEC_RegisterBD, ret = 0x%x", ret);
                    return ret;
                }
            }
            break;

        case AMP_COMPONENT_ADEC_PIPELINE:
            ret = addPtsTag();
            if (SUCCESS != ret){
                AMP_BD_Free(mBD);
                AMP_SHM_Release(mShm);
                AGILE_LOGE("failed to do addPtsTag()");
                return ERR_NOMEM;
            }
                
            AMP_RPC(ret, AMP_ADEC_RegisterBD, *mAMPComp, AMP_PORT_INPUT, 0, mBD);
            if (SUCCESS != ret){
                AMP_BD_Free(mBD);
                AGILE_LOGE("failed to AMP_ADEC_RegisterBD, ret = 0x%x", ret);
                return ret;
            }
            break;

         default:
            break;
    }

    return SUCCESS;
}

void AMPBuffer::Reset(){
    mBufFilledSize = 0;
    mBufOffset = 0;
}

AMP_BD_HANDLE AMPBuffer::getBD(){
    return mBD;
}

AMP_SHM_HANDLE AMPBuffer::getSHM(){
    return mShm;
}

void AMPBuffer::updateDataSize(ui32 size){
    mBufFilledSize += size;
}

void *AMPBuffer::getBufferStatus(ui32 *offset, ui32 *freeSize){
    *offset = mBufFilledSize;
    *freeSize = mBufSize - mBufFilledSize;
    //AGILE_LOGD("vBuf = 0x%x, offset=%d, freesize=%d", reinterpret_cast<ui32>(mpUserBuf), *offset, *freeSize);
    return mpUserBuf;
    //return mpUserBuf;
}

AMP_BDTAG_UNITSTART * AMPBuffer::getUnitStartTag() {
  void *pts_tag = NULL;
  HRESULT err = AMP_BDTAG_GetWithIndex(mBD, mPtsIndex, &pts_tag);
  return static_cast<AMP_BDTAG_UNITSTART *>(pts_tag);
}

HRESULT AMPBuffer::updateUnitStartPtsTagInUS(i64 pts, ui32 stream_pos, ui8 flag){
    HRESULT ret;
    AMP_BDTAG_UNITSTART *unit_start = getUnitStartTag();
    ui64 pts90k = 0;
    
    if (pts >= 0){
        pts90k = pts * 9 / 100;
        unit_start->uPtsHigh = static_cast<ui32>(0xFFFFFFFF & (pts90k >> 32)) | 0x80000000;
        unit_start->uPtsLow  = static_cast<ui32>(0xFFFFFFFF & pts90k);

        /*only add stream position for video stream*/
        if (0 == flag)
            unit_start->uStrmPos = stream_pos;
    }else{
        unit_start->uPtsHigh = 0;
        unit_start->uPtsLow  = 0;
    }
    mUpdatedPTS = pts90k;
    
    return SUCCESS;
}

HRESULT AMPBuffer::updateUnitStartPtsTag(i64 pts, i32 stream_pos){
    HRESULT ret;
    AMP_BDTAG_UNITSTART *unit_start;
    
    unit_start = getUnitStartTag();
    if (pts >= 0){
        unit_start->uPtsHigh = static_cast<ui32>(0xFFFFFFFF & (pts >> 32)) | 0x80000000;
        unit_start->uPtsLow  = static_cast<ui32>(0xFFFFFFFF & pts);
        mUpdatedPTS = pts;
    }else{
        unit_start->uPtsHigh = 0;
        unit_start->uPtsLow  = 0;
        mUpdatedPTS = 0;
    }

    /*only add stream position for video stream*/
    if (stream_pos >= 0)
        unit_start->uStrmPos = stream_pos;
    
    return SUCCESS;
}

i64 AMPBuffer::getPlayingTimeInMS(){
    i64 playingTime;
    HRESULT err = SUCCESS;
    AMP_BGTAG_FRAME_INFO *frame_info = NULL;
    
    err = getFrameInfo(&frame_info);
    i64 pts_low, pts_high;
    if (frame_info) {
      pts_low = static_cast<i64>(frame_info->uiPtsLow);
      pts_high = static_cast<i64>(frame_info->uiPtsHigh);
      if (frame_info->uiPtsHigh & TIME_STAMP_VALID_MASK) {
        playingTime = (((pts_high & 0x7FFFFFFFLL) << 32 | pts_low) * 100 / 9);
      }
    }
    AGILE_LOGD("playing time = %lld", playingTime);
    return playingTime;
}
HRESULT AMPBuffer::addPtsTag() {
  ui32 space = 0;
  AMP_BDTAG_UNITSTART pts_tag;
  memset(&pts_tag, 0, sizeof(AMP_BDTAG_UNITSTART));
  pts_tag.Header = {AMP_BDTAG_BS_UNITSTART_CTRL, sizeof(AMP_BDTAG_UNITSTART)};
  return AMP_BDTAG_Append(mBD, (ui8 *)&pts_tag, &mPtsIndex, &space);
}

HRESULT AMPBuffer::addFrameInfoTag() {
  UINT32 space = 0;
  AMP_BGTAG_FRAME_INFO frame_tag;
  memset(&frame_tag, 0, sizeof(AMP_BGTAG_FRAME_INFO));
  frame_tag.Header = {AMP_BGTAG_FRAME_INFO_META, sizeof(AMP_BGTAG_FRAME_INFO)};
  return AMP_BDTAG_Append(mBD, (UINT8 *)&frame_tag, &mFrameInfoIndex, &space);
}

HRESULT AMPBuffer::getFrameInfo(AMP_BGTAG_FRAME_INFO **frame_info) {
  HRESULT err = SUCCESS;
  void *amp_frame_info = NULL;
  err = AMP_BDTAG_GetWithIndex(mBD, mFrameInfoIndex, &amp_frame_info);
  *frame_info = static_cast<AMP_BGTAG_FRAME_INFO *>(amp_frame_info);
  return err;
}

void AMPBuffer::updateDataPaddingSize(ui32 paddingSize){
#if 0
    if (paddingSize == 0)
        return;
    
    if (paddingSize < 32){
        memset((ui8 *)mpUserBuf + mBufFilledSize, 0x88, paddingSize);
    }else{
        memcpy((ui8 *)mpUserBuf + mBufFilledSize, (ui8 *)eof_padding, 32);
        memset((ui8 *)mpUserBuf + mBufFilledSize + 32, 0x88, (paddingSize - 32));
    }
#else
    memset((ui8 *)mpUserBuf + mBufFilledSize, 0x88, paddingSize);
#endif
    mBufFilledSize += paddingSize;
}

AMP_BDTAG_MEMINFO * AMPBuffer::getMemInfo() {
  void *mem_info = NULL;
  HRESULT err = AMP_BDTAG_GetWithIndex(mBD, mMemInfoIndex, &mem_info);
  if (SUCCESS != err){
    AGILE_LOGE("[AMPBuffer::getMemInfo]: failed to do AMP_BDTAG_GetWithIndex");
    return NULL;
  }
  return static_cast<AMP_BDTAG_MEMINFO *>(mem_info);
}

HRESULT AMPBuffer::clearBdTag() {
  HRESULT err = SUCCESS;
  if (NULL != mBD) {
    err = AMP_BDTAG_Clear(mBD);
  }
  return err;
}

HRESULT AMPBuffer::pushVDECOutputBD(AMP_COMPONENT *ampComp) {
  HRESULT ret;
  AMP_RPC(ret, AMP_VDEC_PushBD, *ampComp, AMP_PORT_OUTPUT, 0, mBD);

  if (ret != SUCCESS){
    AGILE_LOGE("[AMPBuffer::pushVDECOutputBD]: failed to do AMP_VDEC_PushBD(). ret = %d", ret);
    return -1;
  }
  return ret;
}

/*
* flag:
* 0 - Video
* 1 - Audio
* return:
* -1: error
* 1: pushed out the whole buffer
* 0: success but not pushing out the whole buffer
*/
i8 AMPBuffer::updateMemInfo(AMP_COMPONENT *ampComp, ui8 flag, bool isEOS) {
  HRESULT ret = SUCCESS;
  AMP_BDTAG_MEMINFO *mem_info = NULL;

  if (0 == flag){
        mem_info = getMemInfo();
      
        mem_info->uSize      = mBufFilledSize;
        mem_info->uMemOffset = 0;
        if (isEOS)
            mem_info->uFlag |= AMP_MEMINFO_FLAG_EOS_MASK;
        else
            mem_info->uFlag &= ~AMP_MEMINFO_FLAG_EOS_MASK;

        mBufFilledSize       = 0;
        
        if (mem_info->uSize > 0){
            /*could not do clean cache in bg3cd-A0, and enabled in Z1/Z2*/
            /*ret = AMP_SHM_CleanCache(mShm, 0, mBufSize); */
            if (SUCCESS == ret){
                if (isEOS){
                    AGILE_LOGV("push EOS bd(0x%p) to video decoder!", mBD);
                }
                AMP_RPC(ret, AMP_VDEC_PushBD, *ampComp, AMP_PORT_INPUT, 0, mBD);

                if (ret != SUCCESS){
                  AGILE_LOGE("[AMPBuffer::updateMemInfo(video)]: failed to do AMP_VDEC_PushBD(). ret = %d", ret);
                  return -1;
                }

                AGILE_LOGD("[AMPBuffer::updateMemInfo]: pushed out video AMP Buffer: 0x%x, PTS tag is 0X%llX", 
                         reinterpret_cast<ui32>(mpUserBuf), mUpdatedPTS);    
            }else{
                AGILE_LOGE("[AMPBuffer::updateMemInfo(video)]: failed to do AMP_SHM_CleanCache, ret = 0x%x", ret);
                return -1;
            }
        }else{
            AGILE_LOGE("[video updateMemInfo]: size = %d. [mBufFilledSize=%d, mBufSize=%d]", 
                      mem_info->uSize, mBufFilledSize, mBufSize); 
        }

        return 1;
   }else if (1 == flag){
        mem_info = getMemInfo();

        if (isEOS){
            /*mem_info->uSize      = mBufSize;*/
            mem_info->uSize      = 0;
            mem_info->uMemOffset = 0;
            /*memset((ui8 *)mpUserBuf, 0x88, mBufSize);*/
            mem_info->uFlag |= AMP_MEMINFO_FLAG_EOS_MASK;
        }else{
            mem_info->uSize      = mBufFilledSize;
            mem_info->uMemOffset = 0;
            mem_info->uFlag &= ~AMP_MEMINFO_FLAG_EOS_MASK;
        }

        mBufFilledSize       = 0;

        if (mem_info->uSize > 0){
            /*could not do clean cache in bg3cd-A0, and enabled in Z1/Z2*/
            /*ret = AMP_SHM_CleanCache(mShm, 0, mBufSize);*/
            if (SUCCESS == ret){
                if (isEOS){
                    AGILE_LOGV("push EOS bd(0x%p) to audio decoder!", mBD);
                }
                AMP_RPC(ret, AMP_ADEC_PushBD, *ampComp, AMP_PORT_INPUT, 0, mBD);
                
                if (ret != SUCCESS){
                    AGILE_LOGE("[AMPBuffer::updateMemInfo(audio)]: failed to do AMP_ADEC_PushBD. ret = %d", ret);
                    return -1;
                }else{
                    AGILE_LOGD("[AMPBuffer::updateMemInfo]: pushed out audio AMP Buffer: 0x%x, PTS tag is 0X%llX", 
                         reinterpret_cast<ui32>(mpUserBuf), mUpdatedPTS);    
                } 
            }else{
                AGILE_LOGE("[AMPBuffer::updateMemInfo(audio)]: failed to do AMP_SHM_CleanCache, ret = 0x%x", ret);
                return -1;
            }
        }else{
            AGILE_LOGE("[audio updateMemInfo]: size = %d. [mBufFilledSize=%d, mBufSize=%d]", 
                      mem_info->uSize, mBufFilledSize, mBufSize); 
        }

        return 1;
   } 
   return 0;
}

