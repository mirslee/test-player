#ifndef MXTHREAD_H
#define MXTHREAD_H

#include "MxCommon.h"

MX_API void vlc_testcancel(void);

#if defined (_WIN32)
    # include <process.h>
    # ifndef ETIMEDOUT
        #  define ETIMEDOUT 10060 /* This is the value in winsock.h. */
    # endif

    typedef struct vlc_thread *vlc_thread_t;
    # define VLC_THREAD_CANCELED NULL
    # define LIBVLC_NEED_SLEEP
    typedef struct
    {
        bool dynamic;
        union
        {
            struct
            {
                bool locked;
                unsigned long contention;
            };
            CRITICAL_SECTION mutex;
        };
    } vlc_mutex_t;
    #define VLC_STATIC_MUTEX { false, { { false, 0 } } }
    #define LIBVLC_NEED_CONDVAR
    #define LIBVLC_NEED_SEMAPHORE
    #define LIBVLC_NEED_RWLOCK
    typedef struct vlc_threadvar *vlc_threadvar_t;
    typedef struct vlc_timer *vlc_timer_t;

    # define VLC_THREAD_PRIORITY_LOW      0
    # define VLC_THREAD_PRIORITY_INPUT    THREAD_PRIORITY_ABOVE_NORMAL
    # define VLC_THREAD_PRIORITY_AUDIO    THREAD_PRIORITY_HIGHEST
    # define VLC_THREAD_PRIORITY_VIDEO    0
    # define VLC_THREAD_PRIORITY_OUTPUT   THREAD_PRIORITY_ABOVE_NORMAL
    # define VLC_THREAD_PRIORITY_HIGHEST  THREAD_PRIORITY_TIME_CRITICAL

    static inline int vlc_poll(struct pollfd *fds, unsigned nfds, int timeout)
    {
        int val;
        
        vlc_testcancel();
        val = poll(fds, nfds, timeout);
        if (val < 0)
            vlc_testcancel();
        return val;
    }
    # define poll(u,n,t) vlc_poll(u, n, t)

#elif defined (__OS2__)
    # include <errno.h>

    typedef struct vlc_thread *vlc_thread_t;
    #define VLC_THREAD_CANCELED NULL
    typedef struct
    {
        bool dynamic;
        union
        {
            struct
            {
                bool locked;
                unsigned long contention;
            };
            HMTX hmtx;
        };
    } vlc_mutex_t;
    #define VLC_STATIC_MUTEX { false, { { false, 0 } } }
    typedef struct
    {
        HEV      hev;
        unsigned waiters;
        HEV      hevAck;
        unsigned signaled;
    } vlc_cond_t;
    #define VLC_STATIC_COND { NULLHANDLE, 0, NULLHANDLE, 0 }
    #define LIBVLC_NEED_SEMAPHORE
    #define LIBVLC_NEED_RWLOCK
    typedef struct vlc_threadvar *vlc_threadvar_t;
    typedef struct vlc_timer *vlc_timer_t;

    # define VLC_THREAD_PRIORITY_LOW      0
    # define VLC_THREAD_PRIORITY_INPUT \
    MAKESHORT(PRTYD_MAXIMUM / 2, PRTYC_REGULAR)
    # define VLC_THREAD_PRIORITY_AUDIO    MAKESHORT(PRTYD_MAXIMUM, PRTYC_REGULAR)
    # define VLC_THREAD_PRIORITY_VIDEO    0
    # define VLC_THREAD_PRIORITY_OUTPUT \
    MAKESHORT(PRTYD_MAXIMUM / 2, PRTYC_REGULAR)
    # define VLC_THREAD_PRIORITY_HIGHEST  MAKESHORT(0, PRTYC_TIMECRITICAL)

    # define pthread_sigmask  sigprocmask

    static inline int vlc_poll (struct pollfd *fds, unsigned nfds, int timeout)
    {
        static int (*vlc_poll_os2)(struct pollfd *, unsigned, int) = NULL;
        
        if (!vlc_poll_os2)
        {
            HMODULE hmod;
            CHAR szFailed[CCHMAXPATH];
            
            if (DosLoadModule(szFailed, sizeof(szFailed), "vlccore", &hmod))
                return -1;
            
            if (DosQueryProcAddr(hmod, 0, "_vlc_poll_os2", (PFN *)&vlc_poll_os2))
                return -1;
        }
        
        return (*vlc_poll_os2)(fds, nfds, timeout);
    }
    # define poll(u,n,t) vlc_poll(u, n, t)

