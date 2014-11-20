#include "MrvlAMPClock.h"

#define CLK_INPUT_PORT_NUM     0
#define CLK_OUTPUT_PORT_NUM    2

static char *amp_client_argv[] = {"client", "iiop:1.0//127.0.0.1:999/AMP::FACTORY/factory"};

MrvlAMPClock::MrvlAMPClock():
					mhFactory(NULL),
					mhVout(NULL),
					mhAren(NULL),
					mhClock(NULL){
	AMP_GetFactory(&mhFactory);
    if (mhFactory == NULL) {
        AGILE_LOGD("Initialize amp client");
        MV_OSAL_Init();
        AMP_Initialize(2, amp_client_argv, &mhFactory);
    }
}

MrvlAMPClock::~MrvlAMPClock(){
	reset();
}

_status_t MrvlAMPClock::setup(void *AudioComp, ui32 AudioPort, void *VideoComp, ui32 VideoPort){
	HRESULT ret = SUCCESS;
	AMP_COMPONENT_CONFIG amp_config;
	ui32 cnt;
	AMP_PORT_INFO PortInfo;
	i32 ClkPort2VOut = -1;

	AGILE_LOGD("enter! mhClock = 0x%p", mhClock);

	if (mhClock == NULL){
		if (AudioComp){
			mhAren = static_cast<AMP_COMPONENT>(AudioComp);
			mAudioPort = AudioPort;
		}else{
			mhAren = NULL;
			mAudioPort = 0;
		}

		if (VideoComp){
			mhVout = static_cast<AMP_COMPONENT>(VideoComp);
		    mVideoPort = VideoPort;
		}else{
			mhVout = NULL;
			mVideoPort = 0;
		}

	    AGILE_LOGD("before creating AMP clock component!");
		AMP_RPC(ret, AMP_FACTORY_CreateComponent, mhFactory, AMP_COMPONENT_CLK,  1, &mhClock);
	    if(ret != SUCCESS){
	        AGILE_LOGE("failed to create component: clock. ret = %d", ret);
	        mhClock = NULL;
	        return ret;
	    }else{
	        AGILE_LOGD("AMP create component CLOCK -- OK!");
	    }

	    AmpMemClear(&amp_config, sizeof(AMP_COMPONENT_CONFIG));
	    amp_config._d = AMP_COMPONENT_CLK;
	    amp_config._u.pCLK.mode = AMP_TUNNEL;
	    amp_config._u.pCLK.uiInputPortNum  = CLK_INPUT_PORT_NUM;
	    amp_config._u.pCLK.uiOutputPortNum = CLK_OUTPUT_PORT_NUM;
	    amp_config._u.pCLK.uiNotifierNum   = 0;
	    amp_config._u.pCLK.eClockSource    = AMP_CLK_SRC_VPP;
	    amp_config._u.pCLK.eAVSyncPolicy   = AMP_CLK_POLICY_IPTV;
	    
	    AMP_RPC(ret, AMP_CLK_Open, mhClock, &amp_config);
	    if(ret != SUCCESS){
	        AGILE_LOGE("failed to do AMP_CLK_Open(). ret = %d", ret);
	        AMP_RPC(ret, AMP_CLK_Destroy, mhClock);
	        mhClock = NULL;
	        return MAG_NO_INIT;
	    }else{
	        AGILE_LOGD("AMP do AMP_CLK_Open() -- OK!");
	    }

	    if (mhAren){
		    ret = AMP_ConnectComp(mhClock, 1, mhAren, AudioPort);
		    if(ret != SUCCESS){
		        AGILE_LOGE("failed to do AMP_ConnectApp() between clock and aren(ClkPort2ARen: %d - Aren2ClkPort: %d). ret = %d", 
		                  1, AudioPort, ret);
		        return MAG_NO_INIT;
		    }else{
		        AGILE_LOGD("AMP Connect component: Clock[%d] and Aren[%d] - OK!", 1, 0);
		    }
	    }

	    if (mhVout){
		    ret = AMP_ConnectComp(mhClock, 0, mhVout, VideoPort);
		    if(ret != SUCCESS){
		        AGILE_LOGE("failed to do AMP_ConnectComp() between clock[0] and vout[0x%x][%d]. ret = %d", mhVout, VideoPort, ret);
		        return MAG_NO_INIT;
		    }else{
		        AGILE_LOGD("AMP Connect component: Clock[0] and Vout[%d] - OK!", VideoPort);
		    }
		}
	}
	AMP_RPC(ret, AMP_CLK_SetState,  mhClock, AMP_IDLE);
	
    return MAG_NO_ERROR;
}

