#ifndef __MAGPLAYER_MOCK_OMXIL_H__
#define __MAGPLAYER_MOCK_OMXIL_H__

#include "Mag_looper.h"
#include <stdio.h>

class MagPlayer_Mock_OMX{
public:
    MagPlayer_Mock_OMX(char *type);
    ~MagPlayer_Mock_OMX();

    void setMagPlayerNotifier(MagMessageHandle notifyMsg);
    void start();
    
    enum{
        MagMockOMX_EmptyThisBuffer,
    };
    
private:
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
    MagMessageHandle mMagPlayerNotifier;

    MagMessageHandle mEmptyThisBufferMsg;

    char *mType;
    FILE *mDumpFile;
    
    void onEmptyThisBuffer(MagMessageHandle msg);
    void postFillThisBuffer();
    
    _status_t getLooper();
    MagMessageHandle createMessage(ui32 what);
    static void onMessageReceived(const void *msg, void *priv);
};

#endif