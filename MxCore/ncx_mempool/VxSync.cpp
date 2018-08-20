#include "stdafx.h"
#include "vxsync.h"
#include "vxtempl.h"
#ifndef _WIN32
#include <sys/time.h>
#endif

#pragma warning(disable:4996)

#ifndef WIN32

struct VXEVENTS
{
pthread_cond_t cond;
pthread_mutex_t mutex;
VXBOOL bManualReset;
VXBOOL signal;
};

VX_EXT_API VXEVENT vxCreateEvent(const pthread_condattr_t * attr, VXBOOL bManualReset, VXBOOL bInitialState, const char* lpName)
{
	VXEVENTS* vxe = new VXEVENTS;
	memset(vxe, 0, sizeof(VXEVENTS));
	pthread_cond_init(&vxe->cond, attr);
	pthread_mutex_init(&vxe->mutex, NULL);
	vxe->signal = bInitialState;
	vxe->bManualReset = bManualReset;
	return (VXEVENT)vxe;
}

VX_EXT_API VXBOOL vxCloseEvent(VXEVENT vxevent)
{
	if(!vxevent)
	{
		return FALSE;
	}
	VXEVENTS* vxe = (VXEVENTS*)vxevent;
	pthread_mutex_destroy(&vxe->mutex);
	pthread_cond_destroy(&vxe->cond);
	delete vxe;
	return TRUE;
}

VX_EXT_API VXBOOL vxSetEvent(VXEVENT vxevent)
{
	VXEVENTS* vxe = (VXEVENTS*)vxevent;
	pthread_mutex_lock(&vxe->mutex);
	if(vxe->bManualReset && vxe->signal)
	{
		pthread_mutex_unlock(&vxe->mutex);
		return TRUE;
	}
	VXBOOL ret = pthread_cond_broadcast(&vxe->cond) == 0;
	if(ret)
	{
		vxe->signal = TRUE;
	}
	pthread_mutex_unlock(&vxe->mutex);
	return ret;
}

VX_EXT_API VXBOOL vxResetEvent(VXEVENT vxevent)
{
	if(!vxevent)
	{
		return FALSE;
	}
	VXEVENTS* vxe = (VXEVENTS*)vxevent;
	pthread_mutex_lock(&vxe->mutex);
	vxe->signal = FALSE;
	pthread_mutex_unlock(&vxe->mutex);
	return TRUE;
}

VX_EXT_API DWORD vxWaitForSingleObject(VXEVENT vxevent, DWORD dwMilliseconds)
{
	VXEVENTS* vxe = (VXEVENTS*)vxevent;
    pthread_mutex_lock(&vxe->mutex);
	{
		if(vxe->signal)
		{
			if(!vxe->bManualReset)
			{
				vxe->signal = FALSE;
			}
			pthread_mutex_unlock(&vxe->mutex);
			return WAIT_OBJECT_0;
		}
	}
	if(dwMilliseconds == INFINITE)
	{
		while(!vxe->signal)
		{
			int ret = pthread_cond_wait(&vxe->cond, &vxe->mutex);
			if(ret != 0)
			{	
				pthread_mutex_unlock(&vxe->mutex);
				return ret;
			}
		}
		if(!vxe->bManualReset)
		{
			vxe->signal = FALSE;
		}
		pthread_mutex_unlock(&vxe->mutex);
		return WAIT_OBJECT_0;
	}
	else if(dwMilliseconds == 0)
	{
		if(vxe->signal)
		{
			if(!vxe->bManualReset)
			{
				vxe->signal = FALSE;
			}
			pthread_mutex_unlock(&vxe->mutex);
			return WAIT_OBJECT_0;
		}
		else
		{
			pthread_mutex_unlock(&vxe->mutex);
			return WAIT_TIMEOUT;
		}
	}

	dwMilliseconds *= 1000;
	
	struct timespec abstime;
	struct timeval now;
	gettimeofday(&now, 0);
	abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
	abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
	while(!vxe->signal)
	{
		int ret = pthread_cond_timedwait(&vxe->cond, &vxe->mutex, &abstime);
		if(ret != 0)
		{	
			pthread_mutex_unlock(&vxe->mutex);
			return ret;
		}
	}
	if(!vxe->bManualReset)
	{
		vxe->signal = FALSE;
	}
	pthread_mutex_unlock(&vxe->mutex);

	return WAIT_OBJECT_0;
}

