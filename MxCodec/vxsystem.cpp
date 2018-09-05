// VxSysEngine.cpp : ���� DLL Ӧ�ó������ڵ㡣
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
		TRACE("����===ϵͳ����===ģ��!\n");

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

		//�������DLL�Ĳ�һ�������߳�,����Ҳ̫���ˣ����߳��Ȳ���������
		//_vxSetThreadName("���߳�");
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
		TRACE("ж��===ϵͳ����===ģ��!\n");
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

	//�������DLL�Ĳ�һ�������߳�,����Ҳ̫���ˣ����߳��Ȳ���������
	//_vxSetThreadName("���߳�");
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
        vxTrace("�߳�[%s][0x%lx(%d)]�˳�----------------",m_name,m_id,m_id);
#else
        vxTrace("�߳�[%s][0x%lx(%d)]�˳�----------------\n",m_name,m_id,m_id);
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
        vxTrace("�߳�[%s][0x%04lx(%d)]��ʼ\n",m_name,m_id,m_id);
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
#ifdef _DEBUG//	_DEBUG��β�ͺ�����ϼ����Ϣ
			size = (((size+8)+0xF)&(~0xF))-8;		//	��֤m_allocsize����16�ֽڵ�����
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
			size = (((size+4)+0xF)&(~0xF))-4;		//	��֤m_allocsize����16�ֽڵ�����
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
#ifdef _DEBUG//	_DEBUG���β�ͺ������Ϣ
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
		LONG	canAlloc = (LONG)(m_pTail - pData - 0x10000);	//	����64K
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
        vxTrace("�߳�[%s][%d]����MEMORY��ʱ��\n",m_tname,m_tid);
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
	case 0: errStr = "�����ɹ����"; break;
	case 1: errStr = "���ܴ���"; break;
	case 2: errStr = "ϵͳ�Ҳ���ָ�����ļ�"; break;
	case 3: errStr = "ϵͳ�Ҳ���ָ����·��"; break;
	case 4: errStr = "ϵͳ�޷����ļ�"; break;
	case 5: errStr = "�ܾ�����"; break;
	case 6: errStr = "�����Ч"; break;
	case 7: errStr = "�洢���ƿ鱻��"; break;
	case 8: errStr = "�洢�ռ䲻�㣬�޷����������"; break;
	case 9: errStr = "�洢���ƿ��ַ��Ч"; break;
	case 10: errStr = "��������"; break;
	case 11: errStr = "��ͼ���ظ�ʽ����ĳ���"; break;
	case 12: errStr = "��������Ч"; break;
	case 13: errStr = "������Ч"; break;
	case 14: errStr = "�洢�����㣬�޷���ɴ˲���"; break;
	case 15: errStr = "ϵͳ�Ҳ���ָ����������"; break;
	case 16: errStr = "�޷�ɾ��Ŀ¼"; break;
	case 17: errStr = "ϵͳ�޷����ļ��Ƶ���ͬ��������"; break;
	case 18: errStr = "û�и����ļ�"; break;
	case 19: errStr = "������д�뱣��"; break;
	case 20: errStr = "ϵͳ�Ҳ���ָ�����豸"; break;
	case 21: errStr = "�豸δ����"; break;
	case 22: errStr = "�豸��ʶ�������"; break;
	case 23: errStr = "���ݴ��� (ѭ��������)"; break;
	case 24: errStr = "���򷢳����������Ȳ���ȷ"; break;
	case 25: errStr = "�������޷��ҳ��������ض������ŵ���λ��"; break;
	case 26: errStr = "�޷�����ָ���Ĵ��̻�����"; break;
	case 27: errStr = "�������Ҳ������������"; break;
	case 28: errStr = "��ӡ��ȱֽ"; break;
	case 29: errStr = "ϵͳ�޷�д��ָ�����豸"; break;
	case 30: errStr = "ϵͳ�޷���ָ�����豸�϶�ȡ"; break;
	case 31: errStr = "����ϵͳ�ϵ��豸û�з�������"; break;
	case 32: errStr = "�����޷������ļ�����Ϊ��һ����������ʹ�ô��ļ�"; break;
	case 33: errStr = "�����޷������ļ�����Ϊ��һ�������������ļ���һ����"; break;
	case 36: errStr = "��������Ĵ��ļ�����"; break;
	case 38: errStr = "�����ļ���β"; break;
	case 39: errStr = "��������"; break;
	case 50: errStr = "��֧�ָ�����"; break;
	case 51: errStr = "Զ�̼���������� "; break;
	case 52: errStr = "�������������ظ�������"; break;
	case 53: errStr = "�Ҳ�������·��"; break;
	case 54: errStr = "����æ"; break;
	case 55: errStr = "ָ����������Դ���豸���ٿ���"; break;
	case 56: errStr = "�ѵ������� BIOS ��������"; break;
	case 57: errStr = "����������Ӳ������"; break;
	case 58: errStr = "ָ���ķ������޷���������Ĳ���"; break;
	case 59: errStr = "����������������"; break;
	case 60: errStr = "Զ��������������"; break;
	case 61: errStr = "��ӡ����������"; break;
	case 62: errStr = "�޷��ڷ������ϻ�����ڱ������ӡ�ļ��Ŀռ�"; break;
	case 63: errStr = "ɾ���Ⱥ��ӡ���ļ�"; break;
	case 64: errStr = "ָ�������������ٿ���"; break;
	case 65: errStr = "�ܾ��������"; break;
	case 66: errStr = "������Դ���ʹ���"; break;
	case 67: errStr = "�Ҳ���������"; break;
	case 68: errStr = "�������ؼ������������������"; break;
	case 69: errStr = "�������� BIOS �Ự����"; break;
	case 70: errStr = "Զ�̷���������ͣ������������������"; break;
	case 71: errStr = "��ǰ���޷���ͬ��Զ�̼�������ӣ���Ϊ�Ѵﵽ�������������Ŀ����"; break;
	case 72: errStr = "����ָͣ���Ĵ�ӡ��������豸"; break;
	case 80: errStr = "�ļ�����"; break;
	case 82: errStr = "�޷�����Ŀ¼���ļ�"; break;
	case 83: errStr = "INT 24 ʧ��"; break;
	case 84: errStr = "�޷�ȡ�ô��������Ĵ洢�ռ�"; break;
	case 85: errStr = "�����豸������ʹ����"; break;
	case 86: errStr = "ָ���������������"; break;
	case 87: errStr = "��������"; break;
	case 88: errStr = "�����Ϸ���д�����"; break;
	case 89: errStr = "ϵͳ�޷��ڴ�ʱ������һ������"; break;
	case 100: errStr = "�޷�������һ��ϵͳ�źŵ�"; break;
	case 101: errStr = "��һ������ӵ�ж�ռ���źŵ�"; break;
	case 102: errStr = "�������źŵ����޷��ر�"; break;
	case 103: errStr = "�޷��������źŵ�"; break;
	case 104: errStr = "�޷����ж�ʱ�����ռ���źŵ�"; break;
	case 105: errStr = "���źŵƵ�ǰһ������Ȩ�ѽ���"; break;
	case 107: errStr = "����ֹͣ����Ϊ���������δ����"; break;
	case 108: errStr = "������ʹ���У�����һ����������"; break;
	case 109: errStr = "�ܵ��ѽ���"; break;
	case 110: errStr = "ϵͳ�޷���ָ�����豸���ļ�"; break;
	case 111: errStr = "�ļ���̫��"; break;
	case 112: errStr = "���̿ռ䲻��"; break;
	case 113: errStr = "�޷��ٻ���ڲ��ļ��ı�ʶ"; break;
	case 114: errStr = "Ŀ���ڲ��ļ��ı�ʶ����ȷ"; break;
	case 117: errStr = "Ӧ�ó��������� IOCTL ���ô���"; break;
	case 118: errStr = "��֤д����л�����ֵ����"; break;
	case 119: errStr = "ϵͳ��֧�����������"; break;
	case 120: errStr = "�˹���ֻ����ϵͳ֧��"; break;
	case 121: errStr = "�źŵƳ�ʱʱ���ѵ�"; break;
	case 122: errStr = "���ݵ�ϵͳ���õ�������̫С"; break;
	case 123: errStr = "�ļ�����Ŀ¼�������﷨����ȷ"; break;
	case 124: errStr = "ϵͳ���ü������"; break;
	case 125: errStr = "����û�о��"; break;
	case 126: errStr = "�Ҳ���ָ����ģ��"; break;
	case 127: errStr = "�Ҳ���ָ���ĳ���"; break;
	case 128: errStr = "û�еȺ���ӽ���"; break;
	case 130: errStr = "��ͼʹ�ò���(����ԭʼ���� I/O)���Ѵ򿪴��̷������ļ����"; break;
	case 131: errStr = "��ͼ�ƶ��ļ�ָ�뵽�ļ���ͷ֮ǰ"; break;
	case 132: errStr = "�޷���ָ�����豸���ļ��������ļ�ָ��"; break;
	case 133: errStr = "������ǰ�������������������޷�ʹ�� JOIN �� SUBST ����"; break;
	case 134: errStr = "��ͼ���ѱ��ϲ�����������ʹ�� JOIN �� SUBST ����"; break;
	case 135: errStr = "��ͼ���ѱ��ϲ�����������ʹ�� JOIN �� SUBST ����"; break;
	case 136: errStr = "ϵͳ��ͼ���δ�ϲ��������� JOIN"; break;
	case 137: errStr = "ϵͳ��ͼ���δ����������� SUBST"; break;
	case 138: errStr = "ϵͳ��ͼ���������ϲ����ϲ��������ϵ�Ŀ¼"; break;
	case 139: errStr = "ϵͳ��ͼ�����������Ϊ����������ϵ�Ŀ¼"; break;
	case 140: errStr = "ϵͳ��ͼ���������ϲ�������������ϵ�Ŀ¼"; break;
	case 141: errStr = "ϵͳ��ͼ���������Ϊ�ϲ��������ϵ�Ŀ¼"; break;
	case 142: errStr = "ϵͳ�޷��ڴ�ʱ���� JOIN �� SUBST"; break;
	case 143: errStr = "ϵͳ�޷����������ϲ��������Ϊ��ͬ�������ϵ�Ŀ¼"; break;
	case 144: errStr = "Ŀ¼���Ǹ�Ŀ¼�µ���Ŀ¼"; break;
	case 145: errStr = "Ŀ¼�ǿ�"; break;
	case 146: errStr = "ָ����·�����������ʹ��"; break;
	case 147: errStr = "��Դ���㣬�޷����������"; break;
	case 148: errStr = "ָ����·���޷��ڴ�ʱʹ��"; break;
	case 149: errStr = "��ͼ���������ϲ������Ϊ��������Ŀ¼����һ�������Ŀ���������"; break;
	case 150: errStr = "ϵͳ������Ϣδ�� CONFIG.SYS �ļ���ָ�������������"; break;
	case 151: errStr = "Ϊ DosMuxSemWait ָ�����źŵ��¼���������"; break;
	case 152: errStr = "DosMuxSemWait �������С������ù�����źŵ�"; break;
	case 153: errStr = "DosMuxSemWait �嵥����"; break;
	case 154: errStr = "����ľ�곬��Ŀ���ļ�ϵͳ�ĳ�������"; break;
	case 155: errStr = "�޷�������һ���߳�"; break;
	case 156: errStr = "���ս����Ѿܾ����ź�"; break;
	case 157: errStr = "���ѱ��������޷�����"; break;
	case 158: errStr = "���ѽ������"; break;
	case 159: errStr = "�̱߳�ʶ�ĵ�ַ����"; break;
	case 160: errStr = "���ݵ� DosExecPgm �Ĳ����ַ�������"; break;
	case 161: errStr = "ָ����·����Ч"; break;
	case 162: errStr = "�ź�����ͣ"; break;
	case 164: errStr = "�޷���ϵͳ�д���������߳�"; break;
	case 167: errStr = "�޷������ļ�����"; break;
	case 170: errStr = "�������Դ��ʹ����"; break;
	case 173: errStr = "�����ṩȡ�����������������������"; break;
	case 174: errStr = "�ļ�ϵͳ��֧���������͵���С��Ԫ����"; break;
	case 180: errStr = "ϵͳ��������Ķκ�"; break;
	case 183: errStr = "���ļ��Ѵ���ʱ���޷��������ļ�"; break;
	case 186: errStr = "���ݵı�־����"; break;
	case 187: errStr = "�Ҳ���ָ����ϵͳ�źŵ�����"; break;
	case 196: errStr = "����ϵͳ�޷����д�Ӧ�ó���"; break;
	case 197: errStr = "����ϵͳ��ǰ�����ò������д�Ӧ�ó���"; break;
	case 199: errStr = "����ϵͳ�޷����д�Ӧ�ó���"; break;
	case 200: errStr = "����β��ɴ��ڻ���� 64K"; break;
	case 203: errStr = "����ϵͳ�Ҳ���������Ļ���ѡ��"; break;
	case 205: errStr = "���������еĽ���û���źŴ������"; break;
	case 206: errStr = "�ļ�������չ��̫��"; break;
	case 207: errStr = "�� 2 ����ջ�ѱ�ռ��"; break;
	case 208: errStr = "û����ȷ�����ļ���ͨ��� * �� ?����ָ��������ļ���ͨ���"; break;
	case 209: errStr = "���ڷ��͵��źŴ���"; break;
	case 210: errStr = "�޷������źŴ������"; break;
	case 212: errStr = "�����������޷����·���"; break;
	case 214: errStr = "�����ó����̬����ģ��Ķ�̬����ģ��̫��"; break;
	case 215: errStr = "�޷�Ƕ�׵��� LoadModule"; break;
	case 230: errStr = "�ܵ�״̬��Ч"; break;
	case 231: errStr = "���еĹܵ�ʵ������ʹ����"; break;
	case 232: errStr = "�ܵ����ڹر���"; break;
	case 233: errStr = "�ܵ�����һ�������κν���"; break;
	case 234: errStr = "�������ݿ���"; break;
	case 240: errStr = "ȡ���Ự"; break;
	case 254: errStr = "ָ������չ��������Ч"; break;
	case 255: errStr = "��չ���Բ�һ��"; break;
	case 258: errStr = "�ȴ��Ĳ�����ʱ"; break;
	case 259: errStr = "û�п��õ�������"; break;
	case 266: errStr = "�޷�ʹ�ø��ƹ���"; break;
	case 267: errStr = "Ŀ¼����Ч"; break;
	case 275: errStr = "��չ�����ڻ������в�����"; break;
	case 276: errStr = "װ���ļ�ϵͳ�ϵ���չ�����ļ�����"; break;
	case 277: errStr = "��չ���Ա���ļ�����"; break;
	case 278: errStr = "ָ������չ���Ծ����Ч"; break;
	case 282: errStr = "װ����ļ�ϵͳ��֧����չ����"; break;
	case 288: errStr = "��ͼ�ͷŲ��Ǻ��з���ӵ�еĶ��û��ն����г���"; break;
	case 298: errStr = "�����źŵƵ��������"; break;
	case 299: errStr = "����ɲ��ֵ� ReadProcessMemoty �� WriteProcessMemory ����"; break;
	case 300: errStr = "�����������󱻾ܾ�"; break;
	case 301: errStr = "ϵͳ������һ����Ч�Ĳ�������ȷ��"; break;
	case 487: errStr = "��ͼ������Ч�ĵ�ַ"; break;
	case 534: errStr = "����������� 32 λ"; break;
	case 535: errStr = "�ܵ�����һ����һ����"; break;
	case 536: errStr = "�Ⱥ�򿪹ܵ���һ�˵Ľ���"; break;
	case 994: errStr = "�ܾ�������չ����"; break;
	case 995: errStr = "�����߳��˳���Ӧ�ó��������ѷ��� I/O ����"; break;
	case 996: errStr = "�ص� I/O �¼������ź�״̬��"; break;
	case 997: errStr = "�ص� I/O �����ڽ�����"; break;
	case 998: errStr = "�ڴ���������Ч"; break;
	case 999: errStr = "��������ҳ�ڲ���"; break;
	case 1001: errStr = "�ݹ�̫�ջ���"; break;
	case 1002: errStr = "�����޷����ѷ��͵���Ϣ�ϲ���"; break;
	case 1003: errStr = "�޷���ɴ˹���"; break;
	case 1004: errStr = "��Ч��־"; break;
	case 1005: errStr = "�˾�������ʶ����ļ�ϵͳ����ȷ������������ļ�ϵͳ���������Ѽ��أ��Ҵ˾�δ��"; break;
	case 1006: errStr = "�ļ����ڵľ��ѱ��ⲿ�ı䣬��˴򿪵��ļ�������Ч"; break;
	case 1007: errStr = "�޷���ȫ��Ļģʽ����������Ĳ���"; break;
	case 1008: errStr = "��ͼ���ò����ڵ�����"; break;
	case 1009: errStr = "����ע������ݿ���"; break;
	case 1010: errStr = "����ע�������Ч"; break;
	case 1011: errStr = "�޷�������ע�����"; break;
	case 1012: errStr = "�޷���ȡ����ע�����"; break;
	case 1013: errStr = "�޷�д������ע�����"; break;
	case 1014: errStr = "ע������ݿ��е�ĳһ�ļ�����ʹ�ü�¼������������ָ����ָ��ɹ����"; break;
	case 1015: errStr = "ע����𻵡�����ע������ݵ�ĳһ�ļ��ṹ�𻵣���ϵͳ���ļ��ڴ�ӳ���𻵣�����Ϊ�����������־ȱ�ٻ��𻵶��޷��ָ��ļ�"; break;
	case 1016: errStr = "��ע��������� I/O �����ָ�ʧ�ܡ�ע����޷����롢д�����������һ������ע���ϵͳӳ����ļ�"; break;
	case 1017: errStr = "ϵͳ��ͼ���ػ�ԭ�ļ���ע�����ָ�����ļ�����ע����ļ���ʽ"; break;
	case 1018: errStr = "��ͼ�ڱ��Ϊɾ����ע����������в��Ϸ��Ĳ���"; break;
	case 1019: errStr = "ϵͳ�޷�����ע�����־��������Ŀռ�"; break;
	case 1020: errStr = "�޷������������ֵ��ע������д�����������"; break;
	case 1021: errStr = "�޷����ױ丸���´����ȶ�����"; break;
	case 1022: errStr = "֪ͨ����������������У�����Ϣ��δ���ص����з��Ļ������С���ǰ���з�����ö���ļ������Ҹ���"; break;
	case 1051: errStr = "�ѷ���ֹͣ���Ƶ����񣬸÷��������������еķ���������"; break;
	case 1052: errStr = "����Ŀؼ��Դ˷�����Ч"; break;
	case 1053: errStr = "����δ��ʱ��Ӧ�������������"; break;
	case 1054: errStr = "�޷������˷�����߳�"; break;
	case 1055: errStr = "�����������ݿ�"; break;
	case 1056: errStr = "�����ʵ������������"; break;
	case 1057: errStr = "�ʻ�����Ч�򲻴��ڣ������������ָ�����ʻ�����Ч"; break;
	case 1058: errStr = "�޷���������ԭ��������������û�������������豸û������"; break;
	case 1059: errStr = "ָ����ѭ����������"; break;
	case 1060: errStr = "ָ���ķ���δ���Ѱ�װ�ķ������"; break;
	case 1061: errStr = "�����޷��ڴ�ʱ���ܿ�����Ϣ"; break;
	case 1062: errStr = "����δ����"; break;
	case 1063: errStr = "��������޷����ӵ������������"; break;
	case 1064: errStr = "�������������ʱ���ڷ����з����쳣"; break;
	case 1065: errStr = "ָ�������ݿⲻ����"; break;
	case 1066: errStr = "�����ѷ����ض��ķ��������"; break;
	case 1067: errStr = "����������ֹ"; break;
	case 1068: errStr = "�����������޷�����"; break;
	case 1069: errStr = "���ڵ�¼ʧ�ܶ��޷���������"; break;
	case 1070: errStr = "�����󣬷���ͣ����������ͣ״̬"; break;
	case 1071: errStr = "ָ���ķ������ݿ�������Ч"; break;
	case 1072: errStr = "ָ���ķ����ѱ��Ϊɾ��"; break;
	case 1073: errStr = "ָ���ķ����Ѵ���"; break;
	case 1074: errStr = "ϵͳ��ǰ�����µ���Ч��������"; break;
	case 1075: errStr = "������񲻴��ڣ����ѱ����Ϊɾ��"; break;
	case 1076: errStr = "�ѽ���ʹ�õ�ǰ������Ϊ������Ч��������"; break;
	case 1077: errStr = "�ϴ�����֮����δ������������"; break;
	case 1078: errStr = "�����������������������ʾ��"; break;
	case 1079: errStr = "�˷�����ʻ���ͬ��������ͬһ�����ϵ�����������ʻ�"; break;
	case 1080: errStr = "ֻ��Ϊ Win32 ��������ʧ�ܲ���������Ϊ������������"; break;
	case 1081: errStr = "������������еĴ���ͷ�����ƹ�������ͬ�����ԣ�������������������ֹ�Ļ���������ƹ������޷������κβ���"; break;
	case 1082: errStr = "���������δ���ûָ�����"; break;
	case 1083: errStr = "���ó��ڸÿ�ִ�г��������е����������ִ�и÷���"; break;
	case 1100: errStr = "�Ѵ�Ŵ���ʵ�ʽ�β"; break;
	case 1101: errStr = "�Ŵ������Ѵ��ļ����"; break;
	case 1102: errStr = "�Ѵ�Ŵ�����̷����Ŀ�ͷ"; break;
	case 1103: errStr = "�Ŵ������Ѵ�һ���ļ��Ľ�β"; break;
	case 1104: errStr = "�Ŵ��ϲ������κ�����"; break;
	case 1105: errStr = "�Ŵ��޷�����"; break;
	case 1106: errStr = "�ڷ��ʶ��������´Ŵ�ʱ����ǰ�Ŀ��С����ȷ"; break;
	case 1107: errStr = "�����شŴ�ʱ���Ҳ���������Ϣ"; break;
	case 1108: errStr = "�޷�����ý�嵯������"; break;
	case 1109: errStr = "�޷�ж�ؽ���"; break;
	case 1110: errStr = "�������еĽ��ʿ����Ѹ���"; break;
	case 1111: errStr = "��λ I/O ����"; break;
	case 1112: errStr = "��������û��ý��"; break;
	case 1113: errStr = "�ڶ��ֽڵ�Ŀ�����ҳ�У�û�д� Unicode �ַ�����ӳ�䵽���ַ�"; break;
	case 1114: errStr = "��̬���ӿ� (DLL) ��ʼ������ʧ��"; break;
	case 1115: errStr = "ϵͳ�ػ����ڽ���"; break;
	case 1116: errStr = "��Ϊû���κν����еĹػ����̣������޷��ж�ϵͳ�ػ�"; break;
	case 1117: errStr = "��Ϊ I/O �豸���������޷����д�������"; break;
	case 1118: errStr = "û�д����豸����ʼ���ɹ���������������ж��"; break;
	case 1119: errStr = "�޷��������������豸�����ж�����(IRQ)���豸��������һ��ʹ�ø� IRQ �������豸�Ѵ�"; break;
	case 1120: errStr = "���� I/O ����������һ�����пڵ�д����ɡ�(IOCTL_SERIAL_XOFF_COUNTER �Ѵ���)"; break;
	case 1121: errStr = "��Ϊ�ѹ���ʱʱ�䣬���Դ��� I/O ������ɡ�(IOCTL_SERIAL_XOFF_COUNTER δ����)"; break;
	case 1122: errStr = "���������Ҳ��� ID ��ַ���"; break;
	case 1123: errStr = "�������� ID �ַ��������̿������ŵ���ַ�����"; break;
	case 1124: errStr = "���̿�����������������������ʶ��Ĵ���"; break;
	case 1125: errStr = "���̿�������������Ĵ����в�һ�µĽ��"; break;
	case 1126: errStr = "������Ӳ��ʱ������У׼����ʧ�ܣ�������Ȼʧ��"; break;
	case 1127: errStr = "������Ӳ��ʱ�����̲���ʧ�ܣ�������Ȼʧ��"; break;
	case 1128: errStr = "������Ӳ��ʱ����ʹʧ�ܣ����븴λ���̿�����"; break;
	case 1129: errStr = "�Ѵ�Ŵ���β"; break;
	case 1130: errStr = "�������洢�ռ䲻�㣬�޷����������"; break;
	case 1131: errStr = "����Ǳ�ڵ�����״̬"; break;
	case 1132: errStr = "ָ���Ļ�ַ���ļ�ƫ����û���ʵ�����"; break;
	case 1140: errStr = "�ı�ϵͳ����״̬�ĳ��Ա���һӦ�ó��������������"; break;
	case 1141: errStr = "ϵͳ BIOS �ı�ϵͳ����״̬�ĳ���ʧ��"; break;
	case 1142: errStr = "��ͼ��һ�ļ��ϴ�������ϵͳ�������������"; break;
	case 1150: errStr = "ָ������Ҫ����µ� Windows �汾"; break;
	case 1151: errStr = "ָ�������� Windows �� MS-DOS ����"; break;
	case 1152: errStr = "ֻ��������ָ�������һ��ʵ��"; break;
	case 1153: errStr = "��ָ�����������ھɵ� Windows �汾"; break;
	case 1154: errStr = "ִ�и�Ӧ�ó�������Ŀ��ļ�֮һ����"; break;
	case 1155: errStr = "û��Ӧ�ó�����˲�����ָ���ļ��й���"; break;
	case 1156: errStr = "������ָ�Ӧ�ó���Ĺ����г��ִ���"; break;
	case 1157: errStr = "ִ�и�Ӧ�ó�������Ŀ��ļ�֮һ�޷��ҵ�"; break;
	case 1158: errStr = "��ǰ������ʹ���� Window �����������ϵͳ��������о��"; break;
	case 1159: errStr = "��Ϣֻ����ͬ������һ��ʹ��"; break;
	case 1160: errStr = "ָ����ԴԪ��û��ý��"; break;
	case 1161: errStr = "ָ����Ŀ��Ԫ���Ѱ���ý��"; break;
	case 1162: errStr = "ָ����Ԫ�ز�����"; break;
	case 1163: errStr = "ָ����Ԫ����δ��ʾ�Ĵ洢��Դ��һ����"; break;
	case 1164: errStr = "��ʾ�豸��Ҫ���³�ʼ������ΪӲ���д���"; break;
	case 1165: errStr = "�豸��ʾ�ڳ��Խ�һ������֮ǰ��Ҫ���"; break;
	case 1166: errStr = "�豸��ʾ���������Ǵ�״̬"; break;
	case 1167: errStr = "�豸û������"; break;
	case 1168: errStr = "�Ҳ���Ԫ��"; break;
	case 1169: errStr = "������û��ָͬ������ƥ�����"; break;
	case 1170: errStr = "�ڶ����ϲ�����ָ�������Լ�"; break;
	case 1171: errStr = "���ݵ� GetMouseMovePoints �ĵ㲻�ڻ�������"; break;
	case 1172: errStr = "����(����վ)����û����"; break;
	case 1173: errStr = "�Ҳ����� ID"; break;
	case 1175: errStr = "�޷�ɾ��Ҫ���滻���ļ�"; break;
	case 1176: errStr = "�޷����滻�ļ��Ƶ�Ҫ���滻���ļ���Ҫ���滻���ļ�����ԭ��������"; break;
	case 1177: errStr = "�޷����滻�ļ��Ƶ�Ҫ���滻���ļ���Ҫ���滻���ļ��ѱ���������Ϊ��������"; break;
	case 1178: errStr = "����ļ�¼��ɾ��"; break;
	case 1179: errStr = "����ļ�¼���񲻴��ڻ��"; break;
	case 1180: errStr = "�ҵ�һ���ļ������ǿ��ܲ�����ȷ���ļ�"; break;
	case 1181: errStr = "��־�����־�б�ɾ��"; break;
	case 1200: errStr = "ָ�����豸����Ч"; break;
	case 1201: errStr = "�豸��ǰδ�����ϣ�����Ϊһ����¼����"; break;
	case 1202: errStr = "��ͼ��¼��ǰ�ѱ���¼���豸"; break;
	case 1203: errStr = "���κ������ṩ�������ָ��������·��"; break;
	case 1204: errStr = "ָ���������ṩ����������Ч"; break;
	case 1205: errStr = "�޷����������������ļ�"; break;
	case 1206: errStr = "�������������ļ���"; break;
	case 1207: errStr = "�޷�ö�ٿ�����"; break;
	case 1208: errStr = "������չ����"; break;
	case 1209: errStr = "ָ����������ʽ��Ч"; break;
	case 1210: errStr = "ָ���ļ��������ʽ��Ч"; break;
	case 1211: errStr = "ָ�����¼�����ʽ��Ч"; break;
	case 1212: errStr = "ָ����������ʽ��Ч"; break;
	case 1213: errStr = "ָ���ķ�������ʽ��Ч"; break;
	case 1214: errStr = "ָ������������ʽ��Ч"; break;
	case 1215: errStr = "ָ���Ĺ�������ʽ��Ч"; break;
	case 1216: errStr = "ָ���������ʽ��Ч"; break;
	case 1217: errStr = "ָ������Ϣ����ʽ��Ч"; break;
	case 1218: errStr = "ָ������ϢĿ���ʽ��Ч"; break;
	case 1219: errStr = "�ṩ��ƾ�����Ѵ��ڵ�ƾ�ݼ���ͻ"; break;
	case 1220: errStr = "��ͼ��������������ĻỰ�����ѶԸ÷�������������ĻỰ"; break;
	case 1221: errStr = "��������������������ϵ���һ�������ʹ��"; break;
	case 1222: errStr = "����δ���ӻ�����"; break;
	case 1223: errStr = "�����ѱ��û�ȡ��"; break;
	case 1224: errStr = "����Ĳ����޷���ʹ���û�ӳ������򿪵��ļ���ִ��"; break;
	case 1225: errStr = "Զ��ϵͳ�ܾ���������"; break;
	case 1226: errStr = "���������ѱ��ʵ��عر���"; break;
	case 1227: errStr = "���紫���ս��������������ĵ�ַ"; break;
	case 1228: errStr = "��ַ��δ�������ս�����"; break;
	case 1229: errStr = "��ͼ�ڲ����ڵ����������Ͻ��в���"; break;
	case 1230: errStr = "��ͼ��ʹ���е����������Ͻ�����Ч�Ĳ���"; break;
	case 1231: errStr = "���ܷ�������λ�á��й������ų����ϵ���Ϣ������� Windows ����"; break;
	case 1232: errStr = "���ܷ�������λ�á��й������ų����ϵ���Ϣ������� Windows ����"; break;
	case 1233: errStr = "���ܷ�������λ�á��й������ų����ϵ���Ϣ������� Windows ����"; break;
	case 1234: errStr = "û���κη�������Զ��ϵͳ�ϵ�Ŀ�������ս���ϲ���"; break;
	case 1235: errStr = "������ֹ"; break;
	case 1236: errStr = "�ɱ���ϵͳ��ֹ��������"; break;
	case 1237: errStr = "�����޷���ɡ�Ӧ������"; break;
	case 1238: errStr = "��Ϊ�Ѵﵽ���ʻ������ͬʱ���������ƣ������޷����ӷ�����"; break;
	case 1239: errStr = "��ͼ������ʻ�δ����Ȩ��ʱ���ڵ�¼"; break;
	case 1240: errStr = "���ʻ���δ�õ����������վ��¼����Ȩ"; break;
	case 1241: errStr = "����Ĳ�������ʹ����������ַ"; break;
	case 1242: errStr = "�������Ѿ�ע��"; break;
	case 1243: errStr = "ָ���ķ��񲻴���"; break;
	case 1244: errStr = "��Ϊ�û���δ����֤������ִ����Ҫ��Ĳ���"; break;
	case 1245: errStr = "��Ϊ�û���δ��¼���磬����ִ����Ҫ��Ĳ�����ָ���ķ��񲻴���"; break;
	case 1246: errStr = "���ڼ�������"; break;
	case 1247: errStr = "��ͼ���г�ʼ���������ǳ�ʼ�������"; break;
	case 1248: errStr = "û�и���ı����豸"; break;
	case 1249: errStr = "ָ����վ�㲻����"; break;
	case 1250: errStr = "����ָ�����Ƶ���������Ѿ�����"; break;
	case 1251: errStr = "ֻ�����ӵ���������ʱ���ò�������֧��"; break;
	case 1252: errStr = "��ʹû�иĶ�������Կ��ҲӦ�õ�����չ"; break;
	case 1253: errStr = "ָ�����û�û��һ����Ч�������ļ�"; break;
	case 1254: errStr = "Microsoft Small Business Server ��֧�ִ˲���"; break;
	case 1300: errStr = "�������б����õ���Ȩ��ָ�ɸ����з�"; break;
	case 1301: errStr = "�ʻ����Ͱ�ȫ��ʶ���ĳЩӳ��δ���"; break;
	case 1302: errStr = "û��Ϊ���ʻ��ر�����ϵͳ�������"; break;
	case 1303: errStr = "û�п��õļ�����Կ��������һ����֪������Կ"; break;
	case 1304: errStr = "����̫���ӣ��޷�ת���� LAN Manager ���롣���ص� LAN Manager ����Ϊ���ַ���"; break;
	case 1305: errStr = "�޶�����δ֪"; break;
	case 1306: errStr = "���������޶������ǲ����ݵ�"; break;
	case 1307: errStr = "�����ȫ��ʶ����ָ��Ϊ�˶����������"; break;
	case 1308: errStr = "�����ȫ��ʶ����ָ��Ϊ�������Ҫ��"; break;
	case 1309: errStr = "��ǰ��δģ��ͻ����߳���ͼ����ģ������"; break;
	case 1310: errStr = "�����δ������"; break;
	case 1311: errStr = "��ǰû�п��õĵ�¼�������������¼����"; break;
	case 1312: errStr = "ָ���ĵ�¼�Ự�����ڡ������ѱ���ֹ"; break;
	case 1313: errStr = "ָ������Ȩ������"; break;
	case 1314: errStr = "�ͻ�û���������Ȩ"; break;
	case 1315: errStr = "�ṩ�����Ʋ�����ȷ���ʻ�����ʽ"; break;
	case 1316: errStr = "ָ�����û��Ѵ���"; break;
	case 1317: errStr = "ָ�����û�������"; break;
	case 1318: errStr = "ָ�������Ѵ���"; break;
	case 1319: errStr = "ָ�����鲻����"; break;
	case 1320: errStr = "ָ�����û��ʻ�����ָ����ĳ�Ա��������Ϊ�������Ա�����޷�ɾ��ָ������"; break;
	case 1321: errStr = "ָ�����û��ʻ�����ָ�����ʻ��ĳ�Ա"; break;
	case 1322: errStr = "�޷����û�ɾ�����ʣ���ϵͳ�����ʻ�"; break;
	case 1323: errStr = "�޷��������롣�ṩ��Ϊ��ǰ�����ֵ����ȷ"; break;
	case 1324: errStr = "�޷��������롣�ṩ���������ֵ���������в������ֵ"; break;
	case 1325: errStr = "�޷��������롣Ϊ�������ṩ��ֵ�������ַ���ĳ��ȡ������Ի���ʷҪ��"; break;
	case 1326: errStr = "��¼ʧ��: δ֪���û������������"; break;
	case 1327: errStr = "��¼ʧ��: �û��ʻ�����"; break;
	case 1328: errStr = "��¼ʧ��: Υ���ʻ���¼ʱ������"; break;
	case 1329: errStr = "��¼ʧ��: �������û���¼���˼����"; break;
	case 1330: errStr = "��¼ʧ��: ָ�����ʻ������ѹ���"; break;
	case 1331: errStr = "��¼ʧ��: ���õ�ǰ���ʻ�"; break;
	case 1332: errStr = "�ʻ����밲ȫ��ʶ�����κ�ӳ�����"; break;
	case 1333: errStr = "һ���������ı����û���ʶ��(LUIDs)"; break;
	case 1334: errStr = "�޸�����õı����û���ʶ��(LUIDs)"; break;
	case 1335: errStr = "���ڸ��ر��÷�����ȫ ID �Ĵμ���Ȩ������Ч"; break;
	case 1336: errStr = "���ʿ����б�(ACL)�ṹ��Ч"; break;
	case 1337: errStr = "��ȫ ID �ṹ��Ч"; break;
	case 1338: errStr = "��ȫ�������ṹ��Ч"; break;
	case 1340: errStr = "�޷��������еķ��ʿ����б�(ACL)����ʿ�����Ŀ(ACE)"; break;
	case 1341: errStr = "��������ǰ�ѽ���"; break;
	case 1342: errStr = "��������ǰ������"; break;
	case 1343: errStr = "�ṩ��ʶ����Ű䷢������ֵΪ��Чֵ"; break;
	case 1344: errStr = "�޸�����õ��ڴ��Ը��°�ȫ��Ϣ"; break;
	case 1345: errStr = "ָ��������Ч����������Ⱥ������Բ�����"; break;
	case 1346: errStr = "ָ����ģ�⼶����Ч�� �����ṩ��ģ�⼶����Ч"; break;
	case 1347: errStr = "�޷�����������ȫ����"; break;
	case 1348: errStr = "�������֤��Ϣ�����Ч"; break;
	case 1349: errStr = "���Ƶ����Ͷ��䳢��ʹ�õķ������ʵ�"; break;
	case 1350: errStr = "�޷����밲ȫ���޹����Ķ��������а�ȫ�Բ���"; break;
	case 1351: errStr = "δ�ܴ����������ȡ������Ϣ����������Ϊ��������ʹ�ã������Ƿ��ʱ��ܾ�"; break;
	case 1352: errStr = "��ȫ�ʻ�������(SAM)�򱾵ذ�ȫ�䷢����(LSA)�������������а�ȫ�����Ĵ���״̬"; break;
	case 1353: errStr = "�������а�ȫ�����Ĵ���״̬"; break;
	case 1354: errStr = "�˲���ֻ�������Ҫ�����������"; break;
	case 1355: errStr = "ָ�����򲻴��ڣ����޷���ϵ"; break;
	case 1356: errStr = "ָ�������Ѵ���"; break;
	case 1357: errStr = "��ͼ����ÿ�����������������"; break;
	case 1358: errStr = "�޷���������������Ϊ�����ϵ����ؽ���ʧ�ܻ����ݽṹ��"; break;
	case 1359: errStr = "�������ڲ�����"; break;
	case 1360: errStr = "ͨ�÷������Ͱ�������ӳ�䵽��ͨ�����͵ķ���������"; break;
	case 1361: errStr = "��ȫ��������ʽ����ȷ (���Ի�����ص�)"; break;
	case 1362: errStr = "�������ֻ�����ڵ�¼������ʹ�á����ý���δע��Ϊһ����¼����"; break;
	case 1363: errStr = "�޷�ʹ������ʹ���еı�ʶ�����µĻỰ"; break;
	case 1364: errStr = "δ֪��ָ����֤���ݰ�"; break;
	case 1365: errStr = "��¼�Ự���Ǵ������������һ�µ�״̬��"; break;
	case 1366: errStr = "��¼�Ự��ʶ����ʹ����"; break;
	case 1367: errStr = "��¼���������Ч�ĵ�¼����ֵ"; break;
	case 1368: errStr = "��ʹ�������ܵ���ȡ����֮ǰ���޷����ɸùܵ�ģ��"; break;
	case 1369: errStr = "ע���������������״̬������״̬��һ��"; break;
	case 1370: errStr = "��ȫ�����ݿ��ڲ�������"; break;
	case 1371: errStr = "�޷��������ʻ������д˲���"; break;
	case 1372: errStr = "�޷������������������д˲���"; break;
	case 1373: errStr = "�޷������������û������д˲���"; break;
	case 1374: errStr = "�޷�������ɾ���û�����Ϊ��ǰ��Ϊ�û�����Ҫ��"; break;
	case 1375: errStr = "��������Ϊ��Ҫ����ʹ��"; break;
	case 1376: errStr = "ָ���ı����鲻����"; break;
	case 1377: errStr = "ָ�����ʻ������Ǳ�����ĳ�Ա"; break;
	case 1378: errStr = "ָ�����ʻ������Ǳ�����ĳ�Ա"; break;
	case 1379: errStr = "ָ���ı������Ѵ���"; break;
	case 1380: errStr = "��¼ʧ��: δ�����û��ڴ˼�����ϵ������¼����"; break;
	case 1381: errStr = "�ѳ����ڵ�һϵͳ�пɱ�����ܵ�������"; break;
	case 1382: errStr = "���ܵĳ��ȳ����������󳤶�"; break;
	case 1383: errStr = "���ذ�ȫ�䷢�������ݿ��ڲ�������һ����"; break;
	case 1384: errStr = "�ڳ��Ե�¼�Ĺ����У��û��İ�ȫ�����Ļ����˹���İ�ȫ��ʶ"; break;
	case 1385: errStr = "��¼ʧ��: δ�����û��ڴ˼�����ϵ������¼����"; break;
	case 1386: errStr = "�����û�����ʱ��Ҫ�����������"; break;
	case 1387: errStr = "���ڳ�Ա�����ڣ��޷�����Ա��ӵ��������У�Ҳ�޷��ӱ����齫��ɾ��"; break;
	case 1388: errStr = "�޷����³�Ա���뵽�������У���Ϊ��Ա���ʻ����ʹ���"; break;
	case 1389: errStr = "��ָ������İ�ȫ��ʶ"; break;
	case 1390: errStr = "���Ĵ��û�����ʱ��Ҫ�����������"; break;
	case 1391: errStr = "���� ACL δ�����κοɳм̵����"; break;
	case 1392: errStr = "�ļ���Ŀ¼�����޷���ȡ"; break;
	case 1393: errStr = "���̽ṹ�����޷���ȡ"; break;
	case 1394: errStr = "���κ�ָ����¼�Ự���û��Ự��"; break;
	case 1395: errStr = "���ڷ��ʵķ�����������Ŀ����Ȩ���ơ���ʱ���Ѿ��޷������ӣ�ԭ�����Ѿ�����ɽ��ܵ�������Ŀ����"; break;
	case 1396: errStr = "��¼ʧ��: ��Ŀ���ʻ����Ʋ���ȷ"; break;
	case 1397: errStr = "�໥�����֤ʧ�ܡ��÷�����������������������"; break;
	case 1398: errStr = "�ڿͻ����ͷ�����֮����һ��ʱ���"; break;
	default:   errStr = "δ֪ϵͳ����"; break;
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
	default:  errStr = "δ֪ϵͳ����"; break;
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
		errStr = "�����ɹ��ˣ����ǻ���Щ������Ҫ������һ������";
		break;
	case VX_SUCCESS_ADDDEFAULT:
		errStr = "�����ɹ��ˣ����������һЩ��������";
		break;
	case VX_SUCCESS_NODEFAULT:
		errStr = "�����ɹ��ˣ�������Щ���е����û�г���";
		break;
	case VX_SUCCESS_NORT:
		errStr = "�����ɹ��ˣ�������Щ�ط���RT��ɷ�RT";
		break;
	case VX_SUCCESS:
		errStr = "�����ɹ��ˣ�û�з����κ�����";
		break;
	case VX_ERROR:
		errStr = "����û�гɹ�����֪��ԭ��";
		break;
	case VX_E_NOTIMPL:
		errStr = "�ӿ�δʵ��";
		break;
	case VX_E_UNDEFINE:
		errStr = "δ���������";
		break;
	case VX_E_CLIP_DURATING:
		errStr = "Ƭ�εĳ���ʱ�䲻����";
		break;
	case VX_E_OUTOFMEMORY:
		errStr = "�ڴ治��";
		break;
	case VX_E_NOTFOUND:
		errStr = "û�з���Ѱ�ҵĶ���";
		break;
	case VX_E_NOIMPLEMENT:
		errStr = "�������û��ʵ��";
		break;
	case VX_E_OUTOFSLOT:
		errStr = "�б�ﵽ����������";
		break;
	case VX_E_LOSTCONNECT:
		errStr = "���ݿ�ʧȥ����";
		break;
	case VX_E_BUFFERNOTENOUGH:
		errStr = "���ݻ�����������";
		break;
	case VX_E_OBJECTNOTENOUGT:
		errStr = "�������󲻹���";
		break;
	case VX_E_INVALIDPARAM:
		errStr = "�������Ϸ�";
		break;
	case VX_E_TRACKNOSPACE:
		errStr = "�����û�к��ʵĴ�ſռ�";
		break;
	case VX_E_INVALIDFILE:
		errStr = "�ļ���ʽ����";
		break;
	case VX_E_NETSAVEFAILED:
		errStr = "�������ϱ����ļ�ʧ��";
		break;
	case VX_E_TYPENOTMATCH:
		errStr = "���Ͳ�ƥ��";
		break;
	case VX_E_NOFILENAME:
		errStr = "���������Ҫ�Ѿ�������ļ�";
		break;
	case VX_E_CANLINKSELF:
		errStr = "�����������Լ�������";
		break;
	case VX_E_NOTMATTERFILE:
		errStr = "ָ���ļ������ز��ļ�";
		break;
	case VX_E_NOTATTACHHEL:
		errStr = "��ǰ��������û��������������";
		break;
	case VX_E_GFBINVALID:
		errStr = "GFB����������⣬����׼������";
		break;
	case VX_E_CGINVALID:
		errStr = "CG����������⣬����׼������";
		break;
	case VX_E_NOTCUE:
		errStr = "�ڲ�����ǰȱ��׼�����̣����ܽ��벥��״̬";
		break;
	case VX_E_OUTOFHWRESOURCE:
		errStr = "Ӳ����Դ����";
		break;
	case VX_E_CGEFXFACTORYLOST:
		errStr = "ʧȥ����Ļ���๤������û��װ�سɹ�";
		break;
	case VX_E_FORECASEFAILED:
		errStr = "Ԥ��ʧ��";
		break;
	case VX_E_CGRENDERLOST:
		errStr = "CGRenderToolû�м��ؽ���";
		break;
	case VX_E_CGPRAVATELOST:
		errStr = "CGPrivateFactoryû�м��ؽ���";
		break;
	case VX_E_EFXNOTFOUND:
		errStr = "ָ����EFXû��ע��";
		break;
	case VX_E_STATENOTMATCH:
		errStr = "״̬����ȷ";
		break;
	case VX_E_INVALIDSOURCE:
		errStr = "�ڲ�����Դ������";
		break;
	case VX_E_HAVEOWNER:
		errStr = "������������������������������";
		break;
	case VX_E_HAVESIBLING:
		errStr = "������������ֵܣ��������������";
		break;
	case VX_E_INVALIDCOREHANDLE:
		errStr = "���Ķ���ľ������ʶ������ģ����Ļ�е�HTEMPLCG";
		break;
	case VX_E_RENDERPAGEFAILED:
		errStr = "������Ļָ����ҳ��Ⱦʧ��";
		break;
	case VX_E_INVALIDFORMAT:
		errStr = "�������ĸ�ʽ���ܽ����������";
		break;
	case VX_E_WRITEFILEERROR:
		errStr = "д�ļ�ʧ��,������д�����ٶ��������������";
		break;
	case VX_E_READFILEERROR:
		errStr = "���ļ�ʧ��";
		break;
	case VX_E_INVALIDVERSION:
		errStr = "����ȷ�İ汾";
		break;
	case VX_E_INVALIDHANDLE:
		errStr = "����ȷ�ľ�����˲���";
		break;
	case VX_E_INVALIDPTR:
		errStr = "��Ч��ָ�����";
		break;
	case VX_E_INNERPARAMERR:
		errStr = "�ڲ���������";
		break;
	case VX_E_INVALIDCGFACEUP:
		errStr = "ICGFaceUPû����ȷ��ʼ��";
		break;
	case VX_E_NOINITIALIZE:
		errStr = "û�г�ʼ��";
		break;
	case VX_E_NOREPLY:
		errStr = "��ʱ��û��Ӧ��";
		break;
	case VX_E_EDITORBUSY:
		errStr = "�༭����æ";
		break;
	case VX_E_DEFAULTTEMPLLOST:
		errStr = "��ʧ��ȱʡ����";
		break;
	case VX_E_CGSERVERLOADFAIL:
		errStr = "CG�༭������װ���ļ�ʧ��";
		break;
	case VX_E_CGSERVERNOTFOUND:
		errStr = "CG�༭������û���ҵ�";
		break;
	case VX_E_CLIPSNUMBER:
		errStr = "Ƭ����Ŀ����";
		break;
	case VX_E_CLIPSLOCKED:
		errStr = "��ЩƬ�α���ס��";
		break;
	case VX_E_RESERVEPATH:
		errStr = "��Щ·���Ǳ����ģ�����������";
		break;
	case VX_E_INVALIDEPATH:
		errStr = "��Ч·��";
		break;
	case VX_E_BADFILE:
		errStr = "�ļ������Ǵ����";
		break;
	case VX_E_INVALIDTRIM:
		errStr = "Ƭ�ε������������";
		break;
	case VX_E_VIEWNOTACTIVE:
		errStr = "�����ͼ���ǻ��";
		break;
	case VX_E_HARDNOTFOUND:
		errStr = "Ӳ������û�г�ʼ��";
		break;
	case VX_E_HARDBUSY:
		errStr = "Ӳ��������æ";
		break;
	case VX_E_ENGINELACK:
		errStr = "ȱ�ٱ�Ҫ�ĵײ�����";
		break;
	case VX_E_DATAINCHANGING:
		errStr = "���ݺ����������޸�״̬��";
		break;
		// 	case VX_E_CMDSUCCESS:
		// 		errStr = "������ȷִ�����Ѿ��յ���";
		// 		break;

		// 	case VX_E_CMDTESTFUTHER:
		// 		errStr = "������ȷ���ͣ�������ʱû�лش�";
		// 		break;
	case VX_E_CMDENGINEBUSY:
		errStr = "������ָ��ʱ����û�е������״̬������ִ������";
		break;
	case VX_E_CMDUNKNOWN:
		errStr = "����ʶ������";
		break;
	case VX_E_CMDENGINEFREE:
		errStr = "�������ڿ���״̬������ִ������";
		break;
	case VX_E_CMDNOREPLY:
		errStr = "����������ȷ������������ָ��ʱ����û���յ���";
		break;
	case VX_E_CMDNORESPOND:
		errStr = "����������ܾã���������ȴû�н���";
		break;
	case VX_E_UD_CANCEL:
		errStr = "�������汻��������ֹͣ���Ŵ���û���κκۼ�";
		break;
	case VX_E_UD_ABORT:
		errStr = "�������汻��������ֹͣ���Ŵ����в����Ѿ�����";
		break;
	case VX_E_UD_VTRNOTLOCKSERVO:
		errStr = "VTR�޷�����ƽ������״̬�������ǴŴ��ϵ�ʱ���벻����������";
		break;
	case VX_E_UD_VTRNOTREPOUND:
		errStr = "VTR�޷��ܿؽ���ָ��״̬";
		break;
	case VX_E_UD_VTRINPOINTMISS:
		errStr = "VTR�޷�����ָ�������";
		break;
	case VX_E_UD_VTRTIMEMISS:
		errStr = "VTR�޷�ȡ��ʱ�䣬���������������⣬Ҳ������ϵͳ���������е�����̫��";
		break;
	case VX_E_UD_TIMEOVERFLOW:
		errStr = "������׼�������У�Ҫ���һЩ״̬�������ƥ�䣬�ͻ�ֹͣ��,һ���Ǵ����ʱ��";
		break;
	case VX_E_UD_VTRNOTLINKED:
		errStr = "VTRû�г�ʼ���ɹ�";
		break;
	case VX_E_UD_LACKCTLCODE:
		errStr = "����ȱ��CTL�룬��Щ���ܲ������";
		break;
	case VX_E_UD_NOTRESPOND:
		errStr = "����û�б���ȷ�ش𣬿�����̫æ������";
		break;
	case VX_E_UD_SINKLACK:
		errStr = "��û�����òɼ�Ŀ��";
		break;
	case VX_E_UD_VTRNOTINLOCAL:
		errStr = "VTR���ڼ�����Ŀ����£���Ҫ�ֹ���";
		break;
	case VX_E_UD_NOTSUPPORTTCMOD:
		errStr = "VTR��֧��ָ����ʱ���ʽ";
		break;
	case VX_E_UD_HWSTATESWITCH:
		errStr = "Ӳ����״̬�л����̴���һ���ǲɼ����ط��л�";
		break;
	case VX_E_UD_VTROPTIONMISS:
		errStr = "VTR�Ĳ�����׼����ҪУ׼";
		break;
	case VX_E_UD_VTRTESTERR:
		errStr = "VTR�Բ�ʧ��";
		break;
	case VX_E_UD_VTRINLOCK:
		errStr = "���ں��ĵ���Ҫ��ĿǰVTR����ס";
		break;
	case VX_E_UD_VIDBUFOVERFLOW:
		errStr = "�ɼ�ѹ����Ƶ�ڴ�����,������ϵͳ���ܴﲻ��ѹ��Ҫ��";
		break;
	case VX_E_UD_AUDBUFOVERFLOW:
		errStr = "�ɼ�ѹ����Ƶ�ڴ�����,������ϵͳ���ܴﲻ��ѹ��Ҫ��";
		break;
	case VX_E_UD_LOWDISKSPEED:
		errStr = "Ӳ�����ʲ���";
		break;
	case VX_E_UD_VSINGNALERROR:
		errStr = "��Ƶ�����źŲ���ȫ�ж�֡";
		break;
	case VX_E_UD_ASINGNALERROR:
		errStr = "��Ƶ�����źŲ���ȫ�ж�֡	";
		break;
	case VX_E_ENC_MUXER_FAIL:
		errStr = "��������ʼ����ʧ��";
		break;
	case VX_E_ENC_VIDENC_FAIL:
		errStr = "��Ƶѹ����ʼ��ʧ��";
		break;
	case VX_E_ENC_AUDENC_FAIL:
		errStr = "��Ƶѹ����ʼ��ʧ��";
		break;
	case VX_E_ENC_INVALIDDEST:
		errStr = "��Ч������Ŀ��";
		break;
	case VX_E_ENC_INVALIDMUXER:
		errStr = "��Ч�ĸ�����";
		break;
	case VX_E_ENC_EXPORT_FAIL:
		errStr = "ѹ����������ʼ��ʧ��";
		break;
	case VX_E_ENC_VIDENC_BEGIN_FAIL:
		errStr = "������Ƶѹ��ʧ��";
		break;
	case VX_E_ENC_AUDENC_BEGIN_FAIL:
		errStr = "������Ƶѹ��ʧ��";
		break;
	case VX_E_ENC_CODEC_MISSMATCH:
		errStr = "ѹ�����͸�������ƥ��";
		break;
	case VX_E_ENC_MUXER_MISS:
		errStr = "������������";
		break;
	case VX_E_ENC_VSETTING_FAIL:
		errStr = "�������Ƶѹ������";
		break;
	case VX_E_ENC_ASETTING_FAIL:
		errStr = "�������Ƶѹ������";
		break;
	case VX_E_ENC_NOSPLITECAPTURE:
		errStr = "��������֧�ֶַβɼ�";
		break;
	case VX_E_EFX_NOTIMPBKGROUND:
		errStr = "�ؼ������ڱ������ʹ��";
		break;
	case VX_E_EFX_NOTIMPINHERE:
		errStr = "�����ؼ�������ӻ�ɾ��";
		break;
	case VX_E_EFX_CLIPTYPENOTMATCH:
		errStr = "�ؼ���Ƭ�����Ͳ�ƥ��";
		break;
	case VX_E_EFX_CANTUSEWITHKEYVIDEO:
		errStr = "�������ڴ�����Ƶ";
		break;
	case VX_E_EFX_SLOTFULL:
		errStr = "�ؼ������Ѿ��ﵽ���";
		break;
	case VX_E_EFX_CANNOTNAPPE:
		errStr = "����ؼ���֧�ֵ���";
		break;
	case VX_E_EFX_3DCANNOTNAPPE:
		errStr = "Ƭ�����Ѿ��в��ܺ�3D�ؼ�һ��ʹ�õ��ؼ�";
		break;
	case VX_E_EFX_CANTUSEWITH3D:
		errStr = "����ؼ����ܺ�3D�ؼ�һ��ʹ��";
		break;
	case VX_E_EFX_CANTREMOVEINHERE:
		errStr = "�����ؼ�����ɾ��";
		break;
	case VX_E_UD_STOPPING:
		errStr = "�ɼ���ֹͣ������";
		break;
	case VX_E_UD_NONEEDRES:
		errStr = "û����Ҫ����Դ";
		break;
	case VX_E_UD_NOVSINGNAL:
		errStr = "û����Ƶ�����ź�";
		break;
	case VX_E_ENC_VIDPREPROC_FAIL:
		errStr = "��ƵԤ�����ʼ��ʧ��";
		break;
	case VX_E_ENC_VIDWRITE_FAIL:
		errStr = "��Ƶѹ��д�ļ�ʧ��,������д�����ٶ�����";
		break;
	case VX_E_ENC_AUDWRITE_FAIL:
		errStr = "��Ƶѹ��д�ļ�ʧ��,������д�����ٶ�����";
		break;
	case VX_E_ENC_MUXER_BEGIN_FAIL:
		errStr = "����������ʧ��,������ģ�岻֧�ִ�Դ�ļ�";
		break;
	case VX_E_ENC_VIDEO_FAIL:
		errStr = "��Ƶѹ��д�ļ�ʧ��,������д�����ٶ�����";
		break;
	case VX_E_ENC_AUDIO_FAIL:
		errStr = "��Ƶѹ��д�ļ�ʧ��,������д�����ٶ�����";
		break;
	case VX_E_DEC_FILE_OPEN:
		errStr = "���ļ�ʧ��";
		break;
	case VX_E_DEC_FTP_CONNECT:
		errStr = "����ftp������ʧ��";
		break;
	case VX_E_DEC_GET_FILESIZE:
		errStr = "��ȡ�ļ���Сʧ��";
		break;
	case VX_E_DEC_CREATE_DIR:
		errStr = "�����ļ�Ŀ¼ʧ��";
		break;
	case VX_E_DEC_READVIDEOFRAME:
		errStr = "����Ƶ֡����ʧ��";
		break;
	case VX_E_DEC_READAUDIOFRAME:
		errStr = "����Ƶ֡����ʧ��";
		break;
	case VX_E_DEC_SET_FILEPOSITION:
		errStr = "�����ļ�λ��ʧ��";
		break;
	case VX_E_DEC_VIDEODEC:
		errStr = "��Ƶ����ʧ��";
		break;
	case VX_E_DEC_AUDIODEC:
		errStr = "��Ƶ����ʧ��";
		break;
	case VX_E_MUXER_NO_VA:
		errStr = "������û���յ��㹻������Ƶ����";
		break;
	default:
		errStr = "δ֪����";
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
