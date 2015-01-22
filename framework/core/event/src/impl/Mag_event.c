/*
* 1) if the event is triggered before entering the event group waiting, the Mag_WaitForEventGroup would
*    return immediately.
* 2) if the event is triggered after entering the event group waiting, the Mag_WaitForEventGroup would
*    wait on the signal and return while the signal is triggered.
*/
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include "Mag_event.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Event"

#define MAG_DEBUG

#ifdef MAG_DEBUG
static i32 evtNumTotal = 0;
#endif
/*
* 1) if multiple threads waiting upon the same event, only one thread is woken up in terms of the thread priorities
* 2) if 1 thread is waiting upon the event group and multiple threads signal the events, there is possibility to miss the events due to 
*     some events are reveived at the same time. Do we need the event retrigger logic???
*/

MagErr_t Mag_CreateEvent(MagEventHandle *evtHandle, MAG_EVENT_PRIO_t prio){
    /*i32 rc;*/
    
    *evtHandle = (MagEventHandle)mag_mallocz(sizeof(**evtHandle));
    if(NULL == *evtHandle)
        return MAG_NoMemory;

    (*evtHandle)->pEvtCommon = (Mag_EventCommon_t *)mag_mallocz(sizeof(Mag_EventCommon_t));
    if(NULL == (*evtHandle)->pEvtCommon)
        goto err_nomem;

    (*evtHandle)->pEvtCallBack = (Mag_EventCallback_t *)mag_mallocz(sizeof(Mag_EventCallback_t));
    if(NULL == (*evtHandle)->pEvtCallBack)
        goto err_nomem;
        
    /*rc = pthread_mutex_init(&(*evtHandle)->pEvtCommon->lock, NULL  default attributes );
    if(rc != 0){
        goto err_evt_mutex;
    }*/
    INIT_LIST(&(*evtHandle)->pEvtCommon->entry);
    (*evtHandle)->pEvtCommon->signal      = MAG_FALSE;
    (*evtHandle)->pEvtCommon->hEventGroup = NULL;


    /*rc = pthread_mutex_init(&(*evtHandle)->pEvtCallBack->lock, NULL  default attributes );
    if(rc != 0){
        goto err_cb_mutex;
    }*/

    INIT_LIST(&(*evtHandle)->pEvtCallBack->entry);
    (*evtHandle)->pEvtCallBack->priority      = prio;
    (*evtHandle)->pEvtCallBack->armed         = 0;
    (*evtHandle)->pEvtCallBack->hEvtScheduler = NULL;
    (*evtHandle)->pEvtCallBack->hCallback     = NULL;
    
    return MAG_ErrNone;
    
err_nomem:
    if ((*evtHandle)->pEvtCommon)
        mag_freep((void **)&(*evtHandle)->pEvtCommon);
    
    if (*evtHandle)
        mag_freep((void **)evtHandle);

    return MAG_NoMemory;

/*err_cb_mutex:
    pthread_mutex_destroy(&(*evtHandle)->pEvtCommon->lock);*/
    
/*err_evt_mutex:*/    
    if((*evtHandle)->pEvtCommon)
        mag_freep((void **)&(*evtHandle)->pEvtCommon);

    if((*evtHandle)->pEvtCallBack)
        mag_freep((void **)&(*evtHandle)->pEvtCallBack);

    if (*evtHandle)
        mag_freep((void **)evtHandle);

    return MAG_ErrMutexCreate;
}

MagErr_t Mag_DestroyEvent(MagEventHandle *pEvtHandle){
    MagEventHandle evtHandle = *pEvtHandle;
    MagErr_t ret = MAG_ErrNone;

    if (evtHandle == NULL){
        return MAG_Failure;
    }

    if (evtHandle->pEvtCommon->hEventGroup){
        ret = Mag_RemoveEventGroup(evtHandle->pEvtCommon->hEventGroup, evtHandle);
    }

    /*pthread_mutex_destroy(&evtHandle->pEvtCommon->lock);*/
    mag_freep((void **)&evtHandle->pEvtCommon);

    ret = Mag_UnregisterEventCallback(evtHandle);
    if(evtHandle->pEvtCallBack->hCallback){
        pthread_mutex_destroy(&evtHandle->pEvtCallBack->hCallback->lock);
        mag_freep((void **)&evtHandle->pEvtCallBack->hCallback);
    }
    mag_freep((void **)&evtHandle->pEvtCallBack);
    mag_freep((void **)pEvtHandle);
    return ret;
}

