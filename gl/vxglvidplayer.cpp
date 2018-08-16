// VxDX9VidPlayer.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "VxGLVidPlayer.h"
#include "inlinetools.h"
#include "vximage.h"
#include "vxcarrier.h"
#include "vxnleresource.h"
#include "vxmath.h"
#include "vxdatacoreif.h"
#include "VxNLEEngineIF.h"
#ifdef _WIN32
#include "vxgl/wglew.h"
#elif __APPLE__
VXBOOL InitGLView(HVXWND wnd,CGLContextObj cgl,void* p);
VXBOOL SetParant(HVXWND wnd,HVXWND parent,VXRECT* rc);
#else
#include "vxgl/glxew.h"
#endif



CVxGLVidPlayer::CVxGLVidPlayer(IVxSystemSetup* setup,IVxSystemClock* clock,int w,int h,VXBOOL alone)
: CVxObject("CVxGLVidPlayer",NULL)
, m_setup(setup)
, m_clock(clock)
, m_alone(alone)
#ifdef _WIN32
, m_hDC(NULL)
, m_hglrc(NULL)
, m_hglrcP(NULL)
#elif __APPLE__
, m_cglrc(NULL)
, m_cglrcP(NULL)
, m_handleref(nil)
#else
, m_glx(NULL)
, m_glxP(NULL)
, m_fbc(NULL)
, m_cmap(NULL)
, m_xwin(NULL)
#endif
, m_resize(FALSE)
, m_hWnd(NULL)
, m_parentwnd(NULL)
, m_vflist(0)
, m_rgbtex(0)
, m_yuv2rgbFBO(0)
, m_fbo(0)
, m_shader_ytor(NULL)
, m_shader_gamma(NULL)
, m_rgbbuf(NULL)
, m_ftFont(NULL)
, m_uidraw(NULL)
, m_stdres(FALSE)
, m_fullres(FALSE)
, m_saftrect(TRUE)
, m_drawinfo(TRUE)
, m_width(w)
, m_height(h)
, m_3dmode(-1)
, __liveconvert(NULL)
, m_masktex(0)
, m_maskData(NULL),m_iLiveMode(-1),m_iDrawMode(1)
, m_deitype(0)
{
	VXTHREAD_MUTEX_INIT(&m_csLive);
	VXTHREAD_MUTEX_INIT(&m_csLiveRnd);

	INITPTHREAD(m_hLiveThread);


	const sysclk_cinfo* cinfo = m_clock->GetCreateInfo();
	m_rate = cinfo->rate;
	m_scale = cinfo->scale;
	m_aspect = cinfo->aspect;
	m_progressive = cinfo->scantype == SCANTYPE_PROGRESSIVE;
	m_stype = cinfo->colorimetry==COLORIMETRY_BT709 ? shader_ytor709 : shader_ytor601;

	yuy2_to_bgra = yuv_to_bgras[FMT_YUY2][cinfo->colorimetry];

	m_res = cinfo->res;
	m_stdres = !(m_res&VXCUSTOMRESMASK);
	memset(&m_deiyuv422p, 0, sizeof(m_deiyuv422p));
}

CVxGLVidPlayer::~CVxGLVidPlayer(void)
{
	VXTHREAD_MUTEX_DESTROY(&m_csLiveRnd);
	VXTHREAD_MUTEX_DESTROY(&m_csLive);
}



LONG CVxGLVidPlayer::NonDelegatingQueryInterface(LONG iid, void** ppObj)
{
	if(iid==LIID_IVxLiveExt)
		return GetVxInterface(static_cast<IVxLiveExt*>(this),ppObj);
	else if(iid==LIID_IVxLiveExt2)
		return GetVxInterface(static_cast<IVxLiveExt2*>(this),ppObj);
	else if (iid == LIID_IVxLiveExt3)
		return GetVxInterface(static_cast<IVxLiveExt3*>(this), ppObj);
	else if(iid==LIID_IVxLiveCliper)
		return GetVxInterface(static_cast<IVxLiveCliper*>(this),ppObj);
    else if((iid==LIID_IVxSystemClock)&&m_clock)
        return GetVxInterface(m_clock,ppObj);
	else
		return CVxObject::NonDelegatingQueryInterface(iid,ppObj);
}

void _convert_1920x540_to_480x540( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	_1920x540_to_480x540_yuy2(source,ipitch,1920,540,target,opitch);
}

void _convert_1440x540_to_720x540( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	const __m128i ma = _mm_set1_epi32( 0x00FF );
	const __m128i mb = _mm_set_epi32(  0,0xFF00FF00,0,0xFF00FF00 );
	while( h-- )
	{
		__m128i* src = (__m128i*)source;
		__m128i* dst = (__m128i*)target;
		int w = 1440/16;
		while( w-- )
		{
			__m128i m0 = *src++;
			__m128i m1 = _mm_srli_si128( m0,2  );
			__m128i m2 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m2 = _mm_and_si128(  m2,ma );
			__m128i m3 = _mm_srli_si128( m0,4  );
			m3 = _mm_avg_epu8(   m3,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m3 = _mm_and_si128(  m3,mb );

			m0 = *src++;
			m1 = _mm_srli_si128( m0,2  );
			__m128i m4 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m4 = _mm_and_si128(  m4,ma );
			__m128i m5 = _mm_srli_si128( m0,4  );
			m5 = _mm_avg_epu8(   m5,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m5 = _mm_and_si128(  m5,mb );

			m2 = _mm_packus_epi16(m2,m4  );	//	Y1xY2xY3xY4xY5xY6xY7xY8x
			m0 = _mm_unpacklo_epi32(m3,m5  );
			m1 = _mm_unpackhi_epi32(m3,m5  );
			m3 = _mm_unpacklo_epi32(m0,m1  );

			m2 = _mm_or_si128( m2,m3 );
			*dst++ = m2;

		}
		source += ipitch;
		target += opitch;
	}	
	_mm_empty();
}

void _convert_1920x540_to_960x540( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	const __m128i ma = _mm_set1_epi32( 0x00FF );
	const __m128i mb = _mm_set_epi32(  0,0xFF00FF00,0,0xFF00FF00 );
	while( h-- )
	{
		__m128i* src = (__m128i*)source;
		__m128i* dst = (__m128i*)target;
		int w = 1920/16;
		while( w-- )
		{
			__m128i m0 = *src++;
			__m128i m1 = _mm_srli_si128( m0,2  );
			__m128i m2 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m2 = _mm_and_si128(  m2,ma );
			__m128i m3 = _mm_srli_si128( m0,4  );
			m3 = _mm_avg_epu8(   m3,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m3 = _mm_and_si128(  m3,mb );

			m0 = *src++;
			m1 = _mm_srli_si128( m0,2  );
			__m128i m4 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m4 = _mm_and_si128(  m4,ma );
			__m128i m5 = _mm_srli_si128( m0,4  );
			m5 = _mm_avg_epu8(   m5,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m5 = _mm_and_si128(  m5,mb );

			m2 = _mm_packus_epi16(m2,m4  );	//	Y1xY2xY3xY4xY5xY6xY7xY8x
			m0 = _mm_unpacklo_epi32(m3,m5  );
			m1 = _mm_unpackhi_epi32(m3,m5  );
			m3 = _mm_unpacklo_epi32(m0,m1  );

			m2 = _mm_or_si128( m2,m3 );
			*dst++ = m2;

		}
		source += ipitch;
		target += opitch;
	}	
	_mm_empty();
}

void _convert_4096x1080_to_2048x1080( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	const __m128i ma = _mm_set1_epi32( 0x00FF );
	const __m128i mb = _mm_set_epi32(  0,0xFF00FF00,0,0xFF00FF00 );
	while( h-- )
	{
		__m128i* src = (__m128i*)source;
		__m128i* dst = (__m128i*)target;
		int w = 4096/16;
		while( w-- )
		{
			__m128i m0 = *src++;
			__m128i m1 = _mm_srli_si128( m0,2  );
			__m128i m2 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m2 = _mm_and_si128(  m2,ma );
			__m128i m3 = _mm_srli_si128( m0,4  );
			m3 = _mm_avg_epu8(   m3,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m3 = _mm_and_si128(  m3,mb );

			m0 = *src++;
			m1 = _mm_srli_si128( m0,2  );
			__m128i m4 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m4 = _mm_and_si128(  m4,ma );
			__m128i m5 = _mm_srli_si128( m0,4  );
			m5 = _mm_avg_epu8(   m5,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m5 = _mm_and_si128(  m5,mb );

			m2 = _mm_packus_epi16(m2,m4  );	//	Y1xY2xY3xY4xY5xY6xY7xY8x
			m0 = _mm_unpacklo_epi32(m3,m5  );
			m1 = _mm_unpackhi_epi32(m3,m5  );
			m3 = _mm_unpacklo_epi32(m0,m1  );

			m2 = _mm_or_si128( m2,m3 );
			*dst++ = m2;

		}
		source += ipitch;
		target += opitch;
	}	
	_mm_empty();
}

void _convert_3840x1080_to_1920x1080( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	const __m128i ma = _mm_set1_epi32( 0x00FF );
	const __m128i mb = _mm_set_epi32(  0,0xFF00FF00,0,0xFF00FF00 );
	while( h-- )
	{
		__m128i* src = (__m128i*)source;
		__m128i* dst = (__m128i*)target;
		int w = 3840/16;
		while( w-- )
		{
			__m128i m0 = *src++;
			__m128i m1 = _mm_srli_si128( m0,2  );
			__m128i m2 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m2 = _mm_and_si128(  m2,ma );
			__m128i m3 = _mm_srli_si128( m0,4  );
			m3 = _mm_avg_epu8(   m3,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m3 = _mm_and_si128(  m3,mb );

			m0 = *src++;
			m1 = _mm_srli_si128( m0,2  );
			__m128i m4 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m4 = _mm_and_si128(  m4,ma );
			__m128i m5 = _mm_srli_si128( m0,4  );
			m5 = _mm_avg_epu8(   m5,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m5 = _mm_and_si128(  m5,mb );

			m2 = _mm_packus_epi16(m2,m4  );	//	Y1xY2xY3xY4xY5xY6xY7xY8x
			m0 = _mm_unpacklo_epi32(m3,m5  );
			m1 = _mm_unpackhi_epi32(m3,m5  );
			m3 = _mm_unpacklo_epi32(m0,m1  );

			m2 = _mm_or_si128( m2,m3 );
			*dst++ = m2;

		}
		source += ipitch;
		target += opitch;
	}	
	_mm_empty();
}

void _convert_1920x540_to_960x540L( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	cpypic(source,target,540,w,ipitch,opitch);
}

void _convert_1920x540_to_960x540R( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch)
{
	cpypic(source+w,target,540,w,ipitch,opitch);
}

void _convert_1280x720_to_640x360( BYTE* source,int w,int _h,int ipitch,BYTE* target,int opitch )
{
	const __m128i ma = _mm_set1_epi32( 0x00FF );
	const __m128i mb = _mm_set_epi32(  0,0xFF00FF00,0,0xFF00FF00 );
	int h = 360;
	while( h-- )
	{
		__m128i* src = (__m128i*)source;
		__m128i* dst = (__m128i*)target;
		int w = 1280/16;
		while( w-- )
		{
			__m128i m0 = *src++;
			__m128i m1 = _mm_srli_si128( m0,2  );
			__m128i m2 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m2 = _mm_and_si128(  m2,ma );
			__m128i m3 = _mm_srli_si128( m0,4  );
			m3 = _mm_avg_epu8(   m3,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m3 = _mm_and_si128(  m3,mb );

			m0 = *src++;
			m1 = _mm_srli_si128( m0,2  );
			__m128i m4 = _mm_avg_epu8(   m0,m1 );	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m4 = _mm_and_si128(  m4,ma );
			__m128i m5 = _mm_srli_si128( m0,4  );
			m5 = _mm_avg_epu8(   m5,m0 );	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m5 = _mm_and_si128(  m5,mb );

			m2 = _mm_packus_epi16(m2,m4  );	//	Y1xY2xY3xY4xY5xY6xY7xY8x
			m0 = _mm_unpacklo_epi32(m3,m5  );
			m1 = _mm_unpackhi_epi32(m3,m5  );
			m3 = _mm_unpacklo_epi32(m0,m1  );

			m2 = _mm_or_si128( m2,m3 );
			*dst++ = m2;

		}
		source += ipitch;
		target += opitch;
	}	
	_mm_empty();
}

void _convert_960x720_to_640x360( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch )
{
}


void _convert_sd576( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch )
{
	cpypic(source,target,288,720*2,ipitch,opitch);
}


void _convert_sd480( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch )
{
	cpypic(source,target,240,720*2,ipitch,opitch);
}

void _convert_cpy( BYTE* source,int w,int h,int ipitch,BYTE* target,int opitch )
{
	cpypic(source,target,h,w*2,ipitch,opitch);
}

void s_half(BYTE* source, int cx, int cy, int ipitch, BYTE* target, int opitch)
{
	const __m128i ma = _mm_set1_epi32(0x00FF);
	const __m128i mb = _mm_set_epi32(0, 0xFF00FF00, 0, 0xFF00FF00);
	int h = cy;
	while (h--)
	{
		__m128i* src = (__m128i*)source;
		__m128i* dst = (__m128i*)target;
		int w = cx / 16;
		while (w--)
		{
			__m128i m0 = *src++;
			__m128i m1 = _mm_srli_si128(m0, 2);
			__m128i m2 = _mm_avg_epu8(m0, m1);	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m2 = _mm_and_si128(m2, ma);
			__m128i m3 = _mm_srli_si128(m0, 4);
			m3 = _mm_avg_epu8(m3, m0);	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m3 = _mm_and_si128(m3, mb);

			m0 = *src++;
			m1 = _mm_srli_si128(m0, 2);
			__m128i m4 = _mm_avg_epu8(m0, m1);	//	Y ok	Y1xxxY2xxxY3xxxY4xxx
			m4 = _mm_and_si128(m4, ma);
			__m128i m5 = _mm_srli_si128(m0, 4);
			m5 = _mm_avg_epu8(m5, m0);	//	UV ok	xU1xV1xxxxxU3xV3xxxx
			m5 = _mm_and_si128(m5, mb);

			m2 = _mm_packus_epi16(m2, m4);	//	Y1xY2xY3xY4xY5xY6xY7xY8x
			m0 = _mm_unpacklo_epi32(m3, m5);
			m1 = _mm_unpackhi_epi32(m3, m5);
			m3 = _mm_unpacklo_epi32(m0, m1);

			m2 = _mm_or_si128(m2, m3);
			*dst++ = m2;

		}
		source += ipitch;
		target += opitch;
	}
}


vxinline int GetAlignedSize( int i_size )
{
	int i_result = 1;
	while( i_result < i_size )
	{
		i_result *= 2;
	}
	return i_result;
}
const char* g_gamma_glsl = "\
uniform sampler2D lastStep;\n\
void main()\n\
{\n\
	vec4 color = texture2D(lastStep,gl_TexCoord[0].xy);\n\
	vec3 rgbp = 1.099*pow(color.xyz, vec3(0.45,0.45,0.45))-0.099;\n\
	rgbp = max(rgbp,0.0);\n\
	color.xyz = min(rgbp,1.0);\n\
	gl_FragColor = color;\n\
}";


//	R' = 1.164384*(Y - 16) + 1.596027*(Cr - 128 )
//	G' = 1.164384*(Y - 16) - 0.812968*(Cr - 128 )- 0.391160*( Cb - 128 )
//	B' = 1.164384*(Y - 16) + 2.017231*(Cb - 128 )
const char* g_ytor601_glsl = "\n\
uniform sampler2D lastStep;		\n\
void main ( void )\n\
{\n\
	int x = int(gl_FragCoord.x);\n\
	int ii = x-x/2*2;	\n\
	vec4 tex = texture2D( lastStep, gl_TexCoord[0].xy );\n\
	float ytoc = (tex[2*ii]-0.062745)*1.164384;\n\
	float u = tex[1] - 0.501961;\n\
	float v = tex[3] - 0.501961;\n\
	float r = ytoc + 1.596027*v;\n\
	float g = ytoc - 0.391160*u - 0.812968*v;\n\
	float b = ytoc + 2.017231*u;\n\
	gl_FragColor = vec4(r, g, b, 1.0);\n\
}";

//	R' = 1.164384*(Y - 16) + 1.792742*(Cr - 128 )
//	G' = 1.164384*(Y - 16) - 0.532909*(Cr - 128 )- 0.213249*( Cb - 128 )
//	B' = 1.164384*(Y - 16) + 2.112402*(Cb - 128 )
const char* g_ytor709_glsl = "\n\
uniform sampler2D lastStep;	\n\
void main ( void )\n\
{\n\
	int x = int(gl_FragCoord.x);\n\
	int ii = x-x/2*2;	\n\
	vec4 tex = texture2D( lastStep, gl_TexCoord[0].xy );\n\
	float ytoc = (tex[2*ii]-0.062745)*1.164384;\n\
	float u = tex[1] - 0.501961;\n\
	float v = tex[3] - 0.501961;\n\
	float r = ytoc + 1.792742*v;\n\
	float g = ytoc - 0.213249*u - 0.532909*v;\n\
	float b = ytoc + 2.112402*u;\n\
	gl_FragColor = vec4(r, g, b, 1.0);\n\
}";


#if __APPLE__
#ifdef __cplusplus
extern "C" {
#endif
	
	typedef int CGSConnectionID;
	typedef int CGSWindowID;
	typedef int CGSSurfaceID;
	
	typedef enum {
		kCGSSharingNone,
		kCGSSharingReadOnly,
		kCGSSharingReadWrite
	} CGSSharingState;
	
CGLError CGLGetSurface(CGLContextObj, CGSConnectionID*, CGSWindowID*, CGSSurfaceID*);
CGLError CGLSetSurface(CGLContextObj, CGSConnectionID, CGSWindowID, CGSSurfaceID);
CGLError CGLUpdateContext(CGLContextObj);


// Undocumented CGS
CGSConnectionID CGSMainConnectionID();
CGError CGSSetWindowSharingState(CGSConnectionID cid, CGSWindowID winId, CGSSharingState state);
CGError CGSGetSurfaceCount(CGSConnectionID, CGWindowID, int* countIds);
CGError CGSGetSurfaceList(CGSConnectionID, CGWindowID, int countIds, CGSSurfaceID* ids, int* filled);
CGError CGSAddSurface(CGSConnectionID, CGWindowID, CGSSurfaceID*);
CGError CGSRemoveSurface(CGSConnectionID, CGWindowID, CGSSurfaceID);
CGError CGSSetSurfaceBounds(CGSConnectionID, CGWindowID, CGSSurfaceID, float xOrg, float yOrg, float width, float height);
CGError CGSGetSurfaceBounds(CGSConnectionID, CGWindowID, CGSSurfaceID, float* bounds);
CGError CGSGetWindowBounds(CGSConnectionID, CGWindowID, float* bounds);
CGError CGSOrderSurface(CGSConnectionID, CGWindowID, CGSSurfaceID, int param1 = 1, int param2 = 0);
CGError CGSFlushSurface(CGSConnectionID, CGWindowID, CGSSurfaceID, int param = 0);


#ifdef __cplusplus
}
#endif
#endif

enum SHAREGL{sharegl_noname=-1,sharegl_efx,sharegl_up,sharegl_down,sharegl_live1,sharegl_live2,sharegl_finetunning,shardgl_maxs};
extern "C"
{
#ifdef _WIN32
HGLRC (*_vxCreateShareGL)(SHAREGL gltype);
void (*_vxDestroyShareGL)(SHAREGL gltype,HGLRC rc);
#elif __APPLE__
CGLContextObj _vxCreateShareGL(SHAREGL gltype);
void _vxDestroyShareGL(SHAREGL gltype,CGLContextObj rc);
#else
GLXContext _vxCreateShareGL(SHAREGL gltype);
void _vxDestroyShareGL(SHAREGL gltype,GLXContext rc);
#endif;
};

#if __linux__
VXBOOL CVxGLVidPlayer::__recreateglxwindow(Window parent,VXRECT* rc)
{
    if(m_xwin)
    {
        glXDestroyWindow(m_xdy,m_xwin);
        XDestroyWindow(m_xdy,(Window)m_hWnd);
    }

    if(!m_fbc)
    {
        int att[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT,
                     GLX_DOUBLEBUFFER, True,
                     None};
        int nelements;
       m_fbc = glXChooseFBConfig(m_xdy,
                                DefaultScreen(m_xdy),
                                att,
                                &nelements);
        if(!m_fbc)
            return FALSE;
    }
    XVisualInfo *vi = glXGetVisualFromFBConfig (m_xdy,*m_fbc);
    if(!vi)
        return FALSE;

    if(!m_cmap)
        m_cmap = XCreateColormap(m_xdy,
                                 RootWindow(m_xdy, vi->screen),
                                 vi->visual,
                                 AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = m_cmap;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;
    m_hWnd = (HVXWND)XCreateWindow(m_xdy,
                                                            parent?parent:RootWindow(m_xdy, vi->screen),
                                                            rc->left,
                                                            rc->top,
                                                            rc->right-rc->left,
                                                            rc->bottom-rc->top,
                                                            0,
                                                            vi->depth,
                                                            InputOutput,
                                                            vi->visual,
                                                            CWBackPixel|CWBorderPixel|CWColormap,
                                                            &swa);
    if(!m_hWnd)
        return FALSE;
    m_xwin = glXCreateWindow (m_xdy, *m_fbc, (Window)m_hWnd, NULL);
    return TRUE;
}
#endif

VXBOOL CVxGLVidPlayer::CreateOpenGL()
{
	m_rcFill.left = m_rcFill.top = 0;
	m_rcFill.right = m_width;
	m_rcFill.bottom = m_height;

	InitOpenGLExtensions();

#ifdef _WIN32
	m_hDC = GetDC(m_hWnd);
	PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR)};
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int  iPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	VERIFY(SetPixelFormat(m_hDC, iPixelFormat, &pfd)); 
	
	HGLRC glres = m_alone?NULL:(HGLRC)vxGetGLResourceRC();
	if(glres) 
	{
#ifdef _DEBUG
		HMODULE nleengin = GetModuleHandle("VxNLEEngineD.dll");
#else
		HMODULE nleengin = GetModuleHandle("VxNLEEngine.dll");
#endif
		_vxCreateShareGL = (HGLRC (__cdecl *)(SHAREGL gltype))GetProcAddress(nleengin,"_vxCreateShareGL");
		_vxDestroyShareGL = (void (__cdecl *)(SHAREGL gltype,HGLRC))GetProcAddress(nleengin,"_vxDestroyShareGL");

		m_hglrcP = _vxCreateShareGL(sharegl_live1);
		m_hglrc = _vxCreateShareGL(sharegl_live2);
	}
	else
	{
		m_hglrcP = wglCreateContext(m_hDC);
		if(!m_hglrcP) return FALSE;
		m_hglrc = wglCreateContext(m_hDC);
		wglShareLists(m_hglrcP,m_hglrc);
	}
	wglMakeCurrent (m_hDC,m_hglrcP);
#elif __APPLE__

	CGLContextObj glres = m_alone?NULL:(CGLContextObj)vxGetGLResourceRC();
	static int attribs[] =
	{
		kCGLPFAWindow,
		kCGLPFADoubleBuffer,
		kCGLPFAAccelerated,
		kCGLPFAColorSize, 24,
		0
	};	
	GLint numPixelFormats;
	CGLPixelFormatObj pixelFormatObj;
	CGLChoosePixelFormat ((CGLPixelFormatAttribute*)attribs, &pixelFormatObj, &numPixelFormats);
	if(!pixelFormatObj) return FALSE;
	CGLCreateContext(pixelFormatObj,glres,&m_cglrcP);
	if(!m_cglrcP)
	{
		CGLDestroyPixelFormat(pixelFormatObj);
		return FALSE;
	}
	if(glres)
		CGLCreateContext(pixelFormatObj, glres,&m_cglrc);
	else
		CGLCreateContext(pixelFormatObj, m_cglrcP,&m_cglrc);

	CGLDestroyPixelFormat(pixelFormatObj);

	InitGLView(m_hWnd,m_cglrcP,this);

	CGLSetCurrentContext(m_cglrcP);	
#else
    m_xdy = (Display*)m_setup->GetMainWnd();
    GLXContext glres = m_alone?NULL:(GLXContext)vxGetGLResourceRC();
    if(!__recreateglxwindow(NULL,CVxRect(0,0,m_width,m_height)))
        return FALSE;
    m_glx = glXCreateNewContext(m_xdy,*m_fbc,GLX_RGBA_TYPE,glres,true);
    if(glres)
        m_glxP = glXCreateNewContext(m_xdy,*m_fbc,GLX_RGBA_TYPE,glres,true);
    else
        m_glxP = glXCreateNewContext(m_xdy,*m_fbc,GLX_RGBA_TYPE,m_glx,true);
    glXMakeCurrent (m_xdy,m_xwin,m_glxP);
#endif
	if(!glGenFramebuffersEXT) 
	{
		VX_MailMSG(vxLoadMessageLV("OpenGL驱动错误,不支持FrameBuffer;请确认显卡是否支持,假如支持请重新启动计算机"),vxLoadMessageLV("OpenGL错误"),0,MAILSRC_ERROR|MAILID_MESSAGE);
		return FALSE;
	}
	glEnable(GL_TEXTURE_2D);
	
    glGenTextures(1, &m_tex);
	glBindTexture( GL_TEXTURE_2D,m_tex);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW/2,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE,0);
	glBindTexture(GL_TEXTURE_2D,0);

	glGenTextures(1, &m_rgbtex);
	glBindTexture( GL_TEXTURE_2D,m_rgbtex);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D,0);

	
	glGenTextures(1, &m_srceen);
	glBindTexture( GL_TEXTURE_2D,m_srceen);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_rcFill.Width(),m_rcFill.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D,0);

	glGenFramebuffersEXT(1,&m_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,m_srceen,0);
	ASSERT(vxgl::CheckFramebufferStatus());
	glViewport(0,0,m_rcFill.Width(),m_rcFill.Height());
	glClearColor(0.f,0.f,0.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

    if(HasGLSLSupport()&&IsShaderGraphic())
	{
		if(glCreateProgram)
			m_shader_ytor = vxgl::g_shaderManager->loadfromMemory(NULL,m_stype==shader_ytor709?g_ytor709_glsl:g_ytor601_glsl); // load (and compile, link) from file
	}

	if(m_shader_ytor)
	{
		glGenFramebuffersEXT(1,&m_yuv2rgbFBO);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_yuv2rgbFBO);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,m_rgbtex,0);
		ASSERT(vxgl::CheckFramebufferStatus());
		glViewport(0,0,m_texW,m_texH);
		glClearColor(0.f,0.f,0.f,0.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	}
	else
	{
		m_rgbbuf = (BYTE*)_vxmalloc(m_nLiveW*m_nLiveH*4);
	}

	GenVFList();
	glDisable(GL_TEXTURE_2D);

	glFinish();

#ifdef _WIN32
	wglMakeCurrent (0, 0) ;		
#elif __APPLE__
	CGLSetCurrentContext(NULL);
#else
    glXMakeCurrent (m_xdy,0,0);
#endif

	vxString fontpath = vxGetSysDirection("Fonts")+"/simhei.ttf";
	m_ftFont = ftglCreateBitmapFont(fontpath);
	if(m_ftFont)
		ftglSetFontFaceSize(m_ftFont,14,72);

	return TRUE;
}

void CVxGLVidPlayer::DestroyOpenGL()
{
#ifdef _WIN32
	HGLRC glres = m_alone?NULL:(HGLRC)vxGetGLResourceRC();
	if(m_hglrcP)
	{
		HGLRC lastrc = wglGetCurrentContext();HDC lastdc = NULL;
		if(m_hglrcP!=lastrc)
		{
			lastdc = wglGetCurrentDC();
			wglMakeCurrent(m_hDC,m_hglrcP);
		}
		glEnable(GL_TEXTURE_2D);

		if(m_ftFont) ftglDestroyFont(m_ftFont),m_ftFont = NULL;

		if(m_fbo) glDeleteFramebuffersEXT( 1,&m_fbo );
		glDeleteTextures( 1, &m_srceen );

		if(m_yuv2rgbFBO) glDeleteFramebuffersEXT( 1,&m_yuv2rgbFBO );
		glDeleteTextures( 1, &m_rgbtex );
		glDeleteTextures( 1, &m_tex );
		if(m_vflist) 
		{
			glDeleteLists(m_vflist,4);
			m_vflist = 0;
		}
		if(m_maskData)
		{
			glDeleteTextures(1,&m_masktex);
			_vxfree(m_maskData);	
			m_maskData = NULL;
		}

		if(m_shader_ytor) vxgl::g_shaderManager->Free(m_shader_ytor),m_shader_ytor = NULL;
//		if(m_shader_gamma) vxgl::g_shaderManager->free(m_shader_gamma),m_shader_gamma = NULL;
		if(m_rgbbuf) _vxfree(m_rgbbuf),m_rgbbuf = NULL;

		glDisable(GL_TEXTURE_2D);

		glFinish();

		ASSERT(lastdc!=m_hDC);

		if(m_hglrcP!=lastrc)
			wglMakeCurrent (lastdc,lastrc);
		else
			wglMakeCurrent(0,0);
		if(glres) 
		{
			_vxDestroyShareGL(sharegl_live2,m_hglrc);
			_vxDestroyShareGL(sharegl_live1,m_hglrcP);
		}
		else
		{
			VERIFY(wglDeleteContext (m_hglrc));
			VERIFY(wglDeleteContext (m_hglrcP));
		}
		ReleaseDC(m_hWnd,m_hDC);
		m_hDC = NULL;
		m_hglrc = m_hglrcP = NULL;
	}
#elif __APPLE__
	CGLContextObj glres = m_alone?NULL:(CGLContextObj)vxGetGLResourceRC();
	if(m_cglrcP)
	{
		CGLContextObj lastcgl = CGLGetCurrentContext();
		if(lastcgl!=m_cglrcP)
			CGLSetCurrentContext(m_cglrcP);
		glEnable(GL_TEXTURE_2D);
		
		if(m_ftFont) ftglDestroyFont(m_ftFont),m_ftFont = NULL;

		glDeleteFramebuffersEXT( 1,&m_fbo );
		glDeleteTextures( 1, &m_srceen );

		glDeleteFramebuffersEXT( 1,&m_yuv2rgbFBO );
		glDeleteTextures( 1, &m_rgbtex );
		glDeleteTextures( 1, &m_tex );
		if(m_vflist) 
		{
			glDeleteLists(m_vflist,4);
			m_vflist = 0;
		}

		if(m_shader_ytor) vxgl::g_shaderManager->Free(m_shader_ytor),m_shader_ytor = NULL;
//		if(m_shader_gamma) vxgl::g_shaderManager->free(m_shader_gamma),m_shader_gamma = NULL;

		glDisable(GL_TEXTURE_2D);

		glFinish();

		if(lastcgl!=m_cglrcP)
			CGLSetCurrentContext(lastcgl);
		else
			CGLSetCurrentContext(NULL);

		CGLReleaseContext(m_cglrc);
		CGLReleaseContext(m_cglrcP);

		m_cglrc = m_cglrcP = NULL;
	}
#else
    GLXContext glres = m_alone?NULL:(GLXContext)vxGetGLResourceRC();
    if(m_glxP)
    {
        GLXContext lastcgl = glXGetCurrentContext();
        GLXDrawable lastxwin = glXGetCurrentDrawable();
        if(lastcgl!=m_glxP)
            glXMakeCurrent(m_xdy,m_xwin,m_glxP);
        glEnable(GL_TEXTURE_2D);

        if(m_ftFont) ftglDestroyFont(m_ftFont),m_ftFont = NULL;

        glDeleteFramebuffersEXT( 1,&m_fbo );
        glDeleteTextures( 1, &m_srceen );

        glDeleteFramebuffersEXT( 1,&m_yuv2rgbFBO );
        glDeleteTextures( 1, &m_rgbtex );
        glDeleteTextures( 1, &m_tex );
        if(m_vflist)
        {
            glDeleteLists(m_vflist,4);
            m_vflist = 0;
        }

        if(m_shader_ytor) vxgl::g_shaderManager->Free(m_shader_ytor),m_shader_ytor = NULL;
//		if(m_shader_gamma) vxgl::g_shaderManager->free(m_shader_gamma),m_shader_gamma = NULL;

        glDisable(GL_TEXTURE_2D);

        glFinish();

        if(lastcgl!=m_glxP)
            glXMakeCurrent(m_xdy,lastxwin,lastcgl);
        else
            glXMakeCurrent(m_xdy,0,0);

        glXDestroyContext(m_xdy,m_glx);
        glXDestroyContext(m_xdy,m_glxP);
        glXDestroyWindow(m_xdy,m_xwin);
        XDestroyWindow(m_xdy,(Window)m_hWnd);
        XFreeColormap(m_xdy,m_cmap);
        m_glx = m_glxP = NULL;
        m_cmap = NULL;
    }
#endif
	GLLIVEOUT_SURFACE liveo;
	while (m_liveout.Pop(&liveo))
		liveo.surface->Release();
}



VXBOOL CVxGLVidPlayer::CreateBuffers()
{
	__liveconvert = NULL;
	m_livetx = 1.f;m_livety = 1.f;
	if(m_stdres&&!m_fullres)
	{
		m_nLiveW = m_width;m_nLiveH = m_height/2;
		if(m_height == 2160)
		{
			m_nLiveW = m_width==4096?DX9_2160ILIVE_W:DX9_HDV_2160ILIVE_W;
			m_nLiveH = DX9_2160ILIVE_H;
		}
		else if(m_height==1080)
		{
			m_nLiveW = m_width==1920?DX9_1080ILIVE_W:DX9_HDV_1080ILIVE_W;
			m_nLiveH = DX9_1080ILIVE_H;
		}
		else if(m_height==720)
		{
			m_nLiveW = DX9_720PLIVE_W;
			m_nLiveH = DX9_720PLIVE_H;
		}

		if(m_width==720)
			__liveconvert = m_height==576?_convert_sd576:_convert_sd480;
		else if(m_height == 2160)
		{
			if (m_width == 4096)
			{
				__liveconvert = _convert_4096x1080_to_2048x1080;
			}
			else
			{
				__liveconvert = _convert_3840x1080_to_1920x1080;
			}
		}
		else if ((m_height == 1080) && (m_width<=1920))
		{
			if(m_width==1920)
			{
				__liveconvert = _convert_1920x540_to_960x540;
			}
			else
			{
				__liveconvert = _convert_1440x540_to_720x540;
			}
		}
		else if (m_width > 1920)
		{
			m_nLiveW = m_width / 2; m_nLiveH = m_height / 2;
			__liveconvert = s_half;
		}
		else if (m_width == 1280)
		{
			__liveconvert = _convert_1280x720_to_640x360;
		}
	}
	if(!__liveconvert)
	{
		m_nLiveW = m_width;
		m_nLiveH = m_height;
	}

	m_liveout.SetMaxSize(DX9VOUTPUT_COUNTS);

	m_texW = m_nLiveW,m_texH = m_nLiveH;

	return TRUE;
}

void CVxGLVidPlayer::DestroyBuffers()
{
	GLLIVEOUT_SURFACE livesf;
	while (m_liveout.Pop(&livesf))
		livesf.surface->Release();
}


VXBOOL CVxGLVidPlayer::Initialize()
{
	if(!CreateBuffers()) return FALSE;

	if(!CreateDrawWindow()) return FALSE;

	if(!CreateOpenGL()) return FALSE;

	m_bExitLive = FALSE;
	pthread_create(&m_hLiveThread,NULL,LiveThreadProc,this);

	return TRUE;
}

void CVxGLVidPlayer::Uninitialize()
{
	EXITPTHREAD(m_hLiveThread,m_bExitLive);

	SetHwnd(NULL,NULL);

	if (m_deinterlace)
	{
		vxenc_image_free(&m_deiyuv422p);
		m_deinterlace = NULL;
	}

	DestroyOpenGL();
	DestroyDrawWindow();
	DestroyBuffers();
}


VXBOOL CVxGLVidPlayer::Set3DMode(int mode)
{
	if(m_width!=1920) {m_3dmode = mode;return FALSE;}//修改Bug040938，m_3dmode要设置一下，否则Is3D()返回值不正确（lxp2015.08.24）
	if(mode==m_3dmode) return TRUE;
	if(mode>0 && mode!=3)
		__liveconvert = (mode==1)?_convert_1920x540_to_960x540L:_convert_1920x540_to_960x540R;
	else
		__liveconvert = _convert_1920x540_to_960x540;
	m_3dmode = mode;
	return TRUE;
}


VXBOOL CVxGLVidPlayer::Reset(IVxSystemClock* clock,int w,int h)
{
	EXITPTHREAD(m_hLiveThread,m_bExitLive);
	m_clock = clock;
	if (m_clock)
	{
		const sysclk_cinfo* cinfo = m_clock->GetCreateInfo();
		VXBOOL progessive = cinfo->scantype == SCANTYPE_PROGRESSIVE;
		if ((m_width != w) || (m_height != h) || (m_rate != cinfo->rate) || (m_scale != cinfo->scale) || (m_progressive != progessive))
		{
			m_width = w; m_height = h;

			m_rate = cinfo->rate;
			m_scale = cinfo->scale;
			m_aspect = cinfo->aspect;
			m_stdres = !(cinfo->res&VXCUSTOMRESMASK);
			m_progressive = cinfo->scantype == SCANTYPE_PROGRESSIVE;

			yuy2_to_bgra = yuv_to_bgras[FMT_YUY2][cinfo->colorimetry];

			DestroyBuffers();
			CreateBuffers();

#ifdef _WIN32
			HGLRC lastrc = wglGetCurrentContext();HDC lastdc = NULL;
			if(lastrc!=m_hglrcP)
			{
				lastdc = wglGetCurrentDC();
				wglMakeCurrent(m_hDC,m_hglrcP);
			}
#elif __APPLE__
			CGLContextObj lastcgl = CGLGetCurrentContext();
			if(lastcgl!=m_cglrcP)
				CGLSetCurrentContext(m_cglrcP);
#else
            GLXContext lastglx = glXGetCurrentContext();GLXDrawable lastdraw = NULL;
            if(lastglx!=m_glxP)\
            {
                lastdraw = glXGetCurrentDrawable();
                glXMakeCurrent(m_xdy,m_xwin,m_glxP);
            }
#endif
			glEnable(GL_TEXTURE_2D);

			glDeleteTextures( 1, &m_rgbtex );
			glDeleteTextures( 1, &m_tex );
			if(m_maskData) glDeleteTextures(1,&m_masktex);
			if(m_rgbbuf) _vxfree(m_rgbbuf),m_rgbbuf = NULL;

			glGenTextures(1, &m_tex);
			glBindTexture( GL_TEXTURE_2D,m_tex);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW/2,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE,0);
			glBindTexture(GL_TEXTURE_2D,0);

			glGenTextures(1, &m_rgbtex);
			glBindTexture( GL_TEXTURE_2D,m_rgbtex);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D,0);


			if(m_shader_ytor)
			{
				ShaderType  stype = w > 720 ? shader_ytor709 : shader_ytor601;
				if (m_stype != stype)
				{
					vxgl::g_shaderManager->Free(m_shader_ytor);
					m_shader_ytor = vxgl::g_shaderManager->loadfromMemory(NULL, m_stype == shader_ytor709 ? g_ytor709_glsl : g_ytor601_glsl); // load (and compile, link) from file
					m_stype = stype;
				}
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_yuv2rgbFBO);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,m_rgbtex,0);
				ASSERT(vxgl::CheckFramebufferStatus());
				glViewport(0,0,m_texW,m_texH);
				glClearColor(0.f,0.f,0.f,0.f);
				glClear(GL_COLOR_BUFFER_BIT);
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			}
			else
			{
				m_rgbbuf = (BYTE*)_vxmalloc(m_nLiveW*m_nLiveH*4);
			}

			if(m_maskData)
			{
				glGenTextures(1, &m_masktex);
				glBindTexture( GL_TEXTURE_2D,m_masktex);
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_maskData);
				glBindTexture(GL_TEXTURE_2D,0);
			}

			glDisable(GL_TEXTURE_2D);

#ifdef _WIN32
			if(lastrc!=m_hglrcP)
				wglMakeCurrent(lastdc,lastrc);
#elif  __APPLE__
			if(lastcgl!=m_cglrcP)
				CGLSetCurrentContext(lastcgl);
#else
            if(lastglx!=m_glxP)
                glXMakeCurrent(m_xdy,lastdraw,lastglx);
#endif
		}
		m_bExitLive = FALSE;
		pthread_create(&m_hLiveThread,NULL,LiveThreadProc,this);
	}
	return TRUE;
}

