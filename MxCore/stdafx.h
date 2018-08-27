#ifndef stdafx_h
#define stdafx_h

#ifdef _WIN32
#define MXCORE_EXPORT
#endif

#include "MxDllexport.h"
#include "MxInterface.h"

#ifdef _WIN32
#include "windows.h"
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
