#include "stdafx.h"
#include <string.h>
#include <string>
using namespace std;
int readFile()
{
	FILE *fp;
	string filename = "W:\\service01\\info.mycnki.log";
	char buffer[10240] = "";
	errno_t err;
	if ((err = fopen_s(&fp, filename.c_str(), "r"))!=NULL)     //判断文件打开
	{
		char szLog[1280]="";
		char szError[512]="";
		printf_s(szLog, "Couldn't open %s!\n\n %s", filename.c_str(), szError);
		exit(-1);    // #include <stdlib.h>                     
	}
	printf("Opened %s", filename.c_str());
	fseek(fp, (long)50113, 0);
	fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	//
	FILE *save_fp;
	string save_filename = "buffer.txt";
	if (!( fopen_s(&save_fp, save_filename.c_str(), "w")))     //判断文件打开
	{
		TCHAR szLog[1280];
		TCHAR szError[1024];
		_stprintf_s(szLog, _T("Couldn't open %hs!\n\n %Ts"), save_filename.c_str(), szError);
		exit(-1);    // #include <stdlib.h>                     
	}
	fwrite(buffer, 1, sizeof(buffer), save_fp);
	fclose(save_fp);
	return 0;
}