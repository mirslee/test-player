// MxGLTool.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "MxGLTool.h"


// ���ǵ���������һ��ʾ��
MXGLTOOL_API int nMxGLTool=0;

// ���ǵ���������һ��ʾ����
MXGLTOOL_API int fnMxGLTool(void)
{
	GLint status;
	glValidateProgram(status);

    return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� MxGLTool.h
CMxGLTool::CMxGLTool()
{
    return;
}
