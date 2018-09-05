// VxSysEngine.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "vxsysdef.h"
#include "vxmemory.h"
#include "vxstring.h"
#include "vxsync.h"
#include "vxerror.h"
#include "vxcarrier.h"
#include "vxfilename.h"
#include <map>
#include <string>

#if __linux__
#include <sys/syscall.h>
#include <sys/resource.h>
#define gettid() syscall(__NR_gettid)
#endif

void _ReleaseLimit();
class MainInit
{
public:
    MainInit();
};
MainInit::MainInit()
{
    _ReleaseLimit();
}
void _ReleaseLimit()
{
#ifdef _WIN32
    int curMaxFiles = _getmaxstdio();
    _setmaxstdio(2048);
    curMaxFiles = _getmaxstdio();
#else
    rlimit fileLimit;
    if (0 != getrlimit(RLIMIT_NOFILE, &fileLimit))
    {
        return;
    }
    fileLimit.rlim_cur = 30000;
    if (0 != setrlimit(RLIMIT_NOFILE, &fileLimit))
    {
        fileLimit.rlim_cur = 10240;
        setrlimit(RLIMIT_NOFILE, &fileLimit);
    }	
#endif
}
MainInit _mainInit;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

extern BYTE* g_pDiskTest;


extern VXTHREAD_MUTEX g_udfcs;
#ifdef _DEBUG
extern VXTHREAD_MUTEX g_csobj;
#endif
pthread_key_t pt_ThreadName = NULL;
pthread_key_t pt_mempool = NULL;

void DestroyThreadName(void* p);
void DestroyMemPool(void* p);

VXTHREAD_MUTEX	_appResourceLock[3];

VX_EXT_API void vxAppLock(int nLockType)
{
    ASSERT( nLockType >= 0 && nLockType < __countof(_appResourceLock) );
    VXTHREAD_MUTEX_LOCK( &_appResourceLock[nLockType] );
}

VX_EXT_API void vxAppUnlock(int nLockType)
{
    ASSERT( nLockType >= 0 && nLockType < __countof(_appResourceLock) );
    VXTHREAD_MUTEX_UNLOCK( &_appResourceLock[nLockType] );
}


#ifdef _WIN32
HINSTANCE g_hInst = NULL;

#define P2FS_P2CMGRDLL		"p2cmgr.dll"
extern HMODULE		g_p2cmgr;
extern P2CINIT		P2CInit;
extern P2CLOGSENSE	P2CLogSense;
extern P2CFINI		P2CFini;

#include "FileHash.h"
extern _FileHash g_filehash;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	int result = PTW32_TRUE;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE("加载===系统基础===模块!\n");

#ifdef CHECKLEAK
		VLDEnable();
#endif
		g_hInst = hInstance;
		g_p2cmgr = LoadLibrary(P2FS_P2CMGRDLL);
		if(g_p2cmgr)
		{
			P2CInit			= (P2CINIT)GetProcAddress( g_p2cmgr,"DDP2CInit");
			P2CLogSense		= (P2CLOGSENSE)GetProcAddress( g_p2cmgr,"DDP2CLogSense");
			P2CFini			= (P2CFINI)GetProcAddress( g_p2cmgr,"DDP2CFini");
		}

		g_pDiskTest = (BYTE*)VirtualAlloc(NULL,0x100000,MEM_COMMIT,PAGE_READWRITE);

		result = pthread_win32_process_attach_np ();
		pthread_key_create (&pt_ThreadName,DestroyThreadName);
		pthread_key_create (&pt_mempool,DestroyMemPool);

#ifdef _DEBUG
		VXTHREAD_MUTEX_INIT(&g_csobj);
#endif
		VXTHREAD_MUTEX_INIT(&g_udfcs);

        for( int i = 0; i < __countof(_appResourceLock); i++ )
        {
            VXTHREAD_MUTEX_INIT(&_appResourceLock[i]);
        }

		//加载这个DLL的不一定是主线程,这里也太早了，本线程先不设置名字
		//_vxSetThreadName("主线程");
	}
	else if (dwReason == DLL_THREAD_ATTACH)
	{
		result = pthread_win32_thread_attach_np ();
	}
	else if (dwReason == DLL_THREAD_DETACH)
	{
		result = pthread_win32_thread_detach_np ();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
        for( int i = 0; i < __countof(_appResourceLock); i++ )
        {
            VXTHREAD_MUTEX_DESTROY(&_appResourceLock[i]);
        }


		VXTHREAD_MUTEX_DESTROY(&g_udfcs);
#ifdef _DEBUG
		VXTHREAD_MUTEX_DESTROY(&g_csobj);
#endif

		(void) pthread_win32_thread_detach_np ();
		pthread_key_delete(pt_mempool);
		pthread_key_delete(pt_ThreadName);

		result = pthread_win32_process_detach_np ();

		VirtualFree(g_pDiskTest,0,MEM_RELEASE);

		if(g_p2cmgr) FreeLibrary(g_p2cmgr);
		g_filehash.Empty();
		TRACE("卸载===系统基础===模块!\n");
	}
	return TRUE;
}

void* CreateCocoaPool(){return NULL;}
void DestroyCooalPool(void* pool){}

#else

// Initializer.
__attribute__((constructor))
static void initializer(void) 
{                             // 2
	pthread_key_create (&pt_ThreadName,DestroyThreadName);
	pthread_key_create (&pt_mempool,DestroyMemPool);

#ifdef _DEBUG
	VXTHREAD_MUTEX_INIT(&g_csobj);
#endif
	VXTHREAD_MUTEX_INIT(&g_udfcs);

    for( int i = 0; i < __countof(_appResourceLock); i++ )
    {
        VXTHREAD_MUTEX_INIT(&_appResourceLock[i]);
    }

	//加载这个DLL的不一定是主线程,这里也太早了，本线程先不设置名字
	//_vxSetThreadName("主线程");
}

#ifdef __APPLE__
extern IOCompletionUPP g_ioWCompletion;
#endif
// Finalizer.

__attribute__((destructor))
static void finalizer(void) 
{                     
#if __APPLE__&&!defined(USE_SYNCWRITE)
    DisposeIOCompletionUPP(g_ioWCompletion);
#endif
    for( int i = 0; i < __countof(_appResourceLock); i++ )
    {
        VXTHREAD_MUTEX_DESTROY(&_appResourceLock[i]);
    }

	VXTHREAD_MUTEX_DESTROY(&g_udfcs);
#ifdef _DEBUG
	VXTHREAD_MUTEX_DESTROY(&g_csobj);
#endif
	pthread_key_delete(pt_mempool);
	pthread_key_delete(pt_ThreadName);
   TRACE("VxSystem finalizer()\n");
}

void* CreateCocoaPool();
void DestroyCooalPool(void* pool);
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif

#pragma warning( disable : 4996 )


class CThreadName
{
public:
	CThreadName()
	{
#ifdef CHECKLEAK
		VLDEnable();
#endif
#ifdef __APPLE__
        m_autopool = CreateCocoaPool();
#endif
		m_name[0] = 0;
		m_id = 0;
		m_err = 0;
		m_errstr = NULL;
	}

	~CThreadName()
	{
#ifdef _WIN32
        vxTrace("线程[%s][0x%lx(%d)]退出----------------",m_name,m_id,m_id);
#else
        vxTrace("线程[%s][0x%lx(%d)]退出----------------\n",m_name,m_id,m_id);
#endif
		if( m_errstr )
		{
			delete m_errstr;
			m_errstr = NULL;
		}
        
#ifdef __APPLE__
        DestroyCooalPool(m_autopool);
#endif
        
#ifdef CHECKLEAK
		VLDDisable();
#endif
	}

	char	m_name[64];
#ifdef _WIN32
	DWORD	m_id;
#elif defined(__APPLE__)
	ThreadID m_id;
    void*	m_autopool;
#else
    int m_id;
#endif
	DWORD	m_err;
	char*	m_errstr;


	void	SetName( const char* name )
	{
		if(vxstrlen(name)>31) ((char*)name)[31] = '\0';
		strcpy(m_name,name);
#ifdef _WIN32
		m_id = GetCurrentThreadId();
#elif defined(__APPLE__)
        __uint64 tid = 0;
        pthread_t self = pthread_self();
        pthread_threadid_np(self,&tid);
        m_id = tid;
        char szself[256] = {0};
        sprintf(szself," (0x%llx)",(vxuintptr)self);
        strcat(m_name,szself);
        char szutf8[256] = {0};
        gbk2utf8(m_name,szutf8,256);

        pthread_setname_np(szutf8);
#else
        m_id = gettid();
        char szutf8[256] = {0};
        gbk2utf8(m_name,szutf8,256);
//        pthread_setname_np(pthread_self(),szutf8);
#endif
        vxTrace("线程[%s][0x%04lx(%d)]开始\n",m_name,m_id,m_id);
	}
	const char*	GetName()
	{
		return m_name;
	}

	DWORD GetId(){return m_id;}

	void	SetError( DWORD error )
	{
		m_err = error;
	}

	const DWORD	GetError()
	{
		return m_err;
	}

	void	SetErrorStr( const char* err )
	{
		if( err &&  err[0] )
		{
			if( m_errstr == NULL )
				m_errstr = new char[64];

			strncpy( m_errstr,err,63 );
			m_errstr[63] = 0;
		}
		else
		{
			if( m_errstr )
			{
				m_errstr[0] = 0;
			}
		}
	}

	const char*	GetErrorStr()
	{
		return m_errstr ? m_errstr : (const char*)&m_errstr;
	}
};

void DestroyThreadName(void* p)
{
	CThreadName* threadname = (CThreadName*)p;
	delete threadname;
}

VX_EXT_API void __cdecl _vxSetThreadName(const char* lpstr)
{
	const char* enabledProperty = _vxGetGlobalProperty("ThreadNameEnabled");
	if( enabledProperty != NULL && !strcmp(enabledProperty,"Disabled") )
		return;

//#ifdef _WIN32
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	if(!threadname) 
	{
		threadname = new CThreadName;
		pthread_setspecific(pt_ThreadName,threadname);
	}
	threadname->SetName(lpstr);
//#endif	
}

VX_EXT_API const char* __cdecl _vxGetThreadName()
{
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	return threadname?threadname->GetName():0;
}

VX_EXT_API DWORD  __cdecl _vxGetCurrentThreadId()
{
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	return threadname?threadname->GetId():0;
}

std::map<std::string, std::string> g_GlobalPropertyMap;

VX_EXT_API void  __cdecl _vxSetGlobalProperty( const char* key, const char* value )
{
	vxAppLock(CRIT_ALLOC);
	g_GlobalPropertyMap[key] = value;
	vxAppUnlock(CRIT_ALLOC);
}



VX_EXT_API const char* __cdecl _vxGetGlobalProperty(const char* key)
{
	vxAppLock(CRIT_ALLOC);
	const char* property = NULL;
	std::map<std::string, std::string>::iterator it = g_GlobalPropertyMap.find(key);
	if (it != g_GlobalPropertyMap.end())
		property = it->second.c_str();
	vxAppUnlock(CRIT_ALLOC);
	return property;
}

VX_EXT_API DWORD vxGetSysLastError()
{
#ifdef _WIN32
	DWORD err = GetLastError(); // HRESULT_FROM_WIN32(GetLastError());
	return err;
#else
	DWORD err = errno;
	return err;
#endif
}

VX_EXT_API DWORD vxGetLastError()
{
	DWORD err = 0;
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	if(threadname)
		err = threadname->GetError();
	return err;
}

VX_EXT_API void vxSetLastError(DWORD error)
{
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	if(threadname) 
		threadname->SetError(error);
}

VX_EXT_API void vxSetLastErrorStr( const char* err )
{
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	if(threadname) threadname->SetErrorStr(err);
}

VX_EXT_API const char* vxGetLastErrorStr()
{
	CThreadName* threadname = (CThreadName*)pthread_getspecific(pt_ThreadName);
	return threadname?threadname->GetErrorStr():0;
}

