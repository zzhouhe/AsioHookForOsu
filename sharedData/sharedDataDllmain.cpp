#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "../inc/common.h"


#pragma comment(linker, "/SECTION:.SHARE,RWS")
#pragma data_seg(".SHARE")
SHAREDDATA shareData = {0};
#pragma data_seg()

extern "C" PSHAREDDATA __declspec(dllexport) getSharedData()
{
	return &shareData;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	HANDLE handleFirst;
	DWORD dwThreadID;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		
	case DLL_THREAD_ATTACH:

	case DLL_THREAD_DETACH:

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}