#include "stdafx.h"
#include "CMxFastIORead.h"
#include "MxMemory.h"
#include "MxSystem.h"
#include "MxPointer.h"
#include "MxSynchronize.h"

extern MXCORE_API bool g_bSyncRead;
extern MXCORE_API int g_rdblocksize;

struct IONODE
{
	LONG timestamp;
	int size;
};

#define MAX_IONODES 100

class CVxReadLitmit
{
public:
	CVxReadLitmit() : m_front(0), m_last(0), m_slotsize(0), m_limit(FALSE), m_maxtimes(10000)
	{
		m_limitrate = 20 * 1024 * 1024;
	}
	~CVxReadLitmit()
	{

	}
protected:
	bool m_limit;
	LONG m_limitrate;
	LONG m_maxtimes;
	LONG m_totalsize;
	IONODE m_slots[MAX_IONODES];
	int m_front, m_last, m_slotsize;
public:
	void SetLimitRate(LONG rate) { m_limitrate = rate; }
	void EnableLimit(bool limit) { m_limit = limit; }
	void SetQuality(float secs) { m_maxtimes = (LONG)(secs * 1000); }

	int GetDelay(LONG size)
	{
		if (!m_limit)
		{
			return 0;
		}

		LONG curtick = GetTickCount();
		LONG begintick = curtick - m_maxtimes;
		if (begintick < 0)
		{
			begintick = 0;
		}

		__int64 iosize = 0;
		if (m_last > m_front)
		{
			for (int i = m_front; i < m_last; i++)
			{
				if (m_slots[i].timestamp < begintick)
				{
					m_front++;
				}
				else
				{
					if (iosize == 0)
					{
						begintick = m_slots[i].timestamp;
					}
					iosize += m_slots[i].size;
				}
			}
			m_slotsize = m_last - m_front;
		}
		else if (m_slotsize > 0)
		{
			for (int i = m_front; i < MAX_IONODES; i++)
			{
				if (m_slots[i].timestamp < begintick)
				{
					m_front++;
				}
				else
				{
					if (iosize == 0)
					{
						begintick = m_slots[i].timestamp;
					}
					iosize += m_slots[i].size;
				}
			}
			bool nofindinfirst = iosize == 0;
			if (nofindinfirst)
			{
				m_front = 0;
			}
			for (int i = 0; i < m_last; i++)
			{
				if (m_slots[i].timestamp < begintick)
				{
					m_front++;
				}
				else
				{
					if (iosize == 0)
					{
						begintick = m_slots[i].timestamp;
					}
					iosize += m_slots[i].size;
				}
			}
			m_slotsize = nofindinfirst ? (m_last - m_front) : (MAX_IONODES - m_front + m_last);
		}

		int delay = 0;
		iosize += size;

		if (curtick <= begintick)
		{
			curtick = begintick + 1;
		}
		int currate = (int)((iosize * 1000) / (curtick - begintick));
		if (currate > m_limitrate)
		{
			delay = (int)((iosize * 1000 / m_limitrate) - (curtick - begintick));
		}
		if (m_slotsize == MAX_IONODES)
		{
			m_front = (m_front + 1) % MAX_IONODES;
			m_slotsize--;
		}

		m_slots[m_last].timestamp = GetTickCount() + delay;
		m_slots[m_last].size = size;
		m_last = (m_last + 1) % MAX_IONODES;
		m_slotsize++;
		//TRACE("Read slots=%d,[%d,%d],rate=%2.2fMB  DELAY[%d]\n", m_slotsize, m_front, m_last, (float)currate / (1024 * 1024), delay);
		return delay;
	}
};

CVxReadLitmit g_netlitmit;
bool g_limitall = true;

void __cdecl vxReadLimitAllStorage(bool limitall)
{
	g_limitall = limitall;
}

void __cdecl vxEnableReadLimit(bool limit, LONG limitrate, float secs)
{
	g_netlitmit.SetLimitRate(limitrate);
	g_netlitmit.EnableLimit(limit);
	g_netlitmit.SetQuality(secs);
}