static MagErr_t MagPriv_us2timespec(struct timespec *target, i32 timeoutUsec){
    i32 rc;
    struct timeval now;
    rc = gettimeofday(&now, NULL);
    if (0 != rc)
        return MAG_ErrGetTime;

    target->tv_nsec = now.tv_usec * 1000 + (timeoutUsec%1000000) * 1000;
    target->tv_sec = now.tv_sec + (timeoutUsec/1000000);
    if (target->tv_nsec >= 1000000000) {
        target->tv_nsec -=  1000000000;
        target->tv_sec ++;
    }
    return MAG_ErrNone;
}


MagErr_t  Mag_SetEvent(MagEventHandle evtHandle){
    i32 rc;
    MagEventGroupHandle evtGrp;
    Mag_EventCallback_t *pEvtCB;
    MagErr_t ret = MAG_ErrNone;
    struct timespec now;
    Mag_EvtCbTimeStamp_t *evtTSHandle;

    if (NULL == evtHandle)
        return MAG_BadParameter;
        
    evtGrp = evtHandle->pEvtCommon->hEventGroup;
    pEvtCB  = evtHandle->pEvtCallBack;
    
    if(NULL != evtGrp){
        rc = pthread_mutex_lock(&evtGrp->lock);
        MAG_ASSERT(0 == rc);
        
        evtHandle->pEvtCommon->signal = MAG_TRUE;
        
        rc = pthread_mutex_unlock(&evtGrp->lock);
        MAG_ASSERT(0 == rc);

        rc = pthread_cond_signal(&evtGrp->cond);
        MAG_ASSERT(0 == rc);

        AGILE_LOGV("set event [0x%p], evtGrp [0x%p]", evtHandle, evtGrp);
    }else{
        AGILE_LOGV("The event element(0x%lx) is not Added into the event group!", (unsigned long)evtHandle);
    }

    if ((NULL != pEvtCB->hCallback) && (NULL != pEvtCB->hEvtScheduler)){
        rc = pthread_mutex_lock(&pEvtCB->hEvtScheduler->lock);
        MAG_ASSERT(0 == rc);

        /*link all priority default node as the list with trigger time ascending (example: 10ms -> 20ms -> 35ms ....)*/
        rc = clock_gettime(CLOCK_MONOTONIC, &now);
        MAG_ASSERT(0 == rc);
        
        if (MAG_EVT_PRIO_DEFAULT == pEvtCB->priority){
            if(pEvtCB->hEvtScheduler->cbTimeStampFreeListH.next != &pEvtCB->hEvtScheduler->cbTimeStampFreeListH){
                evtTSHandle = (Mag_EvtCbTimeStamp_t *)list_entry(pEvtCB->hEvtScheduler->cbTimeStampFreeListH.next, 
                                                                 Mag_EvtCbTimeStamp_t, tsListNode);
                list_del(pEvtCB->hEvtScheduler->cbTimeStampFreeListH.next);
            }else{
                evtTSHandle = (Mag_EvtCbTimeStamp_t *)mag_mallocz(sizeof(Mag_EvtCbTimeStamp_t));
            }
            evtTSHandle->cbBody = pEvtCB->hCallback;
            evtTSHandle->timeStamp.tv_sec = now.tv_sec;
            evtTSHandle->timeStamp.tv_nsec = now.tv_nsec;

            if (!is_added_list(&pEvtCB->hEvtScheduler->cbTimeStampListH)){
                evtTSHandle->timeDiff = 0;
            }else{
                Mag_EvtCbTimeStamp_t *prev;
                prev = (Mag_EvtCbTimeStamp_t *)list_entry(pEvtCB->hEvtScheduler->cbTimeStampListH.prev, Mag_EvtCbTimeStamp_t, tsListNode);
                evtTSHandle->timeDiff = (now.tv_sec - prev->timeStamp.tv_sec)*1000000 + (now.tv_nsec - prev->timeStamp.tv_nsec)/1000;/*//in us*/
            }
            AGILE_LOGV("add default priority event[0x%p] into the TimeStamp list: tv_sec = %d, tv_nsec = %ld, timeDiff = %d",
                        evtHandle, evtTSHandle->timeStamp.tv_sec, 
                        evtTSHandle->timeStamp.tv_nsec, evtTSHandle->timeDiff);
            list_add_tail(&evtTSHandle->tsListNode, &pEvtCB->hEvtScheduler->cbTimeStampListH);
        }else{
            pEvtCB->armed++;
        }

        pEvtCB->hEvtScheduler->bSignal = MAG_TRUE;

        rc = pthread_mutex_unlock(&pEvtCB->hEvtScheduler->lock);
        MAG_ASSERT(0 == rc);

        rc = pthread_cond_signal(&pEvtCB->hEvtScheduler->cond);
        MAG_ASSERT(0 == rc);
    }else{
        /*AGILE_LOGV("The event element(0x%lx) has no callback registered!", (unsigned long)evtHandle);*/
    }

    return ret;
}

