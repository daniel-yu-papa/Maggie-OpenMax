/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MagBufferObserver.h"
#include <unistd.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_BuffMgr"

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

_status_t MagBufferObserver::start(void *pPolicy){
	_status_t ret = MAG_NO_ERROR;
	Demuxer_BufferPolicy_t *pDemuxerPolicy;
	ContentPipe_BufferPolicy_t *pCPPolicy;

	loadConfigFile();

	if (mType == kBufferObserver_ContentPipe){
		pCPPolicy = static_cast<ContentPipe_BufferPolicy_t *>(pPolicy);
		memcpy(pCPPolicy, &mPolicy.contentPipe, sizeof(ContentPipe_BufferPolicy_t));
	}else{
		pDemuxerPolicy = static_cast<Demuxer_BufferPolicy_t *>(pPolicy);
		memcpy(pDemuxerPolicy, &mPolicy.demuxerStream, sizeof(Demuxer_BufferPolicy_t));
	}

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

void MagBufferObserver::flush(){
	mIsPause       = false;
	mBufPercentage = -1;
}

void MagBufferObserver::setDefaultPolicy(LoadedBufferPolicy_t *policy){
	policy->contentPipe.BufferSize = 30;
	policy->contentPipe.kbps = 10000;
	policy->contentPipe.bufferLowThreshold = 2000;
	policy->contentPipe.bufferPlayThreshold = 5000;
	policy->contentPipe.bufferHighThreshold = 25000;

	policy->demuxerStream.videoBufferPoolSize = 30;
	policy->demuxerStream.audioBufferPoolSize = (2 * 1024 * 1024);
	policy->demuxerStream.otherBufferPoolSize = (2 * 1024 * 1024);
	policy->demuxerStream.normalBitRate.bufferLowThreshold = 2000;
	policy->demuxerStream.normalBitRate.bufferPlayThreshold = 5000;
	policy->demuxerStream.normalBitRate.bufferHighThreshold = 20000;
	policy->demuxerStream.highBitRate.bufferLowThreshold = 2000;
	policy->demuxerStream.highBitRate.bufferPlayThreshold = 5000;
	policy->demuxerStream.highBitRate.bufferHighThreshold = 20000;
	policy->demuxerStream.highestBitRate.bufferLowThreshold = 2000;
	policy->demuxerStream.highestBitRate.bufferPlayThreshold = 5000;
	policy->demuxerStream.highestBitRate.bufferHighThreshold = 20000;
	// policy->demuxerStream.bufFlushPlayThreshold = 2000;
	policy->demuxerStream.kbps = 10000;
	policy->demuxerStream.memPoolSizeLimit = 0;
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
_status_t MagBufferObserver::parseBufThresholdElement(XMLElement* element, PlayingBufThreshold_t *pObj){
	i32 value;
    XMLError res;
    XMLElement* ele;

	ele = element->FirstChildElement( "low_level" );
	if (ele){
		res = ele->QueryIntText( &value );
		if (XML_SUCCESS == res){
			pObj->bufferLowThreshold = value;
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
			pObj->bufferPlayThreshold = value;
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
			pObj->bufferHighThreshold = value;
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

_status_t MagBufferObserver::parseChildElement(XMLElement* element, bool isCP){
	i32 value;
    XMLError res;
    _status_t err;
    XMLElement* ele;
    Demuxer_BufferPolicy_t *pDemuxerPolicy;
    ContentPipe_BufferPolicy_t *pCPipePolicy;

    if (isCP){
    	pCPipePolicy = &mPolicy.contentPipe;
    	ele = element->FirstChildElement( "kbps" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pCPipePolicy->kbps = value;
			}else{
				AGILE_LOGE("failed to get the value of the element: kbps");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: kpbs");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "buffer_size" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pCPipePolicy->BufferSize = value;
			}else{
				AGILE_LOGE("failed to get the value of the element: buffer_size");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: buffer_size");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "low_level" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pCPipePolicy->bufferLowThreshold = value;
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
				pCPipePolicy->bufferPlayThreshold = value;
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
				pCPipePolicy->bufferHighThreshold = value;
			}else{
				AGILE_LOGE("failed to get the value of the element: high_level");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: high_level");
			return MAG_NAME_NOT_FOUND;
		}
    }else{
    	pDemuxerPolicy = &mPolicy.demuxerStream;
    	ele = element->FirstChildElement( "kbps" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pDemuxerPolicy->kbps = value;
			}else{
				AGILE_LOGE("failed to get the value of the element: kbps");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: kpbs");
			return MAG_NAME_NOT_FOUND;
		}

    	ele = element->FirstChildElement( "video_buffer_size" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pDemuxerPolicy->videoBufferPoolSize = value;
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
				pDemuxerPolicy->audioBufferPoolSize = (value * 1024);
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
				pDemuxerPolicy->otherBufferPoolSize = (value * 1024);
			}else{
				AGILE_LOGE("failed to get the value of the element: other_buffer_size");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: other_buffer_size");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "mem_pool_limit" );
		if (ele){
			res = ele->QueryIntText( &value );
			if (XML_SUCCESS == res){
				pDemuxerPolicy->memPoolSizeLimit = (value * 1024);
			}else{
				AGILE_LOGE("failed to get the value of the element: mem_pool_limit");
				return MAG_BAD_VALUE;;
			}
		}else{
			AGILE_LOGE("failed to get the element: mem_pool_limit");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "normal_bitrate_stream" );
		if (ele){
			err = parseBufThresholdElement(ele, &pDemuxerPolicy->normalBitRate);
			if (err != MAG_NO_ERROR){
				return MAG_NAME_NOT_FOUND;
			}
		}else{
			AGILE_LOGE("failed to get the element: normal_bitrate_stream");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "high_bitrate_stream" );
		if (ele){
			err = parseBufThresholdElement(ele, &pDemuxerPolicy->highBitRate);
			if (err != MAG_NO_ERROR){
				return MAG_NAME_NOT_FOUND;
			}
		}else{
			AGILE_LOGE("failed to get the element: high_bitrate_stream");
			return MAG_NAME_NOT_FOUND;
		}

		ele = element->FirstChildElement( "highest_bitrate_stream" );
		if (ele){
			err = parseBufThresholdElement(ele, &pDemuxerPolicy->highestBitRate);
			if (err != MAG_NO_ERROR){
				return MAG_NAME_NOT_FOUND;
			}
		}else{
			AGILE_LOGE("failed to get the element: highest_bitrate_stream");
			return MAG_NAME_NOT_FOUND;
		}
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
    	        mPolicy.contentPipe.BufferSize,
    	        mPolicy.contentPipe.bufferLowThreshold,
    	        mPolicy.contentPipe.bufferPlayThreshold,
    	        mPolicy.contentPipe.bufferHighThreshold);

    AGILE_LOGV("Demuxer: VbufSize[%d ms][%d kbps], AbufSize[%d kB], ObufSize[%d kB], MemPoolSize[%d kB]",
    	        mPolicy.demuxerStream.videoBufferPoolSize,
    	        mPolicy.demuxerStream.kbps,
    	        mPolicy.demuxerStream.audioBufferPoolSize,
    	        mPolicy.demuxerStream.otherBufferPoolSize,
    	        mPolicy.demuxerStream.memPoolSizeLimit);

    AGILE_LOGV("Demuxer: NormalBitRate Stream - low[%d ms], play[%d ms], high[%d ms]",
    	        mPolicy.demuxerStream.normalBitRate.bufferLowThreshold,
    	        mPolicy.demuxerStream.normalBitRate.bufferPlayThreshold,
    	        mPolicy.demuxerStream.normalBitRate.bufferHighThreshold);

    AGILE_LOGV("Demuxer: HighBitRate Stream - low[%d ms], play[%d ms], high[%d ms]",
    	        mPolicy.demuxerStream.highBitRate.bufferLowThreshold,
    	        mPolicy.demuxerStream.highBitRate.bufferPlayThreshold,
    	        mPolicy.demuxerStream.highBitRate.bufferHighThreshold);

    AGILE_LOGV("Demuxer: HighestBitRate Stream - low[%d ms], play[%d ms], high[%d ms]",
    	        mPolicy.demuxerStream.highestBitRate.bufferLowThreshold,
    	        mPolicy.demuxerStream.highestBitRate.bufferPlayThreshold,
    	        mPolicy.demuxerStream.highestBitRate.bufferHighThreshold);
    return res;
}