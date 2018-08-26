//
//  stdafx.h
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/14.
//

//windows



#ifndef stdafx_h
#define stdafx_h

#define MXPLAER_API __declspec(dllexport)
#include <assert.h>
#include "dllexport.h"
#ifdef _WIN32

#include <windows.h>
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

	//ffmpeg
#ifdef __cplusplus
	extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"

#ifdef __cplusplus
	}
#endif



//std
#include <string>
#include <vector>
#include <queue>
#include <iostream>
using namespace std;



#include "mxConfig.h"




#endif /* stdafx_h */
