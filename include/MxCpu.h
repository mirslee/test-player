
#ifndef __MXCPU_H__
#define __MXCPU_H__

#ifdef __i386__
#define MX_i386 1
#else
#define MX_i386 0
#endif

#ifdef __x86_64__
#define MX_x86_64 1
#else
#define MX_x86_64 0
#endif

#ifdef __MMX__
#define MX_MMX 1
#else
#define MX_MMX 0
#endif

#ifdef __SSE__
#define MX_SSE 1
#else
#define MX_SSE 0
#endif

#ifdef __SSE2__
#define MX_SSE2 1
#else
#define MX_SSE2 0
#endif

#ifdef __SSSE3__
#define MX_SSE2 1
#else
#define MX_SSE2 0
#endif

#ifdef __SSE4_1__
#define MX_SSE2 1
#else
#define MX_SSE2 0
#endif

#ifdef __SSE4_2__
#define MX_SSE2 1
#else
#define MX_SSE2 0
#endif

#ifdef __SSE4A__
#define MX_SSE2 1
#else
#define MX_SSE2 0
#endif

#ifdef __AVX__
#define MX_AVX 1
#else
#define MX_AVX 0
#endif

#ifdef __AVX2__
#define MX_AVX2 1
#else
#define MX_AVX2 0
#endif

#ifdef __3dNOW__     /*AMD SIMD指令集*/
#define MX_3dNOW 1
#else
#define MX_3dNOW 0
#endif

#ifdef __XOP__
#define MX_XOP 1
#else
#define MX_XOP 0
#endif

#ifdef __FMA4__
#define MX_FMA4 1
#else
#define MX_FMA4 0
#endif


#endif //__MXCPU_H__
