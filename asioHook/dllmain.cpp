#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "../inc/common.h"
#include <list>

#if _DEBUG
#include <time.h>
#endif

#include "MinHook.h"

#pragma comment(lib, "libMinHook-x86-v100-mt.lib")
//#define BASS2009
HANDLE osuRequest;

volatile PSHAREDDATA psd;

HMODULE hmod;


void Unicode2Ascii(LPCWSTR src,char*tar)
{
	unsigned int n;
	n=WideCharToMultiByte(0,0,src,(unsigned int)-1, 0, 0,0,0);
	WideCharToMultiByte(0,0,src,(unsigned int)-1,(char*)tar,n,0,0);
	tar[n]=0;
}


/*
	auto hook method
	from https://github.com/XTXTMTXTX/osu-External-ASIO-Sound
*/
typedef unsigned __int64 QWORD;
typedef DWORD HSAMPLE;		// sample handle
typedef DWORD HCHANNEL;		// playing sample's channel handle
typedef HSAMPLE (WINAPI *fpBASS_SampleLoad)(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags);
typedef HCHANNEL (WINAPI *fpBASS_SampleGetChannel)(HSAMPLE handle, BOOL onlynew);
//typedef BOOL (WINAPI *fpBASS_ChannelSetAttribute)(DWORD handle, DWORD attrib, float value);
typedef BOOL (WINAPI *fpBASS_ChannelPlay)(DWORD handle, BOOL restart);
typedef BOOL (WINAPI *fpBASS_ChannelStop)(DWORD handle);

HSAMPLE WINAPI MyBASS_SampleLoad(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags);
HCHANNEL WINAPI MyBASS_SampleGetChannel(HSAMPLE handle, BOOL onlynew);
//BOOL WINAPI MyBASS_ChannelSetAttribute(DWORD handle, DWORD attrib, float value);
BOOL WINAPI MyBASS_ChannelPlay(DWORD handle, BOOL restart);
BOOL WINAPI MyBASS_ChannelStop(DWORD handle);

fpBASS_SampleLoad pOrigBASS_SampleLoad = NULL, pBASS_SampleLoad = NULL;
fpBASS_SampleGetChannel pOrigBASS_SampleGetChannel = NULL, pBASS_SampleGetChannel = NULL;
//fpBASS_ChannelSetAttribute pOrigBASS_ChannelSetAttribute = NULL, pBASS_ChannelSetAttribute = NULL;
fpBASS_ChannelPlay pOrigBASS_ChannelPlay = NULL, pBASS_ChannelPlay = NULL;
fpBASS_ChannelStop pOrigBASS_ChannelStop = NULL, pBASS_ChannelStop = NULL;

