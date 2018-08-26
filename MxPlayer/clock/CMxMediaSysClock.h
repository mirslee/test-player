#pragma once

#include "CMxReferenceClock.h"
#include "../../MxCore/MxSynchronize.h"
#include "../../MxCore/mxtypes.h"
#include "../MxCore/MxPointer.h"
#include "../MxCore/CMxArray.h"

#define LIID_IVxSystemClock		0xe0001000
struct MXPLAER_API IVxSystemClock
{
virtual const sysclk_cinfo*	__stdcall GetCreateInfo() = 0;
virtual void				__stdcall WaitSyncForSystemClock(unsigned __int64 clock) = 0;
virtual unsigned __int64 			__stdcall WaitSyncFrameClock() = 0;
virtual unsigned __int64			__stdcall GetClock() = 0;
virtual MxEvent				__stdcall CreateClockEvent(bool bManualReset = FALSE,bool bInitialState = FALSE) = 0;
virtual void				__stdcall CloseClockEvent(MxEvent) = 0;

virtual unsigned __int64			__stdcall GetTime() = 0;
virtual unsigned __int64			__stdcall GetTimeFromSample(unsigned __int64 clock) = 0;
virtual unsigned __int64			__stdcall GetSampleFromTime(unsigned __int64 coretime) = 0;

virtual bool				__stdcall CanReset() = 0;
virtual bool				__stdcall Reset(const sysclk_cinfo* cinfo, IVxClockPulse* clockpulse) = 0;

virtual IVxClockPulse*		__stdcall GetCurrentPulse() = 0;
};

MXPLAER_API int __cdecl vxCreateSystemClock(CMxSharedObject* setup, const sysclk_cinfo* cinfo, IVxClockPulse* clockpulse, IVxSystemClock** obj);

class CVxMediaSysClock : public IVxSystemClock
{
public:
	CVxMediaSysClock(const sysclk_cinfo* info);
	virtual ~CVxMediaSysClock(void);
protected:
	sysclk_cinfo m_cinfo;
	MX_MUTEX m_csLock;

	MX_MUTEX m_csFLock;
	MxEvent	m_hWaitSyncEvent;

#ifdef _WIN32
	UINT    m_TimerResolution;
#endif

	CMxSharedPointer<IVxClockPulse> m_clockpulse;
	MX_MUTEX m_csPulseLock;

	volatile unsigned _int64 m_ulClock;
	CMxArray<MxEvent, MxEvent&> m_hClocks;
	
	bool m_canreset;
public:
	bool Initialize(IVxClockPulse* clockpulse);
	void Uninitialize();
public:
	const sysclk_cinfo*	__stdcall GetCreateInfo(){ return &m_cinfo; }
	void			 __stdcall WaitSyncForSystemClock(unsigned _int64 dwClock);
	unsigned _int64		 __stdcall WaitSyncFrameClock();
	unsigned _int64		 __stdcall GetClock(){ return m_ulClock; }
	MxEvent			 __stdcall CreateClockEvent(bool bManualReset = false, bool bInitialState = false);
	void			 __stdcall CloseClockEvent(MxEvent);

	unsigned _int64		 __stdcall GetTime();
	unsigned _int64		 __stdcall GetTimeFromSample(unsigned _int64 clock);
	unsigned _int64		 __stdcall GetSampleFromTime(unsigned _int64 coretime);

	bool			 __stdcall Reset(const sysclk_cinfo* cinfo,IVxClockPulse* clockpulse);
	bool			 __stdcall CanReset(){return m_canreset;}
	
	IVxClockPulse*		__stdcall GetCurrentPulse(){return m_clockpulse;}
protected:
	long __stdcall NonDelegatingQueryInterface(LONG iid, void**);

	MxEvent m_hCoreClock;
	bool m_bExitClock;
	pthread_t m_hClockThread;
	static void* ClockThreadProc(void* lpThis){((CVxMediaSysClock*)lpThis)->_ClockThread();return 0;}
	void _ClockThread();

	void OnDelete(){Uninitialize();}

};
