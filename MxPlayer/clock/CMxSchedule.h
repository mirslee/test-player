#pragma once

#include "../../MxCore/MxSynchronize.h"
//#include "../../MxCore/mxtypes.h"
#include "MxTypes.h"

const unsigned int RESOLUTION = 1;                      /* High resolution timer */
const int ADVISE_CACHE = 4;                     /* Default cache size */
const __int64 MAX_TIME = 0x7FFFFFFFFFFFFFFFll;   /* Maximum __int64 value */


class CMxSchedule
{
public:
	CMxSchedule(MxEvent ev);
	~CMxSchedule(void);
public:
	unsigned int GetAdviseCount();
	__int64 GetNextAdviseTime();


	// We need a method for derived classes to add advise packets, we return the cookie
	mxuvoidptr AddAdvisePacket( const __int64 & time1, const __int64 & time2, MxEvent h, bool periodic );
	// And a way to cancel
	int Unadvise(mxuvoidptr dwAdviseCookie);

	// Tell us the time please, and we'll dispatch the expired events.  We return the time of the next event.
	// NB: The time returned will be "useless" if you start adding extra Advises.  But that's the problem of
	// whoever is using this helper class (typically a clock).
	__int64 Advise( const __int64 & rtTime );

	// Get the event handle which will be set if advise time requires re-evaluation.
	MxEvent GetEvent() const { return m_ev; }
private:
	 class CVxAdvisePacket
	 {
	 public:
		 CVxAdvisePacket(){}

		 CVxAdvisePacket * m_next;
		 unsigned int       m_dwAdviseCookie;
		 __int64		 m_rtEventTime;      // Time at which event should be set
		 __int64		 m_rtPeriod;         // Periodic time
		 MxEvent         m_hNotify;          // Handle to event or semephore
		 bool            m_bPeriodic;        // TRUE => Periodic event

		 CVxAdvisePacket( CVxAdvisePacket * next, __int64 time ) : m_next(next), m_rtEventTime(time)
		 {}

		 void InsertAfter( CVxAdvisePacket * p )
		 {
			 p->m_next = m_next;
			 m_next    = p;
		 }

		 int IsZ() const // That is, is it the node that represents the end of the list
		 { return m_next == 0; }

		 CVxAdvisePacket * RemoveNext()
		 {
			 CVxAdvisePacket *const next = m_next;
			 CVxAdvisePacket *const new_next = next->m_next;
			 m_next = new_next;
			 return next;
		 }

		 void DeleteNext()
		 {
			 delete RemoveNext();
		 }

		 CVxAdvisePacket * Next() const
		 {
			 CVxAdvisePacket * result = m_next;
			 if (result->IsZ()) result = 0;
			 return result;
		 }

		 unsigned int Cookie() const
		 { return m_dwAdviseCookie; }
	 };
	 // Structure is:
	 // head -> elmt1 -> elmt2 -> z -> null
	 // So an empty list is:       head -> z -> null
	 // Having head & z as links makes insertaion,
	 // deletion and shunting much easier.
	 CVxAdvisePacket   head, z;            // z is both a tail and a sentry

	 volatile unsigned int  m_dwNextCookie;     // Strictly increasing
	 volatile unsigned int  m_dwAdviseCount;    // Number of elements on list

	 MxMutex        m_Serialize;

	 // AddAdvisePacket: adds the packet, returns the cookie (0 if failed)
	 mxuvoidptr AddAdvisePacket( CVxAdvisePacket * pPacket );
	 // Event that we should set if the packed added above will be the next to fire.
	 const MxEvent m_ev;

	 // A Shunt is where we have changed the first element in the
	 // list and want it re-evaluating (i.e. repositioned) in
	 // the list.
	 void ShuntHead();

	 // Rather than delete advise packets, we cache them for future use
	 CVxAdvisePacket * m_pAdviseCache;
	 unsigned int      m_dwCacheCount;
	 enum { dwCacheMax = 5 };             // Don't bother caching more than five

	 void Delete( CVxAdvisePacket * pLink );// This "Delete" will cache the Link
};
