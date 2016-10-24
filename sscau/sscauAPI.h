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