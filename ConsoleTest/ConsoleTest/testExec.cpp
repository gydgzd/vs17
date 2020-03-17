#include "stdafx.h"
#include <stdlib.h>

#include <windows.h>
#include <atlconv.h>
extern void showError();

void myExec(char *cmd)
{
	if (cmd == NULL || *cmd == 0)
		return;


//	WinExec(cmd, SW_SHOW);

/*	HINSTANCE hNewExe = ShellExecuteA(NULL, "open", "cmd", "/c ipconfig /all", NULL, SW_SHOW);//要执行cmd, ipconfig /all  会报文件不存在
	if ((DWORD)hNewExe <= 32)
	{
		printf("return value:%d\n", (DWORD)hNewExe);
	}
	else
	{
		printf("successed!\n");
	}
	*/
	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO  StartupInfo;       //This  is  an  [in]   parameter   

	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo); 
	LPWSTR cmdLine = _T("notepad\0");
	if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
	}
	else
	{
		showError();
	}
}
