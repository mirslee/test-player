#include "stdafx.h"
#include "MxMemory.h"
#include "MxLog.h"
#include "ncx_mempool/ncx_slab.h"
#include <assert.h>



#ifndef _DEBUG
#define USE_VM
#endif

#define VM_PAGESIZE	4096
#define MEM_PADS	16
static unsigned char _bvxAlignLandFill = 0xED;   /* fill no-man's land for aligned routines */
static int __cdecl CheckBytes(unsigned char * pb, unsigned char bCheck, size_t nSize)
{
	int bOkay = 1;
	while (nSize--)
	{
		if (*pb++ != bCheck)
		{
			bOkay = 0;
		}
	}
	return bOkay;
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

#ifdef __APPLE__
#include <mach/mach.h>
#endif

#define MAX_STACK_DEPTH	25

#ifdef _WIN32

BOOL GetFunctionInfoFromAddresses(DWORD64 fnAddress, DWORD64 stackAddress, LPTSTR lpszSymbol, rsize_t lpszSymbolSize);
BOOL GetSourceInfoFromAddress(DWORD64 address, LPTSTR lpszSourceInfo, rsize_t lpszSourceInfoSize);

// Define number of call stack levels to ignore (usually 2, TraceAlloc and operator new)
#define NUM_LEVELS_TO_IGNORE 2
USHORT(WINAPI *backtrace)(ULONG, ULONG, PVOID*, PULONG) = 0;

void vxDumpStack(const char* dumpname)
{
	if (backtrace == 0) {
		const HMODULE hNtDll = ::GetModuleHandle(L"ntdll.dll");
		reinterpret_cast<void*&>(backtrace) = ::GetProcAddress(hNtDll, "RtlCaptureStackBackTrace");
	}
	void* stacktrace[MAX_STACK_DEPTH + 1];
	int capcount = backtrace(NUM_LEVELS_TO_IGNORE, MAX_STACK_DEPTH, stacktrace, NULL);
	for (int i = 0; i < capcount; i++)
	{
		wchar_t symInfo[512] = (L"?");
		wchar_t srcInfo[512] = (L"?");
		GetFunctionInfoFromAddresses((DWORD64)stacktrace[i], (DWORD64)stacktrace[i], symInfo, sizeof(symInfo));
		GetSourceInfoFromAddress((DWORD64)stacktrace[i], srcInfo, sizeof(srcInfo));
		mx_debug("  %s : %s\n", srcInfo, symInfo);
	}
}
#else
#include <execinfo.h>
#include <cxxabi.h>

/** Print a demangled stack backtrace of the caller function to FILE* out. */
void vxDumpStack(const char* dumpname)
{
	mx_debug("%s stack trace:\n", dumpname);

	void* addrlist[MAX_STACK_DEPTH + 1] = { 0 };
	int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

	if (addrlen == 0)
	{
		mx_debug("  <empty, possibly corrupt>\n");
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
			char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
			if (status == 0)
				mx_debug("  %s : %s+%s\n", symbollist[i], ret, begin_offset);
			else
				mx_debug("  %s : %s()+%s\n", symbollist[i], begin_name, begin_offset);
		}
		else
		{
			// couldn't parse the line? print the whole line.
			mx_debug("  %s\n", symbollist[i]);
		}
	}
	free(funcname);
	free(symbollist);
}

#endif

#define CHECKALLOC      1
#if CHECKALLOC
void CheckMemory(_MyAlignMemBlockHdr *pHdr, void * memblock)
{
	if (!CheckBytes(pHdr->Gap, _bvxAlignLandFill, nvxAlignGapSize))
	{
		// We don't know where (file, linenum) memblock was allocated
		mx_debug("Damage before 0x%p which was allocated by aligned routine(begin)\n", memblock);
#ifdef _DEBUG
		assert(false);
#else
		vxDumpStack("_vxfree");
#endif
	}

	if (!CheckBytes((BYTE*)memblock + pHdr->valids, _bvxAlignLandFill, nvxNoMansLandSize))
	{
		// We don't know where (file, linenum) memblock was allocated
		mx_debug("Damage before 0x%p which was allocated by aligned routine(end)\n", memblock);
#ifdef _DEBUG
		assert(false);
#else
		vxDumpStack("_vxfree");
#endif
	}
}
#else
#define CheckMemory
#endif

