// VxMediaSysClock.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "CMxMediaSysClock.h"
#include "../MxCore/MxPointer.h"
#include "MxLog.h"

#ifdef _WIN32
#include "mmsystem.h"
#endif

CVxMediaSysClock::CVxMediaSysClock(const sysclk_cinfo* info)
: m_cinfo(*info)
, m_hCoreClock(NULL)
, m_bExitClock(FALSE)
, m_canreset(FALSE)
{
	MX_MUTEX_INIT(&m_csLock);
	MX_MUTEX_INIT(&m_csFLock);
	MX_MUTEX_INIT(&m_csPulseLock);
	
	INITPTHREAD(m_hClockThread);
}

CVxMediaSysClock::~CVxMediaSysClock(void)
{
	MX_MUTEX_DESTROY(&m_csPulseLock);
	MX_MUTEX_DESTROY(&m_csFLock);
	MX_MUTEX_DESTROY(&m_csLock);
}

IVxReferenceClock* CreateReferenceClock(CLOCKFUNCS* funcs);

bool CVxMediaSysClock::Initialize(IVxClockPulse* clockpulse)
{
	m_clockpulse = clockpulse;
	if (!m_clockpulse)
	{
		mxClockPulse(nullptr, &m_cinfo, nullptr, &m_clockpulse);
		if (!m_clockpulse) return false;
		m_canreset = true;
	}


#ifdef _WIN32
	TIMECAPS tc;
	m_TimerResolution = (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(tc))) ? tc.wPeriodMin : 1;
	timeBeginPeriod(m_TimerResolution);
#endif
	m_hCoreClock = m_clockpulse->GetFieldEvent();
	__int64 rtNow = m_clockpulse->GetTime();
	m_ulClock = m_clockpulse->GetSampleFromTime(rtNow);

	m_bExitClock = false;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr,SCHED_RR);
	int max_priority = sched_get_priority_max(SCHED_RR);;
	struct sched_param param = {0};
	pthread_attr_getschedparam(&attr,&param);
	param.sched_priority = max_priority;
	pthread_attr_setschedparam(&attr,&param);	
	pthread_create(&m_hClockThread,&attr,ClockThreadProc,this);
	pthread_attr_destroy(&attr);
#ifdef _WIN32
	SetThreadPriority(pthread_getw32threadhandle_np(m_hClockThread),THREAD_PRIORITY_TIME_CRITICAL);
#endif

	m_hWaitSyncEvent = CreateClockEvent(false,false);
	return true;

}

void CVxMediaSysClock::Uninitialize()
{
	CloseClockEvent(m_hWaitSyncEvent);
	EXITPTHREAD(m_hClockThread,m_bExitClock);
	
#ifdef _WIN32
	if (m_TimerResolution)
	{
		timeEndPeriod(m_TimerResolution);
	}
#endif

	m_clockpulse = NULL;
}


bool CVxMediaSysClock::Reset(const sysclk_cinfo* cinfo,IVxClockPulse* clockpulse)
{
	if(m_clockpulse==clockpulse) 
	{
		CMxMutexLocker lock(&m_csPulseLock);
		m_hCoreClock = m_clockpulse->GetFieldEvent();
		m_cinfo = *cinfo;
		__int64 rtNow = m_clockpulse->GetTime();
		m_ulClock = m_clockpulse->GetSampleFromTime(rtNow);
		return true;
	}

	if (clockpulse&&!m_canreset)
		return false;
	m_canreset = false;

	CMxSharedPointer<IVxClockPulse> tmppulse(clockpulse);
	if(!tmppulse)
	{	
		mxClockPulse(NULL,cinfo,NULL,&tmppulse);
		m_canreset = TRUE;
	}

	EXITPTHREAD(m_hClockThread,m_bExitClock);

	{
		CMxMutexLocker lock(&m_csPulseLock);
		m_clockpulse = tmppulse;
		m_hCoreClock = m_clockpulse->GetFieldEvent();
		m_cinfo = *cinfo;
		__int64 rtNow = m_clockpulse->GetTime();
		m_ulClock = m_clockpulse->GetSampleFromTime(rtNow);
	}
	
	m_bExitClock = false;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr,SCHED_RR);
	int max_priority = sched_get_priority_max(SCHED_RR);;
	struct sched_param param = {0};
	pthread_attr_getschedparam(&attr,&param);
	param.sched_priority = max_priority;
	pthread_attr_setschedparam(&attr,&param);	
	pthread_create(&m_hClockThread,&attr,ClockThreadProc,this);
	pthread_attr_destroy(&attr);
#ifdef _WIN32
	SetThreadPriority(pthread_getw32threadhandle_np(m_hClockThread),THREAD_PRIORITY_TIME_CRITICAL);
#endif
	
	return TRUE;
}

LONG CVxMediaSysClock::NonDelegatingQueryInterface(LONG iid, void** ppObj)
{
	*ppObj = static_cast<IVxSystemClock*>(this);
	return 0;
	/*if(iid==LIID_IVxSystemClock)
		return GetVxInterface(static_cast<IVxSystemClock*>(this),ppObj);
	else
		return CVxObject::NonDelegatingQueryInterface(iid,ppObj);*/
}


