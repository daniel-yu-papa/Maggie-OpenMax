#include "OMXComponent_state.h"

static OMX_ERRORTYPE Event_Go_StateInvalid_Handler(OMX_COMPONENTTYPE *comp);
static OMX_ERRORTYPE Event_Go_StateLoaded_Handler(OMX_COMPONENTTYPE *comp);
static OMX_ERRORTYPE Event_Go_StateIdle_Handler(OMX_COMPONENTTYPE *comp);
static OMX_ERRORTYPE Event_Go_StateExecuting_Handler(OMX_COMPONENTTYPE *comp);
static OMX_ERRORTYPE Event_Go_StatePause_Handler(OMX_COMPONENTTYPE *comp);
static OMX_ERRORTYPE Event_Go_StateWaitForResources_Handler(OMX_COMPONENTTYPE *comp);

static const OMX_StateTransEventTable_t eventTable[Event_Go_StateTransMax] = {
        {Event_Go_StateInvalid, Event_Go_StateInvalid_Handler},
        {Event_Go_StateLoaded, Event_Go_StateLoaded_Handler},
        {Event_Go_StateIdle, Event_Go_StateIdle_Handler},
        {Event_Go_StateExecuting, Event_Go_StateExecuting_Handler},
        {Event_Go_StatePause, Event_Go_StatePause_Handler},
        {Event_Go_StateWaitForResources, Event_Go_StateWaitForResources_Handler}
};

OMX_ERRORTYPE OMX_StateTransition_Process(OMX_IN OMX_COMPONENTTYPE *comp, OMX_IN OMX_STATETYPE goState){
    OMX_ERRORTYPE ret;
    
    if (NULL == comp)
        return OMX_ErrorBadParameter;

    if (goState >= Event_Go_StateTransMax){
        AGILE_LOGE("the state %d is invalid", goState);
        return OMX_ErrorIncorrectStateTransition;
    }
        
    ret = eventTable[goState].handler(comp);

    return ret;
}

static OMX_ERRORTYPE Event_Go_StateInvalid_Handler(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *pPrivData = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXCompInternalState_t currentSt = pPrivData->state;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    while (*currentSt > OMX_Priv_StateMax){
        AGILE_LOGE("Waiting for state transition[%d] phase complete, then transit to StateInvalid.", currentSt);
        usleep(1000);
    }

    pthread_mutex_lock(&pPrivData->stateTransMutex);
    
    switch (currentSt){
        case OMX_Priv_StateLoaded:

            break;
            
        case OMX_Priv_StateIdle:

            break;

        case OMX_Priv_StateExecuting:

            break;

        case OMX_Priv_StatePause:

            break;

        case OMX_Priv_StateWaitForResources:

            break;
       
        default:
            AGILE_LOGE("incorrect state transition from st[%d] to StateLoaded. Never be here!!", currentSt);
            ret = OMX_ErrorIncorrectStateTransition;
            break;
    }

    if (OMX_ErrorNone == ret){
        pPrivData->state = OMX_Priv_StateInvalid;  
    }
    
    pthread_mutex_unlock(&pPrivData->stateTransMutex);
    
    return ret; 
}

static OMX_ERRORTYPE Event_Go_StateLoaded_Handler(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *pPrivData = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXCompInternalState_t currentSt = pPrivData->state;
    OMX_BOOL bExit = OMX_FALSE;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    pthread_mutex_lock(&pPrivData->stateTransMutex);

    if (currentSt > OMX_Priv_StateMax){
        AGILE_LOGE("In state transition[%d] phase. ignore the set command: StateLoaded", currentSt);
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
        
        return OMX_ErrorIncorrectStateTransition;
    }
    
    do {
        switch (currentSt){
            case OMX_Priv_StateIdle:
                currentSt = OMX_TransState_Idle_to_Loaded;
                break;
                
            case OMX_TransState_Idle_to_Loaded:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;
                
            case OMX_Priv_StateWaitForResources:
                currentSt = OMX_TransState_WaitForResources_to_Loaded;
                break;
                
            case OMX_TransState_WaitForResources_to_Loaded:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;

            default:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                AGILE_LOGE("incorrect state transition from st[%d] to StateLoaded", currentSt);
                ret = OMX_ErrorIncorrectStateTransition;
                bExit = OMX_TRUE;
                break;
        }
    }
    while (!bExit);

     if (OMX_ErrorNone == ret){
        pthread_mutex_lock(&pPrivData->stateTransMutex);
        pPrivData->state = OMX_Priv_StateLoaded;
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
    }
    return ret;
}

