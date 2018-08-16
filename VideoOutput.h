//
//  VideoOutput.h
//  MyPlayer
//
//  Created by sz17112850M01 on 2018/8/15.
//

#import <Cocoa/Cocoa.h>
#import "BaseEffectFilter.h"

@interface VideoOutput : NSOpenGLView

- (id) initWithFrame:(NSRect)frame textureWidth:(NSInteger)textureWitdth textureHeight:(NSInteger)textureHeight usingHWCodec:(BOOL) usingHWCodec;


- (BaseEffectFilter*) createImageProcessFilterInstance;
- (BaseEffectFilter*) getImageProcessFilterInstance;

- (void) destroy;


@end
