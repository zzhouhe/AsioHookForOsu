#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "../inc/common.h"
#include <list>

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

DWORD	addr_BASS_SampleLoad;
DWORD	*p_SampleLoad_args;
DWORD	SampleLoad_ret;
void hook_BASS_SampleLoad_ebd()
{	
	
	if (p_SampleLoad_args[0] == 1){
		return ;
	}

	char sample[1024];
	Unicode2Ascii((LPCWSTR)p_SampleLoad_args[1],  sample);
	while(psd->injectIsBusy)
	{
		__asm pause
	}

	strcpy(psd->buf, sample);
	psd->requestId = OSU_REQUEST_SAMPLE_LOAD;
	psd->hSample = SampleLoad_ret;
	ReleaseSemaphore(osuRequest, 1, NULL);

}
void __declspec(naked) hook_BASS_SampleLoad(){
	__asm{
		pushad
		pushfd
		mov SampleLoad_ret, eax

		lea eax, [ebp + 8]
		mov p_SampleLoad_args, eax
	}
	hook_BASS_SampleLoad_ebd();
	__asm{
		popfd
		popad

		mov esp, ebp
		pop ebp
		ret 0x1c
	}
}

DWORD addr_BASS_SampleGetChannel;
DWORD arg_BASS_SampleGetChannel1;
DWORD arg_BASS_SampleGetChannel2;
DWORD ret_BASS_SampleGetChannel;
void __declspec(naked) real_BASS_SampleGetChannel(){
	__asm{
		push ebp
		mov ebp, esp
		sub esp, 0x3c
		mov eax, addr_BASS_SampleGetChannel
		add eax, 6
		jmp eax
	}
}
void hook_BASS_SampleGetChannel_ebd(){
	__asm{
		push arg_BASS_SampleGetChannel2
		push arg_BASS_SampleGetChannel1
		call real_BASS_SampleGetChannel
		mov ret_BASS_SampleGetChannel, eax
	}

	while(psd->injectIsBusy){
		__asm pause
	}

	psd->requestId = OSU_REQUEST_SAMPLE_GETCHANNEL;
	psd->hSample = arg_BASS_SampleGetChannel1;
	//psd->hChanel = ret_BASS_SampleGetChannel;
	ReleaseSemaphore(osuRequest, 1, NULL);
}
void __declspec(naked) hook_BASS_SampleGetChannel(){
	__asm{
		mov eax, [esp+4]
		mov arg_BASS_SampleGetChannel1, eax
		mov eax, [esp+8]
		mov arg_BASS_SampleGetChannel2, eax
	}
	hook_BASS_SampleGetChannel_ebd();
	__asm ret 8
}


void doHook(){
	printf("doHook...\n");
	
	hmod = LoadLibrary("bass.dll");
	printf("bass.dll: %p\n", hmod);

	DWORD oldProtect;

	addr_BASS_SampleLoad = (DWORD)GetProcAddress(hmod, "BASS_SampleCreate") -6;
	printf("addr_BASS_SampleLoad: %p\n", addr_BASS_SampleLoad);
	if (*(DWORD*)(addr_BASS_SampleLoad+3)  != 0x55001cc2)
	{
		printf("wrong version of bass.dll\n");
		return;
	}
	
	VirtualProtect((void *)addr_BASS_SampleLoad, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(BYTE *)addr_BASS_SampleLoad = 0x68;
	*(DWORD *)(addr_BASS_SampleLoad+1) = (DWORD)hook_BASS_SampleLoad;
	*(BYTE *)(addr_BASS_SampleLoad+5) = 0xc3;

	addr_BASS_SampleGetChannel = (DWORD)GetProcAddress(hmod, "BASS_SampleGetChannel");
	printf("addr_BASS_SampleGetChannel: %p\n", addr_BASS_SampleGetChannel);
	if (*(DWORD*)(addr_BASS_SampleGetChannel+2)  != 0x3cec83ec)
	{
		printf("wrong version of bass.dll\n");
		return;
	}
	
	VirtualProtect((void *)addr_BASS_SampleGetChannel, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(BYTE *)addr_BASS_SampleGetChannel = 0x68;
	*(DWORD *)(addr_BASS_SampleGetChannel+1) = (DWORD)hook_BASS_SampleGetChannel;
	*(BYTE *)(addr_BASS_SampleGetChannel+5) = 0xc3;

}


DWORD WINAPI ThreadFunc(LPVOID param) {  
	osuRequest = CreateSemaphore(NULL, 0, 1, "osuRequest");
	printf("osuRequest: %p\n", osuRequest);

	HMODULE hsd = LoadLibrary("sharedData.dll");
	if (!hsd)
	{
		printf("please put sharedData.dll to osu!.exe dir\n");
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

	case DLL_THREAD_DETACH:

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

