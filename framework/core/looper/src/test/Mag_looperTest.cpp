#include "Mag_looper.h"

#include <stdlib.h>
#include <unistd.h>

#define LOOPER_NAME "TestLooper"

#define HANDLER_MAX_NUM 5

#define MSG_1_DELAY 0
#define MSG_2_DELAY 1100
#define MSG_3_DELAY 1200
#define MSG_4_DELAY 1300
#define MSG_5_DELAY 1400

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
        Msg_Test_5
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
            destroyHandler(&mMsgHandler[i]);
        }
    }
    if (mLooper != NULL){
        destroyLooper(&mLooper);
    }
}

void Comp_A::onMsg_Test_1(MagMessageHandle msg){
    boolean ret;
    char *value;
    char *sidx;
    i64 tnow;
    i64 tpost;

    tnow = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
    ret = msg->findInt64(msg, "tpost", &tpost);
    if (!ret){
        AGILE_LOGE("failed to find the tpost i64!");
        return;
    } 

    if ((tnow > tpost + MSG_1_DELAY + 500) || (tnow < tpost + MSG_1_DELAY - 500)){
        AGILE_LOGE("msg1: wrong delay action! tdiff %lld vs %d", tnow - tpost, MSG_1_DELAY);
    }

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
    // usleep(100000);
}

void Comp_A::onMsg_Test_2(MagMessageHandle msg){
    
    boolean ret;
    char *value;
    i32 idx;
    i64 tnow;
    i64 tpost;

    tnow = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
    ret = msg->findInt64(msg, "tpost", &tpost);
    if (!ret){
        AGILE_LOGE("failed to find the tpost i64!");
        return;
    } 
    
    if ((tnow > tpost + MSG_2_DELAY + 300) || (tnow < tpost + MSG_2_DELAY - 300)){
        AGILE_LOGE("msg2: wrong delay action! tdiff %lld vs %d", tnow - tpost, MSG_2_DELAY);
    }

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
    i64 tnow;
    i64 tpost;

    tnow = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
    ret = msg->findInt64(msg, "tpost", &tpost);
    if (!ret){
        AGILE_LOGE("failed to find the tpost i64!");
        return;
    } 
    
    if ((tnow > tpost + MSG_3_DELAY + 300) || (tnow < tpost + MSG_3_DELAY - 300)){
        AGILE_LOGE("msg3: wrong delay action! tdiff %lld vs %d", tnow - tpost, MSG_3_DELAY);
    }

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
    i64 tnow;
    i64 tpost;

    tnow = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
    ret = msg->findInt64(msg, "tpost", &tpost);
    if (!ret){
        AGILE_LOGE("failed to find the tpost i64!");
        return;
    } 
    
    if ((tnow > tpost + MSG_4_DELAY + 300) || (tnow < tpost + MSG_4_DELAY - 300)){
        AGILE_LOGE("msg4: wrong delay action! tdiff %lld vs %d", tnow - tpost, MSG_4_DELAY);
    }

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
    i64 tnow;
    i64 tpost;

    tnow = Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll;
    ret = msg->findInt64(msg, "tpost", &tpost);
    if (!ret){
        AGILE_LOGE("failed to find the tpost i64!");
        return;
    } 
    
    if ((tnow > tpost + MSG_5_DELAY + 300) || (tnow < tpost + MSG_5_DELAY - 300)){
        AGILE_LOGE("msg5: wrong delay action! tdiff %lld vs %d", tnow - tpost, MSG_5_DELAY);
    }

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

    return MAG_NO_ERROR;
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
        mLooper->setPriority(mLooper, MagLooper_Priority_High);
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
        Msg_reply
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
        destroyHandler(&mMsgHandler);
    }
    
    if (mLooper != NULL){
        destroyLooper(&mLooper);
    }
}

_status_t Comp_B::start(){
    getLooper();

    if (mLooper != NULL)
        mLooper->start(mLooper);

    return MAG_NO_ERROR;
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
    msg5->setMessage(msg5, "reply", reply, MAG_TRUE);
    
    obj->start();
    replyObj->start();
    
    //sleep(5);
    char buf[64];
    
    for (i = 0; i < 50000; i++){
        sprintf(buf, "num%d", i);
        msg1->setString(msg1, "num", buf);
        msg1->setInt64(msg1, "tpost", Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll);
        msg1->postMessage(msg1, MSG_1_DELAY); 

        msg2->setInt32(msg2, "num", i);
        msg1->setInt64(msg2, "tpost", Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll);
        msg2->postMessage(msg2, MSG_2_DELAY);

        msg3->setInt32(msg3, "num", i);
        msg1->setInt64(msg3, "tpost", Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll);
        msg3->postMessage(msg3, MSG_3_DELAY);

        msg4->setInt32(msg4, "num", i);
        msg1->setInt64(msg4, "tpost", Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll);
        msg4->postMessage(msg4, MSG_4_DELAY);

        msg5->setInt32(msg5, "num", i);
        msg1->setInt64(msg5, "tpost", Mag_GetSystemTime(MAG_SYSTEM_TIME_MONOTONIC) / 1000ll);
        msg5->postMessage(msg5, MSG_5_DELAY);

        // usleep(MSG_1_DELAY + MSG_2_DELAY + MSG_3_DELAY + MSG_4_DELAY + MSG_5_DELAY + 2000);
        usleep(MSG_5_DELAY + 2000);
    }
    
    obj->waitOnAllDone();
    
    AGILE_LOGD("begin to destroy the messages!");
    destroyMagMessage(&msg1);
    AGILE_LOGD("111");
    destroyMagMessage(&msg2);
    AGILE_LOGD("222");
    destroyMagMessage(&msg3);
    AGILE_LOGD("333");
    destroyMagMessage(&msg4);
    AGILE_LOGD("444");
    destroyMagMessage(&msg5);
    // AGILE_LOGD("before destroyMagMessage(reply)");
    // destroyMagMessage(reply);
    // AGILE_LOGD("after destroyMagMessage(reply)");
    delete obj;
    delete replyObj;
    AGILE_LOGD("exit!");
    return 0;
}

#undef LOOPER_NAME

