/*****
StretchFix (SMPTE-Stretch-Fix) for recovering time-stretched tape dubs
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/stretchfix/
*****/
//
//  SFWindowController.mm
//  StretchFix
//
//  Created by Kevin Dixon on 3/7/08.
//  Copyright 2008 Yano Signal Processing. All rights reserved.
//

#import <AudioUnit/AUCocoaUIView.h>
#import <CoreAudioKit/CoreAudioKit.h>

#import "SFWindowController.h"

#import "AudioFileListView.h"

@implementation SFWindowController

#pragma mark Constructors
- (void)awakeFromNib {

	mEngine = new SFEngine();
	if(!mEngine->Initialize()) {
		[NSApp terminate:self];
	}
	
	[self showCocoaViewForAU:mEngine->GetSSCAUUnit()];

	mAudioFileList = [[NSMutableArray alloc] init];
	
	//set table data sources
	[uiAudioFileTableView setDataSource:self];
	//setup frame data table
	[uiarsFrameDataTable setDataSource:self];
	//add index column
	NSTableColumn * index = [[NSTableColumn alloc] initWithIdentifier: @"index"];
	[index setWidth: 80];
	[index setEditable:NO];
	[[index headerCell] setStringValue: @"Index"];
	[uiarsFrameDataTable addTableColumn: index];
	//add skew column
	NSTableColumn * skew = [[NSTableColumn alloc] initWithIdentifier: @"skew"];
	[skew setWidth: 50];
	[skew setEditable:NO];
	[[skew headerCell] setStringValue: @"Skew"];
	[uiarsFrameDataTable addTableColumn: skew];
	//add description column
	NSTableColumn * description = [[NSTableColumn alloc] initWithIdentifier: @"desc"];
	[description setWidth: 500];
	[description setEditable:NO];
	[[description headerCell] setStringValue: @"Description"];
	[uiarsFrameDataTable addTableColumn: description];

	//release crap
	[index release];
	[skew release];
	[description release];
	// make this the app. delegate
	[NSApp setDelegate:self];
}


- (void)dealloc {

	[mAudioFileList release];
	
	delete mEngine;
	
	[NSApp setDelegate:nil];
	
	[super dealloc];
}

#pragma mark AudioUnit Handling


/*
	Returns true if a plugin class conforms to the AUCocoaUIBase protocol
*/
- (BOOL)plugInClassIsValid:(Class) pluginClass
{
	if ([pluginClass conformsToProtocol:@protocol(AUCocoaUIBase)]) {
		if ([pluginClass instancesRespondToSelector:@selector(interfaceVersion)] &&
			[pluginClass instancesRespondToSelector:@selector(uiViewForAudioUnit:withSize:)]) {
			return YES;
		}
	}
	
    return NO;
}

