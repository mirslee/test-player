#include "stdafx.h"
#include "MxThread.h"
#include "MxConfig.h"
#include "MxCommon.h"
#include "MxError.h"

#include "MxAtomic.h"
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#if !MX_WINSTORE_APP
#include <mmsystem.h>
#endif

#include "MxCpu.h"

struct vlc_object_t {
};

/*** Static mutex and condition variable ***/
static CRITICAL_SECTION super_mutex;
static CONDITION_VARIABLE super_variable;

#define IS_INTERRUPTIBLE (!MX_WINSTORE_APP || _WIN32_WINNT >= 0x0A00)

/*** Threads ***/
static DWORD thread_key;

struct Mx_Thread
{
	HANDLE         id;

	bool           killable;
	atomic_bool    killed;
	MxCleanup     *cleaners;

	void        *(*entry) (void *);
	void          *data;

	struct
	{
		atomic_int      *addr;
		CRITICAL_SECTION lock;
	} wait;
};

/*** Condition variables (low-level) ***/
#if (_WIN32_WINNT < _WIN32_WINNT_VISTA)
static VOID(WINAPI *InitializeConditionVariable_)(PCONDITION_VARIABLE);
#define InitializeConditionVariable InitializeConditionVariable_
static BOOL(WINAPI *SleepConditionVariableCS_)(PCONDITION_VARIABLE,
	PCRITICAL_SECTION, DWORD);
#define SleepConditionVariableCS SleepConditionVariableCS_
static VOID(WINAPI *WakeAllConditionVariable_)(PCONDITION_VARIABLE);
#define WakeAllConditionVariable WakeAllConditionVariable_

static void WINAPI DummyConditionVariable(CONDITION_VARIABLE *cv)
{
	(void)cv;
}

static BOOL WINAPI SleepConditionVariableFallback(CONDITION_VARIABLE *cv,
	CRITICAL_SECTION *cs,
	DWORD ms)
{
	(void)cv;
	LeaveCriticalSection(cs);
	SleepEx(ms > 5 ? 5 : ms, TRUE);
	EnterCriticalSection(cs);
	return ms != 0;
}
#endif

/*** Mutexes ***/
void mxMutexInit(MxMutex *p_mutex)
{
	/* This creates a recursive mutex. This is OK as fast mutexes have
	* no defined behavior in case of recursive locking. */
	InitializeCriticalSection(&p_mutex->mutex);
	p_mutex->dynamic = true;
}

void mxMutexInitRecursive(MxMutex *p_mutex)
{
	InitializeCriticalSection(&p_mutex->mutex);
	p_mutex->dynamic = true;
}


void mxMutexDestroy(MxMutex *p_mutex)
{
	assert(p_mutex->dynamic);
	DeleteCriticalSection(&p_mutex->mutex);
}

void mxMutexLock(MxMutex *p_mutex)
{
	if (!p_mutex->dynamic)
	{   /* static mutexes */
		EnterCriticalSection(&super_mutex);
		while (p_mutex->locked)
		{
			p_mutex->contention++;
			SleepConditionVariableCS(&super_variable, &super_mutex, INFINITE);
			p_mutex->contention--;
		}
		p_mutex->locked = true;
		LeaveCriticalSection(&super_mutex);
		return;
	}

	EnterCriticalSection(&p_mutex->mutex);
}

int mxMutexTrylock(MxMutex *p_mutex)
{
	if (!p_mutex->dynamic)
	{   /* static mutexes */
		int ret = EBUSY;

		EnterCriticalSection(&super_mutex);
		if (!p_mutex->locked)
		{
			p_mutex->locked = true;
			ret = 0;
		}
		LeaveCriticalSection(&super_mutex);
		return ret;
	}

	return TryEnterCriticalSection(&p_mutex->mutex) ? 0 : EBUSY;
}