static int maxevents = 32;
_FastIO::_FastIO()
{
	mxMutexInit(&m_csWait);
	mxMutexInit(&m_csFile);

	m_qReadFree.SetMaxSize(MAX_READS);
	m_qAReadFree.SetMaxSize(MAX_ASYNCREADS);
	m_qReadCompletes.SetWaitTime(40);
	m_qReadCompletes.SetMaxSize(MAX_READS);
#if __linux__
	m_qTasks.SetWaitTime(10);
#else
	m_qTasks.SetWaitTime(1000);
#endif
	m_qTasks.SetMaxSize(1000);

	memset(m_hReadThreads, 0, sizeof(pthread_t) * MAX_READS);
	INITPTHREAD(m_hCompleteThread);
	INITPTHREAD(m_hSendThread);
	m_bExitSend = FALSE;
	m_bExitComplete = FALSE;
	m_lbufs = 0;
	m_lHistory = 0;
#if __linux__
	memset(&m_aioctx, 0, sizeof(m_aioctx));
	io_setup(maxevents, &m_aioctx);
#endif
}

_FastIO::~_FastIO()
{
	exitThread();
	int size = m_bufs.GetSize();
	for (int i = 0; i < size; i++)
	{
		if (m_bufs[i].buf)
		{
			mx_free(m_bufs[i].buf);
			m_bufs[i].buf = NULL;
		}
	}
#if __linux__
	io_destroy(m_aioctx);
#endif
	mxMutexDestroy(&m_csWait);
	mxMutexDestroy(&m_csFile);
}

void _FastIO::exitThread()
{
	if (PTHREADISVALID(m_hSendThread))
	{
		EXITPTHREAD(m_hSendThread, m_bExitSend);
		INITPTHREAD(m_hSendThread);
		EXITPTHREAD(m_hCompleteThread, m_bExitComplete);
		int i = 0;
		for (; i < MAX_READS; i++)
		{
			SENDREAD task = { 0, NULL };
			m_qReads[i].Push(task);
		}
		for (i = 0; i < MAX_READS; i++)
		{
			pthread_join(m_hReadThreads[i], NULL);
		}
		memset(m_hReadThreads, 0, sizeof(HANDLE) * MAX_READS);
	}
}

void _FastIO::addFile(mxuvoidptr fid, void* srcp)
{
	CMxMutexLocker lock(&m_csFile);
	if (!PTHREADISVALID(m_hSendThread))
	{
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		int max_priority = sched_get_priority_max(SCHED_RR);;
		struct sched_param param = { 0 };
		pthread_attr_getschedparam(&attr, &param);
		param.sched_priority = max_priority;
		pthread_attr_setschedparam(&attr, &param);

		for (int i = 0; i < MAX_READS; i++)
		{
			m_qReads[i].SetMaxSize(2);
			m_qReads[i].SetWaitTime(WAIT_INFINITE);
			READSTARTPARAM* start = new READSTARTPARAM;
			start->pThis = this;
			start->idx = i;
			pthread_create(&m_hReadThreads[i], &attr, ReadFileProc, start);
			m_qReadFree.Push(i);
		}
		for (int i = 0; i < MAX_ASYNCREADS; i++)
		{
			m_qAReadFree.Push(i);
		}

		param.sched_priority = (param.sched_priority + max_priority) / 2;
		pthread_attr_setschedparam(&attr, &param);
		m_bExitComplete = FALSE;
		pthread_create(&m_hCompleteThread, &attr, TaskCompleteProc, this);
		m_bExitSend = FALSE;
		pthread_create(&m_hSendThread, &attr, SendTaskProc, this);
		pthread_attr_destroy(&attr);

		increment(16);
	}

	m_fidmap[fid]++;
	int preads = m_srcpmap[srcp];
	assert(preads == 0);
}

void _FastIO::removeFile(mxuvoidptr fid, void* srcp, bool bRemove)
{
	CMxMutexLocker lock(&m_csFile);
	int& fidref = m_fidmap[fid];
	fidref--;
	if ((fidref == 0) && bRemove)
	{
		CMxMutexLocker lock1(&m_csWait);
		int size = m_bufs.GetSize();
		for (int i = 0; i < size; i++)
		{
			if (m_bufs[i].fid == fid)
			{
				m_bufs[i].fid = 0;
				m_bufs[i].i64BlockNo = -1;
				m_bufs[i].lUsers = 0;
			}
		}
		m_fidmap.erase(fid);
	}

	LONG& preads = m_srcpmap[srcp];
	while (preads > 0)
        mxSleep(10);
	m_srcpmap.erase(srcp);
}

