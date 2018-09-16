#ifndef __MXCPU_H__
#define __MXCPU_H__

#include "MxCommon.h"

MX_API unsigned mxCpu();
MX_API void mxCpuInit();

# if defined (__i386__) || defined (__x86_64__)
#  define HAVE_FPU 1
#  define MX_CPU_MMX    0x00000008
#  define MX_CPU_3dNOW  0x00000010
#  define MX_CPU_MMXEXT 0x00000020
#  define MX_CPU_SSE    0x00000040
#  define MX_CPU_SSE2   0x00000080
#  define MX_CPU_SSE3   0x00000100
#  define MX_CPU_SSSE3  0x00000200
#  define MX_CPU_SSE4_1 0x00000400
#  define MX_CPU_SSE4_2 0x00000800
#  define MX_CPU_SSE4A  0x00001000
#  define MX_CPU_AVX    0x00002000
#  define MX_CPU_AVX2   0x00004000
#  define MX_CPU_XOP    0x00008000
#  define MX_CPU_FMA4   0x00010000

# if defined (__MMX__)
#  define mxCpuMMX() (1)
#  define MX_MMX
# else
#  define mxCpuMMX() ((mxCpu() & MX_CPU_MMX) != 0)
#  define MX_MMX __attribute__ ((__target__ ("mmx")))
# endif

# if defined (__SSE__)
#  define mxCpuMMXExt() (1)
#  define mxCpuSSE() (1)
#  define MX_SSE
# else
#  define mxCpuMMXExt() ((mxCpu() & MX_CPU_MMXEXT) != 0)
#  define mxCpuSSE() ((mxCpu() & MX_CPU_SSE) != 0)
#  define MX_SSE __attribute__ ((__target__ ("sse")))
# endif

# ifdef __SSE2__
#  define mxCpuSSE2() (1)
# else
#  define mxCpuSSE2() ((mxCpu() & MX_CPU_SSE2) != 0)
# endif

# ifdef __SSE3__
#  define mxCpuSSE3() (1)
# else
#  define mxCpuSSE3() ((mxCpu() & MX_CPU_SSE3) != 0)
# endif

# ifdef __SSSE3__
#  define mxCpuSSSE3() (1)
# else
#  define mxCpuSSSE3() ((mxCpu() & MX_CPU_SSSE3) != 0)
# endif

# ifdef __SSE4_1__
#  define mxCpuSSE4_1() (1)
# else
#  define mxCpuSSE4_1() ((mxCpu() & MX_CPU_SSE4_1) != 0)
# endif

# ifdef __SSE4_2__
#  define mxCpuSSE4_2() (1)
# else
#  define mxCpuSSE4_2() ((mxCpu() & MX_CPU_SSE4_2) != 0)
# endif

# ifdef __SSE4A__
#  define mxCpuSSE4A() (1)
# else
#  define mxCpuSSE4A() ((mxCpu() & MX_CPU_SSE4A) != 0)
# endif

# ifdef __AVX__
#  define mxCpuAVX() (1)
# else
#  define mxCpuAVX() ((mxCpu() & MX_CPU_AVX) != 0)
# endif

# ifdef __AVX2__
#  define mxCpuAVX2() (1)
# else
#  define mxCpuAVX2() ((mxCpu() & MX_CPU_AVX2) != 0)
# endif

# ifdef __3dNOW__
#  define mxCpu3dNOW() (1)
# else
#  define mxCpu3dNOW() ((mxCpu() & MX_CPU_3dNOW) != 0)
# endif

# ifdef __XOP__
#  define mxCpuXOP() (1)
# else
#  define mxCpuXOP() ((mxCpu() & MX_CPU_XOP) != 0)
# endif

# ifdef __FMA4__
#  define mxCpuFMA4() (1)
# else
#  define mxCpuFMA4() ((mxCpu() & MX_CPU_FMA4) != 0)
# endif

# elif defined (__ppc__) || defined (__ppc64__) || defined (__powerpc__)
#  define HAVE_FPU 1
#  define MX_CPU_ALTIVEC 2

#  ifdef ALTIVEC
#   define mxCpuALTIVEC() (1)
#  else
#   define MX_CPU_ALTIVEC() ((mxCpu() & MX_CPU_ALTIVEC) != 0)
#  endif

# elif defined (__arm__)
#  if defined (__VFP_FP__) && !defined (__SOFTFP__)
#   define HAVE_FPU 1
#  else
#   define HAVE_FPU 0
#  endif
#  define MX_CPU_ARMv6    4
#  define MX_CPU_ARM_NEON 2

#  if defined (__ARM_ARCH_7A__)
#   define MX_CPU_ARM_ARCH 7
#  elif defined (__ARM_ARCH_6__) || defined (__ARM_ARCH_6T2__)
#   define MX_CPU_ARM_ARCH 6
#  else
#   define MX_CPU_ARM_ARCH 4
#  endif

#  if (MX_CPU_ARM_ARCH >= 6)
#   define MX_CPU_ARMv6() (1)
#  else
#   define MX_CPU_ARMv6() ((mxCpu() & MX_CPU_ARMv6) != 0)
#  endif

#  ifdef __ARM_NEON__
#   define mxCpuARM_NEON() (1)
#  else
#   define mxCpuARM_NEON() ((mxCpu() & MX_CPU_ARM_NEON) != 0)
#  endif

# elif defined (__aarch64__)
#  define HAVE_FPU 1
// NEON is mandatory for general purpose ARMv8-a CPUs
#  define MX_CPU_ARM64_NEON() (1)

# elif defined (__sparc__)
#  define HAVE_FPU 1

# elif defined (__mips_hard_float)
#  define HAVE_FPU 1

# else
/**
 * Are single precision floating point operations "fast"?
 * If this preprocessor constant is zero, floating point should be avoided
 * (especially relevant for audio codecs).
 */
#  define HAVE_FPU 0

# endif

/*#ifdef __i386__
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

#ifdef __3dNOW__
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
#endif*/


#endif //__MXCPU_H__
