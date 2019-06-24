// Monitor_win.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream> 
#include "NetInfo.h"
#include "getCPUInfo.h"
#include "CpuInfo.h"
#include "MemoryInfo.h"
#include "DiskInfo.h"

using namespace std;

int main()
{
	std::cout << "===cpu infomation===" << std::endl;
	V2Vnms::CCpuInfo mycpu;
	mycpu.GetDevInfo();
	std::cout << "===memory information===" << std::endl;
	V2Vnms::CMemoryInfo mymem;
	mymem.GetDevInfo();
	std::cout << "===disk information===" << std::endl;
	V2Vnms::CDiskInfo mydisk;
	mydisk.GetDevInfo();
	std::cout << "===net information===" << std::endl;
	V2Vnms::CNetInfo mynet;
	mynet.GetDevInfo();


	system("pause");
    return 0;
}

