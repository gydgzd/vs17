// ConsoleTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
   
#include <stdlib.h>    
#include <crtdbg.h>        // �ڴ���
#include <WinSock2.h>
#ifdef _DEBUG  
#define _CRTDBG_MAP_ALLOC 

#endif

#ifdef WINVER
#define    WIN32_LEAN_AND_MEAN   //ȥ��һЩ�����õ�, ��winsock.h
#include <windows.h>

#include <direct.h>        // _mkdir
//#include <strsafe.h>       // StringCchPrintf   conflict with sprintf,strcpy, and so on
#endif // WINVER

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <queue>
#include <locale.h>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <string>
#include <string.h>

//#include <iomanip>
#include "sql_conn_cpp.h"  // my sql class
#include "MySort.h"
#include "Mylog.h"
#include "testMultiThread.h"
#include "testStack.cpp"
#include "ProcessMonitor.h"
#include "CFileVersion.h"
#include "TestWinPcap.h"
#include "myTunTap.h"
#include "easylogging++.h"    // v9.96.7
INITIALIZE_EASYLOGGINGPP      // needed by easylogging
#pragma comment(lib,"ws2_32.lib")
//#include "testValist.cpp"
using namespace std;
int socketServer();
// INITIALIZE_EASYLOGGINGPP add to easylogging++.h
/**/void LogInit()
{
	el::Configurations conf("log.conf");
	el::Loggers::reconfigureAllLoggers(conf);
}

#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)  
char* testLeak()
{
	char *test = new char[4];
	return test;
}
char * testLocal()
{
	char * a = "hahaha";
	//	char a[] = "hahaha";  //����ָ��ֲ������ĵ�ַ�����뾯��
	return a;
}
extern int readFile();
extern int str_replace(char str[], int size, int strlenth);
extern int str_compare(char *str1, char *str2);
 
extern int log(const char * fmt, ...);
extern void testValist();
extern void printf_t(FILE *m_file, const char *fmt ...);

extern void simpleInterativeReverse();
extern void interativeReverse();
extern int parse_liveout();
extern int testSet();
extern int readFileStream();
extern int writeFileStream();
extern int testHashMap();
extern void testMap();
extern void testVector();
extern int testList();
extern time_t dateToSeconds(char *str);
extern void testVolatile();
extern int testWMI();
extern void getProcess();
extern void myExec(char *cmd);
extern void printError_Win(const char *msg = "");

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
void WINAPI ServiceHandler(DWORD fdwControl);
DWORD WINAPI MyWork(LPVOID lpParam);
// ����ע��: sc create abcTest binpath= D:\git_project\vs17\ConsoleTest\Release\ConsoleTest.exe
// �޸���ʾ����: sc config abcTest DisplayName="abcTest"
// �޸�����: sc description abcTest "probe"
// ��������: sc config abcTest start= auto

void setrgb(int bgc, int fgc)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((bgc << 4) + fgc));
}
enum {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    YELLOW,
    GRAY,
    INT_GRAY,
    INT_BLUE,
    INT_GREEN,
    INT_CYAN,
    INT_RED,
    INT_MAGENTA,
    INT_YELLOW,
    INT_WHITE
};
int g_socket;
void initWinSocket()
{
#ifdef WINVER
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);    // init windows socket dll
#endif
    g_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
}

void mysleep(long sec, long us)
{

    struct timeval tv;    // wait
    fd_set dummy;
    FD_ZERO(&dummy);
    FD_SET(g_socket, &dummy);
    tv.tv_sec = sec;
    tv.tv_usec = us;
    select(0, 0, 0, &dummy, &tv);
}

