#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <tlhelp32.h>
//#include <list>
#include <map>

#include "../inc/common.h"

#pragma comment(lib, "../fmod/lib/fmod_vc.lib")

BOOL EnableDebugPrivilege()
{
  HANDLE hToken;
  BOOL fOk=FALSE;
  if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
  {
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount=1;
    LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);
   
    fOk=(GetLastError()==ERROR_SUCCESS);
    CloseHandle(hToken);
  }
    return fOk;
}
char szDllPath[1024];
DWORD dwPID = 0;
char szOsuPath[1024];
char szConfigPath[1024];
VOID EnumProcess()
{
	
	int i = 0;
	char tmp[32];
	PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

	HANDLE hMod;
	MODULEENTRY32   me32;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	BOOL bMore = Process32First(hProcessSnap,&pe32);
    while(bMore)
    {
		if (!strcmp(pe32.szExeFile, "osu!.exe"))
		{
			dwPID = pe32.th32ProcessID;
			break;
		}
		bMore = ::Process32Next(hProcessSnap,&pe32);
    }
	CloseHandle(hProcessSnap);
}


BOOL InjectDll()
{
	EnumProcess();
	if (!dwPID){
		printf("not found osu!.exe\n");
		system("pause");
		return false;
	}
	printf("szdllPath = %s\n", szDllPath);
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;
	DWORD dwBufsize = (_tcslen(szDllPath) + 1) * sizeof(TCHAR);
	printf("inject dwBufsize = %d\n", dwBufsize);
	LPTHREAD_START_ROUTINE pThreadProc;
	EnableDebugPrivilege();
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		_tprintf(_T("OpenProcess(%d) failed!!! errno: [%d]\n"), dwPID,
			GetLastError());
		return FALSE;
	}

	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufsize, MEM_COMMIT,
		PAGE_READWRITE);
	printf("inject pRemoteBuf = %p\n", pRemoteBuf);
	WriteProcessMemory(hProcess, pRemoteBuf, szDllPath, dwBufsize, NULL);
	hMod = GetModuleHandle(_T("kernel32.dll"));
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, _T("LoadLibraryA"));
	printf("%p\n", pThreadProc);

	hThread = CreateRemoteThread(
		hProcess,
		NULL,
		0,
		pThreadProc,
		pRemoteBuf,
		0,
		NULL
		);
	printf("%p\n", hThread);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;
}
HANDLE osuRequest;

PSHAREDDATA psd;


