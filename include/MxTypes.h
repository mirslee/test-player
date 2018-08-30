
#ifndef __MXTYPES_H__
#define __MXTYPES_H__

typedef char int8;
typedef char _int8;
typedef char __int8;
typedef unsigned char uint8;
typedef unsigned char _uint8;
typedef unsigned char __uint8;

typedef short int16;
typedef short _int16;
typedef short __int16;
typedef unsigned short uint16;
typedef unsigned short _uint16;
typedef unsigned short __uint16;

typedef long int32;
typedef long _int32;
typedef long __int32;
typedef unsigned long uint32;
typedef unsigned long _uint32;
typedef unsigned long __uint32;

typedef unsigned int uint;
typedef long long int64;
#ifndef _WIN32
typedef long long _int64;
typedef long long __int64;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#endif
typedef unsigned long long uint64;
typedef unsigned long long _uint64;
typedef unsigned long long __uint64;

typedef long LONG;
typedef unsigned long ULONG;

//typedef void* mxuvoidptr;

template <int> struct MxIntegerSize;
template <> struct MxIntegerSize<4> {typedef long IntegerSize;  typedef unsigned long UIntegerSize;};
template <> struct MxIntegerSize<8> {typedef long long IntegerSize; typedef unsigned long long UIntegerSize;};
typedef MxIntegerSize<sizeof(void*)>::IntegerSize mxvoidptr;
typedef MxIntegerSize<sizeof(void*)>::UIntegerSize mxuvoidptr;

/*template <int> struct MxIntegerSize;
template <>    struct MxIntegerSize<1> { typedef BYTE  UIntegerSize; typedef char  IntegerSize; };
template <>    struct MxIntegerSize<2> { typedef WORD UIntegerSize; typedef short IntegerSize; };
template <>    struct MxIntegerSize<4> { typedef DWORD UIntegerSize; typedef int IntegerSize; };
template <>    struct MxIntegerSize<8> { typedef __uint64 UIntegerSize; typedef __int64 IntegerSize; };
template <class T> struct MxIntegerSizeOf : MxIntegerSize<(int)sizeof(T)> { };
typedef MxIntegerSizeOf<void*>::IntegerSize mxvoidptr;
typedef MxIntegerSizeOf<void*>::UIntegerSize mxuvoidptr;*/

#ifndef _WIN32
#define HANDLE void*
#endif

#endif //__MXTYPES_H__