MXCORE_API void* mx_calloc(size_t _NumOfElements, size_t _SizeOfElements, int align)
{
	size_t _Size = _NumOfElements*_SizeOfElements;
	return mx_mallocz(_Size, align);
}

MXCORE_API void*  mx_malloc(size_t _Size, int align)
{
	uintptr_t ptr = 0, r_ptr = 0;
	_MyAlignMemBlockHdr *pHdr;
	if (!IS_2_POW_N(align)) align = 16;
	int alignptr = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) - 1;
	int extsize = alignptr + 1 + sizeof(_MyAlignMemBlockHdr) + MEM_PADS;
#ifdef USE_VM
	if (_Size >= 0x80000)
	{
		size_t reals = (_Size + extsize + VM_PAGESIZE - 1)&~(VM_PAGESIZE - 1);
#ifdef _WIN32
		ptr = (uintptr_t)VirtualAlloc(NULL, reals, MEM_COMMIT, PAGE_READWRITE);
		if (!ptr) return NULL;
#else
		kern_return_t err = vm_allocate((vm_map_t)mach_task_self(), (vm_address_t *)&ptr, reals, VM_FLAGS_ANYWHERE);
		if (err != KERN_SUCCESS) return NULL;
#endif//_WIN32
		r_ptr = (ptr + alignptr + sizeof(_MyAlignMemBlockHdr))&~alignptr;
		pHdr = (_MyAlignMemBlockHdr *)(r_ptr)-1;
		pHdr->size = reals;
		pHdr->type = 1;
	}
	else
	{
#endif//USE_VM
		size_t reals = _Size + extsize;
		if ((ptr = (uintptr_t)malloc(reals)) == (uintptr_t)NULL)
			return NULL;
		r_ptr = (ptr + alignptr + sizeof(_MyAlignMemBlockHdr))&~alignptr;
		pHdr = (_MyAlignMemBlockHdr *)(r_ptr)-1;
		pHdr->size = reals;
		pHdr->type = 0;
#ifdef USE_VM	
	}
#endif//USE_VM

#if CHECKALLOC
	memset((void*)pHdr->Gap, _bvxAlignLandFill, nvxAlignGapSize);
	memset((void*)(r_ptr + _Size), _bvxAlignLandFill, nvxNoMansLandSize);
#endif
	pHdr->pHead = (void *)ptr;
	pHdr->valids = _Size;
	return (void *)r_ptr;
}

MXCORE_API void* mx_mallocz(size_t _Size, int align)
{
	void* p = mx_malloc(_Size, align);
	memset(p, 0, _Size);
	return p;
}

MXCORE_API void* mx_realloc(void * memblock, size_t size, int align)
{
	if (memblock == NULL)
		return mx_mallocz(size, align);

	_MyAlignMemBlockHdr *s_pHdr = (_MyAlignMemBlockHdr*)memblock - 1;
	CheckMemory(s_pHdr, memblock);
	if (s_pHdr->valids >= size) return memblock;

	void* newmemblock = mx_malloc(size, align);
	if (newmemblock)
	{
		memcpy(newmemblock, memblock, s_pHdr->valids);
		memset(((BYTE*)newmemblock) + s_pHdr->valids, 0, size - s_pHdr->valids);
	}
	mx_free(memblock);
	return newmemblock;
}

MXCORE_API void  mx_free(void * memblock)
{
	if (memblock == NULL) return;
	_MyAlignMemBlockHdr *pHdr = (_MyAlignMemBlockHdr*)memblock - 1;
	CheckMemory(pHdr, memblock);
#ifdef USE_VM
	if (pHdr->type == 1)
	{
#ifdef _WIN32
		VirtualFree(pHdr->pHead, 0, MEM_RELEASE);
#else
		vm_deallocate((vm_map_t)mach_task_self(), (vm_address_t)pHdr->pHead, pHdr->size);
#endif
	}
	else
#endif	
		free((void *)pHdr->pHead);
}

MXCORE_API void _vxfreep(void ** memblock)
{
	if (memblock && *memblock)
	{
		mx_free(*memblock);
		*memblock = NULL;
	}
}

