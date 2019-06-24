
#pragma once
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include "global.h"

#include "DevInfo.h"
#include <winternl.h>  // should be after #include <windows.h>

#pragma comment(lib,"ntdll.lib")
using namespace std;
namespace V2Vnms
{
struct CPU_Time
{
	char szName[32];
	UINT64_DELTA lluKernelTime;
	UINT64_DELTA lluUserTime;
	UINT64_DELTA lluIdleTime;
	UINT64_DELTA lluDpcTime;
	UINT64_DELTA lluInterruptTime;
};

struct CPUCoreRate
{
	std::string cpuName;   // core name
	float  rate = 0;       // core usage

};
// 参考 https://baike.baidu.com/item/CPUID/5559847?fr=aladdin
class CCpuInfo : public CDevInfo
{
public:
	CCpuInfo();
	virtual ~CCpuInfo();
	virtual void Append(CDevInfo *pDev);
	virtual void Erase(CDevInfo *pDev);
	
	virtual int GetDevInfo();	
public:
	int         mn_cpuCores;           // num of cores
	float       m_cpuTotalUsage;            // total usage
	std::string mstr_cpuModel;         // "Intel(R) Xeon(R) CPU E3-1220 v3 @ 3.10GHz"
	std::string mstr_vendor;           // GenuineIntel
	vector<CPUCoreRate> mv_cores;      // to store data of every core

private:
	void initCPU(DWORD veax);      // init cpu in assembly language
	int         getCPUCores();     // get number of cores
	long        getBasicFreq();    // CPU basic frequency, not used
	std::string getVendor();       // 
	std::string getModelName();    //  
	int         getCPUTime(int getUsage, CPU_Time * cpu_cores_time);      // get all kind of time, like kernel time, user time.
	int         getCPUUsage();     // calc CPU usage
	// save 4 register variables
	DWORD m_deax;
	DWORD m_debx;
	DWORD m_decx;
	DWORD m_dedx;
	CPU_Time m_CPU_Time;
};
}
