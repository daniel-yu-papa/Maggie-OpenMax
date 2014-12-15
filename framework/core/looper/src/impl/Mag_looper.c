#include <unistd.h>
#include "Mag_looper.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Looper"


static ui32 gLooperId = 0;

static i64 getNowUS() {
    return Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
}

static MagLooperEvent_t *getFreeEvent(MagLooperHandle hLooper){
    List_t *tmpNode;
    MagLooperEvent_t *pEvent = NULL;
    
    tmpNode = hLooper->mFreeEvtQueue.next;

    if(tmpNode == &hLooper->mFreeEvtQueue){
        AGILE_LOGV("[0x%x][free nodes = %d]no nodes in the free list. malloc one", hLooper, hLooper->mFreeNodeNum);
        pEvent = mag_mallocz(sizeof(MagLooperEvent_t));
        if (pEvent != NULL){
            INIT_LIST(&pEvent->node);
            pEvent->mMessage = (struct mag_message *)mag_mallocz(sizeof(struct mag_message));
        }
    }else{
        list_del(tmpNode);
        pEvent = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node); 
        hLooper->mFreeNodeNum--;
    }
    return pEvent;
}

static void putFreeEvent(MagLooperHandle hLooper, MagLooperEvent_t *pEvent){
    MagMessageHandle msg;
    ui32 i;
    msg = pEvent->mMessage;

    for (i = 0; i < msg->mNumItems; i++){
        if(msg->mItems[i].mType == TypeString){
            mag_free(msg->mItems[i].u.stringValue);
        }
    }
    
    Mag_AcquireMutex(hLooper->mLock);
    list_add(&pEvent->node, &hLooper->mFreeEvtQueue);
    hLooper->mFreeNodeNum++;
    hLooper->mEventInExecuting = MAG_FALSE;
    AGILE_LOGV("[0x%x]free node number: %d", hLooper, hLooper->mFreeNodeNum);
    Mag_ReleaseMutex(hLooper->mLock);
}

static MagLooperEvent_t *getNoDelayMessage(MagLooperHandle hLooper){
    List_t *currentNode;
    MagLooperEvent_t *pEvent = NULL;
    
    currentNode = hLooper->mNoDelayEvtQueue.next;

    if(currentNode == &hLooper->mNoDelayEvtQueue){
        AGILE_LOGV("mNoDelayEvtQueue is empty!");
        return NULL;
    }else{
        if (hLooper->mMergeSameTypeMsg == MAG_FALSE){
            list_del(currentNode);
            pEvent = (MagLooperEvent_t *)list_entry(currentNode, MagLooperEvent_t, node);  
            if (NULL == pEvent)
                AGILE_LOGV("pEvent is NULL!");
        }else{
            ui32 msg_key;
            List_t *nextNode;
            MagLooperEvent_t *tmpEvent = NULL;

            pEvent = (MagLooperEvent_t *)list_entry(currentNode, MagLooperEvent_t, node);
            msg_key = pEvent->mMessage->mWhat;
            nextNode = currentNode->next;

            while(nextNode != &hLooper->mNoDelayEvtQueue){
                tmpEvent = (MagLooperEvent_t *)list_entry(nextNode, MagLooperEvent_t, node);
                if (tmpEvent->mMessage->mWhat == msg_key){
                    list_del(currentNode);
                    currentNode = nextNode;
                    pEvent = tmpEvent;
                }
                nextNode = nextNode->next;
            }
            list_del(currentNode);
        }
    }
    return pEvent;
}

static MagHandlerHandle getHandler(MagLooperHandle hLooper, ui32 id){
    return (MagHandlerHandle)rbtree_get(hLooper->mHandlerTreeRoot, id);
}

