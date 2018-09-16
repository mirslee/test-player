#include "stdafx.h"
#include "MxThread.h"
#include "MxConfig.h"

#include <assert.h>
#include <errno.h>

#include "MxCommon.h"

/*** Global locks ***/

void mxGloablMutex (unsigned n, bool acquire)
{
    static MxMutex locks[] = {
        MX_STATIC_MUTEX,
        MX_STATIC_MUTEX,
        MX_STATIC_MUTEX,
        MX_STATIC_MUTEX,
        MX_STATIC_MUTEX,
#ifdef _WIN32
        MX_STATIC_MUTEX, // For MTA holder
#endif
    };
    static_assert (MX_MAX_MUTEX == (sizeof (locks) / sizeof (locks[0])),
                   "Wrong number of global mutexes");
    assert (n < (sizeof (locks) / sizeof (locks[0])));
    
    MxMutex *lock = locks + n;
    if (acquire)
        mxMutexLock (lock);
    else
        mxMutexUnlock (lock);
}

#if defined (_WIN32) && (_WIN32_WINNT < _WIN32_WINNT_WIN8)
/* Cannot define OS version-dependent stuff in public headers */
# undef MX_NEED_SLEEP
# undef MX_NEED_SEMAPHORE
#endif

#if defined(MX_NEED_SLEEP) || defined(MX_NEED_CONDVAR)
#include "MxAtomic.h"

static void mxCancelAddrPrepare(void *addr)
{
    /* Let thread subsystem on address to broadcast for cancellation */
	mxCancelAddrSet(addr);
	mxCleanupPush(mxCancelAddrClear, addr);
    /* Check if cancellation was pending before vlc_cancel_addr_set() */
	mxTestCancel();
    mxCleanupPop();
}

static void mxCancelAddrFinish(void *addr)
{
	mxCancelAddrClear(addr);
    /* Act on cancellation as potential wake-up source */
	mxTestCancel();
}
#endif

#ifdef MX_NEED_SLEEP
void (mwait)(mtime_t deadline)
{
    mtime_t delay;
    atomic_int value = ATOMIC_VAR_INIT(0);
    
	mxCancelAddrPrepare(&value);
    
    while ((delay = (deadline - mdate())) > 0)
    {
		mxAddrTimedwait(&value, 0, delay);
		mxTestCancel();
    }
    
	mxCancelAddrFinish(&value);
}

void (msleep)(mtime_t delay)
{
    mwait(mdate() + delay);
}
#endif

#ifdef MX_NEED_CONDVAR
#include <stdalign.h>

static inline atomic_uint *mxCondValue(MxCond *cond)
{
    /* XXX: ugly but avoids including vlc_atomic.h in vlc_threads.h */
    static_assert (sizeof (cond->value) <= sizeof (atomic_uint),
                   "Size mismatch!");
    static_assert ((alignof (cond->value) % alignof (atomic_uint)) == 0,
                   "Alignment mismatch");
    return (atomic_uint *)&cond->value;
}

void mxCondInit(MxCond *cond)
{
    /* Initial value is irrelevant but set it for happy debuggers */
    atomic_init(mxCondValue(cond), 0);
}

void mxCondInitDaytime(MxCond *cond)
{
	mxCondInit(cond);
}

void mxCondDestroy(MxCond *cond)
{
    /* Tempting sanity check but actually incorrect:
     assert((atomic_load_explicit(vlc_cond_value(cond),
     memory_order_relaxed) & 1) == 0);
     * Due to timeouts and spurious wake-ups, the futex value can look like
     * there are waiters, even though there are none. */
    (void) cond;
}

void mxCondSignal(MxCond *cond)
{
    /* Probably the best documented approach is that of Bionic: increment
     * the futex here, and simply load the value in cnd_wait(). This has a bug
     * as unlikely as well-known: signals get lost if the futex is incremented
     * an exact multiple of 2^(CHAR_BIT * sizeof (int)) times.
     *
     * A different presumably bug-free solution is used here:
     * - cnd_signal() sets the futex to the equal-or-next odd value, while
     * - cnd_wait() sets the futex to the equal-or-next even value.
     **/
    atomic_fetch_or_explicit(mxCondValue(cond), 1, memory_order_relaxed);
	mxAddrSignal(&cond->value);
}

void mxCondBroadcast(MxCond *cond)
{
    atomic_fetch_or_explicit(mxCondValue(cond), 1, memory_order_relaxed);
	mxAddrSignal(&cond->value);
}

void mxCondWait(MxCond *cond, MxMutex *mutex)
{
    unsigned value = atomic_load_explicit(mxCondValue(cond),
                                          memory_order_relaxed);
    while (value & 1)
    {
        if (atomic_compare_exchange_weak_explicit(mxCondValue(cond), &value,
                                                  value + 1,
                                                  memory_order_relaxed,
                                                  memory_order_relaxed))
            value++;
    }
    
	mxCancelAddrPrepare(&cond->value);
	mxMutexUnlock(mutex);
    
	mxAddrWait(&cond->value, value);
    
	mxMutexLock(mutex);
	mxCancelAddrFinish(&cond->value);
}

static int mxCondWaitDelay(MxCond *cond, MxMutex *mutex,
                               mtime_t delay)
{
    unsigned value = atomic_load_explicit(mxCondValue(cond),
                                          memory_order_relaxed);
    while (value & 1)
    {
        if (atomic_compare_exchange_weak_explicit(mxCondValue(cond), &value,
                                                  value + 1,
                                                  memory_order_relaxed,
                                                  memory_order_relaxed))
            value++;
    }
    
	mxCancelAddrPrepare(&cond->value);
	mxMutexUnlock(mutex);
    
    if (delay > 0)
        value = mxAddrTimedwait(&cond->value, value, delay);
    else
        value = 0;
    
	mxMutexLock(mutex);
	mxCancelAddrFinish(&cond->value);
    
    return value ? 0 : ETIMEDOUT;
}

int mxCondTimedwait(MxCond *cond, MxMutex *mutex, mtime_t deadline)
{
    return mxCondWaitDelay(cond, mutex, deadline - mdate());
}

int mxCondTimedwaitDaytime(MxCond *cond, MxMutex *mutex,
                               time_t deadline)
{
    struct timespec ts;
    
    timespec_get(&ts, TIME_UTC);
    deadline -= ts.tv_sec * CLOCK_FREQ;
    deadline -= ts.tv_nsec / (1000000000 / CLOCK_FREQ);
    
    return mxCondTimedwait(cond, mutex, deadline);
}
#endif

#ifdef MX_NEED_RWLOCK
/*** Generic read/write locks ***/
#include <stdlib.h>
#include <limits.h>
/* NOTE:
 * lock->state is a signed long integer:
 *  - The sign bit is set when the lock is held for writing.
 *  - The other bits code the number of times the lock is held for reading.
 * Consequently:
 *  - The value is negative if and only if the lock is held for writing.
 *  - The value is zero if and only if the lock is not held at all.
 */
#define READER_MASK LONG_MAX
#define WRITER_BIT  LONG_MIN

void mxRWLockInit(MxRWLock *lock)
{
	mxMutexInit(&lock->mutex);
	mxCondInit(&lock->wait);
    lock->state = 0;
}

void mxRWLockDestroy(MxRWLock *lock)
{
	mxCondDestroy(&lock->wait);
	mxMutexDestroy(&lock->mutex);
}

void mxRWLockRdlock(MxRWLock *lock)
{
	mxMutexLock(&lock->mutex);
    /* Recursive read-locking is allowed.
     * Ensure that there is no active writer. */
    while (lock->state < 0)
    {
        assert (lock->state == WRITER_BIT);
        mutex_cleanup_push (&lock->mutex);
		mxCondWait(&lock->wait, &lock->mutex);
		mxCleanupPop();
    }
    if (unlikely(lock->state >= READER_MASK))
        abort (); /* An overflow is certainly a recursion bug. */
    lock->state++;
	mxMutexUnlock(&lock->mutex);
}

void mxRWLockWrlock(MxRWLock *lock)
{
	mxMutexLock(&lock->mutex);
    /* Wait until nobody owns the lock in any way. */
    while (lock->state != 0)
    {
		mxCleanupPush(&lock->mutex);
		mxCondWait(&lock->wait, &lock->mutex);
		mxCleanupPop();
    }
    lock->state = WRITER_BIT;
	mxMutexUnlock(&lock->mutex);
}

void mxRWLockUnlock(MxRWLock *lock)
{
	mxMutexLock(&lock->mutex);
    if (lock->state < 0)
    {   /* Write unlock */
        assert (lock->state == WRITER_BIT);
        /* Let reader and writer compete. OS scheduler decides who wins. */
        lock->state = 0;
		mxCondBroadcast(&lock->wait);
    }
    else
    {   /* Read unlock */
        assert (lock->state > 0);
        /* If there are no readers left, wake up one pending writer. */
        if (--lock->state == 0)
			mxCondSignal(&lock->wait);
    }
	mxMutexUnlock(&lock->mutex);
}
#endif /* LIBVLC_NEED_RWLOCK */

#ifdef MX_NEED_SEMAPHORE
/*** Generic semaphores ***/
#include <limits.h>
#include <errno.h>

void mxSemInit(MxSem *sem, unsigned value)
{
	mxMutexInit(&sem->lock);
	mxCondInit(&sem->wait);
    sem->value = value;
}

void mxSemDestroy(MxSem *sem)
{
	mxCondDestroy(&sem->wait);
	mxMutexDestroy(&sem->lock);
}

int mxSemPost(MxSem *sem)
{
    int ret = 0;
    
	mxMutexLock(&sem->lock);
    if (likely(sem->value != UINT_MAX))
        sem->value++;
    else
        ret = EOVERFLOW;
	mxMutexUnlock(&sem->lock);
	mxCondSignal(&sem->wait);
    
    return ret;
}

void mxSemWait(MxSem *sem)
{
	mxMutexLock(&sem->lock);
    mutex_cleanup_push (&sem->lock);
    while (!sem->value)
		mxCondWait(&sem->wait, &sem->lock);
    sem->value--;
	mxCleanupPop();
	mxMutexUnlock(&sem->lock);
}
#endif /* LIBVLC_NEED_SEMAPHORE */
