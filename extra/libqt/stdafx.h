// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifdef _WIN32
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯��������ʽ��
#define VC_EXTRALEAN		// �� Windows ͷ���ų�����ʹ�õ�����

#include <afx.h>
#include <afxwin.h>         // MFC ��������ͱ�׼���

#else

#include "vxconfig.h"

#endif

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