void mxMutexUnlock(MxMutex *p_mutex)
{
	if (!p_mutex->dynamic)
	{   /* static mutexes */
		EnterCriticalSection(&super_mutex);
		assert(p_mutex->locked);
		p_mutex->locked = false;
		if (p_mutex->contention)
			WakeAllConditionVariable(&super_variable);
		LeaveCriticalSection(&super_mutex);
		return;
	}

	LeaveCriticalSection(&p_mutex->mutex);
}

/*** Semaphore ***/
#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
# include <stdalign.h>

static inline HANDLE *mxSemHandleP(MxSem *sem)
{
	/* NOTE: vlc_sem_t layout cannot easily depend on Windows version */
	static_assert (sizeof(HANDLE) <= sizeof(MxSem), "Size mismatch!");
	static_assert ((alignof (HANDLE) % alignof (MxSem)) == 0,
		"Alignment mismatch");
	return (HANDLE *)sem;
}
#define mxSemHandle(sem) (*mxSemHandleP(sem))

void mxSemInit(MxSem *sem, unsigned value)
{
	HANDLE handle = CreateSemaphore(NULL, value, 0x7fffffff, NULL);
	if (handle == NULL)
		abort();

	mxSemHandle(sem) = handle;
}

void mxSemDestroy(vlc_sem_t *sem)
{
	CloseHandle(vlc_sem_handle(sem));
}

int mxSemPost(vlc_sem_t *sem)
{
	ReleaseSemaphore(vlc_sem_handle(sem), 1, NULL);
	return 0; /* FIXME */
}

void mxSemWait(vlc_sem_t *sem)
{
	HANDLE handle = mxSemHandle(sem);
	DWORD result;

	do
	{
		mxTestCancel();
		result = WaitForSingleObjectEx(handle, INFINITE, TRUE);

		/* Semaphore abandoned would be a bug. */
		assert(result != WAIT_ABANDONED_0);
	} while (result == WAIT_IO_COMPLETION || result == WAIT_FAILED);
}
#endif

/*** Thread-specific variables (TLS) ***/
struct Mx_Threadvar
{
	DWORD                 id;
	void(*destroy) (void *);
	struct Mx_Threadvar *prev;
	struct Mx_Threadvar *next;
} *mx_threadvar_last = NULL;

int mxThreadvarCreate(MxThreadvar *p_tls, void(*destr) (void *))
{
	struct Mx_Threadvar *var = (Mx_Threadvar*)malloc(sizeof(*var));
	if (unlikely(var == NULL))
		return errno;

	var->id = TlsAlloc();
	if (var->id == TLS_OUT_OF_INDEXES)
	{
		free(var);
		return EAGAIN;
	}
	var->destroy = destr;
	var->next = NULL;
	*p_tls = var;

	EnterCriticalSection(&super_mutex);
	var->prev = mx_threadvar_last;
	if (var->prev)
		var->prev->next = var;

	mx_threadvar_last = var;
	LeaveCriticalSection(&super_mutex);
	return 0;
}

void mxThreadvarDelete(MxThreadvar *p_tls)
{
	struct Mx_Threadvar *var = *p_tls;

	EnterCriticalSection(&super_mutex);
	if (var->prev != NULL)
		var->prev->next = var->next;

	if (var->next != NULL)
		var->next->prev = var->prev;
	else
		mx_threadvar_last = var->prev;

	LeaveCriticalSection(&super_mutex);

	TlsFree(var->id);
	free(var);
}

int mxThreadvarSet(MxThreadvar key, void *value)
{
	int saved = GetLastError();

	if (!TlsSetValue(key->id, value))
		return ENOMEM;

	SetLastError(saved);
	return 0;
}

void *mxThreadvarGet(MxThreadvar key)
{
	int saved = GetLastError();
	void *value = TlsGetValue(key->id);

	SetLastError(saved);
	return value;
}

