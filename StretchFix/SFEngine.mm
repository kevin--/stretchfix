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
//  SFEngine.mm
//  StretchFix
//
//  Created by Kevin Dixon on 3/12/08.
//  Copyright 2008 Yano Signal Processing. All rights reserved.
//

#include "SFEngine.h"
#include <Cocoa/Cocoa.h>

#pragma mark Constructors
SFEngine::SFEngine() {
	analysis = vector<SSCAUFrameData>();
	currentFileLength = 1;
	totalFilesToProcess = 1;
	errorFrameCount = 0;
	
	inputFile = outputFile = 0;
	
	SetProcessStatus(kSFEngineStatus_NotReady);
}

SFEngine::~SFEngine() {
	GraphDestroy();
}

bool SFEngine::Initialize() {

	Component component = 0;
	//check for AUVarispeed
	ComponentDescription varispeed = VARISPEED_DESC;
	component = FindNextComponent(component, &varispeed);
	if(component == 0) {
		NSRunAlertPanel(@"Missing Component", @"The Varispeed Audio Unit could not be found.\n\nPlease make sure it is installed.", @"Quit", nil, nil);
		return false;
	}
	//check for SSCAU
	component = 0;
	ComponentDescription sscau = SSCAU_DESC;
	component = FindNextComponent(component, &sscau);
	if(component == 0) {
		NSRunAlertPanel(@"Missing Component", @"The SMPTE-Stretch-Calc Audio Unit could not be found.\n\nPlease make sure it is installed.", @"Quit", nil, nil);
		return false;
	}
	
	//Create AU Graph
	GraphCreate();
	//setup AUs
	InitAudioUnits();
	
	SetProcessStatus(kSFEngineStatus_Ready);
	
	return true;
}

void SFEngine::InitAudioUnits()  {
	//setup SSCAU (mSSCAUUnit must be initialized to draw UI)
	ComponentDescription sscau = SSCAU_DESC;
	AUGraphNewNode(mGraph, &sscau, 0, NULL, &mSSCAUNode);
	AUGraphGetNodeInfo(mGraph, mSSCAUNode, NULL, NULL, NULL, &mSSCAUUnit);
	//AUGraphUpdate(mGraph, NULL);
	
	//setup varispeed
	ComponentDescription vari = VARISPEED_DESC;
	AUGraphNewNode(mGraph, &vari, 0, NULL, &mVarispeedNode);
	AUGraphGetNodeInfo(mGraph, mVarispeedNode, NULL, NULL, NULL, &mVarispeedUnit);
	AUGraphUpdate(mGraph, NULL);
	
	//setup bus formats
//	AudioUnitSetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, , );	

}

#pragma mark Processing Status

void SFEngine::AbortCurrentProcess()
{
	SetProcessStatus(kSFEngineStatus_Abort);
}

double SFEngine::GetCurrentProgress()
{
	return (double)currentFilePosition / (double)currentFileLength;;
}

double SFEngine::GetOverallProgress()
{
	return (double)currentFileNumber / (double)totalFilesToProcess;
}

int SFEngine::ProcessStatus()
{
	return processStatus;
}

void SFEngine::SetProcessStatus(int s)
{
	processStatus = s;
}

#pragma mark Processing