void _FastIO::increment(int count)
{
	CMxMutexLocker lock(&m_csWait);
	int size = m_lbufs + count - m_bufs.GetSize();
	for (int i = 0; i < size; i++)
	{
		IOBUF iobuf = { 0, -1 };
		int memalign = 16;
#if __linux__
		memalign = sysconf(_SC_PAGESIZE);
#endif
		iobuf.buf = (uint8*)mx_malloc(g_rdblocksize, memalign);
		m_bufs.Add(iobuf);
	}
	m_lbufs += count;
}

void _FastIO::decrement(int count)
{
	m_lbufs -= count;
}

int _FastIO::_PickVictim(int stream)
{
	LONG fStreamEncounteredBits = 0, fStreamNotLoneBits = 0;
	int iOurLowest = -1, iGlobalLowest = -1, iPreferred = -1;
	LONG fStreamMask = 1L << stream;

	// Look for an unused block.
	int size = m_bufs.GetSize();
	int i = 0;
	for (; i < size; i++)
	{
		if (m_bufs[i].i64BlockNo == -1)
		{
			return i;
		}
	}
	// Compile a list of streams with lone blocks.  These can't be replaced.
	// Look for our lone block.
	for (i = 0; i < size; i++)
	{
		// Encountered bits -> NotLone bits
		fStreamNotLoneBits |= fStreamEncounteredBits & m_bufs[i].fAccessedBits;
		fStreamEncounteredBits |= m_bufs[i].fAccessedBits;
	}

	// Look at the histories, and choose a few candidates.
	LONG lHistory = InterlockedExchange(&m_lHistory, m_lHistory);
	for (i = 0; i < size; i++)
	{
		IOBUF& curbuf = m_bufs[i];
		if (curbuf.i64BlockNo == -2 || curbuf.lUsers)
		{
			continue;
		}

		LONG lThisHistory = lHistory - curbuf.lHistoryVal;
		if (lThisHistory < 0)
		{
			lThisHistory = 0x7FFFFFFF;
		}

		curbuf.lAge = lThisHistory;

		// Our oldest block
		if (curbuf.fAccessedBits & fStreamMask)
			if (iOurLowest < 0 || lThisHistory > m_bufs[iOurLowest].lAge)
			{
				iOurLowest = i;
			}

		// Global oldest block
		if (iGlobalLowest < 0 || lThisHistory > m_bufs[iGlobalLowest].lAge)
		{
			iGlobalLowest = i;
		}

		// Preferred lowest block
		if (curbuf.fAccessedBits & fStreamMask && !(curbuf.fAccessedBits & ~fStreamNotLoneBits))
		{
			if (iPreferred < 0 || lThisHistory > m_bufs[iPreferred].lAge)
			{
				iPreferred = i;
			}
		}
	}
	int sel = iPreferred >= 0 ? iPreferred : iOurLowest >= 0 ? iOurLowest : iGlobalLowest;
	assert(sel >= 0);
	return sel;
}

