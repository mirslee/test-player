#include "stdafx.h"
#include "MxGlobal.h"
#include "CMxFileSource.h"
#include "MxInterface.h"
#include "MxMemory.h"
#include "CMxString.h"
#include "MxSystem.h"

#ifdef _WIN32
#include "io.h"
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#define INVALID_HANDLE_VALUE -1
#endif

CMxFileSource::CMxFileSource() {
    m_fid = -1;
    m_hFile = INVALID_HANDLE_VALUE;
#ifdef _WIN32
    m_bUnbuffer = false;
#endif
    m_storagetype = st_harddisk;
    m_removable = false;
    m_i64FileSize = -1;
    memset(&m_filePath, 0, sizeof(m_filePath));
    memset(&m_pFastIO, 0, sizeof(m_pFastIO)*MX_MAXFASTIONUM);
}

CMxFileSource::~CMxFileSource() {
    
}

long CMxFileSource::queryInterfaceDelegate(long iid, void** ppv) {
    if(iid == LIID_IVxSource)
    {
        return mxGetInterface(static_cast<MxSource *>(this), ppv);
    }
    else if(iid == LIID_IVxFastIO)
    {
        return mxGetInterface(static_cast<MxFastIO *>(this), ppv);
    }
    else
    {
        return CMxObject::queryInterfaceDelgate(iid, ppv);
    }
	return -1;
}

bool CMxFileSource::open(MxPath* file) {
    memcpy(&file, file, sizeof(MxPath));
    if (!__open(file->szPath,true)) {
        
        return false;
    }
    if (file->reffile & 1) {
        m_fid = mxRefFile(file->szPath);
    }
    return true;
}


#if defined(_WIN32)
#include "winioctl.h"

bool isRemovable(const wchar_t* filepath)
{
    if(filepath[1] != ':')
        return false;
  
    wchar_t szDeviceName[64];
    swprintf(szDeviceName, L"\\\\.\\%c:", filepath[0]);
    HANDLE hDevice = CreateFileW(szDeviceName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE == hDevice  || NULL == hDevice)
		return false;

    STORAGE_PROPERTY_QUERY query = {StorageDeviceProperty, PropertyStandardQuery};
    STORAGE_DEVICE_DESCRIPTOR desc = {0, sizeof(STORAGE_DEVICE_DESCRIPTOR)};
    DWORD lOutBytes = 0;
	BOOL ret = DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), &desc, sizeof(STORAGE_DEVICE_DESCRIPTOR), &lOutBytes, NULL);
    CloseHandle(hDevice);
    return ret ? ((desc.BusType == BusType1394) || (desc.BusType == BusTypeUsb)) : false;
}

