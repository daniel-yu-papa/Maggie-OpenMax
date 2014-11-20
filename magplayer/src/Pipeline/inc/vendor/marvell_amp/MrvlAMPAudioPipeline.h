#include "MrvlAMPBuffer.h"
#include "MagAudioPipelineImpl.h"

// #define AMP_AUDIO_STREAM_DUMP

class MrvlAMPAudioPipeline : public MagAudioPipelineImpl{
public:
    MrvlAMPAudioPipeline();
    virtual ~MrvlAMPAudioPipeline();
    
	virtual _status_t setup(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t start();
    virtual _status_t stop();  
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush(); 
    virtual _status_t reset();
    virtual void *getClkConnectedComp(i32 *port);
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume);

    virtual _status_t pushEsPackets(MediaBuffer_t *buf);
    virtual bool      needData();

    bool      isPlaying();
    AMPBuffer *getEOSBuffer();

private:
	AMP_FACTORY   mhFactory;

	AMP_COMPONENT mhAdec;
    AMP_COMPONENT mhAren;
    HANDLE        mhAMPSound;
    
    HANDLE        mAMPSoundTunnel;
    HANDLE        mAdecEvtListener;
    
    AMPBufferMgr  *mpAMPAudioBuf;
    // AMPBufInter_t *mUsingAudioBufDesc;
    AMPBuffer     *mpEOSBuffer;

    TrackInfo_t   *mTrackInfo;
    // i32            mBufferNumber;
    // AudioStreamInfo mAudioInfoPQ;
#ifdef AMP_AUDIO_STREAM_DUMP
    FILE *mDumpAudioFile;
#endif
	static HRESULT audioPushBufferDone(CORBA_Object hCompObj, AMP_PORT_IO ePortIo,
                                 UINT32 uiPortIdx, struct AMP_BD_ST *hBD,
                                 AMP_IN void *pUserData);

	ui32 getAMPAudioFormat(ui32 OMXCodec);

    static HRESULT AdecEventHandle(HANDLE hListener, AMP_EVENT *pEvent, VOID *pUserData);
};