//#include <sys/time.h>
//uint64_t usTime()
//{
//    struct timeval tv;
//    uint64_t usec;
//    
//    gettimeofday(&tv, NULL);
//    
//    usec = ((uint64_t)tv.tv_sec)*1000000LL;
//    usec += tv.tv_usec;
//    
//    return usec;
//}

class CMxSlabMemoryPool
{
public:
	CMxSlabMemoryPool()
	{
		int64 poolSize = 0x8000000;
		u_char* space = (u_char *)mx_malloc(poolSize);
		pool = (ncx_slab_pool_t*)space;
		pool->addr = space;
		pool->min_shift = 3;
		pool->end = space + poolSize;
		ncx_slab_init(pool);
        
        /*uint64_t us_begin;
        uint64_t us_end;
        uint64_t t1, t2;
        
        size_t size[] = { 30, 120, 256, 500, 1000, 2000, 3000, 5000};
        
        for (int j = 0; j < sizeof(size)/sizeof(size_t); j++) {
            size_t s = size[j];
            us_begin  = usTime();
            for(int i = 0; i < 1000000; i++)  {
                void* p = mx_pool_alloc(s);
                mx_pool_free(p);
                //void* p = ncx_slab_alloc(pool, s);
                //ncx_slab_free(pool, p);
            }
            us_end  = usTime();
            t1 = (us_end - us_begin);
            
            us_begin  = usTime();
            for(int i = 0; i < 1000000; i++)  {
                void*p = (char*)malloc(s);
                
                free(p);
            }
            us_end  = usTime();
            t2 = (us_end - us_begin);
            continue;
        }*/
	}
	~CMxSlabMemoryPool()
	{
		ncx_slab_stat_t stat = { 0 };
		ncx_slab_stat(pool, &stat);
		if (pool)
		{
			mxMutexDestroy(&pool->mutex);
			mx_free(pool);
		}
	}
	ncx_slab_pool_t* pool = nullptr;
};

CMxSlabMemoryPool g_memoryPool;

MXCORE_API void* mx_pool_alloc(int size) {
	void *p = ncx_slab_alloc(g_memoryPool.pool,size);
	if (p == 0)
	{
		ncx_slab_stat_t stat;
		ncx_slab_stat(g_memoryPool.pool, &stat);

		/*vxString strMsg;
		char szmsg[256] = { 0 };
		sprintf(szmsg, "pool_size : %u bytes\n", stat.pool_size);
		strMsg += szmsg;
		sprintf(szmsg, "used_size : %u bytes\n", stat.used_size);
		strMsg += szmsg;
		sprintf(szmsg, "used_pct  : %u%%\n", stat.used_pct);
		strMsg += szmsg;

		sprintf(szmsg, "total page count : %u\n", stat.pages);
		strMsg += szmsg;
		sprintf(szmsg, "free page count  : %u\n\n", stat.free_page);
		strMsg += szmsg;

		sprintf(szmsg, "small slab use page : %u,\tbytes : %u\n", stat.p_small, stat.b_small);
		strMsg += szmsg;
		sprintf(szmsg, "exact slab use page : %u,\tbytes : %u\n", stat.p_exact, stat.b_exact);
		strMsg += szmsg;
		sprintf(szmsg, "big   slab use page : %u,\tbytes : %u\n", stat.p_big, stat.b_big);
		strMsg += szmsg;
		sprintf(szmsg, "page slab use page  : %u,\tbytes : %u\n\n", stat.p_page, stat.b_page);
		strMsg += szmsg;

		sprintf(szmsg, "max free pages : %u\n", stat.max_free_pages);
		strMsg += szmsg;
		VX_MailMSG(vxLoadMessageLV(strMsg), vxLoadMessageLV("Memory Unavailable"), 0, MAILSRC_HARDHEL);*/
	}
	return p;
}

MXCORE_API void mx_pool_free(void * ptr) {
    ncx_slab_free(g_memoryPool.pool, ptr);
}

MXCORE_API void*  mxAlloc(int size) {
    void *p = ncx_slab_alloc(g_memoryPool.pool, size);
    if (p == 0)
    {
        ncx_slab_stat_t stat;
        ncx_slab_stat(g_memoryPool.pool, &stat);
    }
    return p;
}

MXCORE_API void  mxFree(void * ptr) {
    ncx_slab_free(g_memoryPool.pool, ptr);
}

#ifdef _WIN32
#define BUFFERSIZE 8192

