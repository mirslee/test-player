
#ifndef MXCOMMON_H
#define MXCOMMON_H

/*****************************************************************************
 * Required system headers
 *****************************************************************************/
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>

#ifndef __cplusplus
# include <stdbool.h>
#endif

/*****************************************************************************
 * Compilers definitions
 *****************************************************************************/
/* Helper for GCC version checks */
#ifdef __GNUC__
# define MX_GCC_VERSION(maj,min) \
((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))
#else
# define MX_GCC_VERSION(maj,min) (0)
#endif

/* Try to fix format strings for all versions of mingw and mingw64 */
#if defined( _WIN32 ) && defined( __USE_MINGW_ANSI_STDIO )
    #undef PRId64
    #define PRId64 "lld"
    #undef PRIi64
    #define PRIi64 "lli"
    #undef PRIu64
    #define PRIu64 "llu"
    #undef PRIo64
    #define PRIo64 "llo"
    #undef PRIx64
    #define PRIx64 "llx"
    #define snprintf __mingw_snprintf
    #define vsnprintf __mingw_vsnprintf
    #define swprintf _snwprintf
#endif

/* Function attributes for compiler warnings */
#ifdef __GNUC__
    # define MX_DEPRECATED __attribute__((deprecated))
    # if MX_GCC_VERSION(6,0)
        #  define MX_DEPRECATED_ENUM __attribute__((deprecated))
    # else
        #  define MX_DEPRECATED_ENUM
    # endif

    # if defined( _WIN32 )
        #  define MX_FORMAT(x,y) __attribute__ ((format(gnu_printf,x,y)))
    # else
        #  define MX_FORMAT(x,y) __attribute__ ((format(printf,x,y)))
    # endif
    # define MX_FORMAT_ARG(x) __attribute__ ((format_arg(x)))
    # define MX_MALLOC __attribute__ ((malloc))
    # define MX_USED __attribute__ ((warn_unused_result))

#else
    # define MX_DEPRECATED
    # define MX_DEPRECATED_ENUM
    # define MX_FORMAT(x,y)
    # define MX_FORMAT_ARG(x)
    # define MX_MALLOC
    # define MX_USED
#endif

/* Branch prediction */
#ifdef __GNUC__
    # define likely(p)     __builtin_expect(!!(p), 1)
    # define unlikely(p)   __builtin_expect(!!(p), 0)
    # define unreachable() __builtin_unreachable()
#else
    # define likely(p)     (!!(p))
    # define unlikely(p)   (!!(p))
    # define unreachable() ((void)0)
#endif

#define MX_assert_unreachable() (assert(!"unreachable"), unreachable())

/* Linkage */
#ifdef __cplusplus
    # define MX_EXTERN extern "C"
#else
    # define MX_EXTERN
#endif

#if defined (_WIN32) && defined (DLL_EXPORT)
    # define MX_EXPORT __declspec(dllexport)
#elif defined (__GNUC__)
    # define MX_EXPORT __attribute__((visibility("default")))
#else
    # define MX_EXPORT
#endif

#define MX_API MX_EXTERN MX_EXPORT

/*****************************************************************************
 * Basic types definitions
 *****************************************************************************/
/**
 * High precision date or time interval
 *
 * Store a high precision date or time interval. The maximum precision is the
 * microsecond, and a 64 bits integer is used to avoid overflows (maximum
 * time interval is then 292271 years, which should be long enough for any
 * video). Dates are stored as microseconds since a common date (usually the
 * epoch). Note that date and time intervals can be manipulated using regular
 * arithmetic operators, and that no special functions are required.
 */
typedef int64_t mtime_t;

typedef uint32_t MxFourcc;
#ifdef WORDS_BIGENDIAN
#define MX_FOURCC( a, b, c, d )
( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) \
| ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#   define MX_TWOCC( a, b ) \
( (uint16_t)(b) | ( (uint16_t)(a) << 8 ) )

#else
#   define MX_FOURCC( a, b, c, d ) \
( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
| ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#   define MX_TWOCC( a, b ) \
( (uint16_t)(a) | ( (uint16_t)(b) << 8 ) )

#endif

static inline void mxFourccToChar( MxFourcc fcc, char *psz_fourcc )
{
    memcpy( psz_fourcc, &fcc, 4 );
}

#define MX_UNUSED(x) (void)(x)

#endif //MXCOMMON_H