VXBOOL CVxGLVidPlayer::SetHwnd(HVXWND hWnd,LPVXRECT lpRect)
{
	TRACE("[0x%08x]::SetHwnd\n",(vxuintptr)hWnd);
	if(lpRect)
	{
		TRACE("SetWnd:[%d,%d,%d,%d]\n",lpRect->left,lpRect->top,lpRect->right,lpRect->bottom);
	}
	if((m_parentwnd==hWnd)&&lpRect&&(memcmp(&m_rcFill,lpRect,sizeof(VXRECT))==0)) 
	{
#if __APPLE__
		CGSConnectionID cid;CGSWindowID wid;CGSSurfaceID sid;
		CGLGetSurface(m_cglrcP,&cid,&wid,&sid);
		CGLSetSurface(m_cglrc,cid,wid,sid);
#endif		
		return TRUE;
	}

	CVxLock lock(&m_csLiveRnd);

	if(lpRect&&hWnd)
	{
		VXBOOL resize = memcmp(&m_rcFill,lpRect,sizeof(VXRECT))!=0;
		if(((lpRect->right-lpRect->left)!=0)&&((lpRect->bottom-lpRect->top)!=0))
		{
			m_rcFill = *lpRect;
			m_resize |= resize;
		}
	}

	if(hWnd != m_parentwnd)
	{
#ifdef _WIN32		
		if( m_parentwnd!= NULL)
		{
			LONG style = GetWindowLong( m_parentwnd,GWL_STYLE );
			SetWindowLong( m_parentwnd,GWL_STYLE,style &(~WS_CLIPCHILDREN) );
			InvalidateRect(m_parentwnd,NULL,FALSE);
			UpdateWindow(m_parentwnd);
		}
		ShowWindow(m_hWnd,SW_HIDE);
		SetParent(m_hWnd,m_setup->GetMainWnd());
#elif __APPLE__
		SetParant(m_hWnd,NULL,NULL);
#else
        XUnmapWindow (m_xdy,(Window)m_hWnd);
        XReparentWindow(m_xdy,(Window)m_hWnd,RootWindow(m_xdy, DefaultScreen(m_xdy)),0,0);
        XFlush(m_xdy);
#endif
	}
	m_parentwnd =  hWnd;
	if(hWnd) 
	{
#ifdef _WIN32		
		LONG style = GetWindowLong( m_parentwnd,GWL_STYLE );
		SetWindowLong( m_parentwnd,GWL_STYLE,style |WS_CLIPCHILDREN );
		SetParent(m_hWnd,m_parentwnd);
		SetWindowPos(m_hWnd,HWND_TOP,m_rcFill.left,m_rcFill.top,m_rcFill.Width(),m_rcFill.Height(),SWP_SHOWWINDOW|SWP_NOREDRAW|SWP_NOCOPYBITS);
#elif __APPLE__
		SetParant(m_hWnd,hWnd,&m_rcFill);
		CGSConnectionID cid;CGSWindowID wid;CGSSurfaceID sid;
		CGLGetSurface(m_cglrcP,&cid,&wid,&sid);
		CGLSetSurface(m_cglrc,cid,wid,sid);
#else
        m_resize = TRUE;
//        EXITPTHREAD(m_hLiveThread,m_bExitLive);
//        __recreateglxwindow((Window)hWnd);
//        XReparentWindow(m_xdy,(Window)m_hWnd,(Window)hWnd,m_rcFill.left,m_rcFill.top);
//        XResizeWindow(m_xdy,(Window)m_hWnd,m_rcFill.Width(),m_rcFill.Height());
//        XMapWindow (m_xdy,(Window)m_hWnd);
 //       XFlush(m_xdy);
//        m_bExitLive = FALSE;
//        pthread_create(&m_hLiveThread,NULL,LiveThreadProc,this);
 #endif
	}
	return TRUE;
}

