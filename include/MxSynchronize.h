
#ifndef MxSynchronize_h
#define MxSynchronize_h

#ifdef _WIN32
#define Mx_inline __forceinline
#else
#define Mx_inline inline
#endif

#ifdef WIN32
#include <windows.h>

#define WAIT_OK             WAIT_OBJECT_0
#define WAIT_INFINITE       INFINITE

#define CMxMutex            CRITICAL_SECTION
#define mxMutexInit(mtx)    InitializeCriticalSection(mtx)
#define mxMutexDestroy(mtx) DeleteCriticalSection(mtx)
#define mxMutexLock(mtx)    EnterCriticalSection(mtx)
#define mxMutexTrylock(mtx) (TryEnterCriticalSection(mtx) != 0)
#define mxMutexUnlock(mtx)  LeaveCriticalSection(mtx)

#define CMxMutex            HANDLE
#define mxCreateEvent       CreateEvent
#define mxCloseEvent        CloseHandle
#define mxSetEvent          SetEvent
#define mxResetEvent        ResetEvent
#define mxWaitEvent         WaitForSingleObject
#define mxWaitEvents(c, h, m)    WaitForMultipleObjects(c, h, TRUE, m)

#else

#define WAIT_OK             0
#define WAIT_TIMEOUT        ETIMEDOUT
#define WAIT_INFINITE       (unsigned long)(-1)

typedef void* CMxMutex;
void mxMutexInit(CMxMutex* mtx);
void mxMutexDestroy(CMxMutex* mtx);
void mxMutexLock(CMxMutex* mtx);
bool mxMutexTrylock(CMxMutex* mtx);
void mxMutexUnlock(CMxMutex* mtx);

typedef void* CMxEvent;
CMxEvent mxCreateEvent(void* lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName);
void mxCloseEvent(CMxEvent event);
bool mxSetEvent(CMxEvent event);
bool mxResetEvent(CMxEvent event);
unsigned long mxWaitEvent(CMxEvent event, unsigned long dwMilliseconds);
unsigned long mxWaitEvents(unsigned long  nCount, CMxEvent* lpEvents, unsigned long  dwMilliseconds);
#endif



#ifdef WIN32
#define INITPTHREAD(t) memset(&t,0,sizeof(t))
#define PTHREADISVALID(t) (t.p!=NULL)
#else	
#define INITPTHREAD(t) t = NULL;
#define PTHREADISVALID(t) (t!=NULL)
#endif


/*typedef void* MxEvent;
MXCORE_API MxEvent mxCreateEvent(pthread_condattr_t *attr, bool broadcast, bool active, const char* name);
MXCORE_API void mxCloseEvent(MxEvent event);
MXCORE_API bool mxActiveEvent(MxEvent event);
MXCORE_API bool mxInactiveEvent(MxEvent event);
MXCORE_API unsigned long mxWaitObject(MxEvent event, unsigned long dwMilliseconds);
MXCORE_API unsigned long mxWaitObjects(unsigned long  nCount, MxEvent event, unsigned long  dwMilliseconds);*/


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
#define EXITPTHREAD(p0,p1)		if(PTHREADISVALID(p0)){ p1 = true; pthread_join(p0,NULL);memset(&p0,0,sizeof(pthread_t));}
#define WAITPTHREAD(p0)			if(PTHREADISVALID(p0)){ pthread_join(p0,NULL);memset(&p0,0,sizeof(pthread_t));}
#define	CLOSEEVENTEX( p )		if( (p)!=NULL ){mxCloseEvent(p); (p) = NULL; }


class MXCORE_API CMxMutexLocker
{
public:
	CMxMutexLocker(CMxMutex* mutex)
	{
		this->mutex = mutex;
		mxMutexLock(mutex);
	};
	virtual ~CMxMutexLocker()
	{
		mxMutexUnlock(mutex);
	}

protected:
	CMxMutex* mutex;
};


struct _av_cinfo
{
	int rate, scale;
	int iochannels;
};

typedef struct _av_cinfo avio_cinfo;
typedef struct _av_cinfo sysclk_cinfo;

#endif
