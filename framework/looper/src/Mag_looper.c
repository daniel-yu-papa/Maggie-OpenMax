#include "Mag_looper.h"

static ui32 gLooperId = 0;

static i64 getNowMs() {
    return Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000000ll;
}

static MagLooperEvent_t *getFreeEvent(MagLooperHandle hLooper){
    List_t *tmpNode;
    MagLooperEvent_t *pEvent = NULL;
    
    tmpNode = hLooper->mFreeEvtQueue.next;

    if(tmpNode == &hLooper->mFreeEvtQueue){
        AGILE_LOGV("no nodes in the free list. malloc one");
        pEvent = mag_malloc(sizeof(MagLooperEvent_t));
        if (pEvent != NULL)
            INIT_LIST(&pEvent->node);
    }else{
        list_del(tmpNode);
        pEvent = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);    
    }
    return pEvent;
}

static struct mag_message *getNoDelayMessage(MagLooperHandle hLooper){
    List_t *tmpNode;
    MagLooperEvent_t *pEvent = NULL;
    
    tmpNode = hLooper->mNoDelayEvtQueue.next;

    if(tmpNode == &hLooper->mNoDelayEvtQueue){
        return NULL;
    }else{
        list_del(tmpNode);
        pEvent = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        list_add(tmpNode, &hLooper->mFreeEvtQueue);
        return pEvent->mMessage;
    }
}

static MagHandlerHandle getHandler(MagLooperHandle hLooper, ui32 id){
    return (MagHandlerHandle)rbtree_get(hLooper->mHandlerTreeRoot, id);
}

static boolean deliverMessage(MagLooperHandle hLooper, struct mag_message *msg){
    MagHandlerHandle h;
    
    if (NULL == msg)
        return MAG_FALSE;

    h = getHandler(hLooper, msg->mTarget);

    if (NULL == h){
        AGILE_LOGE("failed to get Message handler with id(%u)", msg->mTarget);
        return MAG_FALSE;
    }else{
        h->mMsgProcessor(msg, h->priv);
    }

    return MAG_TRUE;
}

static boolean MagEventQueueEmpty(MagLooperHandle hLooper){
    boolean ret;
    
    Mag_AcquireMutex(hLooper->mLock);
    if ((NULL == hLooper->mDelayEvtTreeRoot) && (&hLooper->mNoDelayEvtQueue == hLooper->mNoDelayEvtQueue.next))
        ret = MAG_TRUE;
    else
        ret = MAG_FALSE;
    Mag_ReleaseMutex(hLooper->mLock);
    return ret;
}

static void clearMessageQueue(MagLooperHandle hLooper){
    i64 key;
    void *value;
    MagLooperEvent_t *evt;
    while (hLooper->mDelayEvtTreeRoot){
        rbtree_getMinValue(hLooper->mDelayEvtTreeRoot, &key, &value);
        rbtree_delete(&hLooper->mDelayEvtTreeRoot, key);
        evt = (MagLooperEvent_t *)value;
        list_add(&evt->node, &hLooper->mFreeEvtQueue);
    }
    hLooper->mDelayEvtWhenMS = 0;
    
    List_t *tmpNode;
    
    tmpNode = hLooper->mNoDelayEvtQueue.next;
    while (tmpNode != &hLooper->mNoDelayEvtQueue){
        list_del(tmpNode);
        evt = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        list_add(&evt->node, &hLooper->mFreeEvtQueue);
        tmpNode = hLooper->mNoDelayEvtQueue.next;
    }
}

/*run once before the Looper thread start to be looping*/
static boolean LooperThreadReadyToRun(void *priv){
    return MAG_TRUE;
}

