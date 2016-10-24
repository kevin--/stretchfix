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

