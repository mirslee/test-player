
#ifdef MXGLTOOL_EXPORTS
#define MXGLTOOL_API __declspec(dllexport)
#else
#define MXGLTOOL_API __declspec(dllimport)
#endif

// �����Ǵ� MxGLTool.dll ������
class MXGLTOOL_API CMxGLTool {
public:
	CMxGLTool(void);
	// TODO:  �ڴ�������ķ�����
};

extern MXGLTOOL_API int nMxGLTool;

MXGLTOOL_API int fnMxGLTool(void);
