#include "MrvlAMPBuffer.h"
#include "MagVideoPipelineImpl.h"

/*#define AMP_VIDEO_STREAM_DUMP*/

typedef struct{
    i32 width;
    i32 height;
    bool IsIntlSeq;
    i32 framerate;
    i32 framerate_den;
    i32 ar_w;
    i32 ar_h;
}VideoInfoPQ_t;

class MrvlAMPVideoPipeline : public MagVideoPipelineImpl{
public:
    MrvlAMPVideoPipeline();
    virtual ~MrvlAMPVideoPipeline();

	virtual _status_t setup(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t start();
    virtual _status_t stop();  
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush(); 
    virtual _status_t reset();
    virtual void *getClkConnectedComp(i32 *port);

    virtual _status_t pushEsPackets(MediaBuffer_t *buf);
    virtual bool      needData();
    
    void      setDisplayRect(i32 x, i32 y, i32 w, i32 h);
    bool      isPlaying();
    AMPBuffer *getEOSBuffer();
    void      getDispResolution();

private:
	AMP_FACTORY    mhFactory;

    AMP_DISP       mhDisp;
	AMP_COMPONENT  mhVdec;
	AMP_COMPONENT  mhVout;

	HANDLE         mVdecEvtListener;
    AMPBufferMgr   *mpAMPVideoBuf;
    // AMPBufInter_t  *mUsingVideoBufDesc;
    ui32           mStreamPosition;
	VideoInfoPQ_t   mVideoInfoPQ;
	// i32             mBufferNumber;
	i32             mVoutPort2Clk;
    AMPBuffer       *mpEOSBuffer;

    ui32            mDisplayWidth;
    ui32            mDisplayHeight;
    
    bool            mGetFirstIFrame;
    
    TrackInfo_t     *mTrackInfo;

    ui32            mOutputFrameCnt;

    ui8             *mpRGBData[1];
    ui8             *mpUYVYData[1];

#ifdef AMP_VIDEO_STREAM_DUMP
    FILE *mDumpVideoFile;
#endif

	ui32 getPaddingSize(ui32 data_size);
    ui32 getAMPVideoFormat(ui32 OMXCodec);
    void SwapDisplayPlane();
    void calcPictureRGB();

    static HRESULT videoPushBufferDone(CORBA_Object hCompObj, AMP_PORT_IO ePortIo,
                                  UINT32 uiPortIdx, struct AMP_BD_ST *hBD,
                                  AMP_IN void *pUserData);
	static HRESULT VdecEventHandle(HANDLE hListener, AMP_EVENT *pEvent, VOID *pUserData);
};