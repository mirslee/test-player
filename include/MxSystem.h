#ifndef __MXSYSTEM_H__
#define __MXSYSTEM_H__

extern MXCORE_API bool g_bSyncRead;
extern MXCORE_API bool g_bSyncWrite;
extern MXCORE_API bool g_bRealtimeIDX;
extern MXCORE_API int g_rdblocksize;
extern MXCORE_API int g_wrblocksize;
extern MXCORE_API bool g_bUnbuffer;

#ifdef _WIN32
MXCORE_API HANDLE mxCanUnbuffer(const char* lpstr, bool bWrite);
MXCORE_API HANDLE mxCanUnbuffer(const unsigned short* lpstr, bool bWrite);
MXCORE_API DWORD mxGetSectorSizeForFileName(const char* lpFileName);
MXCORE_API DWORD mxGetSectorSizeForFileName(const unsigned short* lpFileName);
#endif

MXCORE_API mxuvoidptr mxRefFile(const char* file);
MXCORE_API void mxUnrefFile(mxuvoidptr fid);


#endif //__MXSYSTEM_H__
