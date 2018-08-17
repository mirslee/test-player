#pragma once
class HelloTriangles
{
public:
	HelloTriangles();
	~HelloTriangles();
	

	void prepare();
	void flush();
	void release();

private:
	unsigned int shaderProgram; //程序对象
	unsigned int VAO;//顶点数组id
	unsigned int VBO; //缓冲区id
	unsigned int VAO2;//索引顶点数组id
	unsigned int VBO2; //缓冲区id
	unsigned int EBO;//索引缓冲区
};