#elif defined (__ANDROID__)      /* pthreads subset without pthread_cancel() */
    # include <unistd.h>
    # include <pthread.h>
    # include <poll.h>
    # define LIBVLC_USE_PTHREAD_CLEANUP   1
    # define LIBVLC_NEED_SLEEP
    # define LIBVLC_NEED_CONDVAR
    # define LIBVLC_NEED_SEMAPHORE
    # define LIBVLC_NEED_RWLOCK

    typedef struct vlc_thread *vlc_thread_t;
    #define VLC_THREAD_CANCELED NULL
    typedef pthread_mutex_t vlc_mutex_t;
    #define VLC_STATIC_MUTEX PTHREAD_MUTEX_INITIALIZER

    typedef pthread_key_t   vlc_threadvar_t;
    typedef struct vlc_timer *vlc_timer_t;

    # define VLC_THREAD_PRIORITY_LOW      0
    # define VLC_THREAD_PRIORITY_INPUT    0
    # define VLC_THREAD_PRIORITY_AUDIO    0
    # define VLC_THREAD_PRIORITY_VIDEO    0
    # define VLC_THREAD_PRIORITY_OUTPUT   0
    # define VLC_THREAD_PRIORITY_HIGHEST  0

    static inline int vlc_poll (struct pollfd *fds, unsigned nfds, int timeout)
    {
        int val;
        
        do
        {
            int ugly_timeout = ((unsigned)timeout >= 50) ? 50 : timeout;
            if (timeout >= 0)
                timeout -= ugly_timeout;
            
            vlc_testcancel ();
            val = poll (fds, nfds, ugly_timeout);
        }
        while (val == 0 && timeout != 0);
        
        return val;
    }

    # define poll(u,n,t) vlc_poll(u, n, t)

    #elif defined (__APPLE__)
    # define _APPLE_C_SOURCE    1 /* Proper pthread semantics on OSX */
    # include <unistd.h>
    # include <pthread.h>
    /* Unnamed POSIX semaphores not supported on Mac OS X */
    # include <mach/semaphore.h>
    # include <mach/task.h>
    # define LIBVLC_USE_PTHREAD           1
    # define LIBVLC_USE_PTHREAD_CLEANUP   1

    typedef pthread_t       vlc_thread_t;
    #define VLC_THREAD_CANCELED PTHREAD_CANCELED
    typedef pthread_mutex_t vlc_mutex_t;
    #define VLC_STATIC_MUTEX PTHREAD_MUTEX_INITIALIZER
    typedef pthread_cond_t vlc_cond_t;
    #define VLC_STATIC_COND PTHREAD_COND_INITIALIZER
    typedef semaphore_t     vlc_sem_t;
    typedef pthread_rwlock_t vlc_rwlock_t;
    #define VLC_STATIC_RWLOCK PTHREAD_RWLOCK_INITIALIZER
    typedef pthread_key_t   vlc_threadvar_t;
    typedef struct vlc_timer *vlc_timer_t;

    # define VLC_THREAD_PRIORITY_LOW      0
    # define VLC_THREAD_PRIORITY_INPUT   22
    # define VLC_THREAD_PRIORITY_AUDIO   22
    # define VLC_THREAD_PRIORITY_VIDEO    0
    # define VLC_THREAD_PRIORITY_OUTPUT  22
    # define VLC_THREAD_PRIORITY_HIGHEST 22

#else /* POSIX threads */
    # include <unistd.h> /* _POSIX_SPIN_LOCKS */
    # include <pthread.h>
    # include <semaphore.h>

    /**
     * Whether LibVLC threads are based on POSIX threads.
     */
    # define LIBVLC_USE_PTHREAD           1

    /**
     * Whether LibVLC thread cancellation is based on POSIX threads.
     */
    # define LIBVLC_USE_PTHREAD_CLEANUP   1

    /**
     * Thread handle.
     */
    typedef struct
    {
        pthread_t handle;
    } vlc_thread_t;

    /**
     * Return value of a canceled thread.
     */
    #define VLC_THREAD_CANCELED PTHREAD_CANCELED

    /**
     * Mutex.
     *
     * Storage space for a mutual exclusion lock.
     */
    typedef pthread_mutex_t vlc_mutex_t;

    /**
     * Static initializer for (static) mutex.
     */
    #define VLC_STATIC_MUTEX PTHREAD_MUTEX_INITIALIZER

    /**
     * Condition variable.
     *
     * Storage space for a thread condition variable.
     */
    typedef pthread_cond_t  vlc_cond_t;

    /**
     * Static initializer for (static) condition variable.
     *
     * \note
     * The condition variable will use the default clock, which is OS-dependent.
     * Therefore, where timed waits are necessary the condition variable should
     * always be initialized dynamically explicit instead of using this
     * initializer.
     */
    #define VLC_STATIC_COND  PTHREAD_COND_INITIALIZER

    /**
     * Semaphore.
     *
     * Storage space for a thread-safe semaphore.
     */
    typedef sem_t           vlc_sem_t;

    /**
     * Read/write lock.
     *
     * Storage space for a slim reader/writer lock.
     */
    typedef pthread_rwlock_t vlc_rwlock_t;

    /**
     * Static initializer for (static) read/write lock.
     */
    #define VLC_STATIC_RWLOCK PTHREAD_RWLOCK_INITIALIZER

    /**
     * Thread-local key handle.
     */
    typedef pthread_key_t   vlc_threadvar_t;

    /**
     * Threaded timer handle.
     */
    typedef struct vlc_timer *vlc_timer_t;

    # define VLC_THREAD_PRIORITY_LOW      0
    # define VLC_THREAD_PRIORITY_INPUT   10
    # define VLC_THREAD_PRIORITY_AUDIO    5
    # define VLC_THREAD_PRIORITY_VIDEO    0
    # define VLC_THREAD_PRIORITY_OUTPUT  15
    # define VLC_THREAD_PRIORITY_HIGHEST 20

