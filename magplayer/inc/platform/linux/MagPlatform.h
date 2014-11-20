#ifndef __MAG_LINUX_PLATFORM_H__
#define __MAG_LINUX_PLATFORM_H__

#define GET_POINTER(obj) obj.get()

#define ATOMIC_INC(v) __sync_fetch_and_add(v, 1)

#define ATOMIC_DEC(v) __sync_fetch_and_sub(v, 1)

#endif