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
    _status_t        postMessage(MagMessageHandle msg, ui64 delay);
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
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action!", msg->mTarget, value);
    //usleep(100000);
}

void Comp_A::onMsg_Test_2(MagMessageHandle msg){
    
    boolean ret;
    char *value;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action!", msg->mTarget, value);
    //usleep(200000);
}

void Comp_A::onMsg_Test_3(MagMessageHandle msg){
    boolean ret;
    char *value;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action!", msg->mTarget, value);
    //usleep(300000);
}

void Comp_A::onMsg_Test_4(MagMessageHandle msg){
    boolean ret;
    char *value;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action!", msg->mTarget, value);
    //usleep(400000);
}

void Comp_A::onMsg_Test_5(MagMessageHandle msg){
    boolean ret;
    char *value;
    
    ret = msg->findString(msg, "msg", &value);
    if (!ret){
        AGILE_LOGE("failed to find the msg string!");
        return;
    } 
    AGILE_LOGD("handler %d: do message %s action!", msg->mTarget, value);
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

_status_t Comp_A::postMessage(MagMessageHandle msg, ui64 delay){
    if ((NULL == mLooper) || (NULL == mMsgHandler)){
        AGILE_LOGE("Looper is not running correctly(looper=0x%p, handler=0x%p)", mLooper, mMsgHandler);
        return MAG_NO_INIT;
    }

    mLooper->postMessage(mLooper, msg, delay);
    return MAG_NO_ERROR;
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

int main(){
    i32 i;
    MagMessageHandle msg1;
    MagMessageHandle msg2;
    MagMessageHandle msg3;
    MagMessageHandle msg4;
    MagMessageHandle msg5;

    AGILE_LOGE("enter test_looper app");
    
    Comp_A *obj = new Comp_A();

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
    
    obj->start();
    sleep(5);
    
    for (i = 0; i < 20000; i++){
        msg1->postMessage(msg1, 200);
        usleep(10000);
        msg2->postMessage(msg2, 0);
        usleep(2000);
        msg3->postMessage(msg3, 100);
        usleep(3000);
        msg4->postMessage(msg4, 50);
        usleep(400);
        msg5->postMessage(msg5, 400);
    }
    
    obj->waitOnAllDone();
    
    destroyMagMessage(msg1);
    destroyMagMessage(msg2);
    destroyMagMessage(msg3);
    destroyMagMessage(msg4);
    destroyMagMessage(msg5);

    delete obj;
    return 0;
}

#undef LOOPER_NAME

