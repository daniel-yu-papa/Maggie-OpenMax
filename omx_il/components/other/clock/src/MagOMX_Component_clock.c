#include "MagOMX_Component_clock.h"
#include "MagOMX_Port_clock.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "MagOMX_CompClk"

#define CMD_LOOPER_NAME        "CompClockCMDLooper"
#define TR_LOOPER_NAME         "CompClockTRLooper%d"

AllocateClass(MagOmxComponentClock, MagOmxComponentImpl);

static void onCmdMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent      root;
    MagOmxComponentImpl  base;
    MagOmxComponentClock thiz;
    OMX_ERRORTYPE ret;
    OMX_U32       i;
    RBTreeNodeHandle n;          
    MagOmxPort       port;
    MagOmxPortClock  portClock;
    OMX_U32 cmd;

    root = ooc_cast(priv, MagOmxComponent);
    base = ooc_cast(priv, MagOmxComponentImpl);
    thiz = ooc_cast(priv, MagOmxComponentClock);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentClock_CmdStartTimeMsg:
        {
            i64 timeStamp;
            i32 portIndex;
            OMX_TIME_CONFIG_RENDERINGDELAYTYPE renderDelay;

            if (!msg->findInt64(msg, "time_stamp", &timeStamp)){
                COMP_LOGE(root, "failed to find the time_stamp!");
                return;
            }

            if (!msg->findInt32(msg, "port_index", &portIndex)){
                COMP_LOGE(root, "failed to find the port_index!");
                return;
            }

            Mag_AcquireMutex(thiz->mhMutex);
            if (thiz->mState.eState == OMX_TIME_ClockStateStopped){
                Mag_ClearEvent(thiz->mStateChangeEvt);
                Mag_ReleaseMutex(thiz->mhMutex);

                COMP_LOGD(root, "Get the start time but wait for the clock state transition to WaitingForStartTime!");
                Mag_WaitForEventGroup(thiz->mStateChangeEvtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
            }else{
                Mag_ReleaseMutex(thiz->mhMutex);
                if (thiz->mState.eState != OMX_TIME_ClockStateWaitingForStartTime){
                    COMP_LOGE(root, "invalid clock state[%s] for getting start time!", 
                               ClockCompState2String(thiz->mState.eState));
                    return;
                }
            } 

            if ( (thiz->mState.nWaitMask & (1 << portIndex)) ){
                thiz->mState.nWaitMask &= ~(1 << portIndex);
                thiz->mStartTimeTable[portIndex] = timeStamp;
                if (thiz->mState.nWaitMask == 0){
                    COMP_LOGD(root, "All streams have set the start time!");
                    thiz->mState.nWaitMask = thiz->mWaitStartTimeMask;
                    if (MagOmxComponentClockVirtual(thiz)->MagOMX_Clock_DecideStartTime){
                        ret = MagOmxComponentClockVirtual(thiz)->MagOMX_Clock_DecideStartTime(thiz, thiz->mStartTimeTable, thiz->mWaitStartTimeMask, &thiz->mState.nStartTime);
                        COMP_LOGD(root, "Decided start time: 0x%llx!", thiz->mState.nStartTime);
                        if (ret == OMX_ErrorNone){
                            initHeader(&renderDelay, sizeof(OMX_TIME_CONFIG_RENDERINGDELAYTYPE));
                            for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
                                port = ooc_cast(n->value, MagOmxPort);
                                ret = MagOmxPortVirtual(port)->GetParameter(port, OMX_IndexConfigTimeRenderingDelay, &renderDelay);
                                if (ret == OMX_ErrorNone){
                                    COMP_LOGD(root, "get rendering delay: %d", renderDelay.nRenderingDelay);
                                    if (thiz->mMaxRenderDelay < renderDelay.nRenderingDelay){
                                        thiz->mMaxRenderDelay = renderDelay.nRenderingDelay;
                                    }
                                }else{
                                    COMP_LOGE(root, "Failed to get render delay of port %d, set it as default 0", port->getPortIndex(port));
                                    thiz->mMaxRenderDelay = 0;
                                }
                            }
                            COMP_LOGD(root, "Max rendering delay: %d us", thiz->mMaxRenderDelay);

                            /*To initialize the Rbase and Wbase while setting state to running*/
                            if (thiz->mState.nOffset < 0)
                                thiz->mClockOffset   = MagOMX_MIN(thiz->mState.nOffset, (0 - thiz->mMaxRenderDelay));
                            else
                                thiz->mClockOffset   = thiz->mState.nOffset - thiz->mMaxRenderDelay;

                            thiz->mReferenceTimeBase = CONVERT_TO_MICROSECONDS(thiz->mState.nStartTime);
                            thiz->mWallTimeBase      = thiz->getTimeNow(thiz) - thiz->mClockOffset;
                            thiz->mState.eState      = OMX_TIME_ClockStateRunning;

                            COMP_LOGD(root, "mReferenceTimeBase: 0x%llx, mWallTimeBase: 0x%llx, mClockOffset: %lld", 
                                             thiz->mReferenceTimeBase,
                                             thiz->mWallTimeBase, thiz->mClockOffset);
                            /*notify connected components the running state*/
                            for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
                                portClock = ooc_cast(n->value, MagOmxPortClock);
                                port = ooc_cast(portClock, MagOmxPort);
                                ret = thiz->updateClockState(port, OMX_TIME_ClockStateRunning);
                                if (ret != OMX_ErrorNone){
                                    COMP_LOGE(root, "Failed to do port(%d)->updateClockState()", port->getPortIndex(port));
                                }
                            }
                        }
                    }else{
                        thiz->mState.nStartTime = -1;
                        COMP_LOGE(root, "pure virtual function MagOMX_DecideStartTime() should be overrided");
                    }
                }else{
                    COMP_LOGD(root, "Port %d sets the start time 0x%llx!", portIndex, timeStamp);
                }
            }else{
                COMP_LOGE(root, "port index: %d doesn't match the wait mask: 0x%x", portIndex, thiz->mState.nWaitMask);
            }
            break;
        }

        default:
            COMP_LOGE(root, "wrong message %d received!");
            break;
    }
}