void doHook(){
#if _DEBUG
	QueryPerformanceFrequency(&psd->m_liFreq);
#endif
	printf("doHook...\n");
	
	//hmod = LoadLibrary("bass.dll");
	//printf("bass.dll: %p\n", hmod);

	//DWORD oldProtect;
	
//#ifdef BASS2009
//	addr_BASS_SampleLoad = (DWORD)hmod + 0xd567;
//#else
//	addr_BASS_SampleLoad = (DWORD)GetProcAddress(hmod, "BASS_SampleCreate") -6;
//#endif
//	printf("addr_BASS_SampleLoad: %p\n", addr_BASS_SampleLoad);
//#ifdef BASS2009
//	if (*(DWORD*)(addr_BASS_SampleLoad+3)  != 0x56001cc2)
//#else
//	if (*(DWORD*)(addr_BASS_SampleLoad+3)  != 0x55001cc2)
//#endif
//	{
//		printf("wrong version of bass.dll\n");
//		return;
//	}
//	
//	VirtualProtect((void *)addr_BASS_SampleLoad, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
//	*(BYTE *)addr_BASS_SampleLoad = 0x68;
//	*(DWORD *)(addr_BASS_SampleLoad+1) = (DWORD)hook_BASS_SampleLoad;
//	*(BYTE *)(addr_BASS_SampleLoad+5) = 0xc3;
	LoadLibrary("bass.dll");
	MH_Initialize();
	pBASS_SampleLoad = (fpBASS_SampleLoad) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_SampleLoad");
	printf("addr_BASS_SampleLoad: %p\n", pBASS_SampleLoad);
	if(MH_CreateHook((LPVOID)pBASS_SampleLoad,(LPVOID)MyBASS_SampleLoad,(PVOID*)&pOrigBASS_SampleLoad)){
		MessageBoxA(NULL,"Hook Failed: BASS_SampleLoad()", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}

//	addr_BASS_SampleGetChannel = (DWORD)GetProcAddress(hmod, "BASS_SampleGetChannel");
//	printf("addr_BASS_SampleGetChannel: %p\n", addr_BASS_SampleGetChannel);
//#ifdef BASS2009
//	if (*(DWORD*)(addr_BASS_SampleGetChannel+2)  != 0x28ec83ec)
//#else
//	if (*(DWORD*)(addr_BASS_SampleGetChannel+2)  != 0x3cec83ec)
//#endif
//	{
//		printf("wrong version of bass.dll\n");
//		return;
//	}
//	
//	VirtualProtect((void *)addr_BASS_SampleGetChannel, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
//	*(BYTE *)addr_BASS_SampleGetChannel = 0x68;
//	*(DWORD *)(addr_BASS_SampleGetChannel+1) = (DWORD)hook_BASS_SampleGetChannel;
//	*(BYTE *)(addr_BASS_SampleGetChannel+5) = 0xc3;
	pBASS_SampleGetChannel = (fpBASS_SampleGetChannel) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_SampleGetChannel");
	if(MH_CreateHook((LPVOID)pBASS_SampleGetChannel,(LPVOID)MyBASS_SampleGetChannel,(PVOID*)&pOrigBASS_SampleGetChannel)){
		MessageBoxA(NULL,"Hook Failed: BASS_SampleGetChannel()", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}

	pBASS_ChannelPlay = (fpBASS_ChannelPlay) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_ChannelPlay");
	if(MH_CreateHook((LPVOID)pBASS_ChannelPlay,(LPVOID)MyBASS_ChannelPlay,(PVOID*)&pOrigBASS_ChannelPlay)){
		MessageBoxA(NULL,"Hook Failed: BASS_ChannelPlay()", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}

	pBASS_ChannelStop = (fpBASS_ChannelStop) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_ChannelStop");
	if(MH_CreateHook((LPVOID)pBASS_ChannelStop,(LPVOID)MyBASS_ChannelStop,(PVOID*)&pOrigBASS_ChannelStop)){
		MessageBoxA(NULL,"Hook Failed: BASS_ChannelStop()", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}
	MH_EnableHook(MH_ALL_HOOKS);
}

char szSharedDllPath[1024];
DWORD WINAPI ThreadFunc(LPVOID param) {  
	osuRequest = CreateSemaphore(NULL, 0, 5, "osuRequest");
	printf("osuRequest: %p\n", osuRequest);

	GetModuleFileName(GetModuleHandle("asioHook.dll"), szSharedDllPath, 1024); 
	char * p = strrchr(szSharedDllPath, '\\');
	*p = 0;
	strcat(szSharedDllPath, "\\sharedData.dll");

	printf("%s\n", szSharedDllPath);
	HMODULE hsd = LoadLibrary(szSharedDllPath);
	if (!hsd)
	{
		printf("not found sharedData.dll\n");
		system("pause");
		return 0;
	}
	PgetSharedData pgetsd = (PgetSharedData)GetProcAddress(hsd, "getSharedData");
	psd = pgetsd();
	doHook();

	return 0;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	HANDLE handleFirst;
	DWORD dwThreadID;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#if _DEBUG
		AllocConsole();
		freopen("conout$","w",stdout) ;
#endif
		dwThreadID = 0;

		handleFirst = CreateThread(NULL, 0, ThreadFunc, 0, 0, &dwThreadID);

		printf("injected success!\n") ;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HSAMPLE WINAPI MyBASS_SampleLoad(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags){
	
	char name[256];
	HSAMPLE hSample = pOrigBASS_SampleLoad(mem,file,offset,length,max,flags);
	if(hSample==0)return hSample;
	//DWORD NameL=WideCharToMultiByte(0, 0, (LPCWSTR)file, (DWORD)-1, 0, 0, 0, 0);
	//WideCharToMultiByte(0, 0, (LPCWSTR)file, (DWORD)-1, name, NameL, 0, 0);
	//name[NameL]=0;
	if(mem)
	{
		DWORD length = *((DWORD *) file -1);
		//printf("length: %p\n", length);
		if (length > MAX_BUFF_SIZE)
			return hSample;
		//printf("mem:%p, file: %p, length: %p\n", mem, file, offset);
		while(psd->data[0].injectIsBusy)
		{
			if(psd->onExit)
			{
				MH_DisableHook(MH_ALL_HOOKS);
				//MH_RemoveHook((LPVOID)pOrigBASS_SampleLoad);
				//MH_RemoveHook((LPVOID)pOrigBASS_SampleGetChannel);
				//MH_RemoveHook((LPVOID)pOrigBASS_ChannelPlay);
				//MH_RemoveHook((LPVOID)pOrigBASS_ChannelStop);
				//MH_Uninitialize();
				break;
			}
			__asm pause
		}
		psd->data[0].injectIsBusy = 1;
		
		memcpy(psd->buf, (char* )file, length);
		psd->length = length;
		psd->data[0].request = 1;
		psd->data[0].requestId = OSU_REQUEST_SAMPLE_LOAD_MEM;
		psd->data[0].hSample = hSample;
		//printf("ReleaseSemaphore load\n");
		ReleaseSemaphore(osuRequest, 1, NULL);	
		return hSample;
	}

	Unicode2Ascii((LPCWSTR)file,  name);
	while(psd->data[0].injectIsBusy)
	{
		__asm pause
	}
	psd->data[0].injectIsBusy = 1;
	strcpy(psd->buf, name);
	psd->data[0].request = 1;
	psd->data[0].requestId = OSU_REQUEST_SAMPLE_LOAD;
	psd->data[0].hSample = hSample;
	//printf("ReleaseSemaphore load\n");
	ReleaseSemaphore(osuRequest, 1, NULL);

	return hSample;
}
DWORD lastSample, lastCh;
HCHANNEL WINAPI MyBASS_SampleGetChannel(HSAMPLE handle, BOOL onlynew){
#if _DEBUG
	QueryPerformanceCounter(&psd->m_liStart);
#endif
	//printf("sample: %p\n", handle);
	HCHANNEL Ch=pOrigBASS_SampleGetChannel(handle,onlynew);
	//int p=MyPool->Play.tail;
	//while((p+1)%PlayPoolSize==MyPool->Play.head);
	//if(DETAILOUTPUT)MyPool->Play.pool[p].Time=CPUclock();else MyPool->Play.pool[p].Time=0;
	//MyPool->Play.pool[p].hSample=handle;
	//MyPool->Play.pool[p].Ch=Ch;
	//p=(p+1)%PlayPoolSize;
	//MyPool->Play.tail=p;
	//return Ch;
	//for (int i=0; i<5; i++)
	//{
	//	if(!psd->data[i].injectIsBusy){
	//		psd->data[i].injectIsBusy = 1;
	//		psd->data[i].request = 1;
	//		psd->data[i].requestId = OSU_REQUEST_SAMPLE_GETCHANNEL;
	//		psd->data[i].hSample = handle;
	//		psd->data[i].hChannel = Ch;
	//		ReleaseSemaphore(osuRequest, 1, NULL);
	//		//QueryPerformanceCounter(&psd->m_liEnd);
	//		return Ch;
	//	}
	//}
	lastSample = handle;
	lastCh = Ch;
	return Ch;
}
//BOOL WINAPI MyBASS_ChannelSetAttribute(DWORD handle, DWORD attrib, float value){
//	return pOrigBASS_ChannelSetAttribute(handle, attrib, value);
//}
BOOL WINAPI MyBASS_ChannelPlay(DWORD handle, BOOL restart){
	//printf("ch: %p\n", handle);
	if (handle != lastCh)
		return pOrigBASS_ChannelPlay(handle, restart);
	for (int i=0; i<5; i++)
	{
		if(!psd->data[i].injectIsBusy){
			psd->data[i].injectIsBusy = 1;
			psd->data[i].request = 1;
			psd->data[i].requestId = OSU_REQUEST_CHANNEL_PLAY;
			psd->data[i].hSample = lastSample;
			psd->data[i].hChannel = handle;
			//printf("ReleaseSemaphore play\n");
			ReleaseSemaphore(osuRequest, 1, NULL);
#if _DEBUG
			QueryPerformanceCounter(&psd->m_liEnd);
#endif
			return pOrigBASS_ChannelPlay(handle, restart);;
		}
	}
	return pOrigBASS_ChannelPlay(handle, restart);
}
BOOL WINAPI MyBASS_ChannelStop(DWORD handle){
	for (int i=0; i<5; i++)
	{
		if(!psd->data[i].injectIsBusy){
			psd->data[i].injectIsBusy = 1;
			psd->data[i].request = 1;
			psd->data[i].requestId = OSU_REQUEST_CHANNEL_STOP;
			psd->data[i].hChannel = handle;
			//psd->data[i].hSample = lastSample;
			//printf("ReleaseSemaphore stop\n");
			ReleaseSemaphore(osuRequest, 1, NULL);
			pOrigBASS_ChannelStop(handle);
		}
	}
	return pOrigBASS_ChannelStop(handle);
}
