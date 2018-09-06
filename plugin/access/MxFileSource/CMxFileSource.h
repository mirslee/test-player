#ifndef __CMXFILESOURCE_H__
#define __CMXFILESOURCE_H__

#include "MxObject.h"
#include "MxCodec.h"
#include "MxPointer.h"
#include "MxMediaDefine.h"

class CMxFileSource : public CMxObject, public MxSource {
    MX_OBJECT
public:
    CMxFileSource();
    virtual ~CMxFileSource();
    
protected:
    MxPath m_filePath;
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
    
    MxFastIORead* m_pFastIO[MX_MAXFASTIONUM];
    CMxObjectPointer<MxSource> m_extra;
    
public:
    bool open(MxPath* file);
    void close();
    bool __open(const char* filename, bool unbuffer);
    
public:
    uint32 getType() {return MxSourceType_file;}
    long getExtra(MxSource**extra);
    long getExtra(const char*privatefile, MxSource**extra);
    void getFileName(MxPath* mxPath);
    mxuvoidptr getFileId() {return m_fid;}
    storagetype getStorageType() {return m_storagetype;}
    
    int64 getPosition() {return m_i64FilePosition;}
    int64 getSize() {return m_i64FileSize;}
    int64 seek(int64 pos);
    long read(BYTE* buf, long size, int bSeek);
    long fastRead(int64 pos, BYTE* buf, long size, int stream, int bSeek, int nIoID);
    void infoEnd();
    void refresh();

	void addFastIO(MxFastIORead* pFastIO);
	void removeFastIO(int nFastIoID, void* srcp);
	bool isFastIO(int nFastIoID) { return (m_pFastIO[nFastIoID] != nullptr) && (m_fid != -1); };
    
    long queryInterfaceDelegate(long iid, void** ppv);
};

#endif /* __CMXFILESOURCE_H__ */
