//
//  VideoOutput.m
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#import "VideoOutput.h"


@interface VideoOutput()

@property (atomic) BOOL readyToRender;
@property (nonatomic, assign) BOOL shouldEnableOpenGL;
@property (nonatomic, strong) NSLock *shouldEnableOpenGLLock;
@property (nonatomic, strong) NSOperationQueue *renderOperationQueue;

@end

@implementation VideoOutput
{
    NSOpenGLContext *_context;
}

- (id) initWithFrame:(NSRect)frame textureWidth:(NSInteger)textureWitdth textureHeight:(NSInteger)textureHeight usingHWCodec:(BOOL) usingHWCodec {
    self = [super initWithFrame:frame];
    if (self) {
        _shouldEnableOpenGLLock = [NSLock new];
        [_shouldEnableOpenGLLock lock];
        _shouldEnableOpenGL = YES;
        [_shouldEnableOpenGLLock unlock];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:NSApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:NSApplicationDidBecomeActiveNotification object:nil];
    }
    
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion3_2Core,
        0
    };
    
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWidthAtrributes:attrs];
    
    if (!pf) {
        
    }
    NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    [self setPixelFormat:pf];
    [pf release];
    [self setOpenGLContext:context];
}

@end


