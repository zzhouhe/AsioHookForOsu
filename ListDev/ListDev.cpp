#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "../fmod/inc/fmod.hpp"
#include "../fmod/inc/fmod_errors.h"

#pragma comment(lib, "../fmod/lib/fmod_vc.lib")

FMOD::System *fmodSystem;

void main()
{
	FMOD::Sound *sound;
	FMOD_RESULT initRet = FMOD::System_Create(&fmodSystem); 
	int num;
	fmodSystem->setOutput(FMOD_OUTPUTTYPE_ASIO);
	fmodSystem->getNumDrivers(&num);
	printf("find %d drivers\n", num);
	if (!num)
	{
		printf("no device support ASIO\n");
		system("pause");
		return;
	}
	char name[1024];
	for (int i =0; i<num; i++)
	{
		fmodSystem->getDriverInfo(i, name, 1024, NULL, NULL, NULL, NULL);
		printf("%d: %s\n", i, name);
	}
	system("pause");
}