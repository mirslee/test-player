#ifndef __CMXFILESOURCE_H__
#define __CMXFILESOURCE_H__

#include "MxObject.h"
#include "MxCodec.h"
#include "MxPointer.h"


class CMxFileSource : public CMxObject, public MxSource {
    MX_OBJECT
public:
    CMxFileSource();
    virtual ~CMxFileSource();
    
protected:
    MxPath _filePath;
#ifdef _WIN32
    HANDLE m_hFile;
    bool m_bUnbuffer;
#else
    int m_hFile;
#endif
    
    mxuvoidptr m_fid;
    int64 m_i64FilePosition;
    int64 m_i64FileSize;
    uint32 m_sectorsize;
    bool m_removable;
    storagetype m_storagetype;
    
    MxFastIORead* m_pFastIO[MAX_FASTIONUM];
    CMxObjectPointer<MxSource> m_extra;
    
public:
    bool open(MxPath file);
    void close();
    bool __open(const char* filename, bool unbuffer);
    
public:
    uint32 getType() {return MxSourceType_file;}
    long getExtra(MxSource**extra);
    long getExtra(const char*privatefile, MxSource**extra) = 0;
    void getFileName(MxPath* mxPath) = 0;
    mxuvoidptr getFileId() {return m_fid;}
    storagetype getStorageType() {return m_storagetype;}
    
    int64 getPosition() {return m_i64FilePosition;}
    int64 getSize() {return m_i64FileSize;}
    int64 seek(int64 pos) = 0;
    long read(BYTE* buf, long size, bool bSeek = false) = 0;
    long fastRead(int64 pos, BYTE* buf, long size, int stream, bool bSeek, int nIoID) = 0;
    void infoEnd() = 0;
    void refresh() = 0;
    
    long queryInterfaceDelegate(long iid, void** ppv);
};

#endif /* __CMXFILESOURCE_H__ */