LONG __cdecl filefastioread(FASTRDPARAM* frp, asynccallback acb)
{
    HANDLE hFile = (HANDLE)frp->srcp;
    __int64 pos = frp->pos;
    void* lpBuffer = frp->buffer;
    int nNumberOfBytesToRead = frp->requestbytes;
    LONG lread = -1;
    if(g_bSyncRead)
    {
        LONG lLow = (LONG)pos;
        LONG lHigh = (LONG)(pos>>32);
        if (0xFFFFFFFF != SetFilePointer(hFile, lLow, &lHigh, FILE_BEGIN))
        {
            DWORD dwRead;
            if(ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &dwRead, NULL)) {
                lread = dwRead;
            }
            else {
				//DWORD vSysErrId = GetLastError();
				//if (vSysErrId != 38) // 38: 到达文件结尾
            }
        }
        else {
			//DWORD vSysErrId = GetLastError();
			//if (vSysErrId != 38) // 38: 到达文件结尾
        }
    }
    else
    {
        DWORD dwRead;
        OVERLAPPED overlap = {0};
        overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        overlap.Offset = (DWORD)pos;
        overlap.OffsetHigh = (DWORD)(pos>>32);
        if (ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &dwRead, &overlap))
        {
            lread = dwRead;
        }
        else
        {
            DWORD vLastError = GetLastError();
            if(vLastError == ERROR_IO_PENDING)
            {
                if(WAIT_OBJECT_0 == WaitForSingleObject(overlap.hEvent, 50000))
                {
                    if(GetOverlappedResult(hFile, &overlap, &dwRead, TRUE))
                    {
                        lread = dwRead;
                    }
                    else
                    {
						//DWORD vSysErrId = GetLastError();
						//if (vSysErrId != 38) // 38: 到达文件结尾
                    }
                }
                else
                {
                    /*char vErrStr[256] = {0};
                    sprintf(vErrStr, vxLoadMessageLV("Read source file Over Time Error, Pos:%lld, Size:%d"), pos, nNumberOfBytesToRead);
                    VX_MailMSG(vErrStr, vxLoadMessageLV("Error: VxFileSoruce::filefastioread"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
                    
                    if (CancelIo(hFile))
                    {
                        GetOverlappedResult(hFile, &overlap, &dwRead, TRUE);
                    }
                    else
                    {
						//DWORD vSysErrId = GetLastError();
						//if (vSysErrId != 38) // 38: 到达文件结尾
                    }
                }
            }
        }
        CloseHandle(overlap.hEvent);
    }
    return (frp->reads = lread);
}

HANDLE __openfastiohandle(const char* filename, bool unbuffer)
{
    HANDLE fastio = INVALID_HANDLE_VALUE;
    if (unbuffer)
    {
        fastio = mxCanUnbuffer(CMxString(filename).mxStr(), FALSE);
    }
    if ((fastio == INVALID_HANDLE_VALUE) || (fastio == NULL))
    {
        DWORD dwOpenFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS| FILE_FLAG_OVERLAPPED;
        fastio = CreateFileW((LPCWSTR)CMxString(filename).mxStr(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, dwOpenFlags, NULL);
        if ((fastio == INVALID_HANDLE_VALUE) || (fastio == NULL))
        {
            /*char vErrStr[1000] = {0};
            DWORD vSysErrId = vxGetSysLastError();
            sprintf(vErrStr, vxLoadMessageLV("open source file fastio handle Error, FileName: %s, SysError:[%d]%s"), filename, vSysErrId, vxGetSysErrorString(vSysErrId));
            VX_MailMSG(vErrStr, vxLoadMessageLV("Error: VxFileSource::__openfastiohandle"), 0, MAILSRC_HWENGINE | MAILSRC_ERROR);*/
        }
    }
    return fastio;
}

void CMxFileSource::addFastIO(MxFastIORead* pFastIO)
{
    HANDLE hFast = __openfastiohandle(m_filePath.szPath, m_bUnbuffer);
    if(pFastIO->initFile(hFast, this, m_fid, m_sectorsize, filefastioread))
    {
        m_pFastIO[pFastIO->getId()] = pFastIO;
    }
    else
    {
        CloseHandle(hFast);
    }
};

void CMxFileSource::removeFastIO(int nFastIoID, void* hFile)
{
    m_pFastIO[nFastIoID] = NULL;
    CloseHandle(hFile);
}

void CMxFileSource::close()
{
    if((m_hFile != INVALID_HANDLE_VALUE) && (m_hFile != NULL))
    {
        if(m_fid != -1)
        {
            //_vxUnrefFile(m_fid);
        }
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_i64FilePosition = 0;
    }
}

