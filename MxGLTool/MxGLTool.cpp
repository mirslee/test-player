// MxGLTool.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MxGLTool.h"


// 这是导出变量的一个示例
MXGLTOOL_API int nMxGLTool=0;

// 这是导出函数的一个示例。
MXGLTOOL_API int fnMxGLTool(void)
{
	GLint status;
	glValidateProgram(status);

    return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 MxGLTool.h
CMxGLTool::CMxGLTool()
{
    return;
}
