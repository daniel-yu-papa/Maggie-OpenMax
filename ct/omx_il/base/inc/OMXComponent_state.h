#ifndef __OMX_COMPONENT_STATE_H__
#define __OMX_COMPONENT_STATE_H__

#define TRANS_STATE_VALUE(a, b) (a + 1)*100+(b + 1)

typedef enum{
    OMX_Priv_StateInvalid,
    OMX_Priv_StateLoaded,
    OMX_Priv_StateIdle,
    OMX_Priv_StateExecuting,
    OMX_Priv_StatePause,
    OMX_Priv_StateWaitForResources,
    OMX_Priv_StateMax = 100,
    
    OMX_TransState_Loaded_to_Idle = TRANS_STATE_VALUE(OMX_Priv_StateLoaded, OMX_Priv_StateIdle),
    OMX_TransState_Loaded_to_WaitForResources = TRANS_STATE_VALUE(OMX_Priv_StateLoaded, OMX_Priv_StateWaitForResources),
    OMX_TransState_Loaded_to_Invalid = TRANS_STATE_VALUE(OMX_Priv_StateLoaded, OMX_Priv_StateInvalid),
    OMX_TransState_WaitForResources_to_Idle = TRANS_STATE_VALUE(OMX_Priv_StateWaitForResources, OMX_Priv_StateIdle),
    OMX_TransState_WaitForResources_to_Invalid = TRANS_STATE_VALUE(OMX_Priv_StateWaitForResources, OMX_Priv_StateInvalid),
    OMX_TransState_WaitForResources_to_Loaded = TRANS_STATE_VALUE(OMX_Priv_StateWaitForResources, OMX_Priv_StateLoaded),
    OMX_TransState_Idle_to_Loaded = TRANS_STATE_VALUE(OMX_Priv_StateIdle, OMX_Priv_StateLoaded),
    OMX_TransState_Idle_to_Invalid = TRANS_STATE_VALUE(OMX_Priv_StateIdle, OMX_Priv_StateInvalid),
    OMX_TransState_Idle_to_Executing = TRANS_STATE_VALUE(OMX_Priv_StateIdle, OMX_Priv_StateExecuting),
    OMX_TransState_Idle_to_Pause = TRANS_STATE_VALUE(OMX_Priv_StateIdle, OMX_Priv_StatePause),
    OMX_TransState_Executing_to_Idle = TRANS_STATE_VALUE(OMX_Priv_StateExecuting, OMX_Priv_StateIdle),
    OMX_TransState_Executing_to_Pause = TRANS_STATE_VALUE(OMX_Priv_StateExecuting, OMX_Priv_StatePause),
    OMX_TransState_Executing_to_Invalid = TRANS_STATE_VALUE(OMX_Priv_StateExecuting, OMX_Priv_StateInvalid),
    OMX_TransState_Pause_to_Idle = TRANS_STATE_VALUE(OMX_Priv_StatePause, OMX_Priv_StateIdle),
    OMX_TransState_Pause_to_Executing = TRANS_STATE_VALUE(OMX_Priv_StatePause, OMX_Priv_StateExecuting),
    OMX_TransState_Pause_to_Invalid = TRANS_STATE_VALUE(OMX_Priv_StatePause, OMX_Priv_StateInvalid),
}OMXCompInternalState_t;

typedef enum{
    Event_Go_StateInvalid = 0,
    Event_Go_StateLoaded = 1,
    Event_Go_StateIdle = 2,
    Event_Go_StateExecuting = 3,
    Event_Go_StatePause = 4,
    Event_Go_StateWaitForResources = 5,
    Event_Go_StateTransMax = 6,
}OMXCompStateTransEvent_t;

typedef OMX_ERRORTYPE (*OMX_StateTrans_EventHandler)(OMX_COMPONENTTYPE *comp);

typedef struct{
    OMXCompStateTransEvent_t event;
    OMX_StateTrans_EventHandler handler;
}OMX_StateTransEventTable_t;

OMX_ERRORTYPE OMX_StateTransition_Process(OMX_IN OMX_COMPONENTTYPE *comp, OMX_IN OMX_STATETYPE goState);

#endif