#endif

#ifdef LIBVLC_NEED_CONDVAR
    typedef struct
    {
        unsigned value;
    } vlc_cond_t;
    # define VLC_STATIC_COND { 0 }
#endif

#ifdef LIBVLC_NEED_SEMAPHORE
    typedef struct vlc_sem
    {
        vlc_mutex_t lock;
        vlc_cond_t  wait;
        unsigned    value;
    } vlc_sem_t;
#endif

#ifdef LIBVLC_NEED_RWLOCK
    typedef struct vlc_rwlock
    {
        vlc_mutex_t   mutex;
        vlc_cond_t    wait;
        long          state;
    } vlc_rwlock_t;
    # define VLC_STATIC_RWLOCK { VLC_STATIC_MUTEX, VLC_STATIC_COND, 0 }
#endif

MX_API void vlc_mutex_init(vlc_mutex_t *);

MX_API void vlc_mutex_init_recursive(vlc_mutex_t *);

MX_API void vlc_mutex_destroy(vlc_mutex_t *);

MX_API void vlc_mutex_lock(vlc_mutex_t *);

MX_API int vlc_mutex_trylock( vlc_mutex_t * ) MX_USED;

MX_API void vlc_mutex_unlock(vlc_mutex_t *);

MX_API void vlc_cond_init(vlc_cond_t *);

void vlc_cond_init_daytime(vlc_cond_t *);

MX_API void vlc_cond_destroy(vlc_cond_t *);

MX_API void vlc_cond_signal(vlc_cond_t *);

MX_API void vlc_cond_broadcast(vlc_cond_t *);

MX_API void vlc_cond_wait(vlc_cond_t *cond, vlc_mutex_t *mutex);

MX_API int vlc_cond_timedwait(vlc_cond_t *cond, vlc_mutex_t *mutex,
                               mtime_t deadline);

int vlc_cond_timedwait_daytime(vlc_cond_t *, vlc_mutex_t *, time_t);

MX_API void vlc_sem_init(vlc_sem_t *, unsigned count);

MX_API void vlc_sem_destroy(vlc_sem_t *);

MX_API int vlc_sem_post(vlc_sem_t *);

MX_API void vlc_sem_wait(vlc_sem_t *);

MX_API void vlc_rwlock_init(vlc_rwlock_t *);

MX_API void vlc_rwlock_destroy(vlc_rwlock_t *);

MX_API void vlc_rwlock_rdlock(vlc_rwlock_t *);

MX_API void vlc_rwlock_wrlock(vlc_rwlock_t *);

MX_API void vlc_rwlock_unlock(vlc_rwlock_t *);

MX_API int vlc_threadvar_create(vlc_threadvar_t *key, void (*destr) (void *));

MX_API void vlc_threadvar_delete(vlc_threadvar_t *);

MX_API int vlc_threadvar_set(vlc_threadvar_t key, void *value);

MX_API void *vlc_threadvar_get(vlc_threadvar_t);

void vlc_addr_wait(void *addr, unsigned val);

bool vlc_addr_timedwait(void *addr, unsigned val, mtime_t delay);

void vlc_addr_signal(void *addr);

void vlc_addr_broadcast(void *addr);

MX_API int vlc_clone(vlc_thread_t *th, void *(*entry)(void *), void *data,
                      int priority) MX_USED;

MX_API void vlc_cancel(vlc_thread_t);

MX_API void vlc_join(vlc_thread_t th, void **result);

MX_API int vlc_savecancel(void);

MX_API void vlc_restorecancel(int state);

MX_API void vlc_control_cancel(int cmd, ...);

MX_API vlc_thread_t vlc_thread_self(void) MX_USED;

MX_API unsigned long vlc_thread_id(void) MX_USED;

MX_API mtime_t mdate(void);

MX_API void mwait(mtime_t deadline);

