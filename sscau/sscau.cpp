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
	sscau.cpp
	SMPTE-Stretch-Calc Audio Unit
	
	01/29/2008
	Kevin C. Dixon
	Yano Signal Processing
*/

#include "sscau.h"
#include "ComponentBase.h"

#ifdef LOGTOFILE
#include <fstream>
#endif
using namespace std;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AUDIOCOMPONENT_ENTRY(AUBaseFactory, sscau)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	sscau::sscau
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sscau::sscau(AudioUnit component) : AUEffectBase(component)
{
	CreateElements();
	//get input and output busses
	AUInputElement * theInput = GetInput(0);
	AUOutputElement * theOutput = GetOutput(0);
	//modify default connection to have 1 channel in
	CAStreamBasicDescription desc;
	desc = theInput->GetStreamFormat();
	desc.ChangeNumberChannels(1, false);
	theInput->SetStreamFormat(desc);
	//and one channel out
	desc = theOutput->GetStreamFormat();
	desc.ChangeNumberChannels(1, false);
	theOutput->SetStreamFormat(desc);
	
	//setup parameters
	Globals()->UseIndexedParameters(kNumberOfParameters);
	
	SetParameter(kParam_OutputSquare, 0);
	SetParameter(kParam_ClockTolerance, 25.0);
	
	//create analysis queue
	analysis = queue<SSCAUFrameData>();
		
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
	
}

/*
	Called when we need to reset our state
*/
ComponentResult sscau::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	//clear analysis queue
	analysis = queue<SSCAUFrameData>();
	
	return AUEffectBase::Reset(inScope, inElement);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	sscau::GetParameterValueStrings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult sscau::GetParameterValueStrings(	AudioUnitScope inScope,
										AudioUnitParameterID inParameterID,
										CFArrayRef * outStrings)
{
    return kAudioUnitErr_InvalidProperty;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	sscau::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult sscau::GetParameterInfo(AudioUnitScope inScope,
								AudioUnitParameterID inParameterID,
								AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;
    
	if (inScope == kAudioUnitScope_Global)
	{
		
		if(inParameterID >= kNumberOfParameters)
			return kAudioUnitErr_InvalidParameter;
			
		outParameterInfo.flags = kAudioUnitParameterFlag_IsReadable;
		AUBase::FillInParameterName(outParameterInfo, kLabel[inParameterID], false);
		
		switch(inParameterID)
		{
			case kParam_OutputSquare:
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = 0;
				outParameterInfo.maxValue = 1;
				outParameterInfo.defaultValue = 0;
				outParameterInfo.flags |= kAudioUnitParameterFlag_IsWritable;
				break;
			case kParam_ClockTolerance:
				outParameterInfo.unit = kAudioUnitParameterUnit_Percent;
				outParameterInfo.minValue = 0.01;
				outParameterInfo.maxValue = 40.0;
				outParameterInfo.defaultValue = 15.0;
				outParameterInfo.flags |= kAudioUnitParameterFlag_IsWritable;
				break;
			default:
				result = kAudioUnitErr_InvalidParameter;
				break;
		}
	}
	else
	{
		result = kAudioUnitErr_InvalidParameter;
	}
    
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	sscau::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult sscau::GetPropertyInfo(	AudioUnitPropertyID inID,
								AudioUnitScope inScope,
								AudioUnitElement inElement,
								UInt32 & outDataSize,
								Boolean & outWritable)
{
	if(inScope == kAudioUnitScope_Global)
	{
		switch(inID)
		{
			case kAudioUnitProperty_CocoaUI:
				outWritable = false;
				outDataSize = sizeof(AudioUnitCocoaViewInfo);
				return noErr;
			case kAudioUnitProperty_SupportedNumChannels:
				outWritable = false;
				outDataSize = sizeof(AUChannelInfo);
				return noErr;	
			case kAudioUnitProperty_InPlaceProcessing:
				return kAudioUnitErr_InvalidProperty;	
			
			//auol-specific properties
			case kAudioOfflineUnitProperty_InputSize:
				outDataSize = sizeof(UInt64);
				outWritable = true;
				return noErr;
			case kAudioOfflineUnitProperty_OutputSize:
				outDataSize = sizeof(UInt64);
				outWritable = false;
				return noErr;
			case kAudioUnitOfflineProperty_StartOffset:
				outDataSize = sizeof(UInt64);
				outWritable = false;
				return noErr;
			case kAudioUnitOfflineProperty_PreflightRequirements:
				outDataSize = sizeof(UInt32);
				outWritable = false;
				return noErr;
			case kAudioUnitOfflineProperty_PreflightName:
				return kAudioUnitErr_InvalidProperty;
			
			//custom property
			case sscauProperty_NextFrameData:
				outWritable = false;
				outDataSize = sizeof(SSCAUFrameData);
				return noErr;
		}
	}
	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

/*
	Returns an AUChannelInfo struct that shows the number of i and o channels
	outInfo	[out]	output AUChannelInfo struct
	returns number of sets of support channels
*/
UInt32 sscau::SupportedNumChannels(const AUChannelInfo ** outInfo)
{
	static const AUChannelInfo myInfo[1] = {{1, 1}};
	if(outInfo)
		*outInfo = myInfo;
	return sizeof(myInfo) / sizeof(AUChannelInfo);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	sscau::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult sscau::GetProperty(AudioUnitPropertyID inID,
							AudioUnitScope inScope,
							AudioUnitElement inElement,
							void * outData )
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
		{
			case kAudioUnitProperty_CocoaUI:
			{
				
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.YSPf.audiounit.sscau") );

				if (bundle == NULL)
					return fnfErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
													CFSTR("CocoaViewFactory"), 
													CFSTR("bundle"), 
													NULL);
          
				if (bundleURL == NULL)
					return fnfErr;
                
				CFStringRef className = CFSTR("SSCAUViewFactory");
				AudioUnitCocoaViewInfo cocoaInfo = { bundleURL, className };
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;

				return noErr;
			}
			
			//offline properties
			case kAudioOfflineUnitProperty_OutputSize:
				//*(UInt64*)outData = (mNumInputSamples - mStartOffset);
				return noErr;
			case kAudioOfflineUnitProperty_InputSize:
				//*(UInt64*)outData = mNumInputSamples;
				return noErr;
			case kAudioUnitOfflineProperty_StartOffset:
				//*(UInt64*)outData = mStartOffset;
				return noErr;
			case kAudioUnitOfflineProperty_PreflightRequirements:
				*(UInt32*)outData = kOfflinePreflight_NotRequired;
				return noErr;
		/*	case kAudioUnitOfflineProperty_PreflightName:
				if (GetPreflightString (NULL)) {
					GetPreflightString ((CFStringRef*)outData);
					return noErr;
				}*/
			
			case sscauProperty_NextFrameData:
			{
				SSCAUFrameData data = GetNextFrameAnalysis();
				*((SSCAUFrameData *)outData) = data;
				//TODO: implement
				return noErr;
			}
		}
	}

	return AUEffectBase::GetProperty(inID, inScope, inElement, outData);
}

ComponentResult sscau::SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
							AudioUnitElement inElement, const void * inData, UInt32 inDataSize)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
		{
				//whenever these properties are set we *MAY* take this to mean the input has changed
				// at this point we require preflighting again...
			case kAudioOfflineUnitProperty_InputSize:
				if (inDataSize < sizeof(UInt64)) return kAudioUnitErr_InvalidPropertyValue;
				//mNumInputSamples = *(UInt64*)inData;
				return noErr;

			case kAudioUnitOfflineProperty_StartOffset:
				if (inDataSize < sizeof(UInt64)) return kAudioUnitErr_InvalidPropertyValue;
				//mStartOffset = *(UInt64*)inData;
				return noErr;
		}
	}
	return AUEffectBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
}