/*
	displays the UI for a given audio unit
*/
- (void)showCocoaViewForAU:(AudioUnit)inAU
{
	// get AU's Cocoa view property
    UInt32 						dataSize;
    Boolean 					isWritable;
    AudioUnitCocoaViewInfo *	cocoaViewInfo = NULL;
    UInt32						numberOfClasses;
    
    OSStatus result = AudioUnitGetPropertyInfo(	inAU,
                                                kAudioUnitProperty_CocoaUI,
                                                kAudioUnitScope_Global, 
                                                0,
                                                &dataSize,
                                                &isWritable );
    
    numberOfClasses = (dataSize - sizeof(CFURLRef)) / sizeof(CFStringRef);
    
    NSURL 	 *	CocoaViewBundlePath = nil;
    NSString *	factoryClassName = nil;
    
	// Does view have custom Cocoa UI?
    if ((result == noErr) && (numberOfClasses > 0) ) {
        cocoaViewInfo = (AudioUnitCocoaViewInfo *)malloc(dataSize);
        if(AudioUnitGetProperty(		inAU,
                                        kAudioUnitProperty_CocoaUI,
                                        kAudioUnitScope_Global,
                                        0,
                                        cocoaViewInfo,
                                        &dataSize) == noErr) {
            CocoaViewBundlePath	= (NSURL *)cocoaViewInfo->mCocoaAUViewBundleLocation;
			
			// we only take the first view in this example.
            factoryClassName	= (NSString *)cocoaViewInfo->mCocoaAUViewClass[0];
        } else {
            if (cocoaViewInfo != NULL) {
				free (cocoaViewInfo);
				cocoaViewInfo = NULL;
			}
        }
    }
	
	NSView *AUView = nil;
	BOOL wasAbleToLoadCustomView = NO;
	
	// [A] Show custom UI if view has it
	if (CocoaViewBundlePath && factoryClassName) {
		NSBundle *viewBundle  	= [NSBundle bundleWithPath:[CocoaViewBundlePath path]];
		if (viewBundle == nil) {
			NSLog (@"Error loading AU view's bundle");
		} else {
			Class factoryClass = [viewBundle classNamed:factoryClassName];
			NSAssert (factoryClass != nil, @"Error getting AU view's factory class from bundle");
			
			// make sure 'factoryClass' implements the AUCocoaUIBase protocol
			NSAssert(	[self plugInClassIsValid:factoryClass],
						@"AU view's factory class does not properly implement the AUCocoaUIBase protocol");
			
			// make a factory
			id factoryInstance = [[[factoryClass alloc] init] autorelease];
			NSAssert (factoryInstance != nil, @"Could not create an instance of the AU view factory");
			// make a view
			AUView = [factoryInstance	uiViewForAudioUnit:inAU
										withSize:[[uiAUViewContainer contentView] bounds].size];
			
			// cleanup
			[CocoaViewBundlePath release];
			if (cocoaViewInfo) {
				UInt32 i;
				for (i = 0; i < numberOfClasses; i++)
					CFRelease(cocoaViewInfo->mCocoaAUViewClass[i]);
				
				free (cocoaViewInfo);
			}
			wasAbleToLoadCustomView = YES;
		}
	}
	
	if (!wasAbleToLoadCustomView) {
		// [B] Otherwise show generic Cocoa view
		AUView = [[AUGenericView alloc] initWithAudioUnit:inAU];
		[(AUGenericView *)AUView setShowsExpertParameters:YES];
    }
	
	[uiAUViewContainer setContentView:AUView];
}

#pragma mark Actions
/*
	Called when the "Choose" button is pressed
*/
- (IBAction)chooseOutputDirectory: (id)sender {
	NSOpenPanel * openPanel;
	openPanel = [NSOpenPanel openPanel];
	[openPanel setCanChooseDirectories: YES];
	[openPanel setAllowsMultipleSelection: NO];
	[openPanel setResolvesAliases: YES];
	[openPanel setCanChooseFiles: NO];
 
	if([openPanel runModalForTypes:nil] == NSOKButton) {
		NSString * directory = [openPanel directory];
		[uiOutputDirectory setStringValue: directory];
	}
}

/*
	Called when the "Analyze" button is pressed
*/
- (IBAction)iaAnalyzeButtonPressed: (id)sender {
	if([mAudioFileList count] == 0)
		return; 
	
	//set state variables
	analysisResult = NO;
	
	//show progress sheet
	[self showAnalyzeProgressSheet];
		
	//start thread and pass argument of file path
	NSString * smpteFilePath = (NSString*)[mAudioFileList objectAtIndex:[uiAudioFileTableView selectedRow]];
	[NSThread detachNewThreadSelector:@selector(analysisWorkerThreadEntryPoint:) toTarget:self withObject:smpteFilePath];
	
	//update progress bar and monitor status
	[self startProgressTimer];
}