/*
	Analyzes a given audio file.
	Returns true if the analysis returned some data, returns false if an error occurred, or no data was reaped.
*/
bool SFEngine::Analyze(NSString * smpteFilePath)
{
	//set the data for progress updates
	totalFilesToProcess = 1;
	currentFileNumber = 1;

	//open file and do error checking
	AudioStreamBasicDescription client = { };
	OSStatus err = OpenAudioFiles(smpteFilePath, NULL, &client);
	if(err == kSFEOAFErr_InputFileFailed) {
		NSRunAlertPanel(@"Format Not Supported", @"The file selected for analysis is not an audio file, or not a supported format.", nil, nil, nil);
		return false;
	} else if(err == kSFEOAFErr_InputNotMono) {
		NSRunAlertPanel(@"Input File Must Be Mono", @"All files must contain only one channel.", nil, nil, nil);
		return false;
	}
	
	SetProcessStatus(kSFEngineStatus_Processing);
		
	//setup SSCAU io streams
	AudioUnitSetProperty(mSSCAUUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &client, sizeof(client));
	AudioUnitSetProperty(mSSCAUUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &client, sizeof(client));
	//setup SSCAU callback
	AURenderCallbackStruct cb = { SFEngine::AudioSupplyCallback, this };
	AudioUnitSetProperty(mSSCAUUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &cb, sizeof(cb));
	err = AudioUnitInitialize(mSSCAUUnit);
	if(err != noErr) {
		SetProcessStatus(kSFEngineStatus_Ready);
		return false;
	}
	AudioUnitReset(mSSCAUUnit, kAudioUnitScope_Global, 0);
	
	//process data for analysis
	UInt32 kNumFramesPerSlice = 512;
	AudioTimeStamp curTime;
	curTime.mSampleTime = 0;
	curTime.mFlags = kAudioTimeStampSampleTimeValid;
	
	//setup buffers to store the output from the SSCAU
	AudioBufferList * theAudioData = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers[1]));
	theAudioData->mNumberBuffers = 1;
	theAudioData->mBuffers[0].mNumberChannels = 1;
	theAudioData->mBuffers[0].mDataByteSize = kNumFramesPerSlice * sizeof(Float32);
	
	inputFileEOFReached = false;
	
	//RENDER LOOP - run until the SMPTE file is out of data
	while((!inputFileEOFReached) && (processStatus != kSFEngineStatus_Abort)) {
		AudioUnitRenderActionFlags actionFlags = 0;
		theAudioData->mBuffers[0].mData = NULL;
		
		err = AudioUnitRender(mSSCAUUnit, &actionFlags, &curTime, 0, kNumFramesPerSlice, theAudioData);
		
		curTime.mSampleTime += kNumFramesPerSlice;
	}

	//only transfer new data if not aborted
	if(processStatus != kSFEngineStatus_Abort) {
		ClearFrameAnalysis();
		//get data from the SSCAU
		SSCAUFrameData current;
		current.error = sscauErr_ClockMiss;	//set to something other than sscauErr_NoFrameData
		UInt32 sizeofCurrent = sizeof(current);
	
		while(current.error != sscauErr_NoFrameData) {
			err = AudioUnitGetProperty(mSSCAUUnit, sscauProperty_NextFrameData, kAudioUnitScope_Global, 0, &current, &sizeofCurrent);
			if(err != noErr)
				break;
		
			if(current.error != sscauErr_NoFrameData) {
				AddFrameAnalysis(current);
				if((current.error & sscauErr_FoundZeroWhenWorkingOnOne) || 
					(current.error & sscauErr_ClockMiss)) {
					
					errorFrameCount++;
				}
			}
		}
	}
	
	//clean up
	free(theAudioData);
	ExtAudioFileDispose(inputFile);
	AudioUnitUninitialize(mSSCAUUnit);
	
	SetProcessStatus(kSFEngineStatus_Done);
	return (GetFrameCount() > 0);
}

