// VxReferenceClock.cpp: implementation of the CVxReferenceClock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CMxReferenceClock.h"
#include "MxPointer.h"
#include "MxError.h"

#ifdef _WIN32
#include "mmsystem.h"
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"strmiids.lib")
#elif defined(__APPLE__)
#include "CoreAudio/HostTime.h"

#endif

/*Mx_inline int mxgettimestep(int rate, int scale, VXSURFACE_SCANTYPE scantype, int& framestep)
{
	framestep = 1;
	int timestep = scantype == SCANTYPE_PROGRESSIVE ? 1 : 2;
	if ((rate*timestep / scale) > 60)
	{
		framestep = 2;
		//当帧率比较高的时候用framstep来控制一次执行的时间，保证一次执行大于40ms
		int fieldrate = rate*timestep;
		while ((framestep * 1000 * scale / fieldrate) < 40)
			framestep *= 2;
	}
	return timestep;
}*/

Mx_inline int mxgettimestep(const sysclk_cinfo* cinfo, int& framestep)
{
	/*framestep = 1;
	return __vxgettimestep(cinfo->rate, cinfo->scale, cinfo->scantype, framestep);*/
	framestep = 1;
	return 1;
}


const __int64 MILLISECONDS = (1000);            // 10 ^ 3
const __int64 NANOSECONDS = (1000000000);       // 10 ^ 9
const __int64 UNITS = (NANOSECONDS / 100);      // 10 ^ 7

CVxReferenceClock::CVxReferenceClock(CLOCKFUNCS* funcs)
: m_rtLastGotTime(0)
, m_bAbort( FALSE )
, m_pSchedule(new CMxSchedule(mxCreateEvent(nullptr, false, false, nullptr)) )
, m_rtPreRoll(2)
, m_extfunc(FALSE)
{
	mxMutexInit(&m_csLock);
	INITPTHREAD(m_hThread);

	if(funcs)
	{
		memcpy(&m_funcs,funcs,sizeof(CLOCKFUNCS));
		m_extfunc = TRUE;
	}

	if(m_extfunc)
	{
		m_rtPrivateTime = m_funcs.gettime(m_funcs.p);
	}
	else
	{
#ifdef _WIN32
		TIMECAPS tc;
		m_TimerResolution = (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(tc)))
			? tc.wPeriodMin
			: 1;
		timeBeginPeriod(m_TimerResolution);
		m_dwPrevSystemTime = timeGetTime();
		m_rtPrivateTime = (UNITS / MILLISECONDS) * m_dwPrevSystemTime;
#elif defined(__APPLE__)
		m_rtPrivateTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime())/100;
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        m_rtPrivateTime = ts.tv_sec*10000000+ts.tv_nsec/100;
#endif
	}
    
#ifdef _WIN32
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr,SCHED_RR);
    int max_priority = sched_get_priority_max(SCHED_RR);;
    struct sched_param param = {0};
    pthread_attr_getschedparam(&attr,&param);
    param.sched_priority = max_priority;
    pthread_attr_setschedparam(&attr,&param);
	pthread_create(&m_hThread,&attr,AdviseThreadFunction,this);
	pthread_attr_destroy(&attr);
#endif
}

CVxReferenceClock::~CVxReferenceClock()
{
	if (PTHREADISVALID(m_hThread))
	{
		m_bAbort = TRUE;
		mxSetEvent(m_pSchedule->GetEvent());
		pthread_join(m_hThread,NULL);
		mxCloseEvent(m_pSchedule->GetEvent());
		delete m_pSchedule;
	}
	
	if(m_extfunc)
	{
		if(m_funcs.destroy) m_funcs.destroy(m_funcs.p);
	}
	else
	{
#ifdef _WIN32
		if (m_TimerResolution) timeEndPeriod(m_TimerResolution);
#endif

	}
	mxMutexDestroy(&m_csLock);
}

