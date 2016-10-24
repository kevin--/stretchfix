/*
	sscau.r
	SMPTE-Stretch-Calc Audio Unit
	
	Kevin C. Dixon
	Yano Signal Processing
	01/29/2008 - spawned
*/
#include <AudioUnit/AudioUnit.r>

#include "sscauVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_sscau				1000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sscau~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define RES_ID			kAudioUnitResID_sscau
#define COMP_TYPE		kAudioUnitType_OfflineEffect
#define COMP_SUBTYPE	sscau_COMP_SUBTYPE
#define COMP_MANUF		sscau_COMP_MANF	

#define VERSION			ksscauVersion
#define NAME			"Yano Signal Processing: SMPTE-Stretch-Calc"
#define DESCRIPTION		"SMPTE-Stretch-Calc AU"
#define ENTRY_POINT		"sscauEntry"

#include "AUResources.r"