void     Mag_ClearEvent(MagEventHandle evtHandle){
    i32 rc;
    MagEventGroupHandle evtGrp;

    if (NULL == evtHandle)
        return;
        
    evtGrp = evtHandle->pEvtCommon->hEventGroup;

    if(NULL != evtGrp){
        rc = pthread_mutex_lock(&evtGrp->lock);
        MAG_ASSERT(0 == rc);
        
        evtHandle->pEvtCommon->signal = MAG_FALSE;
        
        rc = pthread_mutex_unlock(&evtGrp->lock);
        MAG_ASSERT(0 == rc);
    }else{
        evtHandle->pEvtCommon->signal = MAG_FALSE;
        AGILE_LOGV("The event element(0x%lx) is not Added into the event group!", (unsigned long)evtHandle);
    }
}

MagErr_t Mag_RegisterEventCallback(MagEventSchedulerHandle schedHandle, MagEventHandle evtHandle, void (*pCallback)(void *), void *pContext){
    MagErr_t ret = MAG_ErrNone;
    i32 rc;
    Mag_EventCallback_t *pEvtCB = evtHandle->pEvtCallBack;

    if (NULL == evtHandle){
        AGILE_LOGE("the evtHandle is NULL, quit!!");
        return MAG_BadParameter;
    }

    if (NULL == schedHandle){
        AGILE_LOGE("the schedHandle is NULL, quit!!");
        return MAG_BadParameter;
    }
    
    rc = pthread_mutex_lock(&schedHandle->lock);
    MAG_ASSERT(0 == rc);

    if (NULL == evtHandle->pEvtCallBack->hCallback){
        evtHandle->pEvtCallBack->hCallback = (MagEventCallbackHandle)mag_mallocz(sizeof(MagEventCallbackObj_t));
        if(NULL == evtHandle->pEvtCallBack->hCallback)
            return MAG_NoMemory;
    }

    pEvtCB  = evtHandle->pEvtCallBack;

    INIT_LIST(&pEvtCB->hCallback->exeEntry);
    pEvtCB->hCallback->exeNum    = 0;
    pthread_mutex_init(&pEvtCB->hCallback->lock, NULL);
    MAG_ASSERT(0 == rc);
    pEvtCB->hCallback->pCallback = pCallback;
    pEvtCB->hCallback->pContext  = pContext;
    pEvtCB->hEvtScheduler        = schedHandle;

    list_add_tail(&pEvtCB->entry, &schedHandle->listHead[pEvtCB->priority]);

    rc = pthread_mutex_unlock(&schedHandle->lock);
    MAG_ASSERT(0 == rc);

    return ret;
}

MagErr_t Mag_UnregisterEventCallback(MagEventHandle evtHandle){
    MagErr_t ret = MAG_ErrNone;
    i32 rc;
    Mag_EventCallback_t *pEvtCB = evtHandle->pEvtCallBack;

    if(NULL == pEvtCB){
        AGILE_LOGE("the event(0x%lx) has never registered the callback", (uintptr_t)evtHandle);
        return MAG_BadParameter;
    }

    if(NULL == pEvtCB->hEvtScheduler){
        AGILE_LOGE("the event(0x%lx) was not added into the event scheduler", (uintptr_t)evtHandle);
        return MAG_Failure;
    }

    rc = pthread_mutex_lock(&pEvtCB->hEvtScheduler->lock);
    MAG_ASSERT(0 == rc);

    rc = pthread_mutex_lock(&pEvtCB->hCallback->lock);
    MAG_ASSERT(0 == rc);

    pEvtCB->hCallback->pCallback = NULL;
    pEvtCB->hCallback->pContext  = NULL;
    pEvtCB->hCallback->exeNum    = 0;
    list_del(&pEvtCB->hCallback->exeEntry);
    list_del(&pEvtCB->entry);

    rc = pthread_mutex_unlock(&pEvtCB->hCallback->lock);
    MAG_ASSERT(0 == rc);

    rc = pthread_mutex_unlock(&pEvtCB->hEvtScheduler->lock);
    MAG_ASSERT(0 == rc);

    return ret;
}