// Define how many levels of callstack that should be fetched for each allocation.
// Each level costs 2*sizof(DWORD64) bytes / allocation.
#define MAXSTACK 25

// Define size of no mans land
#define NO_MANS_LAND_SIZE 16

// Define frequency of no mans land checking
#define NML_CHECK_EVERY 1000

// Define number of call stack levels to ignore (usually 2, TraceAlloc and operator new)
#define NUM_LEVELS_TO_IGNORE 2

BOOL GetFunctionInfoFromAddresses(DWORD64 fnAddress, DWORD64 stackAddress, LPTSTR lpszSymbol, rsize_t lpszSymbolSize)
{
	/*BOOL              ret = FALSE;
#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
	DWORD64             dwDisp = 0;
#else
	DWORD             dwDisp = 0;
#endif

	DWORD             dwSymSize = 10000;
	TCHAR             lpszUnDSymbol[BUFFERSIZE] = { _T('?') };
	CHAR              lpszNonUnicodeUnDSymbol[BUFFERSIZE] = { '?' };
	LPTSTR            lpszParamSep = NULL;
	LPCTSTR           lpszParsed = lpszUnDSymbol;
	PIMAGEHLP_SYMBOL  pSym = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, dwSymSize);

	::ZeroMemory(pSym, dwSymSize);
	pSym->SizeOfStruct = dwSymSize;
	pSym->MaxNameLength = dwSymSize - sizeof(IMAGEHLP_SYMBOL);

	// Set the default to unknown
	_tcscpy_s(lpszSymbol, lpszSymbolSize, _T("?"));

	BOOL getsym = SymGetSymFromAddr(GetCurrentProcess(), (vxuintptr)fnAddress, &dwDisp, pSym);
	if (!getsym)
	{
		TCHAR           lpModuleInfo[BUFFERSIZE] = _T("");
		DWORD64 baseaddr = 0;
		if (GetModuleBassAddr(fnAddress, lpModuleInfo, &baseaddr))
		{
			SymLoadModule(GetCurrentProcess(), NULL, lpModuleInfo, NULL, (vxuintptr)baseaddr, 0);
			getsym = SymGetSymFromAddr(GetCurrentProcess(), (vxuintptr)fnAddress, &dwDisp, pSym);
		}
	}
	// Get symbol info for IP
	if (getsym)
	{
		// Make the symbol readable for humans
		UnDecorateSymbolName(pSym->Name, lpszNonUnicodeUnDSymbol, BUFFERSIZE,
			UNDNAME_COMPLETE | UNDNAME_NO_THISTYPE | UNDNAME_NO_SPECIAL_SYMS | UNDNAME_NO_MEMBER_TYPE | UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS);

		// Symbol information is ANSI string
		PCSTR2LPTSTR(lpszNonUnicodeUnDSymbol, lpszUnDSymbol, sizeof(lpszUnDSymbol));

		// I am just smarter than the symbol file :)
		if (_tcscmp(lpszUnDSymbol, _T("_WinMain@16")) == 0)
			_tcscpy_s(lpszUnDSymbol, sizeof(lpszUnDSymbol), _T("WinMain(HINSTANCE,HINSTANCE,LPCTSTR,int)"));
		else
			if (_tcscmp(lpszUnDSymbol, _T("_main")) == 0)
				_tcscpy_s(lpszUnDSymbol, sizeof(lpszUnDSymbol), _T("main(int,TCHAR * *)"));
			else
				if (_tcscmp(lpszUnDSymbol, _T("_mainCRTStartup")) == 0)
					_tcscpy_s(lpszUnDSymbol, sizeof(lpszUnDSymbol), _T("mainCRTStartup()"));
				else
					if (_tcscmp(lpszUnDSymbol, _T("_wmain")) == 0)
						_tcscpy_s(lpszUnDSymbol, sizeof(lpszUnDSymbol), _T("wmain(int,TCHAR * *,TCHAR * *)"));
					else
						if (_tcscmp(lpszUnDSymbol, _T("_wmainCRTStartup")) == 0)
							_tcscpy_s(lpszUnDSymbol, sizeof(lpszUnDSymbol), _T("wmainCRTStartup()"));

		lpszSymbol[0] = _T('\0');

		// Skip all templates
		if (_tcschr(lpszParsed, _T('<')) == NULL)
		{
			// Let's go through the stack, and modify the function prototype, and insert the actual
			// parameter values from the stack
			if (_tcsstr(lpszUnDSymbol, _T("(void)")) == NULL && _tcsstr(lpszUnDSymbol, _T("()")) == NULL)
			{
				ULONG index = 0;
				for (; ; index++)
				{
					lpszParamSep = const_cast<LPTSTR>(_tcschr(lpszParsed, _T(',')));
					if (lpszParamSep == NULL)
						break;

					*lpszParamSep = _T('\0');

					_tcscat_s(lpszSymbol, lpszSymbolSize, lpszParsed);
					_stprintf_s(lpszSymbol + _tcslen(lpszSymbol), lpszSymbolSize - _tcslen(lpszSymbol), _T("=0x%08X,"), *((vxuintptr*)(stackAddress)+2 + index));

					lpszParsed = lpszParamSep + 1;
				}

				lpszParamSep = const_cast<LPTSTR>(_tcschr(lpszParsed, _T(')')));
				if (lpszParamSep != NULL)
				{
					*lpszParamSep = _T('\0');

					_tcscat_s(lpszSymbol, lpszSymbolSize, lpszParsed);
					vxuintptr* ptr = (vxuintptr*)(stackAddress)+2 + index;
					__try
					{
						if (!IsBadReadPtr(ptr, sizeof(vxuintptr)))
							_stprintf_s(lpszSymbol + _tcslen(lpszSymbol), lpszSymbolSize - _tcslen(lpszSymbol), _T("=0x%08X)"), *ptr);
						else
							_stprintf_s(lpszSymbol + _tcslen(lpszSymbol), lpszSymbolSize - _tcslen(lpszSymbol), _T("=0x%08X)"), 0xafafaf);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						_stprintf_s(lpszSymbol + _tcslen(lpszSymbol), lpszSymbolSize - _tcslen(lpszSymbol), _T("=0x%08X)"), 0xafafaf);
					}
					lpszParsed = lpszParamSep + 1;
				}
			}
		}

		_tcscat_s(lpszSymbol, lpszSymbolSize, lpszParsed);

		ret = TRUE;
	}

	GlobalFree(pSym);

	return ret;*/
	return true;
}