/*
	Destretches a given array of files
*/
void SFEngine::Destretch(NSMutableArray * files, NSString * outputDir, SInt32 exclude) {
	
	//determine number of files to process for progress purposes
	totalFilesToProcess = [files count] - ((exclude >= 0) ? 1 : 0);
	currentFileNumber = 0;
	
	SetProcessStatus(kSFEngineStatus_Processing);
		
	for(UInt32 fileIndex = 0; fileIndex < [files count]; fileIndex++) {
		if(processStatus == kSFEngineStatus_Abort)
			return;
	
		//skip the current file if it is the SMPTE file, and should not be destretched
		if((SInt32)fileIndex == exclude)
			continue;
			
		currentFileNumber += 1;
	
		//open the audio files
		AudioStreamBasicDescription client;
		NSString * currentFilePath = (NSString*)[files objectAtIndex:fileIndex];
		ComponentResult err = OpenAudioFiles(currentFilePath, outputDir, &client);
		if(err == kSFEOAFErr_InputFileFailed)
		{
			NSRunAlertPanel(@"Error opening file.", [NSString stringWithFormat:@"The file \"%s\" could not be opened, or may be an unsupported format.", [[currentFilePath lastPathComponent] cString]], nil, nil, nil);
			continue;
		}
		else if(err == kSFEOAFErr_InputNotMono)
		{
			NSRunAlertPanel(@"Input file must be mono.", [NSString stringWithFormat:@"The file \"%s\" could not be processed, because it is not a single-channel audio file.", [[currentFilePath lastPathComponent] cString]], nil, nil, nil);
			continue;
		}
		else if(err == kSFEOAFErr_OutputFileFailed)
		{
			NSRunAlertPanel(@"Could not create output file.", [NSString stringWithFormat:@"The output file for \"%s\" could not be created in the directory \"%s\".", [[currentFilePath lastPathComponent] cString], [outputDir cString]], nil, nil, nil);
			continue;
		}
		
		//setup time stamp
		AudioTimeStamp curTime = { };
		curTime.mSampleTime = 0;
		curTime.mFlags = kAudioTimeStampSampleTimeValid;		
		
		err = AudioUnitUninitialize(mVarispeedUnit);
		
		err = DestretchConfigureVarispeed(&client, 32767);
		
		//initialize the Varispeed
		err = AudioUnitInitialize(mVarispeedUnit);
		if(err != noErr)
		{
			AbortCurrentProcess();
			return;
		}
		
		//PREFLIGHT the Varispeed
		DestretchPreflight(512);

		//DEBUG
		//calculate the real smpte frame size. This could be more accurate, e.g. for drop frame LTC, but should be acceptable
		UInt32 kSMPTEFrameSize = client.mSampleRate / 30;
						
		//setup buffers to store the output from the Varispeed
		AudioBufferList * theAudioData = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers[1]));
		theAudioData->mNumberBuffers = 1;
			
		inputFileEOFReached = false;
	
		SSCAUFrameData sfd;
		UInt32 analysisIndex = 0;
		UInt32 numFramesThisSlice = 0;
		//run until the input file is out of data
		while((!inputFileEOFReached) && (processStatus != kSFEngineStatus_Abort)) {
			
			if(!GetFrameAt(analysisIndex++, &sfd))
			{
				//if we failed to get a frame of analysis data, we must be in a bad state
				AbortCurrentProcess();
				return;
			}
			
			//if the frame we just pulled has good skew information...set the varispeed rate, else leave it alone
			if(sfd.error == (sscauErr_FullFrameParsed | sscauErr_FoundSyncWord))
			{
				Float32 varispeedRate = SkewToVarispeedRatio(sfd.skew, kSMPTEFrameSize);
				//set the kParam_Ratio on the varispeed
				err = AudioUnitSetParameter(mVarispeedUnit, 0, kAudioUnitScope_Global, 0, varispeedRate, 0);
			}
			
			//calculate the number of frames this slice
			numFramesThisSlice = sfd.index - curTime.mSampleTime;
			err = SetVarispeedInputSize(numFramesThisSlice);
		
			//reset buffers
			theAudioData->mBuffers[0].mNumberChannels = 1;
			theAudioData->mBuffers[0].mDataByteSize = numFramesThisSlice * sizeof(Float32);
			theAudioData->mBuffers[0].mData = NULL;
			
			//action to perform
			AudioUnitRenderActionFlags actionFlags = kAudioOfflineUnitRenderAction_Render;

			//TODO: schedule parameters
			
			err = AudioUnitRender(mVarispeedUnit, &actionFlags, &curTime, 0, numFramesThisSlice, theAudioData);
				
			UInt32 returnedFrames = theAudioData->mBuffers[0].mDataByteSize / sizeof(Float32);
			
			if((returnedFrames > 0) && (err == noErr)) //&& (actionFlags & kAudioOfflineUnitRenderAction_Complete)) { //&& (err != noErr))
			{
				err = ExtAudioFileWrite(outputFile, returnedFrames, theAudioData);
				//AudioUnitReset(mVarispeedUnit, kAudioUnitScope_Global, 0);
			}
		
			curTime.mSampleTime += numFramesThisSlice;
		}

		//clean up
		free(theAudioData);
		AudioUnitUninitialize(mVarispeedUnit);
		ExtAudioFileDispose(inputFile);
		ExtAudioFileDispose(outputFile);
	}
	
	SetProcessStatus(kSFEngineStatus_Done);
	return;
}

