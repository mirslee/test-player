#ifndef __BAY_VXCONFIG_H__
#define __BAY_VXCONFIG_H__

#include "vxobject.h"
#include "vxsysdef.h"
#include "pthread.h"

#ifdef _WIN32
#include "intrin.h"
#ifdef _VXEXT
#define VX_EXT_API __declspec(dllexport)
#else
#define VX_EXT_API __declspec(dllimport)
#endif
#pragma warning(disable: 4275)  // deriving exported class from non-exported
#pragma warning(disable: 4251)  // using non-exported as public in exported
#pragma warning(disable: 4996)  // 编译器遇到了标记有 deprecated 的函数。 在未来版本中可能不再支持此函数。
#else
#define VX_EXT_API
#endif

VX_EXT_API void* __cdecl _vxcalloc(size_t _NumOfElements,size_t _SizeOfElements,int _Alignment = 16);
VX_EXT_API void* __cdecl _vxmalloc(size_t _Size,int _Alignment = 16);
VX_EXT_API void* __cdecl _vxmallocz(size_t _Size,int _Alignment = 16);
VX_EXT_API void* __cdecl _vxrealloc(void * _Memory,size_t _Size,int _Alignment = 16);
VX_EXT_API void  __cdecl _vxfree(void * memblock);
VX_EXT_API void  __cdecl _vxfreep(void ** memblock);
VX_EXT_API void  __cdecl _vxmemcheck(void * memblock);

VX_EXT_API vxuintptr __cdecl _vxRefFile(const char* file);
VX_EXT_API void  __cdecl _vxUnrefFile(vxuintptr fid);
VX_EXT_API void  __cdecl _vxRefObject(vxuintptr fid,IVxObject* vxObj);
VX_EXT_API LONG  __cdecl _vxGetRefObject(vxuintptr fid,IVxObject** ppvxobj);
VX_EXT_API LONG  __cdecl _vxFindRefObject(const char* file,IVxObject** ppvxobj);
VX_EXT_API vxuintptr __cdecl _vxFindRef(const char* file);

VX_EXT_API void  __cdecl _vxSetThreadName(const char* lpstr);
VX_EXT_API const char* __cdecl _vxGetThreadName();
VX_EXT_API DWORD  __cdecl _vxGetCurrentThreadId();
VX_EXT_API void  __cdecl _vxSetGlobalProperty(const char* key, const char* value);
VX_EXT_API const char* __cdecl _vxGetGlobalProperty( const char* key );

#ifdef _WIN32
VX_EXT_API void __cdecl _vxSaveContext(const char* filename,PEXCEPTION_POINTERS ExceptionInfo,int extlen,BYTE* extptr,char info[512]);
VX_EXT_API void __cdecl _vxDestroyThreadTLS();
//#define USE_UTF8PATH
#endif

VX_EXT_API HVXFILE __cdecl _vxCanUnbuffer(const char* lpstr,VXBOOL bWrite);
VX_EXT_API HVXFILE __cdecl _vxCanUnbufferW(const unsigned short* lpstr, VXBOOL bWrite);
VX_EXT_API DWORD __cdecl _vxGetSectorSizeForFileName(const char* lpFileName);
VX_EXT_API DWORD __cdecl _vxGetSectorSizeForFileNameW(const unsigned short* lpFileName);

VX_EXT_API DWORD vxGetSysLastError();
VX_EXT_API DWORD vxGetLastError();
VX_EXT_API void vxSetLastError(DWORD);

#ifdef WIN32
#define INITPTHREAD(t) memset(&t,0,sizeof(t))
#define PTHREADISVALID(t) (t.p!=NULL)
#else	
#define INITPTHREAD(t) t = NULL;
#define PTHREADISVALID(t) (t!=NULL)
#endif


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef _WIN32
#if __linux__
#include <stddef.h>
#define _alloca alloca
#else
#define _alloca __alloca
#endif
#define PRId64 "lld"
#define PRIx64 "llx"
#define PRIu64 "llu"
#else
#define PRId64 "I64d"
#define PRIx64 "I64x"
#define PRIu64 "I64u"
#endif


#define VXTRY				try{
#define VXEND_TRY			}catch(...){}
#define VXCATCH_ALL(e)		}catch(CVxException* e){
#define VXEND_CATCH_ALL		}

VX_EXT_API				VXBOOL	vxAssert( const char* code,const char* file,UINT line );
VX_EXT_API				void	vxTrace(const char* lpszFormat, ...);

#if !defined(_WIN32)
static inline void __debugbreak()
{
asm ("int $3") ; 
// __asm int 3;
}
#endif

