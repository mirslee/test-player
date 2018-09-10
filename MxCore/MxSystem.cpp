#include "stdafx.h"
#include "MxSystem.h"
#include "MxGlobal.h"
#include "MxMediaDefine.h"
#include "MxSynchronize.h"

extern MXCORE_API bool g_bSyncRead = false;
extern MXCORE_API bool g_bSyncWrite = false;
extern MXCORE_API bool g_bRealtimeIDX = false;
extern MXCORE_API int g_rdblocksize = 0x100000;
extern MXCORE_API int g_wrblocksize = 0x100000;
extern MXCORE_API bool g_bUnbuffer = true;

static int g_disks = 0;
BYTE* g_pDiskTest = NULL;
static struct DISKTEST
{
	DWORD dwFlags[2];
	bool unbuffer;
}g_diskinfo[64] = { 0 };

#ifdef _WIN32
mxinline char *splitPathRoot(char *dst, const char *path)
{
	if (!path)	return NULL;

	// C:

	if (isalpha(path[0]) && path[1] == ':') {
		dst[0] = path[0];
		dst[1] = ':';
		dst[2] = '\\';
		dst[3] = 0;

		return dst;
	}

	// UNC path?

	if (path[0] == '\\' && path[1] == '\\') {
		const char *s = path + 2;
		char *t = dst;

		*t++ = '\\';
		*t++ = '\\';

		while (*s && *s != '\\')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while (*s && *s != '\\')
			*t++ = *s++;

		*t++ = '\\';
		*t = 0;

		return dst;
	}

	if (path[0] == '/' && path[1] == '/') {
		const char *s = path + 2;
		char *t = dst;

		*t++ = '/';
		*t++ = '/';

		while (*s && *s != '/')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while (*s && *s != '/')
			*t++ = *s++;

		*t++ = '/';
		*t = 0;

		return dst;
	}

	return NULL;
}

mxinline unsigned short *splitPathRoot(unsigned short *dst, const unsigned short *path)
{
	if (!path)	return NULL;

	// C:

	if (isalpha(path[0]) && path[1] == ':') {
		dst[0] = path[0];
		dst[1] = ':';
		dst[2] = '\\';
		dst[3] = 0;

		return dst;
	}

	// UNC path?

	if (path[0] == '\\' && path[1] == '\\') {
		const unsigned short *s = path + 2;
		unsigned short *t = dst;

		*t++ = '\\';
		*t++ = '\\';

		while (*s && *s != '\\')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while (*s && *s != '\\')
			*t++ = *s++;

		*t++ = '\\';
		*t = 0;

		return dst;
	}

	if (path[0] == '/' && path[1] == '/') {
		const unsigned short *s = path + 2;
		unsigned short *t = dst;

		*t++ = '/';
		*t++ = '/';

		while (*s && *s != '/')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while (*s && *s != '/')
			*t++ = *s++;

		*t++ = '/';
		*t = 0;

		return dst;
	}

	return NULL;
}

MXCORE_API HANDLE mxCanUnbuffer(const char* lpstr, bool bWrite) {
	if (!g_bUnbuffer) return INVALID_HANDLE_VALUE;

	bool bFind = FALSE;
	char szRoot[MX_MAXPATH] = { 0 };
	if (!splitPathRoot(szRoot, lpstr)) return INVALID_HANDLE_VALUE;
	DWORD* dwFlag = (DWORD*)szRoot;
	int i = 0;
	for (; i < g_disks; i++)
	{
		if ((dwFlag[0] == g_diskinfo[i].dwFlags[0]) && (dwFlag[1] == g_diskinfo[i].dwFlags[1]))
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind)
	{
		if (!g_diskinfo[i].unbuffer) return INVALID_HANDLE_VALUE;
		DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
		if (bWrite)
		{
			if (!g_bSyncWrite) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
		else
		{
			if (!g_bSyncRead) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
		return CreateFileA(lpstr, bWrite ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ, bWrite ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, bWrite ? CREATE_ALWAYS : OPEN_EXISTING, dwOpenFlag, NULL);
	}
	DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
	if (bWrite)
	{
		if (!g_bSyncWrite) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
	}
	else
	{
		if (!g_bSyncRead) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
	}
	HANDLE hUnbuffer = CreateFileA(lpstr, bWrite ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ, bWrite ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, bWrite ? CREATE_ALWAYS : OPEN_EXISTING, dwOpenFlag, NULL);
	if (hUnbuffer == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;
	DWORD dwActual = 0;
	g_diskinfo[g_disks].dwFlags[0] = dwFlag[0];
	g_diskinfo[g_disks].dwFlags[1] = dwFlag[1];

	if (bWrite)
	{
		if (g_bSyncWrite)
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, NULL);
		else
		{
			OVERLAPPED overlap = { 0 };
			overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, &overlap);
			if (!g_diskinfo[g_disks].unbuffer)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(overlap.hEvent, 5000) == WAIT_OBJECT_0)
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer, &overlap, &dwActual, TRUE);
					else
						CancelIo(hUnbuffer);
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if (g_diskinfo[g_disks++].unbuffer)
		{
			LONG high = 0;
			SetFilePointer(hUnbuffer, 0, &high, FILE_BEGIN);
			SetEndOfFile(hUnbuffer);
			return hUnbuffer;
		}
		else
			CloseHandle(hUnbuffer);
	}
	else
	{
		if (g_bSyncRead)
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, NULL);
		else
		{
			OVERLAPPED overlap = { 0 };
			overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, &overlap);
			if (!g_diskinfo[g_disks].unbuffer)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(overlap.hEvent, 5000) == WAIT_OBJECT_0)
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer, &overlap, &dwActual, TRUE);
					else
						CancelIo(hUnbuffer);
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if (GetLastError() != ERROR_HANDLE_EOF)
		{
			if (g_diskinfo[g_disks++].unbuffer) return hUnbuffer;
			CloseHandle(hUnbuffer);
		}
		else
			CloseHandle(hUnbuffer);
	}


	return INVALID_HANDLE_VALUE;
}

