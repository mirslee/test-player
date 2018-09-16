#ifndef stdafx_h
#define stdafx_h

#ifdef _WIN32
#define MXCORE_EXPORT
#endif

#include "MxDllexport.h"
#include "MxInterface.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include "stdint.h"
#include "GL/glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#define HAVE_STRUCT_TIMESPEC
#include "win/pthread.h"
#endif

//std
#include <string>
#include <vector>
#include <queue>
#include <iostream>
using namespace std;





#endif /* stdafx_h */
