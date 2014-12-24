#ifndef __MAGOMX_COMPONENT_OTHER_CLOCK_H__
#define __MAGOMX_COMPONENT_OTHER_CLOCK_H__

#include "MagOMX_Component_base.h"
#include "MagOMX_Component_baseImpl.h"

#define MAX_CLOCK_PORT_NUMBER 8

#define CONVERT_TO_MICROSECONDS(x) ((x * 1000) / 90)  /*convert 90K pts to the timebase in us*/
enum{
    MagOmxComponentClock_CmdStartTimeMsg = 0,
    MagOmxComponentClock_CmdMTimeRequestMsg
};

static inline OMX_STRING ClockCompState2String(OMX_TIME_CLOCKSTATE state) {
    switch (state) {
        STRINGIFY(OMX_TIME_ClockStateStopped);
        STRINGIFY(OMX_TIME_ClockStateWaitingForStartTime);
        STRINGIFY(OMX_TIME_ClockStateRunning);
        default: return "state - unknown";
    }
}

DeclareClass(MagOmxComponentClock, MagOmxComponentImpl);

Virtuals(MagOmxComponentClock, MagOmxComponentImpl) 
    /*Pure virtual functions. Must be overrided by sub-components*/
    OMX_ERRORTYPE (*MagOMX_Clock_DecideStartTime)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_TICKS* pStartTimeList,
                    OMX_IN  OMX_U32    startTimeMap,  /*((1 << n) & startTimeMap) == (1 << n): the array index nth has the start time*/
                    OMX_OUT OMX_TICKS* pFinalStartTime);

    /*get the time offset value[us] between clock component fulfills request and 
      Clock Client start to do Requested fulfillment. in microseconds
      */
    OMX_ERRORTYPE (*MagOMX_Clock_GetOffset)(
    				OMX_IN  OMX_HANDLETYPE hComponent,
    				OMX_OUT OMX_TICKS *offset);

    /*add the clock port and get the added port index*/
    OMX_ERRORTYPE (*MagOMX_Clock_AddPort)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_OUT OMX_U32 *pPortIdx);

    /*remove the clock port*/
    OMX_ERRORTYPE (*MagOMX_Clock_RemovePort)(
                    OMX_IN  OMX_HANDLETYPE hComponent,
                    OMX_IN  OMX_U32 portIdx);
EndOfVirtuals;

ClassMembers(MagOmxComponentClock, MagOmxComponentImpl, \
	MagMessageHandle (*createCmdMessage)(OMX_HANDLETYPE handle, OMX_U32 what);     \
    _status_t        (*getCmdLooper)(OMX_HANDLETYPE handle); \
    MagMessageHandle (*createTimeRequestMessage)(OMX_HANDLETYPE handle, OMX_U32 what, OMX_U32 port_id); \
    _status_t        (*getTimeRequestLooper)(OMX_HANDLETYPE handle, OMX_U32 port_id); \
    OMX_TICKS        (*getWallTime)(OMX_HANDLETYPE hComponent); \
    OMX_ERRORTYPE    (*updateClockState)(MagOmxPort port, OMX_TIME_CLOCKSTATE state); \
    OMX_ERRORTYPE    (*updateClockScale)(MagOmxPort port, OMX_S32 scale); \
    OMX_ERRORTYPE    (*updateClockReqFulfillment)(MagOmxPort port); \
    OMX_ERRORTYPE    (*addClockPort)(MagOmxComponentClock compClock, OMX_U32 *pPortIndex); \
    OMX_TICKS        (*getMediaTimeNow)(MagOmxComponentClock compClock); \
    OMX_TICKS        (*getMediaTimeRequest)(MagOmxComponentClock compClock, OMX_TICKS mtr, OMX_TICKS offset); \
    OMX_ERRORTYPE    (*sendAVSyncAction)(MagOmxComponentClock compClock, OMX_U32 port_id, OMX_TICKS mediaTimestamp, MagOMX_AVSync_Action_t action); \
)
    MagMutexHandle         mhMutex;
    MagMutexHandle         mhRefTimeUpdateMutex;

    MagLooperHandle        mCmdLooper;
    MagHandlerHandle       mCmdMsgHandler;

    MagLooperHandle        mTimeRequestLooper[MAX_CLOCK_PORT_NUMBER];
    MagHandlerHandle       mTimeRequestMsgHandler[MAX_CLOCK_PORT_NUMBER];

    MagMessageHandle       mCmdStartTimeMsg;
    MagMessageHandle       mCmdMTimeRequestMsg[MAX_CLOCK_PORT_NUMBER];

    OMX_U32                mWaitStartTimeMask;
    
    OMX_TICKS              mStartTimeTable[MAX_CLOCK_PORT_NUMBER];
    OMX_U32                mMaxRenderDelay;

    OMX_S32                mxScale;  /*need to be divided by 10.0 to get the value x.y, y is in [0, 9]*/
    OMX_TIME_CONFIG_CLOCKSTATETYPE           mState;
    OMX_TIME_CONFIG_ACTIVEREFCLOCKUPDATETYPE mRefClockUpdate;
    OMX_TIME_MEDIATIMETYPE                   mMediaTimeType;

    OMX_TICKS               mReferenceTimeBase;   /*in us*/
    OMX_TICKS               mWallTimeBase;        /*in us*/
    OMX_TICKS               mClockOffset;         /*in us*/

    MagEventHandle          mStateChangeEvt;
    MagEventGroupHandle     mStateChangeEvtGrp;

EndOfClassMembers;

#endif