MXCORE_API HANDLE mxCanUnbuffer(const unsigned short* lpstr, bool bWrite) {
	if (!g_bUnbuffer)
	{
		return INVALID_HANDLE_VALUE;
	}

	bool bFind = FALSE;
	unsigned short szRoot[MX_MAXPATH] = { 0 };
	if (!splitPathRoot(szRoot, lpstr))
	{
		return INVALID_HANDLE_VALUE;
	}
	DWORD* dwFlag = (DWORD *)szRoot;
	int i = 0;
	for (; i < g_disks; i++)
	{
		if ((dwFlag[0] == g_diskinfo[i].dwFlags[0]) && (dwFlag[1] == g_diskinfo[i].dwFlags[1]))
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind)
	{
		if (!g_diskinfo[i].unbuffer)
		{
			return INVALID_HANDLE_VALUE;
		}
		DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
		if (bWrite)
		{
			if (!g_bSyncWrite)
			{
				dwOpenFlag |= FILE_FLAG_OVERLAPPED;
			}
		}
		else
		{
			if (!g_bSyncRead)
			{
				dwOpenFlag |= FILE_FLAG_OVERLAPPED;
			}
		}
		return CreateFileW((LPCWSTR)lpstr, bWrite ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ, bWrite ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, bWrite ? CREATE_ALWAYS : OPEN_EXISTING, dwOpenFlag, NULL);
	}
	DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
	if (bWrite)
	{
		if (!g_bSyncWrite)
		{
			dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
	}
	else
	{
		if (!g_bSyncRead)
		{
			dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
	}
	HANDLE hUnbuffer = CreateFileW((LPCWSTR)lpstr, bWrite ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ, bWrite ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, bWrite ? CREATE_ALWAYS : OPEN_EXISTING, dwOpenFlag, NULL);
	if ((hUnbuffer == INVALID_HANDLE_VALUE) || (hUnbuffer == NULL))
	{
		return INVALID_HANDLE_VALUE;
	}
	DWORD dwActual = 0;
	g_diskinfo[g_disks].dwFlags[0] = dwFlag[0];
	g_diskinfo[g_disks].dwFlags[1] = dwFlag[1];

	if (bWrite)
	{
		if (g_bSyncWrite)
		{
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, NULL);
		}
		else
		{
			OVERLAPPED overlap = { 0 };
			overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, &overlap);
			if (!g_diskinfo[g_disks].unbuffer)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(overlap.hEvent, 5000) == WAIT_OBJECT_0)
					{
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer, &overlap, &dwActual, TRUE);
					}
					else
					{
						CancelIo(hUnbuffer);
					}
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if (g_diskinfo[g_disks++].unbuffer)
		{
			LONG high = 0;
			SetFilePointer(hUnbuffer, 0, &high, FILE_BEGIN);
			SetEndOfFile(hUnbuffer);
			return hUnbuffer;
		}
		else
		{
			CloseHandle(hUnbuffer);
		}
	}
	else
	{
		if (g_bSyncRead)
		{
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, NULL);
		}
		else
		{
			OVERLAPPED overlap = { 0 };
			overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, &overlap);
			if (!g_diskinfo[g_disks].unbuffer)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(overlap.hEvent, 5000) == WAIT_OBJECT_0)
					{
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer, &overlap, &dwActual, TRUE);
					}
					else
					{
						CancelIo(hUnbuffer);
					}
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if (GetLastError() != ERROR_HANDLE_EOF)
		{
			if (g_diskinfo[g_disks++].unbuffer)
			{
				return hUnbuffer;
			}
			CloseHandle(hUnbuffer);
		}
		else
		{
			CloseHandle(hUnbuffer);
		}
	}

	return INVALID_HANDLE_VALUE;
}

MXCORE_API DWORD mxGetSectorSizeForFileName(const char* lpFileName) {
	char szRoot[MAX_PATH];
	DWORD dwSectorSize;
	if (splitPathRoot(szRoot, lpFileName) == NULL)
	{
		return 512; // (DWORD) - 1;
	}
	DWORD dwSectorsPerCluster, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	if (0 == GetDiskFreeSpaceA(szRoot, &dwSectorsPerCluster, &dwSectorSize, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
	{
		return 512; // (DWORD) - 1;
	}
	return dwSectorSize;
}
MXCORE_API DWORD mxGetSectorSizeForFileName(const unsigned short* lpFileName) {
	unsigned short szRoot[MAX_PATH];
	DWORD dwSectorSize;
	if (splitPathRoot(szRoot, lpFileName) == NULL)
	{
		return 512; // (DWORD) - 1;
	}
	DWORD dwSectorsPerCluster, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	if (0 == GetDiskFreeSpaceW((LPCWSTR)szRoot, &dwSectorsPerCluster, &dwSectorSize, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
	{
		return 512; // (DWORD) - 1;
	}
	return dwSectorSize;
}
#endif