int CVxReferenceClock::Reset( CLOCKFUNCS* funcs)
{
	if (PTHREADISVALID(m_hThread))
	{
		m_bAbort = TRUE;
		mxSetEvent(m_pSchedule->GetEvent());
		pthread_join(m_hThread,NULL);
	}

	if(m_extfunc)
	{
		m_funcs.destroy(m_funcs.p);
		m_extfunc = FALSE;
	}

	if(funcs)
	{
		memcpy(&m_funcs,funcs,sizeof(CLOCKFUNCS));
		m_extfunc = true;
	}

	if(m_extfunc)
	{
		m_rtPrivateTime = m_funcs.gettime(m_funcs.p);
	}
	else
	{
#ifdef _WIN32
		TIMECAPS tc;
		m_TimerResolution = (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(tc)))
			? tc.wPeriodMin
			: 1;
		timeBeginPeriod(m_TimerResolution);
		m_dwPrevSystemTime = timeGetTime();
		m_rtPrivateTime = (UNITS / MILLISECONDS) * m_dwPrevSystemTime;
#elif defined(__APPLE__)
		m_rtPrivateTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime())/100;
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        m_rtPrivateTime = ts.tv_sec*10000000+ts.tv_nsec/100;
#endif
	}
    
#ifdef _WIN32
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr,SCHED_RR);
	int max_priority = sched_get_priority_max(SCHED_RR);;
	struct sched_param param = {0};
	pthread_attr_getschedparam(&attr,&param);
	param.sched_priority = max_priority;
	pthread_attr_setschedparam(&attr,&param);	
	pthread_create(&m_hThread,&attr,AdviseThreadFunction,this);
	pthread_attr_destroy(&attr);
#endif
    
	return true;
}

LONG CVxReferenceClock::NonDelegatingQueryInterface(LONG riid,void ** ppv)
{
/*
    if (riid == IID_IReferenceClock)
    {
        return GetInterface(static_cast<IReferenceClock *>(this), ppv);
    }
    else
*/
   /* {
        return CVxObject::NonDelegatingQueryInterface(riid, ppv);
    }*/

	return 0;
}

int CVxReferenceClock::GetTime(__int64 *pTime)
{
	int hr;
    if (pTime)
    {
        __int64 rtNow;
		CMxMutexLocker lock(&m_csLock);
        rtNow = GetPrivateTime();
        if (rtNow > m_rtLastGotTime)
        {
            m_rtLastGotTime = rtNow;
            hr = MX_S_OK;
        }
        else
        {
            hr = MX_E_FAIL;
        }
        *pTime = m_rtLastGotTime;
 
    }
    else hr = MX_E_INVALIDARG;

    return hr;
}

Mx_inline __int64 ConvertToMilliseconds(const __int64& RT)
{
	/* This converts an arbitrary value representing a reference time
	into a MILLISECONDS value for use in subsequent system calls */

	return (RT / (UNITS / MILLISECONDS));
}


/* Ask for an async notification that a time has elapsed */
int CVxReferenceClock::AdviseTime(__int64 rtBaseTime, __int64 rtStreamTime,CMxEvent hEvent, mxuvoidptr* pdwAdviseCookie)
{
    *pdwAdviseCookie = 0;

    int hr;

    const __int64 lRefTime = rtBaseTime + rtStreamTime;
    if ( lRefTime <= 0 || lRefTime == MAX_TIME )
    {
        hr = MX_E_INVALIDARG;
    }
    else
    {
        *pdwAdviseCookie = m_pSchedule->AddAdvisePacket( lRefTime, 0, hEvent, FALSE );
        hr = *pdwAdviseCookie ? NOERROR : MX_E_OUTOFMEMORY;
    }

    return hr;
}


/* Ask for an asynchronous periodic notification that a time has elapsed */

