#ifndef __CMXFASTIOREADE_H__
#define __CMXFASTIOREADE_H__

#include "MxObject.h"
#include "MxCodec.h"
#include "MxTypes.h"
#include "MxSynchronize.h"
#include "CMxArray.h"
#include "CMxQueue.h"
#include <map>

class _FastIO
{
public:
	_FastIO();
	~_FastIO();
	struct IOBUF
	{
		mxuvoidptr fid;
		__int64 i64BlockNo;
		LONG fAccessedBits;
		LONG lBytes;
		LONG lAge;
		LONG lHistoryVal;
		LONG lUsers;
		PBYTE buf;
	};
	void increment(int count);
	void decrement(int count);
	void addFile(mxuvoidptr fid, void* srcp);
	void removeFile(mxuvoidptr fid, void* srcp, bool bRemove = true);

	int commit(mxuvoidptr fid, bool bLitmit, void* srcp, __int64 i64BlockNo, __int64 maxblockcount, fastioread ioread, bool asyncrd, CMxEvent hComplete, fastioreadtype mode);
	int searchCache(mxuvoidptr fid, void* srcp, __int64 i64BlockNo, fastioread ioread, bool asyncrd, CMxEvent hComplete);
	IOBUF& operator[](int idx) { return m_bufs[idx]; }

	void exitThread();
private:
#if __linux__
	aio_context_t m_aioctx;
	int checkreads(int waittime);
#endif
	CMxArray<IOBUF, IOBUF> m_bufs;
	LONG m_lbufs;
	LONG m_lHistory;
	std::map<mxuvoidptr, int> m_fidmap;
	std::map<void*, LONG> m_srcpmap;

#define MAX_READS	8
#define MAX_ASYNCREADS 16
	struct SENDREAD
	{
		int stream;
		mxuvoidptr fid;
		void* srcp;
		__int64 i64BlockNo;
		fastioread ioread;
		int idxbuf;
		PBYTE buf;
		LONG size;
	};

	CMxQueue<int> m_qReadFree;
	struct READWAIT
	{
		int mode;
		mxuvoidptr fid;
		bool blitmit;
		void* srcp;
		fastioread ioread;
		bool async;
		CMxEvent hComplete;
		__int64 i64BlockNo;
		int* idxbuf;
	};
	CMxQueue<READWAIT> m_qTasks;

	CMxArray<READWAIT, READWAIT> m_wait[MAX_READS + MAX_ASYNCREADS];
	CMxQueue<int> m_qAReadFree;

	CMxMutex m_csWait;
	CMxMutex m_csFile;
	bool m_bExitSend;
	pthread_t m_hSendThread;
	static void* SendTaskProc(LPVOID lp) { ((_FastIO*)lp)->_SendTask(); return 0; }
	void _SendTask();
	SENDREAD m_task[MAX_READS + MAX_ASYNCREADS];
	CMxQueue<int> m_qReadCompletes;
	bool m_bExitComplete;
	pthread_t m_hCompleteThread;
	static void* TaskCompleteProc(LPVOID lp) { ((_FastIO*)lp)->_TaskComplete(); return 0; }
	void _TaskComplete();
	CMxQueue<SENDREAD> m_qReads[MAX_READS];
	pthread_t m_hReadThreads[MAX_READS];
	struct READSTARTPARAM
	{
		void* pThis;
		int idx;
	};
	static void* ReadFileProc(LPVOID lp)
	{
		READSTARTPARAM* p = (READSTARTPARAM*)lp;
		_FastIO* pThis = (_FastIO*)p->pThis;
		int idx = p->idx;
		delete p;
		pThis->_ReadFile(idx);
		return 0;
	}
	void _ReadFile(int idx);

	FASTRDPARAM m_asyncp[MAX_ASYNCREADS];
	static void __cdecl aiocallback(FASTRDPARAM* frp) { ((_FastIO*)frp->fastio)->__aiocb(frp); }
	void __aiocb(FASTRDPARAM* frp);

	int _PickVictim(int stream);
};

typedef void(*PONRELEASE)(void* p, int nID);
class CMxFastIORead : public MxFastIORead, public CMxObject
{
	MX_OBJECT
public:
	CMxFastIORead(LONG lBlockCount, int nId, _FastIO* fastio, PONRELEASE OnRel, void* p);
	virtual ~CMxFastIORead();
public:
	int   __stdcall getId() { return m_nId; }
	bool  __stdcall initFile(void* srcp, MxFastIO* vxdemul, mxuvoidptr fid, DWORD sectorsize, fastioread ioread, bool asyncrd);
	void  __stdcall uninitFile(bool bRemove = TRUE);
	mxuvoidptr __stdcall getFileId() { return m_fid; }
	LONG  __stdcall read(int stream, __int64 pos, PBYTE buf, LONG lBytes, fastioreadtype mode);
	LONG  __stdcall directRead(__int64 pos, PBYTE buf, LONG lBytes);
	int  __stdcall lockCached(int stream, __int64 pos);
	const BYTE* __stdcall getCache(int idx, LONG& size);
	void  __stdcall unlockCached(int lockidx);

	LONG  __stdcall getBlockSize();
private:
	_FastIO* m_fastio;
	mxuvoidptr m_fid;
	void* m_srcp;
	fastioread m_ioread;
	bool m_asyncrd;
	CMxEvent m_hComplete;
	CMxEvent m_hSearchComplete;
	LONG m_lBlockCount;
	MxFastIO* m_pHandle;
	int m_nId;
	PONRELEASE OnRelease;
	void* m_releasep;
	PBYTE m_direct;
	__int64 m_sectorsize;
	bool m_bnet;

	bool	m_IsMalloced;	// 标记内存是否已经分配
	CMxMutex m_DirBufMutex;
};

#endif //__CMXFASTIOREADE_H__