#pragma mark Processing (Support)
/*
	Converts an amount of skew to the Varispeed param PlaybackRate to compensate appropriately
	skew				skew for a given frame (in samples)
	smpteFrameSize		the size of a SMPTE frame in samples, for the current sampling rate
	returns a value for the kParam_Ratio to compensate for such a skew
*/
Float32 SFEngine::SkewToVarispeedRatio(SInt32 skew, UInt32 smpteFrameSize) {
	return  (Float32)smpteFrameSize / (Float32)(smpteFrameSize + skew);
}

/*
	This is a STATIC method called by AURender so it can suck bytes
*/
OSStatus SFEngine::AudioSupplyCallback(void * inRefCon, AudioUnitRenderActionFlags * ioActionFlags,
							const AudioTimeStamp * inTimeStamp, UInt32 inBusNumber,
							UInt32 inNumFrames, AudioBufferList * ioData)
{
	SFEngine * This = (SFEngine*)inRefCon;
	return This->SupplyInputAudioData(*ioActionFlags, *inTimeStamp, inBusNumber, inNumFrames, ioData);
}

/*
	This method is invoked by AudioSupplyCallback which actually pulls the data from the file.
*/
OSStatus SFEngine::SupplyInputAudioData(AudioUnitRenderActionFlags & ioActionFlags, 
								const AudioTimeStamp & inTimeStamp,
								UInt32 inBusNumber, UInt32 inNumFrames, AudioBufferList * ioData)
{

	ExtAudioFileSeek(inputFile, (SInt64)inTimeStamp.mSampleTime);
	ExtAudioFileRead(inputFile, &inNumFrames, ioData);
	
	inputFileEOFReached = (inNumFrames == 0);
	
	//update current file position for progress meter
	currentFilePosition = (inTimeStamp.mSampleTime + inNumFrames);
		
	return noErr;
}

/*
	Opens a pair of files to read and write audio data to.
	The output file is created as an AIFF file
	input	string representing path to input file
	output	string representing the directory to create the output file in, can be NULL for no output file
	client	the Stream Description of the client interface between the file's data and the audio units
	returns false if the operation was a failure, with all files closed
*/
OSStatus SFEngine::OpenAudioFiles(NSString * input, NSString * outputDir, AudioStreamBasicDescription * client) {

	FSRef fileSystemFile = StringToFSRef(input);
	ExtAudioFileOpen(&fileSystemFile, &inputFile);
	
	if(inputFile == 0)
		return kSFEOAFErr_InputFileFailed;

	//get the input file format
	AudioStreamBasicDescription inFileStream = { };
	UInt32 sizeofDesc = sizeof(inFileStream);
	ExtAudioFileGetProperty(inputFile, kExtAudioFileProperty_FileDataFormat, &sizeofDesc, &inFileStream);
	
	//fail if input is no mono
	if(inFileStream.mChannelsPerFrame > 1) {
		ExtAudioFileDispose(inputFile);
		return kSFEOAFErr_InputNotMono;
	}
	
	//get length of file in frames
	UInt32 sizeofCFL = sizeof(currentFileLength);
	ExtAudioFileGetProperty(inputFile, kExtAudioFileProperty_FileLengthFrames, &sizeofCFL, &currentFileLength);

	
	//setup client format - retaining native sample rate
	(*client) = inFileStream;
	(*client).mFormatID = kAudioFormatLinearPCM;
	(*client).mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
 	(*client).mBytesPerPacket = 4;
	(*client).mFramesPerPacket = 1;
	(*client).mBytesPerFrame = 4;
	(*client).mChannelsPerFrame = 1;
	(*client).mBitsPerChannel = sizeof(Float32) * 8;
	
	//set the client format for the input file
	ExtAudioFileSetProperty(inputFile, kExtAudioFileProperty_ClientDataFormat, sizeof(*client), client);
	AudioChannelLayout mono = { kAudioChannelLayoutTag_Mono };
	ExtAudioFileSetProperty(inputFile, kExtAudioFileProperty_ClientChannelLayout, (UInt32)sizeof(mono), &mono);
	
	//skip the rest of the function if no output file is needed
	if(outputDir == NULL)
		return noErr;
		
	//keep the good bits from the input file stream, and configure for AIFF
	AudioStreamBasicDescription outFileStream = inFileStream;
	outFileStream.mFormatID = kAudioFormatLinearPCM;
	outFileStream.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	
	//create output file
	fileSystemFile = StringToFSRef(outputDir);
	NSString * fileName = @"new_";
	fileName = [fileName stringByAppendingString:[[input lastPathComponent] stringByDeletingPathExtension]];
	fileName = [fileName stringByAppendingString:@".aif"];
	CFStringRef cfFileName = (CFStringRef)fileName;
	
	OSStatus error = ExtAudioFileCreateNew(&fileSystemFile, cfFileName, kAudioFileAIFFType, &outFileStream, &mono, &outputFile);
	
	//if we couldn't open the output file, fail
	if((outputFile == 0) || error) {
		ExtAudioFileDispose(inputFile);
		return kSFEOAFErr_OutputFileFailed;
	}
	
	//set the client data for the output file
	ExtAudioFileSetProperty(outputFile, kExtAudioFileProperty_ClientDataFormat, sizeof(*client), client);
	ExtAudioFileSetProperty(outputFile, kExtAudioFileProperty_ClientChannelLayout, (UInt32)sizeof(mono), &mono);
	
	return noErr;
}

