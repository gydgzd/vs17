// ConsoleTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
   
#include <stdlib.h>    
#include <crtdbg.h>        // 内存检测
#include <WinSock2.h>
#ifdef _DEBUG  
#define _CRTDBG_MAP_ALLOC 

#endif

#ifdef WINVER
#define    WIN32_LEAN_AND_MEAN   //去除一些不常用的, 如winsock.h
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
#include "MyBitree.h"

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
	//	char a[] = "hahaha";  //返回指向局部变量的地址，编译警告
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
extern std::string Utf8ToGbk(const char *src_str);
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
void WINAPI ServiceHandler(DWORD fdwControl);
DWORD WINAPI MyWork(LPVOID lpParam);
// 服务注册: sc create abcTest binpath= D:\git_project\vs17\ConsoleTest\Release\ConsoleTest.exe
// 修改显示名称: sc config abcTest DisplayName="abcTest"
// 修改描述: sc description abcTest "probe"
// 开机启动: sc config abcTest start= auto

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

int fibonacci(int n)
{
    if (n <= 0)
        return 0;
    cout << "The fibonacci serial of " << n << " is:  ";
    if (n == 1 )
    {
        cout << 1 << "  ";
        return 1;
    }
    cout << "1  ";
    int pre1 = 0;
    int pre2 = 1;
    int sum = 0;
    for (int i = 1; i < n ; i++)
    {
        sum = pre1 + pre2;
        pre1 = pre2;
        pre2 = sum;
        cout << sum << "  ";
    }
    cout << endl;
    return sum;
}

Mylog g_mylog;

int main(int argc, char** argv)
{
    initWinSocket();
    LogInit();
    /*
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(1041);	   //在内存分配之前设置内存中断块号
	char *pleak = testLeak();	
	_CrtDumpMemoryLeaks();
    */
    unsigned int id[4] = {};
    id[0] = 399;
    id[1] = 166;
    unsigned char *p = (unsigned char *)id;               // 高精度转化为低精度，内存占用对应减小，只留下低位
    printf("%d - %d\n", (unsigned int)*p, (unsigned int)*(p + 4));
    printf("%d - %d\n", *(unsigned int*)p, *(unsigned int*)(p + 4)); // 低精度转化为高精度，内存占用不会增大,可以先转换指针类型，然后去引用
    

//    setrgb(BLACK, INT_MAGENTA);  //设置背景和前景色

    /*
    myTunTap mytun;
    int ret = mytun.init("ROOT\\NET\\0002", "10.1.1.1", "255.255.255.0", "10.1.1.1", 1500);
    while (ret != 0)
    {
        printf("ERR: 初始化虚拟网卡失败, 请检查虚拟网卡是否可用,5s后将重试。\n");
        Sleep(5000);
        ret = mytun.init("ROOT\\NET\\0002", "10.1.1.1", "255.255.255.0", "10.1.1.1", 1500);
    }
    mytun.process();
    */

	testList();
	testMap();
	testHashMap();
//	testSet();
    fibonacci(10);
    MyBitree<int> bt;
    int aray[6] = {6, 7, 1, 3, 4, 5, };
    bt.init(&bt, aray, 6);
    bt.preTraversal(&bt);
    cout << endl;
    bt.midTraversal(&bt);
    cout << endl;
    bt.postTraversal(&bt);
    cout << endl;
    //	testValist();

    std::string str = Utf8ToGbk("世界"); // _T("你好");
    cout << str << endl;

	// test of sort
/**/
    float fa[] = { 1.2, 0.5, 3.6, 0.1, 3.4, 1.9, 33, 22, 35,1242,11,242.24,2313824,232313,4755,2325,92.81 };
	float *pa = fa;
	MySort<float> mSort;
	mSort.insertionSort(fa, sizeof(fa) / sizeof(fa[0]));

	for (int i = 0; i<sizeof(fa) / sizeof(fa[0]); i++)
		cout << fa[i] << "  ";


    /*
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
    */
    LOG(INFO) << "main finished.";
#if defined(__linux)
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
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;      // 即服务目前状态为 正在初始化
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
	// 从这里开始可以放入你想服务为你所做的事情。
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