MagErr_t Mag_CreateEventGroup(MagEventGroupHandle *evtGrphandle){
    i32 rc;
    /*pthread_condattr_t attr;
    pthread_condattr_t *pAttr = NULL;*/

    *evtGrphandle = (MagEventGroupHandle)mag_mallocz(sizeof(**evtGrphandle));
    if (NULL == *evtGrphandle)
        return MAG_NoMemory;

    rc = pthread_mutex_init(&(*evtGrphandle)->lock, NULL /* default attributes */);
    if(rc != 0){
        goto err_mutex;
    }

    /*rc = pthread_condattr_init(&attr);
    if(rc != 0){
        AGILE_LOGE("failed to initialize the pthread_condattr");
    }else{
        rc = pthread_condattr_setclock(&attr, CLOCK_REALTIME);
        if (rc != 0){
            AGILE_LOGE("failed to setclock as CLOCK_REALTIME");
        }else{
            pAttr = &attr;
        }
    }*/

    rc = pthread_cond_init(&(*evtGrphandle)->cond, NULL);
    if(rc != 0){
        goto err_cond;
    }

    /*if (pAttr)
        pthread_condattr_destroy(pAttr);*/

    INIT_LIST(&(*evtGrphandle)->EventGroupHead);
    (*evtGrphandle)->eventNum = 0;

#ifdef MAG_DEBUG
    evtNumTotal = 0;
#endif
    return MAG_ErrNone;
    
err_cond:
    pthread_mutex_destroy(&(*evtGrphandle)->lock);
    mag_freep((void **)evtGrphandle);
    return MAG_ErrCondCreate;
    
err_mutex:
    mag_freep((void **)evtGrphandle);
    return MAG_ErrMutexCreate;    

}

void Mag_DestroyEventGroup(MagEventGroupHandle *pEvtGrphandle){
    MagEventGroupHandle evtGrphandle = *pEvtGrphandle;
    List_t *tmpNode = NULL;
    MagEventHandle pEvent;

    if (NULL == evtGrphandle){
        return;
    }
    
    tmpNode = evtGrphandle->EventGroupHead.next;

    while (tmpNode != &evtGrphandle->EventGroupHead){
        pEvent = (MagEventHandle)tmpNode->next; /*//list_entry(tmpNode, Mag_EventCommon_t, entry);*/
        Mag_RemoveEventGroup(evtGrphandle, pEvent);
        tmpNode = tmpNode->next;
    }

    pthread_mutex_destroy(&evtGrphandle->lock);
    pthread_cond_destroy(&evtGrphandle->cond);
    mag_freep((void **)pEvtGrphandle);
}

MagErr_t Mag_AddEventGroup(MagEventGroupHandle evtGrphandle, MagEventHandle event){
    i32 rc;
    MagErr_t ret = MAG_ErrNone;

    if (NULL != event->pEvtCommon->hEventGroup){
        AGILE_LOGD("the event(0x%x) has already been added to EventGroup(0x%x)", (unsigned long)event, (unsigned long)evtGrphandle);
        return MAG_ErrEvtSetOp;
    }

    rc = pthread_mutex_lock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    if ((evtGrphandle->eventNum + 1) > MAX_EVENTS_EG){
        AGILE_LOGE("the event number(%d) exceeds the maxium number in the EventGroup. Failed to add!!", evtGrphandle->eventNum);
        ret = MAG_ErrMaxEventNum;
        goto done;
    }

    event->pEvtCommon->hEventGroup = evtGrphandle;
    list_add(&event->pEvtCommon->entry, &evtGrphandle->EventGroupHead);
    evtGrphandle->eventNum++;

done:    
    rc = pthread_mutex_unlock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    if(event->pEvtCommon->signal == MAG_TRUE){
        /* signal condition if signal already set */
        pthread_cond_signal(&evtGrphandle->cond);
    }

    return ret;
}

