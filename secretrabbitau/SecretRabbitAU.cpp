/****
SecretRabbit Varispeed - SecretRabbitCode / libsamplerate AudioUnit wrapper, implementing a Varispeed
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/srvs/
http://www.mega-nerd.com/SRC/

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
****/
/*
	SecretRabbitAU.cpp
	SecretRabbit Varispeed Audio Unit
	
	Kevin C. Dixon
	10/13/2008
	implementation
*/

#include "SecretRabbitAU.h"

AUDIOCOMPONENT_ENTRY(AUBaseFactory, SecretRabbitAU)

#pragma mark Constructors

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	secretrabbitau::secretrabbitau
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SecretRabbitAU::SecretRabbitAU(AudioUnit component)
	: AUBase(component, 1, 1)
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
	
	
	Globals()->UseIndexedParameters(kNumberOfParameters);
	
	Globals()->SetParameter(kParam_Ratio, 1.0);
}

SecretRabbitAU::~SecretRabbitAU()
{
    
}

/*
	Obtains a handle to the SecretRabbitCode Sample Rate Converter
*/
ComponentResult SecretRabbitAU::Initialize()
{
	int error;
	handle = src_new(SRC_SINC_BEST_QUALITY, 1, &error);
	
	if(handle == NULL)
	{
		//throw src_strerror(error);
	}
	
	return error;
}

/*
	Resets the AU to its uninitialized state
*/
ComponentResult SecretRabbitAU::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
    tBase::Reset(inScope, inElement);
    
	if(handle != NULL)
	{
		handle = src_delete(handle);
	}
	
	return noErr;
}

#pragma mark Accessors

bool SecretRabbitAU::StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element)
{
	return IsInitialized() ? false : true;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	secretrabbitau::GetParameterValueStrings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult SecretRabbitAU::GetParameterValueStrings(AudioUnitScope inScope,
											  AudioUnitParameterID inParameterID,
										       CFArrayRef * outStrings)
{
    return kAudioUnitErr_InvalidProperty;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	secretrabbitau::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult SecretRabbitAU::GetParameterInfo(AudioUnitScope inScope,
									    AudioUnitParameterID inParameterID,
                                                 AudioUnitParameterInfo & outParameterInfo)
{
	ComponentResult result = noErr;

	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable |
					     kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
        switch(inParameterID)
        {
            case kParam_Ratio:
                AUBase::FillInParameterName(outParameterInfo, kLabel[kParam_Ratio], false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Rate;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 100.0;
                outParameterInfo.defaultValue = 1.0;
                break;
            default:
                result = kAudioUnitErr_InvalidParameter;
                break;
            }
	} else {
        result = kAudioUnitErr_InvalidParameter;
    }
    


	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	secretrabbitau::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult SecretRabbitAU::GetPropertyInfo(AudioUnitPropertyID inID,
									   AudioUnitScope inScope,
									   AudioUnitElement inElement,
									   UInt32 & outDataSize,
									   Boolean & outWritable)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
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
					
		}
	}

	return AUBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

/*
	Returns an AUChannelInfo struct that shows the number of i and o channels
	outInfo	[out]	output AUChannelInfo struct
	returns number of sets of support channels
*/
UInt32 SecretRabbitAU::SupportedNumChannels(const AUChannelInfo ** outInfo) {
	static const AUChannelInfo myInfo[1] = {{1, 1}};
	if(outInfo)
		*outInfo = myInfo;
	return sizeof(myInfo) / sizeof(AUChannelInfo);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	secretrabbitau::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult SecretRabbitAU::GetProperty(AudioUnitPropertyID inID,
								    AudioUnitScope inScope,
								    AudioUnitElement inElement,
								    void * outData)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
		{
			case kAudioUnitProperty_CocoaUI:
			{
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.Demo.audiounit.secretrabbitau") );
				
				if (bundle == NULL) return fnfErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
                    CFSTR("CocoaViewFactory"), 
                    CFSTR("bundle"), 
                    NULL);
                
				if (bundleURL == NULL) return fnfErr;
                
				CFStringRef className = CFSTR("CocoaViewFactory");
				AudioUnitCocoaViewInfo cocoaInfo = { bundleURL, {className} };
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
		}
	}

	return AUBase::GetProperty (inID, inScope, inElement, outData);
}

ComponentResult SecretRabbitAU::SetProperty(AudioUnitPropertyID inID,
								    AudioUnitScope inScope,
								    AudioUnitElement inElement,
								    const void * inData,
								    UInt32 inDataSize)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
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
	return AUBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
}

#pragma mark Rendering

