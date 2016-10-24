/*
	SRCView.m
	SecretRabbit Sample Rate Converter AU
	
	Kevin C. Dixon
	Yano Signal Processing
	10/04/2008
*/

#import "SRCView.h"

enum {
	kParam_One =0,
	kNumberOfParameters=1
};

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____
AudioUnitParameter parameter[] = {	{ 0, kParam_One, kAudioUnitScope_Global, 0 }	};

void ParameterListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitParameter *inParameter, Float32 inValue) {
	SRCView *SELF = (SRCView *)inRefCon;
    
    [SELF _parameterListener:inObject parameter:inParameter value:inValue];
}

@implementation SRCView
#pragma mark ____ (INIT /) DEALLOC ____
- (void)dealloc {
    [self _removeListeners];
    [super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU {
	// remove previous listeners
	if (mAU) [self _removeListeners];
	mAU = inAU;
    
	// add new listeners
	[self _addListeners];
	
	// initial setup
	[self _synchronizeUIWithParameterValues];
}

#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction)iaParam1Changed:(id)sender {
    float floatValue = [sender floatValue];
	NSAssert(	AUParameterSet(mParameterListener, sender, &parameter[0], (Float32)floatValue, 0) == noErr,
                @"[CocoaView iaParam1Changed:] AUParameterSet()");
    if (sender == uiParam1Slider) {
        [uiParam1TextField setFloatValue:floatValue];
    } else {
        [uiParam1Slider setFloatValue:floatValue];
    }
}


#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)_addListeners {
	NSAssert (	AUListenerCreate(	ParameterListenerDispatcher, self, 
                                    CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0.100, // 100 ms
                                    &mParameterListener	) == noErr,
                @"[CocoaView _addListeners] AUListenerCreate()");
	
    int i;
    for (i = 0; i < kNumberOfParameters; ++i) {
        parameter[i].mAudioUnit = mAU;
        NSAssert (	AUListenerAddParameter (mParameterListener, NULL, &parameter[i]) == noErr,
                    @"[CocoaView _addListeners] AUListenerAddParameter()");
    }
}

- (void)_removeListeners {
    int i;
    for (i = 0; i < kNumberOfParameters; ++i) {
        NSAssert (	AUListenerRemoveParameter(mParameterListener, NULL, &parameter[i]) == noErr,
                    @"[CocoaView _removeListeners] AUListenerRemoveParameter()");
    }
    
	NSAssert (	AUListenerDispose(mParameterListener) == noErr,
                @"[CocoaView _removeListeners] AUListenerDispose()");
}

- (void)_synchronizeUIWithParameterValues {
	Float32 value;
    int i;
    
    for (i = 0; i < kNumberOfParameters; ++i) {
        // only has global parameters
        NSAssert (	AudioUnitGetParameter(mAU, parameter[i].mParameterID, kAudioUnitScope_Global, 0, &value) == noErr,
                    @"[CocoaView synchronizeUIWithParameterValues] (x.1)");
        NSAssert (	AUParameterSet (mParameterListener, self, &parameter[i], value, 0) == noErr,
                    @"[CocoaView synchronizeUIWithParameterValues] (x.2)");
        NSAssert (	AUParameterListenerNotify (mParameterListener, self, &parameter[i]) == noErr,
                    @"[CocoaView synchronizeUIWithParameterValues] (x.3)");
    }
}

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue {
    //inObject ignored in this case.
    
	switch (inParameter->mParameterID) {
		case kParam_One:
                    [uiParam1Slider setFloatValue:inValue];
                    [uiParam1TextField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
	}
}


@end
