
#ifndef __MXGLOBAL_H__
#define __MXGLOBAL_H__

#include "MxDllexport.h"

#ifdef _WIN32
#define mxinline __forceinline
#else
#define mxinline inline
#endif

extern MXCORE_API bool g_bSyncRead;
extern MXCORE_API bool g_bSyncWrite;
extern MXCORE_API bool g_bRealtimeIDX;
extern MXCORE_API int g_rdblocksize;
extern MXCORE_API int g_wrblocksize;
extern MXCORE_API bool g_bUnbuffer;


#endif //__MXGLOBAL_H__
