#include "DiskInfo.h"

namespace V2Vnms
{

CDiskInfo::CDiskInfo()
{
	mnVolCount = 0;
	mlluTotalSpace = 0;
	mlluUsedSpace = 0;
}

CDiskInfo::~CDiskInfo()
{
	
}

void CDiskInfo::Append(CDevInfo *pDev)
{
	
}

void CDiskInfo::Erase(CDevInfo *pDev)
{
	
}

int CDiskInfo::GetDevInfo()
{
	getDiskInfo();
	return 0;
}
/*
use GetLogicalDrives, If the function succeeds, the return value is a bitmask representing the currently available disk drives.
Bit position 0 (the least-significant bit) is drive A, bit position 1 is drive B, bit position 2 is drive C, and so on.
If the function fails, the return value is zero. To get extended error information, call GetLastError.
*/
int CDiskInfo::getDiskInfo()
{
	int DiskCount = 0;
	DWORD DiskInfo = GetLogicalDrives(); 
	if (DiskInfo == 0)
	{
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dwError = GetLastError();
		// FORMAT_MESSAGE_ALLOCATE_BUFFER - allocates a buffer large enough to hold the formatted message, 
		// and places a pointer to the allocated buffer at the address specified by lpBuffer. 
		// The lpBuffer parameter is a pointer to an LPTSTR, and should use LocalFree function to free the buffer.（can use a pre alloced buffer instead）
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		_tprintf(_T("%s"), lpMsgBuf);
		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,LocalSize(lpDisplayBuf) / sizeof(TCHAR),TEXT(" failed with error %d: %s"), dwError, lpMsgBuf);
		_tprintf(_T("%s"), lpDisplayBuf);
		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
	}
	volumeInfo tmpVol;
	while (DiskInfo)//通过循环操作查看每一位数据是否为1，如果为1则磁盘为真,如果为0则磁盘不存在。  
	{
		if (DiskInfo & 1) 
		{
			++DiskCount;
		}
		DiskInfo = DiskInfo >> 1;//通过位运算的右移操作保证每循环一次所检查的位置向右移动一位。  
								 //DiskInfo = DiskInfo/2;  
	}
	std::cout << "Logical Disk Number:" << DiskCount << std::endl;
	//-----------------------------------------------------------------------------------------
	//通过GetLogicalDriveStrings()函数获取所有驱动器字符串信息长度。  
	int DSLength = GetLogicalDriveStrings(0, NULL);
	WCHAR* DeviceStr = new WCHAR[DSLength];
	//通过GetLogicalDriveStrings将字符串信息复制到堆区数组中,其中保存了所有驱动器的信息。  
	GetLogicalDriveStrings(DSLength, (LPTSTR)DeviceStr);
	WCHAR* pos = DeviceStr;

	int DeviceType;
	BOOL fResult;
	TCHAR dir[4] = {};
	for (int i = 0; i < DSLength / 4; ++i)//DeviceStr内部保存的数据是C:\.D:\.E:\.，这样的信息，所以DSLength/4可以获得具体大循环范围  
	{
		// get name 
		//	wcsncpy_s(dir, pos, 3);
		wcscpy_s(dir, pos);
		tmpVol.driverName = dir;
		pos = wcschr(pos, L'\0');
		if (pos != NULL)
			pos++;
		// get type - GetDriveType函数，可以获取驱动器类型，参数为驱动器的根目录 
		DeviceType = GetDriveType(dir);
		 
		if (DeviceType == DRIVE_FIXED){
			tmpVol.driverType = _T("Hard Disk");
		}
		else if (DeviceType == DRIVE_CDROM){
			tmpVol.driverType = _T("CD-ROM");
		}
		else if (DeviceType == DRIVE_REMOVABLE)	{
			tmpVol.driverType = _T("Removable Disk");
		}
		else if (DeviceType == DRIVE_REMOTE){
			tmpVol.driverType = _T("Network Disk");
		}
		else if (DeviceType == DRIVE_RAMDISK){
			tmpVol.driverType = _T("Virtual RAM Disk");
		}
		else if (DeviceType == DRIVE_UNKNOWN){
			tmpVol.driverType = _T("Unknown Device");
		}
		//GetDiskFreeSpaceEx - 可用空间和总空间与用户配额有关，如配额只有1G，则TotalByte为1G
		fResult = GetDiskFreeSpaceEx(
			dir,
			(PULARGE_INTEGER)&tmpVol.UserAvailByte,  
			(PULARGE_INTEGER)&tmpVol.TotalByte,
			(PULARGE_INTEGER)&tmpVol.FreeByte);
		if (fResult)  
		{
			tmpVol.UsedByte = tmpVol.TotalByte - tmpVol.UserAvailByte;
			mvVolums.push_back(tmpVol);
			std::wcout << " Volume: " << tmpVol.driverName << " type: " << tmpVol.driverType << " avail: " << (float)tmpVol.UserAvailByte / GBYTES
				<< " GB/" << (float)tmpVol.TotalByte/GBYTES << " GB" << std::endl;  
		}
		else
		{
			DWORD dwError = GetLastError();
			std::cout << dwError << std::endl;
		}
	}
	delete[] DeviceStr;
	return 0;
}



}