#ifndef __BAY_VXOBJECT_H__
#define __BAY_VXOBJECT_H__

#include "vxtypes.h"
#ifndef vxmax
#define vxmax(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef vxmin
#define vxmin(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
#define vxCreateBitmap CreateBitmap
#define vxCreateBitmapIndirect CreateBitmapIndirect
#define vxSleep(x) Sleep(x)


#else

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <Carbon/Carbon.h>
#else
#include <values.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#define __stdcall
#define __cdecl
#define __declspec(x)
#define __fastcall





#define NOERROR 0

//#define vxSleep(x) usleep(x*1000)

#define INVALID_HANDLE_VALUE				(HANDLE)-1

extern inline LONG InterlockedExchangeAdd( LONG volatile *dest, LONG incr );
extern inline LONG InterlockedExchange( LONG volatile *dest, LONG val );
extern inline __int64 InterlockedExchange64(__int64 volatile *dest, __int64 val);
#if (__GNUC__>=4)&&(__GNUC_MINOR__<1)
extern inline LONG InterlockedExchangeAdd( LONG volatile *dest, LONG incr )
{
    LONG ret;
#ifdef __i386__	
    __asm__ __volatile__( "lock; xaddl %0,(%1)"
#else
	__asm__ __volatile__( "lock; xadd %0,(%1)"
#endif
						 : "=r" (ret) : "r" (dest), "0" (incr) : "memory" );
    return ret;
}
						 
extern inline LONG InterlockedExchange( LONG volatile *dest, LONG val )
{
	LONG ret;
#ifdef __i386__	
	__asm__ __volatile__( "lock; xchgl %0,(%1)"
#else
	 __asm__ __volatile__( "lock; xchg %0,(%1)"
#endif
						  : "=r" (ret) :"r" (dest), "0" (val) : "memory" );
	 return ret;
}
							 
extern inline __int64 InterlockedExchange64(__int64 volatile *dest, __int64 val)
{
	__int64       ret;
	__asm__ __volatile__("lock xchgq %0, (%1)"
						 : "=r" (ret) :"r" (dest), "0" (val) : "memory" );
	return ret;
}

extern inline __int64 InterlockedExchangeAdd64( __int64 volatile *dest, LONG incr )
{
	__int64 ret;
	__asm__ __volatile__( "lock; xaddq %0,(%1)"
						 : "=r" (ret) : "r" (dest), "0" (incr) : "memory" );
	return ret;
}						 
						 
						 
#else
extern inline LONG InterlockedExchangeAdd( LONG volatile *dest, LONG incr )
{
	return __sync_fetch_and_add(dest, incr);	
}
extern inline LONG InterlockedExchange( LONG volatile *dest, LONG val )
{
	return __sync_lock_test_and_set(dest, val);	
}
extern inline __int64 InterlockedExchange64( __int64 volatile *dest, __int64 val )
{
	return __sync_lock_test_and_set(dest, val);	
}
extern inline __int64 InterlockedExchangeAdd64( __int64 volatile *dest, __int64 incr )
{
	return __sync_fetch_and_add(dest, incr);	
}
#endif

extern inline LONG InterlockedIncrement( LONG volatile *dest );
extern inline LONG InterlockedIncrement( LONG volatile *dest )
{
    return InterlockedExchangeAdd( dest, 1 ) + 1;
}

extern inline LONG InterlockedDecrement( LONG volatile *dest );
extern inline LONG InterlockedDecrement( LONG volatile *dest )
{
    return InterlockedExchangeAdd( dest, -1 ) - 1;
}

extern inline __int64 InterlockedIncrement64( __int64 volatile *dest );
extern inline __int64 InterlockedIncrement64( __int64 volatile *dest )
{
	return InterlockedExchangeAdd64( dest, 1 ) + 1;
}

extern inline __int64 InterlockedDecrement64( __int64 volatile *dest );
extern inline __int64 InterlockedDecrement64( __int64 volatile *dest )
{
	return InterlockedExchangeAdd64( dest, -1 ) - 1;
}

extern inline void* InterlockedExchangePointer(void** pp,void* pNew);
extern inline void* InterlockedExchangePointer(void** pp,void* pNew)
{
#if __LP64__
	return( reinterpret_cast<void*>(static_cast<vxuintptr>(
		::InterlockedExchange64(reinterpret_cast<__int64*>(pp), 
		static_cast<__int64>(reinterpret_cast<vxuintptr>(pNew))))) );
#else	
	return( reinterpret_cast<void*>(static_cast<vxuintptr>(
		::InterlockedExchange(reinterpret_cast<LONG*>(pp), 
		static_cast<LONG>(reinterpret_cast<vxuintptr>(pNew))))) );
#endif	
}


#define LoadLibrary vxLoadLibrary
#define LoadLibraryEx(v1,v2,v3) vxLoadLibrary(v1)
#define FreeLibrary vxFreeLibrary
#define GetProcAddress vxGetProcAddress

#define stricmp strcasecmp
#endif

#ifndef vxinterface
#define vxinterface               struct
#endif

typedef unsigned char __uint8;
typedef unsigned int __uint32;

#define VXNULL								0

#define VXNOERROR							0x0
#define VXE_NOINTERFACE                     0x80000004L

#define LIID_IVxObject		0
vxinterface IVxObject
{
	virtual	LONG __stdcall QueryInterface(LONG iid, void**) = 0;	//获取其它接口
	virtual LONG __stdcall AddRef() = 0;			//引用计数管理
	virtual LONG __stdcall Release() = 0;
};
typedef IVxObject *LPVXOBJECT;

#ifndef INONDELEGATINGVXOBJECT_DEFINED
vxinterface INonDelegatingVxObject
{
    virtual LONG __stdcall NonDelegatingQueryInterface(LONG iid, void**) = 0;
    virtual LONG __stdcall NonDelegatingAddRef() = 0;
    virtual LONG __stdcall NonDelegatingRelease() = 0;
};
#define INONDELEGATINGVXOBJECT_DEFINED
#endif

typedef INonDelegatingVxObject *PNDVXOBJECT;

vxinline LONG GetVxInterface(LPVXOBJECT pUnk, void **ppv)
{
	*ppv = pUnk;
	pUnk->AddRef();
	return VXNOERROR;
}

class CVxSimpleObject : public INonDelegatingVxObject
{
private:
	const LPVXOBJECT m_pVxObject; /* Owner of this object */
public:
#ifdef _WIN32
#pragma warning( push )
#pragma warning(disable:4355)
#endif
	CVxSimpleObject():m_cRef(0),m_pVxObject(reinterpret_cast<LPVXOBJECT>(static_cast<PNDVXOBJECT>(this))) {}
#ifdef _WIN32
#pragma warning( pop )
#endif
	virtual ~CVxSimpleObject(){}
	LPVXOBJECT GetOwner() const {
		return m_pVxObject;
	};

	LONG __stdcall NonDelegatingQueryInterface(LONG iid, void** ppobj)
	{
		if (iid ==LIID_IVxObject) 
			return GetVxInterface((LPVXOBJECT)(PNDVXOBJECT) this, ppobj);
		else
			return VXE_NOINTERFACE;
	}
	LONG __stdcall NonDelegatingAddRef()
	{
		InterlockedIncrement( &m_cRef );
		return vxmax(m_cRef,1);
	}

	LONG __stdcall NonDelegatingRelease()
	{
		LONG lRef = InterlockedDecrement( &m_cRef );
		if (lRef == 0) 
		{
			m_cRef++;
			OnDelete();
			delete this;
			return LONG(0);
		} 
		else 
		{
			return vxmax(LONG(m_cRef), 1);
		}
	}
protected:
	volatile LONG m_cRef;       /* Number of reference counts */
	virtual void OnDelete(){}
};

struct VXOBJECT
{
	char	szName[64];		//对象名称
	DWORD	dwVersion;		//对象版本, 用于版本管理
	DWORD	dwObjID;		//对象ID
	DWORD	dwObjType;		//对象类型 VXOBJTYPE
	DWORD	dwFlags;		//暂时只在dev中表示支持高清或标清
	DWORD	reserve[3];		//
	int		type_count;
	DWORD*	types;			//必须使用_vxmalloc来分配！！
	LONG (__cdecl *pfCreate)(IVxObject* setup,IVxObject* param1,vxuintptr param2,vxuintptr param3,IVxObject** obj);//创建对象函数
};

//所有的插件都有一个相同的入口vxGetObjects
//extern "C" __declspec( dllexport ) LONG vxGetObjects(VXOBJECT* vxObj, LONG idx)
typedef LONG (*vxGETOBJECT)(VXOBJECT*, LONG idx);//idx依次递增，直到返回0（返回0也是有效的对象）

//所有的setup都是IVxSystemSetup,没有输入参数名的参数无效,IVxObject** obj输出参数,返回值0表示成功

//LONG __cdecl CreateSource(IVxObject* setup,IVxObject*,DWORD param2(LPVX_AVPATH),DWORD, IVxObject** obj)
//LONG __cdecl CreateSink(IVxObject* setup,IVxObject*,DWORD param2(LPVX_AVPATH),DWORD,IVxObject** obj)

//LONG __cdecl CreateDemultiplexer(IVxObject* setup,IVxObject* source(IVxSource*),DWORD type(vxUnparkSub_*),DWORD, IVxObject** obj)
//LONG __cdecl CreateMultiplexer(IVxObject* setup,IVxObject*,DWORD type(vxUnparkSub_*),DWORD, IVxObject** obj)

//LONG __cdecl CreateVideoDecoder(IVxObject* setup,IVxObject* stream(IVxReadStream*),DWORD param2(LPVX_VIDEOINFO),DWORD decflag, IVxObject** obj)
//LONG __cdecl CreateAudioDecoder(IVxObject* setup,IVxObject* stream(IVxReadStream*),DWORD param2(LPVX_AUDIOINFO),DWORD decflag, IVxObject** obj)

//LONG __cdecl CreateVideoEncoder(IVxObject* setup,IVxObject*,DWORD param2(videnc_init*),DWORD, IVxObject** obj)
//LONG __cdecl CreateAudioEncoder(IVxObject* setup,IVxObject*,DWORD param2(audenc_init*),DWORD, IVxObject** obj)

//LONG __cdecl CreateVideoLive(IVxObject* setup,IVxObject*,DWORD param2(VXSURFACE_RES),DWORD, IVxObject** obj)

//LONG __cdecl CreateSystemClock(IVxObject* setup,IVxObject*,DWORD param2(sysclk_cinfo*),DWORD, IVxObject** obj)

//LONG __cdecl CreateVidInput(IVxObject* setup,IVxObject*,DWORD param2(vidin_cinfo*),DWORD, IVxObject** obj)
//LONG __cdecl CreateVidOutput(IVxObject* setup,IVxObject*,DWORD param2(vidout_cinfo*),DWORD, IVxObject** obj)

//LONG __cdecl CreateAudInput(IVxObject* setup,IVxObject*,DWORD param2(VXAUD_FREQ),DWORD, IVxObject** obj)
//LONG __cdecl CreateAudOutput(IVxObject* setup,IVxObject*,DWORD param2(VXAUD_FREQ),DWORD, IVxObject** obj)

#endif
