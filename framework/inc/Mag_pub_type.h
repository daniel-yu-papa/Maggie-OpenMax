#ifndef __MAG_PUB_TYPE_H__
#define __MAG_PUB_TYPE_H__

#ifdef __cplusplus
extern "C" 
{
#endif

typedef enum{
    MAG_FALSE = 0,
    MAG_TRUE = 1
}MAG_BOOL_t;

/*====== includes ======*/
#ifdef __C99_standard_integer_definitions_not_present__

typedef unsigned char      ui8;        /*///< Unsigned 8-bit integer*/
typedef unsigned short     ui16;       /*///< Unsigned 16-bit integer*/
typedef unsigned int       ui32;       /*///< Unsigned 32-bit integer*/
typedef unsigned long long ui64;       /*///< Unsigned 64-bit integer*/

typedef char               i8;         /*///< Signed 8-bit integer*/
typedef signed short       i16;        /*///< Signed 16-bit integer*/
typedef signed int         i32;        /*///< Signed 32-bit integer*/
typedef signed long long   i64;        /*///< Signed 64-bit integer*/

typedef volatile ui8       io8;        /*///< 8-bit I/O Port*/
typedef volatile ui16      io16;       /*///< 16-bit I/O Port*/
typedef volatile ui32      io32;       /*///< 32-bit I/O Port*/
typedef volatile ui64      io64;       /*///< 64-bit I/O Port*/

typedef float              fp32;       /*///< 32-bit floating point number*/
typedef double             fp64;       /*///< 64-bit floating point number*/

typedef ui8                b8;         /*///< 8-bit boolean value*/
typedef ui32               b32;        /*///< 32-bit boolean value*/

typedef MAG_BOOL_t         boolean;    /*///< 'bool' for C-language*/

typedef ui32               _size_t;    

typedef i32                _status_t;

#else /*__C99_standard_integer_definitions__*/

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

/*====== defines ======*/
typedef uint8_t            ui8;
typedef uint16_t           ui16;       /*///< Unsigned 16-bit integer*/
typedef uint32_t           ui32;       /*///< Unsigned 32-bit integer*/
typedef uint64_t           ui64;       /*///< Unsigned 64-bit integer*/

typedef int8_t             i8;         /*///< Signed 8-bit integer*/
typedef int16_t            i16;        /*///< Signed 16-bit integer*/
typedef int32_t            i32;        /*///< Signed 32-bit integer*/
typedef int64_t            i64;        /*///< Signed 64-bit integer*/

typedef float              fp32;       /*///< 32-bit floating point number*/
typedef double             fp64;       /*///< 64-bit floating point number*/

typedef volatile ui8       io8;        /*///< 8-bit I/O Port*/
typedef volatile ui16      io16;       /*///< 16-bit I/O Port*/
typedef volatile ui32      io32;       /*///< 32-bit I/O Port*/
typedef volatile ui64      io64;       /*///< 64-bit I/O Port*/

typedef ui8                b8;         /*///< 8-bit boolean value*/
typedef ui32               b32;        /*///< 32-bit boolean value*/

typedef MAG_BOOL_t         boolean;    /*///< 'bool' for C-language*/

typedef size_t             _size_t;

typedef int32_t            _status_t;

#endif

#ifdef __cplusplus
}
#endif

#endif