//#define _TRACEASSERT
#if (!defined  _DEBUG) && (defined _TRACEASSERT)

#ifdef ASSERT
#undef ASSERT
#endif//!ASSERT
#ifdef __i386__
#define ASSERT(f)		do{	if (!(f) ){	if(!vxAssert( #f,__FILE__,__LINE__ )) __debugbreak();} }while (0)
#else
#define ASSERT(f)		do{	if (!(f) ){	if(!vxAssert( #f,__FILE__,__LINE__ )) __debugbreak();} }while (0)
#endif
#ifdef VERIFY
#undef VERIFY
#endif//!VERIFY
#define VERIFY( f )		ASSERT( f )

#else

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(f)		do{	if (!(f) ){	if(!vxAssert( #f,__FILE__,__LINE__ )) __debugbreak();} }while (0)
#define VERIFY( f )		ASSERT( f )
#else
#define	ASSERT(f)		((void)0)
#define VERIFY( f )		(f)
#endif
#endif

#endif//(!defined  _DEBUG) && (defined _TRACEASSERT)

#ifndef TRACE
#ifdef _DEBUG
#define TRACE	vxTrace
#define TRACE0(sz)              TRACE("%s",sz)
#else
#define	TRACE
#define TRACE0(sz)
#endif
#endif

#if defined(_DEBUG)&&defined(VXDEBUGNEW)

void* TraceAlloc(size_t nSize);
void TraceDealloc(void* poMem);

inline void* __cdecl operator new(size_t nSize,const char* lpszFileName,int nLine)
{
	return TraceAlloc(nSize);
}

inline void* __cdecl operator new[](size_t nSize,const char* lpszFileName,int nLine)
{
	return TraceAlloc(nSize);
}

inline void __cdecl operator delete(void* pData,const char*  lpszFileName,int nLine)
{
	TraceDealloc(pData);
}

inline void __cdecl operator delete[](void* pData,const char* lpszFileName,int nLine)
{
	TraceDealloc(pData);
}

inline void* __cdecl operator new[](size_t nSize)
{
	return TraceAlloc(nSize);
}
inline void __cdecl operator delete[](void* p)
{
	TraceDealloc(p);
}

inline void* operator new(size_t s)
{
	return TraceAlloc(s);
}

inline void __cdecl operator delete(void* p)
{
	TraceDealloc(p);
}
#elif __linux__

#if 0
#include <libaio.h>
#define aio_context_t io_context_t
static inline void io_set_userdata(struct iocb *iocb, void* userdata)
{
    iocb->data = userdata;
}
static inline long long io_get_offset(struct iocb *iocb)
{
    return iocb->u.c.offset;
}

static inline unsigned long io_get_bytes(struct iocb *iocb)
{
    return iocb->u.c.nbytes;
}
#else
#include <sys/syscall.h>	// for __NR_* definitions
#include <linux/aio_abi.h>  // for AIO types and constants
static inline int io_setup(unsigned nr, aio_context_t *ctxp)
{
	return syscall(__NR_io_setup, nr, ctxp);
}
static inline int io_destroy(aio_context_t ctx)
{
	return syscall(__NR_io_destroy, ctx);
}

static inline int io_cancel(aio_context_t ctx, struct iocb *iocb, struct io_event *evt)
{
    return syscall(__NR_io_cancel, ctx, iocb, evt);
}

static inline int io_submit(aio_context_t ctx, long nr,  struct iocb **iocbpp)
{
	return syscall(__NR_io_submit, ctx, nr, iocbpp);
}
 
static inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
		struct io_event *events, struct timespec *timeout)
{
	return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

static inline void io_prep_pread(struct iocb *iocb, int fd, void *buf, size_t count, long long offset)
{
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PREAD;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = (vxuintptr)buf;
    iocb->aio_nbytes = count;
    iocb->aio_offset = offset;
}

static inline void io_prep_pwrite(struct iocb *iocb, int fd, void *buf, size_t count, long long offset)
{
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PWRITE;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = (vxuintptr)buf;
    iocb->aio_nbytes = count;
    iocb->aio_offset = offset;
}
static inline void io_set_userdata(struct iocb *iocb, void* userdata)
{
    iocb->aio_data = (vxuintptr)userdata;
}

static inline __s64 io_get_offset(struct iocb *iocb)
{
    return iocb->aio_offset;
}

static inline __u64 io_get_bytes(struct iocb *iocb)
{
    return iocb->aio_nbytes;
}


#endif
#endif

#endif//__BAY_VXCONFIG_H__
