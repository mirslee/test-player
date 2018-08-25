
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
#define INFINITE (unsigned long)(-1)
typedef void* MxEvent;

struct MxEvent_t {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    bool bManualReset;
    bool signal;
};

MxEvent mxCreateEvent(pthread_condattr_t *attr, bool bManualReset, bool bInitialState, const char* name);
bool mxCloseEvent(MxEvent event);
bool mxSetEvent(MxEvent event);
bool mxResetEvent(MxEvent event);
unsigned long mxWaitObject(MxEvent event, unsigned long dwMilliseconds);
unsigned long mxWaitObjects(unsigned long  nCount, MxEvent event, unsigned long  dwMilliseconds);
unsigned long GetTickCount();
void mxSleep(unsigned int millseconds);




#endif

#endif /* MxSynchronize_h */