static boolean LooperThreadEntry(void *priv){
    MagLooperHandle looper = (MagLooperHandle)priv;
    MagMessage_t *msg = NULL;
    
    if (NULL == priv)
        return MAG_FALSE;

    if (MagEventQueueEmpty(looper)){
        Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
    }
    
    Mag_AcquireMutex(looper->mLock);
    if (NULL != looper->mDelayEvtTreeRoot){
        i64 nowMS = getNowMs();
        void *value;
        rbtree_getMinValue(looper->mDelayEvtTreeRoot, &looper->mDelayEvtWhenMS, &value);
        //AGILE_LOGD("min value: %lld", looper->mDelayEvtWhenMS);
        if (looper->mDelayEvtWhenMS > nowMS){
            msg = getNoDelayMessage(looper);
            Mag_ReleaseMutex(looper->mLock);
            
            if (NULL == msg){
                //AGILE_LOGD("wait on event, timeout=%d", (i32)(looper->mDelayEvtWhenMS - nowMS));
                Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, (i32)(looper->mDelayEvtWhenMS - nowMS));
                return MAG_TRUE;
            }else{
                deliverMessage(looper, msg);
            }
        }else{
            rbtree_delete(&looper->mDelayEvtTreeRoot, looper->mDelayEvtWhenMS);
            
            MagLooperEvent_t *evt = (MagLooperEvent_t *)value;
            msg = (MagMessage_t *)evt->mMessage;
            list_add(&evt->node, &looper->mFreeEvtQueue);
            Mag_ReleaseMutex(looper->mLock);
            
            deliverMessage(looper, msg);
        }
    }else{
        msg = getNoDelayMessage(looper);
        Mag_ReleaseMutex(looper->mLock);
        
        deliverMessage(looper, msg);
    }
    return MAG_TRUE;
}

static void MagLooper_registerHandler(MagLooperHandle hLooper, const MagHandler_t *handler){
    hLooper->mHandlerTreeRoot = rbtree_insert(hLooper->mHandlerTreeRoot, handler->uiID, (void *)handler);
}

static _status_t MagLooper_unregisterHandler(MagLooperHandle hLooper, i32 handlerID){
    if (rbtree_delete(&hLooper->mHandlerTreeRoot, handlerID) != 0){
        AGILE_LOGE("failed to unregister the handler ID: %d", handlerID);
        return MAG_NAME_NOT_FOUND;
    }
    return MAG_NO_ERROR;
}

static _status_t MagLooper_start(MagLooperHandle hLooper){
    _status_t ret;

    if (hLooper->mLooperThread == NULL){
        return MAG_BAD_VALUE;
    }

    ret = hLooper->mLooperThread->run(hLooper->mLooperThread);
    if (ret != MAG_NO_ERROR){
        AGILE_LOGE("failed to run Looper!, ret = 0x%x", ret);
        Mag_DestroyThread(hLooper->mLooperThread);
        hLooper->mLooperThread = NULL;
    }  
    return ret;
}

static _status_t MagLooper_stop(MagLooperHandle hLooper){
    if (hLooper->mLooperThread == NULL)
        return MAG_INVALID_OPERATION;
    
    Mag_SetEvent(hLooper->mMQEmptyPushEvt);
    hLooper->mLooperThread->requestExitAndWait(hLooper->mLooperThread, MAG_TIMEOUT_INFINITE);
    clearMessageQueue(hLooper);
    
    return MAG_NO_ERROR;
}

static void MagLooper_postMessage(MagLooperHandle hLooper, MagMessage_t *msg, i64 delayMs){
    MagLooperEvent_t *evt;
    boolean postEvt = MAG_FALSE;
    
    Mag_AcquireMutex(hLooper->mLock);
    
    evt = getFreeEvent(hLooper);
    if (evt != NULL){
        if (delayMs > 0){
            evt->mWhenMs = getNowMs() + delayMs;
            evt->mMessage = msg;
            if (hLooper->mDelayEvtTreeRoot == NULL)
                postEvt = MAG_TRUE;
            
            if (evt->mWhenMs < hLooper->mDelayEvtWhenMS)
                postEvt = MAG_TRUE;

            hLooper->mDelayEvtTreeRoot = rbtree_insert(hLooper->mDelayEvtTreeRoot, evt->mWhenMs, (void *)evt);
        }else{
            evt->mWhenMs = 0;
            evt->mMessage = msg;
            /*only take care of the NoDelay Message Queue Empty status*/
            if (&hLooper->mNoDelayEvtQueue == hLooper->mNoDelayEvtQueue.next)
                postEvt = MAG_TRUE;
            list_add_tail(&evt->node, &hLooper->mNoDelayEvtQueue);
        }
    }
    
    Mag_ReleaseMutex(hLooper->mLock);
    
    if (postEvt){
        Mag_SetEvent(hLooper->mMQEmptyPushEvt);
    }
    AGILE_LOGD("post msg: target %d", msg->mTarget);
}

