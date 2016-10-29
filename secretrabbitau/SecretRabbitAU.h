/****
SecretRabbit Varispeed - SecretRabbitCode / libsamplerate AudioUnit wrapper, implementing a Varispeed
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/srvs/
http://www.mega-nerd.com/SRC/
****/
/*
	secretrabbitau.h
	SecretRabbit Varispeed AU
	
	Kevin C. Dixon
	Yano Signal Processing
	10/04/2008
*/

#include "AUBase.h"

#include "secretrabbitauVersion.h"
#include "secretrabbitParams.h"

#include <samplerate.h>

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif


#ifndef __secretrabbitau_h__
#define __secretrabbitau_h__

#pragma mark Parameter Info

static CFStringRef kLabel[kNumberOfParameters] =
{
	//kParam_Ratio
	CFSTR("Ratio")
};
	
	

class SecretRabbitAU : public AUBase
{
private:
	SRC_STATE * handle;
    float * _outputBuffer;
    size_t _outputBufferFrames;

private:
    using tBase = AUBase;
    
public:

#pragma mark Constructors

	SecretRabbitAU(AudioUnit component);
    virtual ~SecretRabbitAU();


	virtual ComponentResult Initialize();
	
    virtual ComponentResult Reset(AudioUnitScope inScope,
                                  AudioUnitElement inElement);
	
#pragma mark Accessors	

	virtual bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element);
	
	virtual ComponentResult GetParameterValueStrings(AudioUnitScope inScope,
										    AudioUnitParameterID inParameterID,
										    CFArrayRef * outStrings);
    
	virtual ComponentResult GetParameterInfo(AudioUnitScope inScope,
									 AudioUnitParameterID inParameterID,
									 AudioUnitParameterInfo & outParameterInfo);
    
	virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outInfo);
    
	virtual ComponentResult GetPropertyInfo(AudioUnitPropertyID inID,
									AudioUnitScope inScope,
									AudioUnitElement inElement,
									UInt32 & outDataSize,
									Boolean & outWritable );
	
	virtual ComponentResult GetProperty(AudioUnitPropertyID inID,
								 AudioUnitScope inScope,
								 AudioUnitElement inElement,
								 void * outData);
								 
	virtual ComponentResult SetProperty(AudioUnitPropertyID inID,
								 AudioUnitScope inScope,
								 AudioUnitElement inElement,
								 const void * inData,
								 UInt32 inDataSize);
	
 	virtual bool SupportsTail () { return false; }
	
	/*! @method Version */
	virtual ComponentResult Version() { return ksecretrabbitauVersion; }
	
#pragma mark Rendering
	
	virtual ComponentResult Render(AudioUnitRenderActionFlags & ioActionFlags,
							 const AudioTimeStamp & inTimeStamp,
							 UInt32 inNumberFrames);
							 
	virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
							      const AudioBufferList & inBuffer,
								 AudioBufferList & outBuffer,
								 UInt32 inFramesToProcess);

#pragma mark Utility
								 
private:

	static void CopyABLToFloatArray(const AudioBufferList & bufferIn,
								float ** arrayOut,
								UInt32 * arrayOutLength);

	static void CopyFloatArrayToABL(float ** arrayIn,
							  UInt32 arrayInLength,
							  AudioBufferList & bufferOut);

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif
