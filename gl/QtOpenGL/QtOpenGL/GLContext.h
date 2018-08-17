#pragma once

#include <windows.h>
#include <gl/GL.h>

class GLContext
{
public:
	GLContext();
	~GLContext();

public:
	bool	setup(HWND hWnd, HDC hDC);
	void    shutdown();
	void    swapBuffer();

private:
	int		format;
	HWND	hWnd;
	HDC		hDc;
	HGLRC	hRc;
};

