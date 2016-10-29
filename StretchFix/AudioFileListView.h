/*****
StretchFix (SMPTE-Stretch-Fix) for recovering time-stretched tape dubs
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/stretchfix/
*****/
//
//  AudioFileListView.h
//  StretchFix
//
//  Created by Kevin Dixon on 3/9/08.
//  Copyright 2008 Yano Signal Processing. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AudioFileListView : NSTableView {
	NSDragOperation		mCurrentDragOp;
}

@end
