#include "stdafx.h"
#include "MxThread.h"
#include "MxConfig.h"

#include "MxCommon.h"
#include "MxError.h"

#include "MxAtomic.h"


#include <signal.h>
#include <errno.h>
#include <assert.h>

#include <pthread.h>
#include <mach/mach_init.h> /* mach_task_self in semaphores */
#include <mach/mach_time.h>
#include <execinfo.h>

static mach_timebase_info_data_t mx_clock_conversion_factor;

static void mx_clock_setup_once (void)
{
    if (unlikely(mach_timebase_info (&mx_clock_conversion_factor) != 0))
        abort ();
}

static pthread_once_t mx_clock_once = PTHREAD_ONCE_INIT;

#define mx_clock_setup() \
pthread_once(&mx_clock_once, mx_clock_setup_once)

static struct timespec mtime_to_ts (mtime_t date)
{
    lldiv_t d = lldiv (date, CLOCK_FREQ);
    struct timespec ts = { d.quot, d.rem * (1000000000 / CLOCK_FREQ) };
    
    return ts;
}

/* Print a backtrace to the standard error for debugging purpose. */
void mxTrace (const char *fn, const char *file, unsigned line)
{
    fprintf (stderr, "at %s:%u in %s\n", file, line, fn);
    fflush (stderr); /* needed before switch to low-level I/O */
    void *stack[20];
    int len = backtrace (stack, sizeof (stack) / sizeof (stack[0]));
    backtrace_symbols_fd (stack, len, 2);
    fsync (2);
}

#ifndef NDEBUG
/* Reports a fatal error from the threading layer, for debugging purposes. */
static void
mx_thread_fatal (const char *action, int error,
                  const char *function, const char *file, unsigned line)
{
    int canc = mxSaveCancel ();
    fprintf (stderr, "LibVLC fatal error %s (%d) in thread %lu ",
             action, error, mxThreadId ());
    mxTrace (function, file, line);
    
    char buf[1000];
    const char *msg;
    
    switch (strerror_r (error, buf, sizeof (buf)))
    {
        case 0:
            msg = buf;
            break;
        case ERANGE: /* should never happen */
            msg = "unknown (too big to display)";
            break;
        default:
            msg = "unknown (invalid error number)";
            break;
    }
    fprintf (stderr, " Error message: %s\n", msg);
    fflush (stderr);
    
    mxRestoreCancel (canc);
    abort ();
}

# define MX_THREAD_ASSERT( action ) \
if (unlikely(val)) \
mx_thread_fatal (action, val, __func__, __FILE__, __LINE__)
#else
# define MX_THREAD_ASSERT( action ) ((void)val)
#endif

/* Initializes a fast mutex. */
void mxMutexInit( MxMutex *p_mutex )
{
    pthread_mutexattr_t attr;
    
    if (unlikely(pthread_mutexattr_init (&attr)))
        abort();
#ifdef NDEBUG
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_DEFAULT);
#else
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK);
#endif
    if (unlikely(pthread_mutex_init(p_mutex, &attr)))
        abort();
    pthread_mutexattr_destroy( &attr );
}

void mxMutexInitRecursive( MxMutex *p_mutex )
{
    pthread_mutexattr_t attr;
    
    if (unlikely(pthread_mutexattr_init (&attr)))
        abort();
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
    if (unlikely(pthread_mutex_init(p_mutex, &attr)))
        abort();
    pthread_mutexattr_destroy( &attr );
}


void mxMutexDestroy (MxMutex *p_mutex)
{
    int val = pthread_mutex_destroy( p_mutex );
    MX_THREAD_ASSERT ("destroying mutex");
}

#ifndef NDEBUG
# ifdef HAVE_VALGRIND_VALGRIND_H
#  include <valgrind/valgrind.h>
# else
#  define RUNNING_ON_VALGRIND (0)
# endif

void mxAssertLocked (MxMutex *p_mutex)
{
    if (RUNNING_ON_VALGRIND > 0)
        return;
    assert (pthread_mutex_lock (p_mutex) == EDEADLK);
}
#endif

void mxMutexLock (MxMutex *p_mutex)
{
    int val = pthread_mutex_lock( p_mutex );
    MX_THREAD_ASSERT ("locking mutex");
}

int mxMutexTrylock (MxMutex *p_mutex)
{
    int val = pthread_mutex_trylock( p_mutex );
    
    if (val != EBUSY)
        MX_THREAD_ASSERT ("locking mutex");
    return val;
}

void mxMutexUnlock (MxMutex *p_mutex)
{
    int val = pthread_mutex_unlock( p_mutex );
    MX_THREAD_ASSERT ("unlocking mutex");
}

void mxCondInit (MxCond *p_condvar)
{
    if (unlikely(pthread_cond_init (p_condvar, NULL)))
        abort ();
}

void mxCondInitDaytime (MxCond *p_condvar)
{
    if (unlikely(pthread_cond_init (p_condvar, NULL)))
        abort ();
}

