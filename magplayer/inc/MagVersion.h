#ifndef __MAG_VERSION_H__
#define __MAG_VERSION_H__

#include "framework/Mag_macros.h"
/*z2: 0.4.x
 *a0: 0.5.x
*/
#define LIBMAGPLAYER_VERSION_MAJOR 0
#define LIBMAGPLAYER_VERSION_MINOR 5
#define LIBMAGPLAYER_VERSION_MICRO 10

#define LIBMAGPLAYER_VERSION_INT MAG_VERSION_INT(LIBMAGPLAYER_VERSION_MAJOR, \
                                                 LIBMAGPLAYER_VERSION_MINOR, \
                                                 LIBMAGPLAYER_VERSION_MICRO)
#define LIBMAGPLAYER_VERSION     MAG_VERSION(LIBMAGPLAYER_VERSION_MAJOR,   \
                                            LIBMAGPLAYER_VERSION_MINOR,   \
                                            LIBMAGPLAYER_VERSION_MICRO)
#define LIBMAGPLAYER_BUILD       LIBMAGPLAYER_VERSION_INT

#define LIBMAGPLAYER_IDENT       "Lmagplayer" MAG_STRINGIFY(LIBMAGPLAYER_VERSION)

#endif