bool CMxFileSource::__open(const char* filename,bool unbuffer)
{
	wchar_t* wStr = (wchar_t*)CMxString(filename).mxStr();
    m_bUnbuffer = unbuffer;
    DWORD dwOpenFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;
    m_hFile = CreateFileW((LPCWSTR)CMxString(filename).mxStr(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, dwOpenFlags, NULL);
    if ((m_hFile == INVALID_HANDLE_VALUE) || (m_hFile == NULL))
    {
        char vErrStr[1000] = {0};
        /*DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("open source file Error, FileName: %s, SysError:[%d]%s"), filename, vSysErrId, vxGetSysErrorString(vSysErrId));
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxFileSource::__open"), 0, MAILSRC_HWENGINE | MAILSRC_ERROR);
        
        VXFree(wchUTF16);*/
        return false;
    }
    m_i64FilePosition = 0;
    BOOL vRet = GetFileSizeEx(m_hFile, (PLARGE_INTEGER)&m_i64FileSize);
    if (0 == vRet)
    {
        char vErrStr[1000] = {0};
        /*DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("source file get file size Error, FileName: %s, SysError:[%d]%s"), filename, vSysErrId, vxGetSysErrorString(vSysErrId));
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxFileSource::__open"), 0, MAILSRC_HWENGINE | MAILSRC_ERROR);
        
        VXFree(wchUTF16);*/
        return false;
    }
    
    m_sectorsize = mxGetSectorSizeForFileName(CMxString(filename).mxStr());
    if(m_sectorsize < 4096)
    {
        // 在SMB读取和新硬盘1TB以上可能出现512读取失败的情况
        m_sectorsize = 4096;
    }
    m_removable = isRemovable((LPCWSTR)CMxString(filename).mxStr());
    if(m_removable)
    {
        m_storagetype = st_removable;
    }
    /*if (((wchUTF16[0] == '\\') && (wchUTF16[1] == '\\')) ||
        ((wchUTF16[0] == '/') && (wchUTF16[1] == '/')))
    {
        m_storagetype = st_netshare;
    }*/

    return true;
}

void CMxFileSource::refresh()
{
    GetFileSizeEx(m_hFile, (PLARGE_INTEGER)&m_i64FileSize);
}

long CMxFileSource::read(uint8* buf, long size, int bSeek)
{
    __int64 realpos = 0;
    if (!SetFilePointerEx(m_hFile, *(LARGE_INTEGER*)&m_i64FilePosition, (LARGE_INTEGER*)&realpos, FILE_BEGIN) || (realpos != m_i64FilePosition))
    {
        char vErrStr[256] = {0};
        //DWORD vSysErrId = vxGetSysLastError();
        //sprintf(vErrStr, vxLoadMessageLV("read source file SetFilePointerEx Error, Pos:%lld, Size:%d, SysError:[%d]%s"), m_i64FilePosition, size, vSysErrId, vxGetSysErrorString(vSysErrId));
        //VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxFileSource::Read"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
        return -1;
    }
    DWORD dwActual;
    if(!ReadFile(m_hFile, buf, size, &dwActual,NULL))
    {
        char vErrStr[256] = {0};
        //DWORD vSysErrId = vxGetSysLastError();
        //sprintf(vErrStr, vxLoadMessageLV("read source file ReadFile Error, Pos:%lld, Size:%d, SysError:[%d]%s"), m_i64FilePosition, size, vSysErrId, vxGetSysErrorString(vSysErrId));
        //VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxFileSource::Read"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
        return -1;
    }
    m_i64FilePosition += dwActual;
    return (LONG)dwActual;
}

void CMxFileSource::infoEnd()
{
    
}

#else

#if defined(__APPLE__)

DWORD __cdecl _vxGetSectorSizeForFileName(const char* lpFileName)
{
	FSRef fileref = { 0 };
	if (FSPathMakeRef((UInt8*)lpFileName, &fileref, NULL) == noErr)
	{
		FSCatalogInfo	catalogInfo;
		if (FSGetCatalogInfo(&fileref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL) == noErr)
		{
			FSVolumeInfo vinfo;
			if (FSGetVolumeInfo(catalogInfo.volume, 0, NULL, kFSVolInfoBlocks, &vinfo, NULL, NULL) == noErr)
				return vinfo.blockSize;
		}
	}

	return 2048;
}

long filefastioread(FASTRDPARAM* frp, asynccallback acb)
{
    int hFile = (int)(mxuvoidptr)frp->srcp;
    long lread = pread((int)hFile, frp->buffer, frp->requestbytes, frp->pos);
    return (frp->reads = lread);
}

#else

long filefastioread(FASTRDPARAM* frp, asynccallback acb)
{
    HANDLE hFile = (HANDLE)(mxuvoidptr)frp->srcp;
    struct iocb* myio = (struct iocb *)&frp->usrdata[8];
    aio_context_t aioctx = *(aio_context_t *)frp->usrdata;
    io_prep_pread(myio, hFile, frp->buffer, frp->requestbytes, frp->pos);
    io_set_userdata(myio, frp);
    int res = io_submit(aioctx, 1, &myio);
    return res == 1 ? 0 : -1;
}

#endif

void CMxFileSource::addFastIO(IVxFastIORead* pFastIO)
{
    int fast = open(m_filepath.szPath, O_RDONLY, 0666);
    if (fast == -1)
    {
        char vErrStr[1000] = {0};
        DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("open source file Error, FileName:%s, SysError:[%d]%s"), m_filepath.szPath, vSysErrId, vxGetSysErrorString(vSysErrId));
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CMxFileSource::AddFastIO"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
    }
#ifdef __APPLE__
    fcntl(fast, F_RDAHEAD, 1);
    fcntl(fast, F_NOCACHE, 1);
    bool bAysnc = false;
#else
	bool bAysnc = true;
#endif
    
    if(pFastIO->initFile((void *)(vxuintptr)fast, this, m_fid, m_sectorsize, filefastioread, bAysnc))
    {
        m_pFastIO[pFastIO->GetId()] = pFastIO;
    }
    else
    {
        close(fast);
    }
}

