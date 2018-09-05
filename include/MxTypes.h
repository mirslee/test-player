
#ifndef __MXTYPES_H__
#define __MXTYPES_H__

#ifdef _WIN32
#include <windows.h>
#endif

typedef char int8;
#ifndef _WIN32
typedef char _int8;
typedef char __int8;
#endif
typedef unsigned char uint8;
typedef unsigned char _uint8;
typedef unsigned char __uint8;

typedef short int16;
#ifndef _WIN32
typedef short _int16;
typedef short __int16;
#endif
typedef unsigned short uint16;
typedef unsigned short _uint16;
typedef unsigned short __uint16;

typedef long int32;
#ifndef _WIN32
typedef long _int32;
typedef long __int32;
#endif
typedef unsigned long uint32;
typedef unsigned long _uint32;
typedef unsigned long __uint32;

typedef unsigned int uint;
typedef long long int64;
#ifndef _WIN32
typedef long long _int64;
typedef long long __int64;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define TRUE true
#define FALSE false
#endif
typedef unsigned long long uint64;
typedef unsigned long long _uint64;
typedef unsigned long long __uint64;

typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long ulong;

//typedef void* mxuvoidptr;

template <int> struct MxIntegerSize;
template <> struct MxIntegerSize<4> {typedef long IntegerSize;  typedef unsigned long UIntegerSize;};
template <> struct MxIntegerSize<8> {typedef long long IntegerSize; typedef unsigned long long UIntegerSize;};
typedef MxIntegerSize<sizeof(void*)>::IntegerSize mxvoidptr;
typedef MxIntegerSize<sizeof(void*)>::UIntegerSize mxuvoidptr;

/*template <int> struct MxIntegerSize;
template <>    struct MxIntegerSize<1> { typedef BYTE  UIntegerSize; typedef char  IntegerSize; };
template <>    struct MxIntegerSize<2> { typedef WORD UIntegerSize; typedef short IntegerSize; };
template <>    struct MxIntegerSize<4> { typedef DWORD UIntegerSize; typedef int IntegerSize; };
template <>    struct MxIntegerSize<8> { typedef __uint64 UIntegerSize; typedef __int64 IntegerSize; };
template <class T> struct MxIntegerSizeOf : MxIntegerSize<(int)sizeof(T)> { };
typedef MxIntegerSizeOf<void*>::IntegerSize mxvoidptr;
typedef MxIntegerSizeOf<void*>::UIntegerSize mxuvoidptr;*/

#ifndef _WIN32
#define HANDLE void*
#endif

//	��Ϣ��Դ������ֽڣ�ģ��
#define		MAILSRC_UNKNOWN			0x00000000						//	��Դ����Ҫ
#define		MAILSRC_HARDHEL			0x01000000						//	��Դ������
#define		MAILSRC_HWENGINE		(MAILSRC_HARDHEL+0x00000000)	//	Ӳ����������
#define		MAILSRC_GETHER			(MAILSRC_HARDHEL+0x01000000)	//	Ӳ���ɼ�����
#define		MAILSRC_SWENGINE		(MAILSRC_HARDHEL+0x02000000)	//	�����������
#define		MAILSRC_DATACORE		0x21000000						//	���ݺ��ĳ���
#define		MAILSRC_CONFIGLV		0x22000000						//	���ݺ��Ĳ�֮�µĻ������ò�
#define		MAILSRC_UIVIEW			0x41000000						//	��Ҫ�����
#define		MAILSRC_UIPANEL			0x61000000						//	���
//	��Ϣ��Դ�Ĵ�����ֽڣ�����
#define		MAILSRC_ERROR			0x00010000						//	ϵͳ�������ش���������˳�
#define		MAILSRC_FAILED			0x00020000						//	ϵͳ���ֿɻָ�����
#define		MAILSRC_WARNING			0x00030000						//	ϵͳ���ֲ�̫���ش�����ô����˳�
#define		MAILSRC_MESSAGE			0x00040000						//	ϵͳ������Ϣ
#define		MAILSRC_LOG				0x00050000						//	����һ����¼��Ϣ����logģʽ�»�д���¼�ļ�
#define     MAILSRC_MASK			0x000f0000	

#endif //__MXTYPES_H__
