
#ifndef __MXTYPES_H__
#define __MXTYPES_H__


typedef unsigned int uint;
typedef long long int64;
#ifndef _WIN32
typedef long long _int64;
typedef long long __int64;
#endif
typedef unsigned long long uint64;
typedef unsigned long long _uint64;
typedef unsigned long long __uint64;

typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;

//typedef void* mxuvoidptr;

/*template <int> struct MxIntegerSize;
template <> struct MxIntegerSize<4> {typedef long IntegerSize;  typedef unsigned long UIntegerSize;};
template <> struct MxIntegerSize<8> {typedef long long IntegerSize; typedef unsigned long long UIntegerSize;};
typedef MxIntegerSize<sizeof(void*)>::IntegerSize mxvoidptr;
typedef MxIntegerSize<sizeof(void*)>::UIntegerSize mxuvoidptr;*/

template <int> struct MxIntegerSize;
template <>    struct MxIntegerSize<1> { typedef BYTE  UIntegerSize; typedef char  IntegerSize; };
template <>    struct MxIntegerSize<2> { typedef WORD UIntegerSize; typedef short IntegerSize; };
template <>    struct MxIntegerSize<4> { typedef DWORD UIntegerSize; typedef int IntegerSize; };
template <>    struct MxIntegerSize<8> { typedef __uint64 UIntegerSize; typedef __int64 IntegerSize; };
template <class T> struct MxIntegerSizeOf : MxIntegerSize<(int)sizeof(T)> { };
typedef MxIntegerSizeOf<void*>::IntegerSize mxvoidptr;
typedef MxIntegerSizeOf<void*>::UIntegerSize mxuvoidptr;

#ifndef _WIN32
#define HANDLE void*
#endif

#endif //__MXTYPES_H__
