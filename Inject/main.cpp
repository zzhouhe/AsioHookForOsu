#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <list>

#include "../inc/common.h"

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


std::list<PHANDLES> g_handle_list;

void do_BASS_SampleLoad(){
	FMOD::Sound *sound = NULL;
	FMOD_MODE mode = FMOD_CREATESAMPLE;
	FMOD_CREATESOUNDEXINFO info = {0};
	
	FMOD_RESULT r = fmodSystem->createSound(psd->buf, FMOD_CREATESAMPLE, 0, &sound);
	printf("create sound %s\n", psd->buf);

	if (r != FMOD_OK)
	{
		printf("[FMOD] Loading Sample (%s) Error: %s\n", psd->buf, FMOD_ErrorString(r));
		return;
	}
	PHANDLES p = (PHANDLES)malloc(sizeof(HANDLES));
	p->sound = sound;
	p->hSample = psd->hSample;
	//p->hchan = 0;
	g_handle_list.push_back(p);
	printf("hSample: %p added to list\n", psd->hSample);
	printf("list size: %d\n", g_handle_list.size());
}

void do_bind_sample(){
	std::list<PHANDLES>::reverse_iterator ir; 
	for (ir =g_handle_list.rbegin(); ir!=g_handle_list.rend();ir++) {   
		PHANDLES p = *ir;
		if(p->hSample == psd->hSample ){
			//p->hchan = psd->hChanel;
			fmodSystem->playSound(p->sound, 0, false, 0);
			//printf("bind hSample %p success\n", psd->hSample);
			return;
		}
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
		psd->injectIsBusy = 1;
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
	return 0;
}