static void mxThreadvarsCleanup(void)
{
	MxThreadvar key;
retry:
	/* TODO: use RW lock or something similar */
	EnterCriticalSection(&super_mutex);
	for (key = mx_threadvar_last; key != NULL; key = key->prev)
	{
		void *value = mxThreadvarGet(key);
		if (value != NULL && key->destroy != NULL)
		{
			LeaveCriticalSection(&super_mutex);
			mxThreadvarSet(key, NULL);
			key->destroy(value);
			goto retry;
		}
	}
	LeaveCriticalSection(&super_mutex);
}

/*** Futeces^WAddress waits ***/
#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
static BOOL(WINAPI *WaitOnAddress_)(VOID volatile *, PVOID, SIZE_T, DWORD);
#define WaitOnAddress (*WaitOnAddress_)
static VOID(WINAPI *WakeByAddressAll_)(PVOID);
#define WakeByAddressAll (*WakeByAddressAll_)
static VOID(WINAPI *WakeByAddressSingle_)(PVOID);
#define WakeByAddressSingle (*WakeByAddressSingle_)

static struct wait_addr_bucket
{
	CRITICAL_SECTION lock;
	CONDITION_VARIABLE wait;
} wait_addr_buckets[32];

static struct wait_addr_bucket *wait_addr_get_bucket(void volatile *addr)
{
	uintptr_t u = (uintptr_t)addr;

	return wait_addr_buckets + ((u >> 3) % ARRAY_SIZE(wait_addr_buckets));
}

static void mxWaitAddrInit(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(wait_addr_buckets); i++)
	{
		struct wait_addr_bucket *bucket = wait_addr_buckets + i;

		InitializeCriticalSection(&bucket->lock);
		InitializeConditionVariable(&bucket->wait);
	}
}

static void mxWaitAddrDeinit(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(wait_addr_buckets); i++)
	{
		struct wait_addr_bucket *bucket = wait_addr_buckets + i;

		DeleteCriticalSection(&bucket->lock);
	}
}

static BOOL WINAPI WaitOnAddressFallback(void volatile *addr, void *value,
	SIZE_T size, DWORD ms)
{
	struct wait_addr_bucket *bucket = wait_addr_get_bucket(addr);
	uint64_t futex, val = 0;
	BOOL ret = 0;

	EnterCriticalSection(&bucket->lock);

	switch (size)
	{
	case 1:
		futex = atomic_load_explicit((atomic_char *)addr,
			memory_order_relaxed);
		val = *(const char *)value;
		break;
	case 2:
		futex = atomic_load_explicit((atomic_short *)addr,
			memory_order_relaxed);
		val = *(const short *)value;
		break;
	case 4:
		futex = atomic_load_explicit((atomic_int *)addr,
			memory_order_relaxed);
		val = *(const int *)value;
		break;
	case 8:
		futex = atomic_load_explicit((atomic_llong *)addr,
			memory_order_relaxed);
		val = *(const long long *)value;
		break;
	default:
		vlc_assert_unreachable();
	}

	if (futex == val)
		ret = SleepConditionVariableCS(&bucket->wait, &bucket->lock, ms);

	LeaveCriticalSection(&bucket->lock);
	return ret;
}

static void WINAPI WakeByAddressFallback(void *addr)
{
	struct wait_addr_bucket *bucket = wait_addr_get_bucket(addr);

	/* Acquire the bucket critical section (only) to enforce proper sequencing.
	* The critical section does not protect any actual memory object. */
	EnterCriticalSection(&bucket->lock);
	/* No other threads can hold the lock for this bucket while it is held
	* here. Thus any other thread either:
	* - is already sleeping in SleepConditionVariableCS(), and to be woken up
	*   by the following WakeAllConditionVariable(), or
	* - has yet to retrieve the value at the wait address (with the
	*   'switch (size)' block). */
	LeaveCriticalSection(&bucket->lock);
	/* At this point, other threads can retrieve the value at the wait address.
	* But the value will have already been changed by our call site, thus
	* (futex == val) will be false, and the threads will not go to sleep. */

	/* Wake up any thread that was already sleeping. Since there are more than
	* one wait address per bucket, all threads must be woken up :-/ */
	WakeAllConditionVariable(&bucket->wait);
}
#endif

