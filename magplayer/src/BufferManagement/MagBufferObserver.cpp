#include "MagBufferObserver.h"
#include <unistd.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagBufferObserver"

#define DEFAULT_CONFIG_FILE_PATH "/data/etc/mag"

MagBufferObserver::MagBufferObserver(bool isMaster, BufferObseverType_t type):
										mMaster(isMaster),
										mType(type),
										mIsPause(true),
										mIsRunning(false),
										mBufPercentage(-1){

	memset(&mPolicy, 0, sizeof(LoadedBufferPolicy_t));
}

MagBufferObserver::~MagBufferObserver(){
	destroyMagMessage(&mMediaPlayerNotifier);
}

void MagBufferObserver::update(BufferringEvent_t event, i32 parameter){
	// AGILE_LOGI("get event: %s", BufferEvent2String(event));

	if (!mIsRunning){
		AGILE_LOGI("it is stopped, ignore the updates!");
		return;
	}

	switch (event){
		case kEvent_Empty:
		case kEvent_BelowLow:
			if (mMaster){
				if (!mIsPause){
					if (mMediaPlayerNotifier){
						AGILE_LOGI("post pause message!!");
						mMediaPlayerNotifier->setString(mMediaPlayerNotifier, "command", "pause");
					    mMediaPlayerNotifier->postMessage(mMediaPlayerNotifier, 0);
					    mIsPause = true;
					    mBufPercentage = -1;
					}else{
						AGILE_LOGE("mMediaPlayerNotifier is NULL (try to do pause() in kEvent_Empty)");
					}
				}
			}
			break;

		case kEvent_LowToPlay:
			break;

		/*received EOS. player needs to play anyway*/
        case kEvent_NoMoreData:
		case kEvent_Full:
		case kEvent_AboveHigh:
		case kEvent_PlayToHigh:
			if (mMaster){
				if (mIsPause){
					if (mMediaPlayerNotifier){
						AGILE_LOGI("post play message!!");
						mMediaPlayerNotifier->setString(mMediaPlayerNotifier, "command", "play");
					    mMediaPlayerNotifier->postMessage(mMediaPlayerNotifier, 0);
					    mIsPause = false;
					}else{
						AGILE_LOGE("mMediaPlayerNotifier is NULL (try to do play() in kEvent_Empty)");
					}
				}
			}
			break;

		case kEvent_BufferStatus:
			if (mMediaPlayerNotifier){
				/*screen out the unchanged buffer status events*/
				if (mIsPause){
					if ((mBufPercentage == -1) || (mBufPercentage != parameter)){
						AGILE_LOGI("post buffer status[%d] message!!", parameter);
						mMediaPlayerNotifier->setString(mMediaPlayerNotifier, "command", "buffer_status");
						mMediaPlayerNotifier->setUInt32(mMediaPlayerNotifier, "percentage", parameter);
					    mMediaPlayerNotifier->postMessage(mMediaPlayerNotifier, 0);
					    mBufPercentage = parameter;
					}
				}else{
					AGILE_LOGD("Still playing, but buffer level drops to %d%%", parameter);
				}
			}else{
				AGILE_LOGE("mMediaPlayerNotifier is NULL (try to report out the buffer status)");
			}
			break;

		default:
			break;
	}
}

void MagBufferObserver::setMediaPlayerNotifier(MagMessageHandle msg){
	mMediaPlayerNotifier = msg;
}

_status_t MagBufferObserver::start(BufferPolicy_t *pPolicy){
	_status_t ret = MAG_NO_ERROR;

	loadConfigFile();

	if (mType == kBufferObserver_ContentPipe)
		memcpy(pPolicy, &mPolicy.contentPipe, sizeof(BufferPolicy_t));
	else
		memcpy(pPolicy, &mPolicy.demuxerStream, sizeof(BufferPolicy_t));

	mIsRunning = true;

	return ret;
}

void MagBufferObserver::stop(){
	mIsPause       = true;
	// mIsRunning     = false;
	mBufPercentage = -1;
}

void MagBufferObserver::reset(){
	mIsPause       = true;
	mIsRunning     = false;
	mBufPercentage = -1;
}

void MagBufferObserver::setDefaultPolicy(LoadedBufferPolicy_t *policy){
	policy->contentPipe.videoBufferPoolSize = 30;
	policy->contentPipe.kbps = 10000;
	policy->contentPipe.bufferLowThreshold = 2000;
	policy->contentPipe.bufferPlayThreshold = 5000;
	policy->contentPipe.bufferHighThreshold = 25000;

	policy->demuxerStream.videoBufferPoolSize = 30;
	policy->demuxerStream.audioBufferPoolSize = (2 * 1024 * 1024);
	policy->demuxerStream.otherBufferPoolSize = (2 * 1024 * 1024);
	policy->demuxerStream.bufferLowThreshold = 2000;
	policy->demuxerStream.bufferPlayThreshold = 5000;
	policy->demuxerStream.bufferHighThreshold = 20000;
	// policy->demuxerStream.bufFlushPlayThreshold = 2000;
	policy->demuxerStream.kbps = 10000;
}

_status_t MagBufferObserver::loadConfigFile(){
	char configFileName[256];
    _status_t res = MAG_NO_ERROR;

	sprintf(configFileName, "%s/bufferMgr.xml", 
             getenv("BUFFER_MGR_PATH") ? getenv("BUFFER_MGR_PATH") : DEFAULT_CONFIG_FILE_PATH);

	if(access(configFileName, R_OK)){
        AGILE_LOGE("Can NOT access config file under %s\n. Using default setting!", configFileName);
        setDefaultPolicy(&mPolicy);
    }else{
    	mXMLParsedDoc.LoadFile(configFileName);
    	res = parseXMLConfig();

    	if(XML_SUCCESS != mXMLParsedDoc.ErrorID()){
            printf("failed to load xml file [%s]. error = %d\n", configFileName, mXMLParsedDoc.ErrorID());
            setDefaultPolicy(&mPolicy);
        }
    }
    
    return res;
}

