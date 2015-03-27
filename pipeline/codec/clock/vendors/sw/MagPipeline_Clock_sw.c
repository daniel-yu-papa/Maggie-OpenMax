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

#include "MagOMX_IL.h"
#include "Omxil_Clock.h"
#include "MagAudioPipeline.h"
#include "MagVideoPipeline.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Pipeline_OMX"

OmxilClock::OmxilClock():
				mhClock(NULL),
                mStartTime(kInvalidTimeStamp){
	Mag_CreateEventGroup(&mStIdleEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mClkStIdleEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStIdleEventGroup, mClkStIdleEvent);
    }

    Mag_CreateEventGroup(&mStLoadedEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mClkStLoadedEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStLoadedEventGroup, mClkStLoadedEvent);
    }

    Mag_CreateEventGroup(&mStExecutingEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mClkStExecutingEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStExecutingEventGroup, mClkStExecutingEvent);
    }

    Mag_CreateEventGroup(&mStPauseEventGroup);
    if (MAG_ErrNone == Mag_CreateEvent(&mClkStPauseEvent, MAG_EVT_PRIO_DEFAULT)){
        Mag_AddEventGroup(mStPauseEventGroup, mClkStPauseEvent);
    }

	mClockCallbacks.EventHandler    = OmxilClock::ClockEventHandler;
    mClockCallbacks.EmptyBufferDone = NULL;
    mClockCallbacks.FillBufferDone  = NULL;
}

OmxilClock::~OmxilClock(){
    Mag_DestroyEvent(&mClkStIdleEvent);
    Mag_DestroyEventGroup(&mStIdleEventGroup);

    Mag_DestroyEvent(&mClkStLoadedEvent);
    Mag_DestroyEventGroup(&mStLoadedEventGroup);

    Mag_DestroyEvent(&mClkStExecutingEvent);
    Mag_DestroyEventGroup(&mStExecutingEventGroup);

    Mag_DestroyEvent(&mClkStPauseEvent);
    Mag_DestroyEventGroup(&mStPauseEventGroup);

    if (mhClock)
	   OMX_FreeHandle(mhClock);
    mhClock = NULL;
}

OMX_ERRORTYPE OmxilClock::ClockEventHandler(
                                OMX_HANDLETYPE hComponent,
                                OMX_PTR pAppData,
                                OMX_EVENTTYPE eEvent,
                                OMX_U32 Data1,
                                OMX_U32 Data2,
                                OMX_PTR pEventData){
	OmxilClock *pClock;

    AGILE_LOGV("Clock event: %d", eEvent);
    pClock = static_cast<OmxilClock *>(pAppData);
    if(eEvent == OMX_EventCmdComplete) {
        if (Data1 == OMX_CommandStateSet) {
            AGILE_LOGD("OMX_CommandStateSet");
            switch ((int)Data2) {
                case OMX_StateMax:
                    AGILE_LOGD("OMX_StateMax\n");
                    break;

                case OMX_StateLoaded:
                    AGILE_LOGD("OMX_StateLoaded\n");
                    Mag_SetEvent(pClock->mClkStLoadedEvent); 
                    break;

                case OMX_StateIdle:
                    AGILE_LOGD("OMX_StateIdle\n");
                    Mag_SetEvent(pClock->mClkStIdleEvent); 
                    break;

                case OMX_StateExecuting:
                    AGILE_LOGD("OMX_StateExecuting\n");
                    Mag_SetEvent(pClock->mClkStExecutingEvent); 
                    break;

                case OMX_StatePause:
                    AGILE_LOGD("OMX_StatePause\n");
                    Mag_SetEvent(pClock->mClkStPauseEvent); 
                    break;

                case OMX_StateWaitForResources:
                    AGILE_LOGD("OMX_StateWaitForResources\n");
                    break;
            }
        }else if (Data1 == OMX_CommandPortEnable){
            AGILE_LOGD("Clock component enables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandPortDisable){
            AGILE_LOGD("Clock component disables port %d is done!", Data2);
        }else if (Data1 == OMX_CommandFlush){
            AGILE_LOGD("Clock component flushes port %d is done!", Data2);
        }
    }else{
        AGILE_LOGD("unsupported event: %d, Data1: %u, Data2: %u\n", eEvent, Data1, Data2);
    }

    return OMX_ErrorNone;
}

_status_t OmxilClock::connectVideoPipeline(void *pVpl){
	i32 vSchPort;
	OMX_HANDLETYPE hvSch;
    MagVideoPipeline *pVideoPipeline;
	_status_t err;
	OMX_CONFIG_UI32TYPE newPortConfig;
	OMX_ERRORTYPE ret;

    pVideoPipeline = static_cast<MagVideoPipeline *>(pVpl);
	if (pVideoPipeline == NULL){
		AGILE_LOGE("Input pVideoPipeline is NULL");
		return MAG_BAD_VALUE;
	}

	err = pVideoPipeline->getClkConnectedComp(&vSchPort, &hvSch);
	if (err == MAG_NO_ERROR){
		initHeader(&newPortConfig, sizeof(OMX_CONFIG_UI32TYPE));
		ret = OMX_GetConfig(mhClock, (OMX_INDEXTYPE)OMX_IndexConfigExtAddPort, &newPortConfig);
		if (ret != OMX_ErrorNone){
			AGILE_LOGE("Failed to add the port dynamically. ret = 0x%x", ret);
			return MAG_INVALID_OPERATION;
		}
		mPortIdxToVSch = newPortConfig.uValue;

		ret = OMX_SetupTunnel(mhClock, mPortIdxToVSch, hvSch, vSchPort);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("To setup up tunnel between Clk[port: %d] and vSch[port: %d] - FAILURE!",
                        mPortIdxToVSch, vSchPort);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To setup up tunnel between Clk[port: %d] and vSch[port: %d] - OK!",
                        mPortIdxToVSch, vSchPort);
        }
	}else{
		AGILE_LOGE("Failed to get vsch component from video pipeline %p", pVideoPipeline);
		return MAG_NAME_NOT_FOUND;
	}

	return MAG_NO_ERROR;
}