/*
	Entry point for the analyze thread
*/
- (void)analysisWorkerThreadEntryPoint: (id)arg {
	//create autorelease pool
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	//if we are an unfortunate victem of circumstance
	NSString * smpteFilePath = (NSString*)arg;
	if(smpteFilePath == nil) {
		[pool release];
		[NSThread exit];
	}
	
	analysisResult = mEngine->Analyze(smpteFilePath);
	
	if(!analysisResult) {
		NSRunAlertPanel(@"Analysis Failed", @"No SMPTE LTC data was parsed from the selected file. Try adjusting the SMPTE-Stretch-Calc settings, or choose a different file.", nil, nil, nil);
	}
	
	[pool release];
}

/*
	Starts de-stretching process
*/
- (IBAction)iaDestretchButtonPressed: (id)sender {
	
	analysisResult = false;
	
	if([[uiOutputDirectory stringValue] length] == 0)
	{
		[self chooseOutputDirectory:self];
	}
	//TODO: validate output directory
	
	[self showDestretchProgressSheet];
	[NSThread detachNewThreadSelector:@selector(destretchWorkerThreadEntryPoint:) toTarget:self withObject:nil];
	[self startProgressTimer];

}

- (void)destretchWorkerThreadEntryPoint: (id)arg {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	analysisResult = false;
	
	SInt32 smpteIndex = -1;
	if([uiDestretchSMPTECheckbox intValue] != 1)
	{
		smpteIndex = [uiAudioFileTableView selectedRow];
	}
		
	mEngine->Destretch(mAudioFileList, [uiOutputDirectory stringValue], smpteIndex);
	
	[pool release];
}

#pragma mark -> Analysis Results Sheet
/*
	Shows the analysis results sheet
*/
- (IBAction)showAnalysisResultsSheet: (id)sender {
	[uiarsFrameDataTable reloadData];

	[NSApp beginSheet: uiAnalysisResultsSheet
		  modalForWindow: [self window]
		  modalDelegate: self
		  didEndSelector: @selector(analysisResultsSheetDidEnd:returnCode:contextInfo:)
		  contextInfo: nil];
}

/*
	Ends the sheet, and determines which button was clicked to do it up, which it passes as the return code.
	(See enumeration of kAnalysisResultsAction_)
*/
- (IBAction)closeAnalysisResultsSheet: (id)sender {

	int code = kAnalysisResultsAction_Nothing;
	if(sender == uiarsDiscardButton) {
		code = kAnalysisResultsAction_Discard;
	} else if(sender == uiarsDestretchButton) {
		code = kAnalysisResultsAction_Destretch;
	}
	
	[NSApp endSheet:uiAnalysisResultsSheet returnCode:code];
}

/*
	Takes care of state changes after viewing the results sheet
*/
- (void)analysisResultsSheetDidEnd: (NSWindow*)sheet returnCode: (int)returnCode contextInfo: (void*)contextInfo {	
	[sheet orderOut:self];

	switch(returnCode) {
		case kAnalysisResultsAction_Destretch:
			[self iaDestretchButtonPressed:self];
		case kAnalysisResultsAction_Nothing:
			[self updateActionEnabledState:self];
			[uiAnalysisResultsButton setHidden:NO];
			break;
		case kAnalysisResultsAction_Discard:
			//Discard analysis
			[uiAnalysisResultsButton setHidden:YES];
			mEngine->ClearFrameAnalysis();
			break;
	}
}

#pragma mark -> Progress Sheet

/*
	Shows the progress sheet
*/
- (IBAction)showProgressSheet: (id)sender {

	abortCurrentAction = NO;

	[NSApp beginSheet: uiProgressSheet
		  modalForWindow: [self window]
		  modalDelegate: self
		  didEndSelector: @selector(progressSheetDidEnd:returnCode:contextInfo:)
		  contextInfo: nil];
}

/*
	Closes the progress sheet
*/
- (IBAction)closeProgressSheet: (id)sender {
	if(sender == uipsAbortButton)
		abortCurrentAction = YES;
				
	[NSApp endSheet:uiProgressSheet];
}