VX_EXT_API	VXBOOL vxAssert( const char* code,const char* file,UINT line )
{
	if( vxGetAppType() & VXAPP_DEBUG )
	{
		char msg[64];
		vxString name;
		vxString vxsfile = file;
		vxFileName::SplitPath( vxsfile,NULL,&name,NULL);
		sprintf( msg,"%s[%d]",name.c_str(),line );

		int len = (int)strlen( msg );
		int morecopy = (int)strlen( code );
		if( len < 63 )
		{		
			strncpy( msg+len,code,63-len );
			msg[63]		= 0;
			morecopy	-= 63-len;
			code		+= 63-len;
		}
		vxSetLastErrorStr( msg );	
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VX_EXT_API void vxTrace(const char* format, ...)
{
	va_list args;
#define MAXREPORTMESSAGESIZE 2048
    char    message [MAXREPORTMESSAGESIZE] = {0};

	va_start(args, format);
	vsnprintf(message, MAXREPORTMESSAGESIZE, format, args);
	va_end(args);
	int len = (int)strlen(message);
	if (message[len - 2] == '\n'&&message[len - 1] == '\n')
		message[len - 1] = 0;
#ifndef _WIN32	
    char utf8[MAXREPORTMESSAGESIZE] = {0};
	gbk2utf8(message,utf8,MAXREPORTMESSAGESIZE);
    fwrite(utf8,strlen(utf8),1,stdout);
#else	
	OutputDebugString(message);
#endif	
}

#define nvxAlignGapSize sizeof(void *)
#define nvxNoMansLandSize 4

typedef struct _myAlignMemBlockHdr
{
	void *pHead;
	size_t size;
	size_t valids;
	int type;
	unsigned char Gap[nvxAlignGapSize];
} _MyAlignMemBlockHdr;

#define IS_2_POW_N(x)   (((x)&(x-1)) == 0)


static unsigned char _bvxAlignLandFill  = 0xED;   /* fill no-man's land for aligned routines */

static int __cdecl CheckBytes(unsigned char * pb,unsigned char bCheck,size_t nSize)
{
	int bOkay = TRUE;
	while (nSize--)
	{
		if (*pb++ != bCheck)
		{
			bOkay = FALSE;
		}
	}
	return bOkay;
}

#if defined(_DEBUG) && defined(_WIN32)
void* TraceAlloc(size_t nSize);
void TraceDealloc(void* poMem);
#define malloc TraceAlloc
#define free TraceDealloc
#endif

#if !defined(_DEBUG)&&!__linux__
#define USE_VM
#endif

#define VM_PAGESIZE	4096
#define MEM_PADS	16
#ifdef __APPLE__
#include <mach/mach.h>
#endif

#define MAX_STACK_DEPTH	25

#ifdef _WIN32

BOOL GetFunctionInfoFromAddresses( DWORD64 fnAddress, DWORD64 stackAddress, LPTSTR lpszSymbol, rsize_t lpszSymbolSize);
BOOL GetSourceInfoFromAddress( DWORD64 address, LPTSTR lpszSourceInfo, rsize_t lpszSourceInfoSize);

// Define number of call stack levels to ignore (usually 2, TraceAlloc and operator new)
#define NUM_LEVELS_TO_IGNORE 2
USHORT (WINAPI *backtrace)(ULONG, ULONG, PVOID*, PULONG) = 0;  

void vxDumpStack(const char* dumpname)
{
	if (backtrace == 0) {  
		const HMODULE hNtDll = ::GetModuleHandle("ntdll.dll");  
		reinterpret_cast<void*&>(backtrace) = ::GetProcAddress(hNtDll, "RtlCaptureStackBackTrace");  
	}
	void* stacktrace[MAX_STACK_DEPTH+1];
	int capcount = backtrace(NUM_LEVELS_TO_IGNORE, MAX_STACK_DEPTH, stacktrace, NULL);
	for(int i=0;i<capcount;i++)
	{
		char symInfo[512] = ("?");
		char srcInfo[512] = ("?");
		GetFunctionInfoFromAddresses((DWORD64)stacktrace[i],(DWORD64)stacktrace[i],symInfo,sizeof(symInfo) );
		GetSourceInfoFromAddress( (DWORD64)stacktrace[i],srcInfo,sizeof(srcInfo));
		vxTrace("  %s : %s\n",srcInfo,symInfo);
	}
}
#else
#include <execinfo.h>
#include <cxxabi.h>

/** Print a demangled stack backtrace of the caller function to FILE* out. */
void vxDumpStack(const char* dumpname)
{
	vxTrace("%s stack trace:\n",dumpname);

	void* addrlist[MAX_STACK_DEPTH+1] = {0};
	int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

	if (addrlen == 0) 
	{
		vxTrace("  <empty, possibly corrupt>\n");
		return;
	}

	// resolve addresses into strings containing "filename(function+address)",
	// this array must be free()-ed
	char** symbollist = backtrace_symbols(addrlist, addrlen);

	// allocate string which will be filled with the demangled function name
	size_t funcnamesize = 256;
	char* funcname = (char*)malloc(funcnamesize);
	for (int i = 1; i < addrlen; i++)
	{
		char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p)
		{
			if (*p == '(')
				begin_name = p;
			else if (*p == '+')
				begin_offset = p;
			else if (*p == ')' && begin_offset) {
				end_offset = p;
				break;
			}
		}

		if (begin_name&&begin_offset&& end_offset&& begin_name < begin_offset)
		{
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status;
			char* ret = abi::__cxa_demangle(begin_name,funcname, &funcnamesize, &status);
			if (status == 0) 
				vxTrace("  %s : %s+%s\n",symbollist[i], ret, begin_offset);
			else 
				vxTrace("  %s : %s()+%s\n",symbollist[i], begin_name, begin_offset);
		}
		else
		{
			// couldn't parse the line? print the whole line.
			vxTrace("  %s\n", symbollist[i]);
		}
	}
	free(funcname);
	free(symbollist);
}

#endif

#define CHECKALLOC      1
#if CHECKALLOC
void CheckMemory(_MyAlignMemBlockHdr *pHdr,void * memblock)
{
	if(!CheckBytes(pHdr->Gap, _bvxAlignLandFill, nvxAlignGapSize))
	{
		// We don't know where (file, linenum) memblock was allocated
		vxTrace("Damage before 0x%p which was allocated by aligned routine(begin)\n", memblock);
#ifdef _DEBUG
		ASSERT(FALSE);
#else
		vxDumpStack("_vxfree");
#endif
	}

	if(!CheckBytes((BYTE*)memblock+pHdr->valids, _bvxAlignLandFill, nvxNoMansLandSize))
	{
		// We don't know where (file, linenum) memblock was allocated
		vxTrace("Damage before 0x%p which was allocated by aligned routine(end)\n", memblock);
#ifdef _DEBUG
		ASSERT(FALSE);
#else
		vxDumpStack("_vxfree");
#endif
	}
}
#else
#define CheckMemory
#endif

VX_EXT_API void* __cdecl _vxmalloc(size_t _Size,int align)
{
	uintptr_t ptr = 0, r_ptr = 0;
	_MyAlignMemBlockHdr *pHdr;
	if(!IS_2_POW_N(align)) align = 16;
	int alignptr = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) -1;
	int extsize = alignptr+1+sizeof(_MyAlignMemBlockHdr)+MEM_PADS;
#ifdef USE_VM
	if(_Size>=0x80000)
	{
		size_t reals = (_Size+extsize+VM_PAGESIZE-1)&~(VM_PAGESIZE-1);
#ifdef _WIN32
		ptr = (uintptr_t)VirtualAlloc(NULL,reals,MEM_COMMIT,PAGE_READWRITE);
		if(!ptr) return NULL;
#else
		kern_return_t err = vm_allocate((vm_map_t) mach_task_self(),(vm_address_t *)&ptr,reals,VM_FLAGS_ANYWHERE );
		if(err!=KERN_SUCCESS) return NULL;
#endif//_WIN32
		r_ptr =(ptr+alignptr+sizeof(_MyAlignMemBlockHdr))&~alignptr;
		pHdr = (_MyAlignMemBlockHdr *)(r_ptr) -1;
		pHdr->size = reals;
		pHdr->type = 1;
	}
	else
	{
#endif//USE_VM
		size_t reals = _Size+extsize;
		if ((ptr = (uintptr_t) malloc(reals)) == (uintptr_t)NULL)
			return NULL;
		r_ptr =(ptr+alignptr+sizeof(_MyAlignMemBlockHdr))&~alignptr;
		pHdr = (_MyAlignMemBlockHdr *)(r_ptr) -1;
		pHdr->size = reals;
		pHdr->type = 0;
#ifdef USE_VM	
	}
#endif//USE_VM

#if CHECKALLOC
	memset((void*)pHdr->Gap, _bvxAlignLandFill,nvxAlignGapSize);
	memset((void*)(r_ptr+_Size), _bvxAlignLandFill,nvxNoMansLandSize);
#endif
	pHdr->pHead = (void *)ptr;
	pHdr->valids = _Size;
	return (void *) r_ptr;
}

VX_EXT_API void   __cdecl _vxfree(void * memblock)
{
	if(memblock==NULL) return;
	_MyAlignMemBlockHdr *pHdr = (_MyAlignMemBlockHdr*)memblock -1;
	CheckMemory(pHdr,memblock);
#ifdef USE_VM
	if(pHdr->type==1)
	{
#ifdef _WIN32
		VirtualFree(pHdr->pHead,0,MEM_RELEASE);
#else
		vm_deallocate((vm_map_t) mach_task_self(),(vm_address_t)pHdr->pHead,pHdr->size);
#endif
	}
	else
#endif	
		free((void *)pHdr->pHead);
}

VX_EXT_API void  __cdecl _vxfreep(void ** memblock)
{
    if (memblock && *memblock)
    {
        _vxfree(*memblock);
        *memblock = NULL;
    }
}

VX_EXT_API int   __cdecl _vxmemsize(void * memblock)
{
	_MyAlignMemBlockHdr *s_pHdr = (_MyAlignMemBlockHdr*)memblock -1;
	return (int)s_pHdr->valids;
}

VX_EXT_API void  __cdecl _vxmemcheck(void * memblock)
{
	if(memblock==NULL) return;
	_MyAlignMemBlockHdr *pHdr = (_MyAlignMemBlockHdr*)memblock -1;
	CheckMemory(pHdr,memblock);
}

VX_EXT_API void* __cdecl _vxmallocz(size_t _Size,int align)
{
	void* p = _vxmalloc(_Size,align);
	memset(p,0,_Size);
	return p;
}

VX_EXT_API void* __cdecl _vxrealloc(void * memblock,size_t size,int align)
{
	if (memblock == NULL)
		return _vxmallocz(size, align);

	_MyAlignMemBlockHdr *s_pHdr = (_MyAlignMemBlockHdr*)memblock -1;
	CheckMemory(s_pHdr,memblock);
	if(s_pHdr->valids>=size) return memblock;

	void* newmemblock = _vxmalloc(size,align);
	if(newmemblock)
	{
		memcpy(newmemblock, memblock,s_pHdr->valids);
		memset(((BYTE*)newmemblock)+s_pHdr->valids,0,size-s_pHdr->valids);
	}
	_vxfree(memblock);
	return newmemblock;
}


VX_EXT_API void* __cdecl _vxcalloc(size_t _NumOfElements,size_t _SizeOfElements,int align)
{
	size_t _Size = _NumOfElements*_SizeOfElements;
	return _vxmallocz(_Size,align);
}



class CVxMemory : public IVxMemory
{
public:
	void* lnew( LONG size )
	{
		if( size <= 0 )
			return NULL;
		else
		{
			if( m_pHead == NULL )
				Alloc();
#ifdef _DEBUG//	_DEBUG，尾巴后面加上检查信息
			size = (((size+8)+0xF)&(~0xF))-8;		//	保证m_allocsize总是16字节的整数
			LPBYTE pData = m_pHead + m_allocsize;
			if( m_pHead == NULL || pData + (size+8) >= m_pTail )
			{
				return (BYTE*)_vxmalloc(size+8);
			}
			else
			{
				LPDWORD pdatafoot	= (LPDWORD)(pData + size);
				pdatafoot[0]		= 0xFDFDFDFD;
				pdatafoot[1]		= m_allocsize;
				m_allocsize	+= size + 8;
				return pData;
			}
#else
			size = (((size+4)+0xF)&(~0xF))-4;		//	保证m_allocsize总是16字节的整数
			LPBYTE pData = m_pHead + m_allocsize;
			if( m_pHead == NULL || pData + (size+4) >= m_pTail )
			{
				return (BYTE*)_vxmalloc(size+4);
			}
			else
			{
				LPDWORD pdatafoot = (LPDWORD)(pData + size);
				pdatafoot[0]		= m_allocsize;
				m_allocsize	+= size+4;
				return pData;
			}
#endif
		}
	}
	void	lfree( void*	ptr )
	{
		if( ptr > m_pTail || ptr < m_pHead )
			_vxfree(ptr);
		else if( ptr )
		{
			LPBYTE	pdatalast = m_pHead + m_allocsize;			
			LPDWORD pdatafoot = (LPDWORD)pdatalast;
#ifdef _DEBUG//	_DEBUG检查尾巴后面的信息
			pdatafoot -= 2;
			ASSERT( pdatafoot[0] == 0xFDFDFDFD );
			m_allocsize = pdatafoot[1];
			ASSERT( ptr ==  m_pHead + m_allocsize );
#else
			pdatafoot--;
			m_allocsize = pdatafoot[0];
#endif//!_DEBUG
		}
	}
	void* lnewAll( LONG* plSize )
	{
		if( m_pHead == NULL )
		{
			Alloc();
		}

		LPBYTE	pData	= m_pHead + m_allocsize;
		LONG	canAlloc = (LONG)(m_pTail - pData - 0x10000);	//	留下64K
		if( canAlloc > 0 )
		{
			*plSize = canAlloc;
			return lnew( canAlloc );
		}
		else
		{
			*plSize = 0;
			return NULL;
		}
	}