_status_t OmxilClock::connectAudioPipeline(void *pApl){
	i32 aRenPort;
    MagAudioPipeline *pAudioPipeline;
	OMX_HANDLETYPE haRen;
	_status_t err;
	OMX_CONFIG_UI32TYPE newPortConfig;
	OMX_ERRORTYPE ret;

    pAudioPipeline = static_cast<MagAudioPipeline *>(pApl);
	if (pAudioPipeline == NULL){
		AGILE_LOGE("Input pAudioPipeline is NULL");
		return MAG_BAD_VALUE;
	}

	err = pAudioPipeline->getClkConnectedComp(&aRenPort, &haRen);
	if (err == MAG_NO_ERROR){
		initHeader(&newPortConfig, sizeof(OMX_CONFIG_UI32TYPE));
		ret = OMX_GetConfig(mhClock, (OMX_INDEXTYPE)OMX_IndexConfigExtAddPort, &newPortConfig);
		if (ret != OMX_ErrorNone){
			AGILE_LOGE("Failed to add the port dynamically. ret = 0x%x", ret);
			return MAG_INVALID_OPERATION;
		}
		mPortIdxToARen = newPortConfig.uValue;

		ret = OMX_SetupTunnel(mhClock, mPortIdxToARen, haRen, aRenPort);
        if(err != OMX_ErrorNone){
            AGILE_LOGE("To setup up tunnel between Clk[port: %d] and aRen[port: %d] - FAILURE!",
                        mPortIdxToARen, aRenPort);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To setup up tunnel between Clk[port: %d] and aRen[port: %d] - OK!",
                        mPortIdxToARen, aRenPort);
        }
	}else{
		AGILE_LOGE("Failed to get vsch component from video pipeline %p", pAudioPipeline);
		return MAG_NAME_NOT_FOUND;
	}

	return MAG_NO_ERROR;
}

_status_t OmxilClock::disconnectVideoPipeline(void *pVpl){
	i32 vSchPort;
    MagVideoPipeline *pVideoPipeline;
	OMX_HANDLETYPE hvSch;
	_status_t err;
	OMX_ERRORTYPE ret;

    pVideoPipeline = static_cast<MagVideoPipeline *>(pVpl);
	if (pVideoPipeline == NULL){
		AGILE_LOGE("Input pVideoPipeline is NULL");
		return MAG_BAD_VALUE;
	}

	err = pVideoPipeline->getClkConnectedComp(&vSchPort, &hvSch);
	if (err == MAG_NO_ERROR){
		ret = OMX_TeardownTunnel(mhClock, mPortIdxToVSch, hvSch, vSchPort);
        if(ret != OMX_ErrorNone){
            AGILE_LOGE("To tear down the tunnel between Clk[port: %d] and vSch[port: %d] - FAILURE!",
                        mPortIdxToVSch, vSchPort);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To tear down the tunnel between Clk[port: %d] and vSch[port: %d] - OK!",
                        mPortIdxToVSch, vSchPort);
        }
	}else{
		AGILE_LOGE("Failed to get vsch component from video pipeline %p", pVideoPipeline);
		return MAG_NAME_NOT_FOUND;
	}

	return MAG_NO_ERROR;
}

