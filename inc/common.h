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

	DWORD request3;
	DWORD requestId3;
	DWORD injectIsBusy3;
	DWORD hSample3;

	DWORD request4;
	DWORD requestId4;
	DWORD injectIsBusy4;
	DWORD hSample4;

	DWORD request5;
	DWORD requestId5;
	DWORD injectIsBusy5;
	DWORD hSample5;

} SHAREDDATA, *PSHAREDDATA;

typedef PSHAREDDATA (*PgetSharedData)();

typedef struct _HANDLES{
	//DWORD	hchan;
	DWORD	hSample;
	FMOD::Sound *sound;
} HANDLES, *PHANDLES;

#endif //__COMMON_H__