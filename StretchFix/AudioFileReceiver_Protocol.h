/*
 *  AudioFileReceiver_Protocol.h
 *  CocoaAUHost
 *
 *  Created by Luke Bellandi 2 on 12/14/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>

@protocol AudioFileReceiver
- (void)addLinkToFiles: (NSArray*)inFiles;
- (void)removeSelectedFile;
@end
