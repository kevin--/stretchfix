/****
SecretRabbit Varispeed - SecretRabbitCode / libsamplerate AudioUnit wrapper, implementing a Varispeed
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/srvs/
http://www.mega-nerd.com/SRC/
****/
/*
	secretrabbitauVersion.h
	SecretRabbitCode sample rate conversion Audio Unit
	
	Kevin C. Dixon
	Yano Signal Processing
	10/04/2008 - inception
*/

#ifndef __secretrabbitauVersion_h__
#define __secretrabbitauVersion_h__


#ifdef DEBUG
	#define ksecretrabbitauVersion 0xFFFFFFFF
#else
	#define ksecretrabbitauVersion 0x00010000	
#endif

#define secretrabbitau_COMP_SUBTYPE		'srvs'
#define secretrabbitau_COMP_MANF			'YSPf'

#endif

