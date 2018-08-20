//
//  MxGLTool.cpp
//  MxGLTool
//
//  Created by sz17112850M01 on 2018/8/20.
//  Copyright © 2018年 lixiaopeng. All rights reserved.
//

#include <iostream>
#include "MxGLTool.hpp"
#include "MxGLToolPriv.hpp"

void MxGLTool::HelloWorld(const char * s)
{
    MxGLToolPriv *theObj = new MxGLToolPriv;
    theObj->HelloWorldPriv(s);
    delete theObj;
};

void MxGLToolPriv::HelloWorldPriv(const char * s) 
{
    std::cout << s << std::endl;
};

