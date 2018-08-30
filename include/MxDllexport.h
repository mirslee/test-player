
#ifndef __MXDLLEXPORT_H__
#define __MXDLLEXPORT_H__

#ifdef _WIN32

#ifdef MXCORE_EXPORT
#define MXCORE_API __declspec(dllexport)
#else
#define MXCORE_API __declspec(dllimport)
#endif //MXCORE_EXPORT

#ifdef MXGLTOOL_EXPORT
#define MXGLTOOL_API __declspec(dllexport)
#else
#define MXGLTOOL_API __declspec(dllimport)
#endif //MXGLTOOL_EXPORT

#ifdef MXPLAYER_EXPORT
#define MXPLAER_API __declspec(dllexport)
#else
#define MXPLAER_API __declspec(dllimport)
#endif //MXPLAYER_EXPORT

#ifdef MXCODEC_EXPORT
#define MXCODEC_API __declspec(dllexport)
#else
#define MXCODEC_API __declspec(dllimport)
#endif //MXCODEC_EXPORT

#else
#define MXCORE_API
#define MXGLTOOL_API
#define MXPLAER_API
#define MXCODEC_API
#endif //_WIN32



#endif//__MXDLLEXPORT_H__