void mxAddrWait(void *addr, unsigned val)
{
	WaitOnAddress(addr, &val, sizeof(val), -1);
}

bool mxAddrTimedwait(void *addr, unsigned val, mtime_t delay)
{
	delay = (delay + 999) / 1000;

	if (delay > 0x7fffffff)
	{
		WaitOnAddress(addr, &val, sizeof(val), 0x7fffffff);
		return true; /* woke up early, claim spurious wake-up */
	}

	return WaitOnAddress(addr, &val, sizeof(val), delay);
}

void mxAddrSignal(void *addr)
{
	WakeByAddressSingle(addr);
}

void mxAddrBroadcast(void *addr)
{
	WakeByAddressAll(addr);
}

/*** Threads ***/
static void mxThreadDestroy(MxThread th)
{
	DeleteCriticalSection(&th->wait.lock);
	free(th);
}

static unsigned __stdcall mxEntry(void *p)
{
	struct Mx_Thread *th = (Mx_Thread*)p;

	TlsSetValue(thread_key, th);
	th->killable = true;
	th->data = th->entry(th->data);
	TlsSetValue(thread_key, NULL);

	if (th->id == NULL) /* Detached thread */
		mxThreadDestroy(th);
	return 0;
}

static int mxCloneAttr(MxThread *p_handle, bool detached,
	void *(*entry) (void *), void *data, int priority)
{
	struct Mx_Thread *th = (Mx_Thread*)malloc(sizeof(*th));
	if (unlikely(th == NULL))
		return ENOMEM;
	th->entry = entry;
	th->data = data;
	th->killable = false; /* not until vlc_entry() ! */
	atomic_init(&th->killed, false);
	th->cleaners = NULL;
	th->wait.addr = NULL;
	InitializeCriticalSection(&th->wait.lock);

	/* When using the MSVCRT C library you have to use the _beginthreadex
	* function instead of CreateThread, otherwise you'll end up with
	* memory leaks and the signal functions not working (see Microsoft
	* Knowledge Base, article 104641) */
	uintptr_t h = _beginthreadex(NULL, 0, mxEntry, th, 0, NULL);
	if (h == 0)
	{
		int err = errno;
		free(th);
		return err;
	}

	if (detached)
	{
		CloseHandle((HANDLE)h);
		th->id = NULL;
	}
	else
		th->id = (HANDLE)h;

	if (p_handle != NULL)
		*p_handle = th;

	if (priority)
		SetThreadPriority(th->id, priority);

	return 0;
}

int mxClone(MxThread *p_handle, void *(*entry) (void *),
	void *data, int priority)
{
	return mxCloneAttr(p_handle, false, entry, data, priority);
}

void mxJoin(MxThread th, void **result)
{
	DWORD ret;

	do
	{
		mxTestCancel();
		ret = WaitForSingleObjectEx(th->id, INFINITE, TRUE);
		assert(ret != WAIT_ABANDONED_0);
	} while (ret == WAIT_IO_COMPLETION || ret == WAIT_FAILED);

	if (result != NULL)
		*result = th->data;
	CloseHandle(th->id);
	mxThreadDestroy(th);
}

int mxCloneDetach(MxThread *p_handle, void *(*entry) (void *),
	void *data, int priority)
{
	MxThread th;
	if (p_handle == NULL)
		p_handle = &th;

	return mxCloneAttr(p_handle, true, entry, data, priority);
}

MxThread mxThreadSelf()
{
	return (MxThread)TlsGetValue(thread_key);
}

unsigned long mxThreadId()
{
	return GetCurrentThreadId();
}

int vlc_set_priority(MxThread th, int priority)
{
	if (!SetThreadPriority(th->id, priority))
		return MX_EGENERIC;
	return MX_SUCCESS;
}

/*** Thread cancellation ***/

#if IS_INTERRUPTIBLE
/* APC procedure for thread cancellation */
static void CALLBACK mxCancelSelf(ULONG_PTR self)
{
	(void)self;
}
#endif

