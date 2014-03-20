#include <errno.h>
#include <string.h>

#include "Mag_msg.h"

MagErr_t Mag_MsgChannelCreate(MagMsgChannelHandle *handle){
    int rc;
    
    *handle = (MagMsgChannelHandle)malloc(sizeof(**handle));
    if(NULL == *handle){
        return MAG_NoMemory;
    }
    
    INIT_LIST(&(*handle)->msgQueueHead);
    INIT_LIST(&(*handle)->freeMsgListHead);

    rc = pthread_mutex_init(&(*handle)->lock, NULL /* default attributes */);
    if(rc != 0){
        goto err_mutex;
    }

    (*handle)->privData       = NULL;
    (*handle)->receiverFunc   = NULL;
    (*handle)->ReceiverThread = (pthread_t)0;
    (*handle)->stopped        = MAG_FALSE;
    
    if (MAG_ErrNone == Mag_CreateEventGroup(&(*handle)->evtGrp)){
        if (MAG_ErrNone == Mag_CreateEvent(&(*handle)->event, 0))
            Mag_AddEventGroup((*handle)->evtGrp, (*handle)->event);
        else
            goto err_event;
    }else{
        goto err_event;
    }
    
    return MAG_ErrNone;

err_event:
    (*handle)->event  = NULL;
    (*handle)->evtGrp = NULL;
    
err_mutex:
    if((*handle))
        free((*handle));
    
    *handle = NULL;
    return MAG_ErrMutexCreate;
}

MagErr_t Mag_MsgChannelDestroy(MagMsgChannelHandle handle){
    List_t *tmpNode;
    Mag_Message_t *msg;
    int freeNodeCount = 0;
    
    if(NULL == handle)
        return MAG_BadParameter;
    
    Mag_MsgChannelReceiverDettach(handle, MSG_FLUSH);

    tmpNode = handle->freeMsgListHead.next;

    while (tmpNode != &handle->freeMsgListHead){
        msg = (Mag_Message_t *)list_entry(tmpNode, Mag_Message_t, node);
        if (msg->msgBody)
            free(msg->msgBody);
        
        list_del(&msg->node);
        free(msg);
        tmpNode = handle->freeMsgListHead.next;
        freeNodeCount++;
    }
    
    AGILE_LOGV("delete total %d nodes from the free list", freeNodeCount);
    Mag_DestroyEvent(handle->event);
    Mag_DestroyEventGroup(handle->evtGrp);

    pthread_mutex_destroy(&handle->lock);
    
    free(handle);
    return MAG_ErrNone;
}

static void *Mag_MsgChannelProcessEntry(void *arg){
    List_t *tmpNode;
    int rc;
    MagMsgChannelHandle msgChHandle = (MagMsgChannelHandle)arg;
    int timeout = MAG_TIMEOUT_INFINITE;
    Mag_Message_t *msg;
    MagErr_t ret;

    if (NULL == msgChHandle->evtGrp){
        AGILE_LOGE("the evtGrp is NULL. exit the Mag_MsgProcessEntry()");
        return NULL;
    }
    
    for(;;){
        if(msgChHandle->stopped){
            AGILE_LOGV("exit the Message Processing Thread");
            break;
        }
        
        Mag_WaitForEventGroup(msgChHandle->evtGrp, MAG_EG_OR, MAG_TIMEOUT_INFINITE);
        
        tmpNode = msgChHandle->msgQueueHead.next;
        
        while (tmpNode != &msgChHandle->msgQueueHead){
            msg = (Mag_Message_t *)list_entry(tmpNode, Mag_Message_t, node);
            if (MSG_CTRL_STOP == msg->ctrlType){
                rc = pthread_mutex_lock(&msgChHandle->lock);
                MAG_ASSERT(0 == rc);

                while (tmpNode != &msgChHandle->msgQueueHead){
                    list_del(tmpNode);
                    list_add(tmpNode, &msgChHandle->freeMsgListHead);
                    tmpNode = msgChHandle->msgQueueHead.next;
                }
                msgChHandle->stopped = MAG_TRUE;
                
                rc = pthread_mutex_unlock(&msgChHandle->lock);
                MAG_ASSERT(0 == rc);
                break;
            }else{
                msgChHandle->receiverFunc(msg->msgBody, msgChHandle->privData);

                rc = pthread_mutex_lock(&msgChHandle->lock);
                MAG_ASSERT(0 == rc);
                
                list_del(tmpNode);
                /*reuse the message*/
                list_add(tmpNode, &msgChHandle->freeMsgListHead);
                tmpNode = msgChHandle->msgQueueHead.next;

                rc = pthread_mutex_unlock(&msgChHandle->lock);
                MAG_ASSERT(0 == rc);
            }
        }
    }
}

