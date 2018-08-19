//
//  BaseEffectFilter.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//
#include "stdafx.h"
#include "BaseEffectFilter.h"


static inline bool validateProgram(GLint prog) {
    GLint status;
    glValidateProgram(prog);
    
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar*)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        qDebug("Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        qDebug("Failed to validate program %d", prog);
        return false;
    }
    
    return true;
}

static inline GLuint compileShader(GLenum type, string shaderString) {
    
    GLint status;
    const GLchar *sources = (GLchar*)shaderString.c_str();
    GLint shader = glCreateShader(type);
    if (shader == 0 || shader == GL_INVALID_ENUM) {
        qDebug("Failed to create shader %d", type);
        return 0;
    }
    
    glShaderSource(shader, 1, &sources, nullptr);
    glCompileShader(shader);
    
    
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar*)malloc(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        qDebug("Shader compile log:\n%s", log);
        free(log);
    }
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status  == GL_FALSE) {
        glDeleteShader(shader);
        qDebug("Failed to compile shader:\n");
        return 0;
    }
    
    return shader;
}

static inline void mat4_LoadOrtho(float left, float right, float bottom, float top, float near1, float far1, float* mout) {
    float r_l = right - left;
    float t_b = top - bottom;
    float f_n = far1 - near1;
    float tx = - (right + left) / (right - left);
    float ty = - (top + bottom) / (top - bottom);
    float tz = - (far1 + near1) / (far1 - near1);
    
    mout[0] = 2.0f / r_l;
    mout[1] = 0.0f;
    mout[2] = 0.0f;
    mout[3] = 0.0f;
    
    mout[4] = 0.0f;
    mout[5] = 2.0f / t_b;
    mout[6] = 0.0f;
    mout[7] = 0.0f;
    
    mout[8] = 0.0f;
    mout[9] = 0.0f;
    mout[10] = -2.0f / f_n;
    mout[11] = 0.0f;
    
    mout[12] = tx;
    mout[13] = ty;
    mout[14] = tz;
    mout[15] = 1.0f;
}


bool BaseEffectFilter::prepareRender(int width, int height) {
    return false;
}

void BaseEffectFilter::renderWithWidth(int width, int height, float position) {
    
}

bool BaseEffectFilter::buildProgram(string vertexShader, string fragmentShader) {
    bool result = false;
    GLuint vertShader = 0, fragShader = 0;
    filterProgram = glCreateProgram();
    vertShader = compileShader(GL_VERTEX_SHADER, vertexShader);
    if (!vertShader) {
        goto exit;
    }
    fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    if (!vertShader) {
        goto exit;
    }
    
    glAttachShader(filterProgram, vertShader);
    glAttachShader(filterProgram, fragShader);
    glLinkProgram(filterProgram);
    filterPositionAttribute = glGetAttribLocation(filterProgram, "position");
    filterTextureCoordinateAttribute = glGetAttribLocation(filterProgram, "texcoord");
    filterInputTextureUniform = glGetAttribLocation(filterProgram, "inputImageTexture");
    
    GLint status;
    glGetProgramiv(filterProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        qDebug("Failed to link program %d", filterProgram);
        goto exit;
    }
    
    result = validateProgram(filterProgram);
    
exit:
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    if (!result) {
        glDeleteProgram(filterProgram);
        filterProgram = 0;
    }
    return result;
}

void BaseEffectFilter::setInputTexture(GLint textureId) {
    _inputTexId = textureId;
}

void BaseEffectFilter::releaseRender() {
    if (filterProgram) {
        glDeleteProgram(filterProgram);
        filterProgram = 0;
    }
}

GLint BaseEffectFilter::outputTextureID() {
    return -1;
}




