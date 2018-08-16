//
//  BaseEffectFilter.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#ifndef BaseEffectFilter_hpp
#define BaseEffectFilter_hpp

#include <stdio.h>

class BaseEffectFilter {
    
public:

    bool prepareRender(int width, int height);
    
    void renderWithWidth(int width, int height, float position);
    
    bool buildProgram(string vertexShader, string fragmentShader);
    
    void setInputTexture(GLint textureId);
    
    void releaseRender();
    
    GLint outputTextureID();
    
public:
    GLint   _inputTexId;
    GLuint  filterProgram;
    GLint   filterPositionAttribute;
    GLint   filterTextureCoordinateAttribute;
    GLint   filterInputTextureUniform;
};

#endif /* BaseEffectFilter_hpp */