static OMX_ERRORTYPE Event_Go_StateIdle_Handler(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *pPrivData = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXCompInternalState_t currentSt = pPrivData->state;
    OMX_BOOL bExit = OMX_FALSE;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    pthread_mutex_lock(&pPrivData->stateTransMutex);

    if (currentSt > OMX_Priv_StateMax){
        AGILE_LOGE("In state transition[%d] phase. ignore the set command: StateIdle", currentSt);
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
        
        return OMX_ErrorIncorrectStateTransition;
    }
    
    do {
        switch (currentSt){
            case OMX_Priv_StateLoaded:
                currentSt = OMX_TransState_Loaded_to_Idle;
                break;
                
            case OMX_TransState_Loaded_to_Idle:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                MagErr_t mag_ret;
                mag_ret = Mag_WaitForEventGroup(pPrivData->EventGroup_PortBufferAllocated, MAG_EG_OR, 1000); /*timeout is 1 second*/
                
                if (MAG_TimeOut == ret){
                    AGILE_LOGE("failed to get all required buffers allocated. Keep state Loaded!");
                    ret = OMX_ErrorInsufficientResources
                }
                    
                bExit = OMX_TRUE;
                break;
                
            case OMX_Priv_StateExecuting:
                currentSt = OMX_TransState_Executing_to_Idle;
                break;
                
            case OMX_TransState_Executing_to_Idle:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;
                
            case OMX_Priv_StatePause:
                currentSt = OMX_TransState_Pause_to_Idle;
                break;
                
            case OMX_TransState_Pause_to_Idle:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;

            case OMX_Priv_StateWaitForResources:
                currentSt = OMX_TransState_WaitForResources_to_Idle;
                break;

            case OMX_TransState_WaitForResources_to_Idle:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/

                bExit = OMX_TRUE;
                break;
                
            default:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                AGILE_LOGE("incorrect state transition from st[%d] to StateIdle", currentSt);
                ret = OMX_ErrorIncorrectStateTransition;
                bExit = OMX_TRUE;
                break;
        }
    }
    while (!bExit);

    if (OMX_ErrorNone == ret){
        pthread_mutex_lock(&pPrivData->stateTransMutex);
        pPrivData->state = OMX_Priv_StateIdle;
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
    }
    return ret;
}

static OMX_ERRORTYPE Event_Go_StateExecuting_Handler(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *pPrivData = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXCompInternalState_t currentSt = pPrivData->state;
    OMX_BOOL bExit = OMX_FALSE;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    pthread_mutex_lock(&pPrivData->stateTransMutex);

    if (currentSt > OMX_Priv_StateMax){
        AGILE_LOGE("In state transition[%d] phase. ignore the set command: StateExecuting", currentSt);
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
        
        return OMX_ErrorIncorrectStateTransition;
    }
    
    do {
        switch (currentSt){
            case OMX_Priv_StateIdle:
                currentSt = OMX_TransState_Idle_to_Executing;
                break;
                
            case OMX_TransState_Idle_to_Executing:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;
                
            case OMX_Priv_StatePause:
                currentSt = OMX_TransState_Pause_to_Executing;
                break;
                
            case OMX_TransState_Pause_to_Executing:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;

            default:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                AGILE_LOGE("incorrect state transition from st[%d] to StateExecuting", currentSt);
                ret = OMX_ErrorIncorrectStateTransition;
                bExit = OMX_TRUE;
                break;
        }
    }
    while (!bExit);

    if (OMX_ErrorNone == ret){
        pthread_mutex_lock(&pPrivData->stateTransMutex);
        pPrivData->state = OMX_Priv_StateExecuting;
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
    }
    return ret;
}

static OMX_ERRORTYPE Event_Go_StatePause_Handler(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *pPrivData = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXCompInternalState_t currentSt = pPrivData->state;
    OMX_BOOL bExit = OMX_FALSE;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    pthread_mutex_lock(&pPrivData->stateTransMutex);

    if (currentSt > OMX_Priv_StateMax){
        AGILE_LOGE("In state transition[%d] phase. ignore the set command: StatePause", currentSt);
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
        
        return OMX_ErrorIncorrectStateTransition;
    }
    
    do {
        switch (currentSt){
            case OMX_Priv_StateIdle:
                currentSt = OMX_TransState_Idle_to_Pause;
                break;
                
            case OMX_TransState_Idle_to_Pause:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;
                
            case OMX_Priv_StateExecuting:
                currentSt = OMX_TransState_Executing_to_Pause;
                break;
                
            case OMX_TransState_Executing_to_Pause:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;

            default:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                AGILE_LOGE("incorrect state transition from st[%d] to StatePause", currentSt);
                ret = OMX_ErrorIncorrectStateTransition;
                bExit = OMX_TRUE;
                break;
        }
    }
    while (!bExit);

    if (OMX_ErrorNone == ret){
        pthread_mutex_lock(&pPrivData->stateTransMutex);
        pPrivData->state = OMX_Priv_StatePause;
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
    }
    return ret;
}

static OMX_ERRORTYPE Event_Go_StateWaitForResources_Handler(OMX_COMPONENTTYPE *comp){
    OMXComponentPrivateBase_t *pPrivData = (OMXComponentPrivateBase_t *)comp->pComponentPrivate;
    OMXCompInternalState_t currentSt = pPrivData->state;
    OMX_BOOL bExit = OMX_FALSE;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    
    pthread_mutex_lock(&pPrivData->stateTransMutex);

    if (currentSt > OMX_Priv_StateMax){
        AGILE_LOGE("In state transition[%d] phase. ignore the set command: StateWaitForResources", currentSt);
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
        
        return OMX_ErrorIncorrectStateTransition;
    }
    
    do {
        switch (currentSt){
            case OMX_Priv_StateLoaded:
                currentSt = OMX_TransState_Loaded_to_WaitForResources;
                break;
                
            case OMX_TransState_Loaded_to_WaitForResources:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                /*do time-consuming actions*/
                
                bExit = OMX_TRUE;
                break;
                
            default:
                pthread_mutex_unlock(&pPrivData->stateTransMutex);
                AGILE_LOGE("incorrect state transition from st[%d] to StateWaitForResources", currentSt);
                ret = OMX_ErrorIncorrectStateTransition;
                bExit = OMX_TRUE;
                break;
        }
    }
    while (!bExit);

    if (OMX_ErrorNone == ret){
        pthread_mutex_lock(&pPrivData->stateTransMutex);
        pPrivData->state = OMX_Priv_StateWaitForResources;
        pthread_mutex_unlock(&pPrivData->stateTransMutex);
    }
    return ret;
}