#pragma mark Analysis Log Functionality
/*
	Adds a bit of analysis data onto the queue
*/
void sscau::EnqueueFrameAnalysis(UInt32 index, SInt32 skew, UInt32 error)
{
	SSCAUFrameData sfd;
	sfd.index = index;
	sfd.skew = skew;
	sfd.error = error;
	EnqueueFrameAnalysis(sfd);
}

/*
	Adds a bit of analysis data onto the queue
*/
void sscau::EnqueueFrameAnalysis(SSCAUFrameData & back)
{
	analysis.push(back);
}

/*
	Returns the next frame analysis bit
	Returns a default SSCAUFrameData if no more frames are available
*/
SSCAUFrameData sscau::GetNextFrameAnalysis()
{
	SSCAUFrameData frame;
	if(analysis.empty())
		return frame;
		
	frame = analysis.front();
	analysis.pop();
	return frame;
}

#pragma mark sscauEffectKernel
/*
	Constructor
*/
sscau::sscauKernel::sscauKernel(AUEffectBase * inAudioUnit)
	: AUKernelBase(inAudioUnit), curFrame(NULL), frameDiffHistory(), zeroElapseHistory()
{
	curFrame = new LTCFrame();
	Reset();
}

sscau::sscauKernel::~sscauKernel()
{
	if(curFrame != NULL)
		delete curFrame;
}

/*
	Resets the kernel for use
*/
void sscau::sscauKernel::Reset()
{

	lastSign = 0;

	curFrame->clear();
	frameStartSample = 0;
	sampleIndex = 0;
	samplesSinceLastZero = 0;
	samplesSinceLastZero2 = 0;
	foundSyncWord = false;
	workingOnOne = false;
	foundFirstSyncWord = false;
	
	realTimeFrameDuration = GetSampleRate() / 30.0;
	clockCycle = realTimeFrameDuration / 80.0;
	clockVariationTolerance = 0.1;

	curFrameDuration = 0;
	frameDiffHistory.clear();
	zeroElapseHistory.clear();
	frameDiffChangedClock = false;
	zeroElapseChangedClock = false;
}