    CVxMemory(const char* tname,int tid)
	{
		m_allocsize	= 0;
		m_totalsize	= 0x200000;
		m_pHead		= NULL;
		m_pTail		= NULL;
		if(tname)
	        strncpy(m_tname,tname,64);
		else
			memset(m_tname,0,sizeof(m_tname));
        m_tid = tid;
	}

	virtual void stacksize( int len )
	{
		ASSERT( m_allocsize ==0 && m_pHead == NULL	);
		m_totalsize = len;

		ASSERT( len > 0 && len <= 0x800000 );	//	< 8M
	}

	void Alloc()
	{
		ASSERT( m_pHead == NULL );

		m_pHead		= (PBYTE)_vxmallocz(m_totalsize);		
		m_pTail		= m_pHead + (m_totalsize-16);
	}

	virtual ~CVxMemory()
	{
		ASSERT( m_allocsize == 0 );
		if( m_pHead )
		{
			_vxfree(m_pHead);
			m_pHead		= m_pTail = NULL;
		}
        vxTrace("线程[%s][%d]析构MEMORY临时堆\n",m_tname,m_tid);
	}
protected:
	BYTE*	m_pHead;
	BYTE*	m_pTail;
	LONG	m_allocsize;
	LONG	m_totalsize;
    char    m_tname[64];
    int     m_tid;
};

VX_EXT_API IVxMemory* GetIVxMemory()
{
	CVxMemory* cmemory = (CVxMemory*)pthread_getspecific(pt_mempool);
	if(!cmemory) 
	{
        cmemory = new CVxMemory(_vxGetThreadName(),_vxGetCurrentThreadId());
		pthread_setspecific(pt_mempool,cmemory);
	}
	return static_cast<IVxMemory*>(cmemory);
}

void DestroyMemPool(void* p)
{
	CVxMemory* cmemory = (CVxMemory*)p;
	delete cmemory;
}


extern "C"
{
VX_EXT_API VXBOOL g_bSyncRead = FALSE;
VX_EXT_API VXBOOL g_bSyncWrite = FALSE;
VX_EXT_API VXBOOL g_bRealtimeIDX = FALSE;
VX_EXT_API int g_rdblocksize = 0x100000;
VX_EXT_API int g_wrblocksize = 0x100000;
VX_EXT_API VXBOOL g_bUnbuffer = TRUE;
}

VX_EXT_API VXBOOL vxEnableRealtimeIDX(VXBOOL enableidx)
{
	VXBOOL ret = g_bRealtimeIDX;
	g_bRealtimeIDX = enableidx;
	return ret;
}


VX_EXT_API void __cdecl vxSetReadWriteBlockSize(int& rdblocksize, int& wrblocksize)
{
	int rdsize = (rdblocksize / 0x80000) * 0x80000;
	int wrsize = (wrblocksize / 0x80000) * 0x80000;
	if (rdsize < 0x80000) rdsize = 0x80000;
	else if (rdsize > 0x1000000) rdsize = 0x1000000;
	if (wrsize < 0x80000) wrsize = 0x80000;
	else if (wrsize > 0x1000000) wrsize = 0x1000000;
	rdblocksize = g_rdblocksize = rdsize;
	wrblocksize = g_wrblocksize = wrsize;
}

VX_EXT_API void __cdecl vxGetReadWriteBlockSize(int& rdblocksize, int& wrblocksize)
{
	rdblocksize = g_rdblocksize;
	wrblocksize = g_wrblocksize;
}

#ifdef _WIN32



#include "vxtempl.h"


vxinline char *SplitPathRoot(char *dst, const char *path) 
{
	if (!path)	return NULL;

	// C:

	if (isalpha(path[0]) && path[1]==':') {
		dst[0] = path[0];
		dst[1] = ':';
		dst[2] = '\\';
		dst[3] = 0;

		return dst;
	}

	// UNC path?

	if (path[0] == '\\' && path[1] == '\\') {
		const char *s = path+2;
		char *t = dst;

		*t++ = '\\';
		*t++ = '\\';

		while(*s && *s != '\\')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while(*s && *s != '\\')
			*t++ = *s++;

		*t++ = '\\';
		*t = 0;

		return dst;
	}

	if (path[0] == '/' && path[1] == '/') {
		const char *s = path+2;
		char *t = dst;

		*t++ = '/';
		*t++ = '/';

		while(*s && *s != '/')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while(*s && *s != '/')
			*t++ = *s++;

		*t++ = '/';
		*t = 0;

		return dst;
	}

	return NULL;
}

vxinline vxUWChar *SplitPathRootW(vxUWChar *dst, const vxUWChar *path)
{
	if (!path)	return NULL;

	// C:

	if (isalpha(path[0]) && path[1] == ':') {
		dst[0] = path[0];
		dst[1] = ':';
		dst[2] = '\\';
		dst[3] = 0;

		return dst;
	}

	// UNC path?

	if (path[0] == '\\' && path[1] == '\\') {
		const vxUWChar *s = path + 2;
		vxUWChar *t = dst;

		*t++ = '\\';
		*t++ = '\\';

		while (*s && *s != '\\')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while (*s && *s != '\\')
			*t++ = *s++;

		*t++ = '\\';
		*t = 0;

		return dst;
	}

	if (path[0] == '/' && path[1] == '/') {
		const vxUWChar *s = path + 2;
		vxUWChar *t = dst;

		*t++ = '/';
		*t++ = '/';

		while (*s && *s != '/')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while (*s && *s != '/')
			*t++ = *s++;

		*t++ = '/';
		*t = 0;

		return dst;
	}

	return NULL;
}

static struct DISKTEST
{
	DWORD dwFlags[2];
	VXBOOL unbuffer;
}g_diskinfo[64] = {0};

static int g_disks = 0;
BYTE* g_pDiskTest = NULL;

VX_EXT_API HVXFILE __cdecl _vxCanUnbuffer(const char* lpstr,VXBOOL bWrite)
{
	if(!g_bUnbuffer) return INVALID_HANDLE_VALUE;

	VXBOOL bFind = FALSE;
	char szRoot[MAX_VXPATH] = {0};
	if(!SplitPathRoot(szRoot,lpstr)) return INVALID_HANDLE_VALUE;
	DWORD* dwFlag = (DWORD*)szRoot;
	int i=0;
	for(;i<g_disks;i++)
	{
		if((dwFlag[0]==g_diskinfo[i].dwFlags[0])&&(dwFlag[1]==g_diskinfo[i].dwFlags[1]))
		{
			bFind = TRUE;
			break;
		}
	}
	if(bFind)
	{
		if(!g_diskinfo[i].unbuffer) return INVALID_HANDLE_VALUE;
		DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
		if(bWrite)
		{
			if(!g_bSyncWrite) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
		else
		{
			if(!g_bSyncRead) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
		return CreateFile(lpstr,bWrite?GENERIC_WRITE|GENERIC_READ:GENERIC_READ,bWrite?FILE_SHARE_READ:FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,bWrite?CREATE_ALWAYS:OPEN_EXISTING,dwOpenFlag,NULL);
	}
	DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
	if(bWrite)
	{
		if(!g_bSyncWrite) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
	}
	else
	{
		if(!g_bSyncRead) dwOpenFlag |= FILE_FLAG_OVERLAPPED;
	}
	HANDLE hUnbuffer = CreateFile(lpstr,bWrite?GENERIC_WRITE|GENERIC_READ:GENERIC_READ,bWrite?FILE_SHARE_READ:FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,bWrite?CREATE_ALWAYS:OPEN_EXISTING,dwOpenFlag,NULL);
	if(hUnbuffer==INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;
	DWORD dwActual = 0;
	g_diskinfo[g_disks].dwFlags[0] = dwFlag[0];
	g_diskinfo[g_disks].dwFlags[1] = dwFlag[1];

	if(bWrite)
	{
		if(g_bSyncWrite)
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual,NULL);
		else
		{
			OVERLAPPED overlap = {0};
			overlap.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual,&overlap);
			if(!g_diskinfo[g_disks].unbuffer)
			{
				if(GetLastError()==ERROR_IO_PENDING)
				{
					if(WaitForSingleObject(overlap.hEvent,5000)==WAIT_OBJECT_0)
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer,&overlap,&dwActual,TRUE);
					else
						CancelIo(hUnbuffer);
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if(g_diskinfo[g_disks++].unbuffer) 
		{
			LONG high = 0;
			SetFilePointer(hUnbuffer,0,&high,FILE_BEGIN);
			SetEndOfFile(hUnbuffer);
			return hUnbuffer;
		}
		else
			CloseHandle(hUnbuffer);
	}
	else
	{
		if(g_bSyncRead)
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual,NULL);
		else
		{
			OVERLAPPED overlap = {0};
			overlap.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual,&overlap);
			if(!g_diskinfo[g_disks].unbuffer)
			{
				if(GetLastError()==ERROR_IO_PENDING)
				{
					if(WaitForSingleObject(overlap.hEvent,5000)==WAIT_OBJECT_0)
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer,&overlap,&dwActual,TRUE);
					else
						CancelIo(hUnbuffer);
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if(GetLastError()!=ERROR_HANDLE_EOF)
		{
			if(g_diskinfo[g_disks++].unbuffer) return hUnbuffer;
			CloseHandle(hUnbuffer);
		}
		else
			CloseHandle(hUnbuffer);
	}


	return INVALID_HANDLE_VALUE;
}

VX_EXT_API HVXFILE __cdecl _vxCanUnbufferW(const unsigned short* lpstr, VXBOOL bWrite)
{
	if (!g_bUnbuffer)
	{
		return INVALID_HANDLE_VALUE;
	}

	VXBOOL bFind = FALSE;
	vxUWChar szRoot[MAX_VXPATH] = {0};
	if (!SplitPathRootW(szRoot, lpstr))
	{
		return INVALID_HANDLE_VALUE;
	}
	DWORD* dwFlag = (DWORD *)szRoot;
	int i = 0;
	for (; i < g_disks; i++)
	{
		if ((dwFlag[0] == g_diskinfo[i].dwFlags[0]) && (dwFlag[1] == g_diskinfo[i].dwFlags[1]))
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind)
	{
		if (!g_diskinfo[i].unbuffer)
		{
			return INVALID_HANDLE_VALUE;
		}
		DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
		if (bWrite)
		{
			if (!g_bSyncWrite)
			{
				dwOpenFlag |= FILE_FLAG_OVERLAPPED;
			}
		}
		else
		{
			if (!g_bSyncRead)
			{
				dwOpenFlag |= FILE_FLAG_OVERLAPPED;
			}
		}
		return CreateFileW((LPCWSTR)lpstr, bWrite ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ, bWrite ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, bWrite ? CREATE_ALWAYS : OPEN_EXISTING, dwOpenFlag, NULL);
	}
	DWORD dwOpenFlag = FILE_FLAG_NO_BUFFERING;
	if (bWrite)
	{
		if (!g_bSyncWrite)
		{
			dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
	}
	else
	{
		if (!g_bSyncRead)
		{
			dwOpenFlag |= FILE_FLAG_OVERLAPPED;
		}
	}
	HANDLE hUnbuffer = CreateFileW((LPCWSTR)lpstr, bWrite ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ, bWrite ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, bWrite ? CREATE_ALWAYS : OPEN_EXISTING, dwOpenFlag, NULL);
	if ((hUnbuffer == INVALID_HANDLE_VALUE) || (hUnbuffer == NULL))
	{
		return INVALID_HANDLE_VALUE;
	}
	DWORD dwActual = 0;
	g_diskinfo[g_disks].dwFlags[0] = dwFlag[0];
	g_diskinfo[g_disks].dwFlags[1] = dwFlag[1];

	if (bWrite)
	{
		if (g_bSyncWrite)
		{
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, NULL);
		}
		else
		{
			OVERLAPPED overlap = { 0 };
			overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			g_diskinfo[g_disks].unbuffer = WriteFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, &overlap);
			if (!g_diskinfo[g_disks].unbuffer)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(overlap.hEvent, 5000) == WAIT_OBJECT_0)
					{
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer, &overlap, &dwActual, TRUE);
					}
					else
					{
						CancelIo(hUnbuffer);
					}
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if (g_diskinfo[g_disks++].unbuffer)
		{
			LONG high = 0;
			SetFilePointer(hUnbuffer, 0, &high, FILE_BEGIN);
			SetEndOfFile(hUnbuffer);
			return hUnbuffer;
		}
		else
		{
			CloseHandle(hUnbuffer);
		}
	}
	else
	{
		if (g_bSyncRead)
		{
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, NULL);
		}
		else
		{
			OVERLAPPED overlap = { 0 };
			overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			g_diskinfo[g_disks].unbuffer = ReadFile(hUnbuffer, g_pDiskTest, 0x100000, &dwActual, &overlap);
			if (!g_diskinfo[g_disks].unbuffer)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(overlap.hEvent, 5000) == WAIT_OBJECT_0)
					{
						g_diskinfo[g_disks].unbuffer = GetOverlappedResult(hUnbuffer, &overlap, &dwActual, TRUE);
					}
					else
					{
						CancelIo(hUnbuffer);
					}
				}
				CloseHandle(overlap.hEvent);
			}
		}

		if (GetLastError() != ERROR_HANDLE_EOF)
		{
			if (g_diskinfo[g_disks++].unbuffer)
			{
				return hUnbuffer;
			}
			CloseHandle(hUnbuffer);
		}
		else
		{
			CloseHandle(hUnbuffer);
		}
	}

	return INVALID_HANDLE_VALUE;
}

