#include "MagPlayerDriverFactory.h"
#include "MagPlayerDriverImpl.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magPlayerDriver"

MAG_SINGLETON_STATIC_INSTANCE(MagPlayerDriverFactory)

MagPlayerDriverFactory::MagPlayerDriverFactory(){

}

MagPlayerDriverFactory::~MagPlayerDriverFactory(){

}

MagPlayerDriverImplBase *MagPlayerDriverFactory::create(void *client, notify_client_callback_f cb){
	MagPlayerDriverImplBase *obj;

	obj = new MagPlayerDriverImpl(client, cb);
	return obj;
}