// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#ifndef __LIBQT2_STDAFX_H__
#define __LIBQT2_STDAFX_H__

#ifdef _WIN32

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����

#include "windows.h"

#else

#include "inttypes.h"
#if __linux__
    #define INT32_MAX		(2147483647)
#endif

#endif

#include "vxconfig.h"

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�

#endif
