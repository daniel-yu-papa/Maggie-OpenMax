#ifndef __MAG_MACROS_H__
#define __MAG_MACROS_H__

#define MAG_TOSTRING(s)      #s
#define MAG_STRINGIFY(s)     MAG_TOSTRING(s)

#define MAG_VERSION_INT(a, b, c) (a<<16 | b<<8 | c)
#define MAG_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define MAG_VERSION(a, b, c) MAG_VERSION_DOT(a, b, c)


#endif