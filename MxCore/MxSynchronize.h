
#ifndef MxSynchronize_h
#define MxSynchronize_h

#ifdef _WIN32

#define MxEvent HANDLE
#define mxCreateEvent CreateEvent
#define mxCloseEvent CloseHandle
#define mxSetEvent SetEvent
#define mxResetEvent ResetEvent
#define mxWaitForSingleObject WaitForSingleObject
#define mxWaitForAllObject(c,h,m) WaitForMulitpleObjects(c,h,TRUE,m)

#define MxMutex CRITICAL_SECTION
#define mxMutexInit(v) InitializeCriticalSection(v)
#define mxMutexDetroy(v) DeletecriticalSection(v)
#define mxMutexLock(v) EnterCriticalSection(v)
#define mxMutexTrylock(v) (TryEnterCriticalSection(v)!=0)
#define mxMutexUnlock(v) LeaveCriticalSection(v)

#else /*__APPLE__*/

#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT ETIMEDOUT
#define INFINITE (DWORD)(-1)
typedef void* MxEvent;

MxEvent mxCreateEvent(const pthread_condattr_t *attr, bool bManualReset, bool bInitialState, const char* name);
bool mxCloseEvent(MxEvent event);
bool mxSetEvent(MxEvent event);
bool mResetEvent(MxEvent event);
DWORD mxWaitForSingleObject(MxEvent event, DWORD dwMilliseconds);
DWORD GetTickCount();
void mxSleep(unsinged int millseconds);




#endif

#endif /* MxSynchronize_h */