/*
	Performs pre-flighting on the varispeed if neccesary
*/
void SFEngine::DestretchPreflight(UInt32 kNumFramesPerSlice) {
	UInt32 requirements;
	UInt32 size = sizeof(requirements);
	AudioUnitGetProperty(mVarispeedUnit, kAudioUnitOfflineProperty_PreflightRequirements, kAudioUnitScope_Global, 0, &requirements, &size);

	if(requirements) {
		//setup buffers to process preflighting with the Varispeed
		AudioBufferList * theAudioData = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers[1]));
		theAudioData->mNumberBuffers = 1;
		theAudioData->mBuffers[0].mNumberChannels = 1;
		theAudioData->mBuffers[0].mDataByteSize = kNumFramesPerSlice * sizeof(Float32);
		
		inputFileEOFReached = false;
		
		AudioTimeStamp curTime = { };
		curTime.mSampleTime = 0;
		curTime.mFlags = kAudioTimeStampSampleTimeValid;	
		
		while(processStatus != kSFEngineStatus_Abort) {
			AudioUnitRenderActionFlags renderFlags = kAudioOfflineUnitRenderAction_Preflight;
			AudioUnitRender(mVarispeedUnit, &renderFlags, &curTime, 0, kNumFramesPerSlice, theAudioData);
			curTime.mSampleTime += kNumFramesPerSlice;
			
			if(renderFlags & kAudioOfflineUnitRenderAction_Complete)
				break;
		}
		
		free(theAudioData);
	}
}

/*
	Configures the Varispeed with its io stream data, and callback proc
	ioStream			The stream description for input and output
	kNumFramesPerSlice	the number of frames per slice.
	returns the result from the AudioUnitSetProperty call that sets the callback
*/
OSStatus SFEngine::DestretchConfigureVarispeed(const AudioStreamBasicDescription * ioStream, UInt32 kMaxFramesPerSlice) {
	
	//AudioChannelLayout mono = { kAudioChannelLayoutTag_Mono };
	
	OSStatus err;
	
	//--input
	err = AudioUnitSetProperty(mVarispeedUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, ioStream, sizeof(*ioStream));
//	err = AudioUnitSetProperty(mVarispeedUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Input, 0, &mono, sizeof(mono));
	
	//--output
//	AudioStreamBasicDescription outStream = (*ioStream);
//	outStream.mSampleRate = 22050;
	err = AudioUnitSetProperty(mVarispeedUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, ioStream, sizeof(*ioStream));
//	err = AudioUnitSetProperty(mVarispeedUnit, kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Output, 0, &mono, sizeof(mono));
	
	//describe the size of the data chunks we'll be using
	err = AudioUnitSetProperty(mVarispeedUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &kMaxFramesPerSlice, sizeof(kMaxFramesPerSlice));
//	err = SetVarispeedInputSize(kMaxFramesPerSlice);
		
	//tell Varispeed where to get its data from
	AURenderCallbackStruct cb = { SFEngine::AudioSupplyCallback, this };
	return AudioUnitSetProperty(mVarispeedUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &cb, sizeof(cb));
}

/*
	Configures the Varispeed's input size
	inputNumFrames		set input size to this value
*/
OSStatus SFEngine::SetVarispeedInputSize(UInt64 inputNumFrames)
{
	return AudioUnitSetProperty(mVarispeedUnit, kAudioOfflineUnitProperty_InputSize, kAudioUnitScope_Global, 0, &inputNumFrames, sizeof(inputNumFrames));
}

#pragma mark Accessors
/*
	Returns the AudioUnit id for the SSCAU
*/
AudioUnit SFEngine::GetSSCAUUnit() {
	return mSSCAUUnit;
}

AudioUnit SFEngine::GetVarispeedUnit() {
	return mVarispeedUnit;
}

#pragma mark Frame Analysis Data

/*
	Returns a frame analysis at a given index
	inIndex	index to get
	output	pointer to SSCAUFrameData requested
	returns false if i is out of bounds
*/
bool SFEngine::GetFrameAt(UInt32 inIndex, SSCAUFrameData * output) const {
	if(inIndex >= analysis.size())
		return false;
	
	(*output) = analysis[inIndex];
	
	return true;
}

/*
	Returns the number of frame analysis data
*/
UInt32 SFEngine::GetFrameCount() const {
	return (UInt32)analysis.size();
}

/*
	Returns the number of analysis frames that were determined to be errors
*/
UInt32 SFEngine::GetErrorFrameCount() const {
	return errorFrameCount;
}

/*
	Adds a frame analysis to the analysis list
*/
void SFEngine::AddFrameAnalysis(const SSCAUFrameData & sfd) {
	analysis.push_back(sfd);
}

void SFEngine::ClearFrameAnalysis() {
	analysis.clear();
	errorFrameCount = 0;
}

#pragma mark AU Graph

/*
	Creates the AU Graph
*/
void SFEngine::GraphCreate() {
	
	NewAUGraph(&mGraph);
	AUGraphOpen(mGraph);

//	ComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_GenericOutput, kAudioUnitManufacturer_Apple };
//	AUGraphNewNode(mGraph, &desc, 0, NULL, &mOutputNode);
	
	
//	AUGraphGetNodeInfo(mGraph, mOutputNode, NULL, NULL, NULL, &mOutputUnit);
}

/*
	Destroys the AU Graph
*/
void SFEngine::GraphDestroy() {
	Boolean isRunning = NO;
	AUGraphIsRunning(mGraph, &isRunning);
	if(isRunning)
		GraphStop();
		
	AUGraphClose(mGraph);
	DisposeAUGraph(mGraph);
}

/*
	Starts the AU Graph
*/
void SFEngine::GraphStart() {
	AUGraphUpdate(mGraph, NULL);
	AUGraphInitialize(mGraph);
	AUGraphStart(mGraph);
}

/*
	Stops the AU Graph
*/
void SFEngine::GraphStop() {
	AUGraphStop(mGraph);
	AUGraphUninitialize(mGraph);
}

#pragma mark Utility

/*
	Converts a given string into an FSRef.
*/
FSRef SFEngine::StringToFSRef(NSString * filePath) {
	FSRef output;
	NSURL * url = [NSURL fileURLWithPath: filePath];
	CFURLGetFSRef((CFURLRef)url, &output);
	return output;
}

