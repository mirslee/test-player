// write by bay at 2008-09-02
#ifndef __BAY_VXSYNC_H__
#define __BAY_VXSYNC_H__

#include "vxconfig.h"

#ifdef WIN32

#define VXEVENT HANDLE
#define vxCreateEvent CreateEvent
#define vxCloseEvent CloseHandle
#define vxSetEvent SetEvent
#define vxResetEvent ResetEvent
#define vxWaitForSingleObject WaitForSingleObject
#define vxWaitForAllObjects(c, h, m) WaitForMultipleObjects(c, h, TRUE, m)

#define VXTHREAD_MUTEX CRITICAL_SECTION
#define VXTHREAD_MUTEX_INIT(v) InitializeCriticalSection(v)
#define VXTHREAD_MUTEX_DESTROY(v) DeleteCriticalSection(v)
#define VXTHREAD_MUTEX_LOCK(v) EnterCriticalSection(v)
#define VXTHREAD_MUTEX_TRYLOCK(v) (TryEnterCriticalSection(v) != 0)
#define VXTHREAD_MUTEX_UNLOCK(v) LeaveCriticalSection(v)
/*
typedef void* VXEVENT;
VX_EXT_API VXEVENT vxCreateEvent( const pthread_condattr_t * attr,VXBOOL bManualReset,VXBOOL bInitialState,const char* lpName);
VX_EXT_API VXBOOL vxCloseEvent(VXEVENT vxevent);
VX_EXT_API VXBOOL vxSetEvent(VXEVENT vxevent);
VX_EXT_API VXBOOL vxResetEvent(VXEVENT vxevent);
VX_EXT_API DWORD vxWaitForSingleObject(VXEVENT hHandle, DWORD dwMilliseconds);
VX_EXT_API DWORD vxWaitForAllObjects(DWORD nCount,VXEVENT *lpHandles,DWORD dwMilliseconds);

VX_EXT_API void vxPThreadMutexInit(pthread_mutex_t* mutex);

#define VXTHREAD_MUTEX pthread_mutex_t
//#define VXHREAD_MUTEX_INIT(v) *(v) = PTHREAD_MUTEX_INITIALIZER;
#define VXHREAD_MUTEX_INIT(v) vxPThreadMutexInit(v)
#define VXHREAD_MUTEX_DESTROY(v) pthread_mutex_destroy(v)
#define VXHREAD_MUTEX_LOCK(v) pthread_mutex_lock(v)
#define VXHREAD_MUTEX_TRYLOCK(v) (pthread_mutex_trylock(v)==0)
#define VXHREAD_MUTEX_UNLOCK(v) pthread_mutex_unlock(v)
*/
#define VXSEM_T HANDLE
#define VXSEM_INIT(v, v1, v2) (*v = CreateSemaphore(NULL, v1, v2, NULL))
#define VXSEM_DESTROY(v) CloseHandle(v)
#define VXSEM_WAIT(v) WaitForSingleObject(v, INFINITE)
#define VXSEM_WAITTIMEOUT(v, time) WaitForSingleObject(v, time)
#define VXSEM_POST(v) ReleaseSemaphore(v, 1, NULL)

#else

#ifdef __APPLE__
#include "_types.h"
#endif

#define WAIT_OBJECT_0		0
#define WAIT_TIMEOUT		ETIMEDOUT
#define INFINITE			(DWORD)(-1)
typedef void* VXEVENT;

VX_EXT_API VXEVENT vxCreateEvent(const pthread_condattr_t * attr, VXBOOL bManualReset, VXBOOL bInitialState, const char* lpName);
VX_EXT_API VXBOOL vxCloseEvent(VXEVENT vxevent);
VX_EXT_API VXBOOL vxSetEvent(VXEVENT vxevent);
VX_EXT_API VXBOOL vxResetEvent(VXEVENT vxevent);
VX_EXT_API DWORD vxWaitForSingleObject(VXEVENT hHandle, DWORD dwMilliseconds);
VX_EXT_API DWORD vxWaitForAllObjects(DWORD nCount, VXEVENT *lpHandles, DWORD dwMilliseconds);