void CVxGLVidPlayer::SwitchDataCore(IVxDataCore* dc,IVxDataCore** olddc)
{
    if(olddc&&m_dc)
        GetVxInterface(m_dc,(void**)olddc);
    m_dc = dc;
}
void CVxGLVidPlayer::SetLiveMode(int iMode)
{
	m_iLiveMode = iMode;
}

static float vecs[]	=  {	-1.f,-1.f,
							 1.f,-1.f,
							 1.f, 1.f,
							-1.f, 1.f };
static float vecs2[] =  {	-1.f, 1.f,
							 1.f, 1.f,
							 1.f,-1.f,
							-1.f,-1.f, };
static float texs[] = {	0.f, 0.f,
						1.f, 0.f,
						1.f, 1.f,
						0.f, 1.f};

#ifdef _WIN32
void CVxGLVidPlayer::__OnPaint(HDC hdc)
{
	TRACE("[0x%08x]::OnPaint\n",(DWORD)m_hWnd);
	CVxTryLock lock(&m_csLiveRnd);
	if(!lock.m_bLock) return;

	HGLRC lastrc = wglGetCurrentContext();
	HDC lastdc = wglGetCurrentDC();
	if(lastrc!=m_hglrcP)
	{
		PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR)};
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 0;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int  iPixelFormat = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, iPixelFormat, &pfd); 
		wglMakeCurrent(hdc,m_hglrcP);
	}
	glViewport(0,0,m_rcFill.Width(),m_rcFill.Height());

	if(m_srceen)
	{
		glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
		glLoadIdentity();

		glClearColor(0.f,0.f,1.f,0.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D,m_srceen);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT,0,vecs); 
		glTexCoordPointer(2, GL_FLOAT,0,texs);  
