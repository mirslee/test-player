#include "stdafx.h"
#include "VxNLEEngine.h"
#include "VxEfxCommon.h"
#include "vximage.h"
#include "vxCommonTools.h"
#include "inlinetools.h"
#include "sqlite/sqlite3.h"
#include "vxnlesyslocked.h"
#include "vxfilename.h"
#include "vxnleresource.h"
#include "VxCorePlayerIF.h"
#include "vxultracg/vxultracgeditor.h"
#include "vxcgfileapi.h"
#include "vxcltoolsapi.h"
#ifdef _WIN32
#include "vxgl/wglew.h"
#endif
#include "VxEngineConfigIF.h"

using namespace vxgl;

CVxNLEEngine* g_nleengine = NULL;
int g_rate = 25,g_scale = 1;
int g_width = 1920, g_height = 1080;
DWORD g_aspect = 0x90010;
VXSURFACE_RES g_res = RES_1080;
VXSURFACE_FMT g_fmt = FMT_YUY2;
VXSURFACE_COLORIMETRY g_colorimetry = COLORIMETRY_BT709;
VXAUD_FREQ g_freq = FREQ_48000;

extern sqlite3* g_pDatabase;
HMODULE g_hcgeditor = NULL;

void __cdecl vxRegisterGLRC(void* gl);
void __cdecl vxRegisterDefaultShader(glShader** glshaders);

#ifdef _DEBUG
#define CHECK_SURFACE
#endif

CVxGLSurface::CVxGLSurface(CVxGLMemPool* pool,CVxQueue<IVxSurface*>* qs,VXSURFACE_DESC* desc,VXSURFACE_LOCK* lock)
: CVxObject("CVxGLSurface",NULL)
, m_pool(pool)
, m_qsurface(qs)
, m_privatedata(0)
, m_type(0)
{
	memcpy(&m_desc,desc,sizeof(VXSURFACE_DESC));
	memcpy(&m_lock,lock,sizeof(VXSURFACE_LOCK));
}

LONG CVxGLSurface::NonDelegatingRelease()
{
#ifdef _DEBUG
	if(m_desc.loc == LOC_HOST)
	{
		_vxmemcheck(m_lock.pLock);
	}
#endif

	LONG ref = CVxObject::NonDelegatingRelease();
#ifndef CHECK_SURFACE
	if(ref==1)
	{
		__releaseprivate();
		m_desc.colorimetry = (m_desc.res&VXCUSTOMRESMASK)?COLORIMETRY_UNKNOWN:g_resinfo2[m_desc.res].colorimetry;
        IVxSurface* sf = static_cast<IVxSurface*>(this);
		m_qsurface->Push(sf);
	}
#endif
	return ref;
}



void CVxGLSurface::__releaseprivate()
{
	if (m_privatedata)
	{
		if (m_type == 'UNKN')
			((IVxObject*)m_privatedata)->Release();
		else if (m_type == 'VPTR')
			_vxfree((void*)m_privatedata);
	}
	m_type = 0;
	m_privatedata = 0;
}

void  CVxGLSurface::SetPrivateData(DWORD type, vxuintptr privatedata)
{
	__releaseprivate();
	m_type = type;
	m_privatedata = privatedata;
	if ((m_type == 'UNKN') && m_privatedata)
		((IVxObject*)m_privatedata)->AddRef();
}

vxuintptr CVxGLSurface::GetPrivateData(DWORD type)
{
	if (type == m_type)
		return m_privatedata;
	else
		return 0;
}


void CVxGLSurface::OnDelete()
{
#ifdef CHECK_SURFACE
	__releaseprivate();
#endif
	if(m_desc.loc == LOC_GPUIN)
	{
		glDeleteBuffers(1, (GLuint*)&m_lock.pLock);
		VX_CHECK_GLERROR();
	}
	else if((m_desc.loc==LOC_GPU)||(m_desc.loc==LOC_GPUOUT))
	{
		glDeleteTextures(1,(GLuint*)&m_lock.pLock);
		VX_CHECK_GLERROR();
	}
	else
	{
		_vxfree(m_lock.pLock);
	}
}


#define MAX_SURFACES 512

#ifdef _WIN32
CVxGLMemPool::CVxGLMemPool(HDC hdc,HGLRC grc,cl_context cl)
: CVxObject("CVxGLMemPool",NULL)
, m_hglrc(grc)
, m_hdc(hdc)
#elif __APPLE__
CVxGLMemPool::CVxGLMemPool(CGLContextObj cgl,cl_context cl)
: CVxObject("CVxGLMemPool",NULL)
, m_cgl(cgl)
#else
CVxGLMemPool::CVxGLMemPool(Display* dp,GLXDrawable draw,GLXContext glx,cl_context cl)
: CVxObject("CVxGLMemPool",NULL)
, m_xdp(dp)
, m_xdraw(draw)
, m_glx(glx)
#endif
, m_cl(cl)
{
	for(int i=RES_480;i<RES_LAST;i++)
	{
		for(int j=LOC_HOST;j<LOC_LAST;j++)
		{
			for(int k=FMT_BGRA;k<FMT_LAST;k++)
			{
				if(j==LOC_HOST)
					m_maxsfss[i][j][k] = MAX_SURFACES;
				else
					m_maxsfss[i][j][k] = MAX_SURFACES/4;
				m_qss[i][j][k].SetMaxSize(m_maxsfss[i][j][k]);
				m_qss[i][j][k].SetWaitTime(5000);
			}
		}
	}
	memset(m_sfss,0,sizeof(int)*RES_LAST*LOC_LAST*FMT_LAST);
	VXTHREAD_MUTEX_INIT(&m_csLock);
}

CVxGLMemPool::~CVxGLMemPool(void)
{
	VXTHREAD_MUTEX_DESTROY(&m_csLock);
}


LONG CVxGLMemPool::NonDelegatingAddRef()
{
	LONG ref = CVxObject::NonDelegatingAddRef();
	return ref;
}

LONG CVxGLMemPool::NonDelegatingRelease()
{
	LONG ref = CVxObject::NonDelegatingRelease();
	return ref;
}

const char* oclErrorString(cl_int error);

vxinline int GetAlignedSize( int i_size )
{
	int i_result = 1;
	while( i_result < i_size )
		i_result *= 2;
	return i_result;
}

//{ RES_480, RES_576, RES_720, RES_HDV_1080, RES_1080,RES_3D_1080, RES_360, RES_UHD2160,RES_2K1152, RES_2K1536, RES_2KDCIFLAT, RES_2KDCIFULL, RES_2KDCISCOPE, RES_2KFULL,   RES_4KDCIFLAT, RES_4KDCIFULL, RES_4KDCISCOPE, RES_4KFULL, RES_LAST}

//{FMT_BGRA, FMT_YUY2, FMT_A8, FMT_L8, FMT_A8L8, FMT_YUY2A, FMT_YUV422P8, FMT_YUV422P16, FMT_YUV444P8, FMT_YUV444P16, FMT_YUVA422P8, FMT_YUVA444P16, FMT_A16, FMT_LAST }
//{LOC_HOST,LOC_IO,LOC_GPU,LOC_GPUIN,LOC_GPUOUT,LOC_LAST}
//{ COLORIMETRY_BT601, COLORIMETRY_BT709, COLORIMETRY_DCIP3, COLORIMETRY_BT2020, COLORIMETRY_LAST } /
///{ SCANTYPE_BOTTOMFIRST, SCANTYPE_TOPFIRST, SCANTYPE_PROGRESSIVE, SCANTYPE_LAST}

static const char* g_fmtstr[] = {	"    FMT_BGRA",
									"      FMT_YUY2",
									"        FMT_A8",
									"        FMT_L8",
									"      FMT_A8L8",
									"     FMT_YUY2A",
                                    "  FMT_YUV422P8",
                                    " FMT_YUV422P16",
                                    "  FMT_YUV444P8",
                                    " FMT_YUV444P16",
                                    " FMT_YUVA422P8",
                                    "FMT_YUVA444P16",
                                    "       FMT_A16"
									"       FMT_L16"
									"       FMT_BGRX"
};
static const char* g_locstr[] = {	"  LOC_HOST",
									"    LOC_IO",
									"   LOC_GPU",
									" LOC_GPUIN",
									"LOC_GPUOUT"};

static const char* g_resstr[] = {	"       RES_480",
									"       RES_576",
									"       RES_720",
									"  RES_HDV_1080",
									"      RES_1080",
									"   RES_3D_1080",
									"	    RES_360",
									"   RES_UHD2160",
									"    RES_2K1152",
                                    "    RES_2K1536",
                                    " RES_2KDCIFLAT",
                                    " RES_2KDCIFULL",
                                    "RES_2KDCISCOPE",
                                    "    RES_2KFULL",
                                    " RES_4KDCIFLAT",
                                    " RES_4KDCIFULL",
                                    "RES_4KDCISCOPE",
                                    "    RES_4KFULL",
									"    RES_CUSTOM",
};

#if __APPLE__
#include <mach/mach_host.h>
#include <mach/mach_vm.h>
__int64 get_freev(int minsize)
{
	return minsize;
	kern_return_t kr;
	mach_vm_size_t empty = 0;
	vm_map_t task = mach_task_self();
	mach_vm_address_t addr;
	mach_vm_size_t size;
	for(addr = 0; ;addr += size) 
	{	
		vm_region_basic_info_data_64_t info;
		mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
		mach_port_t object_name;
		
		kr = mach_vm_region(task, &addr, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &count, &object_name);
		if (kr != KERN_SUCCESS) break;
		if((info.reserved)&&(size>=minsize))
			empty += size;
	}
	return empty;
}

#endif


IVxSurface* CVxGLMemPool::CreateSurface(VXSURFACE_FMT fmt,VXSURFACE_LOC loc,VXSURFACE_RES res)
{
	CVxLock cslock(&m_csLock);
	__int64 availphys = 0;
#ifdef _WIN32	
	MEMORYSTATUSEX status = {sizeof(MEMORYSTATUSEX)};
	GlobalMemoryStatusEx(&status);
	availphys = vxmin(status.ullAvailPhys,status.ullAvailVirtual);
#elif __APPLE__
	mach_port_t host_port = mach_host_self();
	vm_size_t pagesize;
	host_page_size(host_port, &pagesize);
	vm_statistics_data_t vm_stat;
	mach_msg_type_number_t count = sizeof(vm_stat) / sizeof(natural_t);
	kern_return_t kr = host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &count);
	availphys = vm_stat.free_count*pagesize;//get_freev(g_resinfo[res].w*g_resinfo[res].h*g_fmtbytes[fmt]);//500*1024*1024;
#else
#endif	
	if(availphys<300*1024*1024)
	{
		for(int i=RES_480;i<RES_LAST;i++)
			for(int k=FMT_BGRA;k<FMT_LAST;k++)
			{
				if(m_sfss[i][LOC_HOST][k])
				{
					IVxSurface* surface;
					while(m_qss[i][LOC_HOST][k].Pop(&surface))
					{
						surface->Release();
						InterlockedDecrement(&m_sfss[i][LOC_HOST][k]);
					}
				}
			}
	}
#if defined(_DEBUG)	
	__int64 totalhost = 0,totalgpu = 0;
	for(int i=RES_480;i<RES_LAST;i++)
		for(int j=LOC_HOST;j<LOC_LAST;j++)
			for(int k=FMT_BGRA;k<FMT_LAST;k++)
			{
				const fmtinfo& finfo = g_fmtinfo[k];
				int size = m_sfss[i][j][k]*g_resinfo2[i].w*g_resinfo2[i].h*finfo.chs*finfo.bpc/8;
				if(j==LOC_HOST) 
					totalhost += size;
				else
					totalgpu += size;
				if(m_sfss[i][j][k])
				{
					TRACE("(%s,%s,%s)==>%d %d\n",g_resstr[i],g_locstr[j],g_fmtstr[k],m_sfss[i][j][k],m_qss[i][j][k].GetSize());
				}
			}
	TRACE("total host memory:%4.4f MB,total gpu memory:%4.4f MB,AvailPhys:%4.4f MB\n",totalhost/(1024.f*1024.f),totalgpu/(1024.f*1024.f),availphys/(1024.f*1024.f));