Mylog g_mylog;
int main(int argc, char** argv)
{
    
/*
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(1041);	   //���ڴ����֮ǰ�����ڴ��жϿ��
	char *pleak = testLeak();	
	_CrtDumpMemoryLeaks();
    */
    unsigned int id[4] = {};
    id[0] = 399;
    id[1] = 166;
    unsigned char *p = (unsigned char *)id;               // �߾���ת��Ϊ�;��ȣ��ڴ�ռ�ö�Ӧ��С��ֻ���µ�λ
    printf("%d - %d\n", (unsigned int)*p, (unsigned int)*(p + 4));
    printf("%d - %d\n", *(unsigned int*)p, *(unsigned int*)(p + 4)); // �;���ת��Ϊ�߾��ȣ��ڴ�ռ�ò�������,������ת��ָ�����ͣ�Ȼ��ȥ����
    initWinSocket();
    LogInit();
//    setrgb(BLACK, INT_MAGENTA);  //���ñ�����ǰ��ɫ

	SERVICE_TABLE_ENTRY ServTable[2];
	ServTable[0].lpServiceName = _T("abcTest");
	ServTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	ServTable[1].lpServiceName = NULL;
	ServTable[1].lpServiceProc = NULL;
	if (StartServiceCtrlDispatcher(ServTable))
	{
	//	mylog.logException("Service start succeed.");
	}
	else
	{
    //    showError();
	}
    myTunTap mytun;
    int ret = mytun.init("ROOT\\NET\\0002", "10.1.1.1", "255.255.255.0", "10.1.1.1", 1500);
    while (ret != 0)
    {
        printf("ERR: ��ʼ����������ʧ��, �������������Ƿ����,5s�����ԡ�\n");
        Sleep(5000);
        ret = mytun.init("ROOT\\NET\\0002", "10.1.1.1", "255.255.255.0", "10.1.1.1", 1500);
    }
    mytun.process();
	testList();
	testMap();
	testHashMap();
//	testSet();

//	testValist();

/*	wcout.imbue(locale("")); 
	char * lcname = setlocale(LC_ALL, "chs");
	if (NULL == lcname)
	{
		printf("setlocale() from environment failed.\n");
	}
*/	wchar_t szError[1024] = L"����"; // _T("���");
    char tmp[] = "��";
//	swprintf_s(szError, L"����");
	memcpy_s(szError, 10, L"����", sizeof(L"����"));
    cout << "sizeof(L\"����\"):" << sizeof(L"����") << "  sizeof(\"����\"):" << sizeof("����") << endl;
    cout << "strlen(L\"����\"):" << wcslen(L"����") << "  sizeof(\"����\"):" << strlen("����") << endl;
	_stprintf_s(szError, L"����,��ã�");
	wcout << szError << endl;
	printf("printf���\n");

	// test of sort
/**/	
    float fa[] = { 1.2, 0.5, 3.6, 0.1, 3.4, 1.9, 33, 22, 35,1242,11,242.24,2313824,232313,4755,2325,92.81 };
	float *pa = fa;
	MySort<float> mSort;
	mSort.insertionSort(fa, sizeof(fa) / sizeof(fa[0]));

	for (int i = 0; i<sizeof(fa) / sizeof(fa[0]); i++)
		cout << fa[i] << "  ";

    LOG(INFO) << "main finished.";
#ifdef __linux
	printf("Linux\n");
#elif (defined WINVER ||defined WIN32)
	printf("Windows\n");
#endif
//	system("pause");
	return 0;
}

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
//	mylog.logException("Service start");
	hStatus = RegisterServiceCtrlHandler(_T("abcTest"),	ServiceHandler);  // (LPHANDLER_FUNCTION)ServiceHandler
	if (!hStatus)
	{
        printError_Win("RegisterServiceCtrlHandler");
		return;
	}
	else
	{
	//	mylog.logException("Register Service successful!");
	}

	ServiceStatus.dwServiceType = SERVICE_WIN32;               // SERVICE_WIN32_OWN_PROCESS
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;      // ������Ŀǰ״̬Ϊ ���ڳ�ʼ��
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwWaitHint = 5000;
	SetServiceStatus(hStatus, &ServiceStatus);
	if (GetLastError() != NO_ERROR)
	{
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		ServiceStatus.dwWaitHint = 0;
		SetServiceStatus(hStatus, &ServiceStatus);
//		mylog.logException("Start Service Error!");
		LOG(INFO) << "Start Service Error!";
		return;
	}
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	SetServiceStatus(hStatus, &ServiceStatus);
	// �����￪ʼ���Է����������Ϊ�����������顣
	HANDLE hThread = CreateThread(NULL, 0, MyWork, NULL, 0, NULL);
	if (hThread == NULL)
		return;
}

void WINAPI ServiceHandler(DWORD fdwControl)
{
	switch (fdwControl)
	{
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		ServiceStatus.dwWaitHint = 0;
        break;
	default:
		break;
	}

	if (SetServiceStatus(hStatus, &ServiceStatus)==0)
	{
        printError_Win("SetServiceStatus");
	}
	return;
}

DWORD WINAPI MyWork(LPVOID lpParam)
{
#ifdef WINVER
	_mkdir("D:/log");     // the relative path is C:\Windows\SysWOW64\    
#endif // WINVER
	TCHAR tstr[512];
	GetModuleFileName(NULL, tstr, 512);
	USES_CONVERSION;
	char *str = W2A(tstr);
	TCHAR dir[512] = _T("");
	GetCurrentDirectory(512, dir);

	char *curDir = W2A(dir);
	ofstream ofs("D:/log/serverlog.txt", ios::app);
	if (ofs.fail())
	{
		cerr << "Failed to open log file: " << strerror(errno) << endl;
		return -1;
	}
	string mytime = getLocalTime("%Y-%m-%d %H:%M:%S");
	ofs << mytime << " GetModuleFileName:" << str << endl;
	ofs << mytime << " GetCurrentDirectory:" << curDir << endl;
	ofs.close();

	return 0;
}
BOOL IsInstall()
{
	BOOL bResult = FALSE;

	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		SC_HANDLE schService = OpenService(schSCManager, TEXT("abcTest"), SERVICE_ALL_ACCESS);
		if (schService)
		{
			bResult = TRUE;
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
		MessageBox(NULL, TEXT("OpenSCManager error"), TEXT("error"), MB_OK | MB_ICONERROR);
	}

	return bResult;
}

BOOL Install()
{
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!schSCManager)
	{
		MessageBox(NULL, TEXT("OpenSCManager error"), TEXT("error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	TCHAR szFilePath[MAX_PATH];
	GetModuleFileName(NULL, szFilePath, MAX_PATH);

	SC_HANDLE schService = CreateService(schSCManager, TEXT("abcTest"), TEXT("abcTest"), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, TEXT(""), NULL, NULL);
	if (!schService)
	{
		CloseServiceHandle(schSCManager);
		MessageBox(NULL, TEXT("CreateService error"), TEXT("error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return TRUE;
}
