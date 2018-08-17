#pragma once

class LightColor
{
public:
	LightColor();
	~LightColor();

	void prepare();
	void flush();
	void release();

private:


private:
	unsigned int cubeVAO;
	unsigned int cubeVBO;
	unsigned int cubeShaderProgram;

	unsigned int lampVAO;
	unsigned int lampVBO;
	unsigned int lampShaderProgram;

	unsigned int tempShaderProgram;
};

