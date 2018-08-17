
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

	glGenVertexArrays(1, &VAO);//创建顶点数组
	glBindVertexArray(VAO);//绑定VAO

	glGenBuffers(1, &VBO); //创建缓冲对象
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //把缓冲绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //顶点数据复制到缓冲区

	//链接顶点属性
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);//启用顶点属性，//registered VBO as the vertex attribute's bound vertex buffer object
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);//glVertexAttribPointer之后调用

	//索引缓冲对象
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

	glGenBuffers(1, &VBO2); //创建缓冲对象
	glBindBuffer(GL_ARRAY_BUFFER, VBO2); //把缓冲绑定到GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW); //顶点数据复制到缓冲区

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);//glVertexAttribPointer之后调用
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//glVertexAttribPointer之后调用

	//顶点着色器
	const char* vertexShaderSouce = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0);\n"
		"}\0";

	unsigned int vertexShader;//着色器id
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//创建着色器对象
	glShaderSource(vertexShader, 1, &vertexShaderSouce, nullptr);//着色器源码给着色器
	glCompileShader(vertexShader);//编译着色器

	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//检查编译着色器是否成功
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);//获取失败信息
		std::cout << "编译着色器失败: " << infoLog << std::endl;
	}

	//片段着色器
	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 fragColor;\n"
		"void main()\n"
		"{\n"
		"	fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);//检查编译着色器是否成功
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);//获取失败信息
		std::cout << "编译着色器失败: " << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();//创建程序对象
	glAttachShader(shaderProgram, vertexShader);//将顶点着色器附加到程序中
	glAttachShader(shaderProgram, fragmentShader);//将片段着色器附加到程序中
	glLinkProgram(shaderProgram);//链接着色器

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "链接着色器失败: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);//链接完着色器后可以删掉创建的着色器了
	glDeleteShader(fragmentShader);//链接完着色器后可以删掉创建的着色器了

	//glUseProgram(shaderProgram);//激活程序对象
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void HelloTriangles::flush()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgram);//激活程序对象

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