static boolean deliverMessage(MagLooperHandle hLooper, MagLooperEvent_t *evt){
    MagHandlerHandle h;
    MagMessageHandle msg;
    boolean ret = MAG_FALSE;

    AGILE_LOGV("enter!");
    if (NULL == evt){
        AGILE_LOGE("evt is NULL!");
        return MAG_FALSE;
    }
    
    msg = evt->mMessage;
    if (NULL == msg){
        AGILE_LOGE("evt[0x%x]'s message is NULL. Should not be here!!", evt);
        mag_free(evt);
        return MAG_FALSE;
    }
    
    h = getHandler(hLooper, msg->mTarget);

    if (NULL == h){
        AGILE_LOGE("failed to get Message handler with id(%u)", msg->mTarget);
    }else{
        h->mMsgProcessor(msg, h->priv);
        ret = MAG_TRUE;
    }

    putFreeEvent(hLooper, evt);
    return ret;
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
    List_t *tmpNode;

    Mag_AcquireMutex(hLooper->mLock);
    while (hLooper->mDelayEvtTreeRoot){
        rbtree_getMinValue(hLooper->mDelayEvtTreeRoot, &key, &value);
        rbtree_delete(&hLooper->mDelayEvtTreeRoot, key);
        evt = (MagLooperEvent_t *)value;
        list_add(&evt->node, &hLooper->mFreeEvtQueue);
    }
    hLooper->mDelayEvtWhenUS = 0;
    
    tmpNode = hLooper->mNoDelayEvtQueue.next;
    while (tmpNode != &hLooper->mNoDelayEvtQueue){
        list_del(tmpNode);
        evt = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        list_add(&evt->node, &hLooper->mFreeEvtQueue);
        tmpNode = hLooper->mNoDelayEvtQueue.next;
    }
    Mag_ReleaseMutex(hLooper->mLock);
}

/*run once before the Looper thread start to be looping*/
static boolean LooperThreadReadyToRun(void *priv){
    return MAG_TRUE;
}

