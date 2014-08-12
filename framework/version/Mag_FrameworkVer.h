#ifndef __MAG_FRAMEWORK_VERSION_H__
#define __MAG_FRAMEWORK_VERSION_H__

#include "Mag_pub_type.h"
#include "Mag_macros.h"
#include "Mag_agilelog.h"

#ifdef __cplusplus
extern "C" {
#endif
#define LIBMAGFW_VERSION_MAJOR 0
#define LIBMAGFW_VERSION_MINOR 2
#define LIBMAGFW_VERSION_MICRO 3

#define LIBMAGFW_VERSION_INT MAG_VERSION_INT(LIBMAGFW_VERSION_MAJOR, \
                                                 LIBMAGFW_VERSION_MINOR, \
                                                 LIBMAGFW_VERSION_MICRO)
#define LIBMAGFW_VERSION     MAG_VERSION(LIBMAGFW_VERSION_MAJOR,   \
                                            LIBMAGFW_VERSION_MINOR,   \
                                            LIBMAGFW_VERSION_MICRO)
#define LIBMAGFW_BUILD       LIBMAGFW_VERSION_INT

#define LIBMAGFW_IDENT       "Lmagfw" MAG_STRINGIFY(LIBMAGFW_VERSION)

ui32 Mag_getFrameWorkVer(void);

#ifdef __cplusplus
}
#endif

#endif