MagErr_t Mag_RemoveEventGroup(MagEventGroupHandle evtGrphandle, MagEventHandle event){
    i32 rc;
    MagErr_t ret = MAG_ErrNone;

    if (NULL == event->pEvtCommon->hEventGroup){
        AGILE_LOGD("the event(0x%x) don't have EventGroup", (unsigned long)event);
        return MAG_ErrEvtSetOp;
    }

    if (evtGrphandle != event->pEvtCommon->hEventGroup){
        AGILE_LOGD("the event(0x%x) is not in EventGroup(0x%x)", (unsigned long)event, (unsigned long)evtGrphandle);
        return MAG_BadParameter;
    }

    rc = pthread_mutex_lock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    if (0 >= evtGrphandle->eventNum){
        /*should never be here!*/
        AGILE_LOGD("the event number is wrong(%d) in EventGroup(0x%lx)", evtGrphandle->eventNum, (unsigned long)evtGrphandle);
        ret = MAG_Failure;
        goto done;
    }

    event->pEvtCommon->hEventGroup = NULL;
    list_del(&event->pEvtCommon->entry);
    evtGrphandle->eventNum--;

done:   
    rc = pthread_mutex_unlock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    return ret;
}

static MagErr_t Mag_EventStatusCheck(MagEventGroupHandle evtGrphandle, MAG_EVENT_GROUP_OP_t op){
    List_t *tmpNode = evtGrphandle->EventGroupHead.next;
    Mag_EventCommon_t *pEventCommon;
    MagErr_t ret = MAG_EventStatusErr;
    i32 signalEvtNum = 0;

    while (tmpNode != &evtGrphandle->EventGroupHead){
        pEventCommon = (Mag_EventCommon_t *)list_entry(tmpNode, Mag_EventCommon_t, entry);

        if (MAG_EG_OR == op){ /*event1 OR event2 OR event3 is true*/
            if (pEventCommon->signal == MAG_TRUE)
                signalEvtNum++;

        }else{ /*event1 AND event2 AND event3 is true*/
            if (pEventCommon->signal == MAG_TRUE)
                signalEvtNum++;   
        }

        tmpNode = tmpNode->next;
    }

    if (MAG_EG_OR == op){
        if (signalEvtNum > 0){
#ifdef MAG_DEBUG
            evtNumTotal += signalEvtNum;
            AGILE_LOGD("[OR]triggered event number: %d. Total triggered events: %d", signalEvtNum, evtNumTotal);
#endif
            ret = MAG_EventStatusMeet;
        }
    }else{
        if (signalEvtNum == (i32)evtGrphandle->eventNum){
#ifdef MAG_DEBUG
            evtNumTotal += signalEvtNum;
            AGILE_LOGD("[AND]triggered event number: %d. Total triggered events: %d", signalEvtNum, evtNumTotal);
#endif
            ret = MAG_EventStatusMeet;
        }
    }
    return ret;
}


static MagErr_t Mag_ClearAllEvents(MagEventGroupHandle evtGrphandle){
    List_t *tmpNode = evtGrphandle->EventGroupHead.next;
    Mag_EventCommon_t  *pEventCommon;
    
    while (tmpNode != &evtGrphandle->EventGroupHead){
        pEventCommon = (Mag_EventCommon_t *)list_entry(tmpNode, Mag_EventCommon_t, entry);
        pEventCommon->signal = MAG_FALSE; 
        AGILE_LOGV("clear the signal of pEventCommon[0x%p]", pEventCommon);
        tmpNode = tmpNode->next;
    }
    return MAG_ErrNone;
}


MagErr_t Mag_WaitForEventGroup(MagEventGroupHandle evtGrphandle, MAG_EVENT_GROUP_OP_t op, i32 timeoutUsec){
    struct timespec target;
    MagErr_t ret = MAG_ErrNone;
    i32 rc;

    if ((timeoutUsec < 0) && (timeoutUsec != (i32)MAG_TIMEOUT_INFINITE))
        return MAG_BadParameter;

    if(timeoutUsec != (i32)MAG_TIMEOUT_INFINITE){
        if ((ret = MagPriv_us2timespec(&target, timeoutUsec)) != MAG_ErrNone)
            return ret;
    }
    
    rc = pthread_mutex_lock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    if (0 == timeoutUsec){
        AGILE_LOGD("Timeout!!");
        ret = MAG_TimeOut;
        goto done;
    }

    while(MAG_EventStatusMeet != Mag_EventStatusCheck(evtGrphandle, op)){ /* we might have been wokenup and then event has been cleared */
        if (timeoutUsec == (i32)MAG_TIMEOUT_INFINITE){
            AGILE_LOGV("before pthread_cond_wait(0x%p), tid=%d", evtGrphandle, syscall(SYS_gettid));
            rc = pthread_cond_wait(&evtGrphandle->cond, &evtGrphandle->lock);
            AGILE_LOGV("after pthread_cond_wait(0x%p), tid=%d", evtGrphandle, syscall(SYS_gettid));
        }else{
            AGILE_LOGV("before pthread_cond_timedwait(%d us)", timeoutUsec);
            rc = pthread_cond_timedwait(&evtGrphandle->cond, &evtGrphandle->lock, &target);
            if(ETIMEDOUT == rc){
                ret = MAG_TimeOut;
                AGILE_LOGV("WaitfForEventSet(evtSet: 0x%lx): timeout [%d us]", (unsigned long)evtGrphandle, timeoutUsec);
                goto done;
            }
        }

        /*the blocking system call: futex might be interrupted by OS signal.*/
        if (EINTR == rc){
            AGILE_LOGW("WaitfForEventSet(evtSet: 0x%lx): interrupted by system signal", (unsigned long)evtGrphandle);
            continue;
        }

        MAG_ASSERT(0 == rc);
    }
    Mag_ClearAllEvents(evtGrphandle);

done:
    rc = pthread_mutex_unlock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);
    return ret;
}

