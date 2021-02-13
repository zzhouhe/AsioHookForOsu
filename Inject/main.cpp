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
char szHookDllPath[1024];
char szSampleFolderPath[1024];
DWORD dwPID = 0;
//char szSharedDllPath[1024];
char szConfigPath[1024];

char gameName[128];

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
		if (!strcmp(pe32.szExeFile, gameName))
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
	printf("waiting for %s to run ... \n", gameName);
	while(!dwPID)
	{
		EnumProcess();
	}
	//if (!dwPID){
	//	//printf("not found osu!.exe\n");
	//	//system("pause");
	//	return false;
	//}
	printf("szdllPath = %s\n", szHookDllPath);
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;
	DWORD dwBufsize = (_tcslen(szHookDllPath) + 1) * sizeof(TCHAR);
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
	WriteProcessMemory(hProcess, pRemoteBuf, szHookDllPath, dwBufsize, NULL);
	hMod = GetModuleHandle(_T("kernel32.dll"));
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, _T("LoadLibraryA"));
	//printf("%p\n", pThreadProc);

	hThread = CreateRemoteThread(
		hProcess,
		NULL,
		0,
		pThreadProc,
		pRemoteBuf,
		0,
		NULL
		);
	//printf("%p\n", hThread);
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
DWORD processPriority;
DWORD curPriority;
void initAsio(){
	char buf[1024];
	char *p;
	FILE *fp = fopen(szConfigPath, "r");
	if (!fp)
	{
		printf("open config.ini failed\n");
		system("pause");
	}
	while (fgets(buf, 1024, fp)){
		p=strchr(buf, '=')+1;
		if(strstr(buf, "devId"))
		{
			sscanf(p, "%d", &devId);
		}else if (strstr(buf, "gameName")){
			sscanf(p, "%s", &gameName);
		}else if (strstr(buf, "fmodBuffLength")){
			sscanf(p, "%d", &fmodBuffLength);
		}else if (strstr(buf, "fmodMaxBuffers")){
			sscanf(p, "%d", &fmodMaxBuffers);
		}else if (strstr(buf, "sampleRate")){
			sscanf(p, "%d", &sampleRate);
		}else if (strstr(buf, "processPriority")){
			sscanf(p, "%d", &processPriority);
		}

	}

	fclose(fp);
	printf("devId: %d\n", devId);
	printf("initAsio...\n");
	switch (processPriority)
	{
	default:
		break;
	case 2:
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		break;
	case 3:
		//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		//CloseHandle(hProcess);
	//r = SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS | HIGH_PRIORITY_CLASS);
	//printf("r: %d\n", r);
		
		break;
	}
	curPriority = GetPriorityClass(GetCurrentProcess());
	if (curPriority == HIGH_PRIORITY_CLASS)
		printf("Current Process Priority HIGH\n");
	if (curPriority == REALTIME_PRIORITY_CLASS)
		printf("Current Process Priority REALTIME\n");
	//printf("%p\n", GetPriorityClass(GetCurrentProcess()));
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

std::map<DWORD, FMOD::Sound *>			g_sample_map;	//<sample, sound>
//std::map<DWORD, DWORD>					g_channel_map;	//<channel, sample>
//std::map<DWORD, FMOD::Channel*>					g_ch_fmodch_map;	//<ch, fmodchannel>
#define TEMP_CH_SIZE	16
struct _ch_fmodch_node{
	DWORD ch;
	FMOD::Channel* fmodch;
} g_ch_fmodch_node[TEMP_CH_SIZE];
DWORD g_head = 0;
DWORD g_tail = 0;

void Unicode2Ascii(LPCWSTR src,char*tar)
{
	unsigned int n;
	n=WideCharToMultiByte(0,0,src,(unsigned int)-1, 0, 0,0,0);
	WideCharToMultiByte(0,0,src,(unsigned int)-1,(char*)tar,n,0,0);
	tar[n]=0;
}


void do_BASS_SampleLoad(){
	FMOD::Sound *sound = NULL;
	//FMOD_MODE mode = FMOD_CREATESAMPLE;
	
	FMOD_RESULT r = fmodSystem->createSound(psd->buf, FMOD_CREATESAMPLE/* | FMOD_LOOP_NORMAL*/, 0, &sound);
	printf("create sound %s\n", psd->buf);

	if (r != FMOD_OK)
	{
		printf("[FMOD] Loading Sample (%s) Error: %s\n", psd->buf, FMOD_ErrorString(r));
		return;
	}
	g_sample_map[psd->data[0].hSample] = sound;

	//printf("hSample: %p added\n", psd->data[0].hSample);
	printf("sample cllection size: %d\n", g_sample_map.size());
}
void do_BASS_SampleLoad_mem(){

	//char szSkin[16] = {'S', 0, 'k', 0, 'i', 0, 'n', 0, 's', 0, '\\', 0, };
	//char *p = psd->buf;
	//int i;
	//for (i=0; i<0x100; i++){
	//	if (!memcmp(p, szSkin, 12))
	//		break;
	//	p++;
	//}
	//if (i==0x100)
	//	return;
	//p+=12;
	//char name[256];
	//Unicode2Ascii((LPCWSTR)p,  name);
	//char *fname = strrchr(name, '\\');
	//fname++;
	//p = strrchr(fname, '.');
	//p[1] = 0;
	//
	FMOD::Sound *sound = NULL;
	FMOD_CREATESOUNDEXINFO extinfo = {0};
	extinfo.cbsize = sizeof(extinfo);
	extinfo.length = psd->length;
	FMOD_RESULT r = fmodSystem->createSound(psd->buf, FMOD_CREATESAMPLE | FMOD_OPENMEMORY/* | FMOD_LOOP_NORMAL*/, &extinfo, &sound);
	printf("create default sound\n");

	if (r != FMOD_OK)
	{
		printf("[FMOD] Loading default Sample (??) Error: %s\n", FMOD_ErrorString(r));
		return;
	}
	g_sample_map[psd->data[0].hSample] = sound;


	printf("sample cllection size: %d\n", g_sample_map.size());
}
void do_bind_sample(int i){
	//printf("sample: %p\n", psd->data[0].hSample);
	DWORD hSample = psd->data[i].hSample;
	if (!hSample)
		return;
	FMOD::Sound *sound = g_sample_map[hSample];
	FMOD::Channel *fmod_ch;
	if(sound)
	{
		fmodSystem->playSound(sound, 0, false, &fmod_ch);
		fmodSystem->update();
		//g_ch_fmodch_map[psd->data[i].hChannel] = fmod_ch;
		//printf("play smple 1\n");
		g_ch_fmodch_node[g_tail].ch = psd->data[i].hChannel;
		g_ch_fmodch_node[g_tail].fmodch = fmod_ch;
		g_tail++;
		g_tail%=TEMP_CH_SIZE;
		if (g_tail == g_head)
		{
			g_head++;
			g_head%=TEMP_CH_SIZE;
		}
	}
	//DWORD hChannel = psd->data[0].hChannel;
	//if (!hChannel)
	//	return;
	//g_channel_map[hChannel] = psd->data[0].hSample;
	//printf(" %d, %d\n", g_sample_map.size(), g_tail-g_head);
    
}

//std::map<DWORD, FMOD::Channel*>					g_ch_fmodch_map;	//<channel, fmodchannel>

//void do_BASS_ChannelPlay(int i)
//{
//	
//	DWORD hChannel = psd->data[i].hChannel;
//	printf("ch: %p\n", hChannel);
//	if (!hChannel)
//		return;
//	DWORD hSample = g_channel_map[hChannel];
//	if (!hSample)
//		return;
//	FMOD::Sound *sound = g_sample_map[hSample];
//	FMOD::Channel *fmod_ch;
//	if(sound)
//	{
//		fmodSystem->playSound(sound, 0, false, &fmod_ch);
//		fmodSystem->update();
//		g_ch_fmodch_map[hChannel] = fmod_ch;
//		//printf("play smple 1\n");
//	}
//	
//	//printf("%d, %d, %d\n", g_channel_map.size(), g_sample_map.size(), g_ch_fmodch_map.size());
//}
void do_BASS_ChannelStop(int i)
{
	DWORD ch = psd->data[i].hChannel;
	if (!ch)
		return;
	FMOD::Channel *fmod_ch = NULL;
	DWORD idx = g_tail;
	while (idx!=g_head)
	{
		if (g_ch_fmodch_node[idx].ch == ch)
		{
			fmod_ch = g_ch_fmodch_node[idx].fmodch;
			break;
		}
		idx--;
		idx%=TEMP_CH_SIZE;
	}
	if(fmod_ch)
	{
		fmod_ch->stop();
		//g_channel_map.erase(hChannel);
		//g_ch_fmodch_map.erase(hChannel);
		//printf("play smple 1\n");
	}
}
DWORD work=1;

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	printf("on exit...\n");
	work = 0;
	psd->onExit = true;
	return TRUE;
}

