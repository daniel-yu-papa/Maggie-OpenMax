#ifndef __MAG_ANDROID_PLATFORM_H__
#define __MAG_ANDROID_PLATFORM_H__

#include <sys/atomics.h>

#define GET_POINTER(obj) obj.get()

#define ATOMIC_INC(v) __atomic_inc(v)

#define ATOMIC_DEC(v) __atomic_dec(v)

#endif