//		glInterleavedArrays(GL_T2F_V3F,0,vecs);
		glDrawArrays(GL_QUADS,0,4);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindTexture( GL_TEXTURE_2D,0);
		glDisable( GL_TEXTURE_2D );

		glPopMatrix();
	}
	else
	{
		glClearColor(1.f,0.f,0.f,1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	SwapBuffers(hdc);
	if(lastrc!=m_hglrcP)
		wglMakeCurrent(lastdc,lastrc);
	else
		wglMakeCurrent(0,0);
}
#elif __APPLE__
void CVxGLVidPlayer::__Unhide()
{
	if(m_parentwnd) 
	{
		SetParant(m_hWnd,NULL,NULL);
		SetParant(m_hWnd,m_parentwnd,&m_rcFill);
		CGSConnectionID cid;CGSWindowID wid;CGSSurfaceID sid;
		CGLGetSurface(m_cglrcP,&cid,&wid,&sid);
		CGLSetSurface(m_cglrc,cid,wid,sid);
	}
}
#endif
	
	


void CVxGLVidPlayer::Liveoutput(__int64 llFrame, __uint64 timestamp,IVxSurface* sf, VXBOOL bSeek, vxvideoflag vf, float speed)
{
	CVxTryLock lockLive(&m_csLive);
	if(!lockLive.m_bLock) return;

	if (m_deinterlace)
	{
		VXSURFACE_DESC desc;
		sf->GetDesc(&desc);
		VXSURFACE_LOCK lock;
		sf->Lock(&lock);
		void* _srcbuf[4] = { 0 }; int _spitch[4] = { 0 };
		__getplanar(desc.fmt, lock.pLock, lock.pitch, desc.width, desc.height, _srcbuf, _spitch);
		yuy2_to_422p(_srcbuf[0], desc.validw, desc.validh, _spitch[0], m_deiyuv422p.data[0], m_deiyuv422p.data[1], m_deiyuv422p.data[2], m_deiyuv422p.pitch[0]);
		DEIINFO dinfo = { llFrame, timestamp, vf, speed };
		VERIFY(m_qTC.Push(dinfo));
		m_deinterlace->Filter(&m_deiyuv422p);
		if (bSeek) m_deinterlace->Flush();
	}
	else
	{
		GLLIVEOUT_SURFACE liveout = { llFrame, timestamp, sf,vf, speed };
		if (m_liveout.Push(liveout))
			sf->AddRef();
	}
}

VXBOOL CVxGLVidPlayer::__FilterOut(const vxenc_image* img)
{
	CVxLock lockLive(&m_csLive);
	DEIINFO tc;
	m_qTC.Pop(&tc);
	IVxSurface* sf = NULL;
	if (m_pool->GetSurface(FMT_YUY2, LOC_HOST, m_res, &sf))
	{
		VXSURFACE_DESC desc;
		sf->GetDesc(&desc);
		VXSURFACE_LOCK lock;
		sf->Lock(&lock);
		void* _dstbuf[4] = { 0 }; int _dpitch[4] = { 0 };
		__getplanar(desc.fmt, lock.pLock, lock.pitch, desc.width, desc.height, _dstbuf, _dpitch);
		yuv422p8_to_yuy2((void**)img->data, (int*)img->pitch, m_width, m_height, _dstbuf[0], _dpitch[0]);
		sf->Unlock();
		GLLIVEOUT_SURFACE liveout = { tc.frame,tc.tc, sf,tc.vf, tc.speed };
		m_liveout.Push(liveout);
	}
	return TRUE;
}

VXBOOL CVxGLVidPlayer:: LockLiveIn(LIVE_IN* live)
{
	CVxLock lockLive(&m_csLive);
	if (!m_pool) return FALSE;
	IVxSurface* sf = NULL;
	if(m_pool->GetSurface(FMT_YUY2, LOC_HOST, m_res, &sf))
	{
		live->core = sf;
		VXSURFACE_DESC desc;
		sf->GetDesc(&desc);
		VXSURFACE_LOCK lock;
		sf->Lock(&lock);

		live->pByte = (PBYTE)lock.pLock;
		live->pitch = lock.pitch;
		live->w = desc.validw;
		live->h = desc.validh;
		return TRUE;
	}
	return FALSE;
}
void CVxGLVidPlayer::UnlockLiveIn(__uint64 ullSystemClock, LIVE_IN* live, int validw, int validh)
{
	CVxLock lockLive(&m_csLive);
	IVxSurface* sf = (IVxSurface*)live->core;
	sf->Unlock();
	VXSURFACE_DESC desc;
	sf->GetDesc(&desc);
	desc.validw = validw; desc.validh = validh;
	sf->SetDesc(&desc);
	GLLIVEOUT_SURFACE liveout = { 0, ullSystemClock, sf,vf_nono, 1.f };
	if(!m_liveout.Push(liveout))
		sf->Release();
}

void CVxGLVidPlayer::GenVFList()
{
	if(m_vflist) glDeleteLists(m_vflist,4);

	m_vflist = glGenLists(4);
	GLuint box = m_vflist;
	glNewList(box,GL_COMPILE);
		glPushAttrib(GL_CURRENT_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.5f,0.5f,0.9f,0.8f);
		glBegin( GL_QUADS );
			glVertex2f( -0.9f,  -0.5f);
			glVertex2f( -0.86f, -0.5f );
			glVertex2f( -0.86f, -0.8f );
			glVertex2f( -0.9f,  -0.8f );

			glVertex2f( -0.9f,  -0.8f );
			glVertex2f( -0.6f,  -0.8f );
			glVertex2f( -0.6f,  -0.86f );
			glVertex2f( -0.9f,  -0.86f );
		glEnd();
		glDisable(GL_BLEND);
		glPopAttrib();
	glEndList();

	box += 1;
	glNewList(box,GL_COMPILE);
		glPushAttrib(GL_CURRENT_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.5f,0.5f,0.9f,0.8f);
		glBegin( GL_QUADS );
			glVertex2f( 0.86f,-0.5f );
			glVertex2f( 0.9f, -0.5f );
			glVertex2f( 0.9f, -0.8f );
			glVertex2f( 0.86f,-0.8f );

			glVertex2f( 0.6f,  -0.8f );
			glVertex2f( 0.6f,  -0.86f );
			glVertex2f( 0.9f,  -0.86f );
			glVertex2f( 0.9f,  -0.8f );
		glEnd();
		glDisable(GL_BLEND);
		glPopAttrib();
	glEndList();

	box += 1;
	glNewList(box,GL_COMPILE);
		glPushAttrib(GL_CURRENT_BIT);
		glColor4f(0.5f,0.5f,0.9f,0.98f);
		glBegin( GL_QUADS );
			glVertex2f( 0.92f, 1.f);
			glVertex2f( 1.f,   1.f );
			glVertex2f( 1.f,  -1.f );
			glVertex2f( 0.92f,-1.f );
		glEnd();
		glPopAttrib();
	glEndList();

	box += 1;
	glNewList(box,GL_COMPILE);
		glPushAttrib(GL_CURRENT_BIT);
		glColor4f(0.46f,0.43f,0.24f,1.f);
		glBegin(GL_LINES);
			glVertex2f( -0.9f, -0.9f);glVertex2f( 0.9f, -0.9f);
			glVertex2f(0.9f, -0.9f); glVertex2f(0.9f, 0.9f);
			glVertex2f(0.9f, 0.9f); glVertex2f(-0.9f, 0.9f);
			glVertex2f(-0.9f, 0.9f); glVertex2f(-0.9f, -0.9f);
			glVertex2f(-0.8f, -0.8f); glVertex2f(0.8f, -0.8f);
			glVertex2f(0.8f, -0.8f); glVertex2f(0.8f, 0.8f);
			glVertex2f(0.8f, 0.8f); glVertex2f(-0.8f, 0.8f);
			glVertex2f(-0.8f, 0.8f); glVertex2f(-0.8f, -0.8f);
			glEnd();
		glPopAttrib();
	glEndList();
}

void CVxGLVidPlayer::DrawVideoFlag(vxvideoflag vf)
{
	if(vf==vf_nono) return;
	glCallList(m_vflist+vf-1);
}

void CVxGLVidPlayer::DrawSaftRect()
{
	glCallList(m_vflist+3);
}

void CVxGLVidPlayer::SetCliper(float aspect,float ubratio)
{
	CVxLock lock(&m_csLiveRnd);

#ifdef _WIN32
	if(!m_hglrcP) return;
	HGLRC lastrc = wglGetCurrentContext();HDC lastdc = NULL;
	if(lastrc!=m_hglrcP)
	{
		lastdc = wglGetCurrentDC();
		wglMakeCurrent(m_hDC,m_hglrcP);
	}
#elif __APPLE__
	if(!m_cglrcP) return;
	CGLContextObj lastcgl = CGLGetCurrentContext();
	if(lastcgl!=m_cglrcP)
		CGLSetCurrentContext(m_cglrcP);
#else
    if(!m_glxP) return;
    GLXContext lastglx = glXGetCurrentContext();GLXDrawable lastdraw = NULL;
    if(lastglx!=m_glxP)
    {
        lastdraw = glXGetCurrentDrawable();
        glXMakeCurrent(m_xdy,m_xwin,m_glxP);
    }
#endif
	if(aspect<=0.f)
	{
		if(m_masktex)
		{
			glDeleteTextures(1,&m_masktex);
			m_masktex = 0;
		}
		if(m_maskData){_vxfree(m_maskData);m_maskData = NULL;}
	}
	else
	{
		int pitch = m_texW*4;
		if(!m_maskData)  m_maskData = _vxmalloc(pitch*m_texH);
		memset(m_maskData,0,pitch*m_texH);
		int h = (int)((m_width*m_nLiveH)/(aspect*m_height));
		int maskh = m_nLiveH-h;
		if(maskh>0)
		{
			int th = (int)(maskh/2*(1+ubratio));
			int bh = maskh-th;
			BYTE* buf = (BYTE*)m_maskData;
			memset(buf,200,pitch*th);
			memset(buf+(m_nLiveH-bh)*pitch,200,pitch*bh);
		}
		if(m_masktex)
		{
			glBindTexture(GL_TEXTURE_2D,m_masktex);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,m_texW,m_texH,GL_RGBA, GL_UNSIGNED_BYTE,m_maskData);
			glBindTexture(GL_TEXTURE_2D,0);
		}
		else
		{
			glGenTextures(1,&m_masktex);
			glBindTexture(GL_TEXTURE_2D,m_masktex);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE,m_maskData);
			glBindTexture(GL_TEXTURE_2D,0);
		}
	}
	glFinish();
#ifdef _WIN32
	if(lastrc!=m_hglrcP)
		wglMakeCurrent(lastdc,lastrc);
#elif __APPLE__
	if(lastcgl!=m_cglrcP)
		CGLSetCurrentContext(lastcgl);
#else
    if(lastglx!=m_glxP)
        glXMakeCurrent(m_xdy,lastdraw,lastglx);
#endif
}

void CVxGLVidPlayer::SetCliper(float left,float top,float right,float bottom)
{
	CVxLock lock(&m_csLiveRnd);

#ifdef _WIN32
	if(!m_hglrcP) return;
	HGLRC lastrc = wglGetCurrentContext();HDC lastdc = NULL;
	if(lastrc!=m_hglrcP)
	{
		lastdc = wglGetCurrentDC();
		wglMakeCurrent(m_hDC,m_hglrcP);
	}
#elif __APPLE__
    if(!m_cglrcP) return;
    CGLContextObj lastcgl = CGLGetCurrentContext();
    if(lastcgl!=m_cglrcP)
        CGLSetCurrentContext(m_cglrcP);
#else
    if(!m_glxP) return;
    GLXContext lastglx = glXGetCurrentContext();GLXDrawable lastdraw = NULL;
    if(lastglx!=m_glxP)
    {
        lastdraw = glXGetCurrentDrawable();
        glXMakeCurrent(m_xdy,m_xwin,m_glxP);
    }
#endif
    int x = (int)(left*m_nLiveW + 0.5f);
	int y = (int)(top*m_nLiveH + 0.5f);
	int r = (int)(right*m_nLiveW + 0.5f);
	int b = (int)(bottom*m_nLiveH + 0.5f);
	int w = r-x;
	int h = b-y;
	if((w==m_nLiveW)&&(h==m_nLiveH))
	{
		if(m_masktex)
		{
			glDeleteTextures(1,&m_masktex);
			m_masktex = 0;
		}
		if(m_maskData){_vxfree(m_maskData);m_maskData = NULL;}
	}
	else
	{
		ASSERT(x+w<=m_nLiveW);
		ASSERT(y+h<=m_nLiveH);

		int pitch = m_texW*4;
		if(!m_maskData)  m_maskData = _vxmalloc(pitch*m_texH);
		memset(m_maskData,200,pitch*m_texH);
		BYTE* buf = ((BYTE*)m_maskData)+y*pitch+x*4;
		for(int i=0;i<h;i++)
		{
			memset(buf,0,w*4);
			buf += pitch;
		}
		if(m_masktex)
		{
			glBindTexture(GL_TEXTURE_2D,m_masktex);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,m_texW,m_texH,GL_RGBA, GL_UNSIGNED_BYTE,m_maskData);
			glBindTexture(GL_TEXTURE_2D,0);
		}
		else
		{
			glGenTextures(1,&m_masktex);
			glBindTexture(GL_TEXTURE_2D,m_masktex);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_texW,m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE,m_maskData);
			glBindTexture(GL_TEXTURE_2D,0);
		}
	}
	glFinish();
#ifdef _WIN32
	if(lastrc!=m_hglrcP)
		wglMakeCurrent(lastdc,lastrc);
#elif __APPLE__
    if(lastcgl!=m_cglrcP)
        CGLSetCurrentContext(lastcgl);
#else
    if(lastglx!=m_glxP)
        glXMakeCurrent(m_xdy,lastdraw,lastglx);
#endif
}

bool IsMesaGraphic()
{
    static bool detect = false;
    static bool isintel = false;
    if(detect) return isintel;
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    if(vendor)
    {
        TRACE("IsMesaGraphic %s\n",vendor);
        vxString strVendor(vendor);
        strVendor.MakeLower();
        isintel = strVendor.Find("mesa")>=0;
    }
    detect = true;
    return isintel;
}

void CVxGLVidPlayer:: SetDrawExt(DWORD type,int enable)
{
	CVxLock lock(&m_csLiveRnd);
	if (type&DRAWEXT_ODDEVEN) m_iDrawMode = enable;
	if (type&DRAWEXT_SAFTRECT) m_saftrect = enable;
	if (type&DRAWEXT_ENGINEINFO) m_drawinfo = enable;

	if ((type&DRAWEXT_FULLRES) && (m_fullres != enable))
	{
		EXITPTHREAD(m_hLiveThread, m_bExitLive);

		m_fullres = enable;

		DestroyBuffers();
		CreateBuffers();


#ifdef _WIN32
		HGLRC lastrc = wglGetCurrentContext(); HDC lastdc = NULL;
		if (lastrc != m_hglrcP)
		{
			lastdc = wglGetCurrentDC();
			wglMakeCurrent(m_hDC, m_hglrcP);
		}
#elif __APPLE__
		CGLContextObj lastcgl = CGLGetCurrentContext();
		if (lastcgl != m_cglrcP)
			CGLSetCurrentContext(m_cglrcP);
#else
		GLXContext lastglx = glXGetCurrentContext(); GLXDrawable lastdraw = NULL;
		if (lastglx != m_glxP)\
		{
			lastdraw = glXGetCurrentDrawable();
			glXMakeCurrent(m_xdy, m_xwin, m_glxP);
		}
#endif
		glEnable(GL_TEXTURE_2D);

		glDeleteTextures(1, &m_rgbtex);
		glDeleteTextures(1, &m_tex);
		if (m_maskData) glDeleteTextures(1, &m_masktex);
		if (m_rgbbuf) _vxfree(m_rgbbuf), m_rgbbuf = NULL;

		glGenTextures(1, &m_tex);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texW / 2, m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &m_rgbtex);
		glBindTexture(GL_TEXTURE_2D, m_rgbtex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texW, m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);


		if (m_shader_ytor)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_yuv2rgbFBO);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rgbtex, 0);
			ASSERT(vxgl::CheckFramebufferStatus());
			glViewport(0, 0, m_texW, m_texH);
			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}
		else
		{
			m_rgbbuf = (BYTE*)_vxmalloc(m_nLiveW*m_nLiveH * 4);
		}

		if (m_maskData)
		{
			glGenTextures(1, &m_masktex);
			glBindTexture(GL_TEXTURE_2D, m_masktex);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texW, m_texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_maskData);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glDisable(GL_TEXTURE_2D);

#ifdef _WIN32
		if (lastrc != m_hglrcP)
			wglMakeCurrent(lastdc, lastrc);
#elif  __APPLE__
		if (lastcgl != m_cglrcP)
			CGLSetCurrentContext(lastcgl);
#else
		if (lastglx != m_glxP)
			glXMakeCurrent(m_xdy, lastdraw, lastglx);
#endif
		m_bExitLive = FALSE;
		pthread_create(&m_hLiveThread,NULL,LiveThreadProc,this);
	}

	CVxLock lockLive(&m_csLive);
	if (!m_fullres)
	{
		if (m_deinterlace != NULL)
		{
			vxenc_image_free(&m_deiyuv422p);
			m_deinterlace = NULL;
		}
		if (type&DRAWEXT_DEINTERLACE)
			m_deitype = enable;
	}
	else if (((type&DRAWEXT_DEINTERLACE) && (m_deitype != enable)) || (m_deitype && !m_deinterlace))
	{
		if (m_deitype && !m_deinterlace)
			enable = m_deitype;
		if (m_progressive||!m_pool)
			enable = 0;

		if (!enable)
		{
			m_qTC.Clear();
			vxenc_image_free(&m_deiyuv422p);
			m_deitype = 0;
			m_deinterlace = NULL;
		}
		else
		{
			m_deitype = enable;
			if (m_deinterlace)
			{
				m_deinterlace = NULL;
				m_qTC.Clear();
				vxCreateDeinterlace(m_deitype - 1, &m_deinterlace);
				m_deinterlace->Init(m_width, m_height, FMT_YUV422P8, FilterOut, this);
			}
			else
			{
				vxCreateDeinterlace(m_deitype - 1, &m_deinterlace);
				vxenc_image_alloc(&m_deiyuv422p, FMT_YUV422P8, m_width, m_height);
				m_deinterlace->Init(m_width, m_height, FMT_YUV422P8, FilterOut, this);
				m_qTC.SetMaxSize(16);
			}
		}
	}
}


int CVxGLVidPlayer::GetDrawExt(DWORD type)
{
	if (type == DRAWEXT_SAFTRECT)
		return m_saftrect;
	else if (type == DRAWEXT_ENGINEINFO)
		return m_drawinfo;
	else if (type == DRAWEXT_FULLRES)
		return m_fullres;
	else if (type == DRAWEXT_DEINTERLACE)
		return m_deitype;
	else
		return 0;
}

void CVxGLVidPlayer::_LiveThread()
{
	_vxSetThreadName("视频预监");
	VXEVENT hClock = m_clock->CreateClockEvent();

    const GLint swapinterval = 1;
#ifdef _WIN32
	HDC hdc = GetDC(m_hWnd);
	PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR)};
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int  iPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, iPixelFormat, &pfd); 
	wglMakeCurrent(hdc,m_hglrc);
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(swapinterval);
#elif __APPLE__
	CGLSetCurrentContext(m_cglrc);
    CGLSetParameter(m_cglrc,kCGLCPSwapInterval,&swapinterval);
#else
    glXMakeCurrent(m_xdy,m_xwin,m_glx);
    if(glXSwapIntervalEXT)
        glXSwapIntervalEXT(m_xdy,m_xwin,swapinterval);
#endif

    glShadeModel(GL_SMOOTH);

	const sysclk_cinfo* cinfo = m_clock->GetCreateInfo();
	int w, h;
	__vxgetreswh(cinfo->res, w, h);
	int aspectw = cinfo->aspect & 0xFFFF;
	int aspecth = (cinfo->aspect & 0xFFF0000) >> 16;

	int  framestep;
	int timestep = __vxgettimestep(cinfo, framestep);

	char stdchar = m_progressive ? 'p' : 'i';
	char szStandard[128] = { 0 };
	char szStandard1[128] = { 0 };

	if (m_rate%m_scale)
		sprintf(szStandard, "%dx%d %2.2f%c %d channels", w, h, (float)m_rate*timestep / m_scale, stdchar, cinfo->iochannels);
	else
		sprintf(szStandard, "%dx%d %2d%c %d channels", w, h, m_rate*timestep / m_scale, stdchar, cinfo->iochannels);
	sprintf(szStandard1, "%d:%d %dbits(OpenGL)", aspectw, aspecth, cinfo->fmt == FMT_YUV422P16 ? 16 : 8);


	int padtime = timestep*framestep - 1;
	__uint64 timemask = padtime;
	timemask = ~timemask;

    GLuint pbo = 0;
    if(m_shader_ytor)
    {
        glGenBuffers(1,&pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pbo);
        glBufferData(GL_PIXEL_UNPACK_BUFFER,m_nLiveW/2*m_nLiveH*4,NULL,GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
    }

	int livepitch = m_nLiveW * 2;
	BYTE* convbuf = (BYTE*)_vxmalloc(livepitch*m_nLiveH);

	while(!m_bExitLive)
	{
		if(vxWaitForSingleObject(hClock,10000)!=WAIT_OBJECT_0) continue;
		__uint64 ullSystemClock = GetSystemClockTime();
		

		int iAspect = m_aspect;
		if (m_iLiveMode == AFD_169_43_Scale || m_iLiveMode == AFD_169_43_Cut || m_iLiveMode == AFD_169_43_Envlp || m_iLiveMode == AFD_169_43_Spec)
		{
			iAspect = g_resinfo2[RES_576].aspect;
		}
		else if (m_iLiveMode == AFD_43_169_Scale || m_iLiveMode == AFD_43_169_Cut || m_iLiveMode == AFD_43_169_Envlp || m_iLiveMode == AFD_43_169_Spec)
		{
			iAspect = g_resinfo2[RES_1080].aspect;
		}

		if(ullSystemClock&padtime)
		{
			if(m_uidraw)
			{
				UIDRAW uid = {uidraw_idle,0,m_lastframe,m_rcFill.Width(),m_rcFill.Height(),iAspect,m_rate,m_scale,0,0,m_dc,TRUE};
				m_uidraw(&uid,m_uip);
			}
			continue;
		}


		VXBOOL bGet = FALSE;
		GLLIVEOUT_SURFACE liveout;
		GLLIVEOUT_SURFACE get;
		//取到时间最接近的一个做渲染。
		while(m_liveout.GetFront(&liveout))
		{
			if(liveout.ullTimeStamp<=ullSystemClock)
			{
				if (bGet)
                {
					get.surface->Release();
                    TRACE("liveout skip 1 frame(%d,%d)\n",liveout.ullTimeStamp,ullSystemClock);
                }
				get = liveout;
				bGet = TRUE;
				m_liveout.Pop();
			}
			else
				break;
		}

		if(!m_parentwnd)
		{
			if(bGet)
			{
				get.surface->Release();
				bGet = FALSE;
			}
		}

		if(bGet)
		{
			CVxTryLock lock(&m_csLiveRnd);
			if (!lock.m_bLock)
			{
				get.surface->Release();
				continue;
			}

			if(m_resize)
			{
#if __linux__
                glXMakeCurrent(m_xdy,0,NULL);
                __recreateglxwindow((Window)m_parentwnd,m_rcFill);
                XMapWindow (m_xdy,(Window)m_hWnd);
                glXMakeCurrent(m_xdy,m_xwin,m_glx);
#endif
				GLuint tex = m_srceen; 
				glGenTextures(1, &m_srceen);
				glBindTexture( GL_TEXTURE_2D,m_srceen);
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexImage2D( GL_TEXTURE_2D, 0,GL_RGBA,m_rcFill.Width(),m_rcFill.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				VX_CHECK_GLERROR();
				glBindTexture(GL_TEXTURE_2D,0);
				
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,m_srceen,0);
				ASSERT(vxgl::CheckFramebufferStatus());
				glViewport(0,0,m_rcFill.Width(),m_rcFill.Height());
				glClearColor(0.f,0.f,0.f,1.f);
				glClear(GL_COLOR_BUFFER_BIT);
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
				
				glDeleteTextures( 1, &tex );
				m_resize = FALSE;
			}
			
			VXSURFACE_DESC desc;
			get.surface->GetDesc(&desc);
			VXSURFACE_LOCK slock;
			get.surface->Lock(&slock);
			void* _srcbuf[4] = { 0 }; int _spitch[4] = { 0 };
			__getplanar(desc.fmt, slock.pLock, slock.pitch, desc.width, desc.height, _srcbuf, _spitch);

			BYTE* drawbuf = (BYTE*)_srcbuf[0];
			int drawpitch = _spitch[0];
			if (__liveconvert)
			{
				if(m_iDrawMode==1)
				{
					//上场
					__liveconvert((BYTE*)_srcbuf[0], desc.validw, desc.validh / 2, _spitch[0] * 2, convbuf, livepitch);
				}
				else
				{
					//下场
					__liveconvert((BYTE*)_srcbuf[0]+_spitch[0], desc.validw, desc.validh / 2, _spitch[0] * 2, convbuf, livepitch);
				}
				drawbuf = convbuf;
				drawpitch = livepitch;
			}
            else if(m_stdres&&!m_fullres)
            {
                drawpitch *=2;
            }
			get.surface->Unlock();

			if(m_shader_ytor)
			{
				VX_CHECK_GLERROR();
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_yuv2rgbFBO);
				VX_CHECK_GLERROR();
				glViewport(0,0,m_nLiveW,m_nLiveH);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glVertexPointer(2, GL_FLOAT,0,vecs); 
				glTexCoordPointer(2, GL_FLOAT,0,texs);  
//				glInterleavedArrays(GL_T2F_V3F,0,vecs);

				VX_CHECK_GLERROR();

				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, m_tex);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pbo);
				GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
				if(ptr)
				{
                    int dpitch = m_nLiveW*2;
                    int cpysize = vxmin(dpitch,drawpitch);
					cpypic(drawbuf,ptr,m_nLiveH,cpysize,drawpitch,dpitch);
					glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
				}
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,m_nLiveW/2,m_nLiveH,GL_RGBA, GL_UNSIGNED_BYTE,NULL);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

				m_shader_ytor->begin();
				m_shader_ytor->setUniform1i("lastStep",0);
				glDrawArrays(GL_QUADS,0,4);
				m_shader_ytor->end();

				VX_CHECK_GLERROR();

				glBindTexture(GL_TEXTURE_2D, m_rgbtex);
			}
			else
			{
				glEnable( GL_TEXTURE_2D );
				glBindTexture(GL_TEXTURE_2D, m_rgbtex);
				int spitch = m_nLiveW * 2;
				yuy2_to_bgra((void**)&drawbuf,&drawpitch,m_nLiveW, m_nLiveH,m_rgbbuf, m_nLiveW * 4);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,m_nLiveW,m_nLiveH,GL_BGRA, GL_UNSIGNED_BYTE,m_rgbbuf);
			}
			get.surface->Release();

			VX_CHECK_GLERROR();
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbo);
			VX_CHECK_GLERROR();

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            static float tempTexs[8] = {0};
            static float tempVecs[8] = {0};

			if (AFD_169_43_Cut == m_iLiveMode)//16：9---->4：3切边,修改纹理坐标。
			{
				static float texs[] = {	0.125f, 0.f,
					1.f -0.125f, 0.f,
					1.f-0.125f, 1.f,
					0.125f, 1.f};

                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs2,sizeof(vecs2));
			}
			else if (AFD_169_43_Envlp == m_iLiveMode)//16:9---->4:3信封，修改顶点坐标
			{
				static float vecs[] =  {	-1.f, 1.f -0.25f,
					1.f, 1.f -0.25f,
					1.f,-1.f + 0.25f,
					-1.f,-1.f + 0.25f, };

                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs,sizeof(vecs));
			}
			else if (AFD_169_43_Spec == m_iLiveMode)//16:9-->14:9-->4:3信封。裁剪两边纹理各0.0625，
			{
				static float vecs[] =  {	-1.f, 1.f -0.14286f,
					1.f, 1.f -0.14286f,
					1.f,-1.f + 0.14286f,
					-1.f,-1.f + 0.14286f, };

				static float texs[] = {	0.0625f, 0.f,
					1.f -0.0625f, 0.f,
					1.f-0.0625f, 1.f,
					0.0625f, 1.f};

                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs,sizeof(vecs));
			}

			else if (AFD_43_169_Cut == m_iLiveMode)//4:3--->16:9切边, 切上下边。
			{
				static float texs[] = {	0.f, 0.125f,
					1.f, 0.125f,
					1.f, 1.f - 0.125f,
					0.f, 1.f - 0.125f};

                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs2,sizeof(vecs2));
			}
			else if (AFD_43_169_Envlp == m_iLiveMode)//4:3--->16:9 信封。
			{
				static float vecs[] =  {	-1.f + 0.25f, 1.f,
					1.f - 0.25f, 1.f,
					1.f - 0.25f,-1.f,
					-1.f + 0.25f,-1.f, };
                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs,sizeof(vecs));
			}
			else if (AFD_43_169_Spec == m_iLiveMode)//4:3--->14:9--->16:9
			{
				static float vecs[] =  { -1.f + 0.125f, 1.f,
					1.f - 0.125f, 1.f,
					1.f - 0.125f,-1.f,
					-1.f + 0.125f,-1.f, };

				static float texs[] = {	0.f, 0.f + 0.071428f,
					1.f, 0.f + 0.071428f,
					1.f, 1.f - 0.071428f,
					0.f, 1.f - 0.071428f};

                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs,sizeof(vecs));
			}
			else //不做修改。
			{
                memcpy(tempTexs,texs,sizeof(texs));
                memcpy(tempVecs,vecs2,sizeof(vecs2));
			}


			VX_CHECK_GLERROR();

			int ww = m_rcFill.Width(),wh = m_rcFill.Height();
			glViewport(0,0,ww,wh);

			//清理颜色、是为了让AFD显示的时候、不残留以前换边的边缘。
			glClearColor(0.f,0.f,0.f,1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
//			m_shader_gamma->begin();
//			m_shader_ytor->setUniform1i("lastStep",0);

            if (m_3dmode==3)
            {
                float leftTexs[8] = {(float)tempTexs[0],(float)tempTexs[1],
                    (float)(tempTexs[0]+(tempTexs[2]-tempTexs[0])*0.5),(float)tempTexs[3],
                    (float)(tempTexs[6]+(tempTexs[4]-tempTexs[6])*0.5),(float)tempTexs[5],
                    (float)tempTexs[6],(float)tempTexs[7]
                };
                float rightTexs[8] = {(float)(tempTexs[0]+(tempTexs[2]-tempTexs[0])*0.5),(float)tempTexs[3],
                    (float)tempTexs[2],(float)tempTexs[3],
                    (float)tempTexs[4],(float)tempTexs[5],
                    (float)(tempTexs[6]+(tempTexs[4]-tempTexs[6])*0.5),(float)tempTexs[7],
                };
                glVertexPointer(2, GL_FLOAT,0,tempVecs); 
                glTexCoordPointer(2, GL_FLOAT,0,leftTexs);
                glDrawArrays(GL_QUADS,0,4);

                glBlendColor(0.5,0.5,0.5,1.0);
                glBlendFunc(GL_CONSTANT_COLOR,GL_CONSTANT_COLOR);
                glEnable(GL_BLEND);
                glVertexPointer(2, GL_FLOAT,0,tempVecs); 
                glTexCoordPointer(2, GL_FLOAT,0,rightTexs);
                glDrawArrays(GL_QUADS,0,4);
                glDisable(GL_BLEND);
            }
            else
            {
                glVertexPointer(2, GL_FLOAT,0,tempVecs); 
                glTexCoordPointer(2, GL_FLOAT,0,tempTexs);
                glDrawArrays(GL_QUADS,0,4);
            }

//			m_shader_gamma->end();
			if(m_maskData)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glBindTexture(GL_TEXTURE_2D,m_masktex);
				glDrawArrays(GL_QUADS,0,4);
				glDisable(GL_BLEND);
			}

			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_2D);

			VX_CHECK_GLERROR();

            if(!IsMesaGraphic())
                DrawVideoFlag(get.vf);

			m_lastframe = liveout.llFrame;
			VXBOOL isseek = fabs(liveout.speed)<0.01f;

			if (m_alone&&m_saftrect)
			{
				DrawSaftRect();
			}
			else if(m_uidraw)
			{
				UIDRAW uid = {uidraw_draw,liveout.ullTimeStamp,m_lastframe,ww,wh,iAspect,m_rate,m_scale,0,0,m_dc,isseek};
				m_uidraw(&uid,m_uip);
			}

			if (m_ftFont)
			{
				if(!isseek&&(liveout.speed!=1.f))
				{
					glColor4f(0.8f,0.30f,0.f,1.f);
					glRasterPos2f(0.70f,0.9f);
					char szSpeed[64] = {0};
					sprintf(szSpeed,"%1.2fx(%2.2ffps)",liveout.speed,liveout.speed*m_rate/m_scale);
					ftglRenderFont(m_ftFont,szSpeed,RENDER_ALL);
				}

				if (m_drawinfo)
				{
					glColor4f(0.5f, 0.80f, 0.f, 1.f);
					glRasterPos2f(-0.88f, 0.9f - 8.f / wh - 13.f / wh);
					ftglRenderFont(m_ftFont, szStandard, RENDER_ALL);
					glRasterPos2f(-0.88f, 0.9f - 8.f / wh - 13.f * 3 / wh);
					ftglRenderFont(m_ftFont, szStandard1, RENDER_ALL);
				}
			}


			VX_CHECK_GLERROR();

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			VX_CHECK_GLERROR();

			glViewport(0,0,ww,wh);

			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D,m_srceen);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(2, GL_FLOAT,0,vecs); 
			glTexCoordPointer(2, GL_FLOAT,0,texs);  
			VX_CHECK_GLERROR();
			glDrawArrays(GL_QUADS,0,4);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			glBindTexture( GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_2D);
            glFlush();