MX_API void msleep(mtime_t delay);

#define VLC_HARD_MIN_SLEEP   10000 /* 10 milliseconds = 1 tick at 100Hz */
#define VLC_SOFT_MIN_SLEEP 9000000 /* 9 seconds */

#if defined (__GNUC__) && !defined (__clang__)


static
__attribute__((unused))
__attribute__((noinline))
__attribute__((error("sorry, cannot sleep for such short a time")))
mtime_t impossible_delay( mtime_t delay )
{
    (void) delay;
    return VLC_HARD_MIN_SLEEP;
}

static
__attribute__((unused))
__attribute__((noinline))
__attribute__((warning("use proper event handling instead of short delay")))
mtime_t harmful_delay( mtime_t delay )
{
    return delay;
}

# define check_delay( d ) \
((__builtin_constant_p(d < VLC_HARD_MIN_SLEEP) \
&& (d < VLC_HARD_MIN_SLEEP)) \
? impossible_delay(d) \
: ((__builtin_constant_p(d < VLC_SOFT_MIN_SLEEP) \
&& (d < VLC_SOFT_MIN_SLEEP)) \
? harmful_delay(d) \
: d))

static
__attribute__((unused))
__attribute__((noinline))
__attribute__((error("deadlines can not be constant")))
mtime_t impossible_deadline( mtime_t deadline )
{
    return deadline;
}

# define check_deadline( d ) \
(__builtin_constant_p(d) ? impossible_deadline(d) : d)
#else
# define check_delay(d) (d)
# define check_deadline(d) (d)
#endif

#define msleep(d) msleep(check_delay(d))
#define mwait(d) mwait(check_deadline(d))

MX_API int vlc_timer_create(vlc_timer_t *id, void (*func)(void *), void *data)
MX_USED;

MX_API void vlc_timer_destroy(vlc_timer_t timer);

MX_API void vlc_timer_schedule(vlc_timer_t timer, bool absolute,
                                mtime_t value, mtime_t interval);

MX_API unsigned vlc_timer_getoverrun(vlc_timer_t) MX_USED;

MX_API unsigned vlc_GetCPUCount(void);

enum
{
    VLC_CLEANUP_PUSH,
    VLC_CLEANUP_POP,
    VLC_CANCEL_ADDR_SET,
    VLC_CANCEL_ADDR_CLEAR,
};

#if defined (LIBVLC_USE_PTHREAD_CLEANUP)

# define vlc_cleanup_push( routine, arg ) pthread_cleanup_push (routine, arg)

# define vlc_cleanup_pop( ) pthread_cleanup_pop (0)

#else
typedef struct vlc_cleanup_t vlc_cleanup_t;

struct vlc_cleanup_t
{
    vlc_cleanup_t *next;
    void         (*proc) (void *);
    void          *data;
};


# define vlc_cleanup_push( routine, arg ) \
do { \
vlc_cleanup_t vlc_cleanup_data = { NULL, routine, arg, }; \
vlc_control_cancel (VLC_CLEANUP_PUSH, &vlc_cleanup_data)

# define vlc_cleanup_pop( ) \
vlc_control_cancel (VLC_CLEANUP_POP); \
} while (0)

#endif

static inline void vlc_cleanup_lock (void *lock)
{
    vlc_mutex_unlock ((vlc_mutex_t *)lock);
}
#define mutex_cleanup_push( lock ) vlc_cleanup_push (vlc_cleanup_lock, lock)

static inline void vlc_cancel_addr_set(void *addr)
{
    vlc_control_cancel(VLC_CANCEL_ADDR_SET, addr);
}

static inline void vlc_cancel_addr_clear(void *addr)
{
    vlc_control_cancel(VLC_CANCEL_ADDR_CLEAR, addr);
}

#ifdef __cplusplus

class vlc_mutex_locker
{
private:
    vlc_mutex_t *lock;
public:
    vlc_mutex_locker (vlc_mutex_t *m) : lock (m)
    {
        vlc_mutex_lock (lock);
    }
    
    ~vlc_mutex_locker (void)
    {
        vlc_mutex_unlock (lock);
    }
};
#endif

enum
{
    VLC_AVCODEC_MUTEX = 0,
    VLC_GCRYPT_MUTEX,
    VLC_XLIB_MUTEX,
    VLC_MOSAIC_MUTEX,
    VLC_HIGHLIGHT_MUTEX,
#ifdef _WIN32
    VLC_MTA_MUTEX,
#endif
    /* Insert new entry HERE */
    VLC_MAX_MUTEX
};

MX_API void vlc_global_mutex(unsigned, bool);

#define vlc_global_lock( n ) vlc_global_mutex(n, true)

#define vlc_global_unlock( n ) vlc_global_mutex(n, false)

#endif /* MXTHREAD_H */
