
#ifdef MXGLTOOL_EXPORTS
#define MXGLTOOL_API __declspec(dllexport)
#else
#define MXGLTOOL_API __declspec(dllimport)
#endif

// 此类是从 MxGLTool.dll 导出的
class MXGLTOOL_API CMxGLTool {
public:
	CMxGLTool(void);
	// TODO:  在此添加您的方法。
};

extern MXGLTOOL_API int nMxGLTool;

MXGLTOOL_API int fnMxGLTool(void);
