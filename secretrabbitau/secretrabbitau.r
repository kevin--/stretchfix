/****
SecretRabbit Varispeed - SecretRabbitCode / libsamplerate AudioUnit wrapper, implementing a Varispeed
Copyright (C) 2008  Kevin C. Dixon
http://yano.wasteonline.net/software/srvs/
http://www.mega-nerd.com/SRC/
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
