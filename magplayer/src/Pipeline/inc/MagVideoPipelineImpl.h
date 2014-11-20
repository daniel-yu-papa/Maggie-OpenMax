#ifndef __MAGPLAYER_VIDEO_PIPELINE_IMPL_H__
#define __MAGPLAYER_VIDEO_PIPELINE_IMPL_H__

#include <stdio.h>

#include "MagVideoPipelineImplBase.h"
#include "MagMediaBuffer.h"
#include "MagPlayerCommon.h"

// #define DUMP_VIDEO_ES_FILE

class MagVideoPipelineImpl : public MagVideoPipelineImplBase{
public:
    MagVideoPipelineImpl();
    virtual ~MagVideoPipelineImpl();

    virtual _status_t setup(i32 trackID, TrackInfo_t *sInfo);
    virtual _status_t start();
    virtual _status_t stop();  
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t flush(); 
    virtual _status_t reset();
    virtual void *getClkConnectedComp(i32 *port);

    virtual void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    virtual MagMessageHandle getMagPlayerNotifier();

    virtual _status_t     pushEsPackets(MediaBuffer_t *buf) = 0;
    virtual bool          needData() = 0;

    const char *state2String(ui32 state) {
        switch (state) {
            STRINGIFY(ST_INIT);
            STRINGIFY(ST_PLAY);
            STRINGIFY(ST_STOP);
            STRINGIFY(ST_PAUSE);
            default: return "state - unknown";
        }
    }

protected:
    void postFillThisBuffer();
    void setFillBufferFlag(bool flag);
    bool getFillBufferFlag();
    void notifyPlaybackComplete();
    i32 mState;
    bool mIsFlushed;
        
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
    MagMessageHandle mMagPlayerNotifier;
    MagMessageHandle mEmptyThisBufferMsg;

    i32 mTrackID;

    void onEmptyThisBuffer(MagMessageHandle msg);
    void onDestroy(MagMessageHandle msg);
    
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    void proceedMediaBuffer(MediaBuffer_t *buf);
    static void onMessageReceived(const MagMessageHandle msg, void *priv);

    MagMessageHandle mDestroyMsg;
    bool mbPostFillBufEvt;

#ifdef DUMP_VIDEO_ES_FILE
    FILE *mDumpFile;
#endif
};
#endif