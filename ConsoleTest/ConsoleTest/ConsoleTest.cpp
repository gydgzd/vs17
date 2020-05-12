// ConsoleTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#define _CRTDBG_MAP_ALLOC    
#include <stdlib.h>    
#include <crtdbg.h>        // 内存检测
#ifdef _DEBUG  
#define New   new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif
#include "getDate.h"
#ifdef WINVER
#define    WIN32_LEAN_AND_MEAN   //去除一些不常用的
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
#include <iomanip>
#include "sql_conn_cpp.h"  // my sql class
#include "MySort.h"
#include "Mylog.h"
#include "testMultiThread.h"
#include "testStack.cpp"
#include "ProcessMonitor.h"
#include "CFileVersion.h"
#include "TestWinPcap.h"
//#include "testValist.cpp"
using namespace std;
int socketServer();

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
void showError() 
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0, // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	USES_CONVERSION;
	//  LOG(ERROR) << std:: string(T2A((LPCTSTR)lpMsgBuf));
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
	LocalFree(lpMsgBuf);
}
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
void WINAPI ServiceHandler(DWORD fdwControl);
DWORD WINAPI MyWork(LPVOID lpParam);
// 服务注册: sc create abcTest binpath= D:\git_project\vs17\ConsoleTest\Release\ConsoleTest.exe
// 修改显示名称: sc config abcTest DisplayName="abcTest"
// 修改描述: sc description abcTest "probe"
// 开机启动: sc config abcTest start= auto
Mylog mylog("D:/log/log.txt");
int main(int argc, char** argv)
{
//	_CrtSetBreakAlloc(153);	
//	myExec("ipconfig /all");
	showError();
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
//	char *test = testLeak();	
	_CrtDumpMemoryLeaks();

	TestWinPcap test;
//	test.process();
	for(int i = 0; i < 100; i++)
		mylog.logException_fopen("hei, fopen");
	mylog.logException_fopen("hei, fopen");

	testVector();

	cout << uppercase << hex << "12av" << endl;
	//MessageBox(0, _T("Begin Service!\n"), _T("INFO"), 0);
	
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
        showError();
	}
	CFileVersion fv;
	fv.Open(_T("E:\\MediaServer_V5.16.3.0\\SLW.MediaServer.exe"));
	CString descption = fv.getFileDescription();
//	testWMI();
	/*
//	getProcess();
	ProcessMonitor pm;
	while (true)
	{
		pm.getProcess_Win();
		pm.mlistProcess.clear();
		mylog.logException("Run one time.");
		Sleep(3000);
		system("cls");
	}
	*/
socketServer();
	/*
	testVolatile();
	char msg[32] = "Hick lenawd";
	printf("%4.2s\n", msg);
	time_t tm1 = dateToSeconds("2019-02-20 09:52:21");
	printf("%lld\n", tm1);

	Mycounter mc1; 
	std::thread th{ &Mycounter::counter, &mc1,10, 1 };
	th.detach();
	Sleep(1000);
	testList();
	string str1 = "help";
	string a;
	stringstream ss(str1);
	getline(ss, a, 'e');
	ss >> a;
	cout << str1.substr(0, 1);
	if (str1.substr(0, 1) == "h")
		cout << "same" << endl;
	testMap();
	testHashMap();
//	testSet();
	//	parse_liveout();
//	writeFileStream();
	
//	testValist();

/*	wcout.imbue(locale("")); 
	char * lcname = setlocale(LC_ALL, "chs");
	if (NULL == lcname)
	{
		printf("setlocale() from environment failed.\n");
	}
	wchar_t szError[1024] = L"世界"; // _T("你好");
//	swprintf_s(szError, L"世界");
	memcpy_s(szError, 10, L"世界", sizeof("世界"));
	_stprintf_s(szError, L"世界,你好！");
	wcout << szError << endl;
	printf("printf你好\n");
*/

 /*
 */
	// test of cin
 /*
	char str[30];
	string s1;
	getline(cin, s1);
	cout << s1 << endl;
	cin.ignore(100,'\n');    // 清除\n之前100个字符

	cin.getline(str,30);
	cout << str << endl;
	cin.ignore(100, '\n');   //调用getline后会激活键盘输入，输入'\n'，调用get不会
	

	char array[12] = { 0x01 , 0x02 , 0x03 , 0x04 , 0x05 , 0x06 , 0x07 , 0x08 };
	short *pshort = (short *)array;
	int *pint = (int *)array;
	_int64 *pint64 = (_int64 *)array;
	printf("0x%x , 0x%x , 0x%llx , 0x%llx", *pshort, *(pshort + 2), *pint64, *(pint + 2));
	*/
/*
	myqueue sta1;
	sta1.push(1);
	sta1.push(5);
	sta1.push(3);
	sta1.push(7);
	cout << sta1.pop() << endl;
	cout << sta1.pop() << endl;
	cout << sta1.pop() << endl;
	cout << sta1.pop() << endl;
	*/
//	interativeReverse();   // cin.get不会用

	// test of sort
/*	float fa[44] = { 1.2, 0.5, 3.6, 0.1, 3.4, 1.9, 33, 22, 35, 23, 755, 23, 121, 232.12, 193.23, 1201.23,122,232.21,21213,1242,11,242.24,2313824,232313,4242,32,42,423,43564,4755,2325,92.81 };
	float *pa = fa;
	MySort<float> mSort;
	clock_t t1 = clock();
	mSort.insertionSort(fa, sizeof(fa) / sizeof(fa[0]));
	clock_t t2 = clock();
	float tt = (float)(t2 - t1)/ CLOCKS_PER_SEC;
	for (int i = 0; i<sizeof(fa) / sizeof(fa[0]); i++)
		cout << fa[i] << "  ";
	cout << tt << "s"<< endl;
	cout << pa[12] << *(fa + 12)<< endl;
	*/

#ifdef __linux
	printf("Linux\n");
#elif WINVER
	printf("Windows\n");
#endif
	system("pause");
	return 0;
}

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
//	mylog.logException("Service start");
	hStatus = RegisterServiceCtrlHandler(_T("abcTest"),	ServiceHandler);  // (LPHANDLER_FUNCTION)ServiceHandler
	if (!hStatus)
	{
	//	mylog.logException("Register Service Error!");
	//	MessageBox(0,_T("Register Service Error!\n"), _T("Error"),0);
		return;

		LPVOID buf;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&buf,
			0,
			NULL)) 
		{
			MessageBox(0, (LPTSTR)(buf), _T("Register Service Error!"), 0);
			LocalFree(buf);
		}
		else
			MessageBox(0, _T("Unknow Error!"), _T("Register Service Succeed!"), 0);;
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
		system("pause");
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
		SetServiceStatus(hStatus, &ServiceStatus);
		return;
	default:
		break;
	}

	if (SetServiceStatus(hStatus, &ServiceStatus))
	{
		LPVOID buf;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&buf,
			0,
			NULL))
		{
			MessageBox(0, (LPTSTR)(buf), _T("SetServiceStatus Error!"), 0);
			LocalFree(buf);
		}
	}
	return;
}

DWORD WINAPI MyWork(LPVOID lpParam)
{
#ifdef WINVER
	_mkdir("D:/log");     // the relative path is C:\Windows\SysWOW64\    
#endif // WINVER
	
	ofstream ofs("D:/log/serverlog.txt", ios::app);
	if (ofs.fail())
	{
		cerr << "Failed to open log file: " << strerror(errno) << endl;
		return -1;
	}
	string mytime = getLocalTime("%Y-%m-%d %H:%M:%S");
	ofs << mytime << "  ";
	ofs << endl;
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

void myERR(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process
 
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}
