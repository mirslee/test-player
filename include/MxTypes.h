
#ifndef __MXTYPES_H__
#define __MXTYPES_H__


typedef unsigned int uint;
typedef long long int64;
typedef long long _int64;
typedef long long __int64;
typedef unsigned long long uint64;
typedef unsigned long long _uint64;
typedef unsigned long long __uint64;

typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;

//typedef void* mxuvoidptr;

template <int> struct inttype11;
template <> struct inttype11<2> {typedef short x;typedef unsigned short ux;};
template <> struct inttype11<4> {typedef int x;typedef unsigned int ux;};
template <> struct inttype11<8> {typedef long x;typedef unsigned long ux;};
typedef inttype11<sizeof(void*)>::x mxvoidptr;
typedef inttype11<sizeof(void*)>::ux mxuvoidptr;

#ifndef _WIN32
#define HANDLE void*
#endif

#endif //__MXTYPES_H__
