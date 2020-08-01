#ifndef __COMMON_H__
#define __COMMON_H__
#include "../fmod/inc/fmod.hpp"
#include "../fmod/inc/fmod_errors.h"

#define OSU_REQUEST_SAMPLE_LOAD				0
#define OSU_REQUEST_SAMPLE_GETCHANNEL		1

typedef struct _sharedData{
    DWORD requestId;
	DWORD injectIsBusy;
	//DWORD hChanel;
	DWORD hSample;
	char buf[2048];
} SHAREDDATA, *PSHAREDDATA;

typedef PSHAREDDATA (*PgetSharedData)();

typedef struct _HANDLES{
	//DWORD	hchan;
	DWORD	hSample;
	FMOD::Sound *sound;
} HANDLES, *PHANDLES;

#endif //__COMMON_H__