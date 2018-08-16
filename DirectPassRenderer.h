//
//  DirectPassRenderer.hpp
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#ifndef DirectPassRenderer_hpp
#define DirectPassRenderer_hpp

#include "BaseEffectFilter.h"


class DirectPassRenderer: public BaseEffectFilter {
    
    bool prepareRender(int width, int height);
    
    void renderWithWidth(int width, int height, float position);
    
};


#endif /* DirectPassRenderer_hpp */