#ifdef _WIN32
			SwapBuffers(hdc);
#elif __APPLE__
			CGLFlushDrawable(m_cglrc);
#else
            glXSwapBuffers(m_xdy,m_xwin);
#endif
			VX_CHECK_GLERROR();
		}
		else if(m_uidraw)
		{
			UIDRAW uid = {uidraw_idle,0,m_lastframe,m_rcFill.Width(),m_rcFill.Height(),iAspect,m_rate,m_scale,0,0,m_dc,TRUE};
			m_uidraw(&uid,m_uip);
		}
	}

	_vxfree(convbuf);

	if(m_uidraw)
	{
		UIDRAW uid = {uidraw_reset};
		m_uidraw(&uid,m_uip);
	}

    if(pbo)
        glDeleteBuffers(1,&pbo);

	glFinish();

#ifdef _WIN32
	wglMakeCurrent (0, 0) ;		
	ReleaseDC(m_hWnd,hdc);
#elif __APPLE__
	CGLSetCurrentContext(NULL);
#else
    glXMakeCurrent(m_xdy,0,NULL);
#endif
	m_clock->CloseClockEvent(hClock);
}


VXBOOL CVxGLVidPlayer::GetImage(void*& buf,int& w,int& h)
{
	CVxLock lock(&m_csLiveRnd);

	TRACE("[0x%08x]::GetImage\n",(vxuintptr)m_hWnd);

#ifdef _WIN32
	if(!m_hglrcP) return FALSE;
	HGLRC lastrc = wglGetCurrentContext();HDC lastdc = NULL;
	if(lastrc!=m_hglrcP)
	{
		lastdc = wglGetCurrentDC();
		wglMakeCurrent(m_hDC,m_hglrcP);
	}
#elif __APPLE__
    if(!m_cglrcP) return NULL;
    CGLContextObj lastcgl = CGLGetCurrentContext();
    if(lastcgl!=m_cglrcP)
        CGLSetCurrentContext(m_cglrcP);
#else
    if(!m_glxP) return NULL;
    GLXContext lastglx = glXGetCurrentContext();GLXDrawable lastdraw = NULL;
    if(lastglx!=m_glxP)
    {
        lastdraw = glXGetCurrentDrawable();
        glXMakeCurrent(m_xdy,m_xwin,m_glxP);
    }
#endif
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	w = m_rcFill.Width();
	h = m_rcFill.Height();
	buf = _vxmalloc(w*h*4);
	glReadPixels(0,0,w,h,GL_BGRA,GL_UNSIGNED_BYTE,buf);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

	glFinish();

#ifdef _WIN32
	if(lastrc!=m_hglrcP)
		wglMakeCurrent(lastdc,lastrc);
#elif  __APPLE__
    if(lastcgl!=m_cglrcP)
        CGLSetCurrentContext(lastcgl);
#else
    if(lastglx!=m_glxP)
        glXMakeCurrent(m_xdy,lastdraw,lastglx);
#endif
    return TRUE;
}
void CVxGLVidPlayer::RegisterDrawUIFunc(UIDRAWFUNC func,void* p)
{
	if (m_alone) return;
	m_uidraw = func;m_uip = p;
}

#ifdef _WIN32
#include "windowsx.h"
WNDPROC oldoglwndproc = NULL;LONG_PTR olduserdata = NULL;
LRESULT CALLBACK OGLWndProc(
							 HWND hwnd,        // handle to window
							 UINT uMsg,        // message identifier
							 WPARAM wParam,    // first message parameter
							 LPARAM lParam)    // second message parameter
{ 
	CVxGLVidPlayer* glplayer = (CVxGLVidPlayer*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	if(uMsg>=WM_MOUSEFIRST&&uMsg<=WM_MOUSELAST)
	{
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 
		HWND parent = GetParent(hwnd);
		if(parent)
		{
			POINT cc = {xPos,yPos};
			ClientToScreen(hwnd,&cc);
			ScreenToClient(parent,&cc);
			LPARAM parentpos = MAKELONG(cc.x,cc.y);
			PostMessage(parent,uMsg,wParam,parentpos);
		}
		if(uMsg==WM_LBUTTONDOWN||uMsg==WM_RBUTTONDOWN)
			return glplayer->__winmessage(uMsg,xPos,yPos)?0:DefWindowProc(hwnd,uMsg,wParam,lParam);
		else
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	else if(uMsg==WM_PAINT)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd,&ps);
		glplayer->__OnPaint(hdc);
		EndPaint(hwnd,&ps);
		return 1;
	}
	else 
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
} 

extern HINSTANCE g_inst;

VXBOOL CVxGLVidPlayer::CreateDrawWindow()
{
	WNDCLASS wc = {CS_DBLCLKS};
	wc.lpszClassName = "JetsenOpenGLWnd";
	wc.lpfnWndProc = OGLWndProc;
//	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = g_inst;
	::RegisterClass(&wc);
	m_hWnd = CreateWindowEx(0,wc.lpszClassName,"Jetsen OpenGL",WS_CHILD,0,0,m_nLiveW,m_nLiveH,m_setup->GetMainWnd(),0,g_inst,NULL);
	if(!m_hWnd) return FALSE;

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA,(LONG_PTR)this);

	return TRUE;
}

void CVxGLVidPlayer::DestroyDrawWindow()
{
	if(m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	::UnregisterClass("JetsenOpenGLWnd",g_inst);
}

VXBOOL CVxGLVidPlayer::__winmessage(UINT uMsg,int xpos,int ypos)
{
	if(!m_uidraw) return FALSE;
	if(uMsg==WM_LBUTTONDOWN)
	{
		UIDRAW uid = {uidraw_hit,uihit_dragbegin,m_lastframe,m_rcFill.Width(),m_rcFill.Height(),m_aspect,m_rate,m_scale,xpos,ypos,m_dc};
		if(m_uidraw(&uid,m_uip)==0)
		{
			::SetCapture(m_hWnd);
			MSG msg;
			while( ::GetMessage( &msg,NULL,NULL,NULL ) )
			{
				if( msg.message < WM_MOUSEFIRST || msg.message > WM_MOUSELAST )
				{
					::TranslateMessage( &msg );
					::DispatchMessage(	&msg );
				}
				if( msg.message == WM_LBUTTONUP )
				{				
					::ReleaseCapture();
				}
				if( msg.message == WM_MOUSEMOVE )
				{
					uid.timestamp = uihit_dragto;
					uid.x = (short)GET_X_LPARAM( msg.lParam );
					uid.y = (short)GET_Y_LPARAM( msg.lParam );
					m_uidraw(&uid,m_uip);
				}
				if( ::GetCapture() != m_hWnd )
					break;
			}
			uid.timestamp = uihit_dragend;
			uid.isseek = TRUE;
			m_uidraw(&uid,m_uip);
			return TRUE;
		}
		else
			return FALSE;
	}
	else if(uMsg==WM_RBUTTONDOWN)
	{
		UIDRAW uid = {uidraw_hit,uihit_rbhit,m_lastframe,m_rcFill.Width(),m_rcFill.Height(),m_aspect,m_rate,m_scale,xpos,ypos,m_dc};
		m_uidraw(&uid,m_uip);
		return TRUE;
	}
	else
		return FALSE;
}
#elif __linux__
VXBOOL CVxGLVidPlayer::CreateDrawWindow()
{
    return TRUE;
}

void CVxGLVidPlayer::DestroyDrawWindow()
{

}

#endif




#define HAS_OBJECTS		1		//本模块可以创建对象的总数

static LONG __cdecl _CreateOpenGLPlayer(IVxObject* setup,IVxObject* param,vxuintptr param2,vxuintptr param3,IVxObject** obj)
{
	CVxGLVidPlayer* pObj = new CVxGLVidPlayer((IVxSystemSetup*)setup,(IVxSystemClock*)param,(int)param2, (int)param3,FALSE);
	if(!pObj->Initialize())
	{
		pObj->Uninitialize();
		delete pObj;
		return -1;
	}
	return GetVxInterface(static_cast<IVxLiveExt*>(pObj),(void**)obj);
}

static LONG __cdecl _CreateOpenGLPlayer2(IVxObject* setup,IVxObject* param,vxuintptr param2,vxuintptr param3,IVxObject** obj)
{
	CVxGLVidPlayer* pObj = new CVxGLVidPlayer((IVxSystemSetup*)setup,(IVxSystemClock*)param, (int)param2, (int)param3,TRUE);
	if(!pObj->Initialize())
	{
		pObj->Uninitialize();
		delete pObj;
		return -1;
	}
	return GetVxInterface(static_cast<IVxLiveExt*>(pObj),(void**)obj);
}

#pragma warning(disable:4996)
extern "C" __declspec( dllexport ) LONG vxGetObjects(VXOBJECT* vxObj, LONG flags)
{
	if(flags==0)
	{
		vxObj->dwObjID		= 0;
		vxObj->dwObjType	= vxObjVideoLive;
		vxObj->type_count	= HAS_OBJECTS;
		vxObj->types = (DWORD*)_vxmalloc(sizeof(DWORD)*HAS_OBJECTS);
		vxObj->types[0] = vxVideoLive_gl;
		strcpy(vxObj->szName, "OpenGL Video Player");
		vxObj->pfCreate		= _CreateOpenGLPlayer;
		return 1;
	}
	else
	{
		vxObj->dwObjID		= 100;
		vxObj->dwObjType	= vxObjVideoLive;
		vxObj->type_count	= HAS_OBJECTS;
		vxObj->types = (DWORD*)_vxmalloc(sizeof(DWORD)*HAS_OBJECTS);
		vxObj->types[0] = vxVideoLive_gl2;
		strcpy(vxObj->szName, "OpenGL Video Player2");
		vxObj->pfCreate		= _CreateOpenGLPlayer2;
		return 0;
	}
}