#define FASTIO_PREREADS	2
int _FastIO::commit(mxuvoidptr fid, bool bLitmit, void* srcp, __int64 i64BlockNo, __int64 blockcount, fastioread ioread, bool asyncrd, CMxEvent hComplete, fastioreadtype mode)
{
	int idxbuf = -1;
	READWAIT task = { 0, fid, bLitmit, srcp, ioread, asyncrd, hComplete, i64BlockNo, &idxbuf };
	if (m_qTasks.Push(task))
	{
		if (mode == fastio_sequential)
		{
			LONG& preads = m_srcpmap[srcp];
			for (int i = FASTIO_PREREADS; i > 0; i--)
			{
				__int64 rno = i64BlockNo - i;
				if (rno >= 0)
				{
					READWAIT pretask = { 0, fid, bLitmit, srcp, ioread, asyncrd, NULL, rno, NULL };
					if (m_qTasks.Push(pretask))
						InterlockedIncrement(&preads);
				}
			}

			for (int i = 0; i < FASTIO_PREREADS; i++)
			{
				__int64 rno = i64BlockNo + i + 1;
				if (rno < blockcount)
				{
					READWAIT pretask = { 0, fid, bLitmit, srcp, ioread, asyncrd, NULL, i64BlockNo + i + 1, NULL };
					if (m_qTasks.Push(pretask))
						InterlockedIncrement(&preads);
				}
			}
		}
		mxWaitEvent(hComplete, INFINITE);
		return idxbuf;
	}
	else
	{
		/*char vErrStr[256] = { 0 };
		sprintf(vErrStr, vxLoadMessageLV("Read source file fastIO Commit Task Push Error, BlockNo:%lld, Size:%d"), i64BlockNo, g_rdblocksize);
		VX_MailMSG(vErrStr, vxLoadMessageLV("Error: _FastIO::Commit"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
		return -1;
	}
}

int _FastIO::searchCache(mxuvoidptr fid, void* srcp, __int64 i64BlockNo, fastioread ioread, bool asyncrd, CMxEvent hComplete)
{
	int idxbuf = -1;
	READWAIT task = { 1, fid, 0, srcp, ioread, asyncrd, hComplete, i64BlockNo, &idxbuf };
	if (m_qTasks.Push(task))
	{
		mxWaitEvent(hComplete, INFINITE);
		return idxbuf;
	}
	else
	{
		/*char vErrStr[256] = { 0 };
		sprintf(vErrStr, vxLoadMessageLV("Read source file fastIO SearchCache Task Push Error, BlockNo:%lld"), i64BlockNo);
		VX_MailMSG(vErrStr, vxLoadMessageLV("Error: _FastIO::SearchCache"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
		return -1;
	}
}

#ifdef __linux__
int _FastIO::checkreads(int waittime)
{
	struct timespec timeout = { waittime / 1000, (waittime % 1000) * 1000 * 1000 };
	struct io_event events[maxevents];
	int num_events = io_getevents(m_aioctx, waittime ? 1 : 0, maxevents, events, &timeout);
	for (int i = 0; i < num_events; i++)
	{
		struct io_event event = events[i];
		FASTRDPARAM* frp = (FASTRDPARAM*)event.data;
		frp->reads = (event.res2 == 0) ? event.res : -1;
		__aiocb(frp);
	}
	return num_events;
}
#endif

void _FastIO::_SendTask()
{
	//_vxSetThreadName("读文件分发线程");

	while (!m_bExitSend)
	{
		READWAIT task;
		if (!m_qTasks.Pop(&task, TRUE))
		{
#if __linux__
			checkreads(0);
#endif
			continue;
		}
		int stream = 0;
		for (std::map<mxuvoidptr, int>::iterator it = m_fidmap.begin(); it != m_fidmap.end(); it++)
		{
			if (it->first == task.fid) break;
			stream++;
		}

		bool bSend = true;
		{
			CMxMutexLocker lock(&m_csWait);

			// 从缓存队列中查询是否已经读起过某文件的某段数据
			int idxbuf = -1;
			int size = m_bufs.GetSize();
			for (int i = 0; i < size; i++)
			{
				if ((m_bufs[i].fid == task.fid) && (m_bufs[i].i64BlockNo == task.i64BlockNo))
				{
					idxbuf = i;
					break;
				}
			}

			// 从缓存队列中找到数据或查询模式
			if ((idxbuf >= 0) || (task.mode == 1))
			{
				if (idxbuf >= 0)
				{
					m_bufs[idxbuf].fAccessedBits |= 1L << stream;
					m_bufs[idxbuf].lHistoryVal = InterlockedIncrement(&m_lHistory);
					if (task.hComplete)
					{
						InterlockedIncrement(&m_bufs[idxbuf].lUsers);
					}
				}

				if (task.hComplete)
				{
					*task.idxbuf = idxbuf;
					mxSetEvent(task.hComplete);
				}
				else
				{
					InterlockedDecrement(&m_srcpmap[task.srcp]);
				}
#ifdef FASTIO_LOG
				vxTrace("exit b[%d],p[%d],s[%d],m[%d],e[%d]\n", idx, (DWORD)m_bufs[idx].i64BlockNo, stream, task.mode, task.hComplete != NULL);
#endif
				continue;
			}
			else
			{
				for (int i = 0; i < MAX_READS + MAX_ASYNCREADS; i++)
				{
					if (m_wait[i].GetSize() > 0)
					{
						if ((m_wait[i][0].fid == task.fid) && (m_wait[i][0].i64BlockNo == task.i64BlockNo))
						{
							m_wait[i].Add(task);
							bSend = FALSE;
#ifdef FASTIO_LOG
							//vxTrace("same b[-1],p[%d],s[%d],e[%d]\n", (DWORD)task.i64BlockNo, stream, task.hComplete != NULL);
#endif
							break;
						}
					}
				}
			}
		}
		if (bSend)
		{
			if (task.blitmit || g_limitall)
			{
				int delay = g_netlitmit.GetDelay(g_rdblocksize);
				if (delay > 0)
				{
					mxSleep(delay);
				}
			}

			if (task.async)
			{
				int idxwait = -1;
#ifdef __linux__
				if (m_qReadFree.IsEmpty())
				{
					int checks = checkreads(10000);
					if (checks > 0)
					{
						m_qAReadFree.Pop(&idxwait, TRUE);
					}
				}
				else
				{
					m_qAReadFree.Pop(&idxwait, TRUE);
				}
				if (idxwait >= 0)
#else
				if (m_qAReadFree.Pop(&idxwait, TRUE))
#endif
				{
					CMxMutexLocker lock(&m_csWait);

					int idxbuf = _PickVictim(stream);
					m_bufs[idxbuf].i64BlockNo = -2;
					m_bufs[idxbuf].fid = 0;
					m_bufs[idxbuf].lUsers = 0;
					FASTRDPARAM* frp = m_asyncp + idxwait;
					memset(frp, 0, sizeof(FASTRDPARAM));
					frp->fastio = this;
					frp->srcp = task.srcp;
					frp->pos = task.i64BlockNo * g_rdblocksize;
					frp->requestbytes = g_rdblocksize;
					frp->buffer = m_bufs[idxbuf].buf;
					frp->idxwait = idxwait + MAX_READS;
#if __linux__
					*(aio_context_t*)frp->usrdata = m_aioctx;
#endif

					SENDREAD read = { stream,task.fid,task.srcp, task.i64BlockNo, task.ioread, idxbuf, m_bufs[idxbuf].buf };
					m_task[frp->idxwait] = read;
					if (task.ioread(frp, aiocallback) < 0)
					{
						m_bufs[idxbuf].i64BlockNo = -1;
						m_bufs[idxbuf].lUsers = 0;
						if (task.hComplete)
						{
							*task.idxbuf = -1;
							mxSetEvent(task.hComplete);
						}
						else
						{
							InterlockedDecrement(&m_srcpmap[task.srcp]);
						}
					}
					else
					{
						m_wait[frp->idxwait].Add(task);
					}
#if __linux__
					checkreads(0);
#endif
				}
				else
				{
					if (task.hComplete)
					{
						/*char vErrStr[256] = { 0 };
						sprintf(vErrStr, vxLoadMessageLV("Read source file fastIO async AReadFree Pop Error, BlockNo:%lld, Size:%d"), task.i64BlockNo, g_rdblocksize);
						VX_MailMSG(vErrStr, vxLoadMessageLV("Error: _FastIO::_SendTask"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/

						*task.idxbuf = -1;
						mxSetEvent(task.hComplete);
					}
					else
					{
						InterlockedDecrement(&m_srcpmap[task.srcp]);
					}
				}
			}
			else
			{
				// 等待一个空闲的读数据线程
				int idxpool = -1;
				if (m_qReadFree.Pop(&idxpool, TRUE))
				{
					CMxMutexLocker lock(&m_csWait);

					int idxbuf = _PickVictim(stream);
					m_bufs[idxbuf].i64BlockNo = -2;
					m_bufs[idxbuf].fid = 0;
					m_bufs[idxbuf].lUsers = 0;
					SENDREAD read = { stream,task.fid,task.srcp, task.i64BlockNo, task.ioread, idxbuf, m_bufs[idxbuf].buf };
					if (m_qReads[idxpool].Push(read))
					{
						m_wait[idxpool].Add(task);
#ifdef FASTIO_LOG
						vxTrace("\nRead b[%d],p[%d],s[%d],t[%d],e[%d]\n", idxbuf, (DWORD)task.i64BlockNo, stream, idxpool, task.hComplete != NULL);
#endif
					}
					else
					{
						/*char vErrStr[256] = { 0 };
						sprintf(vErrStr, vxLoadMessageLV("Read source file fastIO Reads Push Error, BlockNo:%lld, Size:%d"), task.i64BlockNo, g_rdblocksize);
						VX_MailMSG(vErrStr, vxLoadMessageLV("Error: _FastIO::_SendTask"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/

						m_bufs[idxbuf].i64BlockNo = -1;
						m_qReadFree.Push(idxpool);
						if (task.hComplete)
						{
							*task.idxbuf = -1;
							mxSetEvent(task.hComplete);
						}
						else
						{
							InterlockedDecrement(&m_srcpmap[task.srcp]);
						}
					}
				}
				else
				{
					/*char vErrStr[256] = { 0 };
					sprintf(vErrStr, vxLoadMessageLV("Read source file fastIO ReadFree Pop Error, BlockNo:%lld, Size:%d"), task.i64BlockNo, g_rdblocksize);
					VX_MailMSG(vErrStr, vxLoadMessageLV("Error: _FastIO::_SendTask"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
					if (task.hComplete)
					{
						*task.idxbuf = -1;
						mxSetEvent(task.hComplete);
					}
					else
					{
						InterlockedDecrement(&m_srcpmap[task.srcp]);
					}
				}
			}
		}
	}
}

void _FastIO::__aiocb(FASTRDPARAM* frp)
{
	CMxMutexLocker lock(&m_csWait);
	int idx = frp->idxwait;
	int idxbuf = -1;
	LONG lHistroy = InterlockedIncrement(&m_lHistory);
	IOBUF& iobuf = m_bufs[m_task[idx].idxbuf];
	LONG count = m_wait[idx].GetSize();
	LONG lUsers = count;
	for (int i = 0; i < count; i++)
	{
		READWAIT& task = m_wait[idx][i];
		if (!task.hComplete)
			lUsers--;
	}

	if ((int)frp->reads < 0)
	{
		iobuf.fid = 0;
		iobuf.lBytes = 0;
		iobuf.i64BlockNo = -1;
		iobuf.lUsers = 0;
		iobuf.fAccessedBits = 0;
	}
	else
	{
		iobuf.fid = m_task[idx].fid;
		iobuf.lBytes = frp->reads;
		iobuf.lUsers = lUsers;
		iobuf.i64BlockNo = m_task[idx].i64BlockNo;
		iobuf.lHistoryVal = lHistroy;
		idxbuf = m_task[idx].idxbuf;
	}

	for (int i = 0; i < count; i++)
	{
		READWAIT& task = m_wait[idx][i];
		if (task.hComplete)
		{
			*task.idxbuf = idxbuf;
			mxSetEvent(task.hComplete);
		}
		else
		{
			InterlockedDecrement(&m_srcpmap[task.srcp]);
		}
	}
	m_wait[idx].RemoveAll();
	int cidx = idx - MAX_READS;
	m_qAReadFree.Push(cidx);
}

void _FastIO::_TaskComplete()
{
	//_vxSetThreadName("读文件完成更新");
	while (!m_bExitComplete)
	{
		int idx;
		if (!m_qReadCompletes.Pop(&idx, TRUE))
		{
			continue;
		}

		CMxMutexLocker lock(&m_csWait);

		int idxbuf = -1;
		LONG lHistroy = InterlockedIncrement(&m_lHistory);
		IOBUF& iobuf = m_bufs[m_task[idx].idxbuf];
		int count = m_wait[idx].GetSize();
		LONG lUsers = count;
		for (int i = 0; i < count; i++)
		{
			READWAIT& task = m_wait[idx][i];
			if (!task.hComplete)
				lUsers--;
		}

		if (m_task[idx].size < 0)
		{
			iobuf.fid = 0;
			iobuf.lBytes = 0;
			iobuf.i64BlockNo = -1;
			iobuf.lUsers = 0;
			iobuf.fAccessedBits = 0;
		}
		else
		{
			iobuf.fid = m_task[idx].fid;
			iobuf.lBytes = m_task[idx].size;
			iobuf.lUsers = lUsers;
			iobuf.i64BlockNo = m_task[idx].i64BlockNo;
			iobuf.lHistoryVal = lHistroy;
			idxbuf = m_task[idx].idxbuf;
		}

		for (int i = 0; i < count; i++)
		{
			READWAIT& task = m_wait[idx][i];
			if (task.hComplete)
			{
				*task.idxbuf = idxbuf;
				mxSetEvent(task.hComplete);
			}
			else
			{
				InterlockedDecrement(&m_srcpmap[task.srcp]);
			}
		}
		m_wait[idx].RemoveAll();
		m_qReadFree.Push(idx);
	}
}

void _FastIO::_ReadFile(int idx)
{
#ifdef _WIN32
	CoInitialize(NULL);
#endif	
	/*char szMsg[MAX_VXPATH];
	sprintf(szMsg, "读文件线程POOL第[%d]个线程", idx);
	_vxSetThreadName(szMsg);*/
	SENDREAD& task = m_task[idx];
	while (1)
	{
		if (!m_qReads[idx].Pop(&task, TRUE))
		{
			continue;
		}
		if (task.srcp == nullptr)
		{
			break;
		}
		__int64 pos = task.i64BlockNo * g_rdblocksize;
		FASTRDPARAM frd = { this, task.srcp, pos, g_rdblocksize, task.buf };
		task.size = task.ioread(&frd, NULL);
		if (!m_qReadCompletes.Push(idx))
		{
			/*char vErrStr[256] = { 0 };
			sprintf(vErrStr, vxLoadMessageLV("Read source file fastIO ReadCompletes Push Error, BlockNo:%lld, Size:%d"), task.i64BlockNo, g_rdblocksize);
			VX_MailMSG(vErrStr, vxLoadMessageLV("Error: _FastIO::_ReadFile"), 0, MAILSRC_SWENGINE | MAILSRC_ERROR);*/
		}
	}
}

CMxFastIORead::CMxFastIORead(LONG lBlockCount, int nId, _FastIO* fastio, PONRELEASE OnRel, void* p)
	: m_srcp(NULL)
	, m_fastio(fastio)
	, OnRelease(OnRel)
	, m_releasep(p)
	, m_nId(nId)
	, m_pHandle(NULL)
	, m_direct(NULL)
	, m_lBlockCount(lBlockCount)
	, m_bnet(FALSE)
	, m_IsMalloced(FALSE)
{
	m_hComplete = mxCreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSearchComplete = mxCreateEvent(NULL, FALSE, FALSE, NULL);
	m_fastio->increment(m_lBlockCount);

	mxMutexInit(&m_DirBufMutex);
}

CMxFastIORead::~CMxFastIORead()
{
	uninitFile();
	m_fastio->decrement(m_lBlockCount);
	mxCloseEvent(m_hComplete);
	mxCloseEvent(m_hSearchComplete);
	OnRelease(m_releasep, m_nId);
	if (m_direct)
	{
		mx_free(m_direct);
		m_direct = NULL;
	}
	mxMutexDestroy(&m_DirBufMutex);
}

LONG  CMxFastIORead::getBlockSize()
{
	return g_rdblocksize;
}

bool CMxFastIORead::initFile(void* srcp, MxFastIO* pHandle, mxuvoidptr fid, DWORD sectorsize, fastioread ioread, bool asyncrd)
{
	uninitFile();
	m_srcp = srcp;
	m_pHandle = pHandle;
	CMxObjectPointer<MxSource> source;
	if (pHandle->queryInterface(LIID_IVxSource, (void**)&source) == 0)
	{
		m_bnet = source->getStorageType() >= st_netshare;
	}
	m_fid = fid;
	m_sectorsize = sectorsize;
	m_ioread = ioread;
	m_asyncrd = asyncrd;
	m_fastio->addFile(fid, srcp);
	return TRUE;
}

void CMxFastIORead::uninitFile(bool bRemove)
{
	if (m_pHandle)
	{
		m_fastio->removeFile(m_fid, m_srcp, bRemove);
		m_fid = -1;
		m_pHandle->removeFastIO(m_nId, m_srcp);
		m_pHandle = NULL;
	}
	m_srcp = NULL;
}

///////////////////////////////////////////////////////////////////////////

//#pragma function(memcpy)

LONG CMxFastIORead::read(int stream, __int64 i64Pos, PBYTE pDest, LONG lBytes, fastioreadtype mode)
{
	LONG lActual = 0;
	PBYTE dst = pDest;
	__int64 filesize = ((MxSource*)m_pHandle)->getSize();
	__int64 i64BlockNo = i64Pos / g_rdblocksize;
	__int64 i64blockcount = (filesize + g_rdblocksize - 1) / g_rdblocksize;
	LONG lOffset = i64Pos % g_rdblocksize;
	while (lBytes)
	{
		LONG lToCopy = g_rdblocksize - lOffset;
		if (lToCopy > lBytes)
		{
			lToCopy = lBytes;
		}
		int iCacheBlock = m_fastio->commit(m_fid, m_bnet, m_srcp, i64BlockNo, i64blockcount, m_ioread, m_asyncrd, m_hComplete, mode);
		if (iCacheBlock == -1)
		{
			return lActual > 0 ? lActual : -1;
		}

		_FastIO::IOBUF& curbuf = (*m_fastio)[iCacheBlock];
		assert(curbuf.lUsers > 0);

		LONG lInBlock = curbuf.lBytes - lOffset;
		if (lInBlock < lToCopy)
		{
			if (lInBlock > 0)
			{
				memcpy(dst, curbuf.buf + lOffset, lInBlock);
				lActual += lInBlock;
			}
			InterlockedDecrement(&curbuf.lUsers);
			break;
		}
		else
		{
			memcpy(dst, curbuf.buf + lOffset, lToCopy);
			InterlockedDecrement(&curbuf.lUsers);
		}

		dst += lToCopy;
		lBytes -= lToCopy;
		lActual += lToCopy;
		++i64BlockNo;
		lOffset = 0;
	}
	return lActual;
}

int CMxFastIORead::lockCached(int stream, __int64 i64Pos)
{
	__int64 i64BlockNo = i64Pos / g_rdblocksize;
	return m_fastio->searchCache(m_fid, m_srcp, i64BlockNo, NULL, m_asyncrd, m_hSearchComplete);
}

const BYTE* CMxFastIORead::getCache(int idx, LONG& size)
{
	size = (*m_fastio)[idx].lBytes;
	return (*m_fastio)[idx].buf;
}

void  CMxFastIORead::unlockCached(int lockidx)
{
	_FastIO::IOBUF& curbuf = (*m_fastio)[lockidx];
	assert(curbuf.lUsers > 0);
	InterlockedDecrement(&curbuf.lUsers);
}

void * fast_memcpy(void * to, const void * from, size_t len)
{
#if HAVE_SSE2
	fast_memcpy_SSE(to, from, len);
#elif HAVE_MMX2
	fast_memcpy_MMX2(to, from, len);
#elif HAVE_AMD3DNOW
	fast_memcpy_3DNow(to, from, len);
#elif HAVE_MMX
	fast_memcpy_MMX(to, from, len);
#else
	memcpy(to, from, len); // prior to mmx we use the standart memcpy
#endif
	return to;
}

LONG  CMxFastIORead::directRead(__int64 pos, PBYTE buf, LONG lBytes)
{
	LONG toread = lBytes;

	// 在需要时,如果没有分配内存,则进行分配
	if (!m_IsMalloced)
	{
		CMxMutexLocker vLock(&m_DirBufMutex);
		if (!m_IsMalloced)
		{
			m_direct = (PBYTE)mx_malloc(g_rdblocksize);
			m_IsMalloced = TRUE;
		}
	}
	/*
	int idxbuf = 0;
	while((idxbuf=IsCached(0,pos))>=0)
	{
	_FastIO::IOBUF& curbuf = (*m_fastio)[idxbuf];

	LONG size = 0;
	const BYTE* data = GetCache(idxbuf,size);
	LONG offset = pos%g_rdblocksize;
	LONG lcpy = size-offset;
	if(lcpy>toread) lcpy = toread;
	memcpy(buf,data+offset,lcpy);
	buf += lcpy;
	toread -= lcpy;
	pos += lcpy;

	InterlockedDecrement(&curbuf.lUsers);
	}
	*/
	while (toread > 0)
	{
		__int64 readpos = pos & ~(m_sectorsize - 1);
		LONG offset = (LONG)(pos - readpos);
		LONG read = offset + toread;
		if (read > g_rdblocksize)
		{
			read = g_rdblocksize;
		}
		FASTRDPARAM frp = { this, m_srcp, readpos, (LONG)((read + (m_sectorsize - 1)) & ~(m_sectorsize - 1)), m_direct };
		LONG size = m_ioread(&frp, NULL);
		if (size > read)
		{
			size = read;
		}
		size -= offset;
		if (size <= 0)
		{
			break;
		}
		fast_memcpy(buf, m_direct + offset, size);
		pos += size;
		buf += size;
		toread -= size;
	}
	return lBytes - toread;
}
