#include "MagOMX_Component_clock.h"

#define CMD_LOOPER_NAME        "CompClockCMDLooper"
#define TR_LOOPER_NAME         "CompClockTRLooper%d"

AllocateClass(MagOmxComponentClock, MagOmxComponentImpl);

static i64 getNowUS() {
    return Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
}

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

    root = ooc_cast(MagOmxComponent, priv);
    base = ooc_cast(MagOmxComponentImpl, priv);
    thiz = ooc_cast(MagOmxComponentClock, priv);

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
            MagOmxComponentClock_StartTimePolicy_t policy;
            OMX_TIME_CONFIG_RENDERINGDELAYTYPE renderDelay;

            if (!msg->findInt64(msg, "time_stamp", &timeStamp)){
                COMP_LOGE(root, "failed to find the time_stamp!");
                return;
            }

            if (!msg->findInt32(msg, "port_index", &portIndex)){
                COMP_LOGE(root, "failed to find the port_index!");
                return;
            }

            if (thiz->mState.nWaitMask & (1 << portIndex) == (1 << portIndex)){
                thiz->mState.nWaitMask &= ~(1 << portIndex);
                mStartTimeTable[portIndex] = timeStamp;
                if (thiz->mState.nWaitMask == 0){
                    COMP_LOGD("All streams have set the start time!");
                    if (MagOmxComponentClockVirtual(thiz)->MagOMX_DecideStartTime){
                        ret = thiz->MagOMX_DecideStartTime(thiz, mStartTimeTable, &thiz->mState.nStartTime);
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
                                    COMP_LOGE(root, "Failed to get render delay of port %d", port->getPortIndex(port));
                                    return;
                                }
                            }
                            COMP_LOGD(root, "Max rendering delay: %d", thiz->mMaxRenderDelay);

                            /*initialize the Rbase and Wbase while setting state to running*/
                            if (thiz->mState.nOffset < 0)
                                thiz->mClockOffset   = MagOMX_MIN(thiz->mState.nOffset, (0 - thiz->mMaxRenderDelay));
                            else
                                thiz->mClockOffset   = thiz->mState.nOffset - thiz->mMaxRenderDelay;

                            thiz->mReferenceTimeBase = thiz->mState.nStartTime;
                            thiz->mWallTimeBase      = getNowUS() - thiz->mClockOffset;
                            thiz->mState.eState      = OMX_TIME_ClockStateRunning;

                            /*notify connected components the running state*/
                            for (n = rbtree_first(base->mPortTreeRoot); n; n = rbtree_next(n)) {
                                portClock = ooc_cast(n->value, MagOmxPortClock);
                                port = ooc_cast(portClock, MagOmxPort);
                                ret = portClock->updateClockState(port, OMX_TIME_ClockStateRunning);
                                if (ret != OMX_ErrorNone){
                                    COMP_LOGE(root, "Failed to do port(%d)->updateClockState()", port->getPortIndex(port));
                                }
                            }
                        }
                    }else{
                        thiz->mState.nStartTime = 0;
                        COMP_LOGE(root, "pure virtual function MagOMX_DecideStartTime() should be overrided");
                    }
                }else{
                    COMP_LOGD(root, "Port %d sets the start time %lld!", portIndex, timeStamp);
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

    root = ooc_cast(MagOmxComponent, priv);
    base = ooc_cast(MagOmxComponentImpl, priv);
    thiz = ooc_cast(MagOmxComponentClock, priv);

    if (!msg){
        COMP_LOGE(root, "msg is NULL!");
        return;
    }

    cmd = msg->what(msg);
    switch (cmd){
        case MagOmxComponentClock_CmdMTimeRequestMsg:
            i64 timeStamp;
            i32 portIndex;
            OMX_PTR pClientPriv;

            if (!msg->findInt64(msg, "time_stamp", &timeStamp)){
                COMP_LOGE(root, "failed to find the time_stamp!");
                return;
            }

            if (!msg->setInt32(msg, "port_index", &portIndex)){
                COMP_LOGE(root, "failed to find the port_index!");
                return;
            }

            if (msg->setPointer(msg, "client_private", &pClientPriv)){
                COMP_LOGE(root, "failed to find the client_private!");
                return;
            }
            
            thiz->sendAVSyncAction(thiz, portIndex, timeStamp, AVSYNC_PLAY);
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

    switch (nIndex){
    	case OMX_IndexConfigTimeScale:
	    	OMX_TIME_CONFIG_SCALETYPE *output = (OMX_TIME_CONFIG_SCALETYPE *)pComponentParameterStructure;
	    	initHeader(output, sizeof(OMX_TIME_CONFIG_SCALETYPE));
	    	output->xScale = thiz->mxScale;
    		break;

    	case OMX_IndexConfigTimeClockState:
	    	OMX_TIME_CONFIG_CLOCKSTATETYPE *output = (OMX_TIME_CONFIG_CLOCKSTATETYPE)pComponentParameterStructure;
	    	memcpy(output, &thiz->mState, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
    		break;

    	case OMX_IndexConfigTimeActiveRefClockUpdate:
	    	OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *output = (OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE *)pComponentParameterStructure;
	    	memcpy(output, &thiz->mRefClockUpdate, sizeof(OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE));
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
	    	OMX_TIME_CONFIG_SCALETYPE *input = (OMX_TIME_CONFIG_SCALETYPE *)pComponentParameterStructure;
	    	thiz->mxScale = input->xScale;
    		break;

    	case OMX_IndexConfigTimeClockState:
            OMX_TIME_CONFIG_CLOCKSTATETYPE *input = (OMX_TIME_CONFIG_CLOCKSTATETYPE *)pComponentParameterStructure;
            if ((input->eState == OMX_TIME_ClockStateWaitingForStartTime) &&
                (thiz->mState.eState == OMX_TIME_ClockStateRunning){
                COMP_LOGE(root, "Invalid state transition from ClockStateWaitingForStartTime to ClockStateRunning");
                ret = OMX_ErrorIncorrectStateTransitionl;
            }else{
                COMP_LOGD(root, "State transition from %s to %s", 
                                ClockCompState2String(thiz->mState.eState),
                                ClockCompState2String(input->eState));
                thiz->mState.eState = input->eState;
                thiz->mState.nOffset = input->nOffset;
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

    hCompClock = ooc_cast(MagOmxComponentClock, hComponent);

    hCompClock->mState.nWaitMask |= 1 << portIndex; 
    hCompClock->mWaitStartTimeMask = hCompClock->mState.nWaitMask;
}
    
/*handle the notification from attached port*/
static OMX_ERRORTYPE  virtual_MagOmxComponentClock_Notify(
                                        OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN MagOMX_Component_Notify_Type_t notifyIndex,
                                        OMX_IN OMX_PTR pNotifyData){
    MagOmxComponentClock hCompClock;

    hCompClock = ooc_cast(MagOmxComponentClock, hComponent);
    switch (notifyIndex){
        case MagOMX_Component_Notify_StartTime:
        {
            OMX_TIME_CONFIG_TIMESTAMPTYPE *tst;
            tst = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pNotifyData;
            if (hCompClock->mCmdStartTimeMsg == NULL){
                hCompClock->mCmdStartTimeMsg = hCompClock->createMessage(hComponent, MagOmxComponentClock_CmdStartTimeMsg);  
            }
            hCompClock->mCmdStartTimeMsg->setInt64(hCompClock->mCmdStartTimeMsg, "time_stamp", tst->nTimestamp);
            hCompClock->mCmdStartTimeMsg->setInt32(hCompClock->mCmdStartTimeMsg, "port_index", tst->nPortIndex);
            hCompClock->mCmdStartTimeMsg->postMessage(base->mCmdStartTimeMsg, 0);
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
            hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->setPointer(hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex], "client_private", mtrt->pClientPrivate);

            t_now     = hCompClock->getMediaTimeNow(hCompClock);
            t_request = hCompClock->getMediaTimeRequest(hCompClock, mtrt->nMediaTimestamp, mtrt->nOffset);

            delay = t_request - t_now;
            if (delay >= 0){
                hCompClock->mCmdMTimeRequestMsg[mtrt->nPortIndex]->postMessage(base->mCmdMTimeRequestMsg[mtrt->nPortIndex], delay);
            }else{
                /*only support the forward playback for now, so drop the stream packet*/
                hCompClock->sendAVSyncAction(hCompClock, mtrt->nPortIndex, mtrt->nMediaTimestamp, AVSYNC_DROP);
            }
            
        }
            break;

        case MagOMX_Component_Notify_ReferenceTimeUpdate:
            OMX_TIME_CONFIG_TIMESTAMPTYPE *data = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pNotifyData;
            Mag_AcquireMutex(thiz->mhRefTimeUpdateMutex);
            hCompClock->mReferenceTimeBase = data->nTimestamp;
            hCompClock->mWallTimeBase      = getNowUS() + hCompClock->mClockOffset;
            Mag_ReleaseMutex(thiz->mhRefTimeUpdateMutex);
            break;

        default:
            return OMX_ErrorUnsupportedIndex;
    }
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

    hCompClock = ooc_cast(MagOmxComponentClock, hComponent);
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

static OMX_ERRORTYPE MagOmxComponentClock_addClockPort(MagOmxComponentClock compClock){

}

/*in us unit*/
static OMX_TICKS     MagOmxComponentClock_getMediaTimeNow(MagOmxComponentClock compClock){
    OMX_TICKS tnow;

    if (compClock->mState.eState == OMX_TIME_ClockStateRunning){
        Mag_AcquireMutex(compClock->mhRefTimeUpdateMutex);
        tnow = compClock->mReferenceTimeBase + (compClock->mxScale * (getNowUS() - compClock->mWallTimeBase) / 10);
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
    if (MagOmxComponentClockVirtual(compClock)->MagOMX_GetOffset){
        ret = MagOmxComponentClockVirtual(compClock)->MagOMX_GetOffset(compClock, &clock_offset);
        if (ret == OMX_ErrorNone){

        }
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
    OMX_HANDLETYPE portHandle;
    MagOmxPort     portRoot;
    OMX_TIME_MEDIATIMETYPE timeUpdate;
    OMX_ERRORTYPE ret;

    base = ooc_cast(compClock, MagOmxComponentImpl);

    portHandle = base->getPort(base, port_id);
    portRoot = ooc_cast(portHandle, MagOmxPort);

    initHeader(&timeUpdate, sizeof(OMX_TIME_MEDIATIMETYPE));
    timeUpdate.nClientPrivate       = (OMX_U32)action;
    timeUpdate.eUpdateType          = OMX_TIME_UpdateRequestFulfillment;
    timeUpdate.nMediaTimestamp      = mediaTimestamp;
    timeUpdate.nOffset              = 0;
    timeUpdate.nWallTimeAtMediaTime = 0;

    ret = MagOmxPortVirtual(port)->SetParameter(port, OMX_IndexConfigTimeUpdate, &timeUpdate);
    return ret;
 }


/*Class Constructor/Destructor*/
static void MagOmxComponentClock_initialize(Class this){
	AGILE_LOGV("Enter!");
	
	/*Override the base component pure virtual functions*/
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_getType         = virtual_MagOmxComponentClock_getType;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_GetParameter    = virtual_MagOmxComponentClock_GetParameter;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_SetParameter    = virtual_MagOmxComponentClock_SetParameter;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_DoAddPortAction = virtual_MagOmxComponentClock_DoAddPortAction;
    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_Notify          = virtual_MagOmxComponentClock_Notify;

    MagOmxComponentClockVtableInstance.MagOmxComponentImpl.MagOMX_DecideStartTimePolicy = NULL;
}

static void MagOmxComponentClock_constructor(MagOmxComponentClock thiz, const void *params){
	AGILE_LOGV("Enter!");

	MAG_ASSERT(ooc_isInitialized(MagOmxComponentClock));
    chain_constructor(MagOmxComponentClock, thiz, params);

    Mag_CreateMutex(&thiz->mhMutex);
    Mag_CreateMutex(&thiz->mhRefTimeUpdateMutex);

    thiz->mCmdLooper         = NULL;
    thiz->mCmdMsgHandler     = NULL;
    thiz->mCmdStartTimeMsg   = NULL;
    thiz->mCmdMTimeRequestMsg = NULL;
    thiz->mWaitStartTimeMask = 0;
    thiz->mMaxRenderDelay    = 0;
    thiz->mReferenceTimeBase = 0;
    thiz->mWallTimeBase      = 0;
    thiz->mClockOffset       = 0;

    memset(thiz->mTimeRequestLooper, 0, 8*(sizeof(MagLooperHandle)));
    memset(thiz->mTimeRequestMsgHandler, 0, 8*(sizeof(MagHandlerHandle)));
    memset(thiz->mCmdMTimeRequestMsg, 0, 8*(sizeof(MagMessageHandle)));
    memset(thiz->mStartTimeTable, 0, 8*(sizeof(OMX_TICKS)));

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

    memset(&thiz->mState, 0, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
    initHeader(&thiz->mState, sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
    thiz->mState.eState = OMX_TIME_ClockStateStopped;

    memset(&thiz->mRefClockUpdate, 0, sizeof(OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE));
    initHeader(&thiz->mState, sizeof(OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE));

    memset(&thiz->mMediaTimeType, 0, sizeof(OMX_TIME_MEDIATIMETYPE));
    initHeader(&thiz->mMediaTimeType, sizeof(OMX_TIME_MEDIATIMETYPE));
}

static void MagOmxComponentClock_destructor(MagOmxComponentClock thiz, MagOmxComponentClockVtable vtab){
	AGILE_LOGV("Enter!");

	destroyMagMiniDB(&thiz->mParametersDB);
    Mag_DestroyMutex(&thiz->mhMutex);
    Mag_DestroyMutex(&thiz->mhRefTimeUpdateMutex);
}