#include "stdafx.h"
#include "MxSynchronize.h"
#include <time.h>
#ifdef __APPLE__
#include <sys/time.h>
#endif

#ifdef __APPLE__

#include <dispatch/dispatch.h>
#include <libkern/osatomic.h>
#include <assert.h>

class MxMutex
{
public:
    MxMutex();
    ~MxMutex();
    void lock();
    bool trylock();
    void unlock();
private:
    int32_t count;
    dispatch_semaphore_t sema;
    pthread_t ownerthread;
    int32_t recursive_count;
};

MxMutex::MxMutex()
: count(0), ownerthread(nullptr), recursive_count(0)
{
    assert(sema = dispatch_semaphore_create(0));
}

MxMutex::~MxMutex()
{
    dispatch_release(sema);
}

void MxMutex::lock()
{
    if (pthread_equal(ownerthread, pthread_self()))
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

bool MxMutex::trylock()
{
    if (OSAtomicCompareAndSwap32Barrier(0, 1, &count))
    {
        ownerthread = pthread_self();
        recursive_count = 1;
        return true;
    }
    else if (pthread_equal(ownerthread, pthread_self()))
    {
        recursive_count++;
        return true;
    }
    else
    {
        return false;
    }
}

void MxMutex::unlock()
{
    if (pthread_equal(ownerthread, pthread_self()))
    {
        recursive_count--;
        if (recursive_count == 0)
        {
            ownerthread = NULL;
            if (OSAtomicDecrement32Barrier(&count) > 0) // if (--count > 0)
            {
                dispatch_semaphore_signal(sema); // release a waiting thread
            }
        }
    }
}

struct MxMutex_t {
    MxMutex mutex;
};

void mxMutexInit(CMxMutex* mtx) {
    *mtx = (CMxMutex)new MxMutex_t();
}
void mxMutexDestroy(CMxMutex* mtx) {
    delete *mtx;
    *mtx = nullptr;
}
void mxMutexLock(CMxMutex* mtx) {
    if (mtx && *mtx) {
        ((MxMutex_t*)(*mtx))->mutex.lock();
    }
}
bool mxMutexTrylock(CMxMutex* mtx) {
    if (mtx && *mtx) {
        return ((MxMutex_t*)(*mtx))->mutex.trylock();
    } else {
        return false;
    }
}
void mxMutexUnlock(CMxMutex* mtx) {
    if (mtx && *mtx) {
        ((MxMutex_t*)(*mtx))->mutex.unlock();
    }
}

struct MxEvent {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    bool bManualReset;
    bool bInitialState;
};

CMxEvent mxCreateEvent(void* lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName) {
    MxEvent *mxevent = new MxEvent;
    int ret = pthread_cond_init(&mxevent->cond, (pthread_condattr_t*)lpEventAttributes);
    ret = pthread_mutex_init(&mxevent->mutex, nullptr);
    mxevent->bInitialState = bInitialState;
    mxevent->bManualReset = bManualReset;
    return (CMxEvent)mxevent;
}
void mxCloseEvent(CMxEvent event) {
    if (!event) return;
    MxEvent *mxevent = (MxEvent*)event;
    pthread_mutex_destroy(&mxevent->mutex);
    pthread_cond_destroy(&mxevent->cond);
    delete mxevent;
    mxevent = nullptr;
}
bool mxSetEvent(CMxEvent event) {
    if (!event) { return false; }
    MxEvent *mxevent = (MxEvent*)event;
    pthread_mutex_lock(&mxevent->mutex);
    if (mxevent->bInitialState){
        pthread_mutex_unlock(&mxevent->mutex);
        return true;
    }
    
    bool ret = false;
    if (mxevent->bManualReset) {
        ret = pthread_cond_broadcast(&mxevent->cond) == 0;
    }
    else {
        ret = pthread_cond_signal(&mxevent->cond) == 0;
    }
    if (ret) {
        mxevent->bInitialState = true;
    }
    pthread_mutex_unlock(&mxevent->mutex);
    return ret;
}
bool mxResetEvent(CMxEvent event) {
    if (!event) { return false; }
    MxEvent *mxevent = (MxEvent*)event;
    pthread_mutex_lock(&mxevent->mutex);
    mxevent->bInitialState = false;
    pthread_mutex_unlock(&mxevent->mutex);
    return true;
}
unsigned long mxWaitEvent(CMxEvent event, unsigned long dwMilliseconds) {
    if (!event) { return -1; }
    MxEvent *mxevent = (MxEvent*)event;
    pthread_mutex_lock(&mxevent->mutex);
    if (mxevent->bInitialState){
        if (!mxevent->bManualReset){
            mxevent->bInitialState = false;
        }
        pthread_mutex_unlock(&mxevent->mutex);
        return WAIT_OK;
    }
    if (WAIT_INFINITE == dwMilliseconds) {
        while (!mxevent->bInitialState) {
            int ret = pthread_cond_wait(&mxevent->cond, &mxevent->mutex);
            if (0 != ret) {
                pthread_mutex_unlock(&mxevent->mutex);
                return ret;
            }
        }
        if (!mxevent->bManualReset) {
            mxevent->bInitialState = false;
        }
        pthread_mutex_unlock(&mxevent->mutex);
        return WAIT_OK;
    } else if (0 == dwMilliseconds) {
        if (mxevent->bInitialState) {
            if (!mxevent->bManualReset) {
                mxevent->bInitialState = false;
            }
            pthread_mutex_unlock(&mxevent->mutex);
            return WAIT_OK;
        }
        else {
            pthread_mutex_unlock(&mxevent->mutex);
            return WAIT_TIMEOUT;
        }
    }
    
    dwMilliseconds *= 1000;
    timespec abstime;
    
#ifdef _WIN32
    LARGE_INTEGER now;
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);
    abstime.tv_sec = now.QuadPart / freq.QuadPart;
    abstime.tv_nsec = now.QuadPart - abstime.tv_sec*freq.QuadPart;
#else
    timeval now;
    gettimeofday(&now, 0);
    abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
    abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
#endif
    
    while (!mxevent->bInitialState) {
        int ret = pthread_cond_timedwait(&mxevent->cond, &mxevent->mutex, &abstime);
        if (0 != ret) {
            pthread_mutex_unlock(&mxevent->mutex);
            return ret;
        }
    }
    if (!mxevent->bManualReset) {
        mxevent->bInitialState = false;
    }
    pthread_mutex_unlock(&mxevent->mutex);
    return WAIT_OK;
}
unsigned long mxWaitEvents(unsigned long  nCount, CMxEvent* lpEvents, unsigned long  dwMilliseconds) {
    if (!lpEvents) { return -1; }
    if (WAIT_INFINITE == dwMilliseconds) {
        for (unsigned long i = 0; i < nCount; i++) {
            MxEvent *mxevent = (MxEvent*)lpEvents[i];
            if (!mxevent) { continue; }
            pthread_mutex_lock(&mxevent->mutex);
            if (mxevent->bInitialState) {
                if (!mxevent->bManualReset) {
                    mxevent->bInitialState = false;
                }
                pthread_mutex_unlock(&mxevent->mutex);
                continue;
            }
            int ret = 0;
            while (!mxevent->bInitialState) {
                ret = pthread_cond_wait(&mxevent->cond, &mxevent->mutex);
                if (0 != ret) {
                    pthread_mutex_unlock(&mxevent->mutex);
                    return ret;
                }
            }
            if (!mxevent->bManualReset) {
                mxevent->bInitialState = false;
            }
            pthread_mutex_unlock(&mxevent->mutex);
        }
    } else if (0 == dwMilliseconds) {
        for (unsigned long i = 0; i < nCount; i++) {
            MxEvent *mxevent = (MxEvent*)lpEvents[i];
            if (!mxevent) { continue; }
            pthread_mutex_lock(&mxevent->mutex);
            if (mxevent->bInitialState) {
                if (!mxevent->bManualReset) {
                    mxevent->bInitialState = false;
                }
                pthread_mutex_unlock(&mxevent->mutex);
                continue;
            } else {
                pthread_mutex_unlock(&mxevent->mutex);
                return WAIT_TIMEOUT;
            }
        }
    }
    else {
        dwMilliseconds *= 1000;
        timespec abstime;
#ifdef _WIN32
        LARGE_INTEGER now;
        LARGE_INTEGER freq;
        QueryPerformanceCounter(&now);
        QueryPerformanceFrequency(&freq);
        abstime.tv_sec = now.QuadPart / freq.QuadPart;
        abstime.tv_nsec = now.QuadPart - abstime.tv_sec*freq.QuadPart;
#else
        timeval now;
        gettimeofday(&now, 0);
        abstime.tv_sec = now.tv_sec + (now.tv_usec + dwMilliseconds) / 1000000;
        abstime.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000000) * 1000;
#endif
        for (unsigned long i = 0; i < nCount; i++) {
            MxEvent *mxevent = (MxEvent*)lpEvents[i];
            if (!mxevent) {
                continue;
            }
            pthread_mutex_lock(&mxevent->mutex);
            while (!mxevent->bInitialState) {
                int ret = pthread_cond_timedwait(&mxevent->cond, &mxevent->mutex, &abstime);
                if (0 != ret) {
                    pthread_mutex_unlock(&mxevent->mutex);
                    return ret;
                }
            }
            if (!mxevent->bManualReset) {
                mxevent->bInitialState = false;
            }
            pthread_mutex_unlock(&mxevent->mutex);
        }
    }
    return WAIT_OK;
}

#endif