void mxCancel(MxThread th)
{
	atomic_store_explicit(&th->killed, true, memory_order_relaxed);

	EnterCriticalSection(&th->wait.lock);
	if (th->wait.addr != NULL)
	{
		atomic_fetch_or_explicit(th->wait.addr, 1, memory_order_relaxed);
		mxAddrBroadcast(th->wait.addr);
	}
	LeaveCriticalSection(&th->wait.lock);

#if IS_INTERRUPTIBLE
	QueueUserAPC(mxCancelSelf, th->id, (uintptr_t)th);
#endif
}

int mxSaveCancel()
{
	struct Mx_Thread *th =  mxThreadSelf();
	if (th == NULL)
		return false; /* Main thread - cannot be cancelled anyway */

	int state = th->killable;
	th->killable = false;
	return state;
}

void mxRestoreCancel(int state)
{
	struct Mx_Thread *th = mxThreadSelf();
	assert(state == false || state == true);

	if (th == NULL)
		return; /* Main thread - cannot be cancelled anyway */

	assert(!th->killable);
	th->killable = state != 0;
}

void mxTestCancel(void)
{
	struct Mx_Thread *th = mxThreadSelf();
	if (th == NULL)
		return; /* Main thread - cannot be cancelled anyway */
	if (!th->killable)
		return;
	if (!atomic_load_explicit(&th->killed, memory_order_relaxed))
		return;

	th->killable = true; /* Do not re-enter cancellation cleanup */

	for (MxCleanup *p = th->cleaners; p != NULL; p = p->next)
		p->proc(p->data);

	th->data = NULL; /* TODO: special value? */
	if (th->id == NULL) /* Detached thread */
		mxThreadDestroy(th);
	_endthreadex(0);
}

void mxControlCancel(int cmd, ...)
{
	/* NOTE: This function only modifies thread-specific data, so there is no
	* need to lock anything. */
	va_list ap;

	struct Mx_Thread *th = mxThreadSelf();
	if (th == NULL)
		return; /* Main thread - cannot be cancelled anyway */

	va_start(ap, cmd);
	switch (cmd)
	{
	case MX_CLEANUP_PUSH:
	{
		/* cleaner is a pointer to the caller stack, no need to allocate
		* and copy anything. As a nice side effect, this cannot fail. */
		MxCleanup *cleaner = va_arg(ap, MxCleanup *);
		cleaner->next = th->cleaners;
		th->cleaners = cleaner;
		break;
	}

	case MX_CLEANUP_POP:
	{
		th->cleaners = th->cleaners->next;
		break;
	}

	case MX_CANCEL_ADDR_SET:
	{
		void *addr = va_arg(ap, void *);

		EnterCriticalSection(&th->wait.lock);
		assert(th->wait.addr == NULL);
		th->wait.addr = (std::atomic_int *)addr;
		LeaveCriticalSection(&th->wait.lock);
		break;
	}

	case MX_CANCEL_ADDR_CLEAR:
	{
		void *addr = va_arg(ap, void *);

		EnterCriticalSection(&th->wait.lock);
		assert(th->wait.addr == addr);
		th->wait.addr = NULL;
		LeaveCriticalSection(&th->wait.lock);
		break;
	}
	}
	va_end(ap);
}

/*** Clock ***/
static union
{
#if (_WIN32_WINNT < _WIN32_WINNT_WIN7)
	struct
	{
		BOOL(*query) (PULONGLONG);
	} interrupt;
#endif
#if (_WIN32_WINNT < _WIN32_WINNT_VISTA)
	struct
	{
		ULONGLONG(*get) (void);
	} tick;
#endif
	struct
	{
		LARGE_INTEGER freq;
	} perf;
#if !MX_WINSTORE_APP
	struct
	{
		MMRESULT(WINAPI *timeGetDevCaps)(LPTIMECAPS ptc, UINT cbtc);
		DWORD(WINAPI *timeGetTime)(void);
	} multimedia;
#endif
} clk;

