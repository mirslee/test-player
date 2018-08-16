
#include "stdafx.h"
#import "VXGLView.h"


@implementation VXGLView




- (VXBOOL)setParent:(NSView*)win frame:(NSRect)rect{

	if(win)
	{
		needfreshonunhide = [win isHiddenOrHasHiddenAncestor]==YES;
		[win addSubview:self];
		[self setFrame:rect];
		[self setHidden:NO];
		[self display];
	}
	else 
	{
		NSView* superview = [self superview];
		if(superview)
		{
			[self setHidden:YES];
			[self removeFromSuperview];
			[superview display];
		}
	}
	return TRUE;
}

- (VXBOOL)initCGL:(CGLContextObj)cgl player:(CVxGLVidPlayer*)player {
	vidplayer = player;
	openGLContext = [[NSOpenGLContext alloc] initWithCGLContextObj:cgl];
	if(!openGLContext) return FALSE;
	needfreshonunhide = FALSE;
	return TRUE;
}


#pragma mark - 
#pragma mark NSView overrides
#pragma mark -

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        const NSString *mimeTypeGeneric = @"com.trolltech.qt.MimeTypeName";
		[self registerForDraggedTypes:[NSArray arrayWithObjects:mimeTypeGeneric,NSFilenamesPboardType, nil]];
	}
    return self;
}

- (void)dealloc {
	[openGLContext release];
	[super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect {
	// 	vidplayer->__OnPaint();
}
		
- (BOOL)isOpaque {
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent {
    return YES;
}

-(void)viewDidUnhide
{
	NSView* superview = [self superview];
	if(needfreshonunhide&&superview&&[superview isHiddenOrHasHiddenAncestor]==NO)
	{
		vidplayer->__Unhide();
		needfreshonunhide = FALSE;
	}
}

- (void)lockFocus {
    [super lockFocus];
	
    if ([openGLContext view] != self) {
        [openGLContext setView:self];
    }
    [openGLContext makeCurrentContext];
}

- (void)mouseDown:(NSEvent *)theEvent {
	if(!vidplayer->__macmessage(theEvent,NSLeftMouseDown))
		[super mouseDown:theEvent];
}


- (void)rightMouseDown:(NSEvent *)theEvent {
	vidplayer->__macmessage(theEvent,NSRightMouseDown);
	[[self superview] rightMouseDown:theEvent];
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
	return [[self superview] draggingEntered:sender];
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
	[[self superview] draggingExited:sender];
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	return [[self superview] performDragOperation:sender];
}

@end


#pragma mark - 
#pragma mark CVxGLVidPlayer windows
#pragma mark -

VXBOOL InitGLView(HVXWND wnd,CGLContextObj cgl,void* p)
{
	return [(VXGLView*)wnd initCGL:cgl player:(CVxGLVidPlayer*)p];
}

VXBOOL SetParant(HVXWND wnd,HVXWND parent,VXRECT* rc)
{
    NSRect rect = {0,0,1,1};
	if(rc) rect = NSMakeRect(0,0,rc->right-rc->left,rc->bottom-rc->top);
	return [(VXGLView*)wnd setParent:(NSView*)parent frame:rect];
}


VXBOOL CVxGLVidPlayer::CreateDrawWindow()
{
	NSRect rect = NSMakeRect(0,0,m_nLiveW,m_nLiveH);

	m_hWnd = [[VXGLView alloc] initWithFrame:rect];
	return TRUE;
}


void CVxGLVidPlayer::DestroyDrawWindow()
{
	[(VXGLView*)m_hWnd release];
}

VXBOOL CVxGLVidPlayer::__macmessage(void* event,DWORD type)
{
	if(!m_uidraw) return FALSE;
	int w = m_rcFill.Width();
	int h = m_rcFill.Height();
	NSEvent* theEvent = (NSEvent*)event;
	NSPoint mouseLoc = [(NSView*)m_hWnd convertPoint:[theEvent locationInWindow] fromView:nil];
	if(type==NSLeftMouseDown)
	{
		UIDRAW uid = {uidraw_hit,uihit_dragbegin,m_lastframe,w,h,m_aspect,m_rate,m_scale,mouseLoc.x,h-mouseLoc.y,m_dc};
		if(m_uidraw(&uid,m_uip)==0)
		{
			BOOL keepOn = YES;
			while (keepOn)
			{
				theEvent = [[(NSView*)m_hWnd window] nextEventMatchingMask: NSLeftMouseUpMask|NSLeftMouseDraggedMask];
				switch ([theEvent type]) 
				{
					case NSLeftMouseDragged:
						{
							NSPoint mouseLoc = [(NSView*)m_hWnd convertPoint:[theEvent locationInWindow] fromView:nil];
							uid.timestamp = uihit_dragto;
							uid.x = mouseLoc.x;
							uid.y = h-mouseLoc.y;
							m_uidraw(&uid,m_uip);
						}
						break;
					case NSLeftMouseUp:
						keepOn = NO;
						break;
					default:
						ASSERT(FALSE);
						break;
				}			
			}
			uid.timestamp = uihit_dragend;
			uid.isseek = TRUE;
			m_uidraw(&uid,m_uip);
			return TRUE;
		}
		else
			return FALSE;
	}
	else if(type==NSRightMouseDown)
	{
		UIDRAW uid = {uidraw_hit,uihit_rbhit,m_lastframe,w,h,m_aspect,m_rate,m_scale,mouseLoc.x,h-mouseLoc.y,m_dc};
		m_uidraw(&uid,m_uip);
	}
	return FALSE;
}
