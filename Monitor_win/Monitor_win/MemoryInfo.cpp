#include "MemoryInfo.h"

namespace V2Vnms
{
CMemoryInfo::CMemoryInfo()
{

}

CMemoryInfo::~CMemoryInfo()
{
	
}

void CMemoryInfo::Append(CDevInfo *pDev)
{
	
}

void CMemoryInfo::Erase(CDevInfo *pDev)
{
	
}

int CMemoryInfo::GetDevInfo()
{
	getMemoryInfo();
	return 0;
}
/*
use GlobalMemoryStatusEx, need #include <Windows.h>
minimum support Windows XP/Windows Server 2003
参考https://docs.microsoft.com/zh-cn/windows/desktop/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
*/
int CMemoryInfo::getMemoryInfo()
{
	std::string memory_info;
	TCHAR msg[256] = _T("");
	MEMORYSTATUSEX mem_status;
	mem_status.dwLength = sizeof(mem_status);
	if (GlobalMemoryStatusEx(&mem_status))
	{
		
		// get data
		m_memTotal = mem_status.ullTotalPhys / MBYTES;
		m_memAvail = mem_status.ullAvailPhys / MBYTES;
		m_memVirtTotal = mem_status.ullTotalVirtual / MBYTES;
		m_memVirtAvail = mem_status.ullAvailVirtual / MBYTES;
		m_memRate = mem_status.dwMemoryLoad;            // 百分比
		//
		_tprintf(TEXT("There is  %*ld percent of memory in use.\n"),
			WIDTH, mem_status.dwMemoryLoad);
		_tprintf(TEXT("There are %*I64d total MB of physical memory.\n"),
			WIDTH, mem_status.ullTotalPhys / MBYTES);
		_tprintf(TEXT("There are %*I64d free  MB of physical memory.\n"),
			WIDTH, mem_status.ullAvailPhys / MBYTES);

		_tprintf(TEXT("There are %*I64d total MB of virtual memory.\n"),
			WIDTH, mem_status.ullTotalVirtual / MBYTES);
		_tprintf(TEXT("There are %*I64d free  MB of virtual memory.\n"),
			WIDTH, mem_status.ullAvailVirtual / MBYTES);
	}
	else
		std::cout << "function Error: GlobalMemoryStatusEx()\n" << std::endl;
	return 0;
}

}