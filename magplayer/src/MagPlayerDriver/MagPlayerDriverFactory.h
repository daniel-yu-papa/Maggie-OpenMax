#ifndef __MAGPLAYER_DRIVER_FACTORY_H__
#define __MAGPLAYER_DRIVER_FACTORY_H__

#include "MagSingleton.h"
#include "MagPlayerDriverImplBase.h"

class MagPlayerDriverFactory : public MagSingleton<MagPlayerDriverFactory>{
    friend class MagSingleton<MagPlayerDriverFactory>;
public:
	MagPlayerDriverFactory();
	virtual ~MagPlayerDriverFactory();

	MagPlayerDriverImplBase *create(void *client, notify_client_callback_f cb);
};


#endif