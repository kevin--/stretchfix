/*
	sscauVersion.h
	SMPTE-Stretch-Calc Audio Unit
	
	Kevin C. Dixon
	Yano Signal Processing
	01/29/2008 - spawned
*/
#ifndef __sscauVersion_h__
#define __sscauVersion_h__


#ifdef DEBUG
	#define ksscauVersion 0xFFFFFFFF
#else
	#define ksscauVersion 0x00010000	
#endif

#define sscau_COMP_SUBTYPE		'ssca'
#define sscau_COMP_MANF			'YSPf'

#endif