static ui32 MagLooper_getHandlerID(MagLooperHandle hLooper){
    return ++hLooper->mHandlerID;
}

static _status_t MagLooper_waitOnAllDone(MagLooperHandle hLooper){
    while(!MagEventQueueEmpty(hLooper)){
        usleep(40000);
    }
}

MagLooperHandle createLooper(const char *pName){
    MagLooperHandle pLooper;
    char threadName[64];

    pLooper = (MagLooperHandle)mag_mallocz(sizeof(MagLooper_t));
    if (NULL != pLooper){
        pLooper->mpName             = mag_strdup(pName);
        sprintf(threadName, "MLooper%s", pName);
        
        pLooper->mHandlerTreeRoot   = NULL;
        pLooper->mDelayEvtTreeRoot  = NULL;
        
        INIT_LIST(&pLooper->mNoDelayEvtQueue);
        INIT_LIST(&pLooper->mFreeEvtQueue);
        Mag_CreateMutex(&pLooper->mLock);
        
        pLooper->mLooperThread = Mag_CreateThread(threadName, LooperThreadEntry, (void *)pLooper);
        if (pLooper->mLooperThread != NULL){
            pLooper->mLooperThread->setFunc_readyToRun(pLooper->mLooperThread, LooperThreadReadyToRun);
        }else{
            AGILE_LOGE("failed to create the Looper %s thread!", pName);
        }

        Mag_CreateEventGroup(&pLooper->mMQPushEvtGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&pLooper->mMQEmptyPushEvt, 0)){}
            Mag_AddEventGroup(pLooper->mMQPushEvtGroup, pLooper->mMQEmptyPushEvt);

        pLooper->mLooperID = ++gLooperId;
        
        pLooper->registerHandler   = MagLooper_registerHandler;
        pLooper->unregisterHandler = MagLooper_unregisterHandler;
        pLooper->start             = MagLooper_start;
        pLooper->stop              = MagLooper_stop;
        pLooper->postMessage       = MagLooper_postMessage;
        pLooper->getHandlerID      = MagLooper_getHandlerID;
        pLooper->waitOnAllDone     = MagLooper_waitOnAllDone;
        
        ui32 i;
        MagLooperEvent_t *pEvent;
        for (i = 0; i < NUM_PRE_ALLOCATED_EVENTS; i++){
            pEvent = mag_malloc(sizeof(MagLooperEvent_t));
            INIT_LIST(&pEvent->node);
            list_add(&pEvent->node, &pLooper->mFreeEvtQueue);
        }
    }else{
        AGILE_LOGE("failed to malloc MagLooper_t");
    }
    return pLooper;
}

void destroyLooper(MagLooperHandle hLooper){
    hLooper->stop(hLooper);

    Mag_DestroyThread(hLooper->mLooperThread);
    
    Mag_DestroyMutex(hLooper->mLock);

    Mag_DestroyEvent(hLooper->mMQEmptyPushEvt);
    Mag_DestroyEventGroup(hLooper->mMQPushEvtGroup);

    List_t *tmpNode;
    MagLooperEvent_t *evt;
    
    tmpNode = hLooper->mFreeEvtQueue.next;
    while (tmpNode != &hLooper->mFreeEvtQueue){
        list_del(tmpNode);
        evt = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        mag_free(evt);
        tmpNode = hLooper->mFreeEvtQueue.next;
    }
    mag_free(hLooper);
}

static ui32 MagHandler_id(MagHandlerHandle h){
    return h->uiID;
}

MagHandlerHandle createHandler(MagLooperHandle hLooper, fnOnMessageReceived cb, void *priv){
    MagHandlerHandle h;

    h = mag_mallocz(sizeof(MagHandler_t));

    if (h != NULL){
        h->uiID          = hLooper->getHandlerID(hLooper);
        h->mMsgProcessor = cb;
        h->priv          = priv;

        h->id            = MagHandler_id;
    }
    return h;
}

void destroyHandler(MagHandlerHandle hHandler){
    mag_free(hHandler);
}