_status_t OmxilClock::disconnectAudioPipeline(void *pApl){
	i32 aRenPort;
    MagAudioPipeline *pAudioPipeline;
	OMX_HANDLETYPE haRen;
	_status_t err;
	OMX_ERRORTYPE ret;

    pAudioPipeline = static_cast<MagAudioPipeline *>(pApl);
	if (pAudioPipeline == NULL){
		AGILE_LOGE("Input pAudioPipeline is NULL");
		return MAG_BAD_VALUE;
	}

	err = pAudioPipeline->getClkConnectedComp(&aRenPort, &haRen);
	if (err == MAG_NO_ERROR){
		ret = OMX_TeardownTunnel(mhClock, mPortIdxToARen, haRen, aRenPort);
        if(ret != OMX_ErrorNone){
            AGILE_LOGE("To tear down the tunnel between Clk[port: %d] and aRen[port: %d] - FAILURE!",
                        mPortIdxToARen, aRenPort);
            return MAG_UNKNOWN_ERROR;
        }else{
            AGILE_LOGI("To tear down the tunnel between Clk[port: %d] and aRen[port: %d] - OK!",
                        mPortIdxToARen, aRenPort);
        }
	}else{
		AGILE_LOGE("Failed to get vsch component from video pipeline %p", pAudioPipeline);
		return MAG_NAME_NOT_FOUND;
	}

	return MAG_NO_ERROR;
}

_status_t OmxilClock::init(){
    char       compName[128];
    OMX_ERRORTYPE err;

    err = OMX_ComponentOfRoleEnum(compName, (OMX_STRING)OMX_ROLE_CLOCK_BINARY, 1);
    if (err == OMX_ErrorNone){
        AGILE_LOGV("get the component name[%s] that has the role[%s]",
                    compName, OMX_ROLE_CLOCK_BINARY);
        err = OMX_GetHandle(&mhClock, compName, static_cast<OMX_PTR>(this), &mClockCallbacks);
        if(err != OMX_ErrorNone) {
            AGILE_LOGE("OMX_GetHandle(%s) gets failure", compName);
            return MAG_BAD_VALUE;
        }
    }else{
        AGILE_LOGV("Failed to get the component name with the role[%s]",
                    OMX_ROLE_CLOCK_BINARY);
    }
    return MAG_NO_ERROR;
}

_status_t OmxilClock::setup(){
	OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StateIdle, NULL);

    return MAG_NO_ERROR;
}

_status_t OmxilClock::start(){
    OMX_TIME_CONFIG_CLOCKSTATETYPE clockSt;

	OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    initHeader(&clockSt, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
    clockSt.eState = OMX_TIME_ClockStateWaitingForStartTime;
    clockSt.nOffset = 0;
    OMX_SetConfig(mhClock, OMX_IndexConfigTimeClockState, &clockSt);

	return MAG_NO_ERROR;
}

_status_t OmxilClock::stop(){
    Mag_ClearEvent(mClkStIdleEvent);
	OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StateIdle, NULL);

     Mag_WaitForEventGroup(mStIdleEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
	return MAG_NO_ERROR;
}

_status_t OmxilClock::flush(){
    OMX_SendCommand(mhClock, OMX_CommandFlush, OMX_ALL, NULL);

    return MAG_NO_ERROR;
}

_status_t OmxilClock::pause(){
    Mag_ClearEvent(mClkStPauseEvent);
	OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StatePause, NULL);

    Mag_WaitForEventGroup(mStPauseEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
	return MAG_NO_ERROR;
}

_status_t OmxilClock::resume(){
    Mag_ClearEvent(mClkStExecutingEvent);
	OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StateExecuting, NULL);

    Mag_WaitForEventGroup(mStExecutingEventGroup, MAG_EG_AND, MAG_TIMEOUT_INFINITE);
	return MAG_NO_ERROR;
}

_status_t OmxilClock::reset(){
    Mag_ClearEvent(mClkStIdleEvent);
    OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StateIdle, NULL);

    Mag_WaitForEventGroup(mStIdleEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    AGILE_LOGV("After OMX_StateIdle");

    Mag_ClearEvent(mClkStLoadedEvent);
    OMX_SendCommand(mhClock, OMX_CommandStateSet, OMX_StateLoaded, NULL);

    Mag_WaitForEventGroup(mStLoadedEventGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);

    AGILE_LOGD("exit!");

    return MAG_NO_ERROR;
}

i64       OmxilClock::getPlayingTime(){
    OMX_CONFIG_START_TIME_TYPE startTime;
    i64 pos;

    if (mStartTime == kInvalidTimeStamp){
        initHeader(&startTime, sizeof(OMX_CONFIG_START_TIME_TYPE));
        OMX_GetConfig(mhClock, (OMX_INDEXTYPE)OMX_IndexConfigExtStartTime, &startTime);
        mStartTime = startTime.start_time;
    }

    pos = getMediaTime() - mStartTime;

    return pos;
}

i64       OmxilClock::getMediaTime(){
    OMX_TIME_CONFIG_TIMESTAMPTYPE mediaTime;

    initHeader(&mediaTime, sizeof(OMX_TIME_CONFIG_TIMESTAMPTYPE));
    mediaTime.nPortIndex = mPortIdxToARen;
    OMX_GetConfig(mhClock, OMX_IndexConfigTimeCurrentMediaTime, &mediaTime);

    AGILE_LOGD("media time: %lld", mediaTime.nTimestamp);

    return (i64)(mediaTime.nTimestamp);
}