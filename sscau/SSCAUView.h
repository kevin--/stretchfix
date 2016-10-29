/*
SSCAU - SMPTE-Stretch-Calc Audio Unit
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/
*/
/*
	SSCAUView.h
	
	02/16/2008
	Kevin C. Dixon
	Yano Signal Processing
*/

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

@interface SSCAUView : NSView
{
	
	IBOutlet NSButton * uiSquaredChoice;
	IBOutlet NSSlider * uiClockToleranceSlider;
	IBOutlet NSTextField * uiClockToleranceText;
	
	// Other Members
	AudioUnit mAU;
	AUParameterListenerRef mParameterListener;
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;

#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction)iaSaveLog : (id)sender;
- (IBAction)iaSquaredChoiceChanged : (id)sender;
- (IBAction)iaClockToleranceChanged : (id)sender;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)_synchronizeUIWithParameterValues;
- (void)_addListeners;
- (void)_removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue;

@end
