#ifndef __MXCOMMON_H__
#define __MXCOMMON_H__

#ifdef WORDS_BIGENDIAN
#   define MX_FOURCC( a, b, c, d ) \
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

#ifdef HAVE_ATTRIBUTE_PACKED
#   define ATTR_PACKED __attribute__((__packed__))
#elif defined(__SUNPRO_C) || defined(_MSC_VER)
#   pragma pack(1)
#   define ATTR_PACKED
#elif defined(__APPLE__)
#   pragma pack(push, 1)
#   define ATTR_PACKED
#else
#   error FIXME
#endif

static inline uint32_t GetDWLE (const void *p)
{
    uint32_t x;
    
    memcpy (&x, p, sizeof (x));
#ifdef WORDS_BIGENDIAN
    x = bswap32 (x);
#endif
    return x;
}

#endif //__MXCOMMON_H__
