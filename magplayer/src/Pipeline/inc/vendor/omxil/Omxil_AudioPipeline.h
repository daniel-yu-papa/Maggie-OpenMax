#ifndef __OMXIL_AUDIO_PIPELINE_H__
#define __OMXIL_AUDIO_PIPELINE_H__

#include "MagAudioPipelineImpl.h"
#include "Omxil_BufferMgr.h"

// #define OMXIL_AUDIO_STREAM_DUMP

#define AUIDO_PORT_BUFFER_NUMBER 16

class OmxilAudioPipeline : public MagAudioPipelineImpl{
public:
    OmxilAudioPipeline();
    virtual ~OmxilAudioPipeline();
    
	virtual _status_t init(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t setup();
    virtual _status_t start();
    virtual _status_t stop();  
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush(); 
    virtual _status_t reset();
    virtual _status_t setVolume(fp32 leftVolume, fp32 rightVolume);

    virtual _status_t pushEsPackets(MagOmxMediaBuffer_t *buf);
    virtual bool      needData();

    bool              isPlaying();
    _status_t         getClkConnectedComp(i32 *port, void **ppComp);

private:
    const char *getRoleByCodecId(ui32 OMXCodec);
    OMX_STRING    getCompNameByCodecId(ui32 OMXCodec);

    TrackInfo_t   *mTrackInfo;

    OMX_HANDLETYPE mhAudioDecoder;
    OMX_HANDLETYPE mhAudioRender;

    OMX_CALLBACKTYPE mAudioDecCallbacks;
    OMX_CALLBACKTYPE mAudioRenCallbacks;

    OmxilBufferMgr *mpBufferMgr;
    
    MagEventHandle         mADecStIdleEvent;
    MagEventHandle         mARenStIdleEvent;
    MagEventGroupHandle    mStIdleEventGroup;

    MagEventHandle         mADecStLoadedEvent;
    MagEventHandle         mARenStLoadedEvent;
    MagEventGroupHandle    mStLoadedEventGroup;

    MagEventHandle         mADecStExecutingEvent;
    MagEventHandle         mARenStExecutingEvent;
    MagEventGroupHandle    mStExecutingEventGroup;

    MagEventHandle         mADecStPauseEvent;
    MagEventHandle         mARenStPauseEvent;
    MagEventGroupHandle    mStPauseEventGroup;
    
    i32 mARenClockPortIdx;
    
#ifdef AMP_AUDIO_STREAM_DUMP
    FILE *mDumpAudioFile;
#endif

    static OMX_ERRORTYPE AudioDecoderEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);

    static OMX_ERRORTYPE AudioDecoderEmptyBufferDone(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE AudioDecoderFillBufferDone(
                                OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_PTR pAppData,
                                OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE AudioRenderEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData);
};

#endif