void mxCondDestroy (MxCond *p_condvar)
{
    int val = pthread_cond_destroy (p_condvar);
    
    /* due to a faulty pthread implementation within Darwin 11 and
     * later condition variables cannot be destroyed without
     * terminating the application immediately.
     * This Darwin kernel issue is still present in version 13
     * and might not be resolved prior to Darwin 15.
     * radar://12496249
     *
     * To work-around this, we are just leaking the condition variable
     * which is acceptable due to VLC's low number of created variables
     * and its usually limited runtime.
     * Ideally, we should implement a re-useable pool.
     */
    if (val != 0) {
#ifndef NDEBUG
        printf("pthread_cond_destroy returned %i\n", val);
#endif
        
        if (val == EBUSY)
            return;
    }
    
    MX_THREAD_ASSERT ("destroying condition");
}

void mxCondSignal (MxCond *p_condvar)
{
    int val = pthread_cond_signal (p_condvar);
    MX_THREAD_ASSERT ("signaling condition variable");
}

void mxCondBroadcast (MxCond *p_condvar)
{
    pthread_cond_broadcast (p_condvar);
}

void mxCondWait (MxCond *p_condvar, MxMutex *p_mutex)
{
    int val = pthread_cond_wait (p_condvar, p_mutex);
    MX_THREAD_ASSERT ("waiting on condition");
}

int mxCondTimedwait (MxCond *p_condvar, MxMutex *p_mutex,
                        mtime_t deadline)
{
    /* according to POSIX standards, cond_timedwait should be a cancellation point
     * Of course, Darwin does not care */
    pthread_testcancel();
    
    /*
     * mdate() is the monotonic clock, pthread_cond_timedwait expects
     * origin of gettimeofday(). Use timedwait_relative_np() instead.
     */
    mtime_t base = mdate();
    deadline -= base;
    if (deadline < 0)
        deadline = 0;
    
    struct timespec ts = mtime_to_ts(deadline);
    int val = pthread_cond_timedwait_relative_np(p_condvar, p_mutex, &ts);
    if (val != ETIMEDOUT)
        MX_THREAD_ASSERT ("timed-waiting on condition");
    return val;
}

/* variant for vlc_cond_init_daytime */
int mxCondTimedwaitDaytime (MxCond *p_condvar, MxMutex *p_mutex,
                                time_t deadline)
{
    /*
     * Note that both pthread_cond_timedwait_relative_np and pthread_cond_timedwait
     * convert the given timeout to a mach absolute deadline, with system startup
     * as the time origin. There is no way you can change this behaviour.
     *
     * For more details, see: https://devforums.apple.com/message/931605
     */
    
    pthread_testcancel();
    
    /*
     * FIXME: It is assumed, that in this case the system waits until the real
     * time deadline is passed, even if the real time is adjusted in between.
     * This is not fulfilled, as described above.
     */
    struct timespec ts = mtime_to_ts(deadline);
    int val = pthread_cond_timedwait(p_condvar, p_mutex, &ts);
    
    if (val != ETIMEDOUT)
        MX_THREAD_ASSERT ("timed-waiting on condition");
    return val;
}


/* Initialize a semaphore. */
void mxSemInit (MxSem *sem, unsigned value)
{
    if (unlikely(semaphore_create(mach_task_self(), sem, SYNC_POLICY_FIFO, value) != KERN_SUCCESS))
        abort ();
}

void mxSemDestroy (MxSem *sem)
{
    int val;
    
    if (likely(semaphore_destroy(mach_task_self(), *sem) == KERN_SUCCESS))
        return;
    
    val = EINVAL;
    
    MX_THREAD_ASSERT ("destroying semaphore");
}

int mxSemPost (MxSem *sem)
{
    int val;
    
    if (likely(semaphore_signal(*sem) == KERN_SUCCESS))
        return 0;
    
    val = EINVAL;
    
    if (unlikely(val != EOVERFLOW))
        MX_THREAD_ASSERT ("unlocking semaphore");
    return val;
}

void mxSemWait (MxSem *sem)
{
    int val;
    
    if (likely(semaphore_wait(*sem) == KERN_SUCCESS))
        return;
    
    val = EINVAL;
    
    MX_THREAD_ASSERT ("locking semaphore");
}

void mxRWLockInit (MxRWLock *lock)
{
    if (unlikely(pthread_rwlock_init (lock, NULL)))
        abort ();
}

void mxRWLockDestroy (MxRWLock *lock)
{
    int val = pthread_rwlock_destroy (lock);
    MX_THREAD_ASSERT ("destroying R/W lock");
}

void mxRWLockRdlock (MxRWLock *lock)
{
    int val = pthread_rwlock_rdlock (lock);
    MX_THREAD_ASSERT ("acquiring R/W lock for reading");
}

void mxRWLockWrlock (MxRWLock *lock)
{
    int val = pthread_rwlock_wrlock (lock);
    MX_THREAD_ASSERT ("acquiring R/W lock for writing");
}

