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
//  SFEngine.h
//  StretchFix
//
//  Created by Kevin Dixon on 3/12/08.
//  Copyright 2008 Yano Signal Processing. All rights reserved.
//

#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <AudioUnit/AudioUnit.h>
#include <Cocoa/Cocoa.h>

#include <vector>
using std::vector;

#include "../sscau/sscauAPI.h"

#ifndef _SFENGINE_H_
#define _SFENGINE_H_

//Engine Status info
enum {
	kSFEngineStatus_NotReady		= 0,
	kSFEngineStatus_Ready		= 1,
	kSFEngineStatus_Processing	= 2,
	kSFEngineStatus_Done		= 3,
	kSFEngineStatus_Abort		= 4
};

//OpenAudioFiles return codes
enum {
	kSFEOAFErr_InputFileFailed	= 1,
	kSFEOAFErr_InputNotMono		= 2,
	kSFEOAFErr_OutputFileFailed	= 4
};

#define SSCAU_DESC { kAudioUnitType_OfflineEffect, 'ssca', 'YSPf' }
//#define VARISPEED_DESC { kAudioUnitType_OfflineEffect, kAudioUnitSubType_Varispeed, kAudioUnitManufacturer_Apple }
#define VARISPEED_DESC { kAudioUnitType_OfflineEffect, 'srvs', 'YSPf' }

class SFEngine {

	protected:
		AUGraph mGraph;
		AUNode mVarispeedNode, mSSCAUNode;
		AudioUnit mVarispeedUnit, mSSCAUUnit;
		
		//used to manipulate files
		ExtAudioFileRef inputFile, outputFile;
		bool inputFileEOFReached;
		
		//stores the analysis
		vector<SSCAUFrameData> analysis;
		UInt32 errorFrameCount;
		
		//process progress indicators
		SInt64 currentFilePosition;
		SInt64 currentFileLength;
		UInt64 currentFileNumber;
		UInt64 totalFilesToProcess;
		int processStatus;
		
	public:
		SFEngine();
		~SFEngine();
		
		bool Initialize();
	protected:
		void InitAudioUnits(); 
	
	public:
	
#pragma mark Processing Status
		void AbortCurrentProcess();

		double GetCurrentProgress();
		double GetOverallProgress();
		
		int ProcessStatus();
	protected:
		void SetProcessStatus(int s);
	
	public:	
	
#pragma mark Processing
		bool Analyze(NSString * smpteFilePath);

		void Destretch(NSMutableArray * files, NSString * outputDir, SInt32 exclude = -1);
		
		static OSStatus AudioSupplyCallback(void * inRefCon, AudioUnitRenderActionFlags * ioActionFlags,
									const AudioTimeStamp * inTimeStamp, UInt32 inBusNumber,
									UInt32 inNumFrames, AudioBufferList * ioData);
		OSStatus SupplyInputAudioData(AudioUnitRenderActionFlags & ioActionFlags, const AudioTimeStamp & inTimeStamp,
								UInt32 inBusNumber, UInt32 inNumFrames, AudioBufferList * ioData);
														
	protected:
		OSStatus OpenAudioFiles(NSString * input, NSString * outputDir, AudioStreamBasicDescription * client);

		void DestretchPreflight(UInt32 kNumFramesPerSlice);
		OSStatus DestretchConfigureVarispeed(const AudioStreamBasicDescription * ioStream, UInt32 kNumFramesPerSlice);
			
		inline Float32 SkewToVarispeedRatio(SInt32 skew, UInt32 smpteFrameSize);
		
		inline OSStatus SetVarispeedInputSize(UInt64 inputNumFrames);
	
#pragma mark Accessors 
	public:
		AudioUnit GetSSCAUUnit();
		AudioUnit GetVarispeedUnit();
		
#pragma mark Frame Analysis Data
		bool GetFrameAt(UInt32 i, SSCAUFrameData * sfd) const;
		UInt32 GetFrameCount() const;
		UInt32 GetErrorFrameCount() const;
		void ClearFrameAnalysis();
	protected:
		void AddFrameAnalysis(const SSCAUFrameData & sfd);
		
#pragma mark AUGraph Handlers
		void GraphCreate();
		void GraphDestroy();
		void GraphStart();
		void GraphStop();
		
#pragma mark Utility
		FSRef StringToFSRef(NSString * filePath);
};

#endif