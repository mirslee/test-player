// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#ifndef __LIBQT2_STDAFX_H__
#define __LIBQT2_STDAFX_H__

#ifdef _WIN32

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料

#include "windows.h"

#else

#include "inttypes.h"
#if __linux__
    #define INT32_MAX		(2147483647)
#endif

#endif

#include "vxconfig.h"

// TODO: 在此处引用程序需要的其他头文件

#endif
