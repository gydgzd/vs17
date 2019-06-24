#include "CpuInfo.h"

namespace V2Vnms
{

CCpuInfo::CCpuInfo()
{
	mn_cpuCores = 0;
	m_cpuTotalUsage = 0;
}

CCpuInfo::~CCpuInfo()
{

}

void CCpuInfo::Append(CDevInfo *pDev)
{
	
}

void CCpuInfo::Erase(CDevInfo *pDev)
{
	
}

int CCpuInfo::GetDevInfo()
{
	std::string str = "";
	cout << "Information: " << getModelName() << endl;
	cout << "Vendor: " << getVendor() << endl;
	cout << "Basic frequency: " << getBasicFreq() << endl;
	cout << "Number of cores: " << getCPUCores() << endl;
	for (int i = 0; i < 1; i++)
	{
		getCPUUsage();
		Sleep(2000);
	}
	return 0;
}
// init cpu in assembly language
void CCpuInfo::initCPU(DWORD veax)
{
	// 因为嵌入式的汇编代码不能识别 类成员变量
	// 所以定义四个临时变量作为过渡
	DWORD deax;
	DWORD debx;
	DWORD decx;
	DWORD dedx;
	__asm
	{
		mov eax, veax
		cpuid
		mov deax, eax
		mov debx, ebx
		mov decx, ecx
		mov dedx, edx
	}
	m_deax = deax;
	m_debx = debx;
	m_decx = decx;
	m_dedx = dedx;
	return;
}
// ###get vendor like "GenuineIntel" 
std::string CCpuInfo::getVendor()
{
	char manuID[32] = "";

	initCPU(0);
	memcpy(manuID + 0, &m_debx, 4); // copy to array
	memcpy(manuID + 4, &m_dedx, 4);
	memcpy(manuID + 8, &m_decx, 4);
	mstr_vendor = manuID;
	return mstr_vendor;
}
// ###get name like "Intel(R) Xeon(R) CPU E3-1220 v3 @ 3.10GHz"
std::string CCpuInfo::getModelName()
{
	const DWORD id = 0x80000002; // start 0x80000002 end to 0x80000004
	char cpuType[64] = "";

	for (DWORD t = 0; t < 3; t++)
	{
		initCPU(id + t);

		memcpy(cpuType + 16 * t + 0, &m_deax, 4);
		memcpy(cpuType + 16 * t + 4, &m_debx, 4);
		memcpy(cpuType + 16 * t + 8, &m_decx, 4);
		memcpy(cpuType + 16 * t + 12, &m_dedx, 4);
	}
	mstr_cpuModel = cpuType;
	return mstr_cpuModel;
}
/*
use NtQuerySystemInformation to get information
GetSystemTimes can be alternative(without data of each core)
getUsage = 1时，计算CPU使用率
*/
int CCpuInfo::getCPUTime(int getUsage, CPU_Time* cpu_cores_time)
{
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *CpuInformation = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mn_cpuCores];

	// for CPU
	static UINT64_DELTA cpu_kernel_delta;
	static UINT64_DELTA cpu_user_delta;
	static UINT64_DELTA cpu_idle_delta;
	static SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION cpu_totals;
	memset(&cpu_totals, 0, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));

	NtQuerySystemInformation(
		SystemProcessorPerformanceInformation,
		CpuInformation,
		sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * (ULONG)mn_cpuCores,
		NULL
	);
	
	CPUCoreRate tmp_core;
	ULONG64 tmp_core_time = 0;   // total time of each core
	ULONG64 total_time = 0;      // total time of CPU
	ULONG64 sys_time = 0;        // sys   time of CPU
	for (int i = 0; i < (int)mn_cpuCores; i++)
	{
		SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION& cpu_info = CpuInformation[i];

		// KernelTime includes idle time. 
		LONGLONG dpc_time = cpu_info.Reserved1[0].QuadPart;
		LONGLONG interrupt_time = cpu_info.Reserved1[1].QuadPart;
		cpu_info.KernelTime.QuadPart -= cpu_info.IdleTime.QuadPart;
	//	cpu_info.KernelTime.QuadPart += dpc_time + interrupt_time;

		// get one Processor data, calc the delta
		UpdateDelta(&(cpu_cores_time + i)->lluKernelTime, cpu_info.KernelTime.QuadPart);
		UpdateDelta(&(cpu_cores_time + i)->lluUserTime, cpu_info.UserTime.QuadPart);
		UpdateDelta(&(cpu_cores_time + i)->lluIdleTime, cpu_info.IdleTime.QuadPart);
		tmp_core_time = (cpu_cores_time + i)->lluKernelTime.Delta + (cpu_cores_time + i)->lluUserTime.Delta + (cpu_cores_time + i)->lluIdleTime.Delta;
		if (getUsage)
		{
			if (tmp_core_time)
				tmp_core.rate = ((cpu_cores_time + i)->lluKernelTime.Delta + (cpu_cores_time + i)->lluUserTime.Delta)*100.0 / tmp_core_time;
			else
				tmp_core.rate = 0.0;
			tmp_core.cpuName = (cpu_cores_time + i)->szName;
			mv_cores.push_back(tmp_core);
		}
		// get total 
		cpu_totals.KernelTime.QuadPart += cpu_info.KernelTime.QuadPart;
		cpu_totals.UserTime.QuadPart += cpu_info.UserTime.QuadPart;
		cpu_totals.IdleTime.QuadPart += cpu_info.IdleTime.QuadPart;
	}
	delete[]CpuInformation;
	UpdateDelta(&cpu_kernel_delta, cpu_totals.KernelTime.QuadPart);  
	UpdateDelta(&cpu_user_delta, cpu_totals.UserTime.QuadPart);
	UpdateDelta(&cpu_idle_delta, cpu_totals.IdleTime.QuadPart);

	if (getUsage)
	{
		total_time = cpu_kernel_delta.Delta + cpu_user_delta.Delta + cpu_idle_delta.Delta;
		sys_time = cpu_kernel_delta.Delta + cpu_user_delta.Delta;
		if (total_time)
			m_cpuTotalUsage = (sys_time)*100.0 / total_time;
		else
			m_cpuTotalUsage = 0.0;
		//
		cout << "CPU usage: " << m_cpuTotalUsage << endl;
		for (auto i : mv_cores)
			cout << i.cpuName << "  " << i.rate << endl;
	}
	return 0;
}

int CCpuInfo::getCPUUsage()
{
	int nCores = getCPUCores();
	CPU_Time* cpu_cores = new CPU_Time[nCores];
	for (unsigned int i = 0; i < nCores; i++)
	{
		sprintf_s(cpu_cores[i].szName,"CPU %d", i);
	}
	mv_cores.clear();
	getCPUTime(0, cpu_cores);
	Sleep(1000);
	getCPUTime(1, cpu_cores); // 第二次获取时，更新的delta即是差值，即各项花费的CPU时间

	delete[] cpu_cores;
	return 0;
}

long CCpuInfo::getBasicFreq()
{
	int start, over;
	_asm
	{
		RDTSC
		mov start, eax
	}
	Sleep(50);
	_asm
	{
		RDTSC
		mov over, eax
	}
	return (over - start) / 50000;
}

int CCpuInfo::getCPUCores()
{
	static SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	mn_cpuCores = (int)sys_info.dwNumberOfProcessors;

	return mn_cpuCores;
}


}