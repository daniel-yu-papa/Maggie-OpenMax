#include "Mag_looper.h"

#include <stdlib.h>
#include <unistd.h>

#define LOOPER_NAME "TestLooper"

#define HANDLER_MAX_NUM 5

static ui32 handler_index = 0;

class Comp_A{
public:
    Comp_A();
    ~Comp_A();

    MagMessageHandle createMessage(ui32 what, ui32 handlerID);
    _status_t        start();
    void             waitOnAllDone();
    
    enum{
        Msg_Test_1,
        Msg_Test_2,
        Msg_Test_3,
        Msg_Test_4,
        Msg_Test_5,
    };
    
private:
    void onMsg_Test_1(MagMessageHandle msg);
    void onMsg_Test_2(MagMessageHandle msg);
    void onMsg_Test_3(MagMessageHandle msg);
    void onMsg_Test_4(MagMessageHandle msg);
    void onMsg_Test_5(MagMessageHandle msg);
    
    _status_t getLooper();
    static void onMessageReceived(const MagMessageHandle msg, void *priv);
    
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler[HANDLER_MAX_NUM];
};


Comp_A::Comp_A():
        mLooper(NULL){
    i32 i;
    for (i = 0; i < HANDLER_MAX_NUM; i++)
        mMsgHandler[i] = NULL;
}

Comp_A::~Comp_A(){
    i32 i;

    for (i = 0; i < HANDLER_MAX_NUM; i++){
        if (mMsgHandler[i] != NULL){
            destroyHandler(mMsgHandler[i]);
        }
    }
    if (mLooper != NULL){
        destroyLooper(mLooper);
    }
}

void Comp_A::onMsg_Test_1(MagMessageHandle msg){
    boolean ret;
    char *value;
    char *sidx;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 

    ret = msg->findString(msg, "num", &sidx);
    if (!ret){
        AGILE_LOGE("failed to find the idx string!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action[index: %s]!", msg->mTarget, value, sidx);
    usleep(100000);
}

void Comp_A::onMsg_Test_2(MagMessageHandle msg){
    
    boolean ret;
    char *value;
    i32 idx;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 

    ret = msg->findInt32(msg, "num", &idx);
    if (!ret){
        AGILE_LOGE("failed to find the idx int32!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action[index: %d]!", msg->mTarget, value, idx);
    //usleep(200000);
}

void Comp_A::onMsg_Test_3(MagMessageHandle msg){
    boolean ret;
    char *value;
    i32 idx;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 

    ret = msg->findInt32(msg, "num", &idx);
    if (!ret){
        AGILE_LOGE("failed to find the idx int32!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action[index: %d]!", msg->mTarget, value, idx);
    //usleep(300000);
}

void Comp_A::onMsg_Test_4(MagMessageHandle msg){
    boolean ret;
    char *value;
    i32 idx;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 

    ret = msg->findInt32(msg, "num", &idx);
    if (!ret){
        AGILE_LOGE("failed to find the idx int32!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action[index: %d]!", msg->mTarget, value, idx);
    //usleep(400000);
}

void Comp_A::onMsg_Test_5(MagMessageHandle msg){
    boolean ret;
    char *value;
    i32 idx;
    MagMessageHandle back_msg;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 

    ret = msg->findInt32(msg, "num", &idx);
    if (!ret){
        AGILE_LOGE("failed to find the idx int32!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action[index: %d]!", msg->mTarget, value, idx);

    ret = msg->findMessage(msg, "reply", &back_msg);
    if (!ret){
        AGILE_LOGE("failed to find the reply message!");
        return;
    } 
    back_msg->setString(back_msg, "caller", "Msg_Test_5");
    back_msg->postMessage(back_msg, 0);
    //usleep(500000);
}

_status_t Comp_A::start(){
    getLooper();

    if (mLooper != NULL)
        mLooper->start(mLooper);
}

void Comp_A::waitOnAllDone(){
    if (mLooper != NULL)
        mLooper->waitOnAllDone(mLooper);
}

void Comp_A::onMessageReceived(const MagMessageHandle msg, void *priv){
    Comp_A *thiz = (Comp_A *)priv;

    switch (msg->what(msg)) {
        case Msg_Test_1:
            thiz->onMsg_Test_1(msg);
            break;
            
        case Msg_Test_2:
            thiz->onMsg_Test_2(msg);
            break;

        case Msg_Test_3:
            thiz->onMsg_Test_3(msg);
            break;

        case Msg_Test_4:
            thiz->onMsg_Test_4(msg);
            break;

        case Msg_Test_5:
            thiz->onMsg_Test_5(msg);
            break;

        default:
            break;
    };
}

MagMessageHandle Comp_A::createMessage(ui32 what, ui32 handlerID) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler[handlerID]->id(mMsgHandler[handlerID]));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t Comp_A::getLooper(){
    if (NULL == mLooper){
        mLooper = createLooper(LOOPER_NAME);
    }

    if (NULL != mLooper){
        i32 i;
        for (i = 0; i < HANDLER_MAX_NUM; i++){
            if (NULL == mMsgHandler[i]){
                mMsgHandler[i] = createHandler(mLooper, onMessageReceived, (void *)this);

                if (NULL != mMsgHandler[i]){
                    mLooper->registerHandler(mLooper, mMsgHandler[i]);
                }else{
                    AGILE_LOGE("failed to create Handler");
                    return MAG_NO_INIT;
                }
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}


class Comp_B{
public:
    Comp_B();
    ~Comp_B();

    MagMessageHandle createMessage(ui32 what, ui32 handlerID);
    _status_t        start();
    
    enum{
        Msg_reply,
    };
    
private:
    void onMsg_reply(MagMessageHandle msg);
    
    _status_t getLooper();
    static void onMessageReceived(const MagMessageHandle msg, void *priv);
    
    MagLooperHandle  mLooper;
    MagHandlerHandle mMsgHandler;
};

Comp_B::Comp_B():
        mLooper(NULL),
        mMsgHandler(NULL){
}

Comp_B::~Comp_B(){
    if (mMsgHandler != NULL){
        destroyHandler(mMsgHandler);
    }
    
    if (mLooper != NULL){
        destroyLooper(mLooper);
    }
}

_status_t Comp_B::start(){
    getLooper();

    if (mLooper != NULL)
        mLooper->start(mLooper);
}

void Comp_B::onMsg_reply(MagMessageHandle msg){
    boolean ret;
    char *value;
    
    ret = msg->findString(msg, "caller", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 
    AGILE_LOGD("handler %d: caller %s do the message action!", msg->mTarget, value);
}

void Comp_B::onMessageReceived(const MagMessageHandle msg, void *priv){
    Comp_B *thiz = (Comp_B *)priv;

    switch (msg->what(msg)) {
        case Msg_reply:
            thiz->onMsg_reply(msg);
            break;
    
        default:
            break;
    };
}

MagMessageHandle Comp_B::createMessage(ui32 what, ui32 handlerID) {
    getLooper();
    
    MagMessageHandle msg = createMagMessage(mLooper, what, mMsgHandler->id(mMsgHandler));
    if (msg == NULL){
        AGILE_LOGE("failed to create the message");
    }
    return msg;
}

_status_t Comp_B::getLooper(){
    if (NULL == mLooper){
        mLooper = createLooper(LOOPER_NAME);
    }

    if (NULL != mLooper){
        if (NULL == mMsgHandler){
            mMsgHandler = createHandler(mLooper, onMessageReceived, (void *)this);

            if (NULL != mMsgHandler){
                mLooper->registerHandler(mLooper, mMsgHandler);
            }else{
                AGILE_LOGE("failed to create Handler");
                return MAG_NO_INIT;
            }
        }
    }else{
        AGILE_LOGE("failed to create Looper: %s", LOOPER_NAME);
        return MAG_NO_INIT;
    }
    return MAG_NO_ERROR;
}

int main(){
    i32 i;
    MagMessageHandle msg1;
    MagMessageHandle msg2;
    MagMessageHandle msg3;
    MagMessageHandle msg4;
    MagMessageHandle msg5;

    MagMessageHandle reply;
    
    AGILE_LOGE("enter test_looper app");
    
    Comp_A *obj = new Comp_A();
    Comp_B *replyObj = new Comp_B();
    
    msg1 = obj->createMessage(Comp_A::Msg_Test_1, 0);
    msg1->setString(msg1, "msg", "Msg_Test_1");

    msg2 = obj->createMessage(Comp_A::Msg_Test_2, 1);
    msg2->setString(msg2, "msg", "Msg_Test_2");

    msg3 = obj->createMessage(Comp_A::Msg_Test_3, 2);
    msg3->setString(msg3, "msg", "Msg_Test_3");

    msg4 = obj->createMessage(Comp_A::Msg_Test_4, 3);
    msg4->setString(msg4, "msg", "Msg_Test_4");

    msg5 = obj->createMessage(Comp_A::Msg_Test_5, 4);
    msg5->setString(msg5, "msg", "Msg_Test_5");
    reply = replyObj->createMessage(Comp_B::Msg_reply, 0);
    msg5->setMessage(msg5, "reply", reply);
    
    obj->start();
    replyObj->start();
    
    //sleep(5);
    char buf[64];
    for (i = 0; i < 10; i++){
        sprintf(buf, "num%d", i);
        msg1->setString(msg1, "num", buf);
        msg1->postMessage(msg1, 200);
        usleep(10000);

        msg2->setInt32(msg2, "num", i);
        msg2->postMessage(msg2, 0);
        usleep(2000);

        msg3->setInt32(msg3, "num", i);
        msg3->postMessage(msg3, 100);
        usleep(3000);

        msg4->setInt32(msg4, "num", i);
        msg4->postMessage(msg4, 50);
        usleep(400);

        msg5->setInt32(msg5, "num", i);
        msg5->postMessage(msg5, 400);
    }
    
    obj->waitOnAllDone();
    
    destroyMagMessage(msg1);
    destroyMagMessage(msg2);
    destroyMagMessage(msg3);
    destroyMagMessage(msg4);
    destroyMagMessage(msg5);
    //destroyMagMessage(reply);
    delete obj;
    delete replyObj;
    return 0;
}

#undef LOOPER_NAME