void CMxFileSource::removeFastIO(int nFastIoID,void* srcp)
{
    HVXFILE hFile = (HVXFILE)(vxuintptr)srcp;
    m_pFastIO[nFastIoID] = NULL;
    close((int)hFile);
}

void CMxFileSource::Close()
{
    if((m_hFile != 0) && (m_hFile != -1))
    {
        if(m_fid != -1)
        {
            _vxUnrefFile(m_fid);
        }
        close((int)m_hFile);
        m_hFile = NULL;
        m_i64FilePosition = 0;
    }
}

bool is_removable(const char* devpath)
{
    char link[512] = {0};
    char link_path[512] = {0};
    struct stat buf;
    stat(devpath, &buf);
    if(S_ISBLK(buf.st_mode))
    {
        sprintf(link_path, "/sys/dev/block/%d:%d", major(buf.st_rdev), minor(buf.st_rdev));
        if(access(link_path, F_OK) >= 0)
        {
            int i = readlink(link_path, link, 512);
            if(i == 0)
            {
                return false;
            }
            
            vxString strlink = link;
            int pos = strlink.find("usb", 0);
            if(pos == strlink.length())
            {
                return false;
            }
            
            //removable
            sprintf(link_path, "/sys/dev/block/%d:%d/removable", major(buf.st_rdev), minor(buf.st_rdev));
            memset(link, 0, sizeof(link));
            int fd = open(link_path, O_RDONLY);
            if (fd == -1)
            {
                return false;
            }
            i = read(fd, link, 512);
            close(fd);
            if (i == -1)
            {
                i = errno;
                return false;
            }
            if (link[0] == '0')
            {
                return false;
            }
            return true;
        }
    }
    return false;
}

bool CMxFileSource::__open(const char* filename, bool unbuffer)
{
    m_hFile = (HVXFILE)open(filename, O_RDONLY);
    if (m_hFile == -1)
    {
        char vErrStr[1000] = {0};
        DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("open source file Error, FileName:%s, SysError:[%d]%s"), filename, vSysErrId, vxGetSysErrorString(vSysErrId));
#ifndef _WIN32
        utf82gbk(vErrStr,vErrStr);
#endif
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CMxFileSource::__open"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
        return FALSE;
    }
    struct stat curstat;
    if(fstat(m_hFile, &curstat) < 0)
    {
        char vErrStr[1000] = {0};
        DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("open source file fstat Error, FileName:%s, SysError:[%d]%s"), filename, vSysErrId, vxGetSysErrorString(vSysErrId));
#ifndef _WIN32
        utf82gbk(vErrStr,vErrStr);
#endif
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CMxFileSource::__open"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);
        return FALSE;
    }
    m_i64FilePosition = 0;
    m_i64FileSize = curstat.st_size;
    
    m_sectorsize = curstat.st_blksize;
    if(m_sectorsize < 4096)
    {
        // 在SMB读取和新硬盘1TB以上可能出现512读取失败的情况
        m_sectorsize = 4096;
    }
    m_sectorsize = (m_sectorsize + 4095) / 4096 * 4096;
    
    m_removable = is_removable(filename);
    if(m_removable)
    {
        m_storagetype = st_removable;
    }
    return TRUE;
}

