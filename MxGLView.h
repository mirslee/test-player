//
//  MxGLView.h
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//




struct MxRect {
    int x;
    int y;
    int width;
    int height;
};

void setWnd(void* wnd, MxRect rect);

NSView *createGLView();