void mxRWLockUnlock (MxRWLock *lock)
{
    int val = pthread_rwlock_unlock (lock);
    MX_THREAD_ASSERT ("releasing R/W lock");
}

int mxThreadvarCreate (MxThreadvar *key, void (*destr) (void *))
{
    return pthread_key_create (key, destr);
}

void mxThreadvarDelete (MxThreadvar *p_tls)
{
    pthread_key_delete (*p_tls);
}

int mxThreadvarSet (MxThreadvar key, void *value)
{
    return pthread_setspecific (key, value);
}

void *mxThreadvarGet (MxThreadvar key)
{
    return pthread_getspecific (key);
}

void mxThreadsSetup(libvlc_int_t *p_libvlc)
{
    (void) p_libvlc;
}

static int mxCloneAttr(MxThread *th, pthread_attr_t *attr,
                           void *(*entry) (void *), void *data, int priority)
{
    int ret;
    
    sigset_t oldset;
    {
        sigset_t set;
        sigemptyset (&set);
        sigdelset (&set, SIGHUP);
        sigaddset (&set, SIGINT);
        sigaddset (&set, SIGQUIT);
        sigaddset (&set, SIGTERM);
        
        sigaddset (&set, SIGPIPE); /* We don't want this one, really! */
        pthread_sigmask (SIG_BLOCK, &set, &oldset);
    }
    
    (void) priority;
    
#define MX_STACKSIZE (128 * sizeof (void *) * 1024)
    
#ifdef MX_STACKSIZE
    ret = pthread_attr_setstacksize (attr, MX_STACKSIZE);
    assert (ret == 0); /* fails iif VLC_STACKSIZE is invalid */
#endif
    
    ret = pthread_create (th, attr, entry, data);
    pthread_sigmask (SIG_SETMASK, &oldset, NULL);
    pthread_attr_destroy (attr);
    return ret;
}

int mxClone (MxThread *th, void *(*entry) (void *), void *data,
               int priority)
{
    pthread_attr_t attr;
    
    pthread_attr_init (&attr);
    return mxCloneAttr (th, &attr, entry, data, priority);
}

void mxJoin (MxThread handle, void **result)
{
    int val = pthread_join (handle, result);
    MX_THREAD_ASSERT ("joining thread");
}

int mxCloneDetach (MxThread *th, void *(*entry) (void *), void *data,
                      int priority)
{
    MxThread dummy;
    pthread_attr_t attr;
    
    if (th == NULL)
        th = &dummy;
    
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    return mxCloneAttr (th, &attr, entry, data, priority);
}

MxThread mxThreadSelf (void)
{
    return pthread_self ();
}

unsigned long mxThreadId (void)
{
    return -1;
}

int mxSetPriority (MxThread th, int priority)
{
    (void) th; (void) priority;
    return MX_SUCCESS;
}

void mxCancel (MxThread thread_id)
{
    pthread_cancel (thread_id);
}

int mxSaveCancel (void)
{
    int state;
    int val = pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &state);
    
    MX_THREAD_ASSERT ("saving cancellation");
    return state;
}

void mxRestoreCancel (int state)
{
#ifndef NDEBUG
    int oldstate, val;
    
    val = pthread_setcancelstate (state, &oldstate);
    MX_THREAD_ASSERT ("restoring cancellation");
    
    if (unlikely(oldstate != PTHREAD_CANCEL_DISABLE))
        mx_thread_fatal ("restoring cancellation while not disabled", EINVAL,
                          __func__, __FILE__, __LINE__);
#else
    pthread_setcancelstate (state, NULL);
#endif
}

void mxTestCancel (void)
{
    pthread_testcancel ();
}

void mxControlCancel (int cmd, ...)
{
    (void) cmd;
    MX_assert_unreachable ();
}

mtime_t mdate (void)
{
    mx_clock_setup();
    uint64_t date = mach_absolute_time();
    
    /* denom is uint32_t, switch to 64 bits to prevent overflow. */
    uint64_t denom = mx_clock_conversion_factor.denom;
    
    /* Switch to microsecs */
    denom *= 1000LL;
    
    /* Split the division to prevent overflow */
    lldiv_t d = lldiv (mx_clock_conversion_factor.numer, denom);
    
    return (d.quot * date) + ((d.rem * date) / denom);
}

#undef mwait
void mwait (mtime_t deadline)
{
    deadline -= mdate ();
    if (deadline > 0)
        msleep (deadline);
}

#undef msleep
void msleep (mtime_t delay)
{
    struct timespec ts = mtime_to_ts (delay);
    
    /* nanosleep uses mach_absolute_time and mach_wait_until internally,
     but also handles kernel errors. Thus we use just this. */
    while (nanosleep (&ts, &ts) == -1)
        assert (errno == EINTR);
}

unsigned mxGetCPUCount(void)
{
    return sysconf(_SC_NPROCESSORS_CONF);
}
