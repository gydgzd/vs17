#pragma once

#include "DevInfo.h"
#include <strsafe.h>   // StringCchPrintf
namespace V2Vnms
{
struct volumeInfo      // 每个卷的信息
{
	std::wstring driverName;
	std::wstring driverType;
	ULONG64 TotalByte;
	ULONG64 FreeByte;
	ULONG64 UsedByte;    //  TotalByte - FreeByte
	ULONG64 UserAvailByte;
};
class CDiskInfo : public CDevInfo
{
public:
	CDiskInfo();
	virtual ~CDiskInfo();
	virtual void Append(CDevInfo *pDev);
	virtual void Erase(CDevInfo *pDev);

	virtual int GetDevInfo();
	
private:
	int getDiskInfo();
	

private:
	int     mnVolCount;        // number of volumes
	ULONG64 mlluTotalSpace;    // total
	ULONG64 mlluUsedSpace;     // used
	std::vector<volumeInfo> mvVolums;  // all  volumes
};
}