static mtime_t mdate_interrupt(void)
{
	ULONGLONG ts;
	BOOL ret;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
	ret = QueryUnbiasedInterruptTime(&ts);
#else
	ret = clk.interrupt.query(&ts);
#endif
	if (unlikely(!ret))
		abort();

	/* hundreds of nanoseconds */
	static_assert ((10000000 % CLOCK_FREQ) == 0, "Broken frequencies ratio");
	return ts / (10000000 / CLOCK_FREQ);
}

static mtime_t mdate_tick(void)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
	ULONGLONG ts = GetTickCount64();
#else
	ULONGLONG ts = clk.tick.get();
#endif

	/* milliseconds */
	static_assert ((CLOCK_FREQ % 1000) == 0, "Broken frequencies ratio");
	return ts * (CLOCK_FREQ / 1000);
}
#if !MX_WINSTORE_APP
static mtime_t mdate_multimedia(void)
{
	DWORD ts = clk.multimedia.timeGetTime();

	/* milliseconds */
	static_assert ((CLOCK_FREQ % 1000) == 0, "Broken frequencies ratio");
	return ts * (CLOCK_FREQ / 1000);
}
#endif

static mtime_t mdate_perf(void)
{
	/* We don't need the real date, just the value of a high precision timer */
	LARGE_INTEGER counter;
	if (!QueryPerformanceCounter(&counter))
		abort();

	/* Convert to from (1/freq) to microsecond resolution */
	/* We need to split the division to avoid 63-bits overflow */
	lldiv_t d = lldiv(counter.QuadPart, clk.perf.freq.QuadPart);

	return (d.quot * 1000000) + ((d.rem * 1000000) / clk.perf.freq.QuadPart);
}

static mtime_t mdate_wall(void)
{
	FILETIME ts;
	ULARGE_INTEGER s;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) && (!MX_WINSTORE_APP || _WIN32_WINNT >= 0x0A00)
	GetSystemTimePreciseAsFileTime(&ts);
#else
	GetSystemTimeAsFileTime(&ts);
#endif
	s.LowPart = ts.dwLowDateTime;
	s.HighPart = ts.dwHighDateTime;
	/* hundreds of nanoseconds */
	static_assert ((10000000 % CLOCK_FREQ) == 0, "Broken frequencies ratio");
	return s.QuadPart / (10000000 / CLOCK_FREQ);
}

static mtime_t mdate_default(void)
{
	mxThreadsSetup(NULL);
	return mdate_perf();
}

static mtime_t(*mdate_selected) (void) = mdate_default;

mtime_t mdate(void)
{
	return mdate_selected();
}

#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
void (mwait)(mtime_t deadline)
{
	mtime_t delay;

	vlc_testcancel();
	while ((delay = (deadline - mdate())) > 0)
	{
		delay = (delay + 999) / 1000;
		if (unlikely(delay > 0x7fffffff))
			delay = 0x7fffffff;

		SleepEx(delay, TRUE);
		vlc_testcancel();
	}
}

void (msleep)(mtime_t delay)
{
	mwait(mdate() + delay);
}
#endif

