#pragma once
#include "vxbase.h"
#include "vxsystemsetupif.h"
#include "vxuitools.h"

#include "vxgl/glew.h"


#ifdef _WIN32
#include <gl/gl.h>
#include <gl/glu.h>
#include <vxgl/glext.h>
#elif __APPLE__
#include <OpenGL/OpenGL.h>
#include <opengl/gl.h>
#include <opengl/glu.h>
#include <opengl/glext.h>
#include <OpenGL/CGLTypes.h>
#elif __linux__
#include <vxgl/glxew.h>
#endif


#include "vxgl/glsl.h"
#include "FTGL/ftgl.h"
#include "FTGL/FTGLBitmapFont.h"

using namespace vxgl;
using namespace FTGL;

#define DX9_HDV_2160ILIVE_W 1920
#define DX9_2160ILIVE_W		2048
#define DX9_2160ILIVE_H		1080

#define DX9_HDV_1080ILIVE_W 720
#define DX9_1080ILIVE_W		960
#define DX9_1080ILIVE_H		540

#define DX9_720PLIVE_W		640
#define DX9_720PLIVE_H		360


typedef struct  
{
	__int64 llFrame;
	__uint64 ullTimeStamp;
	IVxSurface* surface;
	vxvideoflag vf;
	float speed;
}GLLIVEOUT_SURFACE;

typedef void (*PLIVECONVERT)( BYTE* source,int ipitch,int w,int h,BYTE* target,int opitch);

#define COLORFILL_LINES	8

#define DX9V_MIPMAPLEVERS 	4
#define DX9VOUTPUT_COUNTS	32

vxinterface IVxDataCore;

class CVxGLVidPlayer : public CVxObject, public IVxLiveExt3,public IVxLiveCliper
{
public:
	CVxGLVidPlayer(IVxSystemSetup* setup,IVxSystemClock* clock,int w,int h,VXBOOL alone);
	virtual ~CVxGLVidPlayer(void);
protected:
	CVxComPtr<IVxSystemSetup> m_setup;
	CVxComPtr<IVxSystemClock> m_clock;
	CVxComPtr<IVxVidMemPool> m_pool;
	VXBOOL m_alone;
	HVXWND m_hWnd;
	HVXWND m_parentwnd;
	CVxRect m_rcFill;
	int m_width,m_height;
	int m_aspect;
	int m_rate,m_scale;
	VXBOOL m_progressive;

	VXBOOL m_stdres;
	VXSURFACE_RES m_res;
	VXTHREAD_MUTEX m_csLive;
	VXTHREAD_MUTEX m_csLiveRnd;

	FTGLfont* m_ftFont;
	UIDRAWFUNC m_uidraw;
	void* m_uip;
	__int64 m_lastframe;


	CVxComPtr<IVxDataCore> m_dc;
	void(*yuy2_to_bgra)(void* src[4], int ipitch[4],int cx, int cy, void* dst, int opitch);
public:
	VXBOOL Initialize();
	void Uninitialize();
public:
	DECLARE_IVXOBJECT
	VXBOOL __stdcall SetHwnd(HVXWND hWnd,LPVXRECT lpRect);
	VXBOOL __stdcall OnPaint(){return TRUE;}

	VXBOOL __stdcall GetImage(void*& buf,int& w,int& h);

	void __stdcall Liveoutput(__int64 llFrame, __uint64 timestamp, IVxSurface* sf, VXBOOL bSeek, vxvideoflag vf, float speed);
	VXBOOL __stdcall LockLiveIn(LIVE_IN* live);
	void __stdcall UnlockLiveIn(__uint64 ullSystemClock, LIVE_IN* live, int validw, int validh);

	void __stdcall RegisterDrawUIFunc(UIDRAWFUNC func,void* p);
    void __stdcall SwitchDataCore(IVxDataCore* dc,IVxDataCore** olddc);

