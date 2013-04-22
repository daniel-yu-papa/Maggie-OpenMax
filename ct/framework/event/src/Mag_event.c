#include <errno.h>
#include "Mag_event.h"
#include "agilelog.h"
#include "Mag_base.h"

#define MODULE_TAG "MAG_EVENT_MODULE"

#ifdef MAG_DEBUG
static int evtNumTotal = 0;
#endif
/*
* 1) if multiple threads waiting upon the same event, only one thread is woken up in terms of the thread priorities
* 2) if 1 thread is waiting upon the event group and multiple threads signal the events, there is possibility to miss the events due to 
*     some events are reveived at the same time. Do we need the event retrigger logic???
*/

MagErr_t Mag_CreateEventGroupElement(MagEventGroupElementHandle *evtHandle){
    int rc;
    
    *evtHandle = (MagEventGroupElementHandle)malloc(sizeof(**evtHandle));
    if(NULL == *evtHandle)
        return MAG_NoMemory;

    rc = pthread_mutex_init(&(*evtHandle)->lock, NULL /* default attributes */);
    if(rc != 0){
        goto err_mutex;
    }
    
    INIT_LIST(&(*evtHandle)->entry);
    (*evtHandle)->signal = MAG_FALSE;
    (*evtHandle)->pEventGroup = NULL;
    return MAG_ErrNone;
    
err_mutex:
    free(*evtHandle);
    return MAG_ErrMutexCreate;
}

MagErr_t Mag_DestroyEventGroupElement(MagEventGroupElementHandle evtHandle){
    if (evtHandle->pEventGroup){
        Mag_RemoveEventGroup(evtHandle->pEventGroup, evtHandle);
    }
    pthread_mutex_destroy(&evtHandle->lock);
    free(evtHandle);
}

static MagErr_t MagPriv_ms2timespec(struct timespec *target, int timeoutMsec){
    int rc;
    struct timeval now;
    rc = gettimeofday(&now, NULL);
    if (0 != rc)
        return MAG_ErrGetTime;

    target->tv_nsec = now.tv_usec * 1000 + (timeoutMsec%1000)*1000000;
    target->tv_sec = now.tv_sec + (timeoutMsec/1000);
    if (target->tv_nsec >= 1000000000) {
        target->tv_nsec -=  1000000000;
        target->tv_sec ++;
    }
    return MAG_ErrNone;
}


MagErr_t  Mag_SetEventGroupElement(MagEventGroupElementHandle evtHandle){
    int rc;
    MagEventGroupHandle evtGrp = evtHandle->pEventGroup;
    MagErr_t ret = MAG_ErrNone;
    
    if(NULL == evtGrp){
        AGILE_LOGE("The event element(0x%lx) is not Added into the event group. Failed to do setEvent!!", 
                   (unsigned long)evtHandle);
        return MAG_ErrEvtSetOp;
    }

    rc = pthread_mutex_lock(&evtGrp->lock);
    MAG_ASSERT(0 == rc);

    rc = pthread_mutex_lock(&evtHandle->lock);
    MAG_ASSERT(0 == rc);

    evtHandle->signal = MAG_TRUE;

    rc = pthread_cond_signal(&evtGrp->cond);
    MAG_ASSERT(0 == rc);

    rc = pthread_mutex_unlock(&evtHandle->lock);
    MAG_ASSERT(0 == rc);

    rc = pthread_mutex_unlock(&evtGrp->lock);
    MAG_ASSERT(0 == rc);

    return ret;
}


MagErr_t Mag_CreateEventGroup(MagEventGroupHandle *evtGrphandle){
    int rc;

    *evtGrphandle = (MagEventGroupHandle)malloc(sizeof(**evtGrphandle));
    if (NULL == *evtGrphandle)
        return MAG_NoMemory;

    rc = pthread_mutex_init(&(*evtGrphandle)->lock, NULL /* default attributes */);
    if(rc != 0){
        goto err_mutex;
    }

    rc = pthread_cond_init(&(*evtGrphandle)->cond, NULL /* default attributes */);
    if(rc != 0){
        goto err_cond;
    }

    INIT_LIST(&(*evtGrphandle)->EventGroupHead);
    (*evtGrphandle)->eventNum = 0;

    evtNumTotal = 0;
    return MAG_ErrNone;
    
err_cond:
    pthread_mutex_destroy(&(*evtGrphandle)->lock);
    free(*evtGrphandle);
    return MAG_ErrCondCreate;
    
err_mutex:
    free(*evtGrphandle);
    return MAG_ErrMutexCreate;    

}

void Mag_DestroyEventGroup(MagEventGroupHandle evtGrphandle){
    List_t *tmpNode = evtGrphandle->EventGroupHead.next;
    MagEventGroupElementHandle pEvent;

    while (tmpNode != &evtGrphandle->EventGroupHead){
        pEvent = (MagEventGroupElementHandle) tmpNode;
        Mag_RemoveEventGroup(evtGrphandle, pEvent);
        Mag_DestroyEventGroupElement(pEvent);
        tmpNode = evtGrphandle->EventGroupHead.next;
    }

    pthread_mutex_destroy(&evtGrphandle->lock);
    pthread_cond_destroy(&evtGrphandle->cond);
    free(evtGrphandle);
}

