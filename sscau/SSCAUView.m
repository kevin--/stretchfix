/*
SSCAU - SMPTE-Stretch-Calc Audio Unit
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*
	SSCAUView.m
	
	02/16/2008
	Kevin C. Dixon
	Yano Signal Processing
*/

#import "SSCAUView.h"
#include "sscauParams.h"

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____
AudioUnitParameter parameter[] = {	{ 0, kParam_OutputSquare, kAudioUnitScope_Global, 0 },
							{ 0, kParam_ClockTolerance, kAudioUnitScope_Global, 0 } };
	

void ParameterListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitParameter *inParameter, Float32 inValue)
{
	SSCAUView * SELF = (SSCAUView*)inRefCon;
    
    [SELF _parameterListener:inObject parameter:inParameter value:inValue];
}

@implementation SSCAUView
#pragma mark ____ (INIT /) DEALLOC ____
- (void)dealloc
{
    [self _removeListeners];
    [super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU
{
	// remove previous listeners
	if (mAU) [self _removeListeners];
	mAU = inAU;
    
	// add new listeners
	[self _addListeners];
	
	// initial setup
	[self _synchronizeUIWithParameterValues];
}

#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction) iaSaveLog : (id) sender
{
	//show save dialog
	//save log
}

- (IBAction) iaSquaredChoiceChanged : (id) sender
{
	//set AU param
	AUParameterSet(mParameterListener, sender, &parameter[kParam_OutputSquare], (Float32)[sender floatValue], 0);
}

- (IBAction) iaClockToleranceChanged : (id) sender
{
	float floatValue = [sender floatValue];
	AUParameterSet(mParameterListener, sender, &parameter[kParam_ClockTolerance], (Float32)floatValue, 0);
	if (sender == uiClockToleranceSlider)
	{
		[uiClockToleranceText setFloatValue:floatValue];
	}
	else
	{
		[uiClockToleranceSlider setFloatValue:floatValue];
	}
}


#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)_addListeners
{
	NSAssert(AUListenerCreate(ParameterListenerDispatcher, self, 
						 CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0.100, // 100 ms
						 &mParameterListener) == noErr,
						 @"[CocoaView _addListeners] AUListenerCreate()");
	
    int i;
    for (i = 0; i < kNumberOfParameters; ++i)
    {
        parameter[i].mAudioUnit = mAU;
        NSAssert (	AUListenerAddParameter (mParameterListener, NULL, &parameter[i]) == noErr,
                    @"[CocoaView _addListeners] AUListenerAddParameter()");
    }
}

- (void)_removeListeners
{
	int i;
	for (i = 0; i < kNumberOfParameters; ++i)
	{
		NSAssert(AUListenerRemoveParameter(mParameterListener, NULL, &parameter[i]) == noErr,
                   @"[CocoaView _removeListeners] AUListenerRemoveParameter()");
	}
    
	NSAssert( AUListenerDispose(mParameterListener) == noErr,
			@"[CocoaView _removeListeners] AUListenerDispose()");
}

- (void)_synchronizeUIWithParameterValues
{
	Float32 value;
    
    //checkbox update
    AudioUnitGetParameter(mAU, kParam_OutputSquare, kAudioUnitScope_Global, 0, &value);
    AUParameterSet(mParameterListener, self, &parameter[kParam_OutputSquare], value, 0);
    AUParameterListenerNotify(mParameterListener, self, &parameter[kParam_OutputSquare]);
    
    //slider update
    AudioUnitGetParameter(mAU, kParam_ClockTolerance, kAudioUnitScope_Global, 0, &value);
    AUParameterSet(mParameterListener, self, &parameter[kParam_ClockTolerance], value, 0);
    AUParameterListenerNotify(mParameterListener, self, &parameter[kParam_ClockTolerance]);
    /*
    //time stamp updater
    NSString * timeStamp;
    //--hours
    AudioUnitGetParameter(mAU, kParam_Hours, kAudioUnitScope_Global, 0, &value);
    timeStamp = [[NSNumber numberWithFloat: value] stringValue];
    timeStamp = [timeStamp stringByAppendingString: @" : "];
    //--minutes
    AudioUnitGetParameter(mAU, kParam_Minutes, kAudioUnitScope_Global, 0, &value);
    timeStamp = [timeStamp stringByAppendingString: [[NSNumber numberWithFloat: value] stringValue]];
    timeStamp = [timeStamp stringByAppendingString: @" : "];
    //--seconds
    AudioUnitGetParameter(mAU, kParam_Seconds, kAudioUnitScope_Global, 0, &value);
    timeStamp = [timeStamp stringByAppendingString: [[NSNumber numberWithFloat: value] stringValue]];
    timeStamp = [timeStamp stringByAppendingString: @" . "];
    //--frames
    AudioUnitGetParameter(mAU, kParam_Frames, kAudioUnitScope_Global, 0, &value);    
    timeStamp = [timeStamp stringByAppendingString: [[NSNumber numberWithFloat: value] stringValue]];
    
    [uiTimeStamp setStringValue: timeStamp];*/
    
    
   // [timeStamp dealloc];
    
/*	for (i = 0; i < kNumberOfParameters; ++i) {
        // only has global parameters
        NSAssert (	AudioUnitGetParameter(mAU, parameter[i].mParameterID, kAudioUnitScope_Global, 0, &value) == noErr,
                    @"[CocoaView synchronizeUIWithParameterValues] (x.1)");
        NSAssert (	AUParameterSet (mParameterListener, self, &parameter[i], value, 0) == noErr,
                    @"[CocoaView synchronizeUIWithParameterValues] (x.2)");
        NSAssert (	AUParameterListenerNotify (mParameterListener, self, &parameter[i]) == noErr,
                    @"[CocoaView synchronizeUIWithParameterValues] (x.3)");
    }*/
}

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue
{
    //inObject ignored in this case.
    
	switch (inParameter->mParameterID)
	{
		case kParam_ClockTolerance:
                    [uiClockToleranceSlider setFloatValue:inValue];
                    [uiClockToleranceText setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
	}
}


@end
