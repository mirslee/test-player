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
	unsigned int shaderProgram; //�������
	unsigned int VAO;//��������id
	unsigned int VBO; //������id
	unsigned int VAO2;//������������id
	unsigned int VBO2; //������id
	unsigned int EBO;//����������
};