/*
	The beef of the kernel
*/
void sscau::sscauKernel::Process(	const Float32 * inSourceP,
							Float32 * inDestP,
							UInt32 inFramesToProcess,
							UInt32 inNumChannels, // for version 2 AudioUnits inNumChannels is always 1
							bool & ioSilence)
{

	//do maintenence
	MaintainState(inFramesToProcess);
	
	//get parameters
	bool outputSquared = GetParameter(kParam_OutputSquare);
	clockVariationTolerance = GetParameter(kParam_ClockTolerance) / 100.0;
	
	//analyze SMPTE carrier
	UInt32 numFrames = inFramesToProcess;
	const Float32 * sourceP = inSourceP;
	Float32 * destP = inDestP;
	short t;
	bool continueFrame = true;
	
#ifdef LOGTOFILE
	ofstream dump("/log.txt", ios::app);		//DEBUG
#endif
	
	while(numFrames-- > 0)
	{
		//counters
		samplesSinceLastZero++;
		sampleIndex++;
		curFrameDuration++;
	
		//check sign/orientation of wave
		t = Sign(*sourceP);
		//if zero crossing
		if((t * -1) == lastSign)
		{
			lastSign = t;
			t = 0;
			
			//check if this clock cycle indicates a "0"
			if(ClockTolerate(sampleIndex, sampleIndex - samplesSinceLastZero))
			{
				
				//record history of "0" width clock pulses
				if(foundSyncWord)
					ZeroElapseHistory(samplesSinceLastZero);
			
				if((workingOnOne) && (foundFirstSyncWord))
				{
					workingOnOne = false;
					//accumulate 0?
				
					//record error
					EnqueueFrameAnalysis(sampleIndex, clockCycle, sscauErr_FoundZeroWhenWorkingOnOne);
					
#ifdef LOGTOFILE
					dump << "\tfound 0 when workingOnOne @ " << sampleIndex << endl;
#endif
				}
				else
				{
					if(!foundSyncWord)
					{
						foundSyncWord = curFrame->foundSyncWord(0);
						if(foundSyncWord)
						{
							curFrameDuration = 0;
							foundFirstSyncWord = true;
							
							//record event
							EnqueueFrameAnalysis(sampleIndex, 0, sscauErr_FoundSyncWord);
#ifdef LOGTOFILE
							dump << "===> foundSyncWord" << endl;	//DEBUG
#endif

						}
					}
					else
					{
						continueFrame = curFrame->accumulateBit(0);
					}
				}
			}
			//check if this clock cycle indicates a "1"
			else if(ClockTolerate(sampleIndex, sampleIndex - samplesSinceLastZero, true))
			{
				if(workingOnOne)
				{
					if(!foundSyncWord)
					{
						//if we are looking for a sync word, add a "1", and see if now we have one
						foundSyncWord = curFrame->foundSyncWord(1);
						
						if(foundSyncWord)
						{
#ifdef LOGTOFILE
							dump << "===> foundSyncWord" << endl;	//DEBUG
#endif
							curFrameDuration = 0;
							foundFirstSyncWord = true;
							
							//record event
							EnqueueFrameAnalysis(sampleIndex, 0, sscauErr_FoundSyncWord);
						}
					}
					else
					{
						//recording the "1" Elapse History  hasn't proven neccesary, but it can't hurt
						ZeroElapseHistory(samplesSinceLastZero + samplesSinceLastZero2);
						continueFrame = curFrame->accumulateBit(1);
					}
					workingOnOne = false;
				}
				else
				{
					workingOnOne = true;
				}
			}
			else if(foundFirstSyncWord)
			{
				EnqueueFrameAnalysis(sampleIndex, clockCycle, sscauErr_ClockMiss);
#ifdef LOGTOFILE
				dump << "\tclock miss error @ " << sampleIndex << " (clockCycle: " << clockCycle << ")" << endl; //DEBUG
#endif
			}
			
			//see if accumulateBit indicated an interesting condition
			if(!continueFrame)
			{
				SSCAUFrameData sfd;
				sfd.index = sampleIndex;
#ifdef LOGTOFILE
				dump << "===> @" << sampleIndex;
#endif
				//-- if we got a full frame --
				if(curFrame->fullFrameParsed())
				{
					sfd.error |= sscauErr_FullFrameParsed;
#ifdef LOGTOFILE
					dump << " (fullFrameParsed)";
#endif
					//-- if the frame is apparently valid --
					if(curFrame->isValidSyncWord())
					{
						sfd.error |= sscauErr_FoundSyncWord;
#ifdef LOGTOFILE
						dump << " (isValidSyncWord)";
						dump << " (" << curFrame->getHours() << ":" << curFrame->getMinutes() << ":" << curFrame->getSeconds() << "." << curFrame->getFrames() << ")";
						dump << " (" << curFrameDuration << " - " << realTimeFrameDuration << ")";
#endif
						//record FrameDiffHistory
						FrameDiffHistory(curFrameDuration - realTimeFrameDuration);
					}
					else
					{
						foundSyncWord = false;
#ifdef LOGTOFILE
						dump << " (foundSyncWord = false)";
#endif
					}
					sfd.skew = curFrameDuration - realTimeFrameDuration;
#ifdef LOGTOFILE
					dump << " (frameDiff: " << sfd.skew << ")" << " (clockCycle: " << clockCycle << ")";
#endif
					
					//note the start of the next frame
					curFrameDuration = 0;
				}
				else
				{
					sfd.error = sscauErr_InvalidFrame;
#ifdef LOGTOFILE
					dump << " (INVALID)";
#endif
					//-- frame is garbage, but we hit a sync word --
					if(curFrame->curSyncWordValid())
					{
						sfd.error |= sscauErr_FoundSyncWord;
						sfd.skew = curFrameDuration - realTimeFrameDuration;
#ifdef LOGTOFILE
						dump << " (curSyncWordValid)";
						dump << " (frameDiff: " << sfd.skew << ")" << " (clockCycle: " << clockCycle << ")";
#endif
						curFrameDuration = 0;
					}
					else
					{
						foundSyncWord = false;
					}
				}
#ifdef LOGTOFILE
				dump << endl;
#endif
				EnqueueFrameAnalysis(sfd);
				curFrame->clear();
				continueFrame = true;
			}
			samplesSinceLastZero2 = samplesSinceLastZero;
			samplesSinceLastZero = 0;
			
		//if we didn't hit a 0 crossing
		}
		else
		{
			lastSign = t;
		}
		
		//actually pass signal
		if(outputSquared)
			*destP = (Float32)t * 0.5;
		else
			*destP = *sourceP;
		
		sourceP += inNumChannels;
		destP += inNumChannels;
	}
	
#ifdef LOGTOFILE
	dump.close();	//DEBUG
#endif

}

