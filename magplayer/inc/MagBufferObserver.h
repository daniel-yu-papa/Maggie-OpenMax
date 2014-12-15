#ifndef __MAGPLAYER_BUFFER_OBSERVER_H__
#define __MAGPLAYER_BUFFER_OBSERVER_H__

#include "framework/MagFramework.h"
#include "MagPlayerCommon.h"

typedef struct{
	/*in ms unit (do player->pause())*/
	ui32 bufferLowThreshold;  
	/*in ms unit (stop reading data)*/
	ui32 bufferHighThreshold; 
	/*in ms unit 
	 *(pause() <UP TO> playthreshold: start play
	 *stop reading data <DOWN TO> playthreshold: start reading data
	 */
	ui32 bufferPlayThreshold; 
}PlayingBufThreshold_t;

typedef struct{
	/*used for full buffer size for content pipe*/
	ui32 videoBufferPoolSize;
	ui32 audioBufferPoolSize;
	ui32 otherBufferPoolSize;
	ui32 kbps;
	ui32 memPoolSizeLimit;      /*in KB*/

	PlayingBufThreshold_t normalBitRate;
	PlayingBufThreshold_t highBitRate;
	PlayingBufThreshold_t highestBitRate;
    /*
    * in ms unit
    * the buffer time for playing after the flushing is done. 
    */
	// ui32 bufFlushPlayThreshold;
}Demuxer_BufferPolicy_t;

typedef struct{
	/*used for full buffer size for content pipe*/
	ui32 BufferSize;
	ui32 kbps;

	/*in ms unit (do player->pause())*/
	ui32 bufferLowThreshold;  
	/*in ms unit (stop reading data)*/
	ui32 bufferHighThreshold; 
	/*in ms unit 
	 *(pause() <UP TO> playthreshold: start play
	 *stop reading data <DOWN TO> playthreshold: start reading data
	 */
	ui32 bufferPlayThreshold; 
}ContentPipe_BufferPolicy_t;

typedef enum{
	kBufferObserver_ContentPipe,
	kBufferObserver_DemuxerStream,
}BufferObseverType_t;

typedef enum{
	kEmpty = 0,
	kBelowLow,
	kBetweenLow_Play,
	kBetweenPlay_High,
	kAboveHigh,
	kFull,
	kInvalid
}BufferStatus_t;

typedef enum{
	kEvent_Empty = 0,
	kEvent_BelowLow,
	kEvent_LowToPlay,
	kEvent_PlayToHigh,
	kEvent_AboveHigh,
	kEvent_Full,
	kEvent_NoMoreData,
	kEvent_BufferStatus
}BufferringEvent_t;

typedef struct{
	ContentPipe_BufferPolicy_t contentPipe;
	Demuxer_BufferPolicy_t     demuxerStream;
}LoadedBufferPolicy_t;

static inline const char *BufferStatus2String(BufferStatus_t bufST) {
    switch (bufST) {
        STRINGIFY(kEmpty);
        STRINGIFY(kBelowLow);
        STRINGIFY(kBetweenLow_Play);
        STRINGIFY(kBetweenPlay_High);
        STRINGIFY(kAboveHigh);
        STRINGIFY(kFull);
        default: return "buffer status - unknown";
    }
}

static inline const char *BufferEvent2String(BufferringEvent_t bufEvt) {
    switch (bufEvt) {
        STRINGIFY(kEvent_Empty);
        STRINGIFY(kEvent_BelowLow);
        STRINGIFY(kEvent_LowToPlay);
        STRINGIFY(kEvent_PlayToHigh);
        STRINGIFY(kEvent_AboveHigh);
        STRINGIFY(kEvent_Full);
        default: return "buffer event - unknown";
    }
}

using namespace tinyxml2;

class MagBufferObserver{
public:
	MagBufferObserver(bool isMaster, BufferObseverType_t type);
	virtual ~MagBufferObserver();

	void update(BufferringEvent_t event, i32 parameter = 0);
    _status_t start(void *pPolicy);
    void stop();
    void reset();
    void setMediaPlayerNotifier(MagMessageHandle msg);

private:
		/*true: working in master mode, could command Player to do play()/pause()/resume()
     *false: working in slave mode. could NOT command player to do anything.
	 */
	bool mMaster;
    BufferObseverType_t mType;

	_status_t parseXMLConfig();
	_status_t parseChildElement(XMLElement* element, bool isCP);
	_status_t parseBufThresholdElement(XMLElement* element, PlayingBufThreshold_t *pObj);
	_status_t loadConfigFile();
    void setDefaultPolicy(LoadedBufferPolicy_t *policy);

	LoadedBufferPolicy_t mPolicy;
	XMLDocument mXMLParsedDoc;

	MagMessageHandle mMediaPlayerNotifier;

	bool mIsPause;
	bool mIsRunning;

	i32 mBufPercentage;
};

#endif