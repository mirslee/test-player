//
//  DirectPassRenderer.cpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//
#include "stdafx.h"
#include "DirectPassRenderer.h"

#define SHADER_STRING(text) #text

const string kDirectPassVertexShaderString =
"#version 330 core \
layout(location=0) in vec4 position;\
layout(location=1) in vec2 texcoord;\
\
out vec2 textureCoordinate;\
\
void main() {\
gl_Position = position;\
textureCoordinate = texcoord;\
}\
);";


const string kDirectPassFragmentShaderString =
"#version 330 core \
in vec2 textureCoordinate;\
uniform sampler2D inputImageTexture;\
\
void main()\
{\
gl_FragColor = texture2D(inputImageTexture,textureCoordinate);\
}\
);";


bool DirectPassRenderer::prepareRender(int width, int height) {
    bool ret = false;
    if (buildProgram(kDirectPassVertexShaderString, kDirectPassFragmentShaderString)) {
        glUseProgram(filterProgram);
        glEnableVertexAttribArray(filterPositionAttribute);
        glEnableVertexAttribArray(filterTextureCoordinateAttribute);
        ret = true;
    }
    return ret;
}

void DirectPassRenderer::renderWithWidth(int width, int height, float position) {
    
    glUseProgram(filterProgram);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _inputTexId);
    glUniform1i(filterInputTextureUniform, 0);
    
    static const GLfloat imageVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };
    
    GLfloat noRotationTextureCoordinates[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };
    
    glVertexAttribPointer(filterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices);
    glEnableVertexAttribArray(filterPositionAttribute);
    glVertexAttribPointer(filterTextureCoordinateAttribute, 2, GL_FLOAT, 0, 0, noRotationTextureCoordinates);
    glEnableVertexAttribArray(filterTextureCoordinateAttribute);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}