#pragma mark Kernel Thought Bubbles

/*
	Maintains the state of the kernel.
	Updates buffer sizes, changes other cool state information
*/
void sscau::sscauKernel::MaintainState(UInt32 inFramesToProcess)
{		
	
	if(lastSampleRate != GetSampleRate())
	{
		realTimeFrameDuration = GetSampleRate() / 30.0;
		
		lastSampleRate = GetSampleRate();
	}
		
	if(clockCycle == 0)
		clockCycle = realTimeFrameDuration / 80.0;

}

/*
	Returns true if the two values are within "lastClockCycle" of each other
	time1	the first time
	time2	the second time
	half		check if these times is half of a clock cycle
*/
bool sscau::sscauKernel::ClockTolerate(UInt32 time1, UInt32 time2, bool half)
{
	UInt32 timeDiff = (time1 > time2)?(time1 - time2):(time2 - time1);
	UInt32 clockDiff;
	UInt32 okVariation = clockVariationTolerance * clockCycle;
	
	if(half)
	{
		clockDiff = (timeDiff > (clockCycle / 2))?(timeDiff - (clockCycle / 2)):((clockCycle / 2) - timeDiff);
	}
	else
	{
		clockDiff = (timeDiff > clockCycle)?(timeDiff - clockCycle):(clockCycle - timeDiff);
	}
		
	return (clockDiff <= okVariation);
}

/*
	Records the frame diff history of the last few frames, and then analizes the history.
	If it finds a trend, then it adjusts the clock cycle accordingly
*/
void sscau::sscauKernel::FrameDiffHistory(SInt32 mostRecentFrameDiff)
{

	if(!(_SSCAU_USE_FRAMEDIFFHISTORY & _SSCAU_HISTORY_METHOD))
		return;

	frameDiffChangedClock = false;

	if(mostRecentFrameDiff == 0)
		return;
		
	frameDiffHistory.push_back(mostRecentFrameDiff);
	while(frameDiffHistory.size() > _SSCAU_FRAME_DIFF_HISTORY_SIZE)
		frameDiffHistory.pop_front();
	
	if(zeroElapseChangedClock)
		return;

	//calculate average
	SInt32 average = GetAverage(frameDiffHistory);
	//calculate slope
	SInt32 slope = GetSlope(frameDiffHistory);
	//calculate acceptable frame variation
	UInt32 okVariation = _SSCAU_FRAME_ADJUST_FACTOR * clockVariationTolerance * clockCycle * 80;

	if((slope > 0) && (abs(average) >= okVariation))
	{	//frames are getting bigger
		clockCycle += _SSCAU_FRAME_ADJUST_FACTOR * clockVariationTolerance * clockCycle;
		frameDiffChangedClock = true;
	}
	else if((slope < 0)	&& (abs(average) >= okVariation))
	{	//frames are getting smaller
		clockCycle -= _SSCAU_FRAME_ADJUST_FACTOR * clockVariationTolerance * clockCycle;
		frameDiffChangedClock = true;
	}
		
}