DWORD devId;
FMOD::System *fmodSystem = nullptr;
DWORD fmodMaxBuffers;
DWORD fmodBuffLength;
DWORD sampleRate;
void initAsio(){
	char buf[1024];
	char *p;
	FILE *fp = fopen(szConfigPath, "r");
	if (!fp)
	{
		printf("open config.ini failed\n");
		printf(szDllPath);
		system("pause");
	}
	while (fgets(buf, 1024, fp)){
		p=strchr(buf, '=')+1;
		if(strstr(buf, "devId"))
		{
			sscanf(p, "%d", &devId);
		}else if (strstr(buf, "osuPath")){
			sscanf(p, "%s", &szOsuPath);
		}else if (strstr(buf, "fmodBuffLength")){
			sscanf(p, "%d", &fmodBuffLength);
		}else if (strstr(buf, "fmodMaxBuffers")){
			sscanf(p, "%d", &fmodMaxBuffers);
		}else if (strstr(buf, "sampleRate")){
			sscanf(p, "%d", &sampleRate);
		}
	}

	fclose(fp);
	printf("devId: %d\n", devId);
	printf("initAsio...\n");

	FMOD_RESULT initRet = FMOD::System_Create(&fmodSystem);      // Create the main system object.
	if (initRet != FMOD_OK)
    {
        printf("Create FMOD System Failed: %s\n", FMOD_ErrorString((FMOD_RESULT)initRet));
        return;
    }
	fmodSystem->setOutput(FMOD_OUTPUTTYPE_ASIO);
  
	initRet = fmodSystem->setDriver(devId);
	if (initRet != FMOD_OK)
    {
        printf("setDriver Failed: %s\n", FMOD_ErrorString((FMOD_RESULT)initRet));
        return;
    }
	fmodSystem->setSoftwareFormat(sampleRate, FMOD_SPEAKERMODE_DEFAULT, 0);
	fmodSystem->setDSPBufferSize(fmodBuffLength, fmodMaxBuffers);
	initRet = fmodSystem->init(32, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.


    if (initRet != FMOD_OK)
    {
        printf("FMOD System Initialize Failed: %s\n", FMOD_ErrorString((FMOD_RESULT)initRet));
        return;
    }
}

std::map<DWORD, FMOD::Sound *> g_sample_map;


void do_BASS_SampleLoad(){
	FMOD::Sound *sound = NULL;
	FMOD_MODE mode = FMOD_CREATESAMPLE;
	
	FMOD_RESULT r = fmodSystem->createSound(psd->buf, FMOD_CREATESAMPLE/* | FMOD_LOOP_NORMAL*/, 0, &sound);
	printf("create sound %s\n", psd->buf);

	if (r != FMOD_OK)
	{
		printf("[FMOD] Loading Sample (%s) Error: %s\n", psd->buf, FMOD_ErrorString(r));
		return;
	}
	g_sample_map[psd->hSample] = sound;

	printf("hSample: %p added\n", psd->hSample);
	printf("sample size: %d\n", g_sample_map.size());
}

void do_bind_sample(){
	FMOD::Sound *sound = g_sample_map[psd->hSample];
	if(sound)
	{
		fmodSystem->playSound(sound, 0, false, 0);
		fmodSystem->update();
		//printf("play smple 1\n");
	}
    
}

void do_bind_sample2(){
	FMOD::Sound *sound = g_sample_map[psd->hSample2];
	if(sound)
	{
		fmodSystem->playSound(sound, 0, false, 0);
		fmodSystem->update();
		//printf("play smple 2\n");
	}
    
}
void do_bind_sample3(){
	FMOD::Sound *sound = g_sample_map[psd->hSample3];
	if(sound)
	{
		fmodSystem->playSound(sound, 0, false, 0);
		fmodSystem->update();
		//printf("play smple 3\n");
	}
    
}
void do_bind_sample4(){
	FMOD::Sound *sound = g_sample_map[psd->hSample4];
	if(sound)
	{
		fmodSystem->playSound(sound, 0, false, 0);
		fmodSystem->update();
		//printf("play smple 4\n");
	}
    
}
void do_bind_sample5(){
	FMOD::Sound *sound = g_sample_map[psd->hSample5];
	if(sound)
	{
		fmodSystem->playSound(sound, 0, false, 0);
		fmodSystem->update();
		//printf("play smple 5\n");
	}
    
}

int _tmain(int argc, TCHAR *argv[])
{ 
    GetModuleFileName(NULL,szDllPath, 1024); 
	char * p = strrchr(szDllPath, '\\');
	*p = 0;
	strcat(szDllPath, "\\asioHook.dll");
	 GetModuleFileName(NULL,szConfigPath, 1024); 
	p = strrchr(szConfigPath, '\\');
	*p = 0;
	strcat(szConfigPath, "\\config.ini");

	initAsio();
	if (InjectDll())
		_tprintf(_T("InjectDll sucess!\n"));
	else
		_tprintf(_T("InjectDll failed!\n"));
	Sleep(2000);

	strcat(szOsuPath, "sharedData.dll");
	osuRequest = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "osuRequest");
	printf("osuRequest: %p\n", osuRequest);
	
	HMODULE hsd = LoadLibrary(szOsuPath);
	if (!hsd)
	{
		printf("please put sharedData.dll to osu!.exe dir\n");
		system("pause");
		return 0;
	}
	PgetSharedData pgetsd = (PgetSharedData)GetProcAddress(hsd, "getSharedData");
	psd = pgetsd();


	while(1)
	{
		WaitForSingleObject(osuRequest, INFINITE);
		if(psd->request)
		{
			psd->injectIsBusy = 1;
			psd->request  = 0;
			switch (psd->requestId)
			{
			case OSU_REQUEST_SAMPLE_LOAD:
				do_BASS_SampleLoad();
				break;
			case OSU_REQUEST_SAMPLE_GETCHANNEL:
				do_bind_sample();
				break;
			}
			psd->injectIsBusy = 0;
		}
		if(psd->request2)
		{
			psd->injectIsBusy2 = 1;
			psd->request2  = 0;
			switch (psd->requestId2)
			{
			case OSU_REQUEST_SAMPLE_GETCHANNEL:
				do_bind_sample2();
				break;
			}
			psd->injectIsBusy2 = 0;
		}
		
		if(psd->request3)
		{
			psd->injectIsBusy3 = 1;
			psd->request3  = 0;
			switch (psd->requestId3)
			{
			case OSU_REQUEST_SAMPLE_GETCHANNEL:
				do_bind_sample3();
				break;
			}
			psd->injectIsBusy3 = 0;
		}
		if(psd->request4)
		{
			psd->injectIsBusy4 = 1;
			psd->request4  = 0;
			switch (psd->requestId4)
			{
			case OSU_REQUEST_SAMPLE_GETCHANNEL:
				do_bind_sample4();
				break;
			}
			psd->injectIsBusy4 = 0;
		}

		if(psd->request5)
		{
			psd->injectIsBusy5 = 1;
			psd->request5  = 0;
			switch (psd->requestId5)
			{
			case OSU_REQUEST_SAMPLE_GETCHANNEL:
				do_bind_sample5();
				break;
			}
			psd->injectIsBusy5 = 0;
		}

	}
	return 0;
}