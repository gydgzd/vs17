#include "stdafx.h"
#include <stdlib.h>

#include <windows.h>

void myExec(char *cmd)
{
	if (cmd == NULL || *cmd == 0)
		return;


	WinExec(cmd, SW_SHOW);

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
}