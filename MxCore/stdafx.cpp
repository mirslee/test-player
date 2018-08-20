//
//  stdafx.h
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/14.
//
#include "stdafx.h"

#ifdef _WIN32
#ifdef _DEBUG

#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "opengl32.lib")
#else
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "pthreadVC2.lib")
#endif
#endif