DWORD WINAPI mainloop(LPVOID param) 
{
	while(work)
	{
		WaitForSingleObject(osuRequest, INFINITE);
		for(int i=0; i<5; i++)
		{
			if(psd->data[i].request)
			{
				//psd->data[i].injectIsBusy = 1;
				psd->data[i].request  = 0;
				switch (psd->data[i].requestId)
				{
				case OSU_REQUEST_SAMPLE_LOAD:
					do_BASS_SampleLoad();
					break;
				case OSU_REQUEST_SAMPLE_LOAD_MEM:
					do_BASS_SampleLoad_mem();
					break;
				case OSU_REQUEST_CHANNEL_PLAY:
					do_bind_sample(i);
					break;
				case OSU_REQUEST_CHANNEL_STOP:
					do_BASS_ChannelStop(i);
					break;
				}
				psd->data[i].injectIsBusy = 0;
			}
		}
#if _DEBUG		
		printf("%lf\n", (double)(psd->m_liEnd.QuadPart - psd->m_liStart.QuadPart)/(double)psd->m_liFreq.QuadPart);
#endif
	}
	return 0;
}

int _tmain(int argc, TCHAR *argv[])
{ 
	printf("AsioHookForOsu v0.8.2\n");
	printf("=====================\n");
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	GetModuleFileName(NULL, szHookDllPath, 1024); 
	char * p = strrchr(szHookDllPath, '\\');
	*p = 0;
	strcat(szHookDllPath, "\\asioHook.dll");

	GetModuleFileName(NULL,szConfigPath, 1024); 
	p = strrchr(szConfigPath, '\\');
	*p = 0;
	strcat(szConfigPath, "\\config.ini");

	GetModuleFileName(NULL,szSampleFolderPath, 1024); 
	p = strrchr(szSampleFolderPath, '\\');
	*p = 0;
	strcat(szSampleFolderPath, "\\bakupSamples\\");



	initAsio();
	//
	HMODULE hsd = LoadLibrary("sharedData.dll");
	if (!hsd)
	{
		printf("please put sharedData.dll to osu!.exe dir\n");
		system("pause");
		return 0;
	}
	PgetSharedData pgetsd = (PgetSharedData)GetProcAddress(hsd, "getSharedData");
	psd = pgetsd();
	psd->onExit = false;

	//system("pause");
	if (InjectDll())
		_tprintf(_T("InjectDll sucess!\n"));
	else
	{
		_tprintf(_T("InjectDll failed!\n"));
		return -1;
	}

	Sleep(2000); //wait for asioHook.dll to create the osuRequest Semaphore
	osuRequest = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "osuRequest");
	if (!osuRequest)
	{
		printf("can not open Semaphore osuRequest!\n");
		system("pause");
		return 0;
	}
	printf("globel Semaphore osuRequest: %p\n", osuRequest);

	HANDLE h1 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mainloop, 0, 0, 0);

	SetThreadPriority(h1, curPriority);
	WaitForSingleObject(h1, INFINITE);
	//MSG msg;
	//while(GetMessage(&msg, NULL, 0, 0)) {
	//	TranslateMessage(&msg);
	//	printf("%p\n", msg.message);
	//	if (msg.message == WM_DESTROY)
	//		break;
	//	DispatchMessage(&msg);
	//}
	//work = 0;
	return 0;
}