// AllVisionReferenceClock.h: vxinterface for the CAllVisionReferenceClock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __BAY_VXREFERENCECLOCK_H__
#define __BAY_VXREFERENCECLOCK_H__

#include "CMxSchedule.h"
#include "pthread.h"
#include "../../MxCore/MxSynchronize.h"
#include "../../MxCore/mxtypes.h"
#include "../../MxCore/MxObject.h"

typedef struct
{
	void* p;
	__int64(*gettime)(void* p);
	void(*destroy)(void* p);
}CLOCKFUNCS;

#define LIID_IVxClockPulse		0xe0001001
struct IVxClockPulse: CMxSharedObject
{
	virtual MxEvent				__stdcall GetFieldEvent() = 0;
virtual unsigned __int64			__stdcall GetTime() = 0;
virtual unsigned __int64			__stdcall GetTimeFromSample(unsigned __int64 clock) = 0;
virtual unsigned __int64			__stdcall GetSampleFromTime(unsigned __int64 coretime) = 0;
virtual unsigned int				__stdcall WaitForClockPluse(unsigned int timeout) = 0;
};

int __cdecl mxClockPulse(CMxSharedObject*, const sysclk_cinfo* cinfo, CLOCKFUNCS* funcs, IVxClockPulse** obj);



struct IVxReferenceClock: CMxSharedObject
{
public:
    virtual int __stdcall GetTime( __int64 *pTime) = 0;
    
    virtual int __stdcall AdviseTime( __int64 baseTime,__int64 streamTime,MxEvent hEvent, mxuvoidptr *pdwAdviseCookie) = 0;
    
    virtual int __stdcall AdvisePeriodic(__int64 startTime,__int64 periodTime,MxEvent hSemaphore, mxuvoidptr *pdwAdviseCookie) = 0;
    
    virtual int __stdcall Unadvise( mxuvoidptr dwAdviseCookie) = 0;

	virtual int __stdcall Reset( CLOCKFUNCS* funcs) = 0;

};

class CVxReferenceClock : public IVxReferenceClock
{
public:
	CVxReferenceClock(CLOCKFUNCS* funcs);
	virtual ~CVxReferenceClock();
public:
	__int64 m_rtPreRoll;
public:
	LONG __stdcall NonDelegatingQueryInterface(LONG riid,void ** ppv);
	int __stdcall GetTime(__int64 *pTime);
	int __stdcall AdviseTime(__int64 rtBaseTime, __int64 rtStreamTime, MxEvent hEvent, mxuvoidptr *pdwAdviseCookie);

	int __stdcall AdvisePeriodic(__int64 rtStartTime, __int64 rtPeriodTime, MxEvent hSemaphore, mxuvoidptr* pdwAdviseCookie);
	int __stdcall Unadvise(mxuvoidptr dwAdviseCookie);

	int __stdcall Reset( CLOCKFUNCS* funcs);

    virtual __int64 GetPrivateTime();
	int SetTimeDelta( const __int64& TimeDelta );
    CMxSchedule * GetSchedule() const { return m_pSchedule; }
private:
	MX_MUTEX m_csLock;
    __int64 m_rtPrivateTime;     // Current best estimate of time
    __int64 m_rtLastGotTime;     // Last time returned by GetTime
    __int64 m_rtNextAdvise;      // Time of next advise
#ifdef _WIN32
	DWORD   m_dwPrevSystemTime;  // Last vaule we got from timeGetTime
    UINT    m_TimerResolution;
#endif
public:
    void TriggerThread()
    {					
		mxActiveEvent(m_pSchedule->GetEvent());
    }

private:
    bool           m_bAbort;            // Flag used for thread shutdown
    pthread_t      m_hThread;           // Thread handle
	static void* AdviseThreadFunction(void* p){((CVxReferenceClock*)p)->__AdviseThread();return 0;} 
	void __AdviseThread();             // Method in which the advise thread runs
protected:
	CMxSchedule * const m_pSchedule;

	bool m_extfunc;
	CLOCKFUNCS m_funcs;
};

#endif