static void onTimeRequestMessageReceived(const MagMessageHandle msg, OMX_PTR priv){
    MagOmxComponent      root;
    MagOmxComponentImpl  base;
    MagOmxComponentClock thiz;
    OMX_U32 cmd;

    root = ooc_cast(priv, MagOmxComponent);
    base = ooc_cast(priv, MagOmxComponentImpl);
    thiz = ooc_cast(priv, MagOmxComponentClock);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentClock_CmdMTimeRequestMsg:
        {
            i64 timeStamp;
            i32 portIndex;
            OMX_PTR pClientPriv;

            if (!msg->findInt64(msg, "time_stamp", &timeStamp)){
                COMP_LOGE(root, "failed to find the time_stamp!");
                return;
            }

            if (!msg->findInt32(msg, "port_index", &portIndex)){
                COMP_LOGE(root, "failed to find the port_index!");
                return;
            }

            if (!msg->findPointer(msg, "client_private", &pClientPriv)){
                COMP_LOGE(root, "failed to find the client_private!");
                return;
            }

            if ( !( base->mTransitionState == OMX_TransitionStateToIdle ||
                    base->mState == OMX_StateIdle ) && 
                    (thiz->mState.eState == OMX_TIME_ClockStateRunning) ){
                thiz->sendAVSyncAction(thiz, portIndex, timeStamp, AVSYNC_PLAY);
            }else{
                COMP_LOGD(root, "The clock port[%d] state is %s, directly drop the packets", 
                                portIndex,
                                base->mTransitionState == OMX_TransitionStateToIdle ? "in transitToIdle" : base->mState == OMX_StateIdle ? "in Idle" : "not in OMX_TIME_ClockStateRunning");
                thiz->sendAVSyncAction(thiz, portIndex, timeStamp, AVSYNC_DROP);
            }
        }
            break;

        default:
            COMP_LOGE(root, "wrong message %d received!");
            break;
    }
}

static MagOMX_Component_Type_t virtual_MagOmxComponentClock_getType(
									OMX_IN  OMX_HANDLETYPE hComponent){
	return MagOMX_Component_Other;
}

static OMX_ERRORTYPE virtual_MagOmxComponentClock_GetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nParamIndex,  
					                OMX_INOUT OMX_PTR pComponentParameterStructure){
	MagOmxComponentClock thiz;
    MagOmxComponent      root;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentClock);
    root = ooc_cast(hComponent, MagOmxComponent);

    Mag_AcquireMutex(thiz->mhMutex);

    switch (nParamIndex){
    	case OMX_IndexConfigTimeScale:
        {
	    	OMX_TIME_CONFIG_SCALETYPE *output = (OMX_TIME_CONFIG_SCALETYPE *)pComponentParameterStructure;
	    	initHeader(output, sizeof(OMX_TIME_CONFIG_SCALETYPE));
	    	output->xScale = thiz->mxScale;
        }
    		break;

    	case OMX_IndexConfigTimeClockState:
        {
	    	OMX_TIME_CONFIG_CLOCKSTATETYPE *output = (OMX_TIME_CONFIG_CLOCKSTATETYPE *)pComponentParameterStructure;
	    	memcpy(output, &thiz->mState, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
        }
    		break;

    	case OMX_IndexConfigTimeActiveRefClockUpdate:
        {
	    	OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *output = (OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *)pComponentParameterStructure;
	    	memcpy(output, &thiz->mRefClockUpdate, sizeof(OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE));
        }
    		break;

        case OMX_IndexConfigTimeCurrentMediaTime:
        {
            /*it is port configuration but the port id is OMX_ALL, so directly goes to component parameter setting*/
            OMX_TIME_CONFIG_TIMESTAMPTYPE *output = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pComponentParameterStructure;
            output->nTimestamp = thiz->getMediaTimeNow(thiz);
        }
            break;

    	default:
    		break;
    }

    Mag_ReleaseMutex(thiz->mhMutex);

	return OMX_ErrorNone;
}
                