/*
	Terminates the sheet when we're notified that it ended
*/
- (void)progressSheetDidEnd: (NSWindow*)sheet returnCode: (int)returnCode contextInfo: (void*)contextInfo
{
	[sheet orderOut:self];
}

/*
	Starts the progress timer
*/
- (void)startProgressTimer
{
	NSTimer * progressUpdateTimer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(fireProgressUpdate:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer: progressUpdateTimer forMode: NSDefaultRunLoopMode];
}

/*
	Called every 0.1 seconds to update the progress bar and monitor work
*/
- (void)fireProgressUpdate:(NSTimer*)theTimer
{

	//calculate current progress
	double progress = mEngine->GetOverallProgress() * mEngine->GetCurrentProgress() * 100;
	[uipsProgress setDoubleValue: progress];
	[uipsProgress displayIfNeeded];
	if(![[uipsSubDescription stringValue] isEqual:@""])
	{
		int curFile = mEngine->GetOverallProgress() * [mAudioFileList count];
		[uipsSubDescription setStringValue:[NSString stringWithFormat:@"Destretching file %d...", curFile]];
	}
	[uipsSubDescription displayIfNeeded];
	
	//determine if abort or done
	if((abortCurrentAction) || (mEngine->ProcessStatus() == kSFEngineStatus_Done)) {
		//close out our monitoring station
		if(abortCurrentAction)
			mEngine->AbortCurrentProcess();
			
		[theTimer invalidate];
		[self closeProgressSheet:self];
		
		//if we completed an analysis
		if(analysisResult) {
			[uiarsSummaryText setStringValue:[NSString stringWithFormat:@"%d frames - %d errors (%d%%)", mEngine->GetFrameCount(), mEngine->GetErrorFrameCount(), (UInt32)(((Float32)mEngine->GetErrorFrameCount() / mEngine->GetFrameCount()) * 100.0)]];
			[uiarsFrameDataTable scrollRowToVisible: 0];
			[self showAnalysisResultsSheet:self];
		}
	}
}

/*
	Prepares the progress sheet to be used for analysis and shows it
*/
- (void)showAnalyzeProgressSheet
{
	[uipsDescription setStringValue:@"Analyzing SMPTE Linear Time Code..."];
	[uipsSubDescription setStringValue:@""];
	
	[uipsProgress setIndeterminate:NO];
	[uipsProgress setDoubleValue:0];
	
	[self showProgressSheet:self];
}

/*
	Prepares the progress sheet to be used for destretching and shows it
*/
- (void)showDestretchProgressSheet
{
	
	int totalFiles = [mAudioFileList count];
	if([uiDestretchSMPTECheckbox intValue] != 1)
	{
		totalFiles -= 1;
	}

	[uipsDescription setStringValue:[NSString stringWithFormat:@"Destretching %d files...", totalFiles]];
	
	[uipsSubDescription setStringValue:[NSString stringWithFormat:@"Destretching file %d...", 0]];
	[uipsProgress setIndeterminate:NO];
	[uipsProgress setDoubleValue:0];
	
	[self showProgressSheet:self];
}

