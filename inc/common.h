#ifndef __COMMON_H__
#define __COMMON_H__
#include "../fmod/inc/fmod.hpp"
#include "../fmod/inc/fmod_errors.h"

#define OSU_REQUEST_SAMPLE_LOAD				0
//#define OSU_REQUEST_SAMPLE_GETCHANNEL		1
#define OSU_REQUEST_CHANNEL_PLAY			2
#define OSU_REQUEST_CHANNEL_STOP			3
#define OSU_REQUEST_SAMPLE_LOAD_MEM			4

#define MAX_BUFF_SIZE					1024*1024*4		//4MB
typedef struct _sharedData{
	struct _data{
	DWORD request;
    DWORD requestId;
	DWORD injectIsBusy;
	DWORD hSample;
	DWORD hChannel;

	} data[5];
	char buf[MAX_BUFF_SIZE];
	DWORD length;
	bool	onExit;

#if _DEBUG
LARGE_INTEGER m_liFreq;
LARGE_INTEGER m_liStart;
LARGE_INTEGER m_liEnd;
#endif

} SHAREDDATA, *PSHAREDDATA;

typedef PSHAREDDATA (*PgetSharedData)();

typedef struct _HANDLES{
	//DWORD	hchan;
	DWORD	hSample;
	FMOD::Sound *sound;
} HANDLES, *PHANDLES;

#endif //__COMMON_H__