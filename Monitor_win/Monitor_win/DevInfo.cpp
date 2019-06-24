
#include "DevInfo.h"
#include <VersionHelpers.h>
namespace V2Vnms
{
CDevInfo::CDevInfo()
{
}

CDevInfo::~CDevInfo()
{
	m_DevVector.clear();
}

int CDevInfo::Init()
{
	return 0;
}

void CDevInfo::UnInit()
{

}

void CDevInfo::Append(CDevInfo *pDev)
{
	if(pDev != NULL)
	{
		m_DevVector.push_back(pDev);
	}
}
	
void CDevInfo::Erase(CDevInfo *pDev)
{
	std::vector<CDevInfo *>::iterator iter = std::find(m_DevVector.begin(), m_DevVector.end(), pDev);
	if(iter != m_DevVector.end())
	{
		m_DevVector.erase(iter);
	}
}
	
int CDevInfo::GetDevInfo(std::string &strDevInfo)
{
	std::string strTemp = "";
	strDevInfo = "";
	std::vector<CDevInfo *>::iterator iter = m_DevVector.begin();
	for(; iter != m_DevVector.end(); iter++)
	{
		if(!strDevInfo.empty())
		{
			strDevInfo += ",";
		}
		strTemp = "";
		(*iter)->GetDevInfo(strTemp);
		strDevInfo += strTemp;
	}		
	return 0;
}
/*
GetFileVersionInfoSize   GetFileVersionInfo
https://docs.microsoft.com/zh-cn/windows/desktop/SysInfo/operating-system-version


*/
void getWinVersion()
{
	if (IsWindows8OrGreater())        // windows 8
	{
		if (IsWindows8Point1OrGreater())
		{
			printf("Windows8Point1 or Greater.\n");
		}
		else
		{
			if (IsWindowsServer())
				printf("Windows Server 2012.\n");
			else
				printf("Windows 8.\n");
		}
	}
	else if (IsWindows7OrGreater())    // windows 7
	{
		if (IsWindows7SP1OrGreater())
			printf("Windows7SP1 or Greater.\n");
		else
		{
			if (IsWindowsServer())
				printf("Windows Server 2008 R2.\n");
			else
				printf("Windows 7.\n");
		}
	}
	else if (IsWindowsVistaOrGreater())  // windows Vista
	{
		if (IsWindowsVistaSP2OrGreater())
			printf("WindowsVistaSP2 or Greater.\n");
		else if (IsWindowsVistaSP1OrGreater())
			printf("WindowsVistaSP1 or Greater.\n");
		else
		{
			if (IsWindowsServer())
				printf("Windows Server 2008.\n");
			else
				printf("Windows Vista.\n");
		}
	}
	else if (IsWindowsXPOrGreater())      // windows XP
	{
		if (IsWindowsXPSP3OrGreater())
			printf("Greater than WindowsXPSP3.\n");
		else if (IsWindowsXPSP2OrGreater())
			printf("Greater than WindowsXPSP2.\n");
		else if (IsWindowsXPSP1OrGreater())
			printf("Greater than WindowsXPSP1.\n");
		else
			printf("Lower than WindowsXPSP1.\n");
	}
	return;
}
}
