
#ifndef MxSynchronize_h
#define MxSynchronize_h

#define _FACLS  0x008
#ifdef _WIN32
#define MAKE_RESULT( code )		MAKE_HRESULT( 1, _FACLS, code )
#else
#define MAKE_RESULT( code )     1
#endif

#define MX_S_OK							0
#define MX_E_FAIL						0x80004005	
#define MX_E_INVALIDARG					0x80070057	
#define MX_E_OUTOFMEMORY				MAKE_RESULT( 0x0003 )			//	ÄÚ´æ²»×ã
#define NOERROR 0

#define Mx_Failed(hr) ((hr)<0)

#ifdef _WIN32
#define Mx_inline __forceinline
#else
#define Mx_inline inline
#endif

#ifdef WIN32
#define INITPTHREAD(t) memset(&t,0,sizeof(t))
#define PTHREADISVALID(t) (t.p!=NULL)
#else	
#define INITPTHREAD(t) t = NULL;
#define PTHREADISVALID(t) (t!=NULL)
#endif

#ifdef _WIN32
#define WAIT_OK			WAIT_OBJECT_0
#define WAIT_INFINITE	INFINITE
//#define WAIT_TIMEOUT	ETIMEDOUT

#define MX_MUTEX CRITICAL_SECTION
typedef MX_MUTEX MxMutex;
#define MX_MUTEX_INIT(v) InitializeCriticalSection(v)
#define MX_MUTEX_DESTROY(v) DeleteCriticalSection(v)
#define MX_MUTEX_LOCK(v) EnterCriticalSection(v)
#define MX_MUTEX_TRYLOCK(v) (TryEnterCriticalSection(v) != 0)
#define MX_MUTEX_UNLOCK(v) LeaveCriticalSection(v)

#else
#define WAIT_OK			0
#define WAIT_TIMEOUT	ETIMEDOUT
#define WAIT_INFINITE	(unsigned long)(-1)
#endif


typedef void* MxEvent;
MXCORE_API MxEvent mxCreateEvent(pthread_condattr_t *attr, bool broadcast, bool active, const char* name);
MXCORE_API void mxCloseEvent(MxEvent event);
MXCORE_API bool mxActiveEvent(MxEvent event);
MXCORE_API bool mxInactiveEvent(MxEvent event);
MXCORE_API unsigned long mxWaitObject(MxEvent event, unsigned long dwMilliseconds);
MXCORE_API unsigned long mxWaitObjects(unsigned long  nCount, MxEvent event, unsigned long  dwMilliseconds);

#ifdef __APPLE__

#include <dispatch/dispatch.h>
#include <libkern/osatomic.h>
#include <assert.h>
class MxMutex_t
{
public:
    MxMutex_t();
    ~MxMutex_t();
    void lock();
    bool trylock();
    void unlock();
private:
	int32_t count;
	dispatch_semaphore_t sema;
	pthread_t ownerthread;
	int32_t recursive_count;
};

typedef MxMutex_t* MxMutex;
#define MX_MUTEX MxMutex
#define MX_MUTEX_INIT(v)  (*v) = new MxMutex_t
#define MX_MUTEX_DESTROY(v) delete (*v)
#define MX_MUTEX_LOCK(v) (*v)->lock()
#define MX_MUTEX_TRYLOCK(v) (*v)->trylock()
#define MX_MUTEX_UNLOCK(v) (*v)->unlock()

#define MxSem dispatch_semaphore_t
#define MxSemInit(v, v1, v2) (*v) = dispatch_semaphore_create(v1)
#define MxSemDestory(v) dispatch_release(v)
#define MxSemWait(v) dispatch_semaphore_wait(v, DISPATCH_TIME_FOREVER)
#define MxSemWaitTime(v, time) dispatch_semaphore_wait(v, dispatch_time(DISPATCH_TIME_NOW, time * NSEC_PER_MSEC))
#define MxSemSignal(v) dispatch_semaphore_signal(v)

#endif




#define szSTR( x )			#x
#define szSTR2( x )			szSTR(x)
#define szmessage( desc )	message( __FILE__"(" szSTR2(__LINE__) "):  " desc)

#define	RELEASEIF( p )			if( (p)!=NULL ){ (p)->Release(); p = NULL; }
#define	RELEASEIF_CHECK( p )	if( (p)!=NULL ){ VERIFY( (p)->Release() == 0 ); (p) = NULL; }
#define	CLOSEHANDLEEX( p )		if( (p)!=NULL ){ CloseHandle(p); (p) = NULL; } 
#define	CLOSEFILE( p )			if( (p)!=INVALID_HANDLE_VALUE ){ CloseHandle(p); (p) = INVALID_HANDLE_VALUE; } 
#define	FREELIBRARYEX( p )		if( (p)!=NULL ){ FreeLibrary(p); (p) = NULL; } 
#define	DELETEPTR( p )			if( (p)!=NULL ){ delete (p); (p) = NULL; } 
#define	DELETEARRPTR( p )		if( (p)!=NULL ){ delete[] (p); (p) = NULL; } 
#define	_DELETEPTR(c,p )		if( p ){ delete ((c*)p); (p) = NULL; }
#define	VIRTUALFREEEX( p )		if( (p)!=NULL ){ VirtualFree((p),0,MEM_RELEASE); (p) = NULL; } 
#define	FREEVXMALLOCPTR(p)		if( p ){ _vxfree(p); (p) = NULL; }


#define EXITTHREAD(p0,p1,p2)    if( (p0)!=NULL){ SetEvent(p1); WaitForSingleObject(p0,INFINITE);CLOSEHANDLEEX(p0);CLOSEHANDLEEX(p1);CLOSEHANDLEEX(p2)}
#define EXITTHREAD2(p0,p1)		if( (p0)!=NULL){ p1 = TRUE; WaitForSingleObject(p0,INFINITE);CLOSEHANDLEEX(p0);}
#define EXITPTHREAD(p0,p1)		if(PTHREADISVALID(p0)){ p1 = TRUE; pthread_join(p0,NULL);memset(&p0,0,sizeof(pthread_t));}
#define WAITPTHREAD(p0)			if(PTHREADISVALID(p0)){ pthread_join(p0,NULL);memset(&p0,0,sizeof(pthread_t));}
#define	CLOSEEVENTEX( p )		if( (p)!=NULL ){mxCloseEvent(p); (p) = NULL; }


class MXCORE_API CMxMutexLocker
{
public:
	CMxMutexLocker(MX_MUTEX* pMutex)
	{
		this->pMutex = pMutex;
		MX_MUTEX_LOCK(pMutex);
	};
	virtual ~CMxMutexLocker()
	{
		MX_MUTEX_UNLOCK(pMutex);
	}

protected:
	MX_MUTEX * pMutex;
};


struct _av_cinfo
{
	int rate, scale;
	int iochannels;
};

typedef struct _av_cinfo avio_cinfo;
typedef struct _av_cinfo sysclk_cinfo;

#endif
