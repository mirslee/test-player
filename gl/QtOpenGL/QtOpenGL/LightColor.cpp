

#include "LightColor.h"
#include "CELLMath.hpp"
#include "glew/glew.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

LightColor::LightColor()
{
	lampVAO = 0;
	lampVBO = 0;
	lampShaderProgram = 0;
	cubeVAO = 0;
	cubeVBO = 0;
	cubeShaderProgram = 0;
}


LightColor::~LightColor()
{
	glDeleteVertexArrays(1, &lampVAO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &lampVBO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteProgram(lampShaderProgram);
	glDeleteProgram(cubeShaderProgram);
}


void LightColor::prepare()
{
	int success;

	//ËÄ·½Ìå
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	const char* cubeVertexShaderSource = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{"
		"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\0";

	unsigned int cubeVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(cubeVertexShader, 1, &cubeVertexShaderSource, NULL);
	glCompileShader(cubeVertexShader);
	glGetShaderiv(cubeVertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(cubeVertexShader, 512, NULL, (GLchar*)infoLog);
		std::cout << "±àÒë¶¥µã×ÅÉ«Æ÷Ê§°Ü: " << infoLog << std::endl;
	}

	const char* cubeFragShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"uniform vec3 objectColor;\n"
		"uniform vec3 lightColor;\n"
		"void main()\n"
		"{"
		"	FragColor = vec4(lightColor * objectColor, 1.0);\n"
		"}\0";

	unsigned int cubeFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(cubeFragShader, 1, &cubeFragShaderSource, NULL);
	glCompileShader(cubeFragShader);
	glGetShaderiv(cubeFragShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(cubeFragShader, 512, NULL, (GLchar*)infoLog);
		std::cout << "±àÒë¶¥µã×ÅÉ«Æ÷Ê§°Ü: " << infoLog << std::endl;
	}

	cubeShaderProgram = glCreateProgram();
	glAttachShader(cubeShaderProgram, cubeVertexShader);
	glAttachShader(cubeShaderProgram, cubeFragShader);
	glLinkProgram(cubeShaderProgram);
	glGetProgramiv(cubeShaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(cubeShaderProgram, 512, NULL, (GLchar*)infoLog);
		std::cout << "Á´½Ó¶¥µã×ÅÉ«Æ÷Ê§°Ü: " << infoLog << std::endl;
	}

	glDeleteShader(cubeVertexShader);
	glDeleteShader(cubeFragShader);


	//¹âÔ´
	glGenVertexArrays(1, &lampVAO);
	glBindVertexArray(lampVAO);
	glGenBuffers(1, &lampVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	const char* lampVertexShaderSource = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{"
		"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\0";

	unsigned int lampVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL);
	glCompileShader(lampVertexShader);
	glGetShaderiv(lampVertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(lampVertexShader, 512, NULL, (GLchar*)infoLog);
		std::cout << "±àÒë¶¥µã×ÅÉ«Æ÷Ê§°Ü: " << infoLog << std::endl;
	}

	//Æ¬¶Î×ÅÉ«Æ÷
	const char* lampFragShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{"
		"	FragColor = vec4(1.0);\n"
		"}\0";

	unsigned int lampFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(lampFragShader, 1, &lampFragShaderSource, NULL);
	glCompileShader(lampFragShader);
	glGetShaderiv(lampFragShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(lampFragShader, 512, NULL, (GLchar*)infoLog);
		std::cout << "±àÒëÆ¬¶Î×ÅÉ«Æ÷Ê§°Ü: " << infoLog << std::endl;
	}

	lampShaderProgram = glCreateProgram();
	glAttachShader(lampShaderProgram, lampVertexShader);
	glAttachShader(lampShaderProgram, lampFragShader);
	glLinkProgram(lampShaderProgram);
	glGetProgramiv(lampShaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(lampShaderProgram, 512, NULL, (GLchar*)infoLog);
		std::cout << "Á´½ÓÆ¬¶Î×ÅÉ«Æ÷Ê§°Ü: " << infoLog << std::endl;
	}

	glDeleteShader(lampVertexShader);
	glDeleteShader(lampFragShader);

	/*tempshader*/
	const char* tempVertexShaderSource = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos, 1.0);\n"
		"}\0";

	unsigned int tempVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(tempVertexShader, 1, &tempVertexShaderSource, NULL);
	glCompileShader(tempVertexShader);
	glGetShaderiv(tempVertexShader, GL_COMPILE_STATUS, &success);

	const char* tempFragShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{"
		"	FragColor = vec4(1.0);\n"
		"}\0";

	unsigned int tempfragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(tempfragShader, 1, &tempFragShaderSource, NULL);
	glCompileShader(tempfragShader);
	glGetShaderiv(tempfragShader, GL_COMPILE_STATUS, &success);

	tempShaderProgram = glCreateProgram();
	glAttachShader(tempShaderProgram, tempVertexShader);
	glAttachShader(tempShaderProgram, tempfragShader);
	glLinkProgram(tempShaderProgram);
	glGetProgramiv(tempShaderProgram, GL_LINK_STATUS, &success);

	glDeleteShader(tempVertexShader);
	glDeleteShader(tempfragShader);
}
void LightColor::flush()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	glUseProgram(cubeShaderProgram);
	glUniform3f(glGetUniformLocation(cubeShaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);
	glUniform3f(glGetUniformLocation(cubeShaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
		(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

	glm::mat4 model;
	glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
	

	
	//glUseProgram(tempShaderProgram);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glUseProgram(lampShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

	model = glm::mat4();
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
	glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

	glBindVertexArray(lampVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
void LightColor::release()
{
	delete this;
}