MagErr_t Mag_MsgChannelReceiverAttach(MagMsgChannelHandle handle, fnMsgChanReceiver func, void *priv_data){
    int rc;

    if ((NULL == handle) || (NULL == func))
        return MAG_BadParameter;
    
    rc = pthread_mutex_lock(&handle->lock);
    MAG_ASSERT(0 == rc);

    if (NULL != handle->receiverFunc){
        AGILE_LOGE("Failed to attach the receiver func. (Error: has been registered)");
        return MAG_InvalidOperation;
    }
    
    if (pthread_create(&handle->ReceiverThread, NULL, Mag_MsgChannelProcessEntry, handle)){
        AGILE_LOGE("failed to create the message process thread. (error: %s)", strerror(errno));
        goto err_thread;
    }

    handle->privData     = priv_data;
    handle->receiverFunc = func;
    
err_thread:  
    rc = pthread_mutex_unlock(&handle->lock);
    MAG_ASSERT(0 == rc);
}

static Mag_Message_t *getMsgNode(MagMsgChannelHandle handle){
    Mag_Message_t *newMsg = NULL;
    List_t *tmpNode;
    int rc;
    MagErr_t ret;
    
    rc = pthread_mutex_lock(&handle->lock);
    MAG_ASSERT(0 == rc);
        
    tmpNode = handle->freeMsgListHead.next;

    if(tmpNode == &handle->freeMsgListHead){
        AGILE_LOGV("no nodes in the free list. malloc one");
        newMsg = (Mag_Message_t *)malloc(sizeof(Mag_Message_t));
        if(NULL == newMsg)
            goto err_memory;
        newMsg->msgBody = NULL;
    }else{
        AGILE_LOGV("get the free node from the list");
        list_del(tmpNode);
        newMsg = (Mag_Message_t *)list_entry(tmpNode, Mag_Message_t, node);    
    }
    
    newMsg->msgLen  = 0;
    INIT_LIST(&newMsg->node);
    
    rc = pthread_mutex_unlock(&handle->lock);
    MAG_ASSERT(0 == rc); 

    return newMsg;
    
err_memory: 
    if (newMsg)
        free(newMsg);

    if (newMsg->msgBody)
        free(newMsg->msgBody);
    
    return NULL;
}

static MagErr_t Mag_MsgChannelStop(MagMsgChannelHandle handle, Mag_MsgDetachFlag_t flag){
    Mag_Message_t *newMsg;
    int rc;
    
    if(NULL == handle)
        return MAG_BadParameter;
    
    newMsg = getMsgNode(handle);
    
    if(NULL == newMsg)
        return MAG_NoMemory;

    newMsg->ctrlType = MSG_CTRL_STOP;

    rc = pthread_mutex_lock(&handle->lock);
    MAG_ASSERT(0 == rc);

    if (MSG_FLUSH == flag){
        list_add_tail(&newMsg->node, &handle->msgQueueHead);
    }else{
        list_add(&newMsg->node, &handle->msgQueueHead);
    }

    rc = pthread_mutex_unlock(&handle->lock);
    MAG_ASSERT(0 == rc);
    
    Mag_SetEvent(handle->event);
    AGILE_LOGD("send out stop message");
    return MAG_ErrNone;
}

MagErr_t Mag_MsgChannelReceiverDettach(MagMsgChannelHandle handle, Mag_MsgDetachFlag_t flag){
    if(!handle->ReceiverThread){
        AGILE_LOGD("the Message Receiver is never attached");
        return MAG_Failure;
    }
    
    Mag_MsgChannelStop(handle, flag);
    pthread_join(handle->ReceiverThread, NULL);

    handle->ReceiverThread = (pthread_t)0;
    handle->receiverFunc   = NULL;
    handle->privData       = NULL;

    return MAG_ErrNone;
}

MagErr_t Mag_MsgChannelSend(MagMsgChannelHandle handle, const void *msg, const unsigned int msg_len){
    List_t *tmpNode;
    Mag_Message_t *newMsg;
    int rc;

    if ((NULL == msg) || (0 == msg_len) || (NULL == handle))
        return MAG_BadParameter;
    
    newMsg = getMsgNode(handle);

    if (NULL == newMsg)
        return MAG_NoMemory;
    
    if (NULL == newMsg->msgBody){
        newMsg->msgBody = (unsigned char *)malloc(msg_len);
        if (NULL == newMsg->msgBody){
            free(newMsg);
            return MAG_NoMemory;
        }
    }
    
    newMsg->msgLen   = msg_len;
    newMsg->ctrlType = MSG_CTRL_RUN;
    memcpy((void *)newMsg->msgBody, msg, msg_len);

    rc = pthread_mutex_lock(&handle->lock);
    MAG_ASSERT(0 == rc);
    
    /*it is FIFO*/
    list_add_tail(&newMsg->node, &handle->msgQueueHead);
    
    rc = pthread_mutex_unlock(&handle->lock);
    MAG_ASSERT(0 == rc); 
        
    Mag_SetEvent(handle->event);
    
    return MAG_ErrNone;
}