// Get source file name and line number from IP address
// The output format is: "sourcefile(linenumber)" or
//                       "modulename!address" or
//                       "address"

BOOL GetSourceInfoFromAddress(DWORD64 address, LPTSTR lpszSourceInfo, rsize_t lpszSourceInfoSize)
{
	/*BOOL            ret = FALSE;
	IMAGEHLP_LINE64 lineInfo;
	DWORD           dwDisp;
	TCHAR           lpszFileName[BUFFERSIZE] = _T("");
	TCHAR           lpModuleInfo[BUFFERSIZE] = _T("");

	_tcscpy_s(lpszSourceInfo, lpszSourceInfoSize, _T("?(?)"));

	::ZeroMemory(&lineInfo, sizeof(lineInfo));
	lineInfo.SizeOfStruct = sizeof(lineInfo);

	if (SymGetLineFromAddr64(GetCurrentProcess(), address, &dwDisp, &lineInfo))
	{
		// Got it. Let's use "sourcefile(linenumber)" format
		PCSTR2LPTSTR(lineInfo.FileName, lpszFileName, sizeof(lpszFileName));
		_stprintf_s(lpszSourceInfo, lpszSourceInfoSize, _T("%s(%d)"), lpszFileName, lineInfo.LineNumber);
		ret = TRUE;
	}
	else
	{
		// There is no source file information. :(
		// Let's use the "modulename!address" format
		GetModuleNameFromAddress(address, lpModuleInfo, sizeof(lpModuleInfo));

		if (lpModuleInfo[0] == _T('?') || lpModuleInfo[0] == _T('\0'))
			// There is no modulename information. :((
			// Let's use the "address" format
			_stprintf_s(lpszSourceInfo, lpszSourceInfoSize, _T("0x%08X"), lpModuleInfo, address);
		else
			_stprintf_s(lpszSourceInfo, lpszSourceInfoSize, _T("%s!0x%08X"), lpModuleInfo, address);

		ret = FALSE;
	}

	return ret;*/
	return true;
}

#endif