int CVxReferenceClock::AdvisePeriodic( __int64 rtStartTime, __int64 rtPeriodTime,CMxEvent hSemaphore, mxuvoidptr* pdwAdviseCookie)
{
    *pdwAdviseCookie = 0;

	int hr;
    if (rtStartTime > 0 && rtPeriodTime > 0 && rtStartTime != MAX_TIME )
    {
        *pdwAdviseCookie = m_pSchedule->AddAdvisePacket( rtStartTime, rtPeriodTime, hSemaphore/*HANDLE(hSemaphore)*/, TRUE );
        hr = *pdwAdviseCookie ? NOERROR : MX_E_OUTOFMEMORY;
    }
    else hr = MX_E_INVALIDARG;

    return hr;
}


int CVxReferenceClock::Unadvise(mxuvoidptr dwAdviseCookie)
{

	int hr = m_pSchedule->Unadvise(dwAdviseCookie);

    return hr;
}


__int64 CVxReferenceClock::GetPrivateTime()
{
	CMxMutexLocker lock(&m_csLock);

	if(m_extfunc)
	{
		m_rtPrivateTime = m_funcs.gettime(m_funcs.p);
	}
	else
	{
#ifdef _WIN32
		DWORD dwTime = timeGetTime();
		{
			m_rtPrivateTime += Int32x32To64(UNITS / MILLISECONDS, (DWORD)(dwTime - m_dwPrevSystemTime));
			m_dwPrevSystemTime = dwTime;
		}
#elif defined(__APPLE__)
		m_rtPrivateTime = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime())/100;
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        m_rtPrivateTime = ts.tv_sec*10000000+ts.tv_nsec/100;
#endif
	}
    return m_rtPrivateTime;
}


/* Adjust the current time by the input value.  This allows an
   external time source to work out some of the latency of the clock
   system and adjust the "current" time accordingly.  The intent is
   that the time returned to the user is synchronised to a clock
   source and allows drift to be catered for.

   For example: if the clock source detects a drift it can pass a delta
   to the current time rather than having to set an explicit time.
*/

int CVxReferenceClock::SetTimeDelta(const __int64 & TimeDelta)
{
	CMxMutexLocker lock(&m_csLock);
    m_rtPrivateTime += TimeDelta;
    // If time goes forwards, and we have advises, then we need to
    // trigger the thread so that it can re-evaluate its wait time.
    // Since we don't want the cost of the thread switches if the change
    // is really small, only do it if clock goes forward by more than
    // 0.5 millisecond.  If the time goes backwards, the thread will
    // wake up "early" (relativly speaking) and will re-evaluate at
    // that time.
    if ( TimeDelta > 5000 && m_pSchedule->GetAdviseCount() > 0 ) TriggerThread();

    return NOERROR;
}




void CVxReferenceClock::__AdviseThread()
{
	//_vxSetThreadName("Windows多媒体时钟");

    DWORD dwWait = WAIT_INFINITE;

    // The first thing we do is wait until something interesting happens
    // (meaning a first advise or shutdown).  This prevents us calling
    // GetPrivateTime immediately which is goodness as that is a virtual
    // routine and the derived class may not yet be constructed.  (This
    // thread is created in the base class constructor.)

    while ( !m_bAbort )
    {
        mxWaitEvent(m_pSchedule->GetEvent(), dwWait);
        if (m_bAbort) break;

        // There are several reasons why we need to work from the internal
        // time, mainly to do with what happens when time goes backwards.
        // Mainly, it stop us looping madly if an event is just about to
        // expire when the clock goes backward (i.e. GetTime stop for a
        // while).
        const __int64  rtNow = GetPrivateTime();
        // We must add in a millisecond, since this is the resolution of our
        // WaitForSingleObject timer.  Failure to do so will cause us to loop
        // franticly for (approx) 1 a millisecond.
        m_rtNextAdvise = m_pSchedule->Advise( 10000 + rtNow );
        __int64 llWait = m_rtNextAdvise - rtNow;
        assert( llWait > 0 );
        llWait = ConvertToMilliseconds(llWait);

        dwWait = (llWait > __int64(UINT_MAX)) ? UINT_MAX : DWORD(llWait);
    };
}


