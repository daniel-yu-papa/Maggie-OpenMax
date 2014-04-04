#ifndef __MAGPLAYER_MOCK_OMXIL_H__
#define __MAGPLAYER_MOCK_OMXIL_H__

#include <stdio.h>

#include "MagFramework.h"
#include "MediaBuffer.h"


class MagPlayer_Mock_OMX{
public:
    MagPlayer_Mock_OMX(const char *type);
    ~MagPlayer_Mock_OMX();

    void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    void start();
    
    enum{
        MagMockOMX_EmptyThisBuffer,
    };
    
private:
    MagMessageHandle mMagPlayerNotifier;
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;

    MagMessageHandle mEmptyThisBufferMsg;

    char *mType;
    FILE *mDumpFile;
    
    void onEmptyThisBuffer(MagMessageHandle msg);
    void postFillThisBuffer();
    
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    void proceedMediaBuffer(MediaBuffer_t *buf);
    static void onMessageReceived(const MagMessageHandle msg, void *priv);
};

#endif