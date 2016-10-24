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
	sscau.h
	SMPTE-Stretch-Calc Audio Unit
	
	01/29/2008
	Kevin C. Dixon
	Yano Signal Processing
*/

#include "AUEffectBase.h"

#include "sscauVersion.h"
#include "sscauParams.h"
#include "sscauAPI.h"
#include "LTCFrame.hpp"


//	#define LOGTOFILE
//	#define _SSCAU_ZERO_DEBUG_LOG

#include <queue>
using std::queue;
#include <deque>
using std::deque;

#ifdef LOGTOFILE
	#include <fstream>
	using std::ofstream;
#endif

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif


#ifndef __sscau_h__
#define __sscau_h__

#pragma mark Parameter Names
static CFStringRef kLabel[kNumberOfParameters] =
{
	//kParam_OutputSquare
	CFSTR("Output Squared Wave"),
	//kParam_ClockTolerance
	CFSTR("Clock Variation Tolerance"),
};

#pragma mark Historical Clock Tracking Settings

#define _SSCAU_USE_FRAMEDIFFHISTORY	1
#define _SSCAU_USE_ZEROELAPSEHISTORY	2
#define _SSCAU_HISTORY_METHOD			_SSCAU_USE_ZEROELAPSEHISTORY

#define _SSCAU_FRAME_DIFF_HISTORY_SIZE	3
#define _SSCAU_FRAME_ADJUST_FACTOR		0.5

#define _SSCAU_ZERO_ELAPSE_HISTORY_SIZE 10
#define _SSCAU_ZERO_ADJUST_FACTOR		0.5

#pragma mark SSCAU
class sscau : public AUEffectBase
{
private:
	queue<SSCAUFrameData> analysis;

public:
	sscau(AudioUnit component);
#if AU_DEBUG_DISPATCHER
	virtual ~sscau () { delete mDebugDispatcher; }
#endif

	virtual ComponentResult Reset(AudioUnitScope inScope, AudioUnitElement inElement);
	
	virtual AUKernelBase * NewKernel() { return new sscauKernel(this); }
	
	virtual ComponentResult GetParameterValueStrings(	AudioUnitScope inScope,
											AudioUnitParameterID inParameterID,
											CFArrayRef * outStrings);
    
	virtual ComponentResult GetParameterInfo(	AudioUnitScope inScope,
										AudioUnitParameterID inParameterID,
										AudioUnitParameterInfo & outParameterInfo);
    
	virtual ComponentResult GetPropertyInfo(AudioUnitPropertyID inID,
									AudioUnitScope inScope,
									AudioUnitElement inElement,
									UInt32 & outDataSize,
									Boolean & outWritable );
	
	virtual ComponentResult GetProperty(	AudioUnitPropertyID inID,
									AudioUnitScope inScope,
									AudioUnitElement inElement,
									void * outData);
	virtual ComponentResult SetProperty(	AudioUnitPropertyID inID,
									AudioUnitScope inScope,
									AudioUnitElement inElement,
									const void * inData,
									UInt32 inDataSize);
	
	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outInfo);
										
 	virtual bool SupportsTail() { return false; }
	
	virtual ComponentResult Version() { return ksscauVersion; }
	
#pragma mark Analysis Log Functionality
	void EnqueueFrameAnalysis(UInt32 index, SInt32 skew, UInt32 error);
	void EnqueueFrameAnalysis(SSCAUFrameData & back);
	SSCAUFrameData GetNextFrameAnalysis();
	
protected:
	class sscauKernel : public AUKernelBase
	{
	private:
#pragma mark sscauKernel: data members

		UInt32 lastSampleRate;
		short lastSign;
		
		LTCFrame * curFrame;			//contains the data of the current frame
		UInt32 frameStartSample;			//marks the sample index where the current frame began
		UInt32 sampleIndex;				//current sample index
		UInt32 samplesSinceLastZero;		//sample index of the last zero crossing
		UInt32 samplesSinceLastZero2;		//sample index of the zero crossing before the last one
		bool foundSyncWord;				//true if we have encountered a sync word and believe it to be valid
		bool workingOnOne;				//used to notate half of a "1" series of pulses
		bool foundFirstSyncWord;			//true if we have ever encountered a sync word
		
		UInt32 clockCycle;				//number of samples in current clock cycle
		Float32 clockVariationTolerance;	//percentage +/- of acceptable variation

		UInt32 realTimeFrameDuration;		//in samples
		UInt32 curFrameDuration;			//in samples
		deque<SInt32> frameDiffHistory;
		deque<SInt32> zeroElapseHistory;
		bool frameDiffChangedClock, zeroElapseChangedClock;

		
	public:
		sscauKernel(AUEffectBase * inAudioUnit);
		~sscauKernel();

		virtual void Process(	const Float32 * inSourceP,
							Float32 * inDestP,
							UInt32 inFramesToProcess,
							UInt32 inNumChannels,
							bool & ioSilence);
		
		virtual void Reset();
		
	protected:
		void MaintainState(UInt32 inFramesToProcess);
		
		bool ClockTolerate(UInt32 time1, UInt32 time2, bool half = false);
		void FrameDiffHistory(SInt32 mostRecentFrameDiff);
		void ZeroElapseHistory(SInt32 mostRecentZeroElapse);
		
		inline short Sign(Float32 value);
		inline UInt32 abs(SInt32 value);
		inline SInt32 GetAverage(deque<SInt32> & q);
		inline SInt32 GetSlope(deque<SInt32> & q);
		void EnqueueFrameAnalysis(UInt32 index, SInt32 skew, UInt32 error);
		void EnqueueFrameAnalysis(SSCAUFrameData & data);
	};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif