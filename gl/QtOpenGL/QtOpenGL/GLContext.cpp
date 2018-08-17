#include "GLContext.h"



GLContext::GLContext()
{
	format = 0;
	hWnd = nullptr;
	hDc = nullptr;
	hRc = nullptr;
}


GLContext::~GLContext()
{
	shutdown();
}

bool GLContext::setup(HWND hWnd, HDC hDC)
{
	this->hWnd = hWnd;
	this->hDc = hDC;

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	if (format == 0)
	{
		format = ChoosePixelFormat(hDc,&pfd);
	}

	if (!SetPixelFormat(hDc, format, &pfd))
	{
		return false;
	}

	hRc = wglCreateContext(hDc);

	if (!wglMakeCurrent(hDc,hRc))
	{
		return false;
	}

	return true;
}

void GLContext::shutdown()
{
	if (hRc)
	{
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hRc);
	}

	if (hDc)
	{
		ReleaseDC(hWnd,hDc);
		hDc = nullptr;
	}
}

void GLContext::swapBuffer()
{
	SwapBuffers(hDc);
}