_status_t MagBufferObserver::parseChildElement(XMLElement* element, bool isCP){
	i32 value;
    XMLError res;
    XMLElement* ele;
    BufferPolicy_t *pPolicy;
    bool defineFullBuffer = false;

    if (isCP){
    	defineFullBuffer = true;
    	pPolicy = &mPolicy.contentPipe;
    }else{
    	pPolicy = &mPolicy.demuxerStream;
    }

    if (defineFullBuffer){
		ele = element->FirstChildElement( "buffer_size" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pPolicy->videoBufferPoolSize = value;
			}else{
				AGILE_LOGE("failed to get the value of the element: buffer_size");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: buffer_size");
			return MAG_NAME_NOT_FOUND;
		}
    }else{
    	ele = element->FirstChildElement( "video_buffer_size" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pPolicy->videoBufferPoolSize = value;
			}else{
				AGILE_LOGE("failed to get the value of the element: video_buffer_size");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: video_buffer_size");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "audio_buffer_size" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pPolicy->audioBufferPoolSize = (value * 1024);
			}else{
				AGILE_LOGE("failed to get the value of the element: audio_buffer_size");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: audio_buffer_size");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "other_buffer_size" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pPolicy->otherBufferPoolSize = (value * 1024);
			}else{
				AGILE_LOGE("failed to get the value of the element: other_buffer_size");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: other_buffer_size");
			return MAG_NAME_NOT_FOUND;
		}
    }

	ele = element->FirstChildElement( "kbps" );
	if (ele){
		res = ele->QueryIntText( &value );
		if (XML_SUCCESS == res){
			pPolicy->kbps = value;
		}else{
			AGILE_LOGE("failed to get the value of the element: kbps");
			return MAG_BAD_VALUE;;
		}
	}else{
		AGILE_LOGE("failed to get the element: kpbs");
		return MAG_NAME_NOT_FOUND;
	}

	ele = element->FirstChildElement( "low_level" );
	if (ele){
		res = ele->QueryIntText( &value );
		if (XML_SUCCESS == res){
			pPolicy->bufferLowThreshold = value;
		}else{
			AGILE_LOGE("failed to get the value of the element: low_level");
			return MAG_BAD_VALUE;;
		}
	}else{
		AGILE_LOGE("failed to get the element: low_level");
		return MAG_NAME_NOT_FOUND;
	}

	ele = element->FirstChildElement( "play_level" );
	if (ele){
		res = ele->QueryIntText( &value );
		if (XML_SUCCESS == res){
			pPolicy->bufferPlayThreshold = value;
		}else{
			AGILE_LOGE("failed to get the value of the element: play_level");
			return MAG_BAD_VALUE;;
		}
	}else{
		AGILE_LOGE("failed to get the element: play_level");
		return MAG_NAME_NOT_FOUND;
	}

	ele = element->FirstChildElement( "high_level" );
	if (ele){
		res = ele->QueryIntText( &value );
		if (XML_SUCCESS == res){
			pPolicy->bufferHighThreshold = value;
		}else{
			AGILE_LOGE("failed to get the value of the element: high_level");
			return MAG_BAD_VALUE;;
		}
	}else{
		AGILE_LOGE("failed to get the element: high_level");
		return MAG_NAME_NOT_FOUND;
	}

	return MAG_NO_ERROR;
}

_status_t MagBufferObserver::parseXMLConfig(){
    _status_t res = MAG_NO_ERROR;
    
    XMLElement* ele = mXMLParsedDoc.FirstChildElement()->FirstChildElement( "content_pipe" );

    if(XML_SUCCESS == mXMLParsedDoc.ErrorID())
        res = parseChildElement(ele, true);
    else{
    	AGILE_LOGE("failed to parse the element: content_pipe. err=%d", mXMLParsedDoc.ErrorID());
        return MAG_NAME_NOT_FOUND;
    }
    
    if (res != MAG_NO_ERROR){
        return res;
    }

    ele = mXMLParsedDoc.FirstChildElement()->FirstChildElement( "demuxer_stream" );

    if(XML_SUCCESS == mXMLParsedDoc.ErrorID())
        res = parseChildElement(ele, false);
    else{
    	AGILE_LOGE("failed to parse the element: demuxer_stream. err=%d", mXMLParsedDoc.ErrorID());
        return MAG_NAME_NOT_FOUND;
    }

    AGILE_LOGV("ContentPipe: bufSize[%d], low[%d ms], play[%d ms], high[%d ms]",
    	        mPolicy.contentPipe.videoBufferPoolSize,
    	        mPolicy.contentPipe.bufferLowThreshold,
    	        mPolicy.contentPipe.bufferPlayThreshold,
    	        mPolicy.contentPipe.bufferHighThreshold);

    AGILE_LOGV("Demuxer: VbufSize[%d ms][%d kbps], AbufSize[%d kB], ObufSize[%d kB], low[%d ms], play[%d ms], high[%d ms]",
    	        mPolicy.demuxerStream.videoBufferPoolSize,
    	        mPolicy.demuxerStream.kbps,
    	        mPolicy.demuxerStream.audioBufferPoolSize,
    	        mPolicy.demuxerStream.otherBufferPoolSize,
    	        mPolicy.demuxerStream.bufferLowThreshold,
    	        mPolicy.demuxerStream.bufferPlayThreshold,
    	        mPolicy.demuxerStream.bufferHighThreshold);
    return res;
}