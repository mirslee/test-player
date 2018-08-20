
#ifndef __DLLEXPORT_H__
#define __DLLEXPORT_H__

#ifdef _WIN32
#ifdef MXDLL_EXPORTS
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


#endif//__DLLEXPORT_H__
