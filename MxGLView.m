//
//  MxGLView.m
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/16.
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "MxGLView.h"

@interface MxGLView : NSOpenGLView {
    float size;
}

- (void)setSize:(float) s;
@end

@implementation MxGLView
- (id)initWithCoder:(NSCoder *)c
{
    self = [super initWithCoder:c];
    NSLog(@"prepare");
    
    [self setSize:0.5];
    
    // The GL Context must be active before you can use OpenGL functions
    NSOpenGLContext *glcontext = [self openGLContext];
    [glcontext makeCurrentContext];
    
    //Add your OpenGL init code here
    GLfloat mat_specular []={0.9, 0.3, 0.2, 1.0};
    GLfloat mat_shininess []={50.0};
    GLfloat light_position []={2.0, 4.0, -1.0, 0.0};
    float white_light []={1.0, 1.0, 1.0, 1.0};
    GLfloat lmodel_ambient []={0.1, 0.5, 0.8, 1.0};
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //    gluPerspective(90.0, 0.7, 0.0, 10.0);
    glRotatef(30, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    return self;
}
- (void)reshape
{
    NSLog(@"reshaping %f", size);
    
    //Get view dimensions
    NSRect baseRect = [self convertRectToBase:[self bounds]];
    int w, h;
    w = baseRect.size.width;
    h = baseRect.size.height;
    
    //Add your OpenGL resize code here
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}
- (void)drawRect:(NSRect)r
{
    //Add your OpenGL drawing code here
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 0.0, 0.0);
    //glutSolidTeapot(size);
    //glutWireCube(size*2.0);
    
    //Signal that drawing is done - causes a buffer swap
    [[self openGLContext] flushBuffer];
    
}
- (void)setSize:(float)s
{
    size=s;
}
@end



void setWnd(void* wnd, MxRect rect) {
    MxGLView *glView = [MxGLView alloc];
    [(MxGLView*)wnd addSubview:glView];
    [glView setFrame:NSMakeRect(rect.x, rect.y, rect.width, rect.height)];
}

NSView *createGLView() {
    return [[MxGLView alloc] init];
}