/*
	Records the history of elapsed samples between zero crossings and then analyzes it.
	If it a trend is noticed, then the clockCycle is adjusted accordingly
	*NOTE* only record history of known "0" pulses, it can't deal with "1" pulses right now
*/
void sscau::sscauKernel::ZeroElapseHistory(SInt32 mostRecentZeroElapse)
{

	if(!(_SSCAU_USE_ZEROELAPSEHISTORY & _SSCAU_HISTORY_METHOD))
		return;

	zeroElapseChangedClock = false;

	if(mostRecentZeroElapse == 0)
		return;
		
#ifdef _SSCAU_ZERO_DEBUG_LOG
	ofstream dump("/log_zeh.txt", ios::out | ios::app);
	dump << "zeh: " << mostRecentZeroElapse;
#endif
		
	zeroElapseHistory.push_back(mostRecentZeroElapse);
	while(zeroElapseHistory.size() > _SSCAU_ZERO_ELAPSE_HISTORY_SIZE)
		zeroElapseHistory.pop_front();
		
	if(frameDiffChangedClock)
		return;
		
	SInt32 average = GetAverage(zeroElapseHistory) - clockCycle;
	SInt32 okVariation = _SSCAU_ZERO_ADJUST_FACTOR * clockVariationTolerance * clockCycle;
	
#ifdef _SSCAU_ZERO_DEBUG_LOG
	dump << " (avg: " << average << ") (okVariation: " << okVariation << ")";
#endif
	
	if(average > okVariation)
	{
		clockCycle += _SSCAU_ZERO_ADJUST_FACTOR * clockVariationTolerance * clockCycle;
		zeroElapseChangedClock = true;
	}
	else if(average < (-1 * okVariation))
	{
		clockCycle -= _SSCAU_ZERO_ADJUST_FACTOR * clockVariationTolerance * clockCycle;
		zeroElapseChangedClock = true;
	}
#ifdef _SSCAU_ZERO_DEBUG_LOG
	if(zeroElapseChangedClock)
		dump << " cc -> " << clockCycle;
	dump << endl;
	dump.close();
#endif
}

#pragma mark Kernel Utilties

/*
	Returns +1 if a number is positive or zero, -1 if a number is negative
*/
short sscau::sscauKernel::Sign(Float32 value)
{
	if(value >= 0)
		return 1;
	
	return -1;
}

/*
	Returns the absolute value of an SInt32
*/
UInt32 sscau::sscauKernel::abs(SInt32 value)
{
	if(value < 0)
		value *= -1;
		
	return (UInt32)value;
}

/*
	Returns the average of the numbers in a given deque of SInt32s
*/
SInt32 sscau::sscauKernel::GetAverage(deque<SInt32> & q)
{

	SInt32 average = 0;
	deque<SInt32>::iterator i;
	for(i = q.begin(); i != q.end(); i++)
		average += (SInt32)(*i);
		
	return average / (SInt32)q.size();
}

/*	
	Returns the slope of the numbers in a given deque of SInt32s by the following formula
	([back] - [front]) / numberOfElements
*/
SInt32 sscau::sscauKernel::GetSlope(deque<SInt32> & q)
{
	//calculate slope
	return (SInt32)(q[q.size() - 1] - q[0]) / (SInt32)q.size();
}

/*
	Enqueues a frame analysis into my owning audio unit's analysis queue
*/
void sscau::sscauKernel::EnqueueFrameAnalysis(UInt32 index, SInt32 skew, UInt32 error)
{
	((sscau*)mAudioUnit)->EnqueueFrameAnalysis(index, skew, error);
}

/*
	Enqueues a frame analysis into my owning audio unit's analysis queue
*/
void sscau::sscauKernel::EnqueueFrameAnalysis(SSCAUFrameData & data)
{
	((sscau*)mAudioUnit)->EnqueueFrameAnalysis(data);
}
