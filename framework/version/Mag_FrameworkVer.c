#include "Mag_FrameworkVer.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magfw_Common"

ui32 Mag_getFrameWorkVer(void){
	AGILE_LOGI("%s", LIBMAGFW_IDENT);
	return (LIBMAGFW_VERSION_INT);
}