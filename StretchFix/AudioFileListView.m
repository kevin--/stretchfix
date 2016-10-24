//
//  AudioFileListView.m
//  AudioFileUtility
//
//  Created by doug on Tue Jun 10 2003.
//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//

#import "AudioFileReceiver_Protocol.h"

#import "AudioFileListView.h"


@interface NSView (DragHighlight)
	- (void)drawDragHighlightOutside:(BOOL)anOutside;
	- (void)clearDragHighlightOutside;
@end

@implementation NSView (DragHighlight)

- (void)drawDragHighlightOutside:(BOOL)anOutside
{
	NSView  		* theDrawView;
	
	theDrawView = ( anOutside ) ? [self superview] : self;
	
	if( theDrawView != nil )
	{
		NSRect  				theRect, winRect;
		
		theRect = ( anOutside ) ? NSInsetRect([theDrawView frame], -3, -3) : [theDrawView bounds];
		winRect = theRect;
		winRect = [theDrawView convertRect: theRect toView: nil];
		[[self window] cacheImageInRect:winRect];
		
		[theDrawView lockFocus];
		
		[[NSColor selectedControlColor] set];
		NSFrameRectWithWidthUsingOperation( theRect, 3, NSCompositeSourceOver );
		[[NSGraphicsContext currentContext] flushGraphics];
		
		[theDrawView unlockFocus];
	}
}

- (void)clearDragHighlightOutside
{
	[[self window] restoreCachedImage];
	[[self window] flushWindow];
}

@end

@implementation AudioFileListView

/*
	awakeFromNib
*/
- (void)awakeFromNib {
	[self registerForDraggedTypes:[NSArray arrayWithObjects: NSFilenamesPboardType, nil]];
	
	[self setDelegate:self];
}

/*
	initWithFrame
*/
- (id)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if (self) {
		mCurrentDragOp = NSDragOperationNone;
   }
	return self;
}

#pragma mark Drag Handling

/*
	draggingEntered
*/
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
	NSPasteboard *pboard;
	NSDragOperation sourceDragMask;

	sourceDragMask = [sender draggingSourceOperationMask];
	pboard = [sender draggingPasteboard];

	if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
		if (sourceDragMask & NSDragOperationLink) {
			[self drawDragHighlightOutside:NO];
			return mCurrentDragOp = NSDragOperationLink;
		}
	}
	return mCurrentDragOp = NSDragOperationNone;
}

/*
	draggingExited
*/
- (void)draggingExited:(id <NSDraggingInfo>)sender {
	[self clearDragHighlightOutside];
	mCurrentDragOp = NSDragOperationNone;
}

/*
	draggingUpdated
*/
- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender {
	return mCurrentDragOp;
}

/*
	prepareForDragOperation
*/
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
	return YES;
}

/*
	performDragOperation
*/
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
	NSPasteboard *pboard;
	NSDragOperation sourceDragMask;

	[self clearDragHighlightOutside];
	sourceDragMask = [sender draggingSourceOperationMask];
	pboard = [sender draggingPasteboard];

	if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
		NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];

		// Depending on the dragging source and modifier keys,
		// the file data may be copied or linked
		if (sourceDragMask & NSDragOperationLink) {
			id<AudioFileReceiver> fileReceiver = (id<AudioFileReceiver>)[self dataSource];
			[fileReceiver addLinkToFiles:files];
		}
	}
	return YES;
}

#pragma mark Keystroke Handling
/*
	called upon a keypress when focused
*/
- (void)keyDown : (NSEvent*)theEvent {
	if([theEvent type] == NSKeyDown) {
		if([theEvent keyCode] == 51) {
			id<AudioFileReceiver> dataMaintainer = (id<AudioFileReceiver>)[self dataSource];
			[dataMaintainer removeSelectedFile];
		}
	}
}

@end