void CVxMediaSysClock::WaitSyncForSystemClock(uint64 dwClock)
{
	CMxMutexLocker lock(&m_csFLock);
	while(m_ulClock<dwClock)
		mxWaitObject(m_hWaitSyncEvent,100);
}

uint64 CVxMediaSysClock::WaitSyncFrameClock()
{
	CMxMutexLocker lock(&m_csFLock);
	uint64 ulTimeCode = 0;
	do
	{
		mxWaitObject(m_hWaitSyncEvent,100);
	}while(((ulTimeCode = m_ulClock)&0x1)/*&&(m_cinfo.scantype!=SCANTYPE_PROGRESSIVE)*/);
	return ulTimeCode;
}

MxEvent CVxMediaSysClock:: CreateClockEvent(bool bManualReset,bool bInitialState)
{
	MxEvent hClockEvent = mxCreateEvent(NULL,bManualReset,bInitialState,NULL);
	CMxMutexLocker lock(&m_csLock);
	m_hClocks.Add(hClockEvent);
	return hClockEvent;
}

void CVxMediaSysClock:: CloseClockEvent(MxEvent hClock)
{
	int nSize = m_hClocks.GetSize();
	for(int i=0;i<nSize;i++)
	{
		if(m_hClocks[i]==hClock)
		{
			CMxMutexLocker lock(&m_csLock);
			m_hClocks.RemoveAt(i);
			mxCloseEvent(hClock);
			break;
		}
	}
}

uint64 CVxMediaSysClock::GetTime()
{
	CMxMutexLocker lock(&m_csPulseLock);
	return m_clockpulse->GetTime();
}

uint64 CVxMediaSysClock::GetTimeFromSample(uint64 clock)
{
	CMxMutexLocker lock(&m_csPulseLock);
	return m_clockpulse->GetTimeFromSample(clock);
}

uint64  CVxMediaSysClock::GetSampleFromTime(uint64 coretime)
{
	CMxMutexLocker lock(&m_csPulseLock);
	return m_clockpulse->GetSampleFromTime(coretime);
}

//#define TESTCLOCK

void vxTrace(const char* format, ...)
{
	va_list args;
#define MAXREPORTMESSAGESIZE 2048
	char    message[MAXREPORTMESSAGESIZE] = { 0 };

	va_start(args, format);
	vsnprintf(message, MAXREPORTMESSAGESIZE, format, args);
	va_end(args);
	int len = (int)strlen(message);
	if (message[len - 2] == '\n'&&message[len - 1] == '\n')
		message[len - 1] = 0;
#ifndef _WIN32	
	char utf8[MAXREPORTMESSAGESIZE] = { 0 };
	//gbk2utf8(message, utf8, MAXREPORTMESSAGESIZE);
	fwrite(utf8, strlen(utf8), 1, stdout);
#else	
	OutputDebugStringA(message);
#endif	
}

#define TESTCLOCK
void CVxMediaSysClock::_ClockThread()
{
	//_vxSetThreadName("软件时钟事件分发");

#ifdef TESTCLOCK
	uint64 ulllastclock = 0;
	uint64 ullsteptime = m_clockpulse->GetTimeFromSample(1);
#endif
	while(!m_bExitClock)
	{
		if (m_hCoreClock)
		{
			if (mxWaitObject(m_hCoreClock, 1000) != WAIT_OK) continue;
		}
		else
		{
			if (m_clockpulse->WaitForClockPluse(1000) != WAIT_OK) continue;
		}

		uint64 rtNow = m_clockpulse->GetTime();
		uint64 clocktime = m_clockpulse->GetSampleFromTime(rtNow);
		if(m_ulClock!=clocktime)
		{
			m_ulClock = clocktime;
#ifdef TESTCLOCK
			uint64 ss = rtNow - ullsteptime*m_ulClock;
			//mx_debug("Time:%llu,%8lu,Sysclock %d,%d---[%d]\n",rtNow, ss,m_ulClock,ulllastclock,m_ulClock-ulllastclock);
            mx_debug("Time:%llu,%8lu,Sysclock %llu,%llu---[%llu]\n",rtNow, ss,m_ulClock,ulllastclock,m_ulClock-ulllastclock);
            ulllastclock = m_ulClock;
#endif
			CMxMutexLocker lock(&m_csLock);
			int nSize = m_hClocks.GetSize();
			for(int i=0;i<nSize;i++) 
				mxActiveEvent(m_hClocks[i]);
		} else {
			mx_debug("Time loss!!!!!!!!!!!!!!!!!");
		}
	}
}

int __cdecl vxCreateSystemClock(CMxSharedObject* setup,const sysclk_cinfo* cinfo,IVxClockPulse* clockpulse,IVxSystemClock** obj)
{
	CVxMediaSysClock* pObj = new CVxMediaSysClock(cinfo);
	pObj->Initialize(clockpulse);
	/*if (!pObj->Initialize(clockpulse))
	{
		pObj->Uninitialize();
		delete pObj;
		return -1;
	}*/

	*obj = static_cast<IVxSystemClock*>(pObj);
	return  1;
	//return GetVxInterface(,(void**)obj);
}
