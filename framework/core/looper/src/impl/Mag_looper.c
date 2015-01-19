#include <unistd.h>
#include "Mag_looper.h"

/*Debug the memory usage*/
#include <sys/sysinfo.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Looper"


static ui32 gLooperId = 0;

static i64 getNowUS(MagLooperHandle hLooper) {
    if (hLooper->mTimer){
        return hLooper->mTimer->get(hLooper->mTimer);
    }
    return Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
}

static MagLooperEvent_t *getFreeEvent(MagLooperHandle hLooper){
    List_t *tmpNode;
    MagLooperEvent_t *pEvent = NULL;
    
    tmpNode = hLooper->mFreeEvtQueue.next;

    if(tmpNode == &hLooper->mFreeEvtQueue){
        AGILE_LOGV("[%s][0x%x][free nodes = %d]no nodes in the free list. malloc one", 
                    hLooper->mpName, hLooper, hLooper->mFreeNodeNum);
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
    AGILE_LOGV("[%s][0x%x]free node number: %d", hLooper->mpName, hLooper, hLooper->mFreeNodeNum);
    Mag_ReleaseMutex(hLooper->mLock);
}

static MagLooperEvent_t *getNoDelayMessage(MagLooperHandle hLooper){
    List_t *currentNode;
    MagLooperEvent_t *pEvent = NULL;
    
    currentNode = hLooper->mNoDelayEvtQueue.next;

    if(currentNode == &hLooper->mNoDelayEvtQueue){
        /*AGILE_LOGV("[%s] mNoDelayEvtQueue is empty!", hLooper->mpName);*/
        return NULL;
    }else{
        if (hLooper->mMergeSameTypeMsg == MAG_FALSE){
            list_del(currentNode);
            pEvent = (MagLooperEvent_t *)list_entry(currentNode, MagLooperEvent_t, node);  
            if (NULL == pEvent)
                AGILE_LOGV("[%s] pEvent is NULL!", hLooper->mpName);
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

    if (NULL == evt){
        AGILE_LOGE("[%s] evt is NULL!", hLooper->mpName);
        return MAG_FALSE;
    }
    
    msg = evt->mMessage;
    if (NULL == msg){
        AGILE_LOGE("[%s] evt[0x%x]'s message is NULL. Should not be here!!", hLooper->mpName, evt);
        mag_free(evt);
        return MAG_FALSE;
    }
    
    h = getHandler(hLooper, msg->mTarget);

    if (NULL == h){
        AGILE_LOGE("[%s] failed to get Message handler with id(%u)", hLooper->mpName, msg->mTarget);
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

#define DELAY_TIME_DIFF_TOLARANCE 1000 /*+-1000us*/
static boolean LooperThreadEntry(void *priv){
    MagLooperHandle looper = (MagLooperHandle)priv;
    MagLooperEvent_t *msg = NULL;
    struct sched_param param;
    int policy;

    if (NULL == priv)
        return MAG_FALSE;

    if (!looper->mReportPolicy){
        pthread_getschedparam(pthread_self(), &policy, &param) ;
   
        if (policy == SCHED_OTHER) 
            AGILE_LOGE("[%s]: SCHED_OTHER@%d\n", looper->mpName, param.sched_priority); 
        if (policy == SCHED_RR) 
            AGILE_LOGE("[%s]: SCHED_RR@%d\n", looper->mpName, param.sched_priority); 
        if (policy == SCHED_FIFO) 
            AGILE_LOGE("[%s]: SCHED_FIFO@%d\n", looper->mpName, param.sched_priority); 

        looper->mReportPolicy = MAG_TRUE;
    }

retry:
    if (!looper->mRunning){
        return MAG_FALSE;
    }

    Mag_AcquireMutex(looper->mLock);

    /*if (MagEventQueueEmpty(looper)){*/
    if ((NULL == looper->mDelayEvtTreeRoot) && (&looper->mNoDelayEvtQueue == looper->mNoDelayEvtQueue.next)){
        AGILE_LOGV("[%s][0x%x] before waiting[evt grp: 0x%x]", looper->mpName, looper, looper->mMQPushEvtGroup);
        Mag_ClearEvent(looper->mMQEmptyPushEvt);
        Mag_ReleaseMutex(looper->mLock);

        Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        AGILE_LOGV("[%s][0x%x] after waiting[evt grp: 0x%x]", looper->mpName, looper, looper->mMQPushEvtGroup);
        
        if (!looper->mRunning){
            return MAG_FALSE;
        }
        Mag_AcquireMutex(looper->mLock);
    }

    if (NULL != looper->mDelayEvtTreeRoot){
        i64 nowUS;
        void *value;
        i32 tdiff;

        rbtree_getMinValue(looper->mDelayEvtTreeRoot, &looper->mDelayEvtWhenUS, &value);
        if (!looper->mForceOut){
            if (looper->mDelayEvtWhenUS != MAG_MAX_INTEGER){
                nowUS = getNowUS(looper);
                /*AGILE_LOGV("min value: %lld", looper->mDelayEvtWhenUS);*/
                tdiff = (i32)(looper->mDelayEvtWhenUS - nowUS);
            }else{
                AGILE_LOGD("[%s] get the message with delay of MAG_MAX_INTEGER", looper->mpName);
                tdiff = 0;
            }

            if (tdiff > DELAY_TIME_DIFF_TOLARANCE){
                msg = getNoDelayMessage(looper);
                if (msg)
                    looper->mEventInExecuting = MAG_TRUE;

                if (NULL == msg){
                    Mag_ClearEvent(looper->mMQEmptyPushEvt);
                    Mag_ReleaseMutex(looper->mLock);

                    AGILE_LOGD("[%s] before waiting on long delay[delay: %d]", looper->mpName, tdiff);
                    Mag_WaitForEventGroup(looper->mMQPushEvtGroup, MAG_EG_OR, tdiff);
                    AGILE_LOGD("[%s] after waiting on long delay[delay: %d]", looper->mpName, tdiff);
                    goto retry;
                }else{
                    Mag_ReleaseMutex(looper->mLock);

                    deliverMessage(looper, msg);
                }
            }else{
                /*debug only*/
                if (tdiff < (0 - DELAY_TIME_DIFF_TOLARANCE)){
                    AGILE_LOGE("[%s] the msg [%lld] delay time passed(%d us)", looper->mpName, looper->mDelayEvtWhenUS, tdiff);
                }

                msg = (MagLooperEvent_t *)value;
                rbtree_delete(&looper->mDelayEvtTreeRoot, looper->mDelayEvtWhenUS);
                if (msg)
                    looper->mEventInExecuting = MAG_TRUE;
                Mag_ReleaseMutex(looper->mLock);
                
                deliverMessage(looper, msg);
            }
        }else{
            AGILE_LOGD("[%s][%p] forces the delay[%lld us] message out", looper->mpName, looper, looper->mDelayEvtWhenUS);
            msg = (MagLooperEvent_t *)value;
            rbtree_delete(&looper->mDelayEvtTreeRoot, looper->mDelayEvtWhenUS);

            /*the delay message queue is empty now, to reset the mForceOut flag*/
            if (looper->mDelayEvtTreeRoot == NULL){
                AGILE_LOGD("[%s][0x%x]the delay message queue is empty now, to reset the mForceOut flag!", looper->mpName, looper);
                looper->mForceOut = MAG_FALSE;
            }

            if (msg)
                looper->mEventInExecuting = MAG_TRUE;
            Mag_ReleaseMutex(looper->mLock);
            
            deliverMessage(looper, msg);
        }
    }else{
        msg = getNoDelayMessage(looper);
        if (NULL != msg){
            looper->mEventInExecuting = MAG_TRUE;
            AGILE_LOGV("[%s][0x%x]get the message: %p for proceeding", looper->mpName, looper, msg->mMessage);
        }
        Mag_ReleaseMutex(looper->mLock);
        
        if (NULL != msg){
            deliverMessage(looper, msg);
        }else{
            AGILE_LOGD("[%s][0x%x]No Delay Message Queue is Empty!", looper->mpName, looper);
        }
    }
    return MAG_TRUE;
}

static void MagLooper_registerHandler(MagLooperHandle hLooper, const MagHandler_t *handler){
    hLooper->mHandlerTreeRoot = rbtree_insert(hLooper->mHandlerTreeRoot, handler->uiID, (void *)handler);
}

static _status_t MagLooper_unregisterHandler(MagLooperHandle hLooper, i32 handlerID){
    if (rbtree_delete(&hLooper->mHandlerTreeRoot, handlerID) != 0){
        AGILE_LOGE("[%s] failed to unregister the handler ID: %d", hLooper->mpName, handlerID);
        return MAG_NAME_NOT_FOUND;
    }
    return MAG_NO_ERROR;
}

static _status_t MagLooper_start(MagLooperHandle hLooper){
    _status_t ret;

    if (hLooper->mLooperThread == NULL){
        return MAG_BAD_VALUE;
    }

    hLooper->mForceOut = MAG_FALSE;
    hLooper->mRunning  = MAG_TRUE;
    ret = hLooper->mLooperThread->run(hLooper->mLooperThread);
    if (ret != MAG_NO_ERROR){
        AGILE_LOGE("[%s] failed to run Looper!, ret = 0x%x", hLooper->mpName, ret);
        Mag_DestroyThread(&hLooper->mLooperThread);
        hLooper->mLooperThread = NULL;
    } 
    
    return ret;
}

static _status_t MagLooper_stop(MagLooperHandle hLooper){
    if (hLooper->mLooperThread == NULL)
        return MAG_INVALID_OPERATION;

    hLooper->mRunning  = MAG_FALSE;
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

static void MagLooper_forceOut(MagLooperHandle hLooper){
    hLooper->mForceOut = MAG_TRUE;
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
/*delayUS: -1 force the message to be added into the no delay queue*/
static void MagLooper_postMessage(MagLooperHandle hLooper, MagMessage_t *msg, i64 delayUS){
    MagLooperEvent_t *evt;
    boolean postEvt = MAG_FALSE;

    Mag_AcquireMutex(hLooper->mLock);
    
    evt = getFreeEvent(hLooper);
    if (evt != NULL){
        internal_copyMessage(evt->mMessage, msg);
#if 1
        if ((hLooper->mDelayEvtTreeRoot || (delayUS > 0)) && (delayUS != -1)){
            /*if there are delayed messages added, all no-delay messages become the delayed messages*/
            /*if (delayUS == 0){
                delayUS = 1;
            }*/

            if (delayUS != MAG_MAX_INTEGER)
                evt->mWhenUS = getNowUS(hLooper) + delayUS;
            else
                evt->mWhenUS = MAG_MAX_INTEGER;

            if (hLooper->mDelayEvtTreeRoot == NULL)
                postEvt = MAG_TRUE;
            
            if (evt->mWhenUS < hLooper->mDelayEvtWhenUS)
                postEvt = MAG_TRUE;

            hLooper->mDelayEvtTreeRoot = rbtree_insert(hLooper->mDelayEvtTreeRoot, evt->mWhenUS, (void *)evt);
            AGILE_LOGV("[%s][0x%x]add msg[%p](mWhenUS=%lld) to DelayQueue", 
                        hLooper->mpName, hLooper, msg, evt->mWhenUS);
        }else{
            evt->mWhenUS = 0;
            /*only take care of the NoDelay Message Queue Empty status*/
            if (&hLooper->mNoDelayEvtQueue == hLooper->mNoDelayEvtQueue.next)
                postEvt = MAG_TRUE;
            list_add_tail(&evt->node, &hLooper->mNoDelayEvtQueue);
            AGILE_LOGV("[%s][0x%x]add msg[%p - copy msg:%p](what=%d) to NoDelayQueue[postEvt=%d]", 
                        hLooper->mpName, hLooper, msg, evt->mMessage, msg->mWhat, postEvt);
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
        AGILE_LOGV("[%s][0x%x]: set event = 0x%x", hLooper->mpName, hLooper, hLooper->mMQEmptyPushEvt);
    }
    
    Mag_ReleaseMutex(hLooper->mLock);
    AGILE_LOGV("post msg: target %d(postEvt:%d)", msg->mTarget, postEvt);
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
    else
        self->mLooperThread->setParm_Priority(self->mLooperThread, MAGTHREAD_PRIORITY_NORMAL);
}

static void MagLooper_setTimer(MagLooperHandle hLooper, MagTimerHandle hTimer){
    hLooper->mTimer = hTimer;
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
        pLooper->mTimer             = NULL;
        pLooper->mEventInExecuting  = MAG_FALSE;
        pLooper->mMergeSameTypeMsg  = MAG_FALSE;
        pLooper->mForceOut          = MAG_FALSE;
        pLooper->mReportPolicy      = MAG_FALSE;
        pLooper->mRunning           = MAG_FALSE;

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

        AGILE_LOGD("[%s][0x%x]event: 0x%x, event grp: 0x%x", 
                    pLooper->mpName, pLooper, pLooper->mMQEmptyPushEvt, pLooper->mMQPushEvtGroup);
        
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
        pLooper->forceOut          = MagLooper_forceOut;
        pLooper->setMergeMsg       = MagLooper_setMergeMsg;
        pLooper->setPriority       = MagLooper_setPriority;
        pLooper->setTimer          = MagLooper_setTimer;

        for (i = 0; i < NUM_PRE_ALLOCATED_EVENTS; i++){
            pEvent = (MagLooperEvent_t *)mag_mallocz(sizeof(MagLooperEvent_t));
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

    if (NULL == *phLooper)
        return;

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

    mag_freep((void **)&(*phLooper)->mpName);
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
    if (NULL == *phHandler)
        return;

    mag_freep((void **)phHandler);
}

