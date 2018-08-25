//
//  stdafx.h
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/14.
//

#ifdef _WIN32
#define MXDLL_EXPORTS
#endif

#include "dllexport.h"
//#include "mxtypes.h"
//windows
#ifdef _WIN32

	#include "windows.h"
	#include "stdint.h"
	#include "GL/glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>

//#include <vxgl/glext.h>

	

#define HAVE_STRUCT_TIMESPEC
#include "win/pthread.h"

	#ifdef __cplusplus
	extern "C" {
	#endif

	#ifdef __cplusplus
	}
	#endif

#endif

#ifndef stdafx_h
#define stdafx_h



//std
#include <string>
#include <vector>
#include <queue>
#include <iostream>
using namespace std;





#endif /* stdafx_h */