ComponentResult SecretRabbitAU::Render(AudioUnitRenderActionFlags & ioActionFlags,
							    const AudioTimeStamp & inTimeStamp,
							    UInt32 inNumberFrames)
{
	if (!HasInput(0))
	{
		return kAudioUnitErr_NoConnection;
	}
	
	ComponentResult result = noErr;
	AUOutputElement * theOutput = GetOutput(0);	// throws if error

	AUInputElement * theInput = GetInput(0);
	result = theInput->PullInput(ioActionFlags, inTimeStamp, 0, inNumberFrames);
	
	if(result == noErr)
	{	
		if(mParamList.size() == 0 )
		{
			result = ProcessBufferLists(ioActionFlags, theInput->GetBufferList(), theOutput->GetBufferList(), inNumberFrames);
		}
	/*	else
		{ 
			// deal with scheduled parameters...
				
			AudioBufferList &inputBufferList = theInput->GetBufferList();
			AudioBufferList &outputBufferList = theOutput->GetBufferList();
				
			ScheduledProcessParams processParams;
			processParams.actionFlags = &ioActionFlags;
			processParams.inputBufferList = &inputBufferList;
			processParams.outputBufferList = &outputBufferList;
	
			// divide up the buffer into slices according to scheduled params then
			// do the DSP for each slice (ProcessScheduledSlice() called for each slice)
			result = ProcessForScheduledParams(mParamList, inNumberFrames, &processParams);
	
				
			// fixup the buffer pointers to how they were before we started
			for(unsigned int i = 0; i < inputBufferList.mNumberBuffers; i++ )
				inputBufferList.mBuffers[i].mData = (Float32 *)inputBufferList.mBuffers[i].mData - inputBufferList.mBuffers[i].mNumberChannels * nFrames;
				
			for(unsigned int i = 0; i < outputBufferList.mNumberBuffers; i++ )
				outputBufferList.mBuffers[i].mData = (Float32 *)outputBufferList.mBuffers[i].mData - outputBufferList.mBuffers[i].mNumberChannels * nFrames;
		}*/

	
		if ((ioActionFlags & kAudioUnitRenderAction_OutputIsSilence))
		{
			AUBufferList::ZeroBuffer(theOutput->GetBufferList() );
		}
	}
	
	return result;
}

OSStatus SecretRabbitAU::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
								    const AudioBufferList & inBuffer,
								    AudioBufferList & outBuffer,
								    UInt32 inFramesToProcess)
{	
	if (inBuffer.mNumberBuffers == 1)
	{
		//set the latest known ratio for libsamplerate
		Float32 ratio = Globals()->GetParameter(kParam_Ratio);
		int srcResult = src_set_ratio(handle, ratio);
		if(srcResult != 0)
		{
			return srcResult;
		}
		
        SRC_DATA srcData = {};
        //setup input, use input buffer
        srcData.data_in = static_cast<float*>(inBuffer.mBuffers[0].mData);
        size_t inputFrames = inBuffer.mBuffers[0].mDataByteSize / sizeof(float) / inBuffer.mBuffers[0].mNumberChannels;
        srcData.input_frames = inputFrames;
		
        size_t outputBufferFrames = static_cast<size_t>(ceil(inputFrames * ratio));
		srcData.data_out = new float[outputBufferFrames];
        srcData.output_frames = outputBufferFrames;
		srcData.src_ratio = ratio;
		
		srcData.end_of_input = 0;	//1 == more data available, 0 == no more data available

		srcResult = src_process(handle, &srcData);
		if(srcResult != 0)
		{
			return srcResult;
		}
		
		CopyFloatArrayToABL(&srcData.data_out, srcData.output_frames, outBuffer);
        
        delete [] srcData.data_out;
	}
	
	return noErr;
}

#pragma mark Utility

/*
	Copies a given array of floats to the first array of an AudioBufferList, as an array of Float32s
	Sets mNumberChannels to 1.
	arrayIn		array to copy
	arrayInLength	number of items in arrayIn
	bufferOut		buffer to copy items to
*/
void SecretRabbitAU::CopyFloatArrayToABL(float ** arrayIn, UInt32 arrayInLength, AudioBufferList & bufferOut)
{
	bufferOut.mBuffers[0].mNumberChannels = 1;
	bufferOut.mBuffers[0].mData = new Float32[arrayInLength];
	bufferOut.mBuffers[0].mDataByteSize = sizeof(Float32) * arrayInLength;
        
	for(UInt32 i = 0; i < arrayInLength; i++)
	{
		((Float32*)bufferOut.mBuffers[0].mData)[i] = (Float32)((*arrayIn)[i]);
	}	
}
