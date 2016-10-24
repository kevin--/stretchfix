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
	secretrabbitau.r
	SecretRabbitCode sample rate conversion Audio Unit
*/
#include <AudioUnit/AudioUnit.r>

#include "secretrabbitauVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_secretrabbitau				1000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ secretrabbitau~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define RES_ID			kAudioUnitResID_secretrabbitau
#define COMP_TYPE		kAudioUnitType_OfflineEffect
#define COMP_SUBTYPE	secretrabbitau_COMP_SUBTYPE
#define COMP_MANUF		secretrabbitau_COMP_MANF	

#define VERSION          ksecretrabbitauVersion
#define NAME			"Yano Signal Processing: SRC Varispeed"
#define DESCRIPTION		"Varispeed (libsamplerate/SecretRabbitCode)"
#define ENTRY_POINT		"SecretRabbitAUEntry"

#include "AUResources.r"