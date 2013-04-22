#ifndef _MAG_EVENT_H__
#define _MAG_EVENT_H__

#include "Mag_pub_def.h"
#include "Mag_list.h"
#include <pthread.h>

/*max 16 events in 1 event group*/
#define MAX_EVENTS_EG    16 

typedef enum{
    MAG_EG_AND,
    MAG_EG_OR
}MAG_EVENT_GROUP_OP_t;

typedef struct mag_event_group_obj{
    List_t EventGroupHead;
    pthread_mutex_t lock;            /* mutex for protecting signal and conditional variables */
    pthread_cond_t  cond;            /* condition to wake up from event set*/
    unsigned int    eventNum;        /* the number of the events*/
}MagEventGroupObj_t;

typedef MagEventGroupObj_t        *MagEventGroupHandle;

typedef struct mag_event_group_element{
    List_t              entry;
    MagEventGroupHandle pEventGroup;
    pthread_mutex_t     lock;            /* mutex for protecting signal and conditional variables */
    MAG_BOOL_t          signal;          /* >0: the event is really triggered. =0: not triggered*/
}MagEventGroupElement_t;

typedef MagEventGroupElement_t    *MagEventGroupElementHandle;

MagErr_t Mag_CreateEventGroupElement(MagEventGroupElementHandle *evtHandle);
MagErr_t Mag_DestroyEventGroupElement(MagEventGroupElementHandle evtHandle);
MagErr_t Mag_SetEventGroupElement(MagEventGroupElementHandle evtHandle);


MagErr_t Mag_CreateEventGroup(MagEventGroupHandle *evtGrphandle);
void     Mag_DestroyEventGroup(MagEventGroupHandle evtGrphandle);
MagErr_t Mag_AddEventGroup(MagEventGroupHandle evtGrphandle, MagEventGroupElementHandle event);
MagErr_t Mag_RemoveEventGroup(MagEventGroupHandle evtGrphandle, MagEventGroupElementHandle event);
MagErr_t Mag_WaitForEventGroup(MagEventGroupHandle evtGrphandle, MAG_EVENT_GROUP_OP_t op, int timeoutMsec);

#endif