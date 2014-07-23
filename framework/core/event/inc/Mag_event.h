#ifndef _MAG_EVENT_H__
#define _MAG_EVENT_H__

#include <pthread.h>
#include <time.h>
#include "Mag_list.h"
#include "Mag_pub_def.h"  
#include "Mag_pub_type.h"
#include "Mag_agilelog.h"
#include "Mag_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*max 16 events in 1 event group*/
#define MAX_EVENTS_EG    16 

typedef enum{
    MAG_EG_AND,
    MAG_EG_OR
}MAG_EVENT_GROUP_OP_t;

typedef enum{

    MAG_EVT_PRIO_DEFAULT = 0, /*most of events should be in this list*/
    MAG_EVT_PRIO_LOW = 1,
    MAG_EVT_PRIO_HIGH = 2,
    MAG_EVT_PRIO_MAX = 3,
}MAG_EVENT_PRIO_t;

typedef enum{
    MAG_EVT_SCHED_NORMAL = 0, /*default is event retrigger while several events occur at the same time*/
    MAG_EVT_SCHED_NO_RETRIGGER, 
}MagEvtSchedPolicy_t;

typedef struct mag_event_group_obj{
    List_t EventGroupHead;
    pthread_mutex_t lock;            /* mutex for protecting signal and conditional variables */
    pthread_cond_t  cond;            /* condition to wake up from event set*/
    ui32    eventNum;        /* the number of the events*/
}MagEventGroupObj_t;

typedef MagEventGroupObj_t    *MagEventGroupHandle;

typedef struct mag_event_scheduler_obj{
    List_t              entry;

    List_t              listHead[MAG_EVT_PRIO_MAX];
    
    /*for default priority event list*/
    List_t              cbTimeStampListH;
    List_t              cbTimeStampFreeListH;
    
    pthread_mutex_t     lock;
    pthread_cond_t      cond;
    MagEvtSchedPolicy_t option;
    
    pthread_t           schedThread;
    MAG_BOOL_t          sTExited;

    MAG_BOOL_t          bSignal;
}Mag_EventScheduler_t;

typedef Mag_EventScheduler_t *MagEventSchedulerHandle;

/*lock: prevent the unregister action in the callback executing*/
typedef struct mag_event_callback_obj{
    List_t          exeEntry;
    ui32            exeNum;
    pthread_mutex_t lock;
    
    void (*pCallback)(void *);
    void *pContext;
}MagEventCallbackObj_t;

typedef MagEventCallbackObj_t *MagEventCallbackHandle;

typedef struct mag_evt_cb_ts_obj{
    List_t          tsListNode;
    struct timespec timeStamp;
    i32             timeDiff; /*the time difference between 2 continuous incoming events*/
    MagEventCallbackHandle cbBody;
}Mag_EvtCbTimeStamp_t;

typedef struct mag_event_common_obj{
    List_t              entry;

    MagEventGroupHandle hEventGroup;
    // pthread_mutex_t     lock;            /* mutex for protecting signal and conditional variables */
    MAG_BOOL_t          signal;          /* >0: the event is really triggered. =0: not triggered*/
}Mag_EventCommon_t;

typedef struct mag_event_callback_t{
    List_t                  entry;

    // pthread_mutex_t         lock;            /* mutex for protecting the object handling */
    unsigned int            armed;          /* >0: the event callback is armed. =0: not armed*/
    MAG_EVENT_PRIO_t        priority;

    MagEventCallbackHandle  hCallback;
    MagEventSchedulerHandle hEvtScheduler;
}Mag_EventCallback_t;

typedef struct mag_event{
    Mag_EventCommon_t   *pEvtCommon; //Must be the first element
    Mag_EventCallback_t *pEvtCallBack;
}MagEvent_t;

typedef MagEvent_t            *MagEventHandle;


MagErr_t Mag_CreateEvent(MagEventHandle *evtHandle, MAG_EVENT_PRIO_t prio);
MagErr_t Mag_DestroyEvent(MagEventHandle evtHandle);
MagErr_t Mag_SetEvent(MagEventHandle evtHandle);
void     Mag_ClearEvent(MagEventHandle evtHandle);

MagErr_t Mag_CreateEventGroup(MagEventGroupHandle *evtGrphandle);
void     Mag_DestroyEventGroup(MagEventGroupHandle evtGrphandle);
MagErr_t Mag_AddEventGroup(MagEventGroupHandle evtGrphandle, MagEventHandle event);
MagErr_t Mag_RemoveEventGroup(MagEventGroupHandle evtGrphandle, MagEventHandle event);
MagErr_t Mag_WaitForEventGroup(MagEventGroupHandle evtGrphandle, MAG_EVENT_GROUP_OP_t op, i32 timeoutMsec);

MagErr_t Mag_CreateEventScheduler(MagEventSchedulerHandle *evtSched, MagEvtSchedPolicy_t option);
MagErr_t Mag_DestroyEventScheduler(MagEventSchedulerHandle evtSched);
MagErr_t Mag_RegisterEventCallback(MagEventSchedulerHandle schedHandle, MagEventHandle evtHandle, void (*pCallback)(void *), void *pContext);
MagErr_t Mag_UnregisterEventCallback(MagEventHandle evtHandle);

#ifdef __cplusplus
}
#endif

#endif