MagErr_t Mag_AddEventGroup(MagEventGroupHandle evtGrphandle, MagEventGroupElementHandle event){
    int rc;
    MagErr_t ret = MAG_ErrNone;

    if (NULL != event->pEventGroup){
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
    
    rc = pthread_mutex_lock(&event->lock);
    MAG_ASSERT(0 == rc);

    event->pEventGroup = evtGrphandle;
    list_add(&event->entry, &evtGrphandle->EventGroupHead);
    evtGrphandle->eventNum++;
    if(event->signal == MAG_TRUE){
        /* signal condition if signal already set */
        pthread_cond_signal(&evtGrphandle->cond);
    }
    
    rc = pthread_mutex_unlock(&event->lock);
    MAG_ASSERT(0 == rc);

done:    
    rc = pthread_mutex_unlock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    return ret;
}

MagErr_t Mag_RemoveEventGroup(MagEventGroupHandle evtGrphandle, MagEventGroupElementHandle event){
    int rc;
    MagErr_t ret = MAG_ErrNone;

    if (NULL == event->pEventGroup){
        AGILE_LOGD("the event(0x%x) don't have EventGroup", (unsigned long)event);
        return MAG_ErrEvtSetOp;
    }

    if (evtGrphandle != event->pEventGroup){
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
    
    rc = pthread_mutex_lock(&event->lock);
    MAG_ASSERT(0 == rc);

    event->pEventGroup = NULL;
    list_del(&event->entry);
    evtGrphandle->eventNum--;
    
    rc = pthread_mutex_unlock(&event->lock);
    MAG_ASSERT(0 == rc);

done:   
    rc = pthread_mutex_unlock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    return ret;
}

static MagErr_t Mag_EventStatusCheck(MagEventGroupHandle evtGrphandle, MAG_EVENT_GROUP_OP_t op){
    List_t *tmpNode = evtGrphandle->EventGroupHead.next;
    MagEventGroupElementHandle pEvent;
    MagErr_t ret = MAG_EventStatusErr;
    int rc;
    int signalEvtNum = 0;
    
    while (tmpNode != &evtGrphandle->EventGroupHead){
        pEvent = (MagEventGroupElementHandle) tmpNode;

        rc = pthread_mutex_lock(&pEvent->lock);
        MAG_ASSERT(0 == rc);
            
        if (MAG_EG_OR == op){ /*event1 OR event2 OR event3 is true*/
            if (pEvent->signal == MAG_TRUE)
                signalEvtNum++;
        }else{ /*event1 AND event2 AND event3 is true*/
            if (pEvent->signal == MAG_TRUE)
                signalEvtNum++;   
        }
        
        rc = pthread_mutex_unlock(&pEvent->lock);
        MAG_ASSERT(0 == rc);

        tmpNode = tmpNode->next;
    }
    if (MAG_EG_OR == op){
        if (signalEvtNum > 0){
#ifdef MAG_DEBUG
            evtNumTotal += signalEvtNum;
            AGILE_LOGD("[OR]triggered event number: %d. Total[%d]", signalEvtNum, evtNumTotal);
#endif
            ret = MAG_EventStatusMeet;
        }
    }else{
        if (signalEvtNum == evtGrphandle->eventNum){
#ifdef MAG_DEBUG
            evtNumTotal += signalEvtNum;
            AGILE_LOGD("[AND]triggered event number: %d. Total[%d]", signalEvtNum, evtNumTotal);
#endif
            ret = MAG_EventStatusMeet;
        }
    }
    return ret;
}


static MagErr_t Mag_ClearAllEvents(MagEventGroupHandle evtGrphandle){
    List_t *tmpNode = evtGrphandle->EventGroupHead.next;
    MagEventGroupElementHandle pEvent;
    MagErr_t ret = MAG_ErrNone;
    int rc;
    
    while (tmpNode != &evtGrphandle->EventGroupHead){
        pEvent = (MagEventGroupElementHandle) tmpNode;

        rc = pthread_mutex_lock(&pEvent->lock);
        MAG_ASSERT(0 == rc);
            
        pEvent->signal = MAG_FALSE; 
        
        rc = pthread_mutex_unlock(&pEvent->lock);
        MAG_ASSERT(0 == rc);

        tmpNode = tmpNode->next;
    }
}


MagErr_t Mag_WaitForEventGroup(MagEventGroupHandle evtGrphandle, MAG_EVENT_GROUP_OP_t op, int timeoutMsec){
    struct timespec target;
    MagErr_t ret = MAG_ErrNone;
    int rc;

    if ((timeoutMsec < 0) && (timeoutMsec != MAG_TIMEOUT_INFINITE))
        return MAG_BadParameter;

    if(timeoutMsec != MAG_TIMEOUT_INFINITE){
        if ((ret = MagPriv_ms2timespec(&target, timeoutMsec)) != MAG_ErrNone)
            return ret;
    }
    
    rc = pthread_mutex_lock(&evtGrphandle->lock);
    MAG_ASSERT(0 == rc);

    if (0 == timeoutMsec){
        AGILE_LOGD("Timeout!!");
        ret = MAG_TimeOut;
        goto done;
    }

    while(MAG_EventStatusMeet != Mag_EventStatusCheck(evtGrphandle, op)){ /* we might have been wokenup and then event has been cleared */
        if (timeoutMsec == MAG_TIMEOUT_INFINITE){
            rc = pthread_cond_wait(&evtGrphandle->cond, &evtGrphandle->lock);
        }else{
            rc = pthread_cond_timedwait(&evtGrphandle->cond, &evtGrphandle->lock, &target);
            if(ETIMEDOUT == rc){
                ret = MAG_TimeOut;
                AGILE_LOGD("WaitfForEventSet(evtSet: 0x%lx): timeout [%d ms]", (unsigned long)evtGrphandle, timeoutMsec);
                goto done;
            }
        }

        /*the blocking system call: futex might be interrupted by OS signal.*/
        if (EINTR == rc){
            AGILE_LOGD("WaitfForEventSet(evtSet: 0x%lx): interrupted by system signal", (unsigned long)evtGrphandle);
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

