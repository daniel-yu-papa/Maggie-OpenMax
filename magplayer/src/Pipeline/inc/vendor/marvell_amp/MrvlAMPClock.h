extern "C" {
#include <OSAL_api.h>
#include <amp_client_support.h>
#include <amp_component.h>
#include <amp_buf_desc.h>
#include <amp_sound_api.h>

// #include "AMPList.h"
}
#include "MagClockImpl.h"
#include "framework/MagFramework.h"

class MrvlAMPClock : public MagClockImpl{
public:
	MrvlAMPClock();
    virtual ~MrvlAMPClock();

    virtual _status_t setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort);
    virtual _status_t start();
    virtual _status_t stop();
    virtual _status_t pause();
    virtual _status_t resume();
    virtual _status_t reset();
    virtual i64       getPlayingTime();

private:
	AMP_FACTORY   mhFactory;

	AMP_COMPONENT mhVout;
    AMP_COMPONENT mhAren;

	AMP_COMPONENT mhClock;

	ui32 mAudioPort;
	ui32 mVideoPort;
};