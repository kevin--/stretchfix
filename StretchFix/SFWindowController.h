/*****
StretchFix (SMPTE-Stretch-Fix) for recovering time-stretched tape dubs
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/stretchfix/

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*****/
//
//  SFWindowController.h
//  StretchFix
//
//  Created by Kevin Dixon on 3/7/08.
//  Copyright 2008 Yano Signal Processing. All rights reserved.
//


#import <Cocoa/Cocoa.h>

#include "SFEngine.h"

#import "AudioFileReceiver_Protocol.h"

enum {
	kAnalysisResultsAction_Nothing = 0,
	kAnalysisResultsAction_Discard = 1,
	kAnalysisResultsAction_Destretch = 2
};

@class AudioFileListView;

@interface SFWindowController : NSWindowController <AudioFileReceiver, NSTableViewDataSource, NSApplicationDelegate>
{
	IBOutlet NSBox * uiAUViewContainer;
	IBOutlet NSButton * uiDestretchSMPTECheckbox;
	IBOutlet NSButton * uiAnalyzeButton;
	IBOutlet NSButton * uiDestretchButton;
	IBOutlet NSButton * uiAnalysisResultsButton;
	
	IBOutlet AudioFileListView * uiAudioFileTableView;
	IBOutlet NSTextField * uiOutputDirectory;
	
	NSMutableArray * mAudioFileList;
	
	SFEngine * mEngine;
	
	//other windows
	//Analysis Results Sheet
	IBOutlet NSPanel * uiAnalysisResultsSheet;
	IBOutlet NSTableView * uiarsFrameDataTable;
	IBOutlet NSButton * uiarsDiscardButton;
	IBOutlet NSButton * uiarsDestretchButton;
	IBOutlet NSTextField * uiarsSummaryText;
	//Progress Sheet
	IBOutlet NSPanel * uiProgressSheet;
	IBOutlet NSTextField * uipsDescription;
	IBOutlet NSTextField * uipsSubDescription;
	IBOutlet NSButton * uipsAbortButton;
	IBOutlet NSProgressIndicator * uipsProgress;
	BOOL abortCurrentAction;
	BOOL analysisResult;
}

#pragma mark Buttons on main window
- (IBAction)chooseOutputDirectory: (id)sender;
- (IBAction)removeAllFiles: (id)sender;

- (IBAction)iaDestretchButtonPressed: (id)sender;
- (IBAction)iaAnalyzeButtonPressed: (id)sender;

- (void)analysisWorkerThreadEntryPoint: (id)arg;

#pragma mark Analysis Results Sheet methods
//methods for Analysis Results sheet
- (IBAction)showAnalysisResultsSheet: (id)sender;
- (IBAction)closeAnalysisResultsSheet: (id)sender;
- (void)analysisResultsSheetDidEnd: (NSWindow*)sheet returnCode: (int)returnCode contextInfo: (void*)contextInfo;	

#pragma mark Progress Sheet methods
//methods for Progress Sheet
- (IBAction)showProgressSheet: (id)sender;
- (IBAction)closeProgressSheet: (id)sender;
- (void)progressSheetDidEnd: (NSWindow*)sheet returnCode: (int)returnCode contextInfo: (void*)contextInfo;

- (void)startProgressTimer;

- (void)showAnalyzeProgressSheet;
- (void)showDestretchProgressSheet;


#pragma mark Utility
- (IBAction)updateActionEnabledState: (id)sender;

- (void)showCocoaViewForAU: (AudioUnit)inAU;

@end
