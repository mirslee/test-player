#pragma once

#include "CMxReferenceClock.h"
#include "MxSynchronize.h"
#include "MxTypes.h"
#include "MxPointer.h"
#include "CMxArray.h"

#define LIID_IVxSystemClock		0xe0001000
struct MXPLAER_API IVxSystemClock
{
virtual const sysclk_cinfo*	__stdcall GetCreateInfo() = 0;
virtual void				__stdcall WaitSyncForSystemClock(uint64 clock) = 0;
virtual uint64			__stdcall WaitSyncFrameClock() = 0;
virtual uint64		__stdcall GetClock() = 0;
virtual CMxEvent				__stdcall CreateClockEvent(bool bManualReset = false,bool bInitialState = false) = 0;
virtual void				__stdcall CloseClockEvent(CMxEvent) = 0;

virtual uint64		__stdcall GetTime() = 0;
virtual uint64		__stdcall GetTimeFromSample(uint64 clock) = 0;
virtual uint64			__stdcall GetSampleFromTime(uint64 coretime) = 0;

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
	CMxMutex m_csLock;

	CMxMutex m_csFLock;
	CMxEvent	m_hWaitSyncEvent;

#ifdef _WIN32
	UINT    m_TimerResolution;
#endif

	CMxSharedPointer<IVxClockPulse> m_clockpulse;
	CMxMutex m_csPulseLock;

	volatile uint64 m_ulClock;
	CMxArray<CMxEvent, CMxEvent&> m_hClocks;
	
	bool m_canreset;
public:
	bool Initialize(IVxClockPulse* clockpulse);
	void Uninitialize();
public:
	const sysclk_cinfo*	__stdcall GetCreateInfo(){ return &m_cinfo; }
	void			 __stdcall WaitSyncForSystemClock(uint64 dwClock);
	uint64		 __stdcall WaitSyncFrameClock();
	uint64		 __stdcall GetClock(){ return m_ulClock; }
	CMxEvent			 __stdcall CreateClockEvent(bool bManualReset = false, bool bInitialState = false);
	void			 __stdcall CloseClockEvent(CMxEvent);

	uint64		 __stdcall GetTime();
	uint64		 __stdcall GetTimeFromSample(uint64 clock);
	uint64		 __stdcall GetSampleFromTime(uint64 coretime);

	bool			 __stdcall Reset(const sysclk_cinfo* cinfo,IVxClockPulse* clockpulse);
	bool			 __stdcall CanReset(){return m_canreset;}
	
	IVxClockPulse*		__stdcall GetCurrentPulse(){return m_clockpulse;}
protected:
	long __stdcall NonDelegatingQueryInterface(LONG iid, void**);

	CMxEvent m_hCoreClock;
	bool m_bExitClock;
	pthread_t m_hClockThread;
	static void* ClockThreadProc(void* lpThis){((CVxMediaSysClock*)lpThis)->_ClockThread();return 0;}
	void _ClockThread();

	void OnDelete(){Uninitialize();}

};
