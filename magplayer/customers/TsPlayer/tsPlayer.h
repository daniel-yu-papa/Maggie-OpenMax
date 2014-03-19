#ifndef __TSPLAYER_DRIVER_H__
#define __TSPLAYER_DRIVER_H__

#include "Mag_pub_def.h"
#include "Mag_pub_type.h"
#include "MagSingleton.h"

#include "MagPlayerClient.h"
#include "streamBuffer.h"

typedef enum {
    VFORMAT_UNKNOWN = -1,
    VFORMAT_MPEG12 = 0,
    VFORMAT_MPEG4,
    VFORMAT_H264,
    VFORMAT_MJPEG,
    VFORMAT_REAL,
    VFORMAT_JPEG,
    VFORMAT_VC1,
    VFORMAT_AVS,
    VFORMAT_SW,
    VFORMAT_UNSUPPORT,
    VFORMAT_MAX
} vformat_t;

typedef struct{
	ui32	  pid;
	i32		  nVideoWidth;
	i32		  nVideoHeight;
	i32	      nFrameRate;
	vformat_t vFmt;
	ui32	  cFmt;
}VIDEO_PARA_T, *PVIDEO_PARA_T;

typedef enum {
    FORMAT_UNKNOWN = -1,
    FORMAT_MPEG   = 0,
    FORMAT_PCM_S16LE = 1,
    FORMAT_AAC   = 2,
    FORMAT_AC3   = 3,
    FORMAT_ALAW = 4,
    FORMAT_MULAW = 5,
    FORMAT_DTS = 6,
    FORMAT_PCM_S16BE = 7,
    FORMAT_FLAC = 8,
    FORMAT_COOK = 9,
    FORMAT_PCM_U8 = 10,
    FORMAT_ADPCM = 11,
    FORMAT_AMR  = 12,
    FORMAT_RAAC  = 13,
    FORMAT_WMA  = 14,
    FORMAT_WMAPRO   = 15,
    FORMAT_PCM_BLURAY  = 16,
    FORMAT_ALAC  = 17,
    FORMAT_VORBIS    = 18,
	FORMAT_DDPlus = 19,
    FORMAT_UNSUPPORT ,
    FORMAT_MAX    
} aformat_t;

typedef struct{
	ui32	   pid;
	i32		   nChannels;
	i32		   nSampleRate;
	aformat_t  aFmt;
	i32		   nExtraSize;
	ui8*	   pExtraData;	
}AUDIO_PARA_T, *PAUDIO_PARA_T;

typedef enum
{
    IPTV_PLAYER_EVT_STREAM_VALID=0,
    IPTV_PLAYER_EVT_FIRST_PTS,      //解出第一帧
    IPTV_PLAYER_EVT_VOD_EOS,        //VOD播放完毕
    IPTV_PLAYER_EVT_ABEND,          //为上报下溢事件而增加的类型
    IPTV_PLAYER_EVT_PLAYBACK_ERROR,	// 播放错误
    IPTV_PLAYER_EVT_STREAM_INFO,    // display video/audio stream info
    IPTV_PLAYER_EVT_MAX,
}IPTV_PLAYER_EVT_e;

typedef void (*IPTV_PLAYER_EVT_CB)(IPTV_PLAYER_EVT_e evt, void *handler);

class TsPlayerListener: public MagPlayerListener{
public:
    virtual void notify(int msg, int ext1, int ext2);
};

class CTC_MediaProcessor{
public:
	CTC_MediaProcessor(){}
	virtual ~CTC_MediaProcessor(){}
public:
	//get play mode
	virtual int  GetPlayMode()=0;
	//set video display window
	virtual int  SetVideoWindow(int x,int y,int width,int height)=0;
	//show video
	virtual int  VideoShow(void)=0;
	//hide video
	virtual int  VideoHide(void)=0;
	//initialize video parameters
	virtual void InitVideo(PVIDEO_PARA_T pVideoPara)=0;
	//initialize audio parameters
	virtual void InitAudio(PAUDIO_PARA_T pAudioPara)=0;
	//start play
	virtual bool StartPlay()=0;
	//write out the stream data
	virtual int WriteData(unsigned char* pBuffer, unsigned int nSize)=0;
	virtual bool Pause()=0;
	virtual bool Resume()=0;
	//fast forword/backword
	virtual bool Fast()=0;
	//stop fast forword/backword
	virtual bool StopFast()=0;
	virtual bool Stop()=0;
    //flush mediaplayer buffer
    virtual bool Seek()=0;
	virtual bool SetVolume(int volume)=0;
	virtual int GetVolume()=0;
	//set video display aspect ratio. it can override SetVideoWindow() operation
	virtual bool SetRatio(int nRatio)=0;
	//get current audio balance setting
	virtual int GetAudioBalance()=0;
	//set audio balance
	virtual bool SetAudioBalance(int nAudioBalance)=0;
	//get video display width and height
	virtual void GetVideoPixels(int& width, int& height)=0;