	VXBOOL __stdcall Is3D(){return m_3dmode>=0;}
	int __stdcall Get3DMode(){return m_3dmode;}
	VXBOOL __stdcall Set3DMode(int mode);

	VXBOOL __stdcall Reset(IVxSystemClock* clock,int w,int h);
	void __stdcall SetVidMemPool(IVxVidMemPool* pool) { CVxLock lockLive(&m_csLive); m_pool = pool; }

	void __stdcall SetCliper(float aspect,float ubratio);
	void __stdcall SetCliper(float left,float top,float right,float bottom);
	void __stdcall SetDrawExt(DWORD type,int enable);
	int __stdcall GetDrawExt(DWORD type);
	void   __stdcall SetLiveMode(int iMode);	


	LONG __stdcall NonDelegatingQueryInterface(LONG iid, void** ppObj);
protected:
	void OnDelete(){Uninitialize();}

	VXBOOL CreateBuffers();
	void DestroyBuffers();

	VXBOOL CreateDrawWindow();
	void DestroyDrawWindow();

	VXBOOL CreateOpenGL();
	void DestroyOpenGL();
	
	void DrawVideoFlag(vxvideoflag vf);
	void DrawSaftRect();
	GLuint m_vflist;
	void GenVFList();
protected:
	VXBOOL m_resize;
#ifdef _WIN32
	HGLRC m_hglrc;
	HGLRC m_hglrcP;
	HDC m_hDC;
#elif __APPLE__
	CGLContextObj m_cglrc;
	CGLContextObj m_cglrcP;
	EventHandlerRef	m_handleref;
	EventHandlerUPP m_EvtHandler;			// main event handler
	VXBOOL m_uidrag;
#elif __linux__
    GLXContext m_glx;
    GLXContext m_glxP;
    Display* m_xdy;
    GLXFBConfig *m_fbc;
    Colormap m_cmap;
    GLXWindow m_xwin;
#endif

	GLuint m_fbo;
	GLuint m_srceen;

	GLuint m_tex;
	GLuint m_rgbtex;
	GLuint m_yuv2rgbFBO;
	ShaderType m_stype;
	vxgl::glShader *m_shader_ytor;
	vxgl::glShader *m_shader_gamma;

	BYTE* m_rgbbuf;

	PLIVECONVERT __liveconvert;
	CVxQueue<GLLIVEOUT_SURFACE> m_liveout;
	int m_nLiveW,m_nLiveH;
	int m_texW,m_texH;
	float m_livetx,m_livety;

	VXBOOL m_bExitLive;
	pthread_t m_hLiveThread;
	static void* LiveThreadProc(LPVOID lpThis){((CVxGLVidPlayer*)lpThis)->_LiveThread();return 0;}
	void _LiveThread();

	int m_3dmode;

	GLuint m_masktex;
	void* m_maskData;

	int m_iLiveMode;//高标清AFD转换预览模式。
	int m_iDrawMode; //1、上场；2、下场
	VXBOOL m_fullres;
	VXBOOL m_saftrect;
	VXBOOL m_drawinfo;

	int m_deitype;
	CVxComPtr<IVxVideoFilter> m_deinterlace;
	vxenc_image m_deiyuv422p;
	struct DEIINFO
	{
		__int64 frame;
		__uint64 tc;
		vxvideoflag vf;
		float speed;
	};
	CVxQueue<DEIINFO> m_qTC;
	static VXBOOL FilterOut(void* p, const vxenc_image* img) { return ((CVxGLVidPlayer*)p)->__FilterOut(img); }
	VXBOOL __FilterOut(const vxenc_image* img);

public:

#ifdef _WIN32
	void __OnPaint(HDC hdc);
	VXBOOL __winmessage(UINT uMsg,int xpos,int ypos);
#elif __APPLE__
	void __Unhide();
	VXBOOL __macmessage(void* event,DWORD type);
#elif __linux__
    VXBOOL __recreateglxwindow(Window parent,VXRECT* rc);
#endif

};