#endif
	TRACE("CVxGLMemPool::CreateSurface(%s,%s,%s)\n", g_fmtstr[fmt],g_locstr[loc],g_resstr[(res&VXCUSTOMRESMASK)?RES_CUSTOM:res]);

	DWORD aspect = 0;
	int w, h;
	__vxgetreswh(res,w, h, &aspect);

	int bytes = __getfmtbytes(fmt);

	VXSURFACE_LOCK lock;
	if (loc == LOC_GPUIN)
	{
#ifdef _WIN32
		ASSERT(wglGetCurrentContext() != NULL);
#elif __APPLE__
		ASSERT(CGLGetCurrentContext() != NULL);
#else
		ASSERT(glXGetCurrentContext() != NULL);
#endif
		GLuint pbo = 0;
		lock.pitch = ((w + 0x3F)&~0x3F)*bytes;
		glGenBuffers(1, &pbo);
		if (pbo==0) return NULL;

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, lock.pitch*h, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);  
		lock.pLock = (BYTE*)(vxuintptr)pbo;
	}
	else if((loc==LOC_GPU)||(loc==LOC_GPUOUT))
	{
		GLuint tex = 0;
		GLint internalformat = GL_RGBA;
		GLenum glfmt = GL_RGBA;
		__getglfmt(fmt, internalformat, glfmt);

//		w = (w + 0x3f)&~0x3f;
		if(!GLEW_ARB_texture_non_power_of_two)
		{
			w = GetAlignedSize(w);
			h = GetAlignedSize(h);
		}
#ifdef _WIN32
		ASSERT(wglGetCurrentContext()!=NULL);
#elif __APPLE__
		ASSERT(CGLGetCurrentContext()!=NULL);
#else
        ASSERT(glXGetCurrentContext()!=NULL);
#endif
		LONG totalmemsize,freememsize;
		vxGetGPUMemoryInfo(totalmemsize,freememsize);
		int texw = (glfmt==GL_RGBA)&&(fmt==FMT_YUY2)?w/2:w;
		int need = (texw*h*bytes+1023)/1024;
		if((need+RESERVEMB)>freememsize)
		{
			for(int i=RES_480;i<RES_LAST;i++)
			{
				for(int j=LOC_GPU;j<=LOC_GPUOUT;j++)
					for(int k=FMT_BGRA;k<FMT_LAST;k++)
					{
						IVxSurface* surface;
						while(m_qss[i][j][k].Pop(&surface))
						{
							surface->Release();
							InterlockedDecrement(&m_sfss[i][j][k]);
						}
					}
			}
		}		

		VX_CHECK_GLERROR();
		loc = LOC_GPU;
		glGenTextures(1, &tex);
		if(tex<=0) return NULL;

		VX_CHECK_GLERROR();
		glBindTexture(GL_TEXTURE_2D,tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.5 );
		glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_FALSE );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

		glTexImage2D( GL_TEXTURE_2D,0,internalformat,texw,h,0,glfmt,GL_UNSIGNED_BYTE,0);

		lock.pitch = texw;
		lock.pLock = (BYTE*)(vxuintptr)tex;
		glBindTexture(GL_TEXTURE_2D,0);
		glFlush();
	}
	else
	{
#ifdef _WIN32		
		MEMORYSTATUSEX status = {sizeof(MEMORYSTATUSEX)};
		GlobalMemoryStatusEx(&status);
		if(status.ullAvailVirtual<=0x10000000)
			return NULL;
#endif

		lock.pitch = ((w+0x3F)&~0x3F)*bytes;
		lock.pLock = (BYTE*)_vxmalloc(lock.pitch*h);
		if(!lock.pLock) return NULL;
	}
	VXSURFACE_RES curres = res;
	if (curres&VXCUSTOMRESMASK) curres = RES_CUSTOM;
	VXSURFACE_DESC desc = { fmt, loc, res, COLORIMETRY_UNKNOWN, SCANTYPE_TOPFIRST,w, h, w, h, aspect, bytes, 0 };
	CVxGLSurface* vxs = new CVxGLSurface(this,&m_qss[curres][loc][fmt],&desc,&lock);
#ifndef CHECK_SURFACE
	vxs->AddRef();
#endif // CHECK_SURFACE
	return static_cast<IVxSurface*>(vxs);
}


VXSURFACE_RES CVxGLMemPool::GetRes(int w,int h)
{
	return __vxgetres(w,h);
}

VXBOOL CVxGLMemPool::GetSurface(VXSURFACE_FMT fmt,VXSURFACE_LOC loc,VXSURFACE_RES res,IVxSurface** surface)
{
	if((loc==LOC_GPU)||(loc==LOC_GPUOUT))
		loc = LOC_GPU;
	VXSURFACE_RES curres = res;
	if (curres&VXCUSTOMRESMASK) curres = RES_CUSTOM;
	if(!m_qss[curres][loc][fmt].Pop(surface))
	{
		if(m_sfss[curres][loc][fmt]<m_maxsfss[curres][loc][fmt])
		{
			*surface = CreateSurface(fmt,loc,res);
#ifndef CHECK_SURFACE
			if(*surface) InterlockedIncrement(&m_sfss[curres][loc][fmt]);
#endif
		}
		else if(m_sfss[curres][loc][fmt]<m_maxsfss[curres][loc][fmt]*2)
		{
			//这个是分配的第二级，是在超过规定的大小后，每次申请之前会先看下能不能等到。
			//等不到，则分配，最大为最大值的2倍。
			if((m_sfss[curres][loc][fmt]-m_maxsfss[curres][loc][fmt]) % 16 == 0)
			{
				if(!m_qss[curres][loc][fmt].Pop(surface,1000))
				{
					*surface = CreateSurface(fmt,loc,res);
#ifndef CHECK_SURFACE
					if(*surface) InterlockedIncrement(&m_sfss[curres][loc][fmt]);
#endif
				}
			}
			else
			{
				*surface = CreateSurface(fmt,loc,res);
#ifndef CHECK_SURFACE
				if(*surface) InterlockedIncrement(&m_sfss[curres][loc][fmt]);
#endif
			}
		}
	}
	if(!(*surface))
	{
		if(!m_qss[curres][loc][fmt].Pop(surface,TRUE))
			return FALSE;

	}
	DWORD aspect = 0;
	int w, h;
	__vxgetreswh(res, w, h, &aspect);
	int bytes = __getfmtbytes(fmt);
	VXSURFACE_DESC desc = { fmt, loc, res,COLORIMETRY_UNKNOWN, SCANTYPE_TOPFIRST,w, h, w, h, aspect, bytes };
	(*surface)->SetDesc(&desc);

	(*surface)->AddRef();
	return TRUE;
}


void CVxGLMemPool::Reset(void* rc)
{
	CVxLock cslock(&m_csLock);
#ifdef _WIN32
	HGLRC hglrc = (HGLRC)rc;
	if(hglrc==NULL) hglrc = m_hglrc;
	HGLRC lastrc = wglGetCurrentContext();
	HDC lastdc = wglGetCurrentDC();
	if(lastrc!=hglrc)
	{
		while(!wglMakeCurrent(m_hdc,(HGLRC)hglrc)) vxSleep(2);
	}
#elif __APPLE__
	CGLContextObj cgl = (CGLContextObj)rc;
	if(cgl==NULL) cgl = m_cgl;

	CGLContextObj lastrc = CGLGetCurrentContext();
	if(lastrc!=cgl)
	{
		CGLLockContext(cgl);
		CGLSetCurrentContext((CGLContextObj)cgl);
	}
#else
    GLXContext glx = (GLXContext)rc;
    if(glx==NULL) glx = m_glx;
    GLXContext lastrc = glXGetCurrentContext();
    GLXDrawable lastdc = glXGetCurrentDrawable();
    if(lastrc!=glx)
    {
        while(!glXMakeCurrent(m_xdp,m_xdraw,glx)) vxSleep(2);
    }
#endif
	TRACE("CVxGLMemPool::Reset()\n");

	for(int i=RES_480;i<RES_LAST;i++)
	{
		for(int j=LOC_HOST;j<LOC_LAST;j++)
			for(int k=FMT_BGRA;k<FMT_LAST;k++)
			{
				IVxSurface* surface;
				while(m_qss[i][j][k].Pop(&surface))
				{
					surface->Release();
					InterlockedDecrement(&m_sfss[i][j][k]);
				}
			}
	}
#ifdef _WIN32
	if(lastrc!=hglrc)
	{
		while(!wglMakeCurrent(lastdc,lastrc)) vxSleep(2);
	}
#elif __APPLE__
	if(lastrc!=cgl)
	{
		CGLSetCurrentContext(lastrc);
		CGLUnlockContext(cgl);
	}
#else
    if(lastrc!=glx)
    {
        while(!glXMakeCurrent(m_xdp,lastdc,lastrc)) vxSleep(2);
    }
#endif
}

void CVxGLMemPool::OnDelete()
{
	__int64 totalhost = 0,totalgpu = 0;
	for(int i=RES_480;i<RES_LAST;i++)
		for(int j=LOC_HOST;j<LOC_LAST;j++)
			for(int k=FMT_BGRA;k<FMT_LAST;k++)
			{
                const struct fmtinfo& finfo = g_fmtinfo[k];
				int w, h;
				__vxgetreswh((VXSURFACE_RES)i, w, h);
				int size = m_sfss[i][j][k]*w*h*finfo.bpc*finfo.chs;
				if(j==LOC_HOST) 
					totalhost += size;
				else
					totalgpu += size;

				if(m_sfss[i][j][k])
				{
					int count = 0;
					IVxSurface* surface;
					while(m_qss[i][j][k].Pop(&surface))
					{
						count++;
						surface->Release();
					}
					ASSERT(count==m_sfss[i][j][k]);
					TRACE("(%s,%s,%s)==>%d %d\n",g_resstr[i],g_locstr[j],g_fmtstr[k],m_sfss[i][j][k],(int)count);
				}
			}
	TRACE("total host memory:%4.4f MB,total gpu memory:%4.4f MB\n",totalhost/(1024.f*1024.f),totalgpu/(1024.f*1024.f));
}

CVxNLEEngine::CVxNLEEngine(IVxSystemSetup* setup)
: CVxObject("CVxNLEEngine",NULL)
, m_setup(setup)
, m_showFlag(VSF_SR_PROJECT|VSF_SR_MATTER|VSF_TC_PROJECT|VSF_TC_MATTER)
, m_DrawRect(0,0,0,0)
, m_uinotify(NULL)
, m_avplayer2(NULL)
, m_pages(NULL)
, m_vpool(NULL)
, m_apool(NULL)
#ifdef _WIN32
, m_hglrc(NULL)
, m_hdc(NULL)
#elif __APPLE__
, m_cgl(NULL)
#else
, m_xdp(NULL)
, m_fbc(NULL)
, m_xwin(NULL)
, m_glx(NULL)
, m_glxdraw(NULL)
#endif
, m_multisamples(4)
, m_cl(NULL)
,m_eyeTarget(No_Eye)
{
	_clipInit();
	_taskInit();
	_recordInit();	

	memset(m_shareshaders,0,sizeof(m_shareshaders));
	memset(m_sharegls,0,sizeof(m_sharegls));
	m_sharegl.SetMaxSize(64);
	//m_padLines = 1;
}

CVxNLEEngine::~CVxNLEEngine(void)
{
	g_nleengine = NULL;
}

LONG CVxNLEEngine::NonDelegatingQueryInterface(LONG iid, void** pObject)
{
	if(iid==LIID_IVxNLEEngine)
		return GetVxInterface(this,pObject);
	else
		return CVxObject::NonDelegatingQueryInterface(iid,pObject);
}

VXBOOL CVxNLEEngine::CreateOpenGL()
{
	InitOpenGLExtensions();//函数中会调用glewInit();

	if (!glewIsSupported( "GL_EXT_framebuffer_blit GL_EXT_framebuffer_multisample"))
		m_multisamples = 0;

    const GLint swapinterval = 1;
    
#ifdef _WIN32
	OSVERSIONINFO osver = {sizeof(OSVERSIONINFO)};
	GetVersionEx(&osver);
	if((osver.dwMajorVersion<=5)&&(m_multisamples>4))
		m_multisamples = 4;

	HWND mainwnd = m_setup->GetMainWnd();
	m_hdc = GetDC(mainwnd);
	PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR)};
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int  iPixelFormat = ChoosePixelFormat(m_hdc, &pfd);
/*
	if(m_multisamples>0&&wglChoosePixelFormatARB)
	{
		float	fAttributes[] = {0,0};
		int iAtributes[] = 
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			0,0
		};
		UINT numofFormats;
		int pixelformat[64] = {0};
		if(wglChoosePixelFormatARB(m_hdc, iAtributes, fAttributes, 64 , pixelformat, &numofFormats)&&(numofFormats>0))
		{
			int bestFormat = -1;
			int minDiff = 0x7FFFFFFF;
			int attrib = WGL_SAMPLES_ARB;
			int samples;
			for (UINT i = 0; i < numofFormats; i++)
			{
				wglGetPixelFormatAttribivARB(m_hdc, pixelformat[i], 0, 1, &attrib, &samples);
				int diff = abs(samples-m_multisamples);
				if (diff < minDiff)
				{
					minDiff = diff;
					bestFormat = i;
				}
			}
			if(bestFormat>=0)
			{
				iPixelFormat = pixelformat[bestFormat];
				wglGetPixelFormatAttribivARB(m_hdc, pixelformat[bestFormat], 0, 1, &attrib, &samples);
				m_multisamples = samples;
			}
			else
				m_multisamples = 0;
		}
		else
			m_multisamples = 0;
	}
	*/
	SetPixelFormat(m_hdc, iPixelFormat, &pfd); 
	m_hglrc = wglCreateContext(m_hdc);
	if(!m_hglrc){ReleaseDC(mainwnd,m_hdc);m_hdc=NULL; return FALSE;}

	wglMakeCurrent (m_hdc,m_hglrc);

#elif __APPLE__
	static GLint attribs[] = {
									kCGLPFAAccelerated,
                                    kCGLPFADoubleBuffer,
									kCGLPFAColorSize, 24,
									kCGLPFAAlphaSize, 8,
									kCGLPFADepthSize, 24,
									kCGLPFASampleBuffers,1,
									kCGLPFASamples,m_multisamples,
									0 };
	CGLPixelFormatObj cglPixelFormat = NULL;
	GLint numPixelFormats = 0;
	CGLChoosePixelFormat ((CGLPixelFormatAttribute*)attribs, &cglPixelFormat, &numPixelFormats);
	if(!cglPixelFormat)
	{
		attribs[9] = attribs[10] = attribs[11] = attribs[12] = 0;
		CGLChoosePixelFormat ((CGLPixelFormatAttribute*)attribs, &cglPixelFormat, &numPixelFormats);
		m_multisamples = 0;
	}
	CGLCreateContext(cglPixelFormat, NULL,&m_cgl);
	CGLDestroyPixelFormat(cglPixelFormat);
	if(!m_cgl)
	{
		return FALSE;
	}
	CGLSetCurrentContext(m_cgl);
    CGLSetParameter(m_cgl,kCGLCPSwapInterval,&swapinterval);
