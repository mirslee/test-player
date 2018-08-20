
#ifndef __MXCONFIG_H__
#define __MXCONFIG_H__

#ifdef _WIN32
#ifdef MXGLTOOL_EXPORTS
#define MXGLTOOL_API __declspec(dllexport)
#define MXCORE_API __declspec(dllexport)
#else
#define MXGLTOOL_API __declspec(dllimport)
#define MXCORE_API __declspec(dllimport)
#endif
#else
#define MXGLTOOL_API
#define MXCORE_API
#endif


#endif//__MXCONFIG_H__