static OMX_ERRORTYPE virtual_MagOmxComponentClock_SetParameter(
					                OMX_IN  OMX_HANDLETYPE hComponent, 
					                OMX_IN  OMX_INDEXTYPE nIndex,
					                OMX_IN  OMX_PTR pComponentParameterStructure){
	MagOmxComponentClock thiz;
    MagOmxComponent      root;
    OMX_ERRORTYPE        ret = OMX_ErrorNone;

	if ((NULL == hComponent) || (NULL == pComponentParameterStructure)){
        return OMX_ErrorBadParameter;
    }

    thiz = ooc_cast(hComponent, MagOmxComponentClock);
    root = ooc_cast(hComponent, MagOmxComponent);

    Mag_AcquireMutex(thiz->mhMutex);

    switch (nIndex){
    	case OMX_IndexConfigTimeScale:
        {
	    	OMX_TIME_CONFIG_SCALETYPE *input = (OMX_TIME_CONFIG_SCALETYPE *)pComponentParameterStructure;
	    	thiz->mxScale = input->xScale;
        }
    		break;

    	case OMX_IndexConfigTimeClockState:
        {
            OMX_TIME_CONFIG_CLOCKSTATETYPE *input = (OMX_TIME_CONFIG_CLOCKSTATETYPE *)pComponentParameterStructure;
            if ((input->eState == OMX_TIME_ClockStateWaitingForStartTime) &&
                (thiz->mState.eState == OMX_TIME_ClockStateRunning)){
                COMP_LOGE(root, "Invalid state transition from ClockStateWaitingForStartTime to ClockStateRunning");
                ret = OMX_ErrorIncorrectStateTransition;
            }else{
                if (input->eState != thiz->mState.eState){
                    COMP_LOGD(root, "State transition from %s to %s", 
                                ClockCompState2String(thiz->mState.eState),
                                ClockCompState2String(input->eState));
                    thiz->mState.eState = input->eState;
                    thiz->mState.nOffset = input->nOffset;

                    Mag_SetEvent(thiz->mStateChangeEvt);
                }else{
                    COMP_LOGD(root, "Same State transition %s. Nothing to be done!", 
                                     ClockCompState2String(thiz->mState.eState));
                }
            }
        }
    		break;

    	default:
    		break;
    }

    Mag_ReleaseMutex(thiz->mhMutex);

	return ret;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentClock_DoAddPortAction(
                                        OMX_IN  OMX_HANDLETYPE hComponent, 
                                        OMX_IN  OMX_U32 portIndex, 
                                        OMX_IN  OMX_HANDLETYPE hPort){

    MagOmxComponentClock hCompClock;

    hCompClock = ooc_cast(hComponent, MagOmxComponentClock);

    hCompClock->mState.nWaitMask |= 1 << portIndex; 
    hCompClock->mWaitStartTimeMask = hCompClock->mState.nWaitMask;
}
    
/*handle the notification from attached port*/
static OMX_ERRORTYPE  virtual_MagOmxComponentClock_Notify(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN MagOMX_Component_Notify_Type_t notifyIndex,
                                        OMX_IN OMX_PTR pNotifyData){
    MagOmxComponentClock hCompClock;
    MagOmxComponent      rootComp;
    MagOmxComponentImpl  compImpl;

    rootComp   = ooc_cast(hComponent, MagOmxComponent);
    compImpl   = ooc_cast(hComponent, MagOmxComponentImpl);
    hCompClock = ooc_cast(hComponent, MagOmxComponentClock);
    switch (notifyIndex){
        case MagOMX_Component_Notify_StartTime:
        {
            OMX_TIME_CONFIG_TIMESTAMPTYPE *tst;
            tst = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pNotifyData;
            if (hCompClock->mCmdStartTimeMsg == NULL){
                hCompClock->mCmdStartTimeMsg = hCompClock->createCmdMessage(hComponent, MagOmxComponentClock_CmdStartTimeMsg);  
            }
            hCompClock->mCmdStartTimeMsg->setInt64(hCompClock->mCmdStartTimeMsg, "time_stamp", tst->nTimestamp);
            hCompClock->mCmdStartTimeMsg->setInt32(hCompClock->mCmdStartTimeMsg, "port_index", tst->nPortIndex);
            hCompClock->mCmdStartTimeMsg->postMessage(hCompClock->mCmdStartTimeMsg, 0);
            COMP_LOGD(rootComp, "post CmdStartTimeMsg(timestamp: 0x%llx, port index: %d)", tst->nTimestamp, tst->nPortIndex);
        }
            break;

        case MagOMX_Component_Notify_MediaTimeRequest:
        {   
            OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE *mtrt;
            OMX_TICKS t_now;
            OMX_TICKS t_request;
            i64 delay; /*in us*/

            mtrt = (OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE *)pNotifyData;
            
            if (hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex] == NULL){
                hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex] = hCompClock->createTimeRequestMessage(hComponent, 
                                                                                                         MagOmxComponentClock_CmdMTimeRequestMsg,
                                                                                                         mtrt->nPortIndex); 
            }
            hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->setInt64(hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex], "time_stamp", mtrt->nMediaTimestamp);
            hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->setInt32(hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex], "port_index", mtrt->nPortIndex);
            hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->setPointer(hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex], "client_private", mtrt->pClientPrivate, MAG_FALSE);

            if ( !( compImpl->mTransitionState == OMX_TransitionStateToIdle ||
                    compImpl->mState == OMX_StateIdle ) && 
                    (hCompClock->mState.eState == OMX_TIME_ClockStateRunning) ){
                if (mtrt->nMediaTimestamp != kInvalidTimeStamp){
                    t_now     = hCompClock->getMediaTimeNow(hCompClock);
                    t_request = hCompClock->getMediaTimeRequest(hCompClock, CONVERT_TO_MICROSECONDS(mtrt->nMediaTimestamp), mtrt->nOffset);

                    delay = t_request - t_now;
                }else{
                    delay = MAG_MAX_INTEGER;
                }
                
                COMP_LOGD(rootComp, "To add timestamp(tr:0x%llx - tn:0x%llx) with delay %lld to port[%d]", 
                                     t_request, t_now,
                                     delay, mtrt->nPortIndex);
                if (delay >= 0){
                    hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->postMessage(hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex], delay);
                }else{
                    /*only support the forward playback for now, so drop the stream packet*/
                    hCompClock->sendAVSyncAction(hCompClock, mtrt->nPortIndex, mtrt->nMediaTimestamp, AVSYNC_DROP);
                }
            }else{
                COMP_LOGD(rootComp, "The clock port[%d] state is in %s, directly drop the packets", 
                                     mtrt->nPortIndex,
                                     compImpl->mTransitionState == OMX_TransitionStateToIdle ? "transitToIdle" : compImpl->mState == OMX_StateIdle ? "Idle" : "not OMX_TIME_ClockStateRunning");
                /*added into the no delay message queue*/
                hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->postMessage(hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex], -1);
                /*hCompClock->sendAVSyncAction(hCompClock, mtrt->nPortIndex, mtrt->nMediaTimestamp, AVSYNC_DROP);*/
            }
            
        }
            break;

        case MagOMX_Component_Notify_ReferenceTimeUpdate:
        {
            OMX_TIME_CONFIG_TIMESTAMPTYPE *data = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pNotifyData;
            Mag_AcquireMutex(hCompClock->mhRefTimeUpdateMutex);
            hCompClock->mReferenceTimeBase = CONVERT_TO_MICROSECONDS(data->nTimestamp);
            hCompClock->mWallTimeBase      = hCompClock->getTimeNow(hCompClock) + hCompClock->mClockOffset;
            Mag_ReleaseMutex(hCompClock->mhRefTimeUpdateMutex);
        }
            break;

        default:
            return OMX_ErrorUnsupportedIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentClock_Query(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN MagOMX_Component_Query_Type_t queryIndex,
                                        OMX_IN OMX_PTR pQueryData){

    MagOmxComponentClock hCompClock;

    hCompClock = ooc_cast(hComponent, MagOmxComponentClock);
    switch (queryIndex){
        default:
            return OMX_ErrorUnsupportedIndex;
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE  virtual_MagOmxComponentClock_AddPortOnRequest(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U32 *pPortIdx){
    MagOmxComponentClock hCompClock;

    hCompClock = ooc_cast(hComponent, MagOmxComponentClock);
    return hCompClock->addClockPort(hCompClock, pPortIdx);
}


static OMX_ERRORTYPE  virtual_MagOmxComponentClock_TearDownTunnel(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_U32 portIdx){
    MagOmxComponentClock  clkComp;
    MagOmxComponentImpl   clkCompImpl;
    MagOmxComponent       clkCompRoot; 
    OMX_HANDLETYPE        clkPort;

    AGILE_LOGV("enter!");

    clkComp     = ooc_cast(hComponent, MagOmxComponentClock);
    clkCompImpl = ooc_cast(hComponent, MagOmxComponentImpl);
    clkCompRoot = ooc_cast(hComponent, MagOmxComponent);

    clkPort = clkCompImpl->getPort(clkCompImpl, portIdx);

    if (MagOmxComponentClockVirtual(clkComp)->MagOMX_Clock_RemovePort){
        return MagOmxComponentClockVirtual(clkComp)->MagOMX_Clock_RemovePort(clkComp, portIdx);
    }else{
        COMP_LOGE(clkCompRoot, "pure virtual function MagOMX_Clock_RemovePort() needs to be overrided!");
    }
    ooc_delete((Object)clkPort);
    
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentClock_Stop(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentClock  clkComp;
    OMX_U32 i;

    AGILE_LOGV("enter!");

    clkComp     = ooc_cast(hComponent, MagOmxComponentClock);

    for (i = 0; i < MAX_CLOCK_PORT_NUMBER; i++){
        if (clkComp->mTimeRequestLooper[i]){
            clkComp->mTimeRequestLooper[i]->forceOut(clkComp->mTimeRequestLooper[i]);
        }
    }

    clkComp->mState.eState = OMX_TIME_ClockStateStopped;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentClock_Start(
                                        OMX_IN  OMX_HANDLETYPE hComponent){
    /*MagOmxComponentClock  clkComp;
    OMX_U32 i;

    AGILE_LOGV("enter!");

    clkComp     = ooc_cast(hComponent, MagOmxComponentClock);

    for (i = 0; i < MAX_CLOCK_PORT_NUMBER; i++){
        if (clkComp->mTimeRequestLooper[i]){
            clkComp->mTimeRequestLooper[i]->setForceOut(clkComp->mTimeRequestLooper[i], MAG_FALSE);
        }
    }*/

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentClock_Pause(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentClock clkComp;
    OMX_U32 i;

    AGILE_LOGV("enter!");
    clkComp = ooc_cast(hComponent, MagOmxComponentClock);

    clkComp->mhTimer->pause(clkComp->mhTimer);
    for (i = 0; i < MAX_CLOCK_PORT_NUMBER; i++){
        if (clkComp->mTimeRequestLooper[i]){
            clkComp->mTimeRequestLooper[i]->suspend(clkComp->mTimeRequestLooper[i]);
        }
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentClock_Resume(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentClock clkComp;
    OMX_U32 i;

    AGILE_LOGV("enter!");
    clkComp = ooc_cast(hComponent, MagOmxComponentClock);

    for (i = 0; i < MAX_CLOCK_PORT_NUMBER; i++){
        if (clkComp->mTimeRequestLooper[i]){
            clkComp->mTimeRequestLooper[i]->resume(clkComp->mTimeRequestLooper[i]);
        }
    }
    clkComp->mhTimer->resume(clkComp->mhTimer);

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE virtual_MagOmxComponentClock_Flush(
                    OMX_IN  OMX_HANDLETYPE hComponent){
    MagOmxComponentClock  clkComp;
    OMX_U32 i;

    AGILE_LOGV("enter!");

    clkComp     = ooc_cast(hComponent, MagOmxComponentClock);

    clkComp->mState.eState = OMX_TIME_ClockStateWaitingForStartTime;

    for (i = 0; i < MAX_CLOCK_PORT_NUMBER; i++){
        if (clkComp->mTimeRequestLooper[i]){
            clkComp->mTimeRequestLooper[i]->forceOut(clkComp->mTimeRequestLooper[i]);
        }
    }

    return OMX_ErrorNone;
}

static OMX_TICKS MagOmxComponentClock_getTimeNow(MagOmxComponentClock compClock) {
    return compClock->mhTimer->get(compClock->mhTimer);
    /*return Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000LL;*/
}

static MagMessageHandle MagOmxComponentClock_createCmdMessage(OMX_HANDLETYPE handle, ui32 what) {
    MagOmxComponentClock hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentClock);
    hComponent->getCmdLooper(handle);
    
    msg = createMagMessage(hComponent->mCmdLooper, what, hComponent->mCmdMsgHandler->id(hComponent->mCmdMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d", what);
    }
    return msg;
}

static _status_t MagOmxComponentClock_getCmdLooper(OMX_HANDLETYPE handle){
    MagOmxComponentClock hComponent = NULL;

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentClock);
    
    if ((NULL != hComponent->mCmdLooper) && (NULL != hComponent->mCmdMsgHandler)){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mCmdLooper){
        hComponent->mCmdLooper = createLooper(CMD_LOOPER_NAME);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mCmdLooper);
    }
    
    if (NULL != hComponent->mCmdLooper){
        if (NULL == hComponent->mCmdMsgHandler){
            hComponent->mCmdMsgHandler = createHandler(hComponent->mCmdLooper, onCmdMessageReceived, handle);

            if (NULL != hComponent->mCmdMsgHandler){
                hComponent->mCmdLooper->registerHandler(hComponent->mCmdLooper, hComponent->mCmdMsgHandler);
                hComponent->mCmdLooper->start(hComponent->mCmdLooper);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", CMD_LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

static MagMessageHandle MagOmxComponentClock_createTimeRequestMessage(OMX_HANDLETYPE handle, OMX_U32 what, OMX_U32 port_id){
    MagOmxComponentClock hComponent = NULL;
    MagMessageHandle msg;

    if (NULL == handle){
        return NULL;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentClock);
    hComponent->getTimeRequestLooper(handle, port_id);
    
    msg = createMagMessage(hComponent->mTimeRequestLooper[port_id], 
                           what, 
                           hComponent->mTimeRequestMsgHandler[port_id]->id(hComponent->mTimeRequestMsgHandler[port_id]));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message: %d for port id:%d", what, port_id);
    }
    return msg;
}

static _status_t MagOmxComponentClock_getTimeRequestLooper(OMX_HANDLETYPE handle, OMX_U32 port_id){
    MagOmxComponentClock hComponent = NULL;
    char lname[64];

    if (NULL == handle){
        return MAG_BAD_VALUE;
    }
    
    hComponent = ooc_cast(handle, MagOmxComponentClock);
    
    if ((NULL != hComponent->mTimeRequestLooper[port_id]) && (NULL != hComponent->mTimeRequestMsgHandler[port_id])){
        return MAG_NO_ERROR;
    }
    
    if (NULL == hComponent->mTimeRequestLooper[port_id]){
        sprintf(lname, TR_LOOPER_NAME, port_id);
        hComponent->mTimeRequestLooper[port_id] = createLooper(lname);
        AGILE_LOGV("looper handler: 0x%x", hComponent->mTimeRequestLooper[port_id]);
    }
    
    if (NULL != hComponent->mTimeRequestLooper[port_id]){
        if (NULL == hComponent->mTimeRequestMsgHandler[port_id]){
            hComponent->mTimeRequestMsgHandler[port_id] = createHandler(hComponent->mTimeRequestLooper[port_id], 
                                                                        onTimeRequestMessageReceived, 
                                                                        handle);

            if (NULL != hComponent->mTimeRequestMsgHandler[port_id]){
                hComponent->mTimeRequestLooper[port_id]->registerHandler(hComponent->mTimeRequestLooper[port_id], 
                                                                         hComponent->mTimeRequestMsgHandler[port_id]);
                
                hComponent->mTimeRequestLooper[port_id]->setTimer(hComponent->mTimeRequestLooper[port_id], hComponent->mhTimer);
                hComponent->mTimeRequestLooper[port_id]->setPriority(hComponent->mTimeRequestLooper[port_id], MagLooper_Priority_High);

                hComponent->mTimeRequestLooper[port_id]->start(hComponent->mTimeRequestLooper[port_id]);
            }else{
                AGILE_LOGE("failed to create Handler[%d]", port_id);
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", lname);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}


static OMX_TICKS MagOmxComponentClock_getWallTime(OMX_HANDLETYPE hComponent){
    MagOmxComponentClock hCompClock;

    hCompClock = ooc_cast(hComponent, MagOmxComponentClock);
}

static OMX_ERRORTYPE MagOmxComponentClock_updateClockState(MagOmxPort port, OMX_TIME_CLOCKSTATE state){
    OMX_TIME_MEDIATIMETYPE mt;
    OMX_ERRORTYPE ret;

    initHeader(&mt, sizeof(OMX_TIME_MEDIATIMETYPE));
    mt.eUpdateType = OMX_TIME_UpdateClockStateChanged;
    mt.eState      = state;

    ret = MagOmxPortVirtual(port)->SetParameter(port, OMX_IndexConfigTimeUpdate, &mt);
    return ret;
}

static OMX_ERRORTYPE MagOmxComponentClock_updateClockScale(MagOmxPort port, OMX_S32 scale){
    OMX_TIME_MEDIATIMETYPE mt;
    OMX_ERRORTYPE ret;

    initHeader(&mt, sizeof(OMX_TIME_MEDIATIMETYPE));
    mt.eUpdateType = OMX_TIME_UpdateScaleChanged;
    mt.xScale      = scale;

    ret = MagOmxPortVirtual(port)->SetParameter(port, OMX_IndexConfigTimeUpdate, &mt);
    return ret;
}

static OMX_ERRORTYPE MagOmxComponentClock_addClockPort(MagOmxComponentClock compClock, OMX_U32 *pPortIndex){
    MagOmxComponent root;

    root = ooc_cast(compClock, MagOmxComponent);
    if (MagOmxComponentClockVirtual(compClock)->MagOMX_Clock_AddPort){
        return MagOmxComponentClockVirtual(compClock)->MagOMX_Clock_AddPort(compClock, pPortIndex);
    }else{
        COMP_LOGE(root, "pure virtual function MagOMX_Clock_AddPort() needs to be overrided!");
    }
}

/*in us unit*/
static OMX_TICKS     MagOmxComponentClock_getMediaTimeNow(MagOmxComponentClock compClock){
    OMX_TICKS tnow;

    if (compClock->mState.eState == OMX_TIME_ClockStateRunning){
        Mag_AcquireMutex(compClock->mhRefTimeUpdateMutex);
        tnow = compClock->mReferenceTimeBase + (compClock->mxScale * 
               (compClock->getTimeNow(compClock) - compClock->mWallTimeBase) / 10.0);
        Mag_ReleaseMutex(compClock->mhRefTimeUpdateMutex);
    }else{
        tnow = 0;
    }
    return tnow;
}

static OMX_TICKS     MagOmxComponentClock_getMediaTimeRequest(MagOmxComponentClock compClock, OMX_TICKS mtr, OMX_TICKS offset){
    OMX_TICKS clock_offset = 0;
    OMX_TICKS mt_request;
    MagOmxComponent root;

    root = ooc_cast(compClock, MagOmxComponent);
    if (MagOmxComponentClockVirtual(compClock)->MagOMX_Clock_GetOffset){
        MagOmxComponentClockVirtual(compClock)->MagOMX_Clock_GetOffset(compClock, &clock_offset);
    }else{
        COMP_LOGE(root, "pure virtual function MagOMX_GetOffset() needs to be overrided!");
    }

    mt_request = mtr - (compClock->mxScale * (offset + clock_offset) / 10);
    return mt_request;
}

static OMX_ERRORTYPE MagOmxComponentClock_sendAVSyncAction(MagOmxComponentClock compClock, 
                                                           OMX_U32 port_id, 
                                                           OMX_TICKS mediaTimestamp,
                                                           MagOMX_AVSync_Action_t action){
    MagOmxComponentImpl  base;
    MagOmxComponent      root;
    OMX_HANDLETYPE portHandle;
    MagOmxPort     portRoot;
    OMX_TIME_MEDIATIMETYPE timeUpdate;
    OMX_ERRORTYPE ret;
    OMX_TICKS t_now;

    base = ooc_cast(compClock, MagOmxComponentImpl);
    root = ooc_cast(compClock, MagOmxComponent);

    portHandle = base->getPort(base, port_id);
    portRoot = ooc_cast(portHandle, MagOmxPort);

    initHeader(&timeUpdate, sizeof(OMX_TIME_MEDIATIMETYPE));
    timeUpdate.nClientPrivate       = (OMX_U32)action;
    timeUpdate.eUpdateType          = OMX_TIME_UpdateRequestFulfillment;
    timeUpdate.nMediaTimestamp      = mediaTimestamp;
    timeUpdate.nOffset              = 0;
    timeUpdate.nWallTimeAtMediaTime = 0;

    t_now = compClock->getMediaTimeNow(compClock);
    COMP_LOGD(root, "sendAVSyncAction[port: %s](tm:0x%llx - tn:0x%llx[diff: %d] action: %s)!", 
                    portRoot->getPortName(portRoot), 
                    CONVERT_TO_MICROSECONDS(mediaTimestamp), 
                    t_now,
                    (OMX_S32)(t_now - CONVERT_TO_MICROSECONDS(mediaTimestamp)),
                    action == AVSYNC_PLAY ? "play" : "drop");

    ret = MagOmxPortVirtual(portRoot)->SetParameter(portRoot, OMX_IndexConfigTimeUpdate, &timeUpdate);
    return ret;
 }


/*Class Constructor/Destructor*/
static void MagOmxComponentClock_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_getType          = virtual_MagOmxComponentClock_getType;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter     = virtual_MagOmxComponentClock_GetParameter;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter     = virtual_MagOmxComponentClock_SetParameter;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_DoAddPortAction  = virtual_MagOmxComponentClock_DoAddPortAction;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Port_Notify      = virtual_MagOmxComponentClock_Notify;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Port_Query       = virtual_MagOmxComponentClock_Query;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_AddPortOnRequest = virtual_MagOmxComponentClock_AddPortOnRequest;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_TearDownTunnel   = virtual_MagOmxComponentClock_TearDownTunnel;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Stop             = virtual_MagOmxComponentClock_Stop;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Start            = virtual_MagOmxComponentClock_Start;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Pause            = virtual_MagOmxComponentClock_Pause;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Resume           = virtual_MagOmxComponentClock_Resume;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Flush            = virtual_MagOmxComponentClock_Flush;


    MagOmxComponentClockVtableInstance.MagOMX_Clock_DecideStartTime = NULL;
    MagOmxComponentClockVtableInstance.MagOMX_Clock_GetOffset       = NULL;
    MagOmxComponentClockVtableInstance.MagOMX_Clock_AddPort         = NULL;
    MagOmxComponentClockVtableInstance.MagOMX_Clock_RemovePort      = NULL;
}

static void MagOmxComponentClock_constructor(MagOmxComponentClock thiz, const void *params){
    OMX_U32 i;

    AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentClock));
    chain_constructor(MagOmxComponentClock, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhRefTimeUpdateMutex);

    thiz->mCmdLooper         = NULL;
    thiz->mCmdMsgHandler     = NULL;
    thiz->mCmdStartTimeMsg   = NULL;
    thiz->mWaitStartTimeMask = 0;
    thiz->mMaxRenderDelay    = 0;
    thiz->mReferenceTimeBase = 0;
    thiz->mWallTimeBase      = 0;
    thiz->mClockOffset       = 0;
    thiz->mxScale            = 10;

    for (i = 0; i < MAX_CLOCK_PORT_NUMBER; i++){
        thiz->mCmdMTimeRequestMsg[i] = NULL;
    }

    memset(thiz->mTimeRequestLooper, 0, 8*(sizeof(MagLooperHandle)));
    memset(thiz->mTimeRequestMsgHandler, 0, 8*(sizeof(MagHandlerHandle)));
    memset(thiz->mCmdMTimeRequestMsg, 0, 8*(sizeof(MagMessageHandle)));
    memset(thiz->mStartTimeTable, -1, 8*(sizeof(OMX_TICKS)));

    thiz->getCmdLooper             = MagOmxComponentClock_getCmdLooper;
    thiz->createCmdMessage         = MagOmxComponentClock_createCmdMessage;
    thiz->getTimeRequestLooper     = MagOmxComponentClock_getTimeRequestLooper;
    thiz->createTimeRequestMessage = MagOmxComponentClock_createTimeRequestMessage;
    thiz->getWallTime              = MagOmxComponentClock_getWallTime;
    thiz->updateClockState         = MagOmxComponentClock_updateClockState;
    thiz->updateClockScale         = MagOmxComponentClock_updateClockScale;
    thiz->addClockPort             = MagOmxComponentClock_addClockPort;
    thiz->getMediaTimeNow          = MagOmxComponentClock_getMediaTimeNow;
    thiz->getMediaTimeRequest      = MagOmxComponentClock_getMediaTimeRequest;
    thiz->sendAVSyncAction         = MagOmxComponentClock_sendAVSyncAction;
    thiz->getTimeNow               = MagOmxComponentClock_getTimeNow;

    memset(&thiz->mState, 0, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
    initHeader(&thiz->mState, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
    thiz->mState.eState     = OMX_TIME_ClockStateStopped;
    thiz->mState.nStartTime = -1;

    memset(&thiz->mRefClockUpdate, 0, sizeof(OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE));
    initHeader(&thiz->mState, sizeof(OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE));

    memset(&thiz->mMediaTimeType, 0, sizeof(OMX_TIME_MEDIATIMETYPE));
    initHeader(&thiz->mMediaTimeType, sizeof(OMX_TIME_MEDIATIMETYPE));

    Mag_CreateEventGroup(&thiz->mStateChangeEvtGrp);
    if (MAG_ErrNone == Mag_CreateEvent(&thiz->mStateChangeEvt, MAG_EVT_PRIO_DEFAULT))
        Mag_AddEventGroup(thiz->mStateChangeEvtGrp, thiz->mStateChangeEvt);

    thiz->mhTimer = Mag_createTimer();
}

static void MagOmxComponentClock_destructor(MagOmxComponentClock thiz, MagOmxComponentClockVtable vtab){
	AGILE_LOGV("Enter!");

    Mag_DestroyMutex(&thiz->mhMutex);
    Mag_DestroyMutex(&thiz->mhRefTimeUpdateMutex);
    Mag_destroyTimer(&thiz->mhTimer);
}