LONG CMxFileSource::Read(PBYTE buf, LONG size, int bSeek)
{
    if (-1 == lseek((int)m_hFile, m_i64FilePosition, SEEK_SET))
    {
        /*char vErrStr[256] = {0};
        DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("read source file lseek Error, Pos:%lld, Size:%d, SysError:[%d]%s"), m_i64FilePosition, size, vSysErrId, vxGetSysErrorString(vSysErrId));
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CMxFileSource::Read"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
        return -1;
    }
    int reads = read((int)m_hFile, buf, size);
    if(reads < 0)
    {
        /*char vErrStr[256] = {0};
        DWORD vSysErrId = vxGetSysLastError();
        sprintf(vErrStr, vxLoadMessageLV("read source file read Error, Pos:%lld, Size:%d, SysError:[%d]%s"), m_i64FilePosition, size, vSysErrId, vxGetSysErrorString(vSysErrId));
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CMxFileSource::Read"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
        return -1;
    }
    m_i64FilePosition += reads;
    return reads;
}

void CMxFileSource::infoEnd()
{
    
}

void CMxFileSource::refresh()
{
    m_i64FileSize = lseek((int)m_hFile, 0, SEEK_END);
}

#endif

long CMxFileSource::getExtra(MxSource** extra)
{
    if(m_extra)
    {
        return mxGetInterface(m_extra, (void**)extra);
    }
    if(strlen(m_filePath.szExtraPath) <= 0)
    {
        return -1;
    }
    CMxFileSource* ext = new CMxFileSource;
    if(!ext->__open(m_filePath.szExtraPath, FALSE))
    {
        ext->close();
        delete ext;
        return -1;
    }
    return mxGetInterface(static_cast<MxSource *>(ext), (void**)extra);
}

long CMxFileSource::getExtra(const char* privatefile, MxSource** extra)
{
	CMxFileSource* ext = new CMxFileSource;
    if(!ext->__open(privatefile, FALSE))
    {
        ext->close();
        delete ext;
        return -1;
    }
    return mxGetInterface(static_cast<MxSource *>(ext), (void**)extra);
}

void CMxFileSource::getFileName(MxPath* file)
{
    memcpy(file, &m_filePath, sizeof(MxPath));
}

int64    CMxFileSource::seek(__int64 pos)
{
    if(pos > m_i64FileSize)
    {
        /*char vErrStr[256] = {0};
        sprintf(vErrStr, vxLoadMessageLV("seek source file Error, Pos:%lld, Size:%lld"), pos, m_i64FileSize);
        VX_MailMSG(vErrStr, vxLoadMessageLV("Error: CVxFileSource::Seek"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
        
        return -1;
    }
    return (m_i64FilePosition = pos);
}

long CMxFileSource::fastRead(int64 pos, uint8* buf, long size, int stream, int bSeek, int nIoID)
{
    if(m_pFastIO[nIoID])
    {
        if((bSeek == 3) || (m_removable && (bSeek != 0)))
        {
            return m_pFastIO[nIoID]->directRead(pos, buf, size);
        }
        else
        {
            return m_pFastIO[nIoID]->read(stream, pos, buf, size, bSeek == 0 ? fastio_sequential : fastio_random);
        }
    }
    else
    {
        assert(false);
        m_i64FilePosition = pos;
        return read(buf, size, bSeek);
    }
}

/*static long _fileCreateSource(IVxObject* setup, IVxObject* param, vxuintptr param2, vxuintptr param3, IVxObject** obj)
{
    CMxFileSource* pObj = new CMxFileSource;
    if(!pObj->Open((LPVX_AVPATH)param2))
    {
        pObj->Close();
        delete pObj;
        return -1;
    }
    return mxGetInterface(static_cast<IVxSource *>(pObj), (void**)obj);
}*/

/*extern "C" __declspec( dllexport ) LONG vxGetObjects(VXOBJECT* vxObj, LONG flags)
{
    vxObj->dwObjID        = 0;
    vxObj->dwObjType    = vxObjSource;
    vxObj->type_count    = 1;
    vxObj->types        = (DWORD *)_vxmalloc(sizeof(DWORD) * vxObj->type_count);
    vxObj->types[0]        = vxSourceSub_file;
    strncpy(vxObj->szName, "file source", 60);
    vxObj->pfCreate        = _fileCreateSource;
    return 0;
}*/
