/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _MAG_MESSAGE_H__
#define _MAG_MESSAGE_H__

#include <pthread.h>
#include "Mag_event.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
* 1) The message format should be all same for the specific message channel
* 2) it is point-to-point message channel
* 3) 
*/

typedef enum{
    MSG_DISCARD = 0,  /*discard all messages and stop immediately*/
    MSG_FLUSH         /*finish all messages before stopping*/
}Mag_MsgDetachFlag_t;

typedef enum{
    MSG_CTRL_STOP = 0,
    MSG_CTRL_RUN
}Mag_MsgCtrlType_t;

typedef void (*fnMsgChanReceiver)(void *msg, void *priv);

typedef struct mag_msg_obj{
    List_t node;

    unsigned char *msgBody;
    unsigned int  msgLen;
    Mag_MsgCtrlType_t ctrlType;   
}Mag_Message_t;

typedef struct mag_msg_channel_obj{
    List_t msgQueueHead;
    List_t freeMsgListHead;

    fnMsgChanReceiver receiverFunc;
    void *privData;
    
    pthread_t ReceiverThread;
    pthread_mutex_t lock;

    MagEventGroupHandle evtGrp;
    MagEventHandle      event;

    MAG_BOOL_t stopped;
}Mag_MsgChannel_t;

typedef Mag_MsgChannel_t* MagMsgChannelHandle;

/*function prototye definitions*/
MagErr_t Mag_MsgChannelCreate(MagMsgChannelHandle *handle);
MagErr_t Mag_MsgChannelDestroy(MagMsgChannelHandle *pHandle);

/*only one receiver could be attached to the message channel*/
MagErr_t Mag_MsgChannelReceiverAttach(MagMsgChannelHandle handle, fnMsgChanReceiver func, void *priv_data);
MagErr_t Mag_MsgChannelReceiverDettach(MagMsgChannelHandle handle, Mag_MsgDetachFlag_t flag);


MagErr_t Mag_MsgChannelSend(MagMsgChannelHandle handle, const void *msg, const unsigned int msg_len);

#ifdef __cplusplus
}
#endif

#endif