int CreateReferenceClock(CLOCKFUNCS* funcs,MxReferenceClock** refclock)
{
	//return GetVxInterface(static_cast<IVxReferenceClock*>(new CVxReferenceClock(funcs)),(void**)refclock);
	*refclock = static_cast<MxReferenceClock*>(new CVxReferenceClock(funcs));
	return 0;
}

class CMxClockPulse :public MxClockPulse, public CMxObject
{
    MX_OBJECT
public:
	CMxClockPulse(const sysclk_cinfo* cinfo);
protected:
	CMxObjectPointer<MxReferenceClock> m_refclock;
	int m_rate, m_scale;
	int m_timestep;
	int m_framestep;

	unsigned int m_uiD;
	unsigned int m_uiTD;

	CMxEvent m_hCoreClock;
	mxuvoidptr m_dwTimeID;
public:
	bool Initialize(CLOCKFUNCS* funcs);
	void Uninitialize();
public:
	CMxEvent				__stdcall GetFieldEvent(){return m_hCoreClock;}
	uint64	__stdcall GetTime();
	uint64			__stdcall GetTimeFromSample(uint64 clock);
	uint64			__stdcall GetSampleFromTime(uint64 coretime);
	unsigned int				__stdcall WaitForClockPluse(unsigned int timeout) { return mxWaitEvent(m_hCoreClock,timeout); }
protected:
	void OnDelete(){Uninitialize();}
    
private:
    bool         m_bExit;            // Flag used for thread shutdown
    pthread_t      m_hThread;           // Thread handle
    static void* AdviseThreadFunction(void* p){((CMxClockPulse*)p)->__AdviseThread();return 0;}
    void __AdviseThread();             // Method in which the advise thread runs
};


CMxClockPulse::CMxClockPulse(const sysclk_cinfo* cinfo)
{
    INITPTHREAD(m_hThread);

	m_rate = cinfo->rate;
	m_scale = cinfo->scale;
	m_timestep = mxgettimestep(cinfo, m_framestep);

	switch (m_rate)
	{
	case 24:
		m_uiD = 5;
		m_uiTD = 125000;
		break;
	case 2398:
	case 24000:
		m_uiD = 5;
		m_uiTD = 125125;
		break;
	case 25:
	case 50:
		m_uiD = 4;
		m_uiTD = 150000;
		break;
	case 30:
	case 60:
		m_uiD = 4;
		m_uiTD = 125000;
		break;
	case 2997:
	case 30000:
	case 60000:
		m_uiD = 4;
		m_uiTD = 125125;
		break;
	default:
		{
			int d_td = 10000000ll * m_scale * 3 / (m_rate*2);
			m_uiD = 4;
			m_uiTD = d_td / m_uiD;
			break;
		}
	}
	/*if (((m_rate / m_scale) <= 30) && (cinfo->scantype == SCANTYPE_PROGRESSIVE))
		m_uiD *= 2;*/
}

#if __APPLE__
#define SUBTIME 5000
#else
#define SUBTIME 0
#endif
bool CMxClockPulse::Initialize(CLOCKFUNCS* funcs)
{
	if(CreateReferenceClock(funcs,&m_refclock)!=0) return false;

    m_hCoreClock = mxCreateEvent(nullptr, false, false,nullptr);

#ifdef _WIN32
    double fps = (double)m_rate*m_timestep / m_scale;
    __int64 rtPeriodTime = (__int64)(10000000 * m_framestep / fps);
    
    int padtime = m_timestep - 1;
	unsigned __int64 timemask = padtime;
    timemask = ~timemask;
    
    __int64 rtStart;
    m_refclock->GetTime(&rtStart);
	unsigned __int64 samples = GetSampleFromTime(rtStart);
    rtStart = GetTimeFromSample((samples + 8 + padtime)&timemask);
    
    int hr = m_refclock->AdvisePeriodic(rtStart,rtPeriodTime-SUBTIME,m_hCoreClock,&m_dwTimeID);
	if(Mx_Failed(hr)) return false;
#else
    m_bExit = FALSE;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr,SCHED_RR);
    int max_priority = sched_get_priority_max(SCHED_RR);;
    struct sched_param param = {0};
    pthread_attr_getschedparam(&attr,&param);
    param.sched_priority = max_priority;
    pthread_attr_setschedparam(&attr,&param);
    pthread_create(&m_hThread,&attr,AdviseThreadFunction,this);
    pthread_attr_destroy(&attr);