static MagErr_t getCallbackExeList(MagEventSchedulerHandle schedHandle, List_t *listHead, int *timeout /*ms*/){
    List_t *tmpNode;
    Mag_EventCallback_t *evtCBHanlde;
    ui32 highPrioTotal = 0;
    ui32 defPrioTotal = 0;
    Mag_EvtCbTimeStamp_t *timeStampNode;
    i32 calcTimeOut = 0; /*//in ms*/
    MagErr_t ret = MAG_ErrNone;
    MagEventCallbackHandle cbHandle;

    /* add all priority HIGH callbacks into the list*/
    tmpNode = schedHandle->listHead[MAG_EVT_PRIO_HIGH].next;
    while(tmpNode != &schedHandle->listHead[MAG_EVT_PRIO_HIGH]){
        evtCBHanlde = (Mag_EventCallback_t *)list_entry(tmpNode, Mag_EventCallback_t, entry);
        
        if(evtCBHanlde->armed > 0){
            if (NULL == evtCBHanlde->hCallback){
                AGILE_LOGE("the evtCBHanlde(0x%x) has no callbacks registered, but armed is %d", 
                            (uintptr_t)evtCBHanlde, evtCBHanlde->armed);
                goto H_done;
            }
            
            if (evtCBHanlde->hEvtScheduler != schedHandle){
                AGILE_LOG_FATAL("the evtCBHandle(0x%x)'s evtScheduler(0x%x) is not input schedHandle(0x%x)",
                                 (uintptr_t)evtCBHanlde, (uintptr_t)evtCBHanlde->hEvtScheduler, schedHandle);
                MAG_ASSERT(MAG_FALSE);
            }

            if (MAG_EVT_PRIO_HIGH != evtCBHanlde->priority){
                AGILE_LOGE("the evtCBHanlde(0x%x)'s priority(%d) is not MAG_EVT_PRIO_HIGH. in the wrong list!", 
                            (uintptr_t)evtCBHanlde, evtCBHanlde->priority);
                goto H_done;
            }

            list_add_tail(&evtCBHanlde->hCallback->exeEntry, listHead);
            evtCBHanlde->hCallback->exeNum = evtCBHanlde->armed;
            highPrioTotal += evtCBHanlde->hCallback->exeNum;
            evtCBHanlde->armed = 0;
        }

H_done:
        tmpNode = tmpNode->next;
    }
    AGILE_LOGD("highPrioTotal = %d", highPrioTotal);
    
    /*go through the default prio event list*/
    tmpNode = schedHandle->cbTimeStampListH.next;
    while(tmpNode != &schedHandle->cbTimeStampListH){
        timeStampNode = (Mag_EvtCbTimeStamp_t *)list_entry(tmpNode, Mag_EvtCbTimeStamp_t, tsListNode);
        cbHandle = timeStampNode->cbBody;   

        if (NULL == cbHandle->pCallback){
            AGILE_LOGE("the cbHandle(0x%x)'s callback is NULL", (uintptr_t)cbHandle);
            goto def_done;
        }
        
        /* - if the number of the high priority events are less than 4, all default priority events' callbacks would be executed
         * - if more than 4 high prio events are executed, then only pick up at most 4 default prio events for handling
         */
        defPrioTotal++;
        if ((highPrioTotal <= 4) || (defPrioTotal <= 4)){
            if (is_added_list(&cbHandle->exeEntry)){
                cbHandle->exeNum++;
            }else{
                list_add_tail(&cbHandle->exeEntry, listHead);
                cbHandle->exeNum = 1;
            }
        }else{
            if (timeStampNode->timeDiff < 10000){
                calcTimeOut = 10; /*at least 10ms timeout*/
            }else{
                calcTimeOut = timeStampNode->timeDiff/1000;
            }
            break;
        }
        list_del(tmpNode);
        list_add_tail(tmpNode, &schedHandle->cbTimeStampFreeListH);
        
def_done:
        tmpNode = schedHandle->cbTimeStampListH.next;
    }
    AGILE_LOGD("defPrioTotal = %d, timeout = %d", defPrioTotal, calcTimeOut);
    
    if(calcTimeOut == 0)
        calcTimeOut = MAG_TIMEOUT_INFINITE;

    *timeout = calcTimeOut;
    
    /*only add 1 low priority event to the exe list*/
    tmpNode = schedHandle->listHead[MAG_EVT_PRIO_LOW].next;
    while(tmpNode != &schedHandle->listHead[MAG_EVT_PRIO_LOW]){
        evtCBHanlde = (Mag_EventCallback_t *)list_entry(tmpNode, Mag_EventCallback_t, entry);

        if(evtCBHanlde->armed > 0){
            if (NULL == evtCBHanlde->hCallback){
                AGILE_LOGE("the evtCBHanlde(0x%x) has no callbacks registered, but armed is %d", 
                            (uintptr_t)evtCBHanlde, evtCBHanlde->armed);
                goto L_done;
            }
            
            if (evtCBHanlde->hEvtScheduler != schedHandle){
                AGILE_LOG_FATAL("the evtCBHandle(0x%x)'s evtScheduler(0x%x) is not input schedHandle(0x%x)",
                                 (uintptr_t)evtCBHanlde, (uintptr_t)evtCBHanlde->hEvtScheduler, schedHandle);
                MAG_ASSERT(MAG_FALSE);
            }

            if (MAG_EVT_PRIO_HIGH != evtCBHanlde->priority){
                AGILE_LOGE("the evtCBHanlde(0x%x)'s priority(%d) is not MAG_EVT_PRIO_LOW. In the wrong list!", 
                            (uintptr_t)evtCBHanlde, evtCBHanlde->priority);
                goto L_done;
            }

            list_add_tail(&evtCBHanlde->hCallback->exeEntry, listHead);
            evtCBHanlde->hCallback->exeNum = 1;
            evtCBHanlde->armed--;
            break;
        }
        
L_done:    
        tmpNode = tmpNode->next;
    }
    
    return ret;
}