#define DELAY_TIME_DIFF_TOLARANCE 100 /*+-100us*/
static boolean LooperThreadEntry(void *priv){
    MagLooperHandle looper = (MagLooperHandle)priv;
    MagLooperEvent_t *msg = NULL;
    
    if (NULL == priv)
        return MAG_FALSE;

    if (MagEventQueueEmpty(looper)){
        AGILE_LOGV("[0x%x] before waiting[evt grp: 0x%x]",looper, looper->mMQPushEvtGroup);
        Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGV("[0x%x] after waiting[evt grp: 0x%x]",looper, looper->mMQPushEvtGroup);
    }

    Mag_AcquireMutex(looper->mLock);
    if (NULL != looper->mDelayEvtTreeRoot){
        i64 nowUS;
        void *value;
        i32 tdiff;

        rbtree_getMinValue(looper->mDelayEvtTreeRoot, &looper->mDelayEvtWhenUS, &value);
        nowUS = getNowUS();
        /*AGILE_LOGV("min value: %lld", looper->mDelayEvtWhenUS);*/
        tdiff = (i32)(looper->mDelayEvtWhenUS - nowUS);
        if (tdiff > DELAY_TIME_DIFF_TOLARANCE){
            msg = getNoDelayMessage(looper);
            if (msg)
                looper->mEventInExecuting = MAG_TRUE;

            Mag_ReleaseMutex(looper->mLock);
            if (NULL == msg){
                #if 0
                if (tdiff == 1){
                    /*AGILE_LOGD("wait on event, timeout=%d", tdiff);*/
                    Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, (i32)(looper->mDelayEvtWhenMS - nowMS));
                }
                #endif
                return MAG_TRUE;
            }else{
                deliverMessage(looper, msg);
            }
        }else{
            MagLooperEvent_t *evt;

            /*debug only*/
            if (tdiff < (0 - DELAY_TIME_DIFF_TOLARANCE)){
                AGILE_LOGE("the delay time passed(%d us)", tdiff);
            }

            evt = (MagLooperEvent_t *)value;
            rbtree_delete(&looper->mDelayEvtTreeRoot, looper->mDelayEvtWhenUS);
            if (evt)
                looper->mEventInExecuting = MAG_TRUE;
            Mag_ReleaseMutex(looper->mLock);
            
            deliverMessage(looper, evt);
        }
    }else{
        msg = getNoDelayMessage(looper);
        if (msg)
            looper->mEventInExecuting = MAG_TRUE;
        Mag_ReleaseMutex(looper->mLock);

        if (NULL != msg){
            deliverMessage(looper, msg);
        }else{
            AGILE_LOGD("[0x%x]No Delay Message Queue is Empty!", looper);
        }
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
        Mag_DestroyThread(&hLooper->mLooperThread);
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

static _status_t MagLooper_suspend(MagLooperHandle hLooper){
    if (hLooper->mLooperThread == NULL)
        return MAG_INVALID_OPERATION;

    return hLooper->mLooperThread->suspend(hLooper->mLooperThread);
}

static _status_t MagLooper_resume(MagLooperHandle hLooper){
    if (hLooper->mLooperThread == NULL)
        return MAG_INVALID_OPERATION;

    return hLooper->mLooperThread->resume(hLooper->mLooperThread);
}

static void MagLooper_clear(MagLooperHandle hLooper){
    clearMessageQueue(hLooper);
    
}

static void internal_copyMessage(MagMessage_t *to, MagMessage_t *from){
    ui32 i;
    
    memcpy(to, from, sizeof(MagMessage_t));
    for (i = 0; i < from->mNumItems; i++){
        if(from->mItems[i].mType == TypeString){
            to->mItems[i].u.stringValue = mag_strdup(from->mItems[i].u.stringValue);
        }
    }
}

static void MagLooper_postMessage(MagLooperHandle hLooper, MagMessage_t *msg, i64 delayUS){
    MagLooperEvent_t *evt;
    boolean postEvt = MAG_FALSE;

    Mag_AcquireMutex(hLooper->mLock);
    
    evt = getFreeEvent(hLooper);
    if (evt != NULL){
        internal_copyMessage(evt->mMessage, msg);
#if 1
        if ((hLooper->mDelayEvtTreeRoot) || (delayUS > 0)){
            /*if there are delayed messages added, all no-delay messages become the delayed messages*/
            /*if (delayUS == 0){
                delayUS = 1;
            }*/
            evt->mWhenUS = getNowUS() + delayUS;
            if (hLooper->mDelayEvtTreeRoot == NULL)
                postEvt = MAG_TRUE;
            
            if (evt->mWhenUS < hLooper->mDelayEvtWhenUS)
                postEvt = MAG_TRUE;

            hLooper->mDelayEvtTreeRoot = rbtree_insert(hLooper->mDelayEvtTreeRoot, evt->mWhenUS, (void *)evt);
        }else{
            evt->mWhenUS = 0;
            /*only take care of the NoDelay Message Queue Empty status*/
            if (&hLooper->mNoDelayEvtQueue == hLooper->mNoDelayEvtQueue.next)
                postEvt = MAG_TRUE;
            list_add_tail(&evt->node, &hLooper->mNoDelayEvtQueue);
            AGILE_LOGV("[0x%x]add msg(what=%d) to NoDelayQueue[postEvt=%d]", hLooper, msg->mWhat, postEvt);
        }
#else
        if (delayMs > 0){
            evt->mWhenUS = getNowMs() + delayMs;
            if (hLooper->mDelayEvtTreeRoot == NULL)
                postEvt = MAG_TRUE;
            
            if (evt->mWhenUS < hLooper->mDelayEvtWhenUS)
                postEvt = MAG_TRUE;

            hLooper->mDelayEvtTreeRoot = rbtree_insert(hLooper->mDelayEvtTreeRoot, evt->mWhenUS, (void *)evt);
        }else{
            evt->mWhenUS = 0;
            /*only take care of the NoDelay Message Queue Empty status*/
            if (&hLooper->mNoDelayEvtQueue == hLooper->mNoDelayEvtQueue.next)
                postEvt = MAG_TRUE;
            list_add_tail(&evt->node, &hLooper->mNoDelayEvtQueue);
            AGILE_LOGV("[0x%x]add msg(what=%d) to NoDelayQueue[postEvt=%d]", hLooper, msg->mWhat, postEvt);
        }
#endif
    }

    if (postEvt){
        Mag_SetEvent(hLooper->mMQEmptyPushEvt);
        AGILE_LOGV("[0x%x]: set event = 0x%x", hLooper, hLooper->mMQEmptyPushEvt);
    }
    
    Mag_ReleaseMutex(hLooper->mLock);
    /*AGILE_LOGV("post msg: target %d(postEvt:%d)", msg->mTarget, postEvt);*/
}

static ui32 MagLooper_getHandlerID(MagLooperHandle hLooper){
    return ++hLooper->mHandlerID;
}

static _status_t MagLooper_waitOnAllDone(MagLooperHandle hLooper){
    while(!(MagEventQueueEmpty(hLooper) && !hLooper->mEventInExecuting)){
        usleep(4000);
    }
    return MAG_NO_ERROR;
}

static void MagLooper_setMergeMsg(MagLooperHandle hLooper){
    /*if it is set, all same type of the messages would be merged to the latest one in the queue for executing*/
    hLooper->mMergeSameTypeMsg = MAG_TRUE;
}

static void MagLooper_setPriority(struct MagLooper *self, MagLooperPriority_t pri){
    if (MagLooper_Priority_High == pri)
        self->mLooperThread->setParm_Priority(self->mLooperThread, MAGTHREAD_PRIORITY_HIGH);
    else if (MagLooper_Priority_Low == pri)
        self->mLooperThread->setParm_Priority(self->mLooperThread, MAGTHREAD_PRIORITY_LOW);
    else
        self->mLooperThread->setParm_Priority(self->mLooperThread, MAGTHREAD_PRIORITY_NORMAL);
}

MagLooperHandle createLooper(const char *pName){
    MagLooperHandle pLooper;
    char threadName[64];
    ui32 i;
    MagLooperEvent_t *pEvent;

    pLooper = (MagLooperHandle)mag_mallocz(sizeof(MagLooper_t));
    if (NULL != pLooper){
        ++gLooperId;
        pLooper->mpName             = (ui8 *)mag_strdup(pName);
        sprintf(threadName, "MLooper%s_%d", pName, gLooperId);
        
        pLooper->mHandlerTreeRoot   = NULL;
        pLooper->mDelayEvtTreeRoot  = NULL;
        pLooper->mEventInExecuting  = MAG_FALSE;
        pLooper->mMergeSameTypeMsg  = MAG_FALSE;

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

        AGILE_LOGD("[0x%x]event: 0x%x, event grp: 0x%x", pLooper, pLooper->mMQEmptyPushEvt, pLooper->mMQPushEvtGroup);
        
        pLooper->mLooperID = gLooperId;
        
        pLooper->registerHandler   = MagLooper_registerHandler;
        pLooper->unregisterHandler = MagLooper_unregisterHandler;
        pLooper->start             = MagLooper_start;
        pLooper->stop              = MagLooper_stop;
        pLooper->suspend           = MagLooper_suspend;
        pLooper->resume            = MagLooper_resume;
        pLooper->postMessage       = MagLooper_postMessage;
        pLooper->getHandlerID      = MagLooper_getHandlerID;
        pLooper->waitOnAllDone     = MagLooper_waitOnAllDone;
        pLooper->clear             = MagLooper_clear;
        pLooper->setMergeMsg       = MagLooper_setMergeMsg;
        pLooper->setPriority       = MagLooper_setPriority;

        for (i = 0; i < NUM_PRE_ALLOCATED_EVENTS; i++){
            pEvent = mag_mallocz(sizeof(MagLooperEvent_t));
            if (NULL != pEvent){
                INIT_LIST(&pEvent->node);
                pEvent->mMessage = (struct mag_message *)mag_mallocz(sizeof(struct mag_message));
                list_add(&pEvent->node, &pLooper->mFreeEvtQueue);
                pLooper->mFreeNodeNum++;
            }
        }
    }else{
        AGILE_LOGE("failed to malloc MagLooper_t");
    }
    return pLooper;
}

void destroyLooper(MagLooperHandle *phLooper){
    List_t *tmpNode;
    MagLooperEvent_t *evt;
    MagLooperHandle hLooper = *phLooper;

    hLooper->stop(hLooper);

    Mag_DestroyThread(&hLooper->mLooperThread);
    
    Mag_DestroyMutex(&hLooper->mLock);

    Mag_DestroyEvent(&hLooper->mMQEmptyPushEvt);
    Mag_DestroyEventGroup(&hLooper->mMQPushEvtGroup);
    
    tmpNode = hLooper->mFreeEvtQueue.next;
    while (tmpNode != &hLooper->mFreeEvtQueue){
        list_del(tmpNode);
        evt = (MagLooperEvent_t *)list_entry(tmpNode, MagLooperEvent_t, node);   
        if (evt->mMessage)
            mag_free(evt->mMessage);
        mag_free(evt);
        tmpNode = hLooper->mFreeEvtQueue.next;
    }
    mag_freep((void **)phLooper);
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

void destroyHandler(MagHandlerHandle *phHandler){
    mag_freep((void **)phHandler);
}

