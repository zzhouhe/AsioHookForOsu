#ifndef __COMMON_H__
#define __COMMON_H__
#include "../fmod/inc/fmod.hpp"
#include "../fmod/inc/fmod_errors.h"

#define OSU_REQUEST_SAMPLE_LOAD				0
#define OSU_REQUEST_SAMPLE_GETCHANNEL		1

typedef struct _sharedData{
	DWORD request;
    DWORD requestId;
	DWORD injectIsBusy;
	DWORD hSample;
	char buf[2048];

	DWORD request2;
	DWORD requestId2;
	DWORD injectIsBusy2;
	DWORD hSample2;
} SHAREDDATA, *PSHAREDDATA;

typedef PSHAREDDATA (*PgetSharedData)();

typedef struct _HANDLES{
	//DWORD	hchan;
	DWORD	hSample;
	FMOD::Sound *sound;
} HANDLES, *PHANDLES;

#endif //__COMMON_H__