static void processEventCallbacks(List_t *exeListHead){
    List_t *tmpNode = exeListHead->next;
    MagEventCallbackHandle evtCbHandle;
    ui32 i = 0;
    i32 rc;
    
    while (tmpNode != exeListHead){
        evtCbHandle = (MagEventCallbackHandle)list_entry(tmpNode, MagEventCallbackObj_t, exeEntry);
        AGILE_LOGD("exeNum is %d", evtCbHandle->exeNum);
        
        rc = pthread_mutex_lock(&evtCbHandle->lock);
        MAG_ASSERT(0 == rc);
        
        for (i = 0; i < evtCbHandle->exeNum; i++){
            if (evtCbHandle->pCallback)
                evtCbHandle->pCallback(evtCbHandle->pContext);
            else
                AGILE_LOGD("evtCbHandle->pCallback is NULL");
        }
        evtCbHandle->exeNum = 0;
        list_del(tmpNode);

        rc = pthread_mutex_unlock(&evtCbHandle->lock);
        MAG_ASSERT(0 == rc);
        
        tmpNode = exeListHead->next;
    }
}

static void *Mag_EventScheduleEntry(void *arg){
    MagEventSchedulerHandle schedHandle = (MagEventSchedulerHandle) arg;
    i32 rc;
    List_t cbExeListHead;
    i32 timeout = MAG_TIMEOUT_INFINITE;
    struct timespec target;
    
    for(;;){
        rc = pthread_mutex_lock(&schedHandle->lock);
        if(rc != 0){
            AGILE_LOGE("failed to do pthread_mutex_lock, error(%s)", strerror(errno));
            break;
        }

restart:
        if (schedHandle->bSignal != MAG_TRUE){
            if ((i32)MAG_TIMEOUT_INFINITE == timeout){
                rc = pthread_cond_wait(&schedHandle->cond, &schedHandle->lock);
            }else{
                MagPriv_us2timespec(&target, timeout);
                rc = pthread_cond_timedwait(&schedHandle->cond, &schedHandle->lock, &target);
            }
        }

        if (schedHandle->sTExited){
            pthread_mutex_unlock(&schedHandle->lock);
            break;
        }   
        /*the blocking system call: futex might be interrupted by OS signal.*/
        if (EINTR == rc){
            AGILE_LOGD("pthread_cond_wait: interrupted by system signal");
            goto restart;
        }

        INIT_LIST(&cbExeListHead);
        getCallbackExeList(schedHandle, &cbExeListHead, &timeout);
        
        schedHandle->bSignal = MAG_FALSE;
        rc = pthread_mutex_unlock(&schedHandle->lock);
        if(rc != 0){
            AGILE_LOGE("failed to do pthread_mutex_unlock, error(%s)", strerror(errno));
            break;
        }
        
        processEventCallbacks(&cbExeListHead);
    }
    return NULL;
}