#pragma mark Table Delegate Business
/*
	Called to return the value of a certain row
*/
- (id)tableView: (NSTableView *)inTableView
		objectValueForTableColumn:(NSTableColumn *)inTableColumn
		row:(int)inRow
{
	//display audio file list
	if((AudioFileListView*)inTableView == uiAudioFileTableView)
	{
	
		[self updateActionEnabledState:self];
		int count = [mAudioFileList count];
		return (count > 0) ?	[(NSString *)[mAudioFileList objectAtIndex:inRow] lastPathComponent] :
							@"< drag audio files here >";
							
	} 
	//display frame data table
	else if(inTableView == uiarsFrameDataTable)
	{
		SSCAUFrameData sfd;
		mEngine->GetFrameAt(inRow, &sfd);
		NSString * identifier = (NSString*)[inTableColumn identifier];
		
		if([identifier isEqualToString:@"index"])
		{
			return [NSString stringWithFormat:@"%d", sfd.index];
		}
		else if(([identifier isEqualToString:@"skew"]) && 
			   (!(sfd.error & sscauErr_FoundZeroWhenWorkingOnOne)) && 
			   (!(sfd.error & sscauErr_ClockMiss)))
		{
				
			if(sfd.skew == 0)
			{
				return @"";
			}
			return [NSString stringWithFormat:@"%d", sfd.skew];
		}
		else if([identifier isEqualToString:@"desc"])
		{
			NSString * desc = @"";
			if(sfd.error & sscauErr_FullFrameParsed)
				desc = [desc stringByAppendingString:@"End of Frame  "];
			if(sfd.error & sscauErr_FoundSyncWord)
				desc = [desc stringByAppendingString:@"(sync)  "];
			if(sfd.error & sscauErr_InvalidFrame)
				desc = [desc stringByAppendingString:@"Invalid Frame  "];
			if(sfd.error & sscauErr_FoundZeroWhenWorkingOnOne)
				desc = [desc stringByAppendingFormat:@"Found Zero when working on One (%d)  ", sfd.skew];
			if(sfd.error & sscauErr_ClockMiss)
				desc = [desc stringByAppendingFormat:@"Clock Miss (%d)  ", sfd.skew];
			return desc;
		}
	}
	
	return @"";
}

/*
	Called to determine the number of rows in a table view
*/
- (int)numberOfRowsInTableView : (NSTableView*)inTableView {
	if((AudioFileListView*)inTableView == uiAudioFileTableView) {
		int count = [mAudioFileList count];
		return (count > 0) ? count : 1;
	} else if(inTableView == uiarsFrameDataTable) {
		return mEngine->GetFrameCount();
	}
	
	return 0;
}

#pragma mark AudioFileListView
/*
	Called when files need to be added
*/
- (void)addLinkToFiles : (NSArray*)inFiles {
	int firstCount = [mAudioFileList count];
	[mAudioFileList addObjectsFromArray:inFiles];

	//get path
	if([[uiOutputDirectory stringValue] length] == 0) {
		NSString * firstFile = (NSString*)[mAudioFileList objectAtIndex:firstCount];
		
		[uiOutputDirectory setStringValue:[firstFile substringToIndex:[firstFile rangeOfString:@"/" options:NSBackwardsSearch].location + 1]];
	}
	
	[uiAudioFileTableView reloadData];
}

/*
	Message received when we should delete the currently selected item
*/
- (void)removeSelectedFile {
	if([mAudioFileList count] == 0)
		return;
		
	[mAudioFileList removeObjectAtIndex:[uiAudioFileTableView selectedRow]];
	[uiAudioFileTableView reloadData];
}

/*
	Message received when we should clear the list of files
*/
- (IBAction)removeAllFiles: (id)sender {
	[mAudioFileList removeAllObjects];
	[uiAudioFileTableView reloadData];
}

#pragma mark Misc UI
- (IBAction)updateActionEnabledState: (id)sender {
	bool enable = [mAudioFileList count] != 0;
	
	[uiAnalyzeButton setEnabled:enable];
	[uiAnalyzeButton displayIfNeeded];
	
	bool ed = enable && (([mAudioFileList count] > 1) || (([mAudioFileList count] == 1) && ([uiDestretchSMPTECheckbox intValue]))) && (mEngine->GetFrameCount() > 0);
	[uiDestretchButton setEnabled:ed];
	[uiDestretchButton displayIfNeeded];
	[uiarsDestretchButton setEnabled:ed];
	[uiarsDestretchButton displayIfNeeded];
	
	//enable/disable uiars destretch button

}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)inSender
{
	return YES;
}


@end