VX_EXT_API DWORD GetTickCount();
VX_EXT_API void vxSleep(ULONG milliseconds);

VX_EXT_API HMODULE vxLoadLibrary(const char* dylib);
VX_EXT_API void* vxGetProcAddress(HMODULE hdylib, const char* psz_function);
VX_EXT_API void vxFreeLibrary(HMODULE hdylib);

#include <semaphore.h>

#ifdef __APPLE__

#include <dispatch/dispatch.h>
#include <libkern/osatomic.h>

class mutex2
{
public:
    mutex2() : count(0), ownerthread(NULL), recursive_count(0) {VERIFY(sema = dispatch_semaphore_create(0));} // initial count is 0
    ~mutex2() {dispatch_release(sema);}
    
    void lock()
    {
        if(pthread_equal(ownerthread, pthread_self()))
		{
            recursive_count++;
		}
        else
        {
            for (unsigned spins = 0; spins != 5000; ++spins)
            {
                if (OSAtomicCompareAndSwap32Barrier(0, 1, &count))
                {
                    ownerthread = pthread_self();
                    recursive_count = 1;
                    return;
                }
                sched_yield();
            }

            // DISPATCH_TIME_FOREVER is unsigned long long (not in ISO C++98/03).
            // Define our own equivalent.
            const uint64_t DISPATCH_TIME_FOREVER_u64 = ~uint64_t(0);

            if (OSAtomicIncrement32Barrier(&count) > 1) // if (++count > 1)
			{
				dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER_u64);
			}
            
            ownerthread = pthread_self();
            recursive_count = 1;
        }
    }

    bool trylock()
    {
		if(OSAtomicCompareAndSwap32Barrier(0, 1, &count))
		{
			ownerthread = pthread_self();
			recursive_count = 1;
			return true;
		}
		else if(pthread_equal(ownerthread, pthread_self()))
		{
			recursive_count++;
			return true;
		}
		else DriverServices.h
		{
			return false;
		}
    }
    
    void unlock()
    {
        if(pthread_equal(ownerthread, pthread_self()))
        {
            recursive_count--;
            if(recursive_count == 0)
            {
                ownerthread = NULL;
                if (OSAtomicDecrement32Barrier(&count) > 0) // if (--count > 0)
				{
                    dispatch_semaphore_signal(sema); // release a waiting thread
				}
            }
        }
    }
    
private:
    int32_t count;
    dispatch_semaphore_t sema;
    pthread_t ownerthread;
    int32_t recursive_count;
};

#else

class mutex2
{
public:
    mutex2() : count(0), ownerthread(NULL), recursive_count(0) {VERIFY(sem_init(&sema, 0, 0) == 0);} // initial count is 0
    ~mutex2() {VERIFY(sem_destroy(&sema) == 0);}
    
    void lock()
    {
        if(pthread_equal(ownerthread, pthread_self()))
		{
            recursive_count++;
		}
        else
        {
            for (unsigned spins = 0; spins != 5000; ++spins)
            {
                if (__sync_bool_compare_and_swap(&count, 0, 1))
                {
                    ownerthread = pthread_self();
                    recursive_count = 1;
                    return;
                }
                sched_yield();
            }
            
            if (__sync_fetch_and_add(&count, 1) > 0) // if (++count > 1)
			{
                sem_wait(&sema); // wait for unlock
			}
            ownerthread = pthread_self();
            recursive_count = 1;
        }
    }

    bool trylock()
    {
		if(__sync_bool_compare_and_swap(&count, 0, 1))
		{
			ownerthread = pthread_self();
			recursive_count = 1;
			return true;
		}
		else if(pthread_equal(ownerthread, pthread_self()))
		{
			recursive_count++;
			return true;
		}
		else
		{
			return false;
		}
    }
    
    void unlock()
    {
        if(pthread_equal(ownerthread, pthread_self()))
        {
            recursive_count--;
            if(recursive_count == 0)
            {
                ownerthread = NULL;
                if (__sync_fetch_and_sub(&count, 1) > 1) // if (--count > 0)
				{
                    sem_post(&sema); // release a waiting thread
				}
            }
        }
    }
private:
    int32_t count;
    sem_t sema;
    pthread_t ownerthread;
    int32_t recursive_count;
};
#endif

