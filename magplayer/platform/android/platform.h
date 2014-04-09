#ifndef __ANDROID_PLATFORM_H__
#define __ANDROID_PLATFORM_H__

#define GET_POINTER(obj) obj.get()

#define ATOMIC_INC(v) android_atomic_inc(v)

#define ATOMIC_DEC(v) android_atomic_dec(v)

#endif