	virtual bool IsSoftFit()=0;
    
	virtual void SetEPGSize(int w, int h)=0;

    virtual void SetSurface(Surface* pSurface) = 0;

	//switch the audio track. pid: the pid of the switching audio stream
	virtual void SwitchAudioTrack(int pid) = 0;
	//switch the subtitle track
	virtual void SwitchSubtitle(int pid) = 0;
    virtual void SetProperty(int nType, int nSub, int nValue) = 0;
	//get current playing time (ms)
	virtual long GetCurrentPlayTime() = 0;
	virtual void leaveChannel() = 0;

    virtual void playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *handler) = 0;    
};



class TsPlayer : public CTC_MediaProcessor, public MagSingleton<TsPlayer>{
    friend class MagSingleton<TsPlayer>;
	TsPlayer();
	virtual ~TsPlayer();
public:
	virtual int  GetPlayMode();
	virtual int  SetVideoWindow(int x,int y,int width,int height);
	virtual int  VideoShow(void);
	virtual int  VideoHide(void);
	virtual void InitVideo(PVIDEO_PARA_T pVideoPara);
	virtual void InitAudio(PAUDIO_PARA_T pAudioPara);
	virtual bool StartPlay();
	virtual int  WriteData(unsigned char* pBuffer, unsigned int nSize);
	virtual bool Pause();
	virtual bool Resume();
	virtual bool Fast();
	virtual bool StopFast();
	virtual bool Stop();
    virtual bool Seek();
	virtual bool SetVolume(int volume);
	virtual int  GetVolume();
	virtual bool SetRatio(int nRatio);
	virtual int  GetAudioBalance();
	virtual bool SetAudioBalance(int nAudioBalance);
	virtual void GetVideoPixels(int& width, int& height);

	virtual bool IsSoftFit();
	virtual void SetEPGSize(int w, int h);

	virtual void SetSurface(Surface* pSurface);

	virtual void SwitchAudioTrack(int pid);
	virtual void SwitchSubtitle(int pid);
	virtual void SetProperty(int nType, int nSub, int nValue);
	virtual long GetCurrentPlayTime();
	virtual void leaveChannel();
	virtual void playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *hander);
    
protected:
	int		m_bLeaveChannel;
    
private:

    enum State_t{
        TSP_IDLE = 0,
        TSP_INITIALIZED,
        TSP_PREPARING,
        TSP_PREPARED,
        TSP_FLUSHING,
        TSP_RUNNING,
        TSP_FASTING,
        TSP_PAUSED,
        TSP_STOPPED,
        TSP_ERROR,
    };

    State_t mState;
    State_t mSeekBackState;
    
    MagPlayerClient_t mPlayer;
    streamBuf_t       mStreamBuf;

    TsPlayerListener  *mpListener;
    
    ui32 convertVideoCodecType(vformat_t vcodec);
    ui32 convertAudioCodecType(aformat_t acodec);

    MagEventGroupHandle mPrepareEvtGroup;
    MagEventHandle      mPrepareDoneEvt;
    MagEventHandle      mPrepareErrorEvt;
    void Prepare();
    static void PrepareCompleteEvtListener(void *priv);
    static void FlushCompleteEvtListener(void *priv);
    static void ErrorEvtListener(void *priv, i32 what, i32 extra);

    void initialize();
};

CTC_MediaProcessor* GetMediaProcessor();
int GetMediaProcessorVersion();


#endif