MagErr_t Mag_CreateEventScheduler(MagEventSchedulerHandle *evtSched, MagEvtSchedPolicy_t option){
    i32 rc;
    i32 i;
    Mag_EvtCbTimeStamp_t *node;
        
    *evtSched = (MagEventSchedulerHandle)mag_mallocz(sizeof(**evtSched));
    if (NULL == *evtSched)
        return MAG_NoMemory;

    rc = pthread_mutex_init(&(*evtSched)->lock, NULL /* default attributes */);
    if(rc != 0){
        goto err_mutex;
    }

    rc = pthread_cond_init(&(*evtSched)->cond, NULL /* default attributes */);
    if(rc != 0){
        goto err_cond;
    }
    
    INIT_LIST(&(*evtSched)->entry);

    for (i = 0; i < MAG_EVT_PRIO_MAX; i++){
        INIT_LIST(&(*evtSched)->listHead[i]);
    }

    INIT_LIST(&(*evtSched)->cbTimeStampListH);
    INIT_LIST(&(*evtSched)->cbTimeStampFreeListH);
    
    /*pre-allocate a number of the nodes for later using*/
    for (i = 0; i < 10; i++){
        node = (Mag_EvtCbTimeStamp_t *)mag_mallocz(sizeof(Mag_EvtCbTimeStamp_t));
        list_add_tail(&node->tsListNode, &(*evtSched)->cbTimeStampFreeListH);
    }
    
    (*evtSched)->option = option;
    (*evtSched)->sTExited = MAG_FALSE;
    (*evtSched)->bSignal  = MAG_FALSE;

    if (pthread_create(&(*evtSched)->schedThread, NULL, Mag_EventScheduleEntry, *evtSched)){
        AGILE_LOGE("failed to create the event scheduler. (error: %s)", strerror(errno));
        goto err_thread;
    }
    return MAG_ErrNone;
    
err_mutex:
    mag_freep((void **)evtSched);
    return MAG_ErrMutexCreate;

err_cond:
    pthread_mutex_destroy(&(*evtSched)->lock);
    mag_freep((void **)evtSched);
    return MAG_ErrCondCreate;

err_thread:
    pthread_mutex_destroy(&(*evtSched)->lock);
    pthread_cond_destroy(&(*evtSched)->cond);
    mag_freep((void **)evtSched);
    return MAG_ErrThreadCreate;

}

MagErr_t Mag_DestroyEventScheduler(MagEventSchedulerHandle *pEvtSched){
    i32 rc;
    MagEventSchedulerHandle evtSched = *pEvtSched;
    MagErr_t ret = MAG_ErrNone;
    
    if (evtSched == NULL){
        return MAG_BadParameter;
    }

    rc = pthread_mutex_lock(&evtSched->lock);
    MAG_ASSERT(0 == rc);

    evtSched->sTExited = MAG_TRUE;
    
    rc = pthread_cond_signal(&evtSched->cond);
    MAG_ASSERT(0 == rc);
    
    pthread_mutex_unlock(&evtSched->lock);
    MAG_ASSERT(0 == rc);

    pthread_join(evtSched->schedThread, NULL);

    rc = pthread_mutex_lock(&evtSched->lock);
    MAG_ASSERT(0 == rc);

    pthread_mutex_destroy(&evtSched->lock);
    pthread_cond_destroy(&evtSched->cond);

    mag_freep((void **)pEvtSched);
    return ret;
}