#endif
	return true;
}

void CMxClockPulse::Uninitialize()
{
    EXITPTHREAD(m_hThread,m_bExit);
   
	if(m_refclock)
	{
#ifdef _WIN32
		m_refclock->Unadvise(m_dwTimeID);
#endif
		m_refclock = NULL;
	}
	CLOSEEVENTEX(m_hCoreClock);
}


Mx_inline uint64 vxRound(uint64 in_ui64Numerator, uint64 in_ui64Denominator)
{
	return (in_ui64Numerator % in_ui64Denominator) ? ((in_ui64Numerator + in_ui64Denominator) / in_ui64Denominator) : (in_ui64Numerator / in_ui64Denominator);
}

uint64 CMxClockPulse::GetTime()
{
	__int64 rtStart;
	m_refclock->GetTime(&rtStart);
	return rtStart;
}

uint64 CMxClockPulse::GetTimeFromSample(uint64 clock)
{
	return vxRound(clock*m_uiD*m_uiTD, 3);
}

uint64  CMxClockPulse::GetSampleFromTime(uint64 coretime)
{
	return (3 * coretime) / (m_uiTD* m_uiD);
}

#if __APPLE__
#include <mach/mach_time.h>
void CMxClockPulse::__AdviseThread()
{
    //_vxSetThreadName("OSX多媒体时钟");
    
    mach_timebase_info_data_t tinfo;
    mach_timebase_info(&tinfo);
    double hTime2nsFactor = (double)tinfo.numer / tinfo.denom;
    
    __uint64 curtime = GetTime();
    __uint64 begintime = GetTimeFromSample(GetSampleFromTime(curtime));
    __uint64 steptime = GetTimeFromSample(1);
    __uint64 nexttime = begintime + steptime;
    __uint64 waittime = nexttime-curtime;
    assert(nexttime>curtime);
    while (!m_bExit)
    {
        uint64_t sleepTimeInTicks = (uint64_t)(waittime*100 / hTime2nsFactor);
        mach_wait_until(mach_absolute_time() + sleepTimeInTicks);
        mxSetEvent(m_hCoreClock);
        nexttime += steptime;
        curtime = GetTime();
        while (nexttime<curtime) nexttime += steptime;
        waittime = nexttime-curtime;
    }
}
#elif __linux__
void CMxClockPulse::__AdviseThread()
{
    _vxSetThreadName("Linux多媒体时钟");

    __uint64 curtime = GetTime();
    __uint64 begintime = GetTimeFromSample(GetSampleFromTime(curtime));
    __uint64 steptime = GetTimeFromSample(1);
    __uint64 nexttime = begintime + steptime;
    __uint64 waittime = nexttime-curtime;
    ASSERT(nexttime>curtime);
    while (!m_bExit)
    {
        struct timeval tv = {0,waittime/10};
        select(0,NULL,NULL,NULL,&tv);
        vxSetEvent(m_hCoreClock);
        nexttime += steptime;
        curtime = GetTime();
        while (nexttime<curtime) nexttime += steptime;
        waittime = nexttime-curtime;
    }
}
#endif

int __cdecl mxClockPulse(MxObject*, const sysclk_cinfo* cinfo, CLOCKFUNCS* funcs, MxClockPulse** obj)
{
	CMxClockPulse* pObj = new CMxClockPulse(cinfo);
	if(!pObj->Initialize(funcs))
	{
		pObj->Uninitialize();
		delete pObj;
		return -1;
	}
	*obj = static_cast<MxClockPulse*>(pObj);
	return 0;
	//return GetVxInterface(static_cast<IVxClockPulse*>(pObj),(void**)obj);
}