_status_t MrvlAMPClock::start(){
	HRESULT ret = SUCCESS;

	AGILE_LOGV("enter!");

	if (mhClock == NULL)
		return ret;

	AMP_RPC(ret, AMP_CLK_SetClockRate, mhClock, 1000, 1000);

	AMP_RPC(ret, AMP_CLK_SetState,  mhClock, AMP_EXECUTING);
    if (ret != SUCCESS){
        AGILE_LOGE("failed to AMP_CLK_SetState: AMP_EXECUTING. ret = %d", ret);
        return ret;
    }else{
        AGILE_LOGD("AMP_CLK_SetState: AMP_EXECUTING - OK!");
    }
}

_status_t MrvlAMPClock::stop(){
	HRESULT ret = SUCCESS;

	AGILE_LOGV("enter!");

	if (mhClock == NULL)
		return ret;

	if (NULL != mhClock){
        AMP_RPC(ret, AMP_CLK_SetState,  mhClock, AMP_IDLE);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to AMP_CLK_SetState: AMP_IDLE. ret = %d", ret);
            return ret;
        }else{
            AGILE_LOGD("AMP_CLK_SetState: AMP_IDLE - OK!");
        }
    }
}

_status_t MrvlAMPClock::pause(){
	HRESULT ret = SUCCESS;

	if (mhClock == NULL)
		return ret;

	AGILE_LOGV("enter!");
	AMP_RPC(ret, AMP_CLK_SetClockRate, mhClock, 0, 1000);
}

_status_t MrvlAMPClock::resume(){
	HRESULT ret = SUCCESS;

	if (mhClock == NULL)
		return ret;

	AGILE_LOGV("enter!");
	AMP_RPC(ret, AMP_CLK_SetClockRate, mhClock, 1000, 1000);
}

_status_t MrvlAMPClock::reset(){
	HRESULT ret;

	if ((NULL != mhClock) && (NULL != mhAren)){
        ret = AMP_DisconnectComp(mhClock, 1, mhAren, mAudioPort);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_DisconnectComp (Clock to Aren). ret = %d", ret);
        }else{
            AGILE_LOGD("do AMP_DisconnectComp (Clock to Aren) - OK!");
        }
    }

    if ((NULL != mhClock) && (NULL != mhVout)){
        ret = AMP_DisconnectComp(mhClock, 0, mhVout, mVideoPort);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_DisconnectComp (Clock to Vout). ret = %d", ret);
        }else{
            AGILE_LOGD("do AMP_DisconnectComp (Clock to Vout) - OK!");
        }
    }

    if (NULL != mhClock){
        AMP_RPC(ret, AMP_CLK_Close,  mhClock);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_CLK_Close. ret = %d", ret);
        }else{
            AGILE_LOGD("do AMP_CLK_Close - OK!");
        }
    }

    if (NULL != mhClock){
        AMP_RPC(ret, AMP_CLK_Destroy,  mhClock);
        if (ret != SUCCESS){
            AGILE_LOGE("failed to do AMP_CLK_Destroy. ret = %d", ret);
        }else{
            AGILE_LOGD("do AMP_CLK_Destroy - OK!");
        }
         AMP_FACTORY_Release(mhClock);
         mhClock = NULL;
    }
}

i64 MrvlAMPClock::getPlayingTime(){
    HRESULT     err;
    AMP_CLK_PTS AMP_stc;
    i64         stc90k = 0;
    AMP_RND_STAT avsync_stat;
    bool isPlayed = false;

    if (mhAren){
    	AMP_RPC(err, AMP_CLK_GetRndStatistics, mhClock, 1, &avsync_stat);
    }else if (mhVout){
    	AMP_RPC(err, AMP_CLK_GetRndStatistics, mhClock, 0, &avsync_stat);
    }else{
    	AGILE_LOGE("no pipelines are connected to the Clock!");
    	return -1;
    }

    if (err == SUCCESS){
    	AGILE_LOGV("type:%s [pts:0x%x-0x%x, In: %d, Ready: %d, Display: %d, Dropped: %d]", 
    		        avsync_stat.uiRndType == 0 ? "video" : "audio",
    		        avsync_stat.uiRndPtsHigh,
    		        avsync_stat.uiRndPtsLow,
    		        avsync_stat.uiNumVidInputFrames,
    		        avsync_stat.uiNumVidReadyFrames,
    		        avsync_stat.uiNumVidDisplayedFrames,
    		        avsync_stat.uiNumVidDroppedFrames);

    	if (avsync_stat.uiNumVidDisplayedFrames > 0){
    		isPlayed = true;
    	}
    }

    if (isPlayed){
	    AMP_RPC(err, AMP_CLK_GetSTC, mhClock, &AMP_stc);
	    if (err != SUCCESS){
	        AGILE_LOGE("failed to do AMP_CLK_GetSTC. ret = 0x%x", err);
	        return -1;
	    }else{
	        stc90k = ((AMP_stc.m_uiHigh << 32) & 0xFFFFFFFF00000000) | (AMP_stc.m_uiLow & 0xFFFFFFFF);
	    }
	}

    return (stc90k / 90);
}