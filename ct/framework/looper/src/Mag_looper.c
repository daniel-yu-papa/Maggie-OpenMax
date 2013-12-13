#include "Mag_looper.h"


static i64 getNowMs() {
    return Mag_GetSystemTime(SYSTEM_TIME_MONOTONIC) / 1000000ll;
}

static MagLooperEvent_t *getFreeEvent(MagLooperHandle hlooper){
    List_t *tmpNode;
    MagLooperEvent_t *pEvent = NULL;
    
    tmpNode = hlooper->mFreeEvtQueue.next;

    if(tmpNode == &hlooper->mFreeEvtQueue){
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

static MagMessage_t *getNoDelayMessage(MagLooperHandle hlooper){
    List_t *tmpNode;
    MagLooperEvent_t *pEvent = NULL;
    
    tmpNode = hlooper->mNoDelayEvtQueue.next;

    if(tmpNode == &hlooper->mNoDelayEvtQueue){
        return NULL;
    }else{
        list_del(tmpNode);
        pEvent = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        list_add(tmpNode, &hlooper->mFreeEvtQueue);
        return pEvent->mMessage;
    }
}

static MagHandlerHandle getHandler(MagLooperHandle hlooper, ui32 id){
    return (MagHandlerHandle)rbtree_get(hlooper->mHandlerTreeRoot, id);
}

static boolean deliverMessage(MagLooperHandle hlooper, MagMessage_t *msg){
    MagHandlerHandle h;
    
    if (NULL == msg)
        return MAG_FALSE;

    h = getHandler(hlooper, msg->mTarget);

    if (NULL == h){
        AGILE_LOGE("failed to get Message handler with id(%u)", msg->mTarget);
        return MAG_FALSE;
    }else{
        h->onMessageReceived(msg, h->priv);
    }

    return MAG_TRUE;
}

static boolean MagEventQueueEmpty(MagLooperHandle hlooper){
    boolean ret;
    
    Mag_AcquireMutex(hlooper->mLock);
    if ((NULL == hlooper->mDelayEvtTreeRoot) && (&hlooper->mNoDelayEvtQueue == hlooper->mNoDelayEvtQueue.next))
        ret = MAG_TRUE;
    else
        ret = MAG_FALSE;
    Mag_ReleaseMutex(hlooper->mLock);
    return ret;
}

static void clearMessageQueue(MagLooperHandle hlooper){
    i64 key;
    void *value;
    MagLooperEvent_t *evt;
    while (hlooper->mDelayEvtTreeRoot){
        rbtree_getMinValue(hlooper->mDelayEvtTreeRoot, &key, &value);
        rbtree_delete(&hlooper->mDelayEvtTreeRoot, key);
        evt = (MagLooperEvent_t *)value;
        list_add(&evt->node, &hlooper->mFreeEvtQueue);
    }

    List_t *tmpNode;
    
    tmpNode = hlooper->mNoDelayEvtQueue.next;
    while (tmpNode != &hlooper->mNoDelayEvtQueue){
        list_del(tmpNode);
        evt = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        list_add(&evt->node, &hlooper->mFreeEvtQueue);
        tmpNode = hlooper->mNoDelayEvtQueue.next;
    }
}

/*run once before the Looper thread start to be looping*/
static boolean LooperThreadReadyToRun(void *priv){

}

static boolean LooperThreadEntry(void *priv){
    MagLooperHandle looper = (MagLooperHandle)priv;
    MagMessage_t *msg = NULL;
    
    if (NULL == priv)
        return MAG_FALSE;
    
    if (MagEventQueueEmpty())
        Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);

    Mag_AcquireMutex(looper->mLock);
    if (NULL != looper->mDelayEvtTreeRoot){
        i64 whenMS;
        i64 nowMS = getNowMs();
        void *value;
        rbtree_getMinValue(looper->mDelayEvtTreeRoot, &whenMS, &value);

        if (whenMS > nowMS){
            msg = getNoDelayMessage(looper);
            Mag_ReleaseMutex(looper->mLock);
            
            if (NULL == msg){
                Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, (whenMS - nowMS));
                return MAG_TRUE;
            }else{
                deliverMessage(looper, msg);
            }
        }else{
            rbtree_delete(&looper->mDelayEvtTreeRoot, whenMS);
            
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

void MagLooper_registerHandler(MagLooperHandle hlooper, const MagHandler_t *handler){
    hlooper->handlerTreeRoot = rbtree_insert(hlooper->mHandlerTreeRoot, handler->uiID, (void *)handler);
}

_status_t MagLooper_unregisterHandler(MagLooperHandle hlooper, i32 handlerID){
    if (rbtree_delete(&hlooper->mHandlerTreeRoot, handlerID) != 0){
        AGILE_LOGE("failed to unregister the handler ID: %d", handlerID);
        return MAG_NAME_NOT_FOUND;
    }
    return MAG_NO_ERROR;
}

_status_t MagLooper_start(MagLooperHandle hlooper){
    _status_t ret;
    
    if (hlooper->mLooperThread == NULL){
        return MAG_BAD_VALUE;
    }

    ret = hlooper->mLooperThread->run(hlooper->mLooperThread);

    if (ret != MAG_NO_ERROR){
        Mag_DestroyThread(hlooper->mLooperThread);
        hlooper->mLooperThread = NULL;
    }  
    return ret;
}

_status_t MagLooper_stop(MagLooperHandle hlooper){
    if (hlooper->mLooperThread == NULL)
        return MAG_INVALID_OPERATION;
    
    Mag_SetEvent(hlooper->mMQEmptyPushEvt);
    hlooper->mLooperThread->requestExitAndWait(hlooper, MAG_TIMEOUT_INFINITE);
    clearMessageQueue(hlooper);
    
    return MAG_NO_ERROR;
}

void MagLooper_postMessage(MagLooperHandle hlooper, const MagMessage_t *msg, i64 delayMs){
    MagLooperEvent_t *evt;
    boolean postEvt = MAG_FALSE;
    
    Mag_AcquireMutex(hlooper->mLock);
    
    evt = getFreeEvent(hlooper);
    if (evt != NULL){
        if (delayMs > 0){
            evt->mWhenMs = getNowMs() + delayMs;
            evt->mMessage = msg;
            hlooper->mDelayEvtTreeRoot = rbtree_insert(hlooper->mDelayEvtTreeRoot, evt->mWhenMs, (void *)evt);
        }else{
            evt->mWhenMs = 0;
            evt->mMessage = msg;
            /*only take care of the NoDelay Message Queue Empty status*/
            if (&hlooper->mNoDelayEvtQueue == hlooper->mNoDelayEvtQueue.next)
                postEvt = MAG_TRUE;
            list_add_tail(&evt->node, &hlooper->mNoDelayEvtQueue);
        }
    }
    
    Mag_ReleaseMutex(hlooper->mLock);
    
    if (postEvt)
        Mag_SetEvent(hlooper->mMQEmptyPushEvt);
}


MagLooperHandle createLooper(const ui8 *pName){
    MagLooperHandle pLooper;
    char threadName[64];
    
    pLooper = (MagLooperHandle)mag_malloc(sizeof(MagLooper_t));

    if (NULL != pLooper){
        pLooper->mpName             = mag_strdup(pName);
        sprintf(threadName, "MLooper%s", pName);
        
        pLooper->mHandlerTreeRoot   = NULL;
        pLooper->mDelayEvtTreeRoot  = NULL;
        
        INIT_LIST(&pLooper->mNoDelayEvtQueue);
        INIT_LIST(&pLooper->mFreeEvtQueue);
        Mag_CreateMutex(&pLooper->mLock);
        
        pLooper->mLooperThread = Mag_CreateThread(threadName, LooperThreadEntry, pLooper)
        if (pLooper->mLooperThread != NULL){
            pLooper->mLooperThread->setFunc_readyToRun(pLooper->mLooperThread, LooperThreadReadyToRun);
        }else{
            AGILE_LOGE("failed to create the Looper %s thread!", pName);
        }

        Mag_CreateEventGroup(&pLooper->mMQPushEvtGroup);
        if (MAG_ErrNone == Mag_CreateEvent(&pLooper->mMQEmptyPushEvt, 0)){}
            Mag_AddEventGroup(pLooper->mMQPushEvtGroup, pLooper->mMQEmptyPushEvt);
            
        pLooper->registerHandler   = MagLooper_registerHandler;
        pLooper->unregisterHandler = MagLooper_unregisterHandler;
        pLooper->start             = MagLooper_start;
        pLooper->stop              = MagLooper_stop;
        pLooper->postMessage       = MagLooper_postMessage;

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

void destroyLooper(MagLooperHandle hlooper){
    hlooper->stop(hlooper);

    Mag_DestroyMutex(hlooper->mLock);

    Mag_DestroyEvent(hlooper->mMQEmptyPushEvt);
    Mag_DestroyEventGroup(hlooper->mMQPushEvtGroup);

    List_t *tmpNode;
    MagLooperEvent_t *evt;
    
    tmpNode = hlooper->mFreeEvtQueue.next;
    while (tmpNode != &hlooper->mFreeEvtQueue){
        list_del(tmpNode);
        evt = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        mag_free(evt);
        tmpNode = hlooper->mFreeEvtQueue.next;
    }
    mag_free(hlooper);
}

