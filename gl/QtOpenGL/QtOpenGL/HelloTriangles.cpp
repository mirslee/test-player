
#include "HelloTriangles.h"
#include <iostream>
#include "glew/glew.h"

HelloTriangles::HelloTriangles()
{
}

HelloTriangles::~HelloTriangles()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO);
}


void HelloTriangles::prepare()
{
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};

	glGenVertexArrays(1, &VAO);//������������
	glBindVertexArray(VAO);//��VAO

	glGenBuffers(1, &VBO); //�����������
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //�ѻ���󶨵�GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //�������ݸ��Ƶ�������

	//���Ӷ�������
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);//���ö������ԣ�//registered VBO as the vertex attribute's bound vertex buffer object
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);//glVertexAttribPointer֮�����

	//�����������
	float vertices2[] = {
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	glGenBuffers(1, &VBO2); //�����������
	glBindBuffer(GL_ARRAY_BUFFER, VBO2); //�ѻ���󶨵�GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW); //�������ݸ��Ƶ�������

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);//glVertexAttribPointer֮�����
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//glVertexAttribPointer֮�����

	//������ɫ��
	const char* vertexShaderSouce = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0);\n"
		"}\0";

	unsigned int vertexShader;//��ɫ��id
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//������ɫ������
	glShaderSource(vertexShader, 1, &vertexShaderSouce, nullptr);//��ɫ��Դ�����ɫ��
	glCompileShader(vertexShader);//������ɫ��

	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//��������ɫ���Ƿ�ɹ�
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);//��ȡʧ����Ϣ
		std::cout << "������ɫ��ʧ��: " << infoLog << std::endl;
	}

	//Ƭ����ɫ��
	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 fragColor;\n"
		"void main()\n"
		"{\n"
		"	fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);//��������ɫ���Ƿ�ɹ�
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);//��ȡʧ����Ϣ
		std::cout << "������ɫ��ʧ��: " << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();//�����������
	glAttachShader(shaderProgram, vertexShader);//��������ɫ�����ӵ�������
	glAttachShader(shaderProgram, fragmentShader);//��Ƭ����ɫ�����ӵ�������
	glLinkProgram(shaderProgram);//������ɫ��

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "������ɫ��ʧ��: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);//��������ɫ�������ɾ����������ɫ����
	glDeleteShader(fragmentShader);//��������ɫ�������ɾ����������ɫ����

	//glUseProgram(shaderProgram);//����������
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void HelloTriangles::flush()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgram);//����������

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	glBindVertexArray(VAO2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void HelloTriangles::release()
{
	delete this;
}