static BOOL SelectClockSource(void *data)
{
	vlc_object_t *obj = (vlc_object_t*)data;

#if MX_WINSTORE_APP
	const char *name = "perf";
#else
	const char *name = "multimedia";
#endif
	char *str = NULL;
	if (obj != NULL)
		//str = var_InheritString(obj, "clock-source");
		str = "clock-source";
	if (str != NULL)
		name = str;
	if (!strcmp(name, "interrupt"))
	{
		//msg_Dbg(obj, "using interrupt time as clock source");
#if (_WIN32_WINNT < _WIN32_WINNT_WIN7)
		HANDLE h = GetModuleHandle(_T("kernel32.dll"));
		if (unlikely(h == NULL))
			return FALSE;
		clk.interrupt.query = (void *)GetProcAddress(h,
			"QueryUnbiasedInterruptTime");
		if (unlikely(clk.interrupt.query == NULL))
			abort();
#endif
		mdate_selected = mdate_interrupt;
	}
	else
		if (!strcmp(name, "tick"))
		{
			//msg_Dbg(obj, "using Windows time as clock source");
#if (_WIN32_WINNT < _WIN32_WINNT_VISTA)
			HANDLE h = GetModuleHandle(_T("kernel32.dll"));
			if (unlikely(h == NULL))
				return FALSE;
			clk.tick.get = (void *)GetProcAddress(h, "GetTickCount64");
			if (unlikely(clk.tick.get == NULL))
				return FALSE;
#endif
			mdate_selected = mdate_tick;
		}
#if !MX_WINSTORE_APP
		else
			if (!strcmp(name, "multimedia"))
			{
				TIMECAPS caps;
				MMRESULT(WINAPI * timeBeginPeriod)(UINT);

				HMODULE hWinmm = LoadLibrary(TEXT("winmm.dll"));
				if (!hWinmm)
					goto perf;

				typedef MMRESULT(WINAPI *timeGetDevCapsfun)(LPTIMECAPS ptc, UINT cbtc);
				typedef DWORD(WINAPI *timeGetTimefunc)(void);
				clk.multimedia.timeGetDevCaps = (timeGetDevCapsfun)GetProcAddress(hWinmm, "timeGetDevCaps");
				clk.multimedia.timeGetTime = (timeGetTimefunc)GetProcAddress(hWinmm, "timeGetTime");
				if (!clk.multimedia.timeGetDevCaps || !clk.multimedia.timeGetTime)
					goto perf;

				//msg_Dbg(obj, "using multimedia timers as clock source");
				if (clk.multimedia.timeGetDevCaps(&caps, sizeof(caps)) != MMSYSERR_NOERROR)
					goto perf;
				/*msg_Dbg(obj, " min period: %u ms, max period: %u ms",
					caps.wPeriodMin, caps.wPeriodMax);*/
				mdate_selected = mdate_multimedia;

				typedef MMRESULT(WINAPI * timeBeginPeriodfun)(UINT);
				timeBeginPeriod = (timeBeginPeriodfun)GetProcAddress(hWinmm, "timeBeginPeriod");
				if (timeBeginPeriod != NULL)
					timeBeginPeriod(5);
			}
#endif
			else
				if (!strcmp(name, "perf"))
				{
				perf:
					//msg_Dbg(obj, "using performance counters as clock source");
					if (!QueryPerformanceFrequency(&clk.perf.freq))
						abort();
					//msg_Dbg(obj, " frequency: %llu Hz", clk.perf.freq.QuadPart);
					mdate_selected = mdate_perf;
				}
				else
					if (!strcmp(name, "wall"))
					{
						//msg_Dbg(obj, "using system time as clock source");
						mdate_selected = mdate_wall;
					}
					else
					{
						//msg_Err(obj, "invalid clock source \"%s\"", name);
						abort();
					}
	free(str);
	return TRUE;
}

size_t EnumClockSource(vlc_object_t *obj, const char *var,
	char ***vp, char ***np)
{
	const size_t max = 6;
	char **values = (char **)xmalloc(sizeof(*values) * max);
	char **names = (char **)xmalloc(sizeof(*names) * max);
	size_t n = 0;

#if (_WIN32_WINNT < _WIN32_WINNT_WIN7)
	DWORD version = LOWORD(GetVersion());
	version = (LOBYTE(version) << 8) | (HIBYTE(version) << 0);
#endif

	values[n] = xstrdup("");
	names[n] = xstrdup(_("Auto"));
	n++;
#if (_WIN32_WINNT < _WIN32_WINNT_WIN7)
	if (version >= 0x0601)
#endif
	{
		values[n] = xstrdup("interrupt");
		names[n] = xstrdup("Interrupt time");
		n++;
	}
#if (_WIN32_WINNT < _WIN32_WINNT_VISTA)
	if (version >= 0x0600)
#endif
	{
		values[n] = xstrdup("tick");
		names[n] = xstrdup("Windows time");
		n++;
	}
#if !MX_WINSTORE_APP
	values[n] = xstrdup("multimedia");
	names[n] = xstrdup("Multimedia timers");
	n++;
#endif
	values[n] = xstrdup("perf");
	names[n] = xstrdup("Performance counters");
	n++;
	values[n] = xstrdup("wall");
	names[n] = xstrdup("System time (DANGEROUS!)");
	n++;

	*vp = values;
	*np = names;
	(void)obj; (void)var;
	return n;
}


