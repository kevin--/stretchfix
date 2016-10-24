/*
	SRCView.h
	SecretRabbit Sample Rate Converter AU
	
	Kevin C. Dixon
	Yano Signal Processing
	10/04/2008
*/

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

@interface SRCView : NSView
{
    // IB Members
    IBOutlet NSSlider *				uiParam1Slider;
    IBOutlet NSTextField *			uiParam1TextField;
	
    // Other Members
    AudioUnit 				mAU;
    AUParameterListenerRef	mParameterListener;
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;

#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction)iaParam1Changed:(id)sender;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)_synchronizeUIWithParameterValues;
- (void)_addListeners;
- (void)_removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue;

@end
