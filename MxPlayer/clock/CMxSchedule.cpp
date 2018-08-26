#include "stdafx.h"
#include "CMxSchedule.h"
//#include "vxtempl.h"
//#include "vxerror.h"

CMxSchedule::CMxSchedule(MxEvent ev)
: head(&z, 0), z(0, MAX_TIME)
, m_dwNextCookie(0), m_dwAdviseCount(0)
, m_pAdviseCache(0), m_dwCacheCount(0)
, m_ev( ev )
{
	MX_MUTEX_INIT(&m_Serialize);
	head.m_dwAdviseCookie = z.m_dwAdviseCookie = 0;
}

CMxSchedule::~CMxSchedule(void)
{
	// Delete cache
	CVxAdvisePacket * p = m_pAdviseCache;
	while (p)
	{
		CVxAdvisePacket *const p_next = p->m_next;
		delete p;
		p = p_next;
	}

	assert( m_dwAdviseCount == 0 );
	// Better to be safe than sorry
	if ( m_dwAdviseCount > 0 )
	{
		while ( !head.m_next->IsZ() )
		{
			head.DeleteNext();
			--m_dwAdviseCount;
		}
	}
	MX_MUTEX_DESTROY(&m_Serialize);
}


/* Public methods */

unsigned int CMxSchedule::GetAdviseCount()
{
	// No need to lock, m_dwAdviseCount is 32bits & declared volatile-
	return m_dwAdviseCount;
}

__int64 CMxSchedule::GetNextAdviseTime()
{
	CMxMutexLocker lck(&m_Serialize); // Need to stop the linked list from changing
	return head.m_next->m_rtEventTime;
}

mxuvoidptr CMxSchedule::AddAdvisePacket( const __int64 & time1 , const __int64 & time2, MxEvent h,bool periodic)
{
	// Since we use MAX_TIME as a sentry, we can't afford to
	// schedule a notification at MAX_TIME
	assert( time1 < MAX_TIME );
	unsigned int Result;
	CVxAdvisePacket * p;

	CMxMutexLocker lock(&m_Serialize);

	if (m_pAdviseCache)
	{
		p = m_pAdviseCache;
		m_pAdviseCache = p->m_next;
		--m_dwCacheCount;
	}
	else
	{
		p = new CVxAdvisePacket();
	}
	if (p)
	{
		p->m_rtEventTime = time1; p->m_rtPeriod = time2;
		p->m_hNotify = h; p->m_bPeriodic = periodic;
		Result = AddAdvisePacket( p );
	}
	else Result = 0;


	return Result;
}

int CMxSchedule::Unadvise(mxuvoidptr dwAdviseCookie)
{
	int hr = MX_E_FAIL;
	CVxAdvisePacket * p_prev = &head;
	CVxAdvisePacket * p_n;
	CMxMutexLocker lock(&m_Serialize);
	while ( p_n = p_prev->Next() ) // The Next() method returns NULL when it hits z
	{
		if ( p_n->m_dwAdviseCookie == dwAdviseCookie )
		{
			Delete( p_prev->RemoveNext() );
			--m_dwAdviseCount;
			hr = MX_S_OK;
			break;
		}
		p_prev = p_n;
	};
	return hr;
}

__int64 CMxSchedule::Advise( const __int64 & rtTime )
{
	__int64  rtNextTime;
	CVxAdvisePacket * pAdvise;

	CMxMutexLocker lock(&m_Serialize);

	while (rtTime>=(rtNextTime=(pAdvise=head.m_next)->m_rtEventTime)&&!pAdvise->IsZ() )
	{
		if (pAdvise->m_bPeriodic == TRUE)
		{
			mxActiveEvent(pAdvise->m_hNotify);
			pAdvise->m_rtEventTime += pAdvise->m_rtPeriod;
			ShuntHead();
		}
		else
		{
			mxActiveEvent(pAdvise->m_hNotify);
			--m_dwAdviseCount;
			Delete( head.RemoveNext() );
		}
	}
	return rtNextTime;
}

/* Private methods */

mxuvoidptr CMxSchedule::AddAdvisePacket(CVxAdvisePacket * pPacket )
{
	CVxAdvisePacket * p_prev = &head;
	CVxAdvisePacket * p_n;

	const unsigned int Result = pPacket->m_dwAdviseCookie = ++m_dwNextCookie;
	// This relies on the fact that z is a sentry with a maximal m_rtEventTime
	for(;;p_prev = p_n)
	{
		p_n = p_prev->m_next;
		if ( p_n->m_rtEventTime >= pPacket->m_rtEventTime ) break;
	}
	p_prev->InsertAfter( pPacket );
	++m_dwAdviseCount;

	// If packet added at the head, then clock needs to re-evaluate wait time.
	if ( p_prev == &head ) mxActiveEvent( m_ev );

	return Result;
}

void CMxSchedule::Delete(CVxAdvisePacket * pPacket )
{
	if ( m_dwCacheCount >= dwCacheMax ) delete pPacket;
	else
	{
		CMxMutexLocker lock(&m_Serialize);
		pPacket->m_next = m_pAdviseCache;
		m_pAdviseCache = pPacket;
		++m_dwCacheCount;
	}
}


// Takes the head of the list & repositions it
void CMxSchedule::ShuntHead()
{
	CVxAdvisePacket * p_prev = &head;
	CVxAdvisePacket * p_n;

	CMxMutexLocker lock(&m_Serialize);

	CVxAdvisePacket *const pPacket = head.m_next;
	// This relies on the fact that z is a sentry with a maximal m_rtEventTime
	for(;;p_prev = p_n)
	{
		p_n = p_prev->m_next;
		if ( p_n->m_rtEventTime > pPacket->m_rtEventTime ) break;
	}
	// If p_prev == pPacket then we're already in the right place
	if (p_prev != pPacket)
	{
		head.m_next = pPacket->m_next;
		(p_prev->m_next = pPacket)->m_next = p_n;
	}
}