/*** CPU ***/
unsigned mxGetCPUCount(void)
{
	SYSTEM_INFO systemInfo;

	GetNativeSystemInfo(&systemInfo);

	return systemInfo.dwNumberOfProcessors;
}


/*** Initialization ***/
static CRITICAL_SECTION setup_lock; /* FIXME: use INIT_ONCE */

void mxThreadsSetup(libvlc_int_t *vlc)
{
	EnterCriticalSection(&setup_lock);
	if (mdate_selected != mdate_default)
	{
		LeaveCriticalSection(&setup_lock);
		return;
	}

	if (!SelectClockSource((vlc != NULL) ? VLC_OBJECT(vlc) : NULL))
		abort();
	assert(mdate_selected != mdate_default);

#if !MX_WINSTORE_APP
	/* Raise default priority of the current process */
#ifndef ABOVE_NORMAL_PRIORITY_CLASS
#   define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
#endif
	/*if (var_InheritBool(vlc, "high-priority"))
	{
		if (SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS)
			|| SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
			msg_Dbg(vlc, "raised process priority");
		else
			msg_Dbg(vlc, "could not raise process priority");
	}*/
#endif
	LeaveCriticalSection(&setup_lock);
}

#define LOOKUP(s) (((s##_) = (void *)GetProcAddress(h, #s)) != NULL)

extern MxRWLock config_lock;
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	(void)hinstDll;
	(void)lpvReserved;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
		HANDLE h = GetModuleHandle(TEXT("kernel32.dll"));
		if (unlikely(h == NULL))
			return FALSE;

		if (!LOOKUP(WaitOnAddress)
			|| !LOOKUP(WakeByAddressAll) || !LOOKUP(WakeByAddressSingle))
		{
# if (_WIN32_WINNT < _WIN32_WINNT_VISTA)
			if (!LOOKUP(InitializeConditionVariable)
				|| !LOOKUP(SleepConditionVariableCS)
				|| !LOOKUP(WakeAllConditionVariable))
			{
				InitializeConditionVariable_ = DummyConditionVariable;
				SleepConditionVariableCS_ = SleepConditionVariableFallback;
				WakeAllConditionVariable_ = DummyConditionVariable;
			}
# endif
			vlc_wait_addr_init();
			WaitOnAddress_ = WaitOnAddressFallback;
			WakeByAddressAll_ = WakeByAddressFallback;
			WakeByAddressSingle_ = WakeByAddressFallback;
		}
#endif
		thread_key = TlsAlloc();
		if (unlikely(thread_key == TLS_OUT_OF_INDEXES))
			return FALSE;
		InitializeCriticalSection(&setup_lock);
		InitializeCriticalSection(&super_mutex);
		InitializeConditionVariable(&super_variable);
		mxRWLockInit(&config_lock);
		mxCpuInit();
		break;
	}

	case DLL_PROCESS_DETACH:
		mxRWLockDestroy(&config_lock);
		DeleteCriticalSection(&super_mutex);
		DeleteCriticalSection(&setup_lock);
		TlsFree(thread_key);
#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
		if (WaitOnAddress_ == WaitOnAddressFallback)
			vlc_wait_addr_deinit();
#endif
		break;

	case DLL_THREAD_DETACH:
		mxThreadvarsCleanup();
		break;
	}
	return TRUE;
}
