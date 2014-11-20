#ifndef __MRVL_AMP_BUFFER_H__
#define __MRVL_AMP_BUFFER_H__

#include "framework/MagFramework.h"

extern "C" {
#include <OSAL_api.h>
#include <amp_client_support.h>
#include <amp_component.h>
#include <amp_buf_desc.h>
#include <amp_sound_api.h>
#include "amp_event_listener.h"
// #include "AMPList.h"
}

//#define VIDEO_NONE_TUNNEL_MODE

#define BUFFER_ALIGN 32

#define VDEC_OUTPUT_BUFFER_NUM 16

typedef struct{
    List_t Node;    
    void *pAMPBuf;
}AMPBufInter_t;

typedef enum{
    AMP_COMPONENT_VDEC_PIPELINE,
    AMP_COMPONENT_ADEC_PIPELINE    
}AMPComponent_type_t;

typedef enum{
    AMP_COMPONENT_PORT_OUTPUT,
    AMP_COMPONENT_PORT_INPUT
}AMPComp_port_dir_t;

class AMPBufferMgr{
public:
    AMPBufferMgr(AMP_COMPONENT *comp, AMPComponent_type_t type, ui32 size, ui32 num);
    ~AMPBufferMgr();

    HRESULT Create();
    AMPBufInter_t *getAMPBuffer();
    i64  putAMPBuffer(struct AMP_BD_ST *hBD);
    AMPBufInter_t *getVDECOutputAMPBuffer();
    i64 putVDECOutputAMPBuffer(struct AMP_BD_ST *hBD);
    
    bool needPushBuffers(void);
    void waitForAllBufFree(bool long_wait);
    int  getPushedDataSize();
    void unregisterBDs();
    
private:
    ui32 mBufSize;
    ui32 mBufNum;
    AMP_COMPONENT   *mAMPComp;
    AMPComponent_type_t mAMPCompType;
    ui32 mFreeNodeNum;

    MagMutexHandle mListMutex;
    List_t mAMPbufFreeListHead;
    List_t mAMPbufBusyListHead;
    
    List_t mVDCOutputBufFreeListHead;
    List_t mVDCOutputBufBusyListHead;
    ui32 mVDECOutputFreeNodeNum;
};

class AMPBuffer{
public:
    AMPBuffer(AMP_COMPONENT *comp, AMPComponent_type_t type, ui32 size, AMPComp_port_dir_t port_dir);
    ~AMPBuffer();
    
    HRESULT Allocate();
    HRESULT unregisterBD();
    HRESULT registerBD();
    
    HRESULT updateUnitStartPtsTag(i64 pts, i32 stream_pos);
    HRESULT updateUnitStartPtsTagInUS(i64 pts, ui32 stream_pos, ui8 flag);
    void updateDataSize(ui32 size);
    void updateDataPaddingSize(ui32 paddingSize);
    void *getBufferStatus(ui32 *offset, ui32 *freeSize);
    struct AMP_BD_ST *getBD();
    void Reset();
    i8 updateMemInfo(AMP_COMPONENT *ampComp, ui8 flag, bool isEOS = false);
    HRESULT pushVDECOutputBD(AMP_COMPONENT *ampComp);
    AMP_SHM_HANDLE getSHM();
    HRESULT clearBdTag();
    i64 getPlayingTimeInMS();
    
private:
    HRESULT addPtsTag();
    HRESULT addFrameInfoTag();
    AMP_BDTAG_MEMINFO *getMemInfo();
    AMP_BDTAG_UNITSTART * getUnitStartTag();
    HRESULT getFrameInfo(AMP_BGTAG_FRAME_INFO **frame_info);

    
    AMP_COMPONENT       *mAMPComp;
    AMPComponent_type_t mAMPCompType;
    AMPComp_port_dir_t  mPortDir;
    AMP_SHM_HANDLE      mShm;
    void                *mpUserBuf;
    AMP_BD_HANDLE       mBD;
    ui32                mBufSize;
    ui32                mBufFilledSize;
    ui32                mBufOffset;
    ui8                 mPtsIndex;
    ui8                 mMemInfoIndex;
    ui8                 mFrameInfoIndex;

    /*for debugging only*/
    ui64                mUpdatedPTS;
};
#endif