typedef mutex2* mutex2_t;

#define VXTHREAD_MUTEX mutex2_t
#define VXTHREAD_MUTEX_INIT(v)  *v = new mutex2
#define VXTHREAD_MUTEX_DESTROY(v) delete *v
#define VXTHREAD_MUTEX_LOCK(v) (*v)->lock()
#define VXTHREAD_MUTEX_TRYLOCK(v) (*v)->trylock()
#define VXTHREAD_MUTEX_UNLOCK(v) (*v)->unlock()

#ifdef __APPLE__
#define VXSEM_T dispatch_semaphore_t
#define VXSEM_INIT(v, v1, v2) (*v) = dispatch_semaphore_create(v1)
#define VXSEM_DESTROY(v) dispatch_release(v)
#define VXSEM_WAIT(v) dispatch_semaphore_wait(v, DISPATCH_TIME_FOREVER)
#define VXSEM_WAITTIMEOUT(v, time) dispatch_semaphore_wait(v, dispatch_time(DISPATCH_TIME_NOW, time * NSEC_PER_MSEC))
#define VXSEM_POST(v) dispatch_semaphore_signal(v)
#else
#define VXSEM_T sem_t
#define VXSEM_INIT(v, v1, v2) sem_init(v, 0, v1);
#define VXSEM_DESTROY(v) sem_destroy(&v)
#define VXSEM_WAIT(v) sem_wait(&v)
#define VXSEM_WAITTIMEOUT(v, time) sem_wait(&v)
#define VXSEM_POST(v) sem_post(&v)
#endif

#endif//!WIN32

VX_EXT_API int	vxGetCPUs();

#define CRIT_PLAYRESOURCE   0			//	�����ؼ�����Դ��ȡ����
#define CRIT_ALLOC			1			//	�ڴ����
#define CRIT_DOWNLOAD		2			//	�����ؼ���Դ���ص���

VX_EXT_API void vxAppLock(int nLockType);
VX_EXT_API void vxAppUnlock(int nLockType);

#define	VXCLOSEEVENT(p)	if((p) != NULL) {vxCloseEvent(p); (p) = NULL;}

typedef void (*sigfunc)(int signo);
VX_EXT_API void vxSigroutine(int signo);
VX_EXT_API void vxSetSigroutine(sigfunc func, double secperf);

typedef void *(*TPRUNFUNC)(void *);//�ص���������
vxinterface IVxThreadPool : public IVxObject
{
	virtual VXBOOL AddTask(TPRUNFUNC run, void *args, VXEVENT finish) = 0;
};
VX_EXT_API VXRESULT vxGetThreadPool(int _maxThreads, IVxThreadPool** tpool);

typedef struct ncx_slab_pool_t* ncx_slab_pool;

typedef struct {
	size_t 			pool_size, used_size, used_pct;
	size_t			pages, free_page;
	size_t			p_small, p_exact, p_big, p_page; /* ����slabռ�õ�page�� */
	size_t			b_small, b_exact, b_big, b_page; /* ����slabռ�õ�byte�� */
	size_t			max_free_pages;					 /* ������������page�� */
} ncx_slab_stat_t;

VX_EXT_API ncx_slab_pool ncx_create_pool(int initsize = 0x1000000);
VX_EXT_API void ncx_destroy_pool(ncx_slab_pool pool);

VX_EXT_API void *ncx_slab_alloc(ncx_slab_pool pool, size_t size);
VX_EXT_API void ncx_slab_free(ncx_slab_pool pool, void *p);
VX_EXT_API void ncx_slab_stat(ncx_slab_pool pool, ncx_slab_stat_t *stat);

VX_EXT_API void *slab_alloc(size_t size);
VX_EXT_API void slab_free(void *p);
VX_EXT_API void slab_stat(ncx_slab_stat_t *stat);

#endif//__BAY_VXSYNC_H__