VX_EXT_API DWORD __cdecl _vxGetSectorSizeForFileName(const char* lpFileName)
{
	char szRoot[MAX_PATH];
	DWORD dwSectorSize;
	if(SplitPathRoot(szRoot,lpFileName) == NULL)
	{
		return 512; // (DWORD) - 1;
	}
	DWORD dwSectorsPerCluster, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	if (0 == GetDiskFreeSpace(szRoot, &dwSectorsPerCluster, &dwSectorSize, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
	{
		return 512; // (DWORD) - 1;
	}
	return dwSectorSize;
}

VX_EXT_API DWORD __cdecl _vxGetSectorSizeForFileNameW(const unsigned short* lpFileName)
{
	vxUWChar szRoot[MAX_PATH];
	DWORD dwSectorSize;
	if (SplitPathRootW(szRoot, lpFileName) == NULL)
	{
		return 512; // (DWORD) - 1;
	}
	DWORD dwSectorsPerCluster, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	if (0 == GetDiskFreeSpaceW((LPCWSTR)szRoot, &dwSectorsPerCluster, &dwSectorSize, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
	{
		return 512; // (DWORD) - 1;
	}
	return dwSectorSize;
}

VX_EXT_API void __cdecl _vxDestroyThreadTLS()
{
	pthread_win32_thread_detach_np();
}

#else

#if __APPLE__
VX_EXT_API DWORD __cdecl _vxGetSectorSizeForFileName(const char* lpFileName)
{
	FSRef fileref = {0};
	if(FSPathMakeRef((UInt8*)lpFileName,&fileref,NULL)==noErr)
	{
		FSCatalogInfo	catalogInfo;
		if(FSGetCatalogInfo(&fileref,kFSCatInfoVolume,&catalogInfo,NULL,NULL,NULL)==noErr)
		{
			FSVolumeInfo vinfo;
			if(FSGetVolumeInfo(catalogInfo.volume,0,NULL,kFSVolInfoBlocks, &vinfo,NULL,NULL)==noErr)
				return vinfo.blockSize;
		}		
	}
	
	return 2048;
}

#else
VX_EXT_API DWORD __cdecl _vxGetSectorSizeForFileName(const char* lpFileName)
{
    struct stat finfo;
    stat( lpFileName, &finfo );
    return finfo.st_blksize;
}
#endif

typedef struct
	{
		BYTE idlength;          /* F1 - number of bytes in Image id field (F6) */
		BYTE maptype;           /* F2 - color map type */
		BYTE imagetype;         /* F3 - Image Type */
		/* F4 - ColorMap Specification */
		/*word maporigin;*/       /* F4.1 - First Entry Index : Index of the first color map entry */
		BYTE mo0, mo1;
		
		/*word maplength;*/       /* F4.2 - color map length */
		BYTE ml0, ml1;
		BYTE mapsize;           /* F4.3 - color map entry size (bits) */
		/* F5 - Image Specification */
		WORD xorg;              /* F5.1 - image x origin */
		WORD yorg;              /* F5.2 - image y origin */
		WORD width;             /* F5.3 - image width in pixels */
		WORD height;            /* F5.4 - image height in pixels */
		BYTE pixsize;           /* F5.5 - image pixel size in bits */
		BYTE ImageDescriptor;   /* F5.6 - 0..3 = attribute, 4,5 = origin */
		/* ... */               /* F6 - image ID */
		/* ... */               /* F7 - color map data */
	}TGAHEADER;

__declspec( dllexport ) 
VXBOOL WriteToTGA(const char* lpstr,PBYTE buf,int nWidth,int nHeight,int bit)
{
	HVXFILE hFile = (HVXFILE)open(lpstr, O_CREAT|O_WRONLY, 0666);
	
	TGAHEADER tga = {0};
	tga.imagetype = 2;
	tga.width = nWidth;
	tga.height = nHeight;
	tga.pixsize = bit;
	tga.ImageDescriptor = 0x10;
	
	write(hFile,&tga,sizeof(TGAHEADER));
	write(hFile,buf,nWidth*nHeight*bit/8);
	close(hFile);
	
	
	return TRUE;
}

__declspec( dllexport ) 
VXBOOL ReadFromTGA(const char* lpstr,PBYTE& buf,int& nWidth,int& nHeight,int& bit)
{
	HVXFILE hFile = (HVXFILE)open(lpstr, O_RDONLY, 0666);
	if(hFile)
	{
		TGAHEADER tga = {0};
		read(hFile,&tga,sizeof(TGAHEADER));
		nWidth = tga.width;nHeight = tga.height;
		bit = tga.pixsize;
		buf = (BYTE*)_vxmalloc(nWidth*nHeight*bit/8);
		read(hFile,buf,nWidth*nHeight*bit/8);
		close(hFile);
	}
	return TRUE;
}

#endif

const char* vxGetSysErrorString(DWORD aSysErrId)
{
	const char* errStr = NULL;
#ifdef _WIN32
	switch (aSysErrId)
	{
	case 0: errStr = "操作成功完成"; break;
	case 1: errStr = "功能错误"; break;
	case 2: errStr = "系统找不到指定的文件"; break;
	case 3: errStr = "系统找不到指定的路径"; break;
	case 4: errStr = "系统无法打开文件"; break;
	case 5: errStr = "拒绝访问"; break;
	case 6: errStr = "句柄无效"; break;
	case 7: errStr = "存储控制块被损坏"; break;
	case 8: errStr = "存储空间不足，无法处理此命令"; break;
	case 9: errStr = "存储控制块地址无效"; break;
	case 10: errStr = "环境错误"; break;
	case 11: errStr = "试图加载格式错误的程序"; break;
	case 12: errStr = "访问码无效"; break;
	case 13: errStr = "数据无效"; break;
	case 14: errStr = "存储器不足，无法完成此操作"; break;
	case 15: errStr = "系统找不到指定的驱动器"; break;
	case 16: errStr = "无法删除目录"; break;
	case 17: errStr = "系统无法将文件移到不同的驱动器"; break;
	case 18: errStr = "没有更多文件"; break;
	case 19: errStr = "介质受写入保护"; break;
	case 20: errStr = "系统找不到指定的设备"; break;
	case 21: errStr = "设备未就绪"; break;
	case 22: errStr = "设备不识别此命令"; break;
	case 23: errStr = "数据错误 (循环冗余检查)"; break;
	case 24: errStr = "程序发出命令，但命令长度不正确"; break;
	case 25: errStr = "驱动器无法找出磁盘上特定区域或磁道的位置"; break;
	case 26: errStr = "无法访问指定的磁盘或软盘"; break;
	case 27: errStr = "驱动器找不到请求的扇区"; break;
	case 28: errStr = "打印机缺纸"; break;
	case 29: errStr = "系统无法写入指定的设备"; break;
	case 30: errStr = "系统无法从指定的设备上读取"; break;
	case 31: errStr = "连到系统上的设备没有发挥作用"; break;
	case 32: errStr = "进程无法访问文件，因为另一个程序正在使用此文件"; break;
	case 33: errStr = "进程无法访问文件，因为另一个程序已锁定文件的一部分"; break;
	case 36: errStr = "用来共享的打开文件过多"; break;
	case 38: errStr = "到达文件结尾"; break;
	case 39: errStr = "磁盘已满"; break;
	case 50: errStr = "不支持该请求"; break;
	case 51: errStr = "远程计算机不可用 "; break;
	case 52: errStr = "在网络上已有重复的名称"; break;
	case 53: errStr = "找不到网络路径"; break;
	case 54: errStr = "网络忙"; break;
	case 55: errStr = "指定的网络资源或设备不再可用"; break;
	case 56: errStr = "已到达网络 BIOS 命令限制"; break;
	case 57: errStr = "网络适配器硬件出错"; break;
	case 58: errStr = "指定的服务器无法运行请求的操作"; break;
	case 59: errStr = "发生意外的网络错误"; break;
	case 60: errStr = "远程适配器不兼容"; break;
	case 61: errStr = "打印机队列已满"; break;
	case 62: errStr = "无法在服务器上获得用于保存待打印文件的空间"; break;
	case 63: errStr = "删除等候打印的文件"; break;
	case 64: errStr = "指定的网络名不再可用"; break;
	case 65: errStr = "拒绝网络访问"; break;
	case 66: errStr = "网络资源类型错误"; break;
	case 67: errStr = "找不到网络名"; break;
	case 68: errStr = "超过本地计算机网卡的名称限制"; break;
	case 69: errStr = "超出网络 BIOS 会话限制"; break;
	case 70: errStr = "远程服务器已暂停，或正在启动过程中"; break;
	case 71: errStr = "当前已无法再同此远程计算机连接，因为已达到计算机的连接数目极限"; break;
	case 72: errStr = "已暂停指定的打印机或磁盘设备"; break;
	case 80: errStr = "文件存在"; break;
	case 82: errStr = "无法创建目录或文件"; break;
	case 83: errStr = "INT 24 失败"; break;
	case 84: errStr = "无法取得处理此请求的存储空间"; break;
	case 85: errStr = "本地设备名已在使用中"; break;
	case 86: errStr = "指定的网络密码错误"; break;
	case 87: errStr = "参数错误"; break;
	case 88: errStr = "网络上发生写入错误"; break;
	case 89: errStr = "系统无法在此时启动另一个进程"; break;
	case 100: errStr = "无法创建另一个系统信号灯"; break;
	case 101: errStr = "另一个进程拥有独占的信号灯"; break;
	case 102: errStr = "已设置信号灯且无法关闭"; break;
	case 103: errStr = "无法再设置信号灯"; break;
	case 104: errStr = "无法在中断时请求独占的信号灯"; break;
	case 105: errStr = "此信号灯的前一个所有权已结束"; break;
	case 107: errStr = "程序停止，因为替代的软盘未插入"; break;
	case 108: errStr = "磁盘在使用中，或被另一个进程锁定"; break;
	case 109: errStr = "管道已结束"; break;
	case 110: errStr = "系统无法打开指定的设备或文件"; break;
	case 111: errStr = "文件名太长"; break;
	case 112: errStr = "磁盘空间不足"; break;
	case 113: errStr = "无法再获得内部文件的标识"; break;
	case 114: errStr = "目标内部文件的标识不正确"; break;
	case 117: errStr = "应用程序制作的 IOCTL 调用错误"; break;
	case 118: errStr = "验证写入的切换参数值错误"; break;
	case 119: errStr = "系统不支持请求的命令"; break;
	case 120: errStr = "此功能只被此系统支持"; break;
	case 121: errStr = "信号灯超时时间已到"; break;
	case 122: errStr = "传递到系统调用的数据区太小"; break;
	case 123: errStr = "文件名、目录名或卷标语法不正确"; break;
	case 124: errStr = "系统调用级别错误"; break;
	case 125: errStr = "磁盘没有卷标"; break;
	case 126: errStr = "找不到指定的模块"; break;
	case 127: errStr = "找不到指定的程序"; break;
	case 128: errStr = "没有等候的子进程"; break;
	case 130: errStr = "试图使用操作(而非原始磁盘 I/O)的已打开磁盘分区的文件句柄"; break;
	case 131: errStr = "试图移动文件指针到文件开头之前"; break;
	case 132: errStr = "无法在指定的设备或文件上设置文件指针"; break;
	case 133: errStr = "包含先前加入驱动器的驱动器无法使用 JOIN 或 SUBST 命令"; break;
	case 134: errStr = "试图在已被合并的驱动器上使用 JOIN 或 SUBST 命令"; break;
	case 135: errStr = "试图在已被合并的驱动器上使用 JOIN 或 SUBST 命令"; break;
	case 136: errStr = "系统试图解除未合并驱动器的 JOIN"; break;
	case 137: errStr = "系统试图解除未替代驱动器的 SUBST"; break;
	case 138: errStr = "系统试图将驱动器合并到合并驱动器上的目录"; break;
	case 139: errStr = "系统试图将驱动器替代为替代驱动器上的目录"; break;
	case 140: errStr = "系统试图将驱动器合并到替代驱动器上的目录"; break;
	case 141: errStr = "系统试图替代驱动器为合并驱动器上的目录"; break;
	case 142: errStr = "系统无法在此时运行 JOIN 或 SUBST"; break;
	case 143: errStr = "系统无法将驱动器合并到或替代为相同驱动器上的目录"; break;
	case 144: errStr = "目录并非根目录下的子目录"; break;
	case 145: errStr = "目录非空"; break;
	case 146: errStr = "指定的路径已在替代中使用"; break;
	case 147: errStr = "资源不足，无法处理此命令"; break;
	case 148: errStr = "指定的路径无法在此时使用"; break;
	case 149: errStr = "企图将驱动器合并或替代为驱动器上目录是上一个替代的目标的驱动器"; break;
	case 150: errStr = "系统跟踪信息未在 CONFIG.SYS 文件中指定，或不允许跟踪"; break;
	case 151: errStr = "为 DosMuxSemWait 指定的信号灯事件个数错误"; break;
	case 152: errStr = "DosMuxSemWait 不可运行。已设置过多的信号灯"; break;
	case 153: errStr = "DosMuxSemWait 清单错误"; break;
	case 154: errStr = "输入的卷标超过目标文件系统的长度限制"; break;
	case 155: errStr = "无法创建另一个线程"; break;
	case 156: errStr = "接收进程已拒绝此信号"; break;
	case 157: errStr = "段已被放弃且无法锁定"; break;
	case 158: errStr = "段已解除锁定"; break;
	case 159: errStr = "线程标识的地址错误"; break;
	case 160: errStr = "传递到 DosExecPgm 的参数字符串错误"; break;
	case 161: errStr = "指定的路径无效"; break;
	case 162: errStr = "信号已暂停"; break;
	case 164: errStr = "无法在系统中创建更多的线程"; break;
	case 167: errStr = "无法锁定文件区域"; break;
	case 170: errStr = "请求的资源在使用中"; break;
	case 173: errStr = "对于提供取消区域进行锁定的请求不明显"; break;
	case 174: errStr = "文件系统不支持锁定类型的最小单元更改"; break;
	case 180: errStr = "系统检测出错误的段号"; break;
	case 183: errStr = "当文件已存在时，无法创建该文件"; break;
	case 186: errStr = "传递的标志错误"; break;
	case 187: errStr = "找不到指定的系统信号灯名称"; break;
	case 196: errStr = "操作系统无法运行此应用程序"; break;
	case 197: errStr = "操作系统当前的配置不能运行此应用程序"; break;
	case 199: errStr = "操作系统无法运行此应用程序"; break;
	case 200: errStr = "代码段不可大于或等于 64K"; break;
	case 203: errStr = "操作系统找不到已输入的环境选项"; break;
	case 205: errStr = "命令子树中的进程没有信号处理程序"; break;
	case 206: errStr = "文件名或扩展名太长"; break;
	case 207: errStr = "第 2 环堆栈已被占用"; break;
	case 208: errStr = "没有正确输入文件名通配符 * 或 ?，或指定过多的文件名通配符"; break;
	case 209: errStr = "正在发送的信号错误"; break;
	case 210: errStr = "无法设置信号处理程序"; break;
	case 212: errStr = "段已锁定且无法重新分配"; break;
	case 214: errStr = "连到该程序或动态链接模块的动态链接模块太多"; break;
	case 215: errStr = "无法嵌套调用 LoadModule"; break;
	case 230: errStr = "管道状态无效"; break;
	case 231: errStr = "所有的管道实例都在使用中"; break;
	case 232: errStr = "管道正在关闭中"; break;
	case 233: errStr = "管道的另一端上无任何进程"; break;
	case 234: errStr = "更多数据可用"; break;
	case 240: errStr = "取消会话"; break;
	case 254: errStr = "指定的扩展属性名无效"; break;
	case 255: errStr = "扩展属性不一致"; break;
	case 258: errStr = "等待的操作过时"; break;
	case 259: errStr = "没有可用的数据了"; break;
	case 266: errStr = "无法使用复制功能"; break;
	case 267: errStr = "目录名无效"; break;
	case 275: errStr = "扩展属性在缓冲区中不适用"; break;
	case 276: errStr = "装在文件系统上的扩展属性文件已损坏"; break;
	case 277: errStr = "扩展属性表格文件已满"; break;
	case 278: errStr = "指定的扩展属性句柄无效"; break;
	case 282: errStr = "装入的文件系统不支持扩展属性"; break;
	case 288: errStr = "企图释放并非呼叫方所拥有的多用户终端运行程序"; break;
	case 298: errStr = "发向信号灯的请求过多"; break;
	case 299: errStr = "仅完成部分的 ReadProcessMemoty 或 WriteProcessMemory 请求"; break;
	case 300: errStr = "操作锁定请求被拒绝"; break;
	case 301: errStr = "系统接收了一个无效的操作锁定确认"; break;
	case 487: errStr = "试图访问无效的地址"; break;
	case 534: errStr = "算术结果超过 32 位"; break;
	case 535: errStr = "管道的另一端有一进程"; break;
	case 536: errStr = "等候打开管道另一端的进程"; break;
	case 994: errStr = "拒绝访问扩展属性"; break;
	case 995: errStr = "由于线程退出或应用程序请求，已放弃 I/O 操作"; break;
	case 996: errStr = "重叠 I/O 事件不在信号状态中"; break;
	case 997: errStr = "重叠 I/O 操作在进行中"; break;
	case 998: errStr = "内存分配访问无效"; break;
	case 999: errStr = "错误运行页内操作"; break;
	case 1001: errStr = "递归太深；栈溢出"; break;
	case 1002: errStr = "窗口无法在已发送的消息上操作"; break;
	case 1003: errStr = "无法完成此功能"; break;
	case 1004: errStr = "无效标志"; break;
	case 1005: errStr = "此卷不包含可识别的文件系统。请确定所有请求的文件系统驱动程序已加载，且此卷未损坏"; break;
	case 1006: errStr = "文件所在的卷已被外部改变，因此打开的文件不再有效"; break;
	case 1007: errStr = "无法在全屏幕模式下运行请求的操作"; break;
	case 1008: errStr = "试图引用不存在的令牌"; break;
	case 1009: errStr = "配置注册表数据库损坏"; break;
	case 1010: errStr = "配置注册表项无效"; break;
	case 1011: errStr = "无法打开配置注册表项"; break;
	case 1012: errStr = "无法读取配置注册表项"; break;
	case 1013: errStr = "无法写入配置注册表项"; break;
	case 1014: errStr = "注册表数据库中的某一文件必须使用记录或替代复制来恢复。恢复成功完成"; break;
	case 1015: errStr = "注册表损坏。包含注册表数据的某一文件结构损坏，或系统的文件内存映像损坏，或因为替代副本、日志缺少或损坏而无法恢复文件"; break;
	case 1016: errStr = "由注册表启动的 I/O 操作恢复失败。注册表无法读入、写出或清除任意一个包含注册表系统映像的文件"; break;
	case 1017: errStr = "系统试图加载或还原文件到注册表，但指定的文件并非注册表文件格式"; break;
	case 1018: errStr = "试图在标记为删除的注册表项上运行不合法的操作"; break;
	case 1019: errStr = "系统无法配置注册表日志中所请求的空间"; break;
	case 1020: errStr = "无法在已有子项或值的注册表项中创建符号链接"; break;
	case 1021: errStr = "无法在易变父项下创建稳定子项"; break;
	case 1022: errStr = "通知更改请求正在完成中，且信息并未返回到呼叫方的缓冲区中。当前呼叫方必须枚举文件来查找更改"; break;
	case 1051: errStr = "已发送停止控制到服务，该服务被其它正在运行的服务所依赖"; break;
	case 1052: errStr = "请求的控件对此服务无效"; break;
	case 1053: errStr = "服务并未及时响应启动或控制请求"; break;
	case 1054: errStr = "无法创建此服务的线程"; break;
	case 1055: errStr = "锁定服务数据库"; break;
	case 1056: errStr = "服务的实例已在运行中"; break;
	case 1057: errStr = "帐户名无效或不存在，或者密码对于指定的帐户名无效"; break;
	case 1058: errStr = "无法启动服务，原因可能是它被禁用或与它相关联的设备没有启动"; break;
	case 1059: errStr = "指定了循环服务依存"; break;
	case 1060: errStr = "指定的服务并未以已安装的服务存在"; break;
	case 1061: errStr = "服务无法在此时接受控制信息"; break;
	case 1062: errStr = "服务未启动"; break;
	case 1063: errStr = "服务进程无法连接到服务控制器上"; break;
	case 1064: errStr = "当处理控制请求时，在服务中发生异常"; break;
	case 1065: errStr = "指定的数据库不存在"; break;
	case 1066: errStr = "服务已返回特定的服务错误码"; break;
	case 1067: errStr = "进程意外终止"; break;
	case 1068: errStr = "依存服务或组无法启动"; break;
	case 1069: errStr = "由于登录失败而无法启动服务"; break;
	case 1070: errStr = "启动后，服务停留在启动暂停状态"; break;
	case 1071: errStr = "指定的服务数据库锁定无效"; break;
	case 1072: errStr = "指定的服务已标记为删除"; break;
	case 1073: errStr = "指定的服务已存在"; break;
	case 1074: errStr = "系统当前以最新的有效配置运行"; break;
	case 1075: errStr = "依存服务不存在，或已被标记为删除"; break;
	case 1076: errStr = "已接受使用当前引导作为最后的有效控制设置"; break;
	case 1077: errStr = "上次启动之后，仍未尝试引导服务"; break;
	case 1078: errStr = "名称已用作服务名或服务显示名"; break;
	case 1079: errStr = "此服务的帐户不同于运行于同一进程上的其它服务的帐户"; break;
	case 1080: errStr = "只能为 Win32 服务设置失败操作，不能为驱动程序设置"; break;
	case 1081: errStr = "这个服务所运行的处理和服务控制管理器相同。所以，如果服务处理程序意外中止的话，服务控制管理器无法进行任何操作"; break;
	case 1082: errStr = "这个服务尚未设置恢复程序"; break;
	case 1083: errStr = "配置成在该可执行程序中运行的这个服务不能执行该服务"; break;
	case 1100: errStr = "已达磁带的实际结尾"; break;
	case 1101: errStr = "磁带访问已达文件标记"; break;
	case 1102: errStr = "已达磁带或磁盘分区的开头"; break;
	case 1103: errStr = "磁带访问已达一组文件的结尾"; break;
	case 1104: errStr = "磁带上不再有任何数据"; break;
	case 1105: errStr = "磁带无法分区"; break;
	case 1106: errStr = "在访问多卷分区的新磁带时，当前的块大小不正确"; break;
	case 1107: errStr = "当加载磁带时，找不到分区信息"; break;
	case 1108: errStr = "无法锁定媒体弹出功能"; break;
	case 1109: errStr = "无法卸载介质"; break;
	case 1110: errStr = "驱动器中的介质可能已更改"; break;
	case 1111: errStr = "复位 I/O 总线"; break;
	case 1112: errStr = "驱动器中没有媒体"; break;
	case 1113: errStr = "在多字节的目标代码页中，没有此 Unicode 字符可以映射到的字符"; break;
	case 1114: errStr = "动态链接库 (DLL) 初始化例程失败"; break;
	case 1115: errStr = "系统关机正在进行"; break;
	case 1116: errStr = "因为没有任何进行中的关机过程，所以无法中断系统关机"; break;
	case 1117: errStr = "因为 I/O 设备错误，所以无法运行此项请求"; break;
	case 1118: errStr = "没有串行设备被初始化成功。串行驱动程序将卸载"; break;
	case 1119: errStr = "无法打开正在与其他设备共享中断请求(IRQ)的设备。至少有一个使用该 IRQ 的其他设备已打开"; break;
	case 1120: errStr = "序列 I/O 操作已由另一个串行口的写入完成。(IOCTL_SERIAL_XOFF_COUNTER 已达零)"; break;
	case 1121: errStr = "因为已过超时时间，所以串行 I/O 操作完成。(IOCTL_SERIAL_XOFF_COUNTER 未达零)"; break;
	case 1122: errStr = "在软盘上找不到 ID 地址标记"; break;
	case 1123: errStr = "软盘扇区 ID 字符域与软盘控制器磁道地址不相符"; break;
	case 1124: errStr = "软盘控制器报告软盘驱动程序不能识别的错误"; break;
	case 1125: errStr = "软盘控制器返回与其寄存器中不一致的结果"; break;
	case 1126: errStr = "当访问硬盘时，重新校准操作失败，重试仍然失败"; break;
	case 1127: errStr = "当访问硬盘时，磁盘操作失败，重试仍然失败"; break;
	case 1128: errStr = "当访问硬盘时，即使失败，仍须复位磁盘控制器"; break;
	case 1129: errStr = "已达磁带结尾"; break;
	case 1130: errStr = "服务器存储空间不足，无法处理此命令"; break;
	case 1131: errStr = "检测出潜在的死锁状态"; break;
	case 1132: errStr = "指定的基址或文件偏移量没有适当对齐"; break;
	case 1140: errStr = "改变系统供电状态的尝试被另一应用程序或驱动程序否决"; break;
	case 1141: errStr = "系统 BIOS 改变系统供电状态的尝试失败"; break;
	case 1142: errStr = "试图在一文件上创建超过系统允许数额的链接"; break;
	case 1150: errStr = "指定程序要求更新的 Windows 版本"; break;
	case 1151: errStr = "指定程序不是 Windows 或 MS-DOS 程序"; break;
	case 1152: errStr = "只能启动该指定程序的一个实例"; break;
	case 1153: errStr = "该指定程序适用于旧的 Windows 版本"; break;
	case 1154: errStr = "执行该应用程序所需的库文件之一被损坏"; break;
	case 1155: errStr = "没有应用程序与此操作的指定文件有关联"; break;
	case 1156: errStr = "在输送指令到应用程序的过程中出现错误"; break;
	case 1157: errStr = "执行该应用程序所需的库文件之一无法找到"; break;
	case 1158: errStr = "当前程序已使用了 Window 管理器对象的系统允许的所有句柄"; break;
	case 1159: errStr = "消息只能与同步操作一起使用"; break;
	case 1160: errStr = "指出的源元素没有媒体"; break;
	case 1161: errStr = "指出的目标元素已包含媒体"; break;
	case 1162: errStr = "指出的元素不存在"; break;
	case 1163: errStr = "指出的元素是未显示的存储资源的一部分"; break;
	case 1164: errStr = "显示设备需要重新初始化，因为硬件有错误"; break;
	case 1165: errStr = "设备显示在尝试进一步操作之前需要清除"; break;
	case 1166: errStr = "设备显示它的门仍是打开状态"; break;
	case 1167: errStr = "设备没有连接"; break;
	case 1168: errStr = "找不到元素"; break;
	case 1169: errStr = "索引中没有同指定项相匹配的项"; break;
	case 1170: errStr = "在对象上不存在指定的属性集"; break;
	case 1171: errStr = "传递到 GetMouseMovePoints 的点不在缓冲区中"; break;
	case 1172: errStr = "跟踪(工作站)服务没运行"; break;
	case 1173: errStr = "找不到卷 ID"; break;
	case 1175: errStr = "无法删除要被替换的文件"; break;
	case 1176: errStr = "无法将替换文件移到要被替换的文件。要被替换的文件保持原来的名称"; break;
	case 1177: errStr = "无法将替换文件移到要被替换的文件。要被替换的文件已被重新命名为备份名称"; break;
	case 1178: errStr = "卷更改记录被删除"; break;
	case 1179: errStr = "卷更改记录服务不处于活动中"; break;
	case 1180: errStr = "找到一份文件，但是可能不是正确的文件"; break;
	case 1181: errStr = "日志项从日志中被删除"; break;
	case 1200: errStr = "指定的设备名无效"; break;
	case 1201: errStr = "设备当前未连接上，但其为一个记录连接"; break;
	case 1202: errStr = "企图记录先前已被记录的设备"; break;
	case 1203: errStr = "无任何网络提供程序接受指定的网络路径"; break;
	case 1204: errStr = "指定的网络提供程序名称无效"; break;
	case 1205: errStr = "无法打开网络连接配置文件"; break;
	case 1206: errStr = "网络连接配置文件损坏"; break;
	case 1207: errStr = "无法枚举空载体"; break;
	case 1208: errStr = "发生扩展错误"; break;
	case 1209: errStr = "指定的组名格式无效"; break;
	case 1210: errStr = "指定的计算机名格式无效"; break;
	case 1211: errStr = "指定的事件名格式无效"; break;
	case 1212: errStr = "指定的域名格式无效"; break;
	case 1213: errStr = "指定的服务名格式无效"; break;
	case 1214: errStr = "指定的网络名格式无效"; break;
	case 1215: errStr = "指定的共享名格式无效"; break;
	case 1216: errStr = "指定的密码格式无效"; break;
	case 1217: errStr = "指定的消息名格式无效"; break;
	case 1218: errStr = "指定的消息目标格式无效"; break;
	case 1219: errStr = "提供的凭据与已存在的凭据集冲突"; break;
	case 1220: errStr = "企图创建网络服务器的会话，但已对该服务器创建过多的会话"; break;
	case 1221: errStr = "工作组或域名已由网络上的另一部计算机使用"; break;
	case 1222: errStr = "网络未连接或启动"; break;
	case 1223: errStr = "操作已被用户取消"; break;
	case 1224: errStr = "请求的操作无法在使用用户映射区域打开的文件上执行"; break;
	case 1225: errStr = "远程系统拒绝网络连接"; break;
	case 1226: errStr = "网络连接已被适当地关闭了"; break;
	case 1227: errStr = "网络传输终结点已有与其关联的地址"; break;
	case 1228: errStr = "地址仍未与网络终结点关联"; break;
	case 1229: errStr = "企图在不存在的网络连接上进行操作"; break;
	case 1230: errStr = "企图在使用中的网络连接上进行无效的操作"; break;
	case 1231: errStr = "不能访问网络位置。有关网络排除故障的信息，请参阅 Windows 帮助"; break;
	case 1232: errStr = "不能访问网络位置。有关网络排除故障的信息，请参阅 Windows 帮助"; break;
	case 1233: errStr = "不能访问网络位置。有关网络排除故障的信息，请参阅 Windows 帮助"; break;
	case 1234: errStr = "没有任何服务正在远程系统上的目标网络终结点上操作"; break;
	case 1235: errStr = "请求被终止"; break;
	case 1236: errStr = "由本地系统终止网络连接"; break;
	case 1237: errStr = "操作无法完成。应该重试"; break;
	case 1238: errStr = "因为已达到此帐户的最大同时连接数限制，所以无法连接服务器"; break;
	case 1239: errStr = "试图在这个帐户未被授权的时间内登录"; break;
	case 1240: errStr = "此帐户并未得到从这个工作站登录的授权"; break;
	case 1241: errStr = "请求的操作不能使用这个网络地址"; break;
	case 1242: errStr = "服务器已经注册"; break;
	case 1243: errStr = "指定的服务不存在"; break;
	case 1244: errStr = "因为用户还未被验证，不能执行所要求的操作"; break;
	case 1245: errStr = "因为用户还未登录网络，不能执行所要求的操作。指定的服务不存在"; break;
	case 1246: errStr = "正在继续工作"; break;
	case 1247: errStr = "试图进行初始操作，但是初始化已完成"; break;
	case 1248: errStr = "没有更多的本地设备"; break;
	case 1249: errStr = "指定的站点不存在"; break;
	case 1250: errStr = "具有指定名称的域控制器已经存在"; break;
	case 1251: errStr = "只有连接到服务器上时，该操作才受支持"; break;
	case 1252: errStr = "即使没有改动，组策略框架也应该调用扩展"; break;
	case 1253: errStr = "指定的用户没有一个有效的配置文件"; break;
	case 1254: errStr = "Microsoft Small Business Server 不支持此操作"; break;
	case 1300: errStr = "并非所有被引用的特权都指派给呼叫方"; break;
	case 1301: errStr = "帐户名和安全标识间的某些映射未完成"; break;
	case 1302: errStr = "没有为该帐户特别设置系统配额限制"; break;
	case 1303: errStr = "没有可用的加密密钥。返回了一个已知加密密钥"; break;
	case 1304: errStr = "密码太复杂，无法转换成 LAN Manager 密码。返回的 LAN Manager 密码为空字符串"; break;
	case 1305: errStr = "修订级别未知"; break;
	case 1306: errStr = "表明两个修订级别是不兼容的"; break;
	case 1307: errStr = "这个安全标识不能指派为此对象的所有者"; break;
	case 1308: errStr = "这个安全标识不能指派为对象的主要组"; break;
	case 1309: errStr = "当前并未模拟客户的线程试图操作模拟令牌"; break;
	case 1310: errStr = "组可能未被禁用"; break;
	case 1311: errStr = "当前没有可用的登录服务器来服务登录请求"; break;
	case 1312: errStr = "指定的登录会话不存在。可能已被终止"; break;
	case 1313: errStr = "指定的特权不存在"; break;
	case 1314: errStr = "客户没有所需的特权"; break;
	case 1315: errStr = "提供的名称并非正确的帐户名形式"; break;
	case 1316: errStr = "指定的用户已存在"; break;
	case 1317: errStr = "指定的用户不存在"; break;
	case 1318: errStr = "指定的组已存在"; break;
	case 1319: errStr = "指定的组不存在"; break;
	case 1320: errStr = "指定的用户帐户已是指定组的成员，或是因为组包含成员所以无法删除指定的组"; break;
	case 1321: errStr = "指定的用户帐户不是指定组帐户的成员"; break;
	case 1322: errStr = "无法禁用或删除最后剩余的系统管理帐户"; break;
	case 1323: errStr = "无法更新密码。提供作为当前密码的值不正确"; break;
	case 1324: errStr = "无法更新密码。提供给新密码的值包含密码中不允许的值"; break;
	case 1325: errStr = "无法更新密码。为新密码提供的值不符合字符域的长度、复杂性或历史要求"; break;
	case 1326: errStr = "登录失败: 未知的用户名或错误密码"; break;
	case 1327: errStr = "登录失败: 用户帐户限制"; break;
	case 1328: errStr = "登录失败: 违反帐户登录时间限制"; break;
	case 1329: errStr = "登录失败: 不允许用户登录到此计算机"; break;
	case 1330: errStr = "登录失败: 指定的帐户密码已过期"; break;
	case 1331: errStr = "登录失败: 禁用当前的帐户"; break;
	case 1332: errStr = "帐户名与安全标识间无任何映射完成"; break;
	case 1333: errStr = "一次请求过多的本地用户标识符(LUIDs)"; break;
	case 1334: errStr = "无更多可用的本地用户标识符(LUIDs)"; break;
	case 1335: errStr = "对于该特别用法，安全 ID 的次级授权部分无效"; break;
	case 1336: errStr = "访问控制列表(ACL)结构无效"; break;
	case 1337: errStr = "安全 ID 结构无效"; break;
	case 1338: errStr = "安全描述符结构无效"; break;
	case 1340: errStr = "无法创建固有的访问控制列表(ACL)或访问控制项目(ACE)"; break;
	case 1341: errStr = "服务器当前已禁用"; break;
	case 1342: errStr = "服务器当前已启用"; break;
	case 1343: errStr = "提供给识别代号颁发机构的值为无效值"; break;
	case 1344: errStr = "无更多可用的内存以更新安全信息"; break;
	case 1345: errStr = "指定属性无效，或与整个群体的属性不兼容"; break;
	case 1346: errStr = "指定的模拟级别无效， 或所提供的模拟级别无效"; break;
	case 1347: errStr = "无法打开匿名级安全令牌"; break;
	case 1348: errStr = "请求的验证信息类别无效"; break;
	case 1349: errStr = "令牌的类型对其尝试使用的方法不适当"; break;
	case 1350: errStr = "无法在与安全性无关联的对象上运行安全性操作"; break;
	case 1351: errStr = "未能从域控制器读取配置信息，或者是因为机器不可使用，或者是访问被拒绝"; break;
	case 1352: errStr = "安全帐户管理器(SAM)或本地安全颁发机构(LSA)服务器处于运行安全操作的错误状态"; break;
	case 1353: errStr = "域处于运行安全操作的错误状态"; break;
	case 1354: errStr = "此操作只对域的主要域控制器可行"; break;
	case 1355: errStr = "指定的域不存在，或无法联系"; break;
	case 1356: errStr = "指定的域已存在"; break;
	case 1357: errStr = "试图超出每服务器域个数的限制"; break;
	case 1358: errStr = "无法完成请求操作，因为磁盘上的严重介质失败或数据结构损坏"; break;
	case 1359: errStr = "出现了内部错误"; break;
	case 1360: errStr = "通用访问类型包含于已映射到非通用类型的访问掩码中"; break;
	case 1361: errStr = "安全描述符格式不正确 (绝对或自相关的)"; break;
	case 1362: errStr = "请求操作只限制在登录进程中使用。调用进程未注册为一个登录进程"; break;
	case 1363: errStr = "无法使用已在使用中的标识启动新的会话"; break;
	case 1364: errStr = "未知的指定验证数据包"; break;
	case 1365: errStr = "登录会话并非处于与请求操作一致的状态中"; break;
	case 1366: errStr = "登录会话标识已在使用中"; break;
	case 1367: errStr = "登录请求包含无效的登录类型值"; break;
	case 1368: errStr = "在使用命名管道读取数据之前，无法经由该管道模拟"; break;
	case 1369: errStr = "注册表子树的事务处理状态与请求状态不一致"; break;
	case 1370: errStr = "安全性数据库内部出现损坏"; break;
	case 1371: errStr = "无法在内置帐户上运行此操作"; break;
	case 1372: errStr = "无法在内置特殊组上运行此操作"; break;
	case 1373: errStr = "无法在内置特殊用户上运行此操作"; break;
	case 1374: errStr = "无法从组中删除用户，因为当前组为用户的主要组"; break;
	case 1375: errStr = "令牌已作为主要令牌使用"; break;
	case 1376: errStr = "指定的本地组不存在"; break;
	case 1377: errStr = "指定的帐户名不是本地组的成员"; break;
	case 1378: errStr = "指定的帐户名已是本地组的成员"; break;
	case 1379: errStr = "指定的本地组已存在"; break;
	case 1380: errStr = "登录失败: 未授予用户在此计算机上的请求登录类型"; break;
	case 1381: errStr = "已超过在单一系统中可保存机密的最大个数"; break;
	case 1382: errStr = "机密的长度超过允许的最大长度"; break;
	case 1383: errStr = "本地安全颁发机构数据库内部包含不一致性"; break;
	case 1384: errStr = "在尝试登录的过程中，用户的安全上下文积累了过多的安全标识"; break;
	case 1385: errStr = "登录失败: 未授予用户在此计算机上的请求登录类型"; break;
	case 1386: errStr = "更改用户密码时需要交叉加密密码"; break;
	case 1387: errStr = "由于成员不存在，无法将成员添加到本地组中，也无法从本地组将其删除"; break;
	case 1388: errStr = "无法将新成员加入到本地组中，因为成员的帐户类型错误"; break;
	case 1389: errStr = "已指定过多的安全标识"; break;
	case 1390: errStr = "更改此用户密码时需要交叉加密密码"; break;
	case 1391: errStr = "表明 ACL 未包含任何可承继的组件"; break;
	case 1392: errStr = "文件或目录损坏且无法读取"; break;
	case 1393: errStr = "磁盘结构损坏且无法读取"; break;
	case 1394: errStr = "无任何指定登录会话的用户会话项"; break;
	case 1395: errStr = "正在访问的服务有连接数目标授权限制。这时候已经无法再连接，原因是已经到达可接受的连接数目上限"; break;
	case 1396: errStr = "登录失败: 该目标帐户名称不正确"; break;
	case 1397: errStr = "相互身份验证失败。该服务器在域控制器的密码过期"; break;
	case 1398: errStr = "在客户机和服务器之间有一个时间差"; break;
	default:   errStr = "未知系统错误"; break;
	}
#else
	switch (aSysErrId)
	{
	case 0: errStr = "Success"; break;
	case 1: errStr = "Operation not permitted"; break;
	case 2: errStr = "No such file or directory"; break;
	case 3: errStr = "No such process"; break;
	case 4: errStr = "Interrupted system call"; break;
	case 5: errStr = "Input/output error"; break;
	case 6: errStr = "No such device or address"; break;
	case 7: errStr = "Argument list too long"; break;
	case 8: errStr = "Exec format error"; break;
	case 9: errStr = "Bad file descriptor"; break;
	case 10: errStr = "No child processes"; break;
	case 11: errStr = "Resource temporarily unavailable"; break;
	case 12: errStr = "Cannot allocate memory"; break;
	case 13: errStr = "Permission denied"; break;
	case 14: errStr = "Bad address"; break;
	case 15: errStr = "Block device required"; break;
	case 16: errStr = "Device or resource busy"; break;
	case 17: errStr = "File exists"; break;
	case 18: errStr = "Invalid cross-device link"; break;
	case 19: errStr = "No such device"; break;
	case 20: errStr = "Not a directory"; break;
	case 21: errStr = "Is a directory"; break;
	case 22: errStr = "Invalid argument"; break;
	case 23: errStr = "Too many open files in system"; break;
	case 24: errStr = "Too many open files"; break;
	case 25: errStr = "Inappropriate ioctl for device"; break;
	case 26: errStr = "Text file busy"; break;
	case 27: errStr = "File too large"; break;
	case 28: errStr = "No space left on device"; break;
	case 29: errStr = "Illegal seek"; break;
	case 30: errStr = "Read-only file system"; break;
	case 31: errStr = "Too many links"; break;
	case 32: errStr = "Broken pipe"; break;
	case 33: errStr = "Numerical argument out of domain"; break;
	case 34: errStr = "Numerical result out of range"; break;
	case 35: errStr = "Resource deadlock avoided"; break;
	case 36: errStr = "File name too long"; break;
	case 37: errStr = "No locks available"; break;
	case 38: errStr = "Function not implemented"; break;
	case 39: errStr = "Directory not empty"; break;
	case 40: errStr = "Too many levels of symbolic links"; break;
	case 41: errStr = "Unknown error 41"; break;
	case 42: errStr = "No message of desired type"; break;
	case 43: errStr = "Identifier removed"; break;
	case 44: errStr = "Channel number out of range"; break;
	case 45: errStr = "Level 2 not synchronized"; break;
	case 46: errStr = "Level 3 halted"; break;
	case 47: errStr = "Level 3 reset"; break;
	case 48: errStr = "Link number out of range"; break;
	case 49: errStr = "Protocol driver not attached"; break;
	case 50: errStr = "No CSI structure available"; break;
	case 51: errStr = "Level 2 halted"; break;
	case 52: errStr = "Invalid exchange"; break;
	case 53: errStr = "Invalid request descriptor"; break;
	case 54: errStr = "Exchange full"; break;
	case 55: errStr = "No anode"; break;
	case 56: errStr = "Invalid request code"; break;
	case 57: errStr = "Invalid slot"; break;
	case 58: errStr = "Unknown error 58"; break;
	case 59: errStr = "Bad font file format"; break;
	case 60: errStr = "Device not a stream"; break;
	case 61: errStr = "No data available"; break;
	case 62: errStr = "Timer expired"; break;
	case 63: errStr = "Out of streams resources"; break;
	case 64: errStr = "Machine is not on the network"; break;
	case 65: errStr = "Package not installed"; break;
	case 66: errStr = "Object is remote"; break;
	case 67: errStr = "Link has been severed"; break;
	case 68: errStr = "Advertise error"; break;
	case 69: errStr = "Srmount error"; break;
	case 70: errStr = "Communication error on send"; break;
	case 71: errStr = "Protocol error"; break;
	case 72: errStr = "Multihop attempted"; break;
	case 73: errStr = "RFS specific error"; break;
	case 74: errStr = "Bad message"; break;
	case 75: errStr = "Value too large for defined data type"; break;
	case 76: errStr = "Name not unique on network"; break;
	case 77: errStr = "File descriptor in bad state"; break;
	case 78: errStr = "Remote address changed"; break;
	case 79: errStr = "Can not access a needed shared library"; break;
	case 80: errStr = "Accessing a corrupted shared library"; break;
	case 81: errStr = ".lib section in a.out corrupted"; break;
	case 82: errStr = "Attempting to link in too many shared libraries"; break;
	case 83: errStr = "Cannot exec a shared library directly"; break;
	case 84: errStr = "Invalid or incomplete multibyte or wide character"; break;
	case 85: errStr = "Interrupted system call should be restarted"; break;
	case 86: errStr = "Streams pipe error"; break;
	case 87: errStr = "Too many users"; break;
	case 88: errStr = "Socket operation on non-socket"; break;
	case 89: errStr = "Destination address required"; break;
	case 90: errStr = "Message too long"; break;
	case 91: errStr = "Protocol wrong type for socket"; break;
	case 92: errStr = "Protocol not available"; break;
	case 93: errStr = "Protocol not supported"; break;
	case 94: errStr = "Socket type not supported"; break;
	case 95: errStr = "Operation not supported"; break;
	case 96: errStr = "Protocol family not supported"; break;
	case 97: errStr = "Address family not supported by protocol"; break;
	case 98: errStr = "Address already in use"; break;
	case 99: errStr = "Cannot assign requested address"; break;
	case 100: errStr = "Network is down"; break;
	case 101: errStr = "Network is unreachable"; break;
	case 102: errStr = "Network dropped connection on reset"; break;
	case 103: errStr = "Software caused connection abort"; break;
	case 104: errStr = "Connection reset by peer"; break;
	case 105: errStr = "No buffer space available"; break;
	case 106: errStr = "Transport endpoint is already connected"; break;
	case 107: errStr = "Transport endpoint is not connected"; break;
	case 108: errStr = "Cannot send after transport endpoint shutdown"; break;
	case 109: errStr = "Too many references: cannot splice"; break;
	case 110: errStr = "Connection timed out"; break;
	case 111: errStr = "Connection refused"; break;
	case 112: errStr = "Host is down"; break;
	case 113: errStr = "No route to host"; break;
	case 114: errStr = "Operation already in progress"; break;
	case 115: errStr = "Operation now in progress"; break;
	case 116: errStr = "Stale NFS file handle"; break;
	case 117: errStr = "Structure needs cleaning"; break;
	case 118: errStr = "Not a XENIX named type file"; break;
	case 119: errStr = "No XENIX semaphores available"; break;
	case 120: errStr = "Is a named type file"; break;
	case 121: errStr = "Remote I/O error"; break;
	case 122: errStr = "Disk quota exceeded"; break;
	case 123: errStr = "No medium found"; break;
	case 124: errStr = "Wrong medium type"; break;
	case 125: errStr = "Operation canceled"; break;
	case 126: errStr = "Required key not available"; break;
	case 127: errStr = "Key has expired"; break;
	case 128: errStr = "Key has been revoked"; break;
	case 129: errStr = "Key was rejected by service"; break;
	case 130: errStr = "Owner died"; break;
	case 131: errStr = "State not recoverable"; break;
	default:  errStr = "未知系统错误"; break;
	}
#endif
	return errStr;
}

const char* vxGetErrorString(VXERR errCode)
{
    const char* errStr = NULL;
	switch (errCode)
	{
	case VX_SUCCESS_TESTFUTUER:
		errStr = "函数成功了，但是还有些事情需要将来进一步检验";
		break;
	case VX_SUCCESS_ADDDEFAULT:
		errStr = "函数成功了，但是添加了一些必须的组件";
		break;
	case VX_SUCCESS_NODEFAULT:
		errStr = "函数成功了，但是有些该有的组件没有出现";
		break;
	case VX_SUCCESS_NORT:
		errStr = "函数成功了，但是有些地方由RT变成非RT";
		break;
	case VX_SUCCESS:
		errStr = "函数成功了，没有发现任何问题";
		break;
	case VX_ERROR:
		errStr = "函数没有成功，不知道原因";
		break;
	case VX_E_NOTIMPL:
		errStr = "接口未实现";
		break;
	case VX_E_UNDEFINE:
		errStr = "未定义的命令";
		break;
	case VX_E_CLIP_DURATING:
		errStr = "片段的持续时间不合适";
		break;
	case VX_E_OUTOFMEMORY:
		errStr = "内存不足";
		break;
	case VX_E_NOTFOUND:
		errStr = "没有发现寻找的对象";
		break;
	case VX_E_NOIMPLEMENT:
		errStr = "这个函数没有实现";
		break;
	case VX_E_OUTOFSLOT:
		errStr = "列表达到了最大的限制";
		break;
	case VX_E_LOSTCONNECT:
		errStr = "数据库失去连接";
		break;
	case VX_E_BUFFERNOTENOUGH:
		errStr = "数据缓冲区不够大";
		break;
	case VX_E_OBJECTNOTENOUGT:
		errStr = "操作对象不够多";
		break;
	case VX_E_INVALIDPARAM:
		errStr = "参数不合法";
		break;
	case VX_E_TRACKNOSPACE:
		errStr = "轨道上没有合适的存放空间";
		break;
	case VX_E_INVALIDFILE:
		errStr = "文件格式错误";
		break;
	case VX_E_NETSAVEFAILED:
		errStr = "向网络上保存文件失败";
		break;
	case VX_E_TYPENOTMATCH:
		errStr = "类型不匹配";
		break;
	case VX_E_NOFILENAME:
		errStr = "这个操作需要已经保存过文件";
		break;
	case VX_E_CANLINKSELF:
		errStr = "不能连接在自己的连上";
		break;
	case VX_E_NOTMATTERFILE:
		errStr = "指定文件不是素材文件";
		break;
	case VX_E_NOTATTACHHEL:
		errStr = "当前操作数据没有连接在引擎上";
		break;
	case VX_E_GFBINVALID:
		errStr = "GFB层好象有问题，不能准备播放";
		break;
	case VX_E_CGINVALID:
		errStr = "CG层好象有问题，不能准备播放";
		break;
	case VX_E_NOTCUE:
		errStr = "在播放以前缺乏准备过程，不能进入播放状态";
		break;
	case VX_E_OUTOFHWRESOURCE:
		errStr = "硬件资源不足";
		break;
	case VX_E_CGEFXFACTORYLOST:
		errStr = "失去了字幕的类工厂，或没有装载成功";
		break;
	case VX_E_FORECASEFAILED:
		errStr = "预测失败";
		break;
	case VX_E_CGRENDERLOST:
		errStr = "CGRenderTool没有加载进来";
		break;
	case VX_E_CGPRAVATELOST:
		errStr = "CGPrivateFactory没有加载进来";
		break;
	case VX_E_EFXNOTFOUND:
		errStr = "指定的EFX没有注册";
		break;
	case VX_E_STATENOTMATCH:
		errStr = "状态不正确";
		break;
	case VX_E_INVALIDSOURCE:
		errStr = "内部数据源有问题";
		break;
	case VX_E_HAVEOWNER:
		errStr = "由于物件有了宿主，这个操作不允许";
		break;
	case VX_E_HAVESIBLING:
		errStr = "由于物件有了兄弟，这个操作不允许";
		break;
	case VX_E_INVALIDCOREHANDLE:
		errStr = "核心对象的句柄不认识，比如模板字幕中的HTEMPLCG";
		break;
	case VX_E_RENDERPAGEFAILED:
		errStr = "摸板字幕指定的页渲染失败";
		break;
	case VX_E_INVALIDFORMAT:
		errStr = "这个物件的格式不能进行这个操作";
		break;
	case VX_E_WRITEFILEERROR:
		errStr = "写文件失败,可能是写磁盘速度问题或网络问题";
		break;
	case VX_E_READFILEERROR:
		errStr = "读文件失败";
		break;
	case VX_E_INVALIDVERSION:
		errStr = "不正确的版本";
		break;
	case VX_E_INVALIDHANDLE:
		errStr = "不正确的句柄做了参数";
		break;
	case VX_E_INVALIDPTR:
		errStr = "无效的指针参数";
		break;
	case VX_E_INNERPARAMERR:
		errStr = "内部参数错误";
		break;
	case VX_E_INVALIDCGFACEUP:
		errStr = "ICGFaceUP没有正确初始化";
		break;
	case VX_E_NOINITIALIZE:
		errStr = "没有初始化";
		break;
	case VX_E_NOREPLY:
		errStr = "长时间没有应答";
		break;
	case VX_E_EDITORBUSY:
		errStr = "编辑器正忙";
		break;
	case VX_E_DEFAULTTEMPLLOST:
		errStr = "丢失了缺省摸板";
		break;
	case VX_E_CGSERVERLOADFAIL:
		errStr = "CG编辑服务器装载文件失败";
		break;
	case VX_E_CGSERVERNOTFOUND:
		errStr = "CG编辑服务器没有找到";
		break;
	case VX_E_CLIPSNUMBER:
		errStr = "片段数目不对";
		break;
	case VX_E_CLIPSLOCKED:
		errStr = "有些片段被锁住了";
		break;
	case VX_E_RESERVEPATH:
		errStr = "这些路径是保留的，有特殊意义";
		break;
	case VX_E_INVALIDEPATH:
		errStr = "无效路径";
		break;
	case VX_E_BADFILE:
		errStr = "文件内容是错误的";
		break;
	case VX_E_INVALIDTRIM:
		errStr = "片段的入出点有问题";
		break;
	case VX_E_VIEWNOTACTIVE:
		errStr = "这个视图不是活动的";
		break;
	case VX_E_HARDNOTFOUND:
		errStr = "硬件引擎没有初始化";
		break;
	case VX_E_HARDBUSY:
		errStr = "硬件引擎正忙";
		break;
	case VX_E_ENGINELACK:
		errStr = "缺少必要的底层引擎";
		break;
	case VX_E_DATAINCHANGING:
		errStr = "数据核心正处于修改状态中";
		break;
		// 	case VX_E_CMDSUCCESS:
		// 		errStr = "命令正确执行了已经收到答复";
		// 		break;

		// 	case VX_E_CMDTESTFUTHER:
		// 		errStr = "命令正确发送，但是暂时没有回答";
		// 		break;
	case VX_E_CMDENGINEBUSY:
		errStr = "引擎在指定时间内没有到达空闲状态，不能执行命令";
		break;
	case VX_E_CMDUNKNOWN:
		errStr = "不认识的命令";
		break;
	case VX_E_CMDENGINEFREE:
		errStr = "引擎正在空闲状态，不能执行命令";
		break;
	case VX_E_CMDNOREPLY:
		errStr = "引擎命令正确发出，但是在指定时间内没有收到答复";
		break;
	case VX_E_CMDNORESPOND:
		errStr = "引擎命令发出很久，但是引擎却没有接收";
		break;
	case VX_E_UD_CANCEL:
		errStr = "下载引擎被界面命令停止，磁带上没有任何痕迹";
		break;
	case VX_E_UD_ABORT:
		errStr = "下载引擎被界面命令停止，磁带上有部分已经下载";
		break;
	case VX_E_UD_VTRNOTLOCKSERVO:
		errStr = "VTR无法进入平稳运行状态，可能是磁带上的时间码不持续不连续";
		break;
	case VX_E_UD_VTRNOTREPOUND:
		errStr = "VTR无法受控进入指定状态";
		break;
	case VX_E_UD_VTRINPOINTMISS:
		errStr = "VTR无法到达指定的入点";
		break;
	case VX_E_UD_VTRTIMEMISS:
		errStr = "VTR无法取得时间，可能是连线有问题，也可能是系统中正在运行的任务太多";
		break;
	case VX_E_UD_TIMEOVERFLOW:
		errStr = "上下载准备过程中，要检查一些状态，如果不匹配，就会停止的,一般是错过了时间";
		break;
	case VX_E_UD_VTRNOTLINKED:
		errStr = "VTR没有初始化成功";
		break;
	case VX_E_UD_LACKCTLCODE:
		errStr = "机器缺少CTL码，有些功能不能完成";
		break;
	case VX_E_UD_NOTRESPOND:
		errStr = "请求没有被正确回答，可能试太忙或死锁";
		break;
	case VX_E_UD_SINKLACK:
		errStr = "还没有设置采集目标";
		break;
	case VX_E_UD_VTRNOTINLOCAL:
		errStr = "VTR不在计算机的控制下，需要手工拨";
		break;
	case VX_E_UD_NOTSUPPORTTCMOD:
		errStr = "VTR不支持指定的时码格式";
		break;
	case VX_E_UD_HWSTATESWITCH:
		errStr = "硬件的状态切换过程错误，一般是采集／回放切换";
		break;
	case VX_E_UD_VTROPTIONMISS:
		errStr = "VTR的参数不准，需要校准";
		break;
	case VX_E_UD_VTRTESTERR:
		errStr = "VTR自测失败";
		break;
	case VX_E_UD_VTRINLOCK:
		errStr = "由于核心的需要，目前VTR被锁住";
		break;
	case VX_E_UD_VIDBUFOVERFLOW:
		errStr = "采集压缩视频内存上溢,可能是系统性能达不到压缩要求";
		break;
	case VX_E_UD_AUDBUFOVERFLOW:
		errStr = "采集压缩音频内存上溢,可能是系统性能达不到压缩要求";
		break;
	case VX_E_UD_LOWDISKSPEED:
		errStr = "硬盘速率不够";
		break;
	case VX_E_UD_VSINGNALERROR:
		errStr = "视频输入信号不完全有丢帧";
		break;
	case VX_E_UD_ASINGNALERROR:
		errStr = "音频输入信号不完全有丢帧	";
		break;
	case VX_E_ENC_MUXER_FAIL:
		errStr = "复用器初始化流失败";
		break;
	case VX_E_ENC_VIDENC_FAIL:
		errStr = "视频压缩初始化失败";
		break;
	case VX_E_ENC_AUDENC_FAIL:
		errStr = "音频压缩初始化失败";
		break;
	case VX_E_ENC_INVALIDDEST:
		errStr = "无效的数据目标";
		break;
	case VX_E_ENC_INVALIDMUXER:
		errStr = "无效的复用器";
		break;
	case VX_E_ENC_EXPORT_FAIL:
		errStr = "压缩管理器初始化失败";
		break;
	case VX_E_ENC_VIDENC_BEGIN_FAIL:
		errStr = "启动视频压缩失败";
		break;
	case VX_E_ENC_AUDENC_BEGIN_FAIL:
		errStr = "启动音频压缩失败";
		break;
	case VX_E_ENC_CODEC_MISSMATCH:
		errStr = "压缩器和复用器不匹配";
		break;
	case VX_E_ENC_MUXER_MISS:
		errStr = "复用器不存在";
		break;
	case VX_E_ENC_VSETTING_FAIL:
		errStr = "错误的视频压缩配置";
		break;
	case VX_E_ENC_ASETTING_FAIL:
		errStr = "错误的音频压缩配置";
		break;
	case VX_E_ENC_NOSPLITECAPTURE:
		errStr = "复用器不支持分段采集";
		break;
	case VX_E_EFX_NOTIMPBKGROUND:
		errStr = "特技不能在背景轨道使用";
		break;
	case VX_E_EFX_NOTIMPINHERE:
		errStr = "固有特技不能添加或删除";
		break;
	case VX_E_EFX_CLIPTYPENOTMATCH:
		errStr = "特技和片段类型不匹配";
		break;
	case VX_E_EFX_CANTUSEWITHKEYVIDEO:
		errStr = "不能用于带键视频";
		break;
	case VX_E_EFX_SLOTFULL:
		errStr = "特技数量已经达到最大";
		break;
	case VX_E_EFX_CANNOTNAPPE:
		errStr = "这个特技不支持叠加";
		break;
	case VX_E_EFX_3DCANNOTNAPPE:
		errStr = "片段上已经有不能和3D特技一起使用的特技";
		break;
	case VX_E_EFX_CANTUSEWITH3D:
		errStr = "这个特技不能和3D特技一起使用";
		break;
	case VX_E_EFX_CANTREMOVEINHERE:
		errStr = "固有特技不能删除";
		break;
	case VX_E_UD_STOPPING:
		errStr = "采集在停止过程中";
		break;
	case VX_E_UD_NONEEDRES:
		errStr = "没有需要的资源";
		break;
	case VX_E_UD_NOVSINGNAL:
		errStr = "没有视频输入信号";
		break;
	case VX_E_ENC_VIDPREPROC_FAIL:
		errStr = "视频预处理初始化失败";
		break;
	case VX_E_ENC_VIDWRITE_FAIL:
		errStr = "视频压缩写文件失败,可能是写磁盘速度问题";
		break;
	case VX_E_ENC_AUDWRITE_FAIL:
		errStr = "音频压缩写文件失败,可能是写磁盘速度问题";
		break;
	case VX_E_ENC_MUXER_BEGIN_FAIL:
		errStr = "复用器启动失败,可能是模板不支持此源文件";
		break;
	case VX_E_ENC_VIDEO_FAIL:
		errStr = "视频压缩写文件失败,可能是写磁盘速度问题";
		break;
	case VX_E_ENC_AUDIO_FAIL:
		errStr = "音频压缩写文件失败,可能是写磁盘速度问题";
		break;
	case VX_E_DEC_FILE_OPEN:
		errStr = "打开文件失败";
		break;
	case VX_E_DEC_FTP_CONNECT:
		errStr = "连接ftp服务器失败";
		break;
	case VX_E_DEC_GET_FILESIZE:
		errStr = "获取文件大小失败";
		break;
	case VX_E_DEC_CREATE_DIR:
		errStr = "创建文件目录失败";
		break;
	case VX_E_DEC_READVIDEOFRAME:
		errStr = "读视频帧数据失败";
		break;
	case VX_E_DEC_READAUDIOFRAME:
		errStr = "读音频帧数据失败";
		break;
	case VX_E_DEC_SET_FILEPOSITION:
		errStr = "设置文件位置失败";
		break;
	case VX_E_DEC_VIDEODEC:
		errStr = "视频解码失败";
		break;
	case VX_E_DEC_AUDIODEC:
		errStr = "音频解码失败";
		break;
	case VX_E_MUXER_NO_VA:
		errStr = "复用器没有收到足够的视音频数据";
		break;
	default:
		errStr = "未知错误";
		break;
	}

	return errStr;
}

/*
extern "C" int __isoc99_fscanf(FILE *__stream, const char *__format)
{
    return  fscanf(__stream,__format);
}
*/