#else
    m_xdp = (Display*)m_setup->GetMainWnd();

    int att[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT,
                 GLX_DOUBLEBUFFER, true,
                 GLX_DEPTH_SIZE,1,
                 GLX_SAMPLE_BUFFERS,m_multisamples?1:0,
                 GLX_SAMPLES,m_multisamples,
                 None};
    int nelements = 0;
    m_fbc = glXChooseFBConfig(m_xdp,
                            DefaultScreen(m_xdp),
                            att,
                            &nelements);
    if(!m_fbc)
    {
        int att2[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT,
                     GLX_DOUBLEBUFFER, true,
                     GLX_DEPTH_SIZE,1,
                     None};
        m_fbc = glXChooseFBConfig(m_xdp,
                                DefaultScreen(m_xdp),
                                att2,
                                &nelements);
        m_multisamples = 0;
    }
    if(!m_fbc)
        return FALSE;
    XVisualInfo *vi = glXGetVisualFromFBConfig (m_xdp,*m_fbc);
    if(!vi)
        return FALSE;

    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(m_xdp,
                        RootWindow(m_xdp, vi->screen),
                        vi->visual,
                        AllocNone);
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;
    m_xwin = XCreateWindow(m_xdp,
                RootWindow(m_xdp, vi->screen),
                0,
                0,
                2,
                2,
                0,
                vi->depth,
                InputOutput,
                vi->visual,
                CWColormap|CWEventMask,
                &swa);
    if(!m_xwin)
        return FALSE;
    m_glxdraw = glXCreateWindow (m_xdp, *m_fbc, m_xwin, NULL);
    m_glx = glXCreateNewContext(m_xdp,*m_fbc,GLX_RGBA_TYPE,NULL,true);
    glXMakeCurrent (m_xdp,m_glxdraw,m_glx);
#endif

#ifdef _WIN32
	vxRegisterGLRC(m_hglrc);
#elif __APPLE__
	vxRegisterGLRC(m_cgl);
#else
    vxRegisterGLRC(m_glx);
#endif

	if(HasGLSLSupport()&&IsShaderGraphic())
	{
		const char* shader_filename[shader_max] = {
			"ytor601.frag.glsl","ytor709.frag.glsl","ytor2020.frag.glsl",
			"ytor601_alpha.frag.glsl","ytor709_alpha.frag.glsl","ytor2020_alpha.frag.glsl",
			"rtoy601_rgba.frag.glsl","rtoy709_rgba.frag.glsl","rtoy2020_rgba.frag.glsl",
			"rtoy601_bgra.frag.glsl","rtoy709_bgra.frag.glsl","rtoy2020_bgra.frag.glsl",
			"deinterlace.frag.glsl",
			"stretchwidth.frag.glsl",
			"imagescale.frag.glsl",
			"wipe.frag.glsl"
		};
		VXBOOL force = FALSE;
#ifdef _DEBUG
		force = TRUE;
#endif
		for (int i = 0; i < shader_max; i++)
		{
			void* frag_glsl = NULL;
			if (vxLoadShader(shader_filename[i], &frag_glsl, force) < 0) return FALSE;
			m_shareshaders[i] = g_shaderManager->loadfromMemory(NULL, (const char*)frag_glsl); // load (and compile, link) from file
			_vxfree(frag_glsl);
		}
		vxRegisterDefaultShader(m_shareshaders);
	}
	return TRUE;
}

void CVxNLEEngine::DestroyOpenGL()
{
	vxRegisterDefaultShader(NULL);
	vxRegisterGLRC(NULL);

	for(int i=0;i<shader_max;i++)
	{
		if(m_shareshaders[i]) g_shaderManager->Free(m_shareshaders[i]);
	}
	memset(m_shareshaders,0,sizeof(m_shareshaders));

#ifdef _WIN32
	wglMakeCurrent(0,0);
	if(m_hglrc)
	{
		for(int i=0;i<shardgl_maxs;i++)
		{
			if(m_sharegls[i]) 
				wglDeleteContext(m_sharegls[i]);
		}

		HGLRC rc;
		while(m_sharegl.Pop(&rc))
			wglDeleteContext(rc);
		wglDeleteContext(m_hglrc);
		ReleaseDC(m_setup->GetMainWnd(),m_hdc);
		m_hglrc = NULL;
		m_hdc = NULL;
	}
#elif __APPLE__
	CGLSetCurrentContext(NULL);
	if(m_cgl)
	{
		for(int i=0;i<shardgl_maxs;i++)
		{
			if(m_sharegls[i]) 
				CGLReleaseContext(m_sharegls[i]);
		}

		CGLContextObj rc;
		while(m_sharegl.Pop(&rc))
			CGLReleaseContext(rc);
		CGLReleaseContext(m_cgl);
		m_cgl = NULL;
	}
#else
    glXMakeCurrent(m_xdp,0,0);
    if(m_glx)
    {
        for(int i=0;i<shardgl_maxs;i++)
        {
            if(m_sharegls[i])
                glXDestroyContext(m_xdp,m_sharegls[i]);
        }

        GLXContext rc;
        while(m_sharegl.Pop(&rc))
             glXDestroyContext(m_xdp,rc);

        glXDestroyContext(m_xdp,m_glx);
        glXDestroyWindow(m_xdp,m_glxdraw);
        XDestroyWindow(m_xdp,m_xwin);
        m_glx = NULL;
        m_glxdraw = NULL;
        m_xwin = NULL;
        m_fbc = NULL;
        m_xdp = NULL;
    }
#endif
	memset(m_sharegls,0,sizeof(m_sharegls));

}


#if defined (__APPLE__) || defined(MACOSX)
#define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif



VXBOOL CVxNLEEngine::CreateOpenCL()
{
	cl_platform_id cpPlatform;
	cl_int ciErrNum = oclGetPlatformID(&cpPlatform);
	if(ciErrNum<0) return FALSE;


	// Get OpenCL platform name and version
	char cBuffer[1024];
	ciErrNum = _clGetPlatformInfo (cpPlatform, CL_PLATFORM_NAME, sizeof(cBuffer), cBuffer, NULL);
	if(ciErrNum == CL_SUCCESS)
		TRACE(" CL_PLATFORM_NAME: \t%s\n", cBuffer);
	ciErrNum = _clGetPlatformInfo (cpPlatform, CL_PLATFORM_VERSION, sizeof(cBuffer), cBuffer, NULL);
	if (ciErrNum == CL_SUCCESS)
		TRACE(" CL_PLATFORM_VERSION: \t%s\n", cBuffer);


	cl_uint uiDevCount;
	ciErrNum = _clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiDevCount);
	if (ciErrNum != CL_SUCCESS) return FALSE;

	cl_device_id* cdDevices = new cl_device_id [uiDevCount];
	ciErrNum = _clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, uiDevCount, cdDevices, NULL);

	cl_uint uiDeviceUsed = 0;
	bool bSharingSupported = false;
	for(unsigned int i = 0; (!bSharingSupported && (i<uiDevCount)); ++i) 
	{
		size_t extensionSize;
		ciErrNum = _clGetDeviceInfo(cdDevices[i], CL_DEVICE_EXTENSIONS, 0, NULL, &extensionSize );

		if(extensionSize > 0) 
		{
			char* extensions = (char*)malloc(extensionSize+10);
			ciErrNum = _clGetDeviceInfo(cdDevices[i], CL_DEVICE_EXTENSIONS, extensionSize, extensions, &extensionSize);
			extensions[extensionSize-1] = ' ';extensions[extensionSize] = '\n';extensions[extensionSize+1] = 0;
			TRACE(extensions);
			std::string stdDevString(extensions);
			free(extensions);

			size_t szOldPos = 0;
			size_t szSpacePos = stdDevString.find(' ', szOldPos); // extensions string is space delimited
			while (szSpacePos != stdDevString.npos)
			{
				if( strcmp(GL_SHARING_EXTENSION, stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0 ) 
				{
					// Device supports context sharing with OpenGL
					uiDeviceUsed = i;
					bSharingSupported = true;
					break;
				}
				do 
				{
					szOldPos = szSpacePos + 1;
					szSpacePos = stdDevString.find(' ', szOldPos);
				} 
				while (szSpacePos == szOldPos);
			}
		}
	}
	if(!bSharingSupported)
	{
		delete[] cdDevices;
		return FALSE;
	}
	cl_device_id seldevid = cdDevices[uiDeviceUsed];
	delete[] cdDevices;

	TRACE(" ---------------------------------\n");
	_clGetDeviceInfo(seldevid, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
	TRACE(" Device %s\n", cBuffer);
	TRACE(" ---------------------------------\n");
	// CL_DEVICE_NAME
	_clGetDeviceInfo(seldevid, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
	TRACE("  CL_DEVICE_NAME: \t\t\t%s\n", cBuffer);
	vxString devname(cBuffer);

	// CL_DEVICE_VENDOR
	_clGetDeviceInfo(seldevid, CL_DEVICE_VENDOR, sizeof(cBuffer), &cBuffer, NULL);
	TRACE("  CL_DEVICE_VENDOR: \t\t\t%s\n", cBuffer);
	vxString sinfovendor(cBuffer);

	VXBOOL nvidia = sinfovendor.Find("NVIDIA")!=-1;
	VXBOOL ati = sinfovendor.Find("Advanced Micro Devices")!=-1;
	VXBOOL intel = sinfovendor.Find("Intel") != -1;
	if (ati&&m_multisamples) return FALSE;
	// CL_DRIVER_VERSION
	_clGetDeviceInfo(seldevid, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	TRACE("  CL_DRIVER_VERSION: \t\t\t%s\n", cBuffer);
	vxString sdrvversion(cBuffer);

	// CL_DEVICE_VERSION
	_clGetDeviceInfo(seldevid, CL_DEVICE_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	TRACE("  CL_DEVICE_VERSION: \t\t\t%s\n", cBuffer);

	// CL_DEVICE_MAX_MEM_ALLOC_SIZE
	cl_ulong max_mem_alloc_size;
	_clGetDeviceInfo(seldevid, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
	TRACE("  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t%u MByte\n", (unsigned int)(max_mem_alloc_size / (1024 * 1024)));

	// CL_DEVICE_GLOBAL_MEM_SIZE
	cl_ulong mem_size;
	_clGetDeviceInfo(seldevid, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	TRACE("  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t%u MByte\n", (unsigned int)(mem_size / (1024 * 1024)));

	// CL_DEVICE_LOCAL_MEM_TYPE
	cl_device_local_mem_type local_mem_type;
	_clGetDeviceInfo(seldevid, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
	TRACE("  CL_DEVICE_LOCAL_MEM_TYPE:\t\t%s\n", local_mem_type == 1 ? "local" : "global");

	// CL_DEVICE_LOCAL_MEM_SIZE
	_clGetDeviceInfo(seldevid, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	TRACE("  CL_DEVICE_LOCAL_MEM_SIZE:\t\t%u KByte\n", (unsigned int)(mem_size / 1024));

	// CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
	_clGetDeviceInfo(seldevid, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(mem_size), &mem_size, NULL);
	TRACE("  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:\t%u KByte\n", (unsigned int)(mem_size / 1024));

	//CL_DEVICE_IMAGE_SUPPORT
	cl_bool imgsupport = 0;
	_clGetDeviceInfo(seldevid, CL_DEVICE_IMAGE_SUPPORT, sizeof(imgsupport), &imgsupport, NULL);
	TRACE("  CL_DEVICE_IMAGE_SUPPORT:\t%s\n",imgsupport?"YES":"NO");

	// Define OS-specific context properties and create the OpenCL context
#if defined (__APPLE__)
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(m_cgl);
	cl_context_properties props[] = {CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 
										0};
	m_cl = _clCreateContext(props, 0,0, NULL, NULL, &ciErrNum);
#elif defined(_WIN32)
	cl_context_properties props[] = 
									{
										CL_GL_CONTEXT_KHR, (cl_context_properties)m_hglrc, 
										CL_WGL_HDC_KHR, (cl_context_properties)m_hdc, 
										CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform, 
										0
									};
	m_cl = _clCreateContext(props, 1, &seldevid, NULL, NULL, &ciErrNum);
#endif
	
/*
	cl_uint uiNumSupportedFormats = 0;
	_clGetSupportedImageFormats(m_cl, CL_MEM_READ_ONLY,CL_MEM_OBJECT_IMAGE2D,NULL, NULL, &uiNumSupportedFormats);
	cl_image_format* ImageFormats = new cl_image_format[uiNumSupportedFormats];
	_clGetSupportedImageFormats(m_cl, CL_MEM_READ_ONLY,CL_MEM_OBJECT_IMAGE2D,uiNumSupportedFormats, ImageFormats, NULL);
	VXBOOL supportbgra = FALSE;
	for(int i=0;i<uiNumSupportedFormats;i++)
	{
		if(ImageFormats[i].image_channel_order==CL_BGRA)
		{
			supportbgra = TRUE;
			break;
		}
	}
	delete[] ImageFormats;

	if(!supportbgra)
	{
		_clReleaseContext(m_cl);
		m_cl = NULL;
	}
*/
#if defined(_WIN32)	
	if (!((nvidia&&devname >= "GeForce 8"&&sdrvversion >= "260") || (ati&&sdrvversion >= "CAL 1.4.1000") || (intel&&sdrvversion >= "10.18.10.4061")))
	{
		_clReleaseContext(m_cl);
		m_cl = NULL;
	}
#elif __APPLE__
    DWORD version = vxGetOSVersion();
    VXBOOL lion100704up = ( version >= 0x100704 );
	if(lion100704up&&nvidia)
	{
		_clReleaseContext(m_cl);
		m_cl = NULL;
	}
#endif	
 	return m_cl!=NULL;
}

void CVxNLEEngine::DestroyOpenCL()
{
	if(m_cl){_clReleaseContext(m_cl);m_cl = NULL;}
}

VXBOOL CVxNLEEngine::Initialize()
{
	if(!CreateOpenGL()) return FALSE;

#if 0//!defined(_WIN32)
	CreateOpenCL();
#endif

#ifdef _WIN32
	GetVxInterface(static_cast<IVxVidMemPool*>(new CVxGLMemPool(m_hdc,m_hglrc,m_cl)),(void**)&m_vpool);
#elif __APPLE__
	GetVxInterface(static_cast<IVxVidMemPool*>(new CVxGLMemPool(m_cgl,m_cl)),(void**)&m_vpool);
#else
    GetVxInterface(static_cast<IVxVidMemPool*>(new CVxGLMemPool(m_xdp,m_glxdraw,m_glx,m_cl)),(void**)&m_vpool);
#endif
	avio_cinfo cinfo = { g_res, SCANTYPE_PROGRESSIVE, COLORIMETRY_BT709, FMT_YUY2, g_rate, g_scale, 0, g_freq, 0 };
	m_setup->CreateAudMemPool(&cinfo,&m_apool);

	int step = 0;
	if(m_xInitEffect.Init(0,step)<0)
		return FALSE;

	vxInitEffect(static_cast<IVxInitEffect*>(&m_xInitEffect));

	bool isShaderGraphic = HasGLSLSupport()&&IsShaderGraphic();
	m_xCGFace.InitBy(m_setup,m_vpool,isShaderGraphic);
	IVxCGFaceUP::InitLV(&m_xCGFace);
	IVxCGRenderTool::Init();

	vxRegisterStaticCB(staticframecb,this);
	VX_CHECK_GLERROR();
#ifdef _WIN32
	wglMakeCurrent(0,0);
#elif __APPLE__
	CGLSetCurrentContext(NULL);
#else
    glXMakeCurrent(m_xdp,0,0);
#endif
	return TRUE;
}


void LeakDump(vxString& roOut);

void CVxNLEEngine::Uninitialize()
{
#ifdef _WIN32
	wglMakeCurrent(m_hdc,m_hglrc);
#elif __APPLE__
	CGLSetCurrentContext(m_cgl);
#else
    glXMakeCurrent(m_xdp,m_glxdraw,m_glx);
#endif

	_clipExit();
	_taskExit();
	_recordExit();


#if defined(_DEBUG)&&defined(_WIN32)
	vxString leaks;
	LeakDump(leaks);
#endif
	vxRegisterStaticCB(NULL,NULL);

	IVxCGRenderTool::Exit();
	IVxCGFaceUP::ExitLV();
	m_xCGFace.Exit();


	vxExitEffect(static_cast<IVxInitEffect*>(&m_xInitEffect));

	m_xInitEffect.Exit();
	
	m_apool = NULL;
	m_vpool = NULL;
	m_liveExt = NULL;

	DestroyOpenCL();
	DestroyOpenGL();
}

void __exitthreadgl(void* p);

#ifdef _WIN32
VXRESULT _vxCreateOpenGLEngine(IVxSystemSetup* setup, IVxSystemClock* clock, IVxVidMemPool* pool, const avio_cinfo* cinfo, EFXARRAY* efxplugins, HDC hdc, HGLRC glrc, int multisamples, cl_context cl, IVxVideoEfx** efxengine);
VXRESULT _vxCreateOpenGL3DEngine(IVxSystemSetup* setup, IVxSystemClock* clock, IVxVidMemPool* pool, const avio_cinfo* cinfo, EFXARRAY* efxplugins, HDC hdc, HGLRC glrc, int multisamples, cl_context cl, IVxVideoEfx** efxengine);
#elif __APPLE__
VXRESULT _vxCreateOpenGLEngine(IVxSystemSetup* setup,IVxSystemClock* clock,IVxVidMemPool* pool,const avio_cinfo* cinfo,EFXARRAY* efxplugins,CGLContextObj cgl,int multisamples,cl_context cl,IVxVideoEfx** efxengine);
VXRESULT _vxCreateOpenGL3DEngine(IVxSystemSetup* setup,IVxSystemClock* clock,IVxVidMemPool* pool,const avio_cinfo* cinfo,EFXARRAY* efxplugins,CGLContextObj cgl,int multisamples,cl_context cl,IVxVideoEfx** efxengine);
#else
VXRESULT _vxCreateOpenGLEngine(IVxSystemSetup* setup,IVxSystemClock* clock,IVxVidMemPool* pool,const avio_cinfo* cinfo,EFXARRAY* efxplugins,GLXDrawable draw,GLXContext glx,int multisamples,cl_context cl,IVxVideoEfx** efxengine);
VXRESULT _vxCreateOpenGL3DEngine(IVxSystemSetup* setup,IVxSystemClock* clock,IVxVidMemPool* pool,const avio_cinfo* cinfo,EFXARRAY* efxplugins,GLXDrawable draw,GLXContext glx,int multisamples,cl_context cl,IVxVideoEfx** efxengine);
#endif
VXRESULT _vxCreateSimpleVideoEfxEngine(IVxSystemSetup* setup, IVxSystemClock* clock, IVxVidMemPool* pool, const avio_cinfo* cinfo, IVxVideoEfx** efxengine);

VXRESULT CVxNLEEngine::CreateNLEPlayer(avio_cinfo* cinfo,IVxVideoOutput* vout,IVxLiveExt* liveext,videfxtype etype,IVxAudioOutput* aout,IVxAudioOutput* alive,IVxAVPlayer2** avplayer2)
{
	if(cinfo->res==RES_3D_1080){cinfo->res = RES_1080,etype=vefx_gpu3d;}
/*
	if (((etype == vefx_gpu) || (etype == vefx_gpu3d)) && (cinfo->fmt == FMT_YUV422P8))
		cinfo->fmt = FMT_YUY2;
*/
	
	CVxComPtr<IVxSystemClock> clock;
	if(vout)
	{
		vout->QueryInterface(LIID_IVxSystemClock,(void**)&clock);
	}
	else if(aout)
	{
		aout->QueryInterface(LIID_IVxSystemClock,(void**)&clock);
	}
	else if(alive)
	{
		alive->QueryInterface(LIID_IVxSystemClock,(void**)&clock);
	}
	else if(liveext)
	{
		liveext->QueryInterface(LIID_IVxSystemClock,(void**)&clock);
	}

	if(!clock)
	{
		vxCreateSystemClock(m_setup,(avio_cinfo*)cinfo,NULL,&clock);
	}

	if ((g_rate != cinfo->rate) || (g_scale != cinfo->scale) || (g_freq != cinfo->freq))
	{
		m_apool = NULL;
		m_setup->CreateAudMemPool(cinfo, &m_apool);
	}

	g_res = cinfo->res;
	g_rate = cinfo->rate;
	g_scale = cinfo->scale;
	g_aspect = cinfo->aspect;
	g_freq = cinfo->freq;
	__vxgetreswh(g_res, g_width, g_height);

	int framestep;
	int timestep = __vxgettimestep(cinfo, framestep);
	VXBOOL createvideo = (cinfo->rate*timestep / cinfo->scale) <= 60;


	VXRESULT lr = VXNOERROR;
	CVxComPtr<IVxVideoEfx> vefxengine;
	CVxComPtr<IVxAudioEfx> aefxengine;
#if 1
	if (createvideo&&(etype == vefx_gpu || etype == vefx_gpu3d))
	{
		if(!HasOpenGL2Support()) 
		{
			VX_MailMSG(vxLoadMessageLV("需要支持OpenGL 2.0和Shader显卡！"),vxLoadMessageLV("错误(Error):播放引擎"),0,MAILSRC_HWENGINE|MAILSRC_ERROR);
			return -1;
		}
		if(!glGenBuffers||!glGenFramebuffersEXT)
		{
			VX_MailMSG(vxLoadMessageLV("需要支持PBO,FBO和Shader的OpenGL显卡！"),vxLoadMessageLV("错误(Error):播放引擎"),0,MAILSRC_HWENGINE|MAILSRC_ERROR);
			return -1;
		}
		if(!HasGLSLSupport()||!IsShaderGraphic()) return -1;

		LONG totalmemsize,freememsize;
		vxGetGPUMemoryInfo(totalmemsize,freememsize);
		cl_context cl = m_cl;
		if((totalmemsize<=524288)&&(cinfo->res>=RES_720))
		{
			cl = NULL;
			m_multisamples = 0;
		}
		if(etype==vefx_gpu)
		{
#ifdef _WIN32
			lr = _vxCreateOpenGLEngine(m_setup,clock,m_vpool,cinfo,&m_xInitEffect.m_dataVid,m_hdc,m_hglrc,m_multisamples,cl,&vefxengine);
#elif __APPLE__
			lr = _vxCreateOpenGLEngine(m_setup,clock,m_vpool,cinfo,&m_xInitEffect.m_dataVid,m_cgl,m_multisamples,cl,&vefxengine);
#else
            lr = _vxCreateOpenGLEngine(m_setup,clock,m_vpool,cinfo,&m_xInitEffect.m_dataVid,m_glxdraw,m_glx,m_multisamples,cl,&vefxengine);
#endif
		}
		else
		{
#ifdef _WIN32
			lr = _vxCreateOpenGL3DEngine(m_setup,clock,m_vpool,cinfo,&m_xInitEffect.m_dataVid,m_hdc,m_hglrc,m_multisamples,cl,&vefxengine);
#elif __APPLE__
			lr = _vxCreateOpenGL3DEngine(m_setup,clock,m_vpool,cinfo,&m_xInitEffect.m_dataVid,m_cgl,m_multisamples,cl,&vefxengine);
#else
            lr = _vxCreateOpenGL3DEngine(m_setup,clock,m_vpool,cinfo,&m_xInitEffect.m_dataVid,m_glxdraw,m_glx,m_multisamples,cl,&vefxengine);
#endif
		}
		if(lr) 
		{
			VX_MailMSG(vxLoadMessageLV("初始化OpenGL特技引擎失败！"),vxLoadMessageLV("错误(Error):播放引擎"),0,MAILSRC_HWENGINE|MAILSRC_ERROR);
			return lr;
		}
		//_vxCreateCCTVAudEfxEngine(m_setup,m_apool,audfreq,1,&aefxengine);
		vxCreateBufferAudEfxEngine(m_setup, clock, m_apool, cinfo, 1, &aefxengine);

		EFXARRAY& audefxs = m_xInitEffect.m_dataA;
		int counts = audefxs.GetSize();
		for(int i=0;i<counts;i++)
		{
			EFXINITEX& aefx = audefxs[i];
			if(aefx.guid==MAKEDAIEFFECTID(1))
			{
				audefxs.RemoveAt(i);
				break;
			}
		}
	}
	else
	{
		if (createvideo) 
			_vxCreateSimpleVideoEfxEngine(m_setup, clock, m_vpool, cinfo, &vefxengine);
		//_vxCreateCCTVAudEfxEngine(m_setup,m_apool,audfreq,etype==vefx_cpu?0:1,&aefxengine);
		vxCreateBufferAudEfxEngine(m_setup, clock, m_apool, cinfo,etype == vefx_cpu ? 0 : 1, &aefxengine);
	}
#endif

	// P-N制变换时，需要重新创建VideoLive,不然无法更新时钟
	CVxComPtr<IVxLiveExt> inliveext(liveext);
	if(createvideo&&!inliveext)
	{
		if(m_liveExt)
		{
			CVxComPtr<IVxLiveExt2> live2;
			if(m_liveExt->QueryInterface(LIID_IVxLiveExt2,(void**)&live2)==0)
			{
				live2->Reset(clock, g_width, g_height);
			}
			else
			{
				m_liveExt = NULL;
				if (m_setup->CreateVideoLive(vxVideoLive_gl, clock, g_width, g_height, &m_liveExt) != 0)
					m_setup->CreateVideoLive(0, clock, g_width, g_height, &m_liveExt);
			}
		}
		else
		{
			if (m_setup->CreateVideoLive(vxVideoLive_gl, clock, g_width, g_height, &m_liveExt) != 0)
				m_setup->CreateVideoLive(0, clock, g_width, g_height, &m_liveExt);
		}
		inliveext = m_liveExt;
	}

	CVxComPtr<IVxAVPlayer> avplayer;
	LONG ret = m_setup->CreateAVPlayer(cinfo,vefxengine,inliveext,vout,aefxengine,alive,aout,&avplayer);
	if(ret) 
	{
		VX_MailMSG(vxLoadMessageLV("初始化播放引擎失败！"),vxLoadMessageLV("错误(Error):播放引擎"),0,MAILSRC_HWENGINE|MAILSRC_ERROR);
		return ret;
	}
	ret = avplayer->QueryInterface(LIID_IVxAVPlayer2,(void**)avplayer2);
	if(ret) return ret;

	m_avplayer2 = *avplayer2;

	struct local
	{
		static void player2destructorcb(IVxAVPlayer2* player2,void* cbp)
		{((CVxNLEEngine*)cbp)->__player2destructor(player2);}
		static void player2dcchgGpucb(IVxAVPlayer2* player2,IVxDataCore* dc,void* p )
		{	((CVxNLEEngine*)p)->__player2dcchgGpu( player2,dc );	}
		static void player2dcchgCpucb(IVxAVPlayer2* player2,IVxDataCore* dc,void* p )
		{	((CVxNLEEngine*)p)->__player2dcchgCpu( player2,dc );	}
		static VXRESULT drawopfunccb( CGRENDERINFO* drawparam,void* p )
		{	return	((CVxNLEEngine*)p)->__drawopfunc( drawparam )?0:-1;	}
		static VXRESULT drawbeforeoutputfunccb( BeforeOutputSurface* drawparam,void* p )
		{	return	((CVxNLEEngine*)p)->__drawbeforeoutputfunccb( drawparam )?0:-1;	}
		static VXRESULT drawuifunccb( UIDRAW* uid,void* p )
		{	return	((CVxNLEEngine*)p)->__drawuifunc(uid)?0:-1;	}
	};
	m_avplayer2->RegisterDestructorFunc(local::player2destructorcb,this);
	if (etype == vefx_gpu || etype == vefx_gpu3d)
		m_avplayer2->RegisterDatacoreChangeFunc(local::player2dcchgGpucb,this);
	else
		m_avplayer2->RegisterDatacoreChangeFunc(local::player2dcchgCpucb, this);

	m_avplayer2->RegisterCGFunc(local::drawopfunccb,this);
	m_avplayer2->RegisterBeforeOutputFunc(local::drawbeforeoutputfunccb,this);
	m_avplayer2->RegisterDrawUIFunc(local::drawuifunccb,this);
	m_avplayer2->RegisterExitThreadGLFunc(__exitthreadgl,this);



	g_res = cinfo->res;
	g_fmt = cinfo->fmt;
	g_colorimetry = cinfo->colorimetry;
	g_aspect = cinfo->aspect;
	g_rate = cinfo->rate;
	g_scale = cinfo->scale;
	g_freq = cinfo->freq;

	int aspectw = g_aspect&0xFFFF;
	int aspecth = g_aspect>>16;
	VXNLEVIDINFO nleinfo = { g_width, g_height, g_scale, g_rate, aspectw, aspecth, g_width, g_height };
	vxInitVideoInfo(nleinfo);
	m_framescale = (float)aspectw/aspecth;

	if (cinfo->fmt == FMT_YUY2)
	{
		m_yuvafmt = FMT_YUY2A;
	}
	else if (cinfo->fmt == FMT_YUV422P8)
	{
		m_yuvafmt = FMT_YUVA4224P8;
	}
	else
	{
		m_yuvafmt = FMT_YUVA4224P16;
	}
	bgrx_to_yuv = bgra_to_yuvs[cinfo->fmt][cinfo->colorimetry];
	bgra_to_yuv = bgra_to_yuvs[m_yuvafmt][cinfo->colorimetry];


	if(g_hcgeditor&&((etype>=vefx_gpu)||(etype>=vefx_cpu2)))
	{
		IVxCGRenderTool::Exit();
		if(etype>=vefx_gpu)
		{
			IVxCGFaceUP::InitLV(&m_xCGFace);
		}
		else
		{
			IVxCGFaceUP::InitLV(&m_xCGCpuFace);
		}
		IVxCGRenderTool::Init();

		CREATEULTRACGEDIT createeditor = (CREATEULTRACGEDIT)GetProcAddress(g_hcgeditor,"vxCreateUltraCGEdit");
		createeditor(NULL, CVxSize(g_width, g_height), CVxSize(aspectw, aspecth), NULL, NULL);
	}
	
	if(!m_pages&&!vxGetPageManager())
	{
		m_pages = vxCreatePageManager();
		vxRegisterDefaultManager(m_pages);
	}

	CVxComPtr<IVxLiveExt2> live2;
	m_avplayer2->QueryInterface(LIID_IVxLiveExt2,(void**)&live2);
	if(live2) live2->Set3DMode(etype==vefx_gpu3d?1:-1);

	return 0;
}

void CVxNLEEngine::GetEffectPath(char* path)
{
	vxString strPath = vxGetSysDirection("FX Plugins");
	strcpy(path,strPath.c_str());
}

VXBOOL EfxMatchFlag( DWORD efxFlag,DWORD engFlag )
{
	//	1，需要查看是否支持
	return TRUE;
}


VXRESULT CVxNLEEngine::OnLoadEfx( DWORD flag,const char* filename,EFXARRAY* array)
{
	HMODULE handle = LoadLibraryEx( filename,0, LOAD_WITH_ALTERED_SEARCH_PATH);
	if( handle == NULL )
		return VX_E_NOTFOUND;

	EEFFECTMAIN effect;
	if( (effect = (EEFFECTMAIN)GetProcAddress( handle,"VxEffectMain" ) ) == NULL )
	{
		FreeLibrary( handle );
		return VX_E_INVALIDFILE;
	}

	CVxComPtr<IVxEffectLoad> loader;
	if(effect(this,&loader)!=0)
	{
		FreeLibrary( handle );
		return VX_E_ENGINELACK;
	}

	int nefx = loader->GetEfxs();
	if( nefx > 0 )
	{
		int nAdd = array->GetSize();
		for( int i = 0; i < nefx; i++ )
		{
			EFXINITEX efxinit;
			loader->GetEfx( i,&efxinit );
			if( EfxMatchFlag( efxinit.flag,flag ) )
			{
				efxinit.library = NULL;
				efxinit.loader = NULL;
				array->Add( efxinit );
			}
		}
		if( array->GetSize() > nAdd )
		{
			(*array)[array->GetSize()-1].library = handle;
			(*array)[array->GetSize()-1].loader = loader;
			loader->AddRef();
		}
		else
		{
			FreeLibrary( handle );
			return VX_E_ENGINELACK;
		}

		return VX_SUCCESS;
	}
	else
	{
		FreeLibrary( handle );
		return VX_E_ENGINELACK;
	}
}

vxinline void ReleaseEfxArray(EFXARRAY& efxar)
{
	for( int i = 0; i < efxar.GetSize(); i++ )
	{							
		if( efxar[i].library )		
		{											
			efxar[i].loader->Release();			
			FreeLibrary( efxar[i].library );		
		}											
	}												
	efxar.RemoveAll();
}

CVxNLEEngine::XInitEffect::~XInitEffect()
{
}

#include "VxInhereEfx.cxx"
#include "VxPlayerData.h"

VXRESULT ExecuteAlpha(void* p,VXGUID,void* param,void* key,void* data );
VXRESULT ExecuteFader(void* p,VXGUID,void* param,void* key,void* data );
VXRESULT ExecuteStandardWipe(void* p,VXGUID,void* param,void* key,void* data );
VXRESULT ExecuteMatrixWipe(void* p,VXGUID,void* param,void* key,void* data );
VXRESULT ExecuteImageWipe(void* p,VXGUID,void* param,void* key,void* data );

VXRESULT CVxNLEEngine::XInitEffect::Init( DWORD reserve,int& step )
{
	vxString strPath = vxGetSysDirection(NULL);
	char szadv[MAX_VXPATH] = {0};
#ifdef _WIN32
#ifdef _DEBUG
	sprintf(szadv,"%s\\%s",(const vxChar*)strPath,"VxEfxPanelAdvD.dll");
#else
	sprintf(szadv,"%s\\%s",(const vxChar*)strPath,"VxEfxPanelAdv.dll");
#endif
#elif __APPLE__
	sprintf(szadv,"%s/%s",(const vxChar*)strPath,"VxEfxPanelAdv.framework/VxEfxPanelAdv");
#else
	sprintf(szadv,"%s/%s",(const vxChar*)strPath,"libVxEfxPanelAdv.so");
#endif
	m_heditor = LoadLibraryEx(szadv,0, LOAD_WITH_ALTERED_SEARCH_PATH);
	if(m_heditor)
	{
		g_bitmapmenu = (BITMAPMENU)GetProcAddress(m_heditor,"vxCreateBitmapMenu");
		g_iconmenu = (ICONMENU)GetProcAddress(m_heditor,"vxCreateIconMenu");
	}
	
	//	初始化Effect列表的接口
	CVxNLEEngine* pThis = ((CVxNLEEngine*)(((BYTE*)this) - offsetof(CVxNLEEngine,m_xInitEffect)));

	EFXINITEX tInit;
	memset( &tInit,0,sizeof(tInit) );
	tInit.guid			= VXALPHAEFFECTID;//MAKEDVIEFFECTID( 1 );
	tInit.name			= vxLoadMessageLV("基本运动");
	tInit.folder		= vxLoadMessageLV("二维特技");
	tInit.flag			= EFXFLAG_INHERE;
	tInit.keylength		= sizeof(VXVFX_DVE2D_KEY);
	tInit.paramlength	= sizeof(VXVFX_DVE2D_PARAM);	
	tInit.setdefault	= SetAphaDefault;
	tInit.serialize		= SerializeApha;
	tInit.getformat		= GetAphaFormat;
	tInit.bitmap		= GetAphaBitmap;
	tInit.effect		= ExecuteAlpha;
	tInit.effecttype	= GPUEFXPARAM|EFXTYPE_GPU;
	m_dataV.Add( tInit );

	memset( &tInit,0,sizeof(tInit) );
	tInit.guid			= MAKEDAIEFFECTID( 0 );
	tInit.name			= vxLoadMessageLV("音    量");
	tInit.folder		= vxLoadMessageLV("音频调整");
	tInit.flag			= EFXFLAG_INHERE;
	tInit.keylength		= sizeof(float);
	tInit.paramlength	= 0;	
	tInit.setdefault	= SetIntensityDefault;
	tInit.serialize		= SerializeIntensity;
	tInit.getformat		= GetIntensityFormat;
//	tInit.editor		= CreateIntensityEditor;
	tInit.bitmap		= GetIntensityBitmap;
	m_dataA.Add( tInit );

	memset( &tInit,0,sizeof(tInit) );
	tInit.guid			= MAKEDAIEFFECTID( 1 );
	tInit.name			= vxLoadMessageLV("左右平衡");
	tInit.folder		= vxLoadMessageLV("音频调整");
	tInit.flag			= EFXFLAG_INHERE;
	tInit.keylength		= sizeof(float);
	tInit.paramlength	= 0;	
	tInit.setdefault	= SetBalanceDefault;
	tInit.serialize		= SerializeBalance;
	tInit.getformat		= GetBalanceFormat;
	//	tInit.editor		= CreateBalanceEditor;
	tInit.bitmap		= GetBalanceBitmap;
	m_dataA.Add( tInit );

	memset( &tInit,0,sizeof(tInit) );
	tInit.guid			= MAKEDAIEFFECTID( 2 );
	tInit.name			= vxLoadMessageLV("音频增益");
	tInit.folder		= vxLoadMessageLV("音频调整");
	tInit.flag			= EFXFLAG_INHERE;
	tInit.keylength		= sizeof(VXAFX_MAGNIFY_KEY);
	tInit.paramlength	= 0;
	tInit.setdefault	= SetMagnifyDefault;
	tInit.serialize		= SerializeMagnify;
	tInit.getformat		= GetMagnifyFormat;
	//	tInit.editor		= CreateMagnifyEditor;
	tInit.bitmap		= GetMagnifyBitmap;
	m_dataA.Add( tInit );

	char	EffectDir[MAX_VXPATH];
	pThis->GetEffectPath( EffectDir );
#ifdef _WIN32
	char olddir[MAX_VXPATH];
	GetCurrentDirectory(MAX_VXPATH,olddir);
	SetCurrentDirectory(EffectDir);
#endif

	__uint64 caps = vxGetSystemSetup()->Encryption();
	if(caps&vxcaps_effects)
	{
		memset( &tInit,0,sizeof(tInit) );
		tInit.guid			= MAKEVTRANSITIONID(0x000);
		tInit.name			= vxLoadMessageLV("淡入淡出");
		tInit.folder		= vxLoadMessageLV("划像特技");
		tInit.keylength		= sizeof(float);
		tInit.paramlength	= 0;	
		tInit.setdefault	= SetFadeDefault;
		tInit.getformat		= GetFadeFormat;
		tInit.onconstruct   = FaderOnConstruct;
		tInit.bitmap		= GetFaderBitmap;
		tInit.effecttype	= GPUTSPARAM;
		tInit.effect		= ExecuteFader;
		m_dataVT.Add( tInit );

		memset( &tInit,0,sizeof(tInit) );
		tInit.guid			= MAKEVTRANSITIONID(0x001);
		tInit.name			= vxLoadMessageLV("标准划像");
		tInit.folder		= vxLoadMessageLV("划像特技");
		tInit.keylength		= sizeof(float);
		tInit.paramlength	= sizeof(VXTFX_STANDARDWIPE);
		tInit.openefx		= OpenWipe;
		tInit.closeefx		= CloseWipe;
		tInit.setdefault	= SetStandardWipeDefault;
		tInit.destory		= DestroyStandardWipe;
		tInit.getformat		= GetStandardWipeFormat;
		tInit.onconstruct   = StandardWipeOnConstruct;
		tInit.bitmap		= GetStandardWipeBitmap;
		tInit.selecttype	= StandardWipeSelecttype;
		tInit.copyparam		= StandardCopyParam;
		tInit.runop			= RunOpStandardWipe;
		tInit.effecttype	= GPUTSPARAM;
		tInit.effect		= ExecuteStandardWipe;
		m_dataVT.Add( tInit );

		memset( &tInit,0,sizeof(tInit) );
		tInit.guid			= MAKEVTRANSITIONID(0x002);
		tInit.name			= vxLoadMessageLV("矩阵划像");
		tInit.folder		= vxLoadMessageLV("划像特技");
		tInit.keylength		= sizeof(float);
		tInit.paramlength	= sizeof(VXTFX_MATRIXWIPE);	
		tInit.openefx		= OpenWipe;
		tInit.closeefx		= CloseWipe;
		tInit.setdefault	= SetMatrixWipeDefault;
		tInit.destory		= DestroyMatrixWipe;
		tInit.getformat		= GetMatrixWipeFormat;
		tInit.onconstruct   = MatrixWipeOnConstruct;
		tInit.bitmap		= GetMatrixWipeBitmap;
		tInit.selecttype	= MatrixWipeSelecttype;
		tInit.copyparam		= MatrixCopyParam;
		tInit.runop			= RunOpMatrixWipe;
		tInit.effecttype	= GPUTSPARAM;
		tInit.effect		= ExecuteMatrixWipe;
		m_dataVT.Add( tInit );

/*      模板划像界面暂没有做路径选择，先去掉
		memset( &tInit,0,sizeof(tInit) );
		tInit.guid			= MAKEVTRANSITIONID(0x002);
		tInit.name			= vxLoadMessageLV("模板划像");
		tInit.folder		= vxLoadMessageLV("划像特技");
		tInit.keylength		= sizeof(float);
		tInit.paramlength	= sizeof(VXTFX_IMAGEWIPE);	
		tInit.openefx		= OpenWipe;
		tInit.closeefx		= CloseWipe;
		tInit.setdefault	= SetImageWipeDefault;
		tInit.destory		= DestroyImageWipe;
		tInit.getformat		= GetImageWipeFormat;
		tInit.onconstruct   = ImageWipeOnConstruct;
		tInit.bitmap		= GetImageWipeBitmap;
		tInit.copyparam		= ImageCopyParam;
		tInit.runop			= RunOpImageWipe;
		tInit.effecttype	= GPUTSPARAM;
		tInit.effect		= ExecuteImageWipe;
		m_dataVT.Add( tInit );
		*/

	/*
		memset( &tInit,0,sizeof(tInit) );
		tInit.guid			= MAKEDAIEFFECTID( 3 );
		tInit.name			= "通道管理";
		tInit.folder		= "音频调整";
		tInit.flag			= EFXFLAG_INHERE;
		tInit.keylength		= sizeof(VXAFX_CHNMANAGE_KEY);
		tInit.paramlength	= sizeof(VXAFX_CHNMANAGE_PARAM);
		tInit.setdefault	= SetChnManageDefault;
		tInit.serialize		= SerializeChnManage;
		tInit.getformat		= GetChnManageFormat;
		tInit.onconstruct	= OnConstructChnManage;
	//	tInit.editor		= CreateInterCrossEditor;
		tInit.bitmap		= GetChnManageBitmap;
		m_dataA.Add( tInit );
	*/
		
		struct
		{
			const char* filter;
			const char* infofix;
			void*	array;
		}LoadEffectInfo[] = {	
								{"*.fvt","transition",		&m_dataVT },
								{"*.fat","transition",		&m_dataAT },
								{"*.fxv","video effect",	&m_dataV },
								{"*.fxa","audio effect",	&m_dataA },
	//							{"*.fxc","cg    effect",	&m_dataC },
	//							{"*.fxb","back ground",		&m_dataK } 
							};
		for( int i = 0; i < sizeof(LoadEffectInfo)/sizeof(LoadEffectInfo[0]); i++ )
		{
			CVxComPtr<IVxFileEnum> tool;
			vxCreateFileEnum(&tool);
			if( tool->Start( EffectDir,LoadEffectInfo[i].filter ) )
			{
				do{
					if( tool->MatchAttr() )
					{
						char* fullpath = tool->GetPath();
						pThis->OnLoadEfx(0,fullpath,(EFXARRAY *)LoadEffectInfo[i].array );
					}
				}while( tool->Next() );
			}
		}

	}
	
	{//加载cg特效
		struct
		{
			const char* filter;
			const char* infofix;
			void*	array;
		}LoadEffectInfo[] = {	
			{"*.fxc","cg    effect",	&m_dataC },
		};
		for( int i = 0; i < sizeof(LoadEffectInfo)/sizeof(LoadEffectInfo[0]); i++ )
		{
			CVxComPtr<IVxFileEnum> tool;
			vxCreateFileEnum(&tool);
			if( tool->Start( EffectDir,LoadEffectInfo[i].filter ) )
			{
				do{
					if( tool->MatchAttr() )
					{
						char* fullpath = tool->GetPath();
						pThis->OnLoadEfx(0,fullpath,(EFXARRAY *)LoadEffectInfo[i].array );
					}
				}while( tool->Next() );
			}
		}
	}
#ifdef _WIN32
	SetCurrentDirectory(olddir);
#endif

	m_dataVid.SetSize(m_dataVT.GetSize()+m_dataV.GetSize()+m_dataC.GetSize());
	EFXINITEX* dstefx = m_dataVid.GetData();
	memcpy(dstefx										,m_dataV.GetData(),sizeof(EFXINITEX)*m_dataV.GetSize());
	memcpy(dstefx+m_dataV.GetSize()						,m_dataVT.GetData(),sizeof(EFXINITEX)*m_dataVT.GetSize());
	memcpy(dstefx+m_dataV.GetSize()+m_dataVT.GetSize()	,m_dataC.GetData(),sizeof(EFXINITEX)*m_dataC.GetSize());
	CallStandupFunc();

	int efxs = m_dataVid.GetSize();
	for(int i=0;i<efxs;i++)
	{
		const EFXINITEX& efxinit = m_dataVid.GetAt(i);
		if(efxinit.loader&&IsShaderGraphic()) 
			efxinit.loader->InitGL(g_shaderManager);
	}

	return 0;
}

void CVxNLEEngine::XInitEffect::Exit()
{
	int efxs = m_dataVid.GetSize();
	for(int i=0;i<efxs;i++)
	{
		const EFXINITEX& efxinit = m_dataVid.GetAt(i);
		if(efxinit.loader) efxinit.loader->ExitGL(g_shaderManager);
	}

	m_dataVid.RemoveAll();

	ReleaseEfxArray( m_dataVT );
	ReleaseEfxArray( m_dataAT );
	ReleaseEfxArray( m_dataV );
	ReleaseEfxArray( m_dataA );
	ReleaseEfxArray( m_dataC );
	ReleaseEfxArray( m_dataK );
	
	if(m_heditor)
		FreeLibrary(m_heditor);
}


VXRESULT CVxNLEEngine::CreateClipInfoTools2(IVxClipInfoTools2** infotools2)
{
	CVxComPtr<IVxClipInfoTools> infotools;
	VXRESULT hr = m_setup->CreateClipInfoTools(m_vpool,m_apool,&infotools);
	if(hr!=0) return hr;
	return infotools->QueryInterface(LIID_IVxClipInfoTools2,(void**)infotools2);
}

VXRESULT _vxCreateFineTuning(IVxSystemSetup* setup,IVxVidMemPool* pool,IVxFineTuning** finetune);

VXRESULT CVxNLEEngine::CreateFineTuning(IVxFineTuning** finetune)
{
	return _vxCreateFineTuning(m_setup,m_vpool,finetune);
}


void CVxNLEEngine::__player2destructor(IVxAVPlayer2* player2)
{
	m_avplayer2 = NULL;

	if(g_hcgeditor)
	{
		DESTROYULTRACGEDIT destroyeditor = (DESTROYULTRACGEDIT)GetProcAddress(g_hcgeditor,"vxDestroyUltraCGEdit");
		destroyeditor();
	}
	if(m_pages)
	{
		vxRegisterDefaultManager(NULL);
		m_pages->Release();
		m_pages = NULL;
	}
}

void CVxNLEEngine::__player2dcchgGpu(IVxAVPlayer2* player2, IVxDataCore* dc)
{
#ifdef _WIN32
	HGLRC lastrc = wglGetCurrentContext();
	HDC lastdc = wglGetCurrentDC();
	if(lastrc!=m_hglrc)
	{
		while(!wglMakeCurrent(m_hdc,m_hglrc)) 
			vxSleep(2);
	}
	m_vpool->Reset(m_hglrc);
#elif __APPLE__
	CGLContextObj lastcgl = CGLGetCurrentContext();
	if(lastcgl!=m_cgl)
	{
		CGLLockContext(m_cgl);
		CGLSetCurrentContext(m_cgl);
	}
	m_vpool->Reset(m_cgl);
#else

#endif	


	IVxCGFaceUP* faceup = IVxCGFaceUP::Get();
	if(faceup) faceup->SetHelDC(player2,dc);

#ifdef _WIN32
	if(lastrc!=m_hglrc)
	{
		while(!wglMakeCurrent(lastdc,lastrc)) vxSleep(2);
	}
#elif __APPLE__
	if(lastcgl!=m_cgl)
	{
		CGLSetCurrentContext(lastcgl);
		CGLUnlockContext(m_cgl);
	}
#endif
}

void CVxNLEEngine::__player2dcchgCpu(IVxAVPlayer2* player2, IVxDataCore* dc)
{
	if (NULL != m_vpool)
	{
		m_vpool->Reset(NULL);
	}
}

VXBOOL CVxNLEEngine::__staticframe(IVxCompoundFile* filename,int strmid,IVxSurface** color,IVxCompoundFile* keyfilename,int keystrmid,IVxSurface** key,int frame,int bFrameProgress)
{
	if(!filename) return FALSE;
	VXFILETYPE	type = vxGetFileTypeHandle(filename);
	if( vxIsImage(type))
	{
		VXFILEPATH& filepath = filename->GetAt(0);
		VXNLEVIDINFO vinfo;
		vxNLEVideoInfo(vinfo);

		VXIMAGE img;
		if(!VxImage_Load32(filepath.mainfile,&img,FALSE)) return FALSE;

		VXIMAGE timg = img;

		BYTE* tmpImg = NULL;
		int tmpImgw = timg.Width;
		int tmpImgH = timg.Height;
		VXBOOL flag = FALSE;
		if (tmpImgw < vinfo.vidwidth)
		{
			if(1 == vinfo.vidwidth - tmpImgw)
				tmpImgw += 1;
			else
				tmpImgw += 2;
			flag = TRUE;
		}
		if (tmpImgH < vinfo.vidheight)
		{
			if(1 == vinfo.vidheight - tmpImgH)
				tmpImgH += 1;
			else
				tmpImgH += 2;
			flag = TRUE;
		}
		if(flag)
		{
			tmpImg = (BYTE*)_vxmallocz(tmpImgw*tmpImgH*4);
			BYTE* sImg = (BYTE*)timg.data;
			DWORD* dst = NULL;
			if (vinfo.vidwidth > timg.Width)
			{
				int hdif = vinfo.vidheight - timg.Height;
				for(int k = 1; k <= timg.Height; k++)
				{
					if(hdif > 0)
						dst = (DWORD*)(tmpImg+k*tmpImgw*4);
					else
						dst = (DWORD*)(tmpImg+(k-1)*tmpImgw*4);
					*dst = *((DWORD*)(sImg+(k-1)*timg.Width*4));
					*dst &= 0x00FFFFFF;

					if(vinfo.vidwidth - timg.Width > 1)
					{
						if (hdif<=0)
							dst = (DWORD*)(tmpImg+k*tmpImgw*4 - 4);
						else
							dst = (DWORD*)(tmpImg+(k+1)*tmpImgw*4 - 4);
						*dst = *((DWORD*)(sImg+k*timg.Width*4 - 4));
						*dst &= 0x00FFFFFF;
					}
				}
			}
			if(vinfo.vidheight > timg.Height)
			{
				for(int j = 0; j < tmpImgH - timg.Height; j++)
				{
					int wdif = vinfo.vidwidth - timg.Width;
					if (wdif>0)
						dst = (j==0)?(DWORD*)(tmpImg+4):(DWORD*)(tmpImg+(tmpImgH-1)*tmpImgw*4+4);
					else
						dst = (j==0)?(DWORD*)(tmpImg):(DWORD*)(tmpImg+(tmpImgH-1)*tmpImgw*4);
					sImg = (BYTE*)timg.data;
					if(j==1)sImg = sImg+(timg.Height-1)*timg.Width*4;
					for(int i = 0; i < timg.Width; i++)
					{
						*dst = *((DWORD*)sImg);
						*dst &= 0x00FFFFFF;
						dst++;
						sImg += 4;
					}
				}
			}
			BYTE* dImg = tmpImg + ((int)(ceil((float)(tmpImgH-timg.Height)/2)))*tmpImgw*4 
				+ ((int)(ceil((float)(tmpImgw-timg.Width)/2)))*4;
			cpypic(timg.data,dImg,timg.Height,timg.Width*4,timg.Pitch,tmpImgw*4);
			sImg = (BYTE*)timg.data;
			timg.data = tmpImg;
			timg.Width = tmpImgw;
			timg.Height = tmpImgH;
			timg.Pitch = tmpImgw*4;
		}


		int tpitch = (vinfo.vidwidth*4+0x7F)&~0x7F;
		BYTE* tmpbuf = (BYTE*)_vxmallocz(tpitch*vinfo.vidheight);

		int scaleprec = 50;
		GetEngineConfig()->GetConfigInt(scaleprec, "EngineConfig/SystemSetting/ImageOnVTrackScaleThreshold");
		VXBOOL scaleimage = (timg.Width > (vinfo.vidwidth*(100 + scaleprec) / 100)) || (timg.Height > (vinfo.vidheight*(100 + scaleprec) / 100));

		if (scaleimage)
		{
			BYTE* sbuf = (BYTE*)timg.data;
			BYTE* dbuf = (BYTE*)tmpbuf;

			int sx = 0, sy = 0;;
			int sw = timg.Width, sh = timg.Height;
			int dx = 0, dy = 0;
			int dw = vinfo.vidwidth, dh = vinfo.vidheight;
			if ((timg.Width > vinfo.vidwidth) && (timg.Height > vinfo.vidheight))
			{
				int dw1 = timg.Width*vinfo.vidheight / timg.Height;
				int dh1 = timg.Height*vinfo.vidwidth / timg.Width;
				if (dw1 > vinfo.vidwidth)
				{
					sw = (vinfo.vidwidth*timg.Width / dw1)&~0x3;
					sx = ((timg.Width - sw) / 2)&~0x3;
				}
				else
				{
					sh = vinfo.vidheight*timg.Height / dh1;
					sy = (timg.Height - sh) / 2;
				}
			}
			else if (timg.Width>vinfo.vidwidth)
			{
				dh = timg.Height*vinfo.vidwidth / timg.Width;
				dy = (vinfo.vidheight - dh) / 2;
			}
			else
			{
				dw = (timg.Width*vinfo.vidheight / timg.Height)&~0x3;
				dx = ((vinfo.vidwidth - dw) / 2)&~0x3;
			}
			sbuf += sy*timg.Pitch + sx * 4;
			dbuf += dy*tpitch + dx * 4;
			VxImage_Resize32(sbuf, timg.Pitch, sw, sh, dbuf, tpitch, dw, dh);
		}
		else
		{
			BYTE* sbuf = (BYTE*)timg.data;
			BYTE* dbuf = (BYTE*)tmpbuf;
			int cpyw = 0; int cpyh = 0;
			if (timg.Width > vinfo.vidwidth)
			{
				sbuf += (timg.Width - vinfo.vidwidth) / 2 * 4;
				cpyw = vinfo.vidwidth;
			}
			else
			{
				dbuf += (vinfo.vidwidth - timg.Width) / 2 * 4;
				cpyw = timg.Width;
			}
			if (timg.Height > vinfo.vidheight)
			{
				sbuf += (timg.Height - vinfo.vidheight) / 2 * timg.Pitch;
				cpyh = vinfo.vidheight;
			}
			else
			{
				dbuf += (vinfo.vidheight - timg.Height) / 2 * tpitch;
				cpyh = timg.Height;
			}
			cpypic(sbuf, dbuf, cpyh, cpyw * 4, timg.Pitch, tpitch);
		}

		VXSURFACE_RES res = __vxgetres(vinfo.vidwidth,vinfo.vidheight);
		VXSURFACE_FMT fmt = timg.BPPSource == 32 ? m_yuvafmt : g_fmt;
		CVxComPtr<IVxSurface> imgsf;
		if (!m_vpool->GetSurface(fmt, LOC_HOST, res, &imgsf))
		{
			_vxfree(tmpImg);
			_vxfree(tmpbuf);
			VxImage_Free(&img);
			return FALSE;
		}
		
		VXSURFACE_DESC desc;
		imgsf->GetDesc(&desc);

		VXSURFACE_LOCK lock;
		imgsf->Lock(&lock);
		void* dstbuf[4] = { 0 }; int dpitch[4] = { 0 };
		__getplanar(desc.fmt, lock.pLock, lock.pitch, desc.width, desc.height, dstbuf, dpitch);

		if(timg.BPPSource==32)
			bgra_to_yuv(tmpbuf, tpitch, vinfo.vidwidth, vinfo.vidheight, dstbuf, dpitch);
		else
			bgrx_to_yuv(tmpbuf, tpitch, vinfo.vidwidth, vinfo.vidheight, dstbuf, dpitch);
		imgsf->Unlock();

		*color = imgsf;
		imgsf->AddRef();

		desc.validw = vinfo.vidwidth;
		desc.validh = vinfo.vidheight;
		desc.aspect = (vinfo.aspecth << 16) | vinfo.aspectw;
		desc.colorimetry = g_colorimetry;
		imgsf->SetDesc(&desc);

		_vxfree(tmpImg);
		_vxfree(tmpbuf);
		VxImage_Free(&img);		
	}
	else if(vxIsCG(type))
	{
		VXFILEPATH& filepath = filename->GetAt(0);
		ULTRAEDITCG ultracg = {0};
		if(vxLoadCGImage(filepath.mainfile,ultracg,'BGRA')==VX_S_OK)
		{
			VXNLEVIDINFO einfo;
			vxNLEVideoInfo(einfo);
			CVxSize maxSz(einfo.vidwidth,einfo.vidheight);
			int w = ultracg.imgrect.right-ultracg.imgrect.left;
			int h = ultracg.imgrect.bottom-ultracg.imgrect.top;
			if((w!=maxSz.cx)||(h!=maxSz.cy))
			{
				DWORD* tmp = new DWORD[maxSz.cx*maxSz.cy];
				memset(tmp,0,maxSz.cx*maxSz.cy*4);
				if(ultracg.imgdata)
				{
					int top = einfo.vidheight/2+ultracg.imgrect.top;
					int bottom = einfo.vidheight/2+ultracg.imgrect.bottom;
					int left = einfo.vidwidth/2+ultracg.imgrect.left;
					int right = einfo.vidwidth/2+ultracg.imgrect.right;
					if((top<maxSz.cy)&&(left<maxSz.cx))
					{
						DWORD* src = ultracg.imgdata;
						if(left<0) {src -= left; left = 0;}
						if(top<0) { src -= top*w; top = 0;}
						DWORD* dst = tmp+top*maxSz.cx+left;
						int cpyw = vxmin(right,maxSz.cx)-left;
						int cpyh = vxmin(bottom,maxSz.cy)-top;
						for(int i=0;i<cpyh;i++)
						{
							memcpy(dst,src,cpyw*4);
							dst += maxSz.cx;
							src += w;
						}
					}
					delete ultracg.imgdata;
				}
				ultracg.imgdata = tmp;
				ultracg.imgrect = CVxRect(0,0,maxSz.cx,maxSz.cy);
				w = maxSz.cx; h = maxSz.cy;
			}

			CVxComPtr<IVxSurface> imgsf;
			VXSURFACE_RES res = __vxgetres(w,h);
			m_vpool->GetSurface(m_yuvafmt,LOC_HOST,res,&imgsf);

			VXSURFACE_DESC desc;
			imgsf->GetDesc(&desc);
			VXSURFACE_LOCK lock;
			imgsf->Lock(&lock);
			void* dstbuf[4] = { 0 }; int dpitch[4] = { 0 };
			__getplanar(m_yuvafmt, lock.pLock, lock.pitch, desc.width, desc.height, dstbuf, dpitch);
			bgra_to_yuv(ultracg.imgdata, w * 4, w, h, dstbuf, dpitch);
			imgsf->Unlock();

			desc.validw = w;
			desc.validh = h;
			desc.aspect = (einfo.aspecth<<16)|einfo.aspectw;
			desc.scantype = SCANTYPE_PROGRESSIVE;
			desc.colorimetry = g_colorimetry;
			imgsf->SetDesc(&desc);
			*color = imgsf;
			imgsf->AddRef();
			delete ultracg.imgdata;
		}
	}
	else
	{
		CVxComPtr<IVxDemultiplexer> demux;
		if(m_setup->CreateDemultiplexer2(filename,&demux)!=0) return FALSE;
		CVxComPtr<IVxReadStream> stream;
		if(demux->GetStream(vxstreamVIDEO,strmid,&stream)!=0) return FALSE;
		CVxComPtr<IVxVideoStreamDecoder> dec;
		if(m_setup->CreateStreamDecoderV(m_vpool,stream,decflag_singlethread,frame,SCANTYPE_PROGRESSIVE,&dec)!=0) return FALSE;

		if(!dec->GetFrame(frame,color,1)) return FALSE;
		if(!*color) dec->GetFrameResult(frame,color);
	}

	if(keyfilename&&(keyfilename!=filename))
	{
		if(*key)
		{
			(*key)->Release();
			(*key) = NULL;
		}

		VXFILETYPE	type = vxGetFileTypeHandle( keyfilename);
		if( vxIsImage(type))
		{
			VXFILEPATH& filepath = filename->GetAt(0);
			VXIMAGE img;
			if(!VxImage_Load8(filepath.mainfile,&img,FALSE)) return FALSE;

			int validw = img.Width,validh = img.Height;
			LONG pitch = img.Pitch;
			while(validw>2048||validh>2048)
			{
				validw /= 2;
				validh /= 2;
				pitch *= 2;
			}
			validw = (validw&~0x3);
			validh = img.Height*validw/img.Width;
			if(pitch==img.Pitch)
			{
				img.Width = validw;
				img.Height = validh;
			}
			if(!validw||!validh)
			{
				VxImage_Free(&img);	
				return FALSE;
			}
			
			CVxComPtr<IVxSurface> keysf;
			VXSURFACE_RES res = __vxgetres(validw,validh);
			m_vpool->GetSurface(FMT_A8,LOC_HOST,res,&keysf);
			if(!keysf)
			{
				VxImage_Free(&img);
				return FALSE;
			}
			if(validw!=img.Width||validh!=img.Height)
			{
				int npitch = (validw+0x3F)&~0x3F;
				VxImage_Resize8(img.data,pitch,img.Width,validh,img.data,npitch,validw+2,validh);
				pitch = npitch;
			}

			VXSURFACE_LOCK lock;
			keysf->Lock(&lock);
			cpypic(img.data,lock.pLock,validh,validw,pitch,lock.pitch);
			keysf->Unlock();
			VXSURFACE_DESC desc;
			keysf->GetDesc(&desc);
			desc.validw = validw;
			desc.validh = validh;
			int gcd = vx_gcd(img.Width,img.Height);
			desc.aspect = ((img.Height/gcd)<<16)|(img.Width/gcd);
			keysf->SetDesc(&desc);
			VxImage_Free(&img);
			
			*key = keysf;
			keysf->AddRef();
		}
		else
		{
			CVxComPtr<IVxDemultiplexer> demux;
			if(m_setup->CreateDemultiplexer2(keyfilename,&demux)!=0) return FALSE;
			CVxComPtr<IVxReadStream> stream;
			if(demux->GetStream(vxstreamVIDEO,keystrmid,&stream)!=0) return FALSE;
			CVxComPtr<IVxVideoStreamDecoder> dec;
			if(m_setup->CreateStreamDecoderV(m_vpool,stream,decflag_singlethread,frame,SCANTYPE_PROGRESSIVE,&dec)!=0) return FALSE;

			if(!dec->GetFrame(frame,key,1)) return FALSE;
			if(!*key) dec->GetFrameResult(frame,key);
		}
	}

	return TRUE;
}

/*
VXBOOL CVxNLEEngine::ResetSystemClock(sysclk_cinfo* cinfo)
{
	g_vt = cinfo->std;
	g_res = cinfo->res;
	g_rate = g_vtinfo[g_vt].rate;
	g_scale = g_vtinfo[g_vt].scale;

	m_clock->Reset(cinfo);

	return TRUE;
}
*/

#ifdef _WIN32
HGLRC CVxNLEEngine::__CreateShareGL(SHAREGL gltype)
{
	if(gltype==sharegl_noname)
	{
		HGLRC rc = NULL;
		if(m_sharegl.Pop(&rc))
			return rc;
		rc = wglCreateContext(m_hdc);
		while (!wglShareLists(m_hglrc, rc))
		{
			DWORD err = GetLastError();
			vxSleep(2);
		}
		return rc;
	}
	else
	{
		if(!m_sharegls[gltype])
		{
			m_sharegls[gltype] = wglCreateContext(m_hdc);
			while (!wglShareLists(m_hglrc, m_sharegls[gltype]))
			{
				DWORD err = GetLastError();
				vxSleep(2);
			}
		}
		return m_sharegls[gltype];
	}
}

void CVxNLEEngine::__DestroyShareGL(SHAREGL gltype,HGLRC rc)
{
	if(gltype==sharegl_noname)
		m_sharegl.Push(rc);
}

extern "C"
HGLRC _vxCreateShareGL(SHAREGL gltype)
{
	ASSERT(g_nleengine);
	return g_nleengine->__CreateShareGL(gltype);
}

extern "C"
void _vxDestroyShareGL(SHAREGL gltype,HGLRC rc)
{
	ASSERT(g_nleengine);
	g_nleengine->__DestroyShareGL(gltype,rc);
}

#elif __APPLE__

CGLContextObj CVxNLEEngine::__CreateShareGL(SHAREGL gltype)
{
	CVxLockCGL lock(m_cgl);
	
	static GLint attribs[] = {
		kCGLPFAAccelerated,
		kCGLPFAColorSize, 24,
		kCGLPFAAlphaSize, 8,
		kCGLPFADepthSize, 24,
		kCGLPFASampleBuffers,1,
		kCGLPFASamples,m_multisamples,
		0 };
	if(m_multisamples==0)
	{
		attribs[9] = attribs[10] = attribs[11] = 0;
	}
	if(gltype==sharegl_noname)
	{
		CGLContextObj rc;
		if(m_sharegl.Pop(&rc))
			return rc;

		CGLPixelFormatObj cglPixelFormat = NULL;
		GLint numPixelFormats = 0;
		CGLChoosePixelFormat ((CGLPixelFormatAttribute*)attribs, &cglPixelFormat, &numPixelFormats);
		VERIFY(CGLCreateContext(cglPixelFormat, m_cgl,&rc)==0);
		CGLDestroyPixelFormat(cglPixelFormat);
		return rc;
	}
	else
	{
		if(!m_sharegls[gltype])
		{
			CGLPixelFormatObj cglPixelFormat = NULL;
			GLint numPixelFormats = 0;
			CGLChoosePixelFormat ((CGLPixelFormatAttribute*)attribs, &cglPixelFormat, &numPixelFormats);
			VERIFY(CGLCreateContext(cglPixelFormat, m_cgl,&m_sharegls[gltype])==0);
			CGLDestroyPixelFormat(cglPixelFormat);
		}
		return m_sharegls[gltype];
	}
}

void CVxNLEEngine::__DestroyShareGL(SHAREGL gltype,CGLContextObj rc)
{
	if(gltype==sharegl_noname)
		m_sharegl.Push(rc);
}

extern "C"
CGLContextObj _vxCreateShareGL(SHAREGL gltype)
{
	ASSERT(g_nleengine);
	return g_nleengine->__CreateShareGL(gltype);
}

extern "C"
void _vxDestroyShareGL(SHAREGL gltype,CGLContextObj rc)
{
	ASSERT(g_nleengine);
	g_nleengine->__DestroyShareGL(gltype,rc);
}
#else
GLXContext CVxNLEEngine::__CreateShareGL(SHAREGL gltype)
{
    if(gltype==sharegl_noname)
    {
        GLXContext rc = NULL;
        if(m_sharegl.Pop(&rc))
            return rc;
        return glXCreateNewContext(m_xdp,*m_fbc,GLX_RGBA_TYPE,m_glx,true);
    }
    else
    {
        if(!m_sharegls[gltype])
        {
            m_sharegls[gltype] = glXCreateNewContext(m_xdp,*m_fbc,GLX_RGBA_TYPE,m_glx,true);
        }
        return m_sharegls[gltype];
    }
}

void CVxNLEEngine::__DestroyShareGL(SHAREGL gltype,GLXContext glx)
{
    if(gltype==sharegl_noname)
        m_sharegl.Push(glx);
}

extern "C"
GLXContext _vxCreateShareGL(SHAREGL gltype)
{
    ASSERT(g_nleengine);
    return g_nleengine->__CreateShareGL(gltype);
}

extern "C"
void _vxDestroyShareGL(SHAREGL gltype,GLXContext rc)
{
    ASSERT(g_nleengine);
    g_nleengine->__DestroyShareGL(gltype,rc);
}
#endif

VXBOOL	GetStandardWipePattern(VXTFX_STANDARDWIPE* ,WORD* );
VXBOOL	GetMatrixWipePattern( VXTFX_MATRIXWIPE*  ,WORD*);

VXERR RunOpStandardWipe(EFXRUNPARAM* runparam )
{
	VXTFX_STANDARDWIPE* spri = (VXTFX_STANDARDWIPE*)runparam->param;
	VXTFX_STANDARDWIPE dpri = *spri;
	static int i2value[] = {1,2,4,8,16,32,64,128};
	dpri.nRepeatHor = i2value[spri->nRepeatHor];
	dpri.nRepeatVer = i2value[spri->nRepeatVer];

	AppLockHelper Helper( CRIT_PLAYRESOURCE );
	if( runparam->op == RO_UPLOAD )
	{
		IVxGPUImage*& maskkey = *(IVxGPUImage**)runparam->efxp; 
		if(!maskkey)
		{
			if( IVxCGFaceUP::Get() )
			{
				WORD* w = VXNew( WORD,256*256 );
				GetStandardWipePattern( &dpri,w );
				BYTE* byte = VXNew(BYTE, 256 * 256);
				for (int h = 0; h < 256; h++)
				{
					for (int iw = 0; iw < 256; iw++)
						*(byte + (255 - h) * 256 + iw) = (BYTE)((*(w + h * 256 + iw)) >> 8);
				}
#if defined(_WIN32)&&defined(_DEBUG)
				WriteToTGA("C:\\standardwipe.tga", byte, 256, 256, 8);
#endif
				IVxCGFaceUP::Get()->Create(byte, CVxSize(256, 256), 256, IFUTYPE_HW | IFUTYPE_ALPHA | IFUTYPE_ACCURATE, &maskkey);
				VXFree(byte);
				VXFree( w );
			}
		}
	}
	else if( runparam->op == RO_RECYCLE ) 
	{
		AppLockHelper Helper( CRIT_DOWNLOAD );
		void*& maskkey = *(void**)runparam->efxp;
		IVxGPUImage* oldImage = (IVxGPUImage*)InterlockedExchangePointer( (void**)&maskkey,NULL );
		Helper.Unlock();
		if( oldImage )oldImage->Release();
	}

	return VX_SUCCESS;
}

void DestroyStandardWipe(void* p,void* param,void* key)
{
	if( param )
	{
		EFXRUNPARAM runparam = {  sizeof(runparam),p,RO_RECYCLE,0,param,0 };
		RunOpStandardWipe( &runparam );
	}
}

VXERR RunOpMatrixWipe(EFXRUNPARAM* runparam )
{
	VXTFX_MATRIXWIPE* spri = (VXTFX_MATRIXWIPE*)runparam->param;
	VXTFX_MATRIXWIPE dpri = *spri;
	//static int i2value[] = {2,4,8,16,32,64,128};
	static int i2value[] = {4,8,16,32,64,128};
	dpri.nTileWidth = i2value[spri->nTileWidth];
	dpri.nTileHeight = i2value[spri->nTileHeight];
	IVxGPUImage*& maskkey = *(IVxGPUImage**)runparam->efxp; 

	AppLockHelper Helper( CRIT_PLAYRESOURCE );
	if( runparam->op == RO_UPLOAD )
	{
		if(!maskkey)
		{
			if( IVxCGFaceUP::Get() )
			{
				WORD* w = VXNew( WORD,256*256 );
				GetMatrixWipePattern( &dpri,w );
				BYTE* byte = VXNew(BYTE, 256 * 256);
				for (int h = 0; h < 256; h++)
				{
					for (int iw = 0; iw < 256; iw++)
						*(byte + (255 - h) * 256 + iw) = (BYTE)((*(w + h * 256 + iw)) >> 8);
				}
#if defined(_WIN32)&&defined(_DEBUG)
				WriteToTGA("C:\\matrixwipe.tga", byte, 256, 256, 8);
#endif
				IVxCGFaceUP::Get()->Create(byte, CVxSize(256, 256), 256, IFUTYPE_HW | IFUTYPE_ALPHA | IFUTYPE_ACCURATE, &maskkey);
				VXFree(byte);
				VXFree(w);
			}
		}
	}
	else if( runparam->op == RO_RECYCLE ) 
	{
		AppLockHelper Helper( CRIT_DOWNLOAD );
		IVxGPUImage* oldImage = (IVxGPUImage*)InterlockedExchangePointer( (void**)&maskkey,NULL );
		Helper.Unlock();
		if( oldImage ) oldImage->Release();
	}

	return VX_SUCCESS;
}

void DestroyMatrixWipe(void* p,void* param,void* key)
{
	if( param )
	{
		EFXRUNPARAM runparam = {  sizeof(runparam),p,RO_RECYCLE,0,param,0 };
		RunOpMatrixWipe( &runparam );
	}
}

#include "SWFunction.h"
#include "MWFunction.h"
//=============================================================================
// Function: GetWipePattern()
// Return value: TRUE if successful; otherwise FALSE.
VXBOOL GetStandardWipePattern(VXTFX_STANDARDWIPE* param,WORD *pWipe)
{
	if( pWipe == NULL )
		return FALSE;
	DWORD pattern = param->dwPattern;
	if( pattern < 103 )
	{
		float x, y;
		x = param->fPositionHor;
		y = param->fPositionVer;
		SW_Function[pattern+1]( x, y, pWipe );
	}
	else return FALSE;

	return TRUE;
}

VXBOOL GetMatrixWipePattern(VXTFX_MATRIXWIPE* param,WORD *pWipe)
{
	if( pWipe == NULL )	return FALSE;
	DWORD pattern = param->dwPattern;
	if( pattern<=78)
	{
		WORD x, y;
//		static int repeats[] = {2,4,8,16,32,64,128,256};
		x = param->nTileWidth/MW_FACTOR;	if( x==0 )	x = 1;
		y = param->nTileHeight/MW_FACTOR;	if( y==0 )	y = 1;
		MW_Function[pattern]( x, y, pWipe );
	}
	else
		return FALSE;
	return TRUE;
}


VXBOOL __cdecl VxImage_LoadFromCompound8( IVxCompoundFile* compound,IVxSystemSetup* setup,VXIMAGE* image,VXBOOL bottomLeft )
{
	if(!setup||!compound) return FALSE;
	if(compound->GetCount()==0) return FALSE;
	VX_AVPATH path = {FALSE};
	VXFILEPATH& cfile = compound->GetAt(0);
	strcpy(path.szPath,cfile.mainfile);
	compound->GetUserPassword(path.szUserName,path.szPassword);
	CVxComPtr<IVxSource> source;
	if(setup->CreateSource(&path,&source)!=0) return FALSE;
	LONG size = (LONG)source->GetSize();
	void* data = _vxmallocz(size+16);
	source->Read((BYTE*)data,size);
	VXBOOL ret = VxImage_LoadFromBuffer24(data,size,image,bottomLeft);
	_vxfree(data);
	return ret;
}

VXERR	RunOpImageWipe(	EFXRUNPARAM* runparam)
{
	VXTFX_IMAGEWIPE* spri = (VXTFX_IMAGEWIPE*)runparam->param;
	VXTFX_IMAGEWIPE dpri = *spri;
	static int i2value[] = {1,2,4,8,16,32,64,128};
	dpri.nRepeatHor = i2value[spri->nRepeatHor];
	dpri.nRepeatVer = i2value[spri->nRepeatVer];

	AppLockHelper Helper( CRIT_PLAYRESOURCE );
	if( runparam->op == RO_UPLOAD )
	{
		IVxGPUImage*& maskkey = *(IVxGPUImage**)runparam->efxp; 
		IVxCGFaceUP* faceup = IVxCGFaceUP::Get();
		if(!maskkey&&faceup)
		{
			CVxComPtr<IVxCompoundFile> compound;
			if(vxGetCompoundFile(&dpri.imgid,&compound)==0)
			{
				VXIMAGE image;
				if(VxImage_LoadFromCompound8(compound,vxGetSystemSetup(),&image,FALSE))
				{
					faceup->Create( image.data,CVxSize(image.Width,image.Height),image.Pitch,IFUTYPE_HW|IFUTYPE_ALPHA|IFUTYPE_ACCURATE,&maskkey );
					VxImage_Free(&image);
				}
			}
			else
			{
				VXTFX_STANDARDWIPE tmp = {0};
				tmp.nRepeatHor = 1;
				tmp.nRepeatVer = 1;		
				tmp.nSoftnessSize = 50;		
				tmp.fAspectRatio = 1.f;	
				WORD* w = VXNew( WORD,256*256 );
				GetStandardWipePattern( &tmp,w );
				for( int i = 0; i < 256*256; i++ )
					((BYTE*)w)[i] = BYTE(w[i]>>8);
				IVxCGFaceUP::Get()->Create( w,CVxSize(256,256),256,IFUTYPE_HW|IFUTYPE_ALPHA|IFUTYPE_ACCURATE,&maskkey );
				VXFree( w );
			}
		}
	}
	else if( runparam->op == RO_RECYCLE ) 
	{
		AppLockHelper Helper( CRIT_DOWNLOAD );
		void*& maskkey = *(void**)runparam->efxp;
		IVxGPUImage* oldImage = (IVxGPUImage*)InterlockedExchangePointer( (void**)&maskkey,NULL );
		Helper.Unlock();
		if( oldImage )oldImage->Release();
	}

	return VX_SUCCESS;
}

void DestroyImageWipe(void* p,void* param,void* key)
{
	if( param )
	{
		EFXRUNPARAM runparam = {  sizeof(runparam),p,RO_RECYCLE,0,param,0 };
		RunOpImageWipe( &runparam );
	}
}



const char* str2utf8(char* str);


VXRESULT vxCreateNLEEngine(IVxSystemSetup* setup,IVxNLEEngine** engine)
{
	if(!g_nleengine)
	{
		vxAppLock(CRIT_ALLOC);
		if(!g_pDatabase)
		{
			vxString strTmp = vxGetTempDirection();
			char vxsfile[MAX_VXPATH] = {0};
			sprintf(vxsfile,"%s/.jetsenmatinfo.db",strTmp.c_str());
			VXBOOL newtable = !vxFileExists(vxsfile);
			if(sqlite3_open(str2utf8(vxsfile),&g_pDatabase)==SQLITE_OK)
			{
				if(newtable)
				{
					char*	sqliteErrorMsg	= 0;
					if(sqlite3_exec(g_pDatabase,"create table matinfo(pathname varchar(4096) not null primary key,filesize int64,icon blob,wave blob,info blob)",NULL,0,&sqliteErrorMsg)!=SQLITE_OK)
					{
					}
					free(sqliteErrorMsg);
				}
			}
		}
		vxAppUnlock(CRIT_ALLOC);

		g_nleengine = new CVxNLEEngine(setup);
		if(!g_nleengine->Initialize())
		{
			g_nleengine->Uninitialize();
			delete g_nleengine;
			return -1;
		}
	}
	return GetVxInterface(static_cast<IVxNLEEngine*>(g_nleengine),(void**)engine);
}



VX_NLEEXT_API LONG		vxSecondToFrame( float second )
{
	return (LONG)(second*g_rate/g_scale+0.5f);
}

VX_NLEEXT_API float		vxFrameToSecond( LONG frames)
{
	return (float)frames*g_scale/g_rate;
}