VX_EXT_API DWORD vxWaitForAllObjects(DWORD nCount, VXEVENT *lpHandles, DWORD dwMilliseconds)
{
	if(dwMilliseconds == INFINITE)
	{
		for(DWORD i = 0; i < nCount; i++)
		{
			VXEVENTS* vxe = (VXEVENTS*)lpHandles[i];
			if (!vxe)
			{
				continue;
			}
			pthread_mutex_lock(&vxe->mutex);
			if(vxe->signal) 
			{
				if(!vxe->bManualReset)
				{
					vxe->signal = FALSE;
				}
				pthread_mutex_unlock(&vxe->mutex);
				continue;
			}
			int ret = 0;
			while(!vxe->signal)
			{
				ret = pthread_cond_wait(&vxe->cond, &vxe->mutex);
				if(ret != 0)
				{
					pthread_mutex_unlock(&vxe->mutex);
					return ret;
				}
			}
			if(!vxe->bManualReset)
			{
				vxe->signal = FALSE;
			}
			pthread_mutex_unlock(&vxe->mutex);
		}
	}
	else
	{
		dwMilliseconds *= 1000;

		struct timespec abstime;
		struct timeval now;
		gettimeofday(&now, 0);
		abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
		abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
		
		for(DWORD i = 0; i < nCount; i++)
		{
			VXEVENTS* vxe = (VXEVENTS*)lpHandles[i];
			if(!vxe)
			{
				continue;
			}
			pthread_mutex_lock(&vxe->mutex);
			if(dwMilliseconds == 0)
			{
				if(!vxe->signal)
				{
					pthread_mutex_unlock(&vxe->mutex);
					return WAIT_TIMEOUT;
				}
				if(!vxe->bManualReset)
				{
					vxe->signal = FALSE;
				}
				pthread_mutex_unlock(&vxe->mutex);
				continue;
			}

			while(!vxe->signal)
			{
				int ret = pthread_cond_timedwait(&vxe->cond, &vxe->mutex, &abstime);
				if(ret != 0) 
				{
					pthread_mutex_unlock(&vxe->mutex);
					return ret;
				}
			}
			if(!vxe->bManualReset)
			{
				vxe->signal = FALSE;
			}
			pthread_mutex_unlock(&vxe->mutex);
		}
	}
	return 0;
}

VX_EXT_API DWORD GetTickCount()
{
#ifdef __APPLE__
	LONG upt = AbsoluteToDuration(UpTime());
	if(upt < 0)
	{
		upt = (-upt) / 1000;
	}
	return upt;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);                      //此处可以判断一下返回值
    return (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000));
#endif
}

VX_EXT_API void vxSleep(ULONG milliseconds)	
{
#ifdef __APPLE__
	AbsoluteTime wakeup = AddDurationToAbsolute(milliseconds, UpTime());
	MPDelayUntil(&wakeup);
#else
    struct timeval delay;
    delay.tv_sec = milliseconds / 1000;
    delay.tv_usec = (milliseconds % 1000) * 1000; 
    select(0, NULL, NULL, NULL, &delay);
#endif
}

#include "dlfcn.h"
VX_EXT_API HMODULE vxLoadLibrary(const char* dylib)
{
    HMODULE dl = dlopen(dylib, RTLD_LAZY);
    if(!dl)
    {
        char* dlstr = dlerror();
        printf("%s\n", dlstr);
//        free(dlstr);
    }
    return dl;
}

VX_EXT_API void* vxGetProcAddress(HMODULE hdylib, const char* psz_function)
{
    void* addr = dlsym(hdylib, psz_function);
    if(!addr)
    {
        printf("%s\n", dlerror());
    }
    return addr;
}

VX_EXT_API void vxFreeLibrary(HMODULE hdylib)
{
	dlclose(hdylib);
}

VX_EXT_API void vxPThreadMutexInit(pthread_mutex_t* mutex)
{
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
}

#endif