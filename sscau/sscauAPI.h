/*
SSCAU - SMPTE-Stretch-Calc Audio Unit
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/
*/
/*
	sscauAPI.h
	Contains stuff neccesary to deal with the sscau as a host
	
	Kevin C. Dixon
	Yano Signal Processing
	03/04/2008
*/

#include <CoreServices/CoreServices.h>


#ifndef _SSCAU_API_H_
#define _SSCAU_API_H_

#pragma mark Properties

enum
{
	sscauProperty_NextFrameData = 65000,
};

#pragma mark Structs

//error codes for SSCAUFrameData
enum
{
	sscauErr_NoFrameData			= 0,
	sscauErr_FullFrameParsed			= 1,
	sscauErr_FoundSyncWord			= 2,
	sscauErr_InvalidFrame			= 4,
	sscauErr_FoundZeroWhenWorkingOnOne = 8,
	sscauErr_ClockMiss				= 16,
};
	
//contains data about one frame, or reports an error condition
struct SSCAUFrameData
{
	//default constructor
	SSCAUFrameData() { index = 0; skew = 0; error = sscauErr_NoFrameData;}

	//sample index where event occurred
	UInt32 index;
	//skew for the frame ending on this sample index
	SInt32 skew;
	//info code. can be any combination of sscauErr_ codes
	UInt32 error;
};

#endif
