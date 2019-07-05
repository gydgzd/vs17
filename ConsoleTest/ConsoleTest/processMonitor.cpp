#include "stdafx.h"
#include "processMonitor.h"

//https://blog.csdn.net/nicolas16/article/details/1587323
//getProcessList - https://docs.microsoft.com/zh-cn/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
#include <TCHAR.H>
#include <iostream>
using namespace std;

extern void getProcess();

processMonitor::processMonitor()
{
}


processMonitor::~processMonitor()
{
}

int processMonitor::getProcess_Win()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	OSVERSIONINFO osvi;//定义OSVERSIONINFO数据结构对象
	memset(&osvi, 0, sizeof(OSVERSIONINFO));//开空间 
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);//定义大小 
	GetVersionEx(&osvi);//获得版本信息 
	





	getProcess();
	return 0;
}

int processMonitor::getProcessList_Win()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError_Win(_T("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		printError_Win(_T("Process32First"));  // Show cause of failure
		CloseHandle(hProcessSnap);     // Must clean up the snapshot object!
		return(FALSE);
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		printf("\n\n=====================================================");
		wprintf(_T("\nPROCESS NAME:  %s"), pe32.szExeFile);
		printf("\n-----------------------------------------------------");

		// Retrieve the priority class.
		dwPriorityClass = 0;
		/*
		The size of the PROCESS_ALL_ACCESS flag increased on Windows Server 2008 and Windows Vista. 
		If an application compiled for Windows Server 2008 and Windows Vista is run on Windows Server 2003 or Windows XP, 
		the PROCESS_ALL_ACCESS flag is too large and the function specifying this flag fails with ERROR_ACCESS_DENIED.
		To avoid this problem, specify the minimum set of access rights required for the operation. 
		If PROCESS_ALL_ACCESS must be used, set _WIN32_WINNT to the minimum operating system targeted by your application 
		(for example, #define _WIN32_WINNT _WIN32_WINNT_WINXP). For more information
		*/
	//	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			printError_Win(_T("OpenProcess"));
		else
		{
			dwPriorityClass = GetPriorityClass(hProcess);
			if (!dwPriorityClass)
				printError_Win(_T("GetPriorityClass"));
			CloseHandle(hProcess);
		}

		printf("\n  process ID        = 0x%08X", pe32.th32ProcessID);
		printf("\n  thread count      = %d",     pe32.cntThreads);
		printf("\n  parent process ID = 0x%08X", pe32.th32ParentProcessID);
		printf("\n  Priority Base     = %d",     pe32.pcPriClassBase);
		if (dwPriorityClass)
			printf("\n  Priority Class    = %d", dwPriorityClass);

		// List the modules and threads associated with this process
	//	listProcessModules_Win(pe32.th32ProcessID);
	//	listProcessThreads_Win(pe32.th32ProcessID);

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(TRUE);
}


int processMonitor::listProcessModules_Win(unsigned long dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		printError_Win(_T("CreateToolhelp32Snapshot (of modules)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{
		printError_Win(_T("Module32First"));  // Show cause of failure
		CloseHandle(hModuleSnap);     // Must clean up the snapshot object!
		return(FALSE);
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		wprintf(_T("\n\n     MODULE NAME:     %s"), me32.szModule);
		wprintf(_T("\n     executable     = %s"), me32.szExePath);
		printf("\n     process ID     = 0x%08X", me32.th32ProcessID);
		printf("\n     ref count (g)  =     0x%04X", me32.GlblcntUsage);
		printf("\n     ref count (p)  =     0x%04X", me32.ProccntUsage);
		printf("\n     base address   = 0x%08X", (unsigned long)me32.modBaseAddr);
		printf("\n     base size      = %d", me32.modBaseSize);

	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
	return(TRUE);
}

int processMonitor::listProcessThreads_Win(unsigned long dwOwnerPID)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if (!Thread32First(hThreadSnap, &te32))
	{
		printError_Win(_T("Thread32First"));  // Show cause of failure
		CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
		return(FALSE);
	}

	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			printf("\n\n     THREAD ID      = 0x%08X", te32.th32ThreadID);
			printf("\n     base priority  = %d", te32.tpBasePri);
			printf("\n     delta priority = %d", te32.tpDeltaPri);
		}
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return(TRUE);
}

void processMonitor::printError_Win(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256] = _T("");
	TCHAR* p;
	LPVOID lpMsgBuf;
	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		0, //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	USES_CONVERSION;
	printf( "\n  ERROR: %s failed with error %d (%s)", msg, eNum, T2A((LPCTSTR)lpMsgBuf));
	LocalFree(lpMsgBuf);
}




////////////////////////////////////////

#include <iostream>
#include <string>

#pragma comment(lib,"ntdll.lib")
using namespace std;
#define printseg(x) cout<<"\t"###x##":"<<x<<endl

typedef HANDLE HLOCAL;

extern "C"
{
	KSERVICE_TABLE_DESCRIPTOR* KeServiceDescriptorTableShadow;
	NTSTATUS __stdcall NtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, OUT PVOID SystemInformation, IN ULONG SystemInformationLength, OUT PULONG ReturnLength OPTIONAL);
	HLOCAL __stdcall LocalAlloc(IN UINT uFlags, SIZE_T uBytes);
	LPVOID __stdcall LocalLock(IN HLOCAL hMem);
	HLOCAL __stdcall LocalFree(IN HLOCAL hMem);
}

template<typename systeminfo>
void GetInformationTemplate(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, IN void(*GetCallBack)(systeminfo* si))
{
	ULONG retlen = 2;
	NTSTATUS status = STATUS_SUCCESS;
	HLOCAL hMem = NULL;

	status = NtQuerySystemInformation(SystemInformationClass, &status, sizeof(status), &retlen);
	switch (status)
	{
	case STATUS_INVALID_INFO_CLASS:
		cout << "INVALID_INFO_CLASS" << endl;
		return;
	case STATUS_ACCESS_VIOLATION:
		cout << "ACCESS_VIOLATION" << endl;
		return;
	case STATUS_INSUFFICIENT_RESOURCES:
		cout << "INSUFFICIENT_RESOURCES" << endl;
		return;
	case STATUS_WORKING_SET_QUOTA:
		cout << "WORKING_SET_QUOTA" << endl;
		return;
	case STATUS_NOT_IMPLEMENTED:
		cout << "NOT_IMPLEMENTED" << endl;
		return;
	case STATUS_INFO_LENGTH_MISMATCH:
		do
		{
			hMem = LocalAlloc(0, retlen);
			if (hMem)
			{
				systeminfo* si = (systeminfo*)LocalLock(hMem);
				if (si)
				{
					memset(si, 0, retlen);
					status = NtQuerySystemInformation(SystemInformationClass, si, retlen, &retlen);
					if (NT_SUCCESS(status))
					{
						GetCallBack(si);
					}
				}
				LocalFree(hMem);
			}
		} while (status == STATUS_INFO_LENGTH_MISMATCH);
		return;
	case STATUS_SUCCESS:
		break;
	default:
		cout << "UNKNOWN ERROR" << endl;
		return;
	}

	if (retlen < sizeof(systeminfo))
		retlen = sizeof(systeminfo);
	cout << "structsize:" << sizeof(systeminfo) << " realsize:" << retlen << endl;
	hMem = LocalAlloc(0, retlen);
	if (hMem)
	{
		systeminfo* si = (systeminfo*)LocalLock(hMem);
		if (si)
		{
			memset(si, 0, retlen);
			status = NtQuerySystemInformation(SystemInformationClass, si, retlen, &retlen);
			if (NT_SUCCESS(status))
			{
				GetCallBack(si);
			}
			else
			{
				cout << "ERROR" << endl;
			}
		}
		LocalFree(hMem);
	}
}

void GetSystemBasicInformation(SYSTEM_BASIC_INFORMATION* psbi)//0
{
	cout << "\t\t0 SystemBasicInformation" << endl;
	cout << "\t时间解析度ms:" << psbi->TimerResolution << endl;
	cout << "\t物理页大小:" << psbi->PageSize << endl;
	cout << "\t物理页个数:" << psbi->NumberOfPhysicalPages << endl;
	cout << "\t最小物理页个数:" << psbi->LowestPhysicalPageNumber << endl;
	cout << "\t最大物理页个数:" << psbi->HighestPhysicalPageNumber << endl;
	cout << "\t逻辑页大小:" << psbi->AllocationGranularity << endl;
	cout << "\t最小用户地址:" << psbi->MinimumUserModeAddress << endl;
	cout << "\t最大用户地址:" << psbi->MaximumUserModeAddress << endl;
	cout << "\t处理器个数:" << (int)psbi->NumberOfProcessors << endl;
}

void GetSystemProcessorInformation(SYSTEM_PROCESSOR_INFORMATION* pspri)//1
{   //      1 SystemProcessorInformation
	cout << "\t\t1 SystemProcessorInformation" << endl;
	switch (pspri->ProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		cout << "\tINTEL ";
		if (pspri->ProcessorLevel == 3)
			cout << "386 ";
		else if (pspri->ProcessorLevel == 4)
			cout << "486 ";
		else if (pspri->ProcessorLevel == 5)
			cout << "586 or Pentium ";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		cout << "\tIA64 ";
		if (pspri->ProcessorLevel == 7)
			cout << "Itanium ";
		else if (pspri->ProcessorLevel == 31)
			cout << "Itanium 2 ";
		break;
	}
	cout << pspri->ProcessorRevision << " " << pspri->ProcessorFeatureBits << endl;
}

void GetSystemPerformanceInformation(SYSTEM_PERFORMANCE_INFORMATION* pspei)//2
{
	cout << "\t\t2 SystemPerformanceInformation" << endl;
	cout << "\tIdle进程时间:" << pspei->IdleProcessTime.QuadPart << endl;
	cout << "\tIO读字节" << pspei->IoReadTransferCount.QuadPart << endl;
	cout << "\tIO写字节" << pspei->IoWriteTransferCount.QuadPart << endl;
	cout << "\tIO其他字节" << pspei->IoOtherTransferCount.QuadPart << endl;
	cout << "\tIO读次数" << pspei->IoReadOperationCount << endl;
	cout << "\tIO写次数" << pspei->IoWriteOperationCount << endl;
	cout << "\tIO其他次数" << pspei->IoOtherOperationCount << endl;
	cout << "\t未用页" << pspei->AvailablePages << endl;
	cout << "\t已用页" << pspei->CommittedPages << endl;
	cout << "\t最多使用页" << pspei->CommitLimit << endl;
	cout << "\t已用页峰值" << pspei->PeakCommitment << endl;
	cout << "\t页错误数" << pspei->PageFaultCount << endl;
	cout << "\tCopyOnWrite数" << pspei->CopyOnWriteCount << endl;
	printseg(pspei->TransitionCount);
	printseg(pspei->CacheTransitionCount);
	printseg(pspei->DemandZeroCount);
	printseg(pspei->PageReadCount);
	printseg(pspei->PageReadIoCount);
	printseg(pspei->CacheReadCount);
	printseg(pspei->CacheIoCount);
	printseg(pspei->DirtyPagesWriteCount);
	printseg(pspei->DirtyWriteIoCount);
	printseg(pspei->MappedPagesWriteCount);
	printseg(pspei->MappedWriteIoCount);
	printseg(pspei->PagedPoolPages);
	printseg(pspei->NonPagedPoolPages);
	printseg(pspei->PagedPoolAllocs);
	printseg(pspei->PagedPoolFrees);
	printseg(pspei->NonPagedPoolAllocs);
	printseg(pspei->NonPagedPoolFrees);
	printseg(pspei->FreeSystemPtes);
	printseg(pspei->ResidentSystemCodePage);
	printseg(pspei->TotalSystemDriverPages);
	printseg(pspei->TotalSystemCodePages);
	printseg(pspei->NonPagedPoolLookasideHits);
	printseg(pspei->PagedPoolLookasideHits);
	printseg(pspei->AvailablePagedPoolPages);
	printseg(pspei->ResidentSystemCachePage);
	printseg(pspei->ResidentPagedPoolPage);
	printseg(pspei->ResidentSystemDriverPage);
	printseg(pspei->CcFastReadNoWait);
	printseg(pspei->CcFastReadWait);
	printseg(pspei->CcFastReadResourceMiss);
	printseg(pspei->CcFastReadNotPossible);
	printseg(pspei->CcFastMdlReadNoWait);
	printseg(pspei->CcFastMdlReadWait);
	printseg(pspei->CcFastMdlReadResourceMiss);
	printseg(pspei->CcFastMdlReadNotPossible);
	printseg(pspei->CcMapDataNoWait);
	printseg(pspei->CcMapDataWait);
	printseg(pspei->CcMapDataNoWaitMiss);
	printseg(pspei->CcMapDataWaitMiss);
	printseg(pspei->CcPinMappedDataCount);
	printseg(pspei->CcPinReadNoWait);
	printseg(pspei->CcPinReadWait);
	printseg(pspei->CcPinReadNoWaitMiss);
	printseg(pspei->CcPinReadWaitMiss);
	printseg(pspei->CcCopyReadNoWait);
	printseg(pspei->CcCopyReadWait);
	printseg(pspei->CcCopyReadNoWaitMiss);
	printseg(pspei->CcCopyReadWaitMiss);
	printseg(pspei->CcMdlReadNoWait);
	printseg(pspei->CcMdlReadWait);
	printseg(pspei->CcMdlReadNoWaitMiss);
	printseg(pspei->CcMdlReadWaitMiss);
	printseg(pspei->CcReadAheadIos);
	printseg(pspei->CcLazyWriteIos);
	printseg(pspei->CcLazyWritePages);
	printseg(pspei->CcDataFlushes);
	printseg(pspei->CcDataPages);
	printseg(pspei->ContextSwitches);
	printseg(pspei->FirstLevelTbFills);
	printseg(pspei->SecondLevelTbFills);
	printseg(pspei->SystemCalls);
}

void GetSystemTimeOfDayInformation(SYSTEM_TIMEOFDAY_INFORMATION* psti)//3
{
	cout << "\t\t3 SystemTimeOfDayInformation" << endl;
	cout << "\t启动时间:" << psti->BootTime.QuadPart << endl;
	cout << "\t当前时间:" << psti->CurrentTime.QuadPart << endl;
	printseg(psti->TimeZoneBias.QuadPart);
	printseg(psti->TimeZoneId);
	printseg(psti->BootTimeBias);
	printseg(psti->SleepTimeBias);
}

// void GetSystemPathInformation(SYSTEM_PATH_INFORMATION* pspi)//4
// {
// }

void GetSystemProcessInformation(SYSTEM_PROCESS_INFORMATION* pspri1)//5
{
	cout << "\t\t5 SystemProcessInformation" << endl;
	do
	{
		if (pspri1->ImageName.Buffer)
			wcout << "\tImageName:" << wstring((wchar_t*)pspri1->ImageName.Buffer) << endl;
		else
			wcout << "\tno name" << endl;
		cout << "\t线程数:" << pspri1->NumberOfThreads << endl;
		printseg(pspri1->SpareLi1.QuadPart);
		printseg(pspri1->SpareLi2.QuadPart);
		printseg(pspri1->SpareLi3.QuadPart);
		cout << "\t创建时间:" << pspri1->CreateTime.QuadPart << endl;
		cout << "\t用户态时间:" << pspri1->UserTime.QuadPart << endl;
		cout << "\t内核态时间:" << pspri1->KernelTime.QuadPart << endl;
		cout << "\t基础优先级:" << pspri1->BasePriority << endl;
		cout << "\t进程Id:" << (int)pspri1->UniqueProcessId << endl;
		cout << "\t父进程Id:" << (int)pspri1->InheritedFromUniqueProcessId << endl;
		cout << "\t句柄数:" << pspri1->HandleCount << endl;
		cout << "\t会话Id:" << pspri1->SessionId << endl;
		cout << "\t页目录机制:" << pspri1->PageDirectoryBase << endl;
		cout << "\t虚拟内存峰值:" << pspri1->PeakVirtualSize << endl;
		cout << "\t虚拟内存大小:" << pspri1->VirtualSize << endl;
		cout << "\t页错误数:" << pspri1->PageFaultCount << endl;
		cout << "\t物理内存峰值:" << pspri1->PeakWorkingSetSize << endl;
		cout << "\t物理内存大小:" << pspri1->WorkingSetSize << endl;
		cout << "\t分页池配额峰值:" << pspri1->QuotaPeakPagedPoolUsage << endl;
		cout << "\t分页池配额:" << pspri1->QuotaPagedPoolUsage << endl;
		cout << "\t非分页池配额峰值:" << pspri1->QuotaPeakNonPagedPoolUsage << endl;
		cout << "\t非分页池配额:" << pspri1->QuotaNonPagedPoolUsage << endl;
		cout << "\t页面文件使用:" << pspri1->PagefileUsage << endl;
		cout << "\t页面文件使用峰值:" << pspri1->PeakPagefileUsage << endl;
		cout << "\t私有页面数:" << pspri1->PrivatePageCount << endl;
		cout << "\t读操作数:" << pspri1->ReadOperationCount.QuadPart << endl;
		cout << "\t写操作数:" << pspri1->WriteOperationCount.QuadPart << endl;
		cout << "\t其他操作数:" << pspri1->OtherOperationCount.QuadPart << endl;
		cout << "\t读字节数:" << pspri1->ReadTransferCount.QuadPart << endl;
		cout << "\t写字节数:" << pspri1->WriteTransferCount.QuadPart << endl;
		cout << "\t其他字节数:" << pspri1->OtherTransferCount.QuadPart << endl;
		SYSTEM_PROCESS_INFORMATION* newpspri1 = (SYSTEM_PROCESS_INFORMATION*)((BYTE*)pspri1 + pspri1->NextEntryOffset);
		SYSTEM_THREAD_INFORMATION* pesti = (SYSTEM_THREAD_INFORMATION*)(pspri1 + 1);
		int threadindex = 0;
		while ((LPVOID)pesti < (LPVOID)newpspri1)
		{
			++threadindex;
			cout << "\t内核态时间:" << pesti->KernelTime.QuadPart << endl;
			cout << "\t用户态时间:" << pesti->UserTime.QuadPart << endl;
			cout << "\t创建时间:" << pesti->CreateTime.QuadPart << endl;
			cout << "\t等待时间:" << pesti->WaitTime << endl;
			cout << "\t起始地址:" << hex << pesti->StartAddress << endl;
			cout << "\tUniqueProcess:" << hex << pesti->ClientId.UniqueProcess << endl;
			cout << "\tUniqueThread:" << hex << pesti->ClientId.UniqueThread << endl;
			cout << dec;
			cout << "\t优先级:" << pesti->Priority << endl;
			cout << "\t基础优先级:" << pesti->BasePriority << endl;
			cout << "\t模式切换次数:" << pesti->ContextSwitches << endl;
			cout << "\t线程状态:" << pesti->ThreadState << endl;
			cout << "\t等待原因:" << pesti->WaitReason << endl;
			cout << dec;
			pesti++;
		}
		pspri1 = newpspri1;
	} while (pspri1->NextEntryOffset);
}

void GetSystemCallCountInformation(SYSTEM_CALL_COUNT_INFORMATION* pscci)//6
{
	cout << "\t\t6 SystemCallCountInformation" << endl;
	printseg(pscci->Length);
	printseg(pscci->NumberOfTables);
	ULONG* limits = (ULONG*)((BYTE*)pscci + sizeof(SYSTEM_CALL_COUNT_INFORMATION));
	ULONG* tables = (ULONG*)((BYTE*)pscci + sizeof(SYSTEM_CALL_COUNT_INFORMATION) + pscci->NumberOfTables * sizeof(PULONG));
	int index = 0;
	for (int i = 0; i < pscci->NumberOfTables; i++)
	{
		for (int j = 0; j < limits[i]; j++)
		{
			cout << "tablecount" << index << ":" << tables[index] << endl;
			++index;
		}
	}
}

void GetSystemDeviceInformation(SYSTEM_DEVICE_INFORMATION* psdi)//7
{
	cout << "\t\t7 SystemDeviceInformation" << endl;
	cout << "\t磁盘数:" << psdi->NumberOfDisks << endl;
	cout << "\t软盘数:" << psdi->NumberOfFloppies << endl;
	cout << "\t光驱数:" << psdi->NumberOfCdRoms << endl;
	cout << "\t磁带数:" << psdi->NumberOfTapes << endl;
	cout << "\t串行端口数:" << psdi->NumberOfSerialPorts << endl;
	cout << "\t并行端口数:" << psdi->NumberOfParallelPorts << endl;
}

void GetSystemProcessorPerformanceInformation(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* psppei)//8
{
	cout << "\t\t8 SystemProcessorPerformanceInformation" << endl;
	cout << "\t空闲时间:" << psppei->IdleTime.QuadPart << endl;
	cout << "\t内核态时间:" << psppei->KernelTime.QuadPart << endl;
	cout << "\t用户态时间:" << psppei->UserTime.QuadPart << endl;
	cout << "\tDPC时间:" << psppei->DpcTime.QuadPart << endl;
	cout << "\t中断时间:" << psppei->InterruptTime.QuadPart << endl;
	cout << "\t中断次数:" << psppei->InterruptCount << endl;
}

void GetSystemFlagsInformation(SYSTEM_FLAGS_INFORMATION* psfi)//9
{
	cout << "\t\t9 SystemFlagsInformation" << endl;
	if (psfi->Flags&FLG_STOP_ON_EXCEPTION)
		cout << "FLG_STOP_ON_EXCEPTION" << endl;
	if (psfi->Flags&FLG_SHOW_LDR_SNAPS)
		cout << "FLG_SHOW_LDR_SNAPS" << endl;
	if (psfi->Flags&FLG_DEBUG_INITIAL_COMMAND)
		cout << "FLG_DEBUG_INITIAL_COMMAND" << endl;
	if (psfi->Flags&FLG_STOP_ON_HUNG_GUI)
		cout << "FLG_STOP_ON_HUNG_GUI" << endl;

	if (psfi->Flags&FLG_HEAP_ENABLE_TAIL_CHECK)
		cout << "FLG_HEAP_ENABLE_TAIL_CHECK" << endl;
	if (psfi->Flags&FLG_HEAP_ENABLE_FREE_CHECK)
		cout << "FLG_HEAP_ENABLE_FREE_CHECK" << endl;
	if (psfi->Flags&FLG_HEAP_VALIDATE_PARAMETERS)
		cout << "FLG_HEAP_VALIDATE_PARAMETERS" << endl;
	if (psfi->Flags&FLG_HEAP_VALIDATE_ALL)
		cout << "FLG_HEAP_VALIDATE_ALL" << endl;

	if (psfi->Flags&FLG_APPLICATION_VERIFIER)
		cout << "FLG_APPLICATION_VERIFIER" << endl;
	if (psfi->Flags&FLG_POOL_ENABLE_TAGGING)
		cout << "FLG_POOL_ENABLE_TAGGING" << endl;
	if (psfi->Flags&FLG_HEAP_ENABLE_TAGGING)
		cout << "FLG_HEAP_ENABLE_TAGGING" << endl;

	if (psfi->Flags&FLG_USER_STACK_TRACE_DB)
		cout << "FLG_USER_STACK_TRACE_DB" << endl;
	if (psfi->Flags&FLG_KERNEL_STACK_TRACE_DB)
		cout << "FLG_KERNEL_STACK_TRACE_DB" << endl;
	if (psfi->Flags&FLG_MAINTAIN_OBJECT_TYPELIST)
		cout << "FLG_MAINTAIN_OBJECT_TYPELIST" << endl;
	if (psfi->Flags&FLG_HEAP_ENABLE_TAG_BY_DLL)
		cout << "FLG_HEAP_ENABLE_TAG_BY_DLL" << endl;

	if (psfi->Flags&FLG_DISABLE_STACK_EXTENSION)
		cout << "FLG_DISABLE_STACK_EXTENSION" << endl;
	if (psfi->Flags&FLG_ENABLE_CSRDEBUG)
		cout << "FLG_ENABLE_CSRDEBUG" << endl;
	if (psfi->Flags&FLG_ENABLE_KDEBUG_SYMBOL_LOAD)
		cout << "FLG_ENABLE_KDEBUG_SYMBOL_LOAD" << endl;
	if (psfi->Flags&FLG_DISABLE_PAGE_KERNEL_STACKS)
		cout << "FLG_DISABLE_PAGE_KERNEL_STACKS" << endl;

	if (psfi->Flags&FLG_ENABLE_SYSTEM_CRIT_BREAKS)
		cout << "FLG_ENABLE_SYSTEM_CRIT_BREAKS" << endl;
	if (psfi->Flags&FLG_HEAP_DISABLE_COALESCING)
		cout << "FLG_HEAP_DISABLE_COALESCING" << endl;
	if (psfi->Flags&FLG_ENABLE_CLOSE_EXCEPTIONS)
		cout << "FLG_ENABLE_CLOSE_EXCEPTIONS" << endl;
	if (psfi->Flags&FLG_ENABLE_EXCEPTION_LOGGING)
		cout << "FLG_ENABLE_EXCEPTION_LOGGING" << endl;

	if (psfi->Flags&FLG_ENABLE_HANDLE_TYPE_TAGGING)
		cout << "FLG_ENABLE_HANDLE_TYPE_TAGGING" << endl;
	if (psfi->Flags&FLG_HEAP_PAGE_ALLOCS)
		cout << "FLG_HEAP_PAGE_ALLOCS" << endl;
	if (psfi->Flags&FLG_DEBUG_INITIAL_COMMAND_EX)
		cout << "FLG_DEBUG_INITIAL_COMMAND_EX" << endl;
	if (psfi->Flags&FLG_DISABLE_DBGPRINT)
		cout << "FLG_DISABLE_DBGPRINT" << endl;

	if (psfi->Flags&FLG_CRITSEC_EVENT_CREATION)
		cout << "FLG_CRITSEC_EVENT_CREATION" << endl;
	if (psfi->Flags&FLG_LDR_TOP_DOWN)
		cout << "FLG_LDR_TOP_DOWN" << endl;
	if (psfi->Flags&FLG_ENABLE_HANDLE_EXCEPTIONS)
		cout << "FLG_ENABLE_HANDLE_EXCEPTIONS" << endl;
	if (psfi->Flags&FLG_DISABLE_PROTDLLS)
		cout << "FLG_DISABLE_PROTDLLS" << endl;
}

// void GetSystemCallTimeInformation(SYSTEM_CALLTIME_INFORMATION* pscti)
// {
// }

void GetSystemModuleInformation(RTL_PROCESS_MODULES* prpm)//11
{
	cout << "\t\t11 SystemModuleInformation" << endl;
	for (int i = 0; i < prpm->NumberOfModules; i++)
	{
		cout << "module" << i << " FullPathName:" << (char*)prpm->Modules[i].FullPathName << endl;
		cout << "\tSection:" << hex << prpm->Modules[i].Section << endl;
		cout << "\tMappedBase:" << hex << prpm->Modules[i].MappedBase << endl;
		cout << "\tImageBase:" << hex << prpm->Modules[i].ImageBase << endl;
		cout << "\tImageSize:" << hex << prpm->Modules[i].ImageSize << endl;
		cout << "\tFlags:" << hex << prpm->Modules[i].Flags << endl;
		cout << dec;
		cout << "\tLoadOrderIndex:" << (int)prpm->Modules[i].LoadOrderIndex << endl;
		cout << "\tInitOrderIndex:" << (int)prpm->Modules[i].InitOrderIndex << endl;
		cout << "\tLoadCount:" << (int)prpm->Modules[i].LoadCount << endl;
		cout << "\tOffsetToFile:" << prpm->Modules[i].OffsetToFileName << endl;//距离文件名偏移(取最后一个\之后的部分)   
	}
}

void GetSystemLocksInformation(RTL_PROCESS_LOCKS* prpl)//12
{
	cout << "\t\t12 SystemLocksInformation" << endl;
	for (int i = 0; i < prpl->NumberOfLocks; i++)
	{
		cout << "\tlock" << i << ":" << endl;
		cout << "\tAddress:" << hex << prpl->Locks[i].Address << endl;
		cout << "\tOwningThread:" << hex << prpl->Locks[i].OwningThread << endl;
		cout << dec;
		cout << "\tType:" << (int)prpl->Locks[i].Type << endl;
		cout << "\tCreatorBackTraceIndex:" << (int)prpl->Locks[i].CreatorBackTraceIndex << endl;
		cout << "\tLockCount:" << prpl->Locks[i].LockCount << endl;
		cout << "\tContentionCount:" << prpl->Locks[i].ContentionCount << endl;
		cout << "\tEntryCount:" << prpl->Locks[i].EntryCount << endl;
		cout << "\tRecursionCount:" << prpl->Locks[i].RecursionCount << endl;
		cout << "\tNumberOfWaitingShared:" << prpl->Locks[i].NumberOfWaitingShared << endl;
		cout << "\tNumberOfWaitingExclusive:" << prpl->Locks[i].NumberOfWaitingExclusive << endl;
	}
}

void GetSystemStackTraceInformation(RTL_PROCESS_BACKTRACES* prpb)//13
{
	cout << "\t\t13 SystemStackTraceInformation" << endl;
	cout << "\tCommittedMemory:" << prpb->CommittedMemory << endl;
	cout << "\tReservedMemory:" << prpb->ReservedMemory << endl;
	cout << "\tNumberOfBackTraceLookups:" << prpb->NumberOfBackTraceLookups << endl;
	for (int i = 0; i < prpb->NumberOfBackTraces; i++)
	{
		cout << "\tTraceCount:" << prpb->BackTraces[i].TraceCount << endl;
		cout << "\tIndex:" << prpb->BackTraces[i].Index << endl;
		cout << "\tDepth:" << prpb->BackTraces[i].Depth << endl;
		for (int j = 0; j < MAX_STACK_DEPTH; j++)
		{
			cout << "\t" << hex << prpb->BackTraces[i].BackTrace[i] << endl;
		}
	}
}

// void GetSystemPagedPoolInformation(SYSTEM_POOL_INFORMATION* pspi)//14
// {
// }

// void GetSystemNonPagedPoolInformation(SYSTEM_POOL_INFORMATION* pspi)//15
// {
// }

void GetSystemHandleInformation(SYSTEM_HANDLE_INFORMATION* pshi)//16
{
	cout << "\t\t16 SystemHandleInformation" << endl;
	for (int i = 0; i < pshi->NumberOfHandles; i++)
	{
		cout << "\t" << i + 1 << endl;
		cout << "\tUniqueProcessId:" << (int)pshi->Handles[i].UniqueProcessId << endl;
		cout << "\tCreatorBackTraceIndex:" << (int)pshi->Handles[i].CreatorBackTraceIndex << endl;
		cout << "\tObjectTypeIndex:" << (int)pshi->Handles[i].ObjectTypeIndex << endl;
		cout << "\tHandleAttributes:" << (int)pshi->Handles[i].HandleAttributes << endl;
		cout << "\tHandleValue:" << (int)pshi->Handles[i].HandleValue << endl;
		cout << "\tObject:" << hex << (int)pshi->Handles[i].Object << endl;
		cout << "\tGrantedAccess:" << hex << (int)pshi->Handles[i].GrantedAccess << endl;
	}
}

void GetSystemObjectInformation(SYSTEM_OBJECTTYPE_INFORMATION* pstoi)//17
{
	cout << "\t\t17 SystemObjectInformation" << endl;
	int nextpos = pstoi->NextEntryOffset;
	SYSTEM_OBJECT_INFORMATION* curpsoi = NULL;
	do
	{
		cout << "\tNumberOfObjects:" << pstoi->NumberOfObjects << endl;
		cout << "\tNumberOfHandles:" << pstoi->NumberOfHandles << endl;
		cout << "\tTypeIndex:" << pstoi->TypeIndex << endl;
		cout << "\tInvalidAttributes:" << hex << pstoi->InvalidAttributes << endl;
		cout << "\tValidAccessMask:" << hex << pstoi->ValidAccessMask << endl;
		cout << dec;
		cout << "\tPoolType:" << pstoi->PoolType << endl;
		cout << "\tSecurityRequired:" << pstoi->SecurityRequired << endl;
		cout << "\tWaitableObject:" << pstoi->WaitableObject << endl;
		wcout << "\tTypeName:" << (wchar_t*)pstoi->TypeName.Buffer << endl;
		curpsoi = (SYSTEM_OBJECT_INFORMATION*)((PWSTR)(pstoi + 1) + pstoi->TypeName.MaximumLength);
		cout << "\tObject:" << hex << curpsoi->Object << endl;
		cout << "\tCreatorUniqueProcess:" << hex << curpsoi->CreatorUniqueProcess << endl;
		cout << "\tCreatorBackTraceIndex:" << hex << curpsoi->CreatorBackTraceIndex << endl;
		cout << "\tFlags" << hex << curpsoi->Flags << endl;
		cout << "\tExclusiveProcessId:" << hex << curpsoi->ExclusiveProcessId << endl;
		cout << "\tSecurityDescriptor:" << hex << curpsoi->SecurityDescriptor << endl;
		cout << dec;
		cout << "\tPointerCount:" << curpsoi->PointerCount << endl;
		cout << "\tHandleCount:" << curpsoi->HandleCount << endl;
		cout << "\tPagedPoolCharge:" << curpsoi->PagedPoolCharge << endl;
		cout << "\tNonPagedPoolCharge:" << curpsoi->NonPagedPoolCharge << endl;
		wcout << "\tNameInfo:" << (wchar_t*)curpsoi->NameInfo.Name.Buffer << endl;
		pstoi = (SYSTEM_OBJECTTYPE_INFORMATION*)((BYTE*)pstoi + pstoi->NextEntryOffset);
	} while (nextpos);
}

void GetSystemPageFileInformation(SYSTEM_PAGEFILE_INFORMATION* pspi)//18
{
	cout << "\t\t18 SystemPageFileInformation" << endl;
	ULONG nextoffset = 0;
	int index = 0;
	do
	{
		cout << "PAGEFILE" << index + 1 << ":" << endl;
		nextoffset = pspi->NextEntryOffset;
		cout << "\tTotalSize:" << pspi->TotalSize << endl;
		cout << "\tTotalInUse:" << pspi->TotalInUse << endl;
		cout << "\tPeakUsage:" << pspi->PeakUsage << endl;
		cout << "\tPageFileName:" << (wchar_t*)pspi->PageFileName.Buffer << endl;
		pspi = (SYSTEM_PAGEFILE_INFORMATION*)((BYTE*)pspi + pspi->NextEntryOffset);
	} while (nextoffset);
}

void GetSystemVdmInstemulInformation(SYSTEM_VDM_INSTEMUL_INFO* psvii)//19
{
	cout << "\t\t19 SystemVdmInstemulInformation" << endl;
	printseg(psvii->SegmentNotPresent);
	printseg(psvii->VdmOpcode0F);
	printseg(psvii->OpcodeESPrefix);
	printseg(psvii->OpcodeCSPrefix);
	printseg(psvii->OpcodeSSPrefix);
	printseg(psvii->OpcodeDSPrefix);
	printseg(psvii->OpcodeFSPrefix);
	printseg(psvii->OpcodeGSPrefix);
	printseg(psvii->OpcodeOPER32Prefix);
	printseg(psvii->OpcodeADDR32Prefix);
	printseg(psvii->OpcodeINSB);
	printseg(psvii->OpcodeINSW);
	printseg(psvii->OpcodeOUTSB);
	printseg(psvii->OpcodeOUTSW);
	printseg(psvii->OpcodePUSHF);
	printseg(psvii->OpcodePOPF);
	printseg(psvii->OpcodeINTnn);
	printseg(psvii->OpcodeINTO);
	printseg(psvii->OpcodeIRET);
	printseg(psvii->OpcodeINBimm);
	printseg(psvii->OpcodeINWimm);
	printseg(psvii->OpcodeOUTBimm);
	printseg(psvii->OpcodeOUTWimm);
	printseg(psvii->OpcodeINB);
	printseg(psvii->OpcodeINW);
	printseg(psvii->OpcodeOUTB);
	printseg(psvii->OpcodeOUTW);
	printseg(psvii->OpcodeLOCKPrefix);
	printseg(psvii->OpcodeREPPrefix);
	printseg(psvii->OpcodeREPPrefix);
	printseg(psvii->OpcodeHLT);
	printseg(psvii->OpcodeCLI);
	printseg(psvii->OpcodeSTI);
	printseg(psvii->BopCount);
}

// void GetSystemVdmBopInformation(SYSTEM_VDM_BOP_INFO* psvbi)//20
// {
// 
// }

void GetSystemFileCacheInformation(SYSTEM_FILECACHE_INFORMATION* psfci)//21
{
	cout << "\t\t21 SystemFileCacheInformation" << endl;
	printseg(psfci->CurrentSize);
	printseg(psfci->PeakSize);
	printseg(psfci->PageFaultCount);
	printseg(psfci->MinimumWorkingSet);
	printseg(psfci->MaximumWorkingSet);
	printseg(psfci->CurrentSizeIncludingTransitionInPages);
	printseg(psfci->PeakSizeIncludingTransitionInPages);
	printseg(psfci->TransitionRePurposeCount);
	printseg(psfci->Flags);
}

void GetSystemPoolTagInformation(SYSTEM_POOLTAG_INFORMATION* pspti)//22
{
	cout << "\t\t22 SystemPoolTagInformation" << endl;
	for (int i = 0; i < pspti->Count; i++)
	{
		cout << pspti->TagInfo[i].TagUlong << endl;
		cout << pspti->TagInfo[i].PagedAllocs << endl;
		cout << pspti->TagInfo[i].PagedFrees << endl;
		cout << pspti->TagInfo[i].PagedUsed << endl;
		cout << pspti->TagInfo[i].NonPagedAllocs << endl;
		cout << pspti->TagInfo[i].NonPagedFrees << endl;
		cout << pspti->TagInfo[i].NonPagedUsed << endl;
	}
}

void GetSystemInterruptInformation(SYSTEM_INTERRUPT_INFORMATION* psii)//23
{
	cout << "\t\t23 SystemInterruptInformation" << endl;
	printseg(psii->ContextSwitches);
	printseg(psii->DpcCount);
	printseg(psii->DpcRate);
	printseg(psii->TimeIncrement);
	printseg(psii->DpcBypassCount);
	printseg(psii->ApcBypassCount);
}

void GetSystemDpcBehaviorInformation(SYSTEM_DPC_BEHAVIOR_INFORMATION* psdbi)//24
{
	cout << "\t\t24 SystemDpcBehaviorInformation" << endl;
	printseg(psdbi->Spare);
	printseg(psdbi->DpcQueueDepth);
	printseg(psdbi->MinimumDpcRate);
	printseg(psdbi->AdjustDpcThreshold);
	printseg(psdbi->IdealDpcRate);
}

void GetSystemFullMemoryInformation(SYSTEM_MEMORY_INFORMATION* psmi)//25
{
	cout << "\t\t25 SystemFullMemoryInformation" << endl;
	printseg(psmi->StringStart);
	for (int i = 0; i < psmi->InfoSize; i++)
	{
		cout << (char*)psmi->Memory[i].StringOffset << endl;
		printseg((int)psmi->Memory[i].ValidCount);
		printseg((int)psmi->Memory[i].TransitionCount);
		printseg((int)psmi->Memory[i].ModifiedCount);
		printseg((int)psmi->Memory[i].PageTableCount);
	}
}

void GetSystemLoadGdiDriverInformation(SYSTEM_GDI_DRIVER_INFORMATION* psgdi)//26
{
	cout << "\t\t26 SystemLoadGdiDriverInformation" << endl;
	wcout << psgdi->DriverName.Buffer << endl;
	cout << "\tImageLength:" << psgdi->ImageLength << endl;
	cout << "\tImageAddress:" << hex << psgdi->ImageAddress << endl;
	cout << "\tSectionPointer:" << hex << psgdi->SectionPointer << endl;
	cout << "\tEntryPoint:" << hex << psgdi->EntryPoint << endl;
}

// void GetSystemUnloadGdiDriverInformation(SYSTEM_UNLOAD_GDIDRIVER_INFORMATION)//27
// {
// 
// }

void GetSystemTimeAdjustmentInformation(SYSTEM_SET_TIME_ADJUST_INFORMATION* psstai)//28
{
	cout << "\t\t28 SystemTimeAdjustmentInformation" << endl;
	cout << "\tTimeAdjustment:" << psstai->TimeAdjustment << endl;
	cout << "\tEnable:" << psstai->Enable << endl;
}

void GetSystemSummaryMemoryInformation(SYSTEM_MEMORY_INFORMATION* psmi)//29
{
	cout << "\t\t29 SystemSummaryMemoryInformation" << endl;
	printseg(psmi->StringStart);
	for (int i = 0; i < psmi->InfoSize; i++)
	{
		cout << (char*)psmi->Memory[i].StringOffset << endl;
		printseg((int)psmi->Memory[i].ValidCount);
		printseg((int)psmi->Memory[i].TransitionCount);
		printseg((int)psmi->Memory[i].ModifiedCount);
		printseg((int)psmi->Memory[i].PageTableCount);
	}
}

// void GetSystemMirrorMemoryInformation(SYSTEM_MEMORY_INFORMATION* psmi)//30
// {
// }

// void GetSystemPerformanceTraceInformation(SYSTEM_PERFORMANCE_TRANCE_INFORMATION* pspti)//31
// {
// }

void GetSystemExceptionInformation(SYSTEM_EXCEPTION_INFORMATION* psei)//32
{
	cout << "\t\t32 SystemExceptionInformation" << endl;
	printseg(psei->AlignmentFixupCount);
	printseg(psei->ExceptionDispatchCount);
	printseg(psei->FloatingEmulationCount);
	printseg(psei->ByteWordEmulationCount);
}

// void GetSystemCrashDumpStateInformation(SYSTEM_CRASHDUMP_STATE_INFORMATION* pscsi)//33
// {
// }

//SystemCrashDumpStateInformation 34
void GetSystemKernelDebuggerInformation(SYSTEM_KERNEL_DEBUGGER_INFORMATION* pskdi)//35
{
	cout << "\t\t35 SystemKernelDebuggerInformation" << endl;
	printseg(pskdi->KernelDebuggerEnabled);
	printseg(pskdi->KernelDebuggerNotPresent);
}

void GetSystemContextSwitchInformation(SYSTEM_CONTEXT_SWITCH_INFORMATION* pscsi)//36
{
	cout << "\t\t36 SystemContextSwitchInformation" << endl;
	printseg(pscsi->ContextSwitches);
	printseg(pscsi->FindAny);
	printseg(pscsi->FindLast);
	printseg(pscsi->FindIdeal);
	printseg(pscsi->IdleAny);
	printseg(pscsi->IdleCurrent);
	printseg(pscsi->IdleLast);
	printseg(pscsi->IdleIdeal);
	printseg(pscsi->PreemptAny);
	printseg(pscsi->PreemptCurrent);
	printseg(pscsi->PreemptLast);
	printseg(pscsi->SwitchToIdle);
}

void GetSystemRegistryQuotaInformation(SYSTEM_REGISTRY_QUOTA_INFORMATION* psrqi)//37
{
	cout << "\t\t37 SystemRegistryQuotaInformation" << endl;
	printseg(psrqi->RegistryQuotaAllowed);
	printseg(psrqi->RegistryQuotaUsed);
	printseg(psrqi->PagedPoolSize);
}

// void GetSystemExtendServiceTableInformation(SYSTEM_EXTEND_SERVICE_TABLE_INFORAMTION* psesti)//38
// {
// }

// void GetSystemPrioritySeperation(SYSTEM_PRIORITY_SEPERATION* psps)// 39
// {
// }

// void GetSystemVerifierAddDriverInformation(SYSTEM_VERIFIER_ADDDRIVER_INFORAMTION* psvai) //40
// {
// }

// void GetSystemVerifierRemoveDriverInformation(SYSTEM_VERIFIER_REMOVE_DRIVER_INFORMATION* psvrdi) //41
// {
// }

void GetSystemProcessorIdleInformation(SYSTEM_PROCESSOR_IDLE_INFORMATION* pspii)//42
{
	cout << "\t\t42 SystemProcessorIdleInformation" << endl;
	printseg(pspii->IdleTime);
	printseg(pspii->C1Time);
	printseg(pspii->C2Time);
	printseg(pspii->C3Time);
	printseg(pspii->C1Transitions);
	printseg(pspii->C2Transitions);
	printseg(pspii->C3Transitions);
	printseg(pspii->Padding);
}

void GetSystemLegacyDriverInformation(SYSTEM_LEGACY_DRIVER_INFORMATION* psldi)
{
	cout << "\t\t43 SystemLegacyDriverInformation" << endl;
	printseg(psldi->VetoType);
	cout << (wchar_t*)psldi->VetoList.Buffer << endl;
}

void GetSystemCurrentTimeZoneInformation(RTL_TIME_ZONE_INFORMATION* prtzi)
{
	cout << "\t\t44 SystemCurrentTimeZoneInformation" << endl;
	printseg(prtzi->Bias);
	cout << "Standard:" << endl;
	wcout << (wchar_t*)prtzi->StandardName << endl;
	cout << "\tYear:" << prtzi->StandardStart.Year << endl;
	cout << "\tMonth:" << prtzi->StandardStart.Month << endl;
	cout << "\tDay:" << prtzi->StandardStart.Day << endl;
	cout << "\tHour:" << prtzi->StandardStart.Hour << endl;
	cout << "\tMinute:" << prtzi->StandardStart.Minute << endl;
	cout << "\tSecond:" << prtzi->StandardStart.Second << endl;
	cout << "\tMilliseconds:" << prtzi->StandardStart.Milliseconds << endl;
	cout << "\tWeekday:" << prtzi->StandardStart.Weekday << endl;
	printseg(prtzi->StandardBias);

	cout << "Daylight:" << endl;
	wcout << (wchar_t*)prtzi->DaylightName << endl;
	cout << "\tYear:" << prtzi->DaylightStart.Year << endl;
	cout << "\tMonth:" << prtzi->DaylightStart.Month << endl;
	cout << "\tDay:" << prtzi->DaylightStart.Day << endl;
	cout << "\tHour:" << prtzi->DaylightStart.Hour << endl;
	cout << "\tMinute:" << prtzi->DaylightStart.Minute << endl;
	cout << "\tSecond:" << prtzi->DaylightStart.Second << endl;
	cout << "\tMilliseconds:" << prtzi->DaylightStart.Milliseconds << endl;
	cout << "\tWeekday:" << prtzi->DaylightStart.Weekday << endl;
	printseg(prtzi->DaylightBias);
}

void GetSystemLookasideInformation(SYSTEM_LOOKASIDE_INFORMATION* psli, int length)
{
	cout << "\t\t45 SystemLookasideInformation" << endl;
	int num = length / sizeof(SYSTEM_LOOKASIDE_INFORMATION);
	for (int i = 0; i < num; i++)
	{
		printseg((int)psli->CurrentDepth);
		printseg((int)psli->MaximumDepth);
		printseg(psli->TotalAllocates);
		printseg(psli->AllocateMisses);
		printseg(psli->TotalFrees);
		printseg(psli->FreeMisses);
		printseg(psli->Type);
		printseg(psli->Tag);
		printseg(psli->Size);
	}
}

// void GetSystemTimeSlipNotification(SYSTEM_TIME_SLIP_NOTIFICATION* pstsn)// 46
// {
// }

void GetSystemSessionCreate(ULONG* SessionId)//SystemSessionCreate 47
{
	cout << "\t\t47 SystemSessionCreate" << endl;
	cout << "SessionId" << endl;
}

// void GetSystemSessionDetach(SYSTEM_SESSION_DETACH* pssd)// 48
// {
// }

//void GetSystemSessionInformation(SYSTEM_SESSION_INFORMATION* pssi)// 49
// {
// }

void GetSystemRangeStartInformation(ULONG_PTR* data)//50
{
	cout << "\t\t50 SystemRangeStartInformation" << endl;
	cout << hex << data << endl;
	cout << dec;
}

void GetSystemVerifierInformation(SYSTEM_VERIFIER_INFORMATION* psvi)// 51
{
	cout << "\t\t51 SystemVerifierInformation" << endl;
	ULONG offset = 0;
	int index = 0;
	do
	{
		offset = psvi->NextEntryOffset;
		cout << index + 1 << ":" << endl;
		printseg(psvi->Level);
		wcout << (wchar_t*)psvi->DriverName.Buffer << endl;
		printseg(psvi->RaiseIrqls);
		printseg(psvi->AcquireSpinLocks);
		printseg(psvi->SynchronizeExecutions);
		printseg(psvi->AllocationsAttempted);
		printseg(psvi->AllocationsSucceeded);
		printseg(psvi->AllocationsSucceededSpecialPool);
		printseg(psvi->AllocationsWithNoTag);
		printseg(psvi->TrimRequests);
		printseg(psvi->Trims);
		printseg(psvi->AllocationsFailed);
		printseg(psvi->AllocationsFailedDeliberately);
		printseg(psvi->Loads);
		printseg(psvi->Unloads);
		printseg(psvi->UnTrackedPool);
		printseg(psvi->CurrentPagedPoolAllocations);
		printseg(psvi->CurrentNonPagedPoolAllocations);
		printseg(psvi->PeakPagedPoolAllocations);
		printseg(psvi->PeakNonPagedPoolAllocations);
		printseg(psvi->PagedPoolUsageInBytes);
		printseg(psvi->NonPagedPoolUsageInBytes);
		printseg(psvi->PeakPagedPoolUsageInBytes);
		printseg(psvi->PeakNonPagedPoolUsageInBytes);
		psvi = (SYSTEM_VERIFIER_INFORMATION*)((BYTE*)psvi + offset);
	} while (offset);
}

// void GetSystemVerifierThunkExtend(SYSTEM_VERIFIER_THUNK_EX* psvie)//52
// {
// }

void GetSystemSessionProcessInformation(SYSTEM_SESSION_PROCESS_INFORMATION* psspi)//53
{
	cout << "\t\t53 SystemSessionProcessInformation" << endl;
	printseg(psspi->SessionId);
	printseg(psspi->SizeOfBuf);
	//  SYSTEM_SESSION_POOLTAG_INFORMATION* psspti = (SYSTEM_SESSION_POOLTAG_INFORMATION*)psspi->Buffer
}

//54 SystemLoadGdiDriverInSystemSpace
void GetSystemNumaProcessorMap(SYSTEM_NUMA_INFORMATION* psni)
{
	cout << "\t\t55 SystemLoadGdiDriverInSystemSpace" << endl;
	printseg(psni->HighestNodeNumber);
	for (int i = 0; i < MAXIMUM_NUMA_NODES; i++)
	{
		cout << "\tActiveProcessorsAffinityMask" << i << psni->ActiveProcessorsAffinityMask[i] << endl;
		cout << "\tAvailableMemory" << i << psni->AvailableMemory[i] << endl;
	}
}

//56 SystemPrefetcherInformation

void GetSystemExtendedProcessInformation(SYSTEM_PROCESS_INFORMATION* pspri1)//57
{
	cout << "\t\t57 SystemExtendedProcessInformation" << endl;
	do
	{
		if (pspri1->ImageName.Buffer)
			wcout << "\tImageName:" << wstring((wchar_t*)pspri1->ImageName.Buffer) << endl;
		else
			wcout << "no name" << endl;
		cout << "\t线程数:" << pspri1->NumberOfThreads << endl;
		printseg(pspri1->SpareLi1.QuadPart);
		printseg(pspri1->SpareLi2.QuadPart);
		printseg(pspri1->SpareLi3.QuadPart);
		cout << "\t创建时间:" << pspri1->CreateTime.QuadPart << endl;
		cout << "\t用户态时间:" << pspri1->UserTime.QuadPart << endl;
		cout << "\t内核态时间:" << pspri1->KernelTime.QuadPart << endl;
		cout << "\t基础优先级:" << pspri1->BasePriority << endl;
		cout << "\t进程Id:" << (int)pspri1->UniqueProcessId << endl;
		cout << "\t父进程Id:" << (int)pspri1->InheritedFromUniqueProcessId << endl;
		cout << "\t句柄数:" << pspri1->HandleCount << endl;
		cout << "\t会话Id:" << pspri1->SessionId << endl;
		cout << "\t页目录机制:" << pspri1->PageDirectoryBase << endl;
		cout << "\t虚拟内存峰值:" << pspri1->PeakVirtualSize << endl;
		cout << "\t虚拟内存大小:" << pspri1->VirtualSize << endl;
		cout << "\t页错误数:" << pspri1->PageFaultCount << endl;
		cout << "\t物理内存峰值:" << pspri1->PeakWorkingSetSize << endl;
		cout << "\t物理内存大小:" << pspri1->WorkingSetSize << endl;
		cout << "\t分页池配额峰值:" << pspri1->QuotaPeakPagedPoolUsage << endl;
		cout << "\t分页池配额:" << pspri1->QuotaPagedPoolUsage << endl;
		cout << "\t非分页池配额峰值:" << pspri1->QuotaPeakNonPagedPoolUsage << endl;
		cout << "\t非分页池配额:" << pspri1->QuotaNonPagedPoolUsage << endl;
		cout << "\t页面文件使用:" << pspri1->PagefileUsage << endl;
		cout << "\t页面文件使用峰值:" << pspri1->PeakPagefileUsage << endl;
		cout << "\t私有页面数:" << pspri1->PrivatePageCount << endl;
		cout << "\t读操作数:" << pspri1->ReadOperationCount.QuadPart << endl;
		cout << "\t写操作数:" << pspri1->WriteOperationCount.QuadPart << endl;
		cout << "\t其他操作数:" << pspri1->OtherOperationCount.QuadPart << endl;
		cout << "\t读字节数:" << pspri1->ReadTransferCount.QuadPart << endl;
		cout << "\t写字节数:" << pspri1->WriteTransferCount.QuadPart << endl;
		cout << "\t其他字节数:" << pspri1->OtherTransferCount.QuadPart << endl;
		SYSTEM_PROCESS_INFORMATION* newpspri1 = (SYSTEM_PROCESS_INFORMATION*)((BYTE*)pspri1 + pspri1->NextEntryOffset);
		SYSTEM_EXTENDED_THREAD_INFORMATION* pesti = (SYSTEM_EXTENDED_THREAD_INFORMATION*)(pspri1 + 1);
		int threadindex = 0;
		while ((LPVOID)pesti < (LPVOID)newpspri1)
		{
			++threadindex;
			cout << "\t内核态时间:" << pesti->ThreadInfo.KernelTime.QuadPart << endl;
			cout << "\t用户态时间:" << pesti->ThreadInfo.UserTime.QuadPart << endl;
			cout << "\t创建时间:" << pesti->ThreadInfo.CreateTime.QuadPart << endl;
			cout << "\t等待时间:" << pesti->ThreadInfo.WaitTime << endl;
			cout << "\t起始地址:" << hex << pesti->ThreadInfo.StartAddress << endl;
			cout << "\tUniqueProcess:" << hex << pesti->ThreadInfo.ClientId.UniqueProcess << endl;
			cout << "\tUniqueThread:" << hex << pesti->ThreadInfo.ClientId.UniqueThread << endl;
			cout << dec;
			cout << "\t优先级:" << pesti->ThreadInfo.Priority << endl;
			cout << "\t基础优先级:" << pesti->ThreadInfo.BasePriority << endl;
			cout << "\t模式切换次数:" << pesti->ThreadInfo.ContextSwitches << endl;
			cout << "\t线程状态:" << pesti->ThreadInfo.ThreadState << endl;
			cout << "\t等待原因:" << pesti->ThreadInfo.WaitReason << endl;
			cout << "\t栈基址:" << hex << pesti->StackBase << endl;
			cout << "\t栈范围:" << hex << pesti->StackLimit << endl;
			cout << "\tWin32StartAddress" << hex << pesti->Win32StartAddress << endl;
			cout << dec;
			pesti++;
		}
		pspri1 = newpspri1;
	} while (pspri1->NextEntryOffset);
}

void GetSystemRecommendedSharedDataAlignment(ULONG* data)//58
{
	cout << "\t\t`58 SystemRecommendedSharedDataAlignment" << endl;
	cout << *data << endl;
}

void GetSystemComPlusPackage(ULONG* data)//59
{
	cout << "\t\t 59 SystemComPlusPackage" << endl;
	cout << *data << endl;
}

void GetSystemNumaAvailableMemory(SYSTEM_NUMA_INFORMATION* psni)//60
{
	cout << "\t\t60 SystemNumaAvailableMemory" << endl;
	printseg(psni->HighestNodeNumber);
	for (int i = 0; i < MAXIMUM_NUMA_NODES; i++)
	{
		cout << "\tActiveProcessorsAffinityMask" << i << psni->ActiveProcessorsAffinityMask[i] << endl;
		cout << "\tAvailableMemory" << i << psni->AvailableMemory[i] << endl;
	}
}

void GetSystemProcessorPowerInformation(SYSTEM_PROCESSOR_POWER_INFORMATION* psppi)//61
{
	cout << "\t\t61 SystemProcessorPowerInformation" << endl;
	printseg((int)psppi->CurrentFrequency);
	printseg((int)psppi->ThermalLimitFrequency);
	printseg((int)psppi->ConstantThrottleFrequency);
	printseg((int)psppi->DegradedThrottleFrequency);
	printseg((int)psppi->LastBusyFrequency);
	printseg((int)psppi->LastC3Frequency);
	printseg((int)psppi->LastAdjustedBusyFrequency);
	printseg((int)psppi->ProcessorMinThrottle);
	printseg((int)psppi->ProcessorMaxThrottle);
	printseg(psppi->NumberOfFrequencies);
	printseg(psppi->PromotionCount);
	printseg(psppi->DemotionCount);
	printseg(psppi->ErrorCount);
	printseg(psppi->RetryCount);
	printseg(psppi->CurrentFrequencyTime);
	printseg(psppi->CurrentProcessorTime);
	printseg(psppi->CurrentProcessorIdleTime);
	printseg(psppi->LastProcessorTime);
	printseg(psppi->LastProcessorIdleTime);
}

void GetSystemEmulationBasicInformation(SYSTEM_BASIC_INFORMATION* psbi)//62
{
	cout << "\t\t62 SystemEmulationBasicInformation" << endl;
	cout << "\t时间解析度ms:" << psbi->TimerResolution << endl;
	cout << "\t物理页大小:" << psbi->PageSize << endl;
	cout << "\t物理页个数:" << psbi->NumberOfPhysicalPages << endl;
	cout << "\t最小物理页个数:" << psbi->LowestPhysicalPageNumber << endl;
	cout << "\t最大物理页个数:" << psbi->HighestPhysicalPageNumber << endl;
	cout << "\t逻辑页大小:" << psbi->AllocationGranularity << endl;
	cout << "\t最小用户地址:" << psbi->MinimumUserModeAddress << endl;
	cout << "\t最大用户地址:" << psbi->MaximumUserModeAddress << endl;
	cout << "\t处理器个数:" << (int)psbi->NumberOfProcessors << endl;
}

void GetSystemEmulationProcessorInformation(SYSTEM_PROCESSOR_INFORMATION* pspri)//63
{
	cout << "\t\t63 SystemEmulationProcessorInformation" << endl;
	switch (pspri->ProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		cout << "\tINTEL ";
		if (pspri->ProcessorLevel == 3)
			cout << "386 ";
		else if (pspri->ProcessorLevel == 4)
			cout << "486 ";
		else if (pspri->ProcessorLevel == 5)
			cout << "586 or Pentium ";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		cout << "IA64 ";
		if (pspri->ProcessorLevel == 7)
			cout << "Itanium ";
		else if (pspri->ProcessorLevel == 31)
			cout << "Itanium 2 ";
		break;
	}
	cout << pspri->ProcessorRevision << " " << pspri->ProcessorFeatureBits << endl;
}

//64 SystemExtendedHandleInformation
//65 SystemLostDelayedWriteInformation
void GetSystemBigPoolInformation(SYSTEM_BIGPOOL_INFORMATION* psbi)//66
{
	cout << "\t\t63 SystemEmulationProcessorInformation" << endl;
	for (int i = 0; i < psbi->Count; i++)
	{
		cout << hex << "\tVirutalAddress:" << psbi->AllocatedInfo[i].VirtualAddress;
		cout << dec;
		cout << "\tSizeInBytes:" << psbi->AllocatedInfo[i].SizeInBytes;
		cout << "\tTagUlong:" << psbi->AllocatedInfo[i].TagUlong << endl;
	}
}

void GetSystemSessionPoolTagInformation(SYSTEM_SESSION_PROCESS_INFORMATION* psspi)//67
{
	cout << "\t\t67 SystemSessionPoolTagInformation" << endl;
	printseg(psspi->SessionId);
	printseg(psspi->SizeOfBuf);
}

void GetSystemSessionMappedViewInformation(SYSTEM_SESSION_MAPPED_VIEW_INFORMATION* pssmvi)//68
{
	cout << "\t\t68 SystemSessionMappedViewInformation" << endl;
	SIZE_T nextoffset = 0;
	do
	{
		pssmvi = (SYSTEM_SESSION_MAPPED_VIEW_INFORMATION*)((BYTE*)pssmvi + (pssmvi->NextEntryOffset));
	} while (nextoffset);
}

//69 SystemHotpatchInformation

void GetSystemObjectSecurityMode(ULONG* data)//70
{
	cout << "\t\t69 SystemObjectSecurityMode" << endl;
	cout << *data << endl;
}

void GetSystemWatchdogTimerHandler(SYSTEM_WATCHDOG_HANDLER_INFORMATION* pswhi)//71
{
}

void GetSystemWatchdogTimerInformation(SYSTEM_WATCHDOG_TIMER_INFORMATION* pswti)//72
{
}

void GetSystemLogicalProcessorInformation(SYSTEM_LOGICAL_PROCESSOR_INFORMATION* pslpi)//73
{
	cout << "\t\t73 SystemLogicalProcessorInformation" << endl;
}

//74 SystemWow64SharedInformation
void GetSystemRegisterFirmwareTableInformationHandler(SYSTEM_FIRMWARE_TABLE_HANDLER* psfth)//75
{
	cout << "\t\t75 SystemRegisterFirmwareTableInformationHandler" << endl;
}

void GetSystemFirmwareTableInformation(SYSTEM_FIRMWARE_TABLE_HANDLER* psfth)//76
{
	cout << "\t\t76 SystemFirmwareTableInformation" << endl;
}

// void GetSystemExtendedHandleInformation(SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX* pshteie)
// {
//  cout << "\t\t64 SystemExtendedHandleInformation" << endl;
//  for (int i = 0; i < pshteie->NumberOfHandles; i++)
//  {
//      cout << "\t" << i + 1 << endl;
//      cout << "\tUniqueProcessId:" << (int)pshteie->Handles[i].UniqueProcessId << endl;
//      cout << "\tCreatorBackTraceIndex:" << (int)pshteie->Handles[i].CreatorBackTraceIndex << endl;
//      cout << "\tObjectTypeIndex:" << (int)pshteie->Handles[i].ObjectTypeIndex << endl;
//      cout << "\tHandleAttributes:" << (int)pshteie->Handles[i].HandleAttributes << endl;
//      cout << "\tHandleValue:" << (int)pshteie->Handles[i].HandleValue << endl;
//      cout << "\tObject:" << hex << (int)pshteie->Handles[i].Object << endl;
//      cout << "\tGrantedAccess:" << hex << (int)pshteie->Handles[i].GrantedAccess << endl;
//  }
// }

//77 SystemModuleInformationEx
//78 SystemVerifierTriageInformation
//79 SystemSuperfetchInformation
//80 SystemMemoryListInformation

void GetSystemFileCacheInformationEx(SYSTEM_FILECACHE_INFORMATION* psfci)//81
{
	cout << "\t\t81 SystemFileCacheInformationEx" << endl;
	printseg(psfci->CurrentSize);
	printseg(psfci->PeakSize);
	printseg(psfci->PageFaultCount);
	printseg(psfci->MinimumWorkingSet);
	printseg(psfci->MaximumWorkingSet);
	printseg(psfci->CurrentSizeIncludingTransitionInPages);
	printseg(psfci->PeakSizeIncludingTransitionInPages);
	printseg(psfci->TransitionRePurposeCount);
	printseg(psfci->Flags);
}

#define do(x) GetInformationTemplate(x,Get##x)
extern int getProc();
void getProcess()
{
	getProc();
	//  do(SystemBasicInformation);//0
	   do(SystemProcessorInformation);//1
	//  do(SystemPerformanceInformation);//2
	//  do(SystemTimeOfDayInformation);//3
	//  do(SystemPathInformation);//4
	  do(SystemProcessInformation);//5
	//  do(SystemCallCountInformation);//6
	//  do(SystemDeviceInformation);//7
	//  do(SystemProcessorPerformanceInformation);//8
	//  do(SystemFlagsInformation);//9
	//  do(SystemCallTimeInformation);//10
	//  do(SystemModuleInformation);//11
	//  do(SystemLocksInformation);//12
	//  do(SystemStackTraceInformation);//13
	//  do(SystemPagedPoolInformation);//14
	//  do(SystemNonPagedPoolInformation);//15
	//  do(SystemHandleInformation);//16
	//  do(SystemObjectInformation);//17
	//  do(SystemPageFileInformation);//18
	//  do(SystemVdmInstemulInformation);//19
	//  do(SystemVdmBopInformation);//20
	//  do(SystemFileCacheInformatio);//21
	//  do(SystemPoolTagInformation);//22
	//  do(SystemInterruptInformation);//23
	//  do(SystemDpcBehaviorInformation);//24
	//  do(SystemFullMemoryInformation);//25
	//  do(SystemLoadGdiDriverInformation);//26
	//  do(SystemUnloadGdiDriverInformation);//27
	//  do(SystemTimeAdjustmentInformation);//28
	//  do(SystemSummaryMemoryInformation);//29
	//  do(SystemMirrorMemoryInformation);//30
	//  do(SystemPerformanceTraceInformation);//31
	//  do(SystemObsolete0);//32
	//  do(SystemExceptionInformation);//33
	//  do(SystemCrashDumpStateInformation);//34
	//  do(SystemKernelDebuggerInformation);//35
	//  do(SystemContextSwitchInformation);//36
	//  do(SystemRegistryQuotaInformation);//37
	//  do(SystemExtendServiceTableInformation);//38
	//  do(SystemPrioritySeperation);//39
	//  do(SystemVerifierAddDriverInformation);//40
	//  do(SystemVerifierRemoveDriverInformation);//41
	//  do(SystemProcessorIdleInformation);//42
	//  do(SystemLegacyDriverInformation);//43
	//  do(SystemCurrentTimeZoneInformation);//44
	//  do(SystemLookasideInformation);//45
	//  do(SystemTimeSlipNotification);//46
	//  do(SystemSessionCreate);//47
	//  do(SystemSessionDetach);//48
	//  do(SystemSessionInformation);//49
	//  do(SystemRangeStartInformation);//50
	//  do(SystemVerifierInformation);//51
	//  do(SystemVerifierThunkExtend);//52
	//  do(SystemSessionProcessInformation);//53
	//  do(SystemLoadGdiDriverInSystemSpace);//54
	//  do(SystemNumaProcessorMap);//55
	//  do(SystemPrefetcherInformation);//56
	//  do(SystemExtendedProcessInformation);//57
	//  do(SystemRecommendedSharedDataAlignment);//58
	//  do(SystemComPlusPackage);//59
	//  do(SystemNumaAvailableMemory);//60
	//  do(SystemProcessorPowerInformation);//61
	//  do(SystemEmulationBasicInformation);//62
	//  do(SystemEmulationProcessorInformation);//63
	//  do(SystemExtendedHandleInformation);//64
	//  do(SystemLostDelayedWriteInformation);//65
	//  do(SystemBigPoolInformation);//66
	//  do(SystemSessionPoolTagInformation);//67
	//  do(SystemSessionMappedViewInformation);//68
	//  do(SystemHotpatchInformation);//69
	//  do(SystemObjectSecurityMode);//70
	//  do(SystemWatchdogTimerHandler);//71
	//  do(SystemWatchdogTimerInformation);//72
	//  do(SystemLogicalProcessorInformation);//73
	//  do(SystemWow64SharedInformation);//74
	//  do(SystemRegisterFirmwareTableInformationHandler);//75
	//  do(SystemFirmwareTableInformation);//76
	//  do(SystemModuleInformationEx);//77
	//  do(SystemBigPoolInformation);//78
	//  do(SystemSessionPoolTagInformation);//79
	//  do(SystemSessionMappedViewInformation);//80
	//  do(SystemHotpatchInformation);//81
	//  do(SystemObjectSecurityMode);//82
	//  do(SystemWatchdogTimerHandler);//82
	//  do(SystemWatchdogTimerInformation);//83
	//  do(SystemLogicalProcessorInformation);//84
	//  do(SystemWow64SharedInformation);//85
	//  do(SystemRegisterFirmwareTableInformationHandler);//86
	//  do(SystemFirmwareTableInformation);//87
	//  do(SystemModuleInformationEx);//88
	//  do(SystemVerifierTriageInformation);//89
	//  do(SystemSuperfetchInformation);//90
	//  do(SystemMemoryListInformation);//91
	//  do(SystemFileCacheInformationEx);//92
}

typedef NTSTATUS(WINAPI *PFUN_NtQuerySystemInformation)(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
	);

int getProc( )
{
	PFUN_NtQuerySystemInformation pFun = NULL;
//	pFun = (PFUN_NtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation");
 
	// 由于没有导出,所以得自己定义函数的原型
//	typedef DWORD(WINAPI* PQUERYSYSTEM)(UINT, PVOID, DWORD, PDWORD);
//	PQUERYSYSTEM NtQuerySystemInformation = NULL;
//	PSYSTEM_PROCESS_INFORMATION pInfo = { 0 };
//	NtQuerySystemInformation = (PQUERYSYSTEM)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
	pFun = (PFUN_NtQuerySystemInformation)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
 
	char szInfo[0x20000] = { 0 };
	ULONG uReturnedLEngth = 0;
	NTSTATUS status = pFun(SystemProcessInformation, szInfo, sizeof(szInfo), &uReturnedLEngth);
	if (status != 0)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0, // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
		);
		USES_CONVERSION;
		//  LOG(ERROR) << std:: string(T2A((LPCTSTR)lpMsgBuf));
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
		LocalFree(lpMsgBuf);
		return 0;
	}

	PSYSTEM_PROCESS_INFORMATION pSystemInformation = (PSYSTEM_PROCESS_INFORMATION)szInfo;
	DWORD dwID = (DWORD)pSystemInformation->UniqueProcessId;
	HANDLE hHandle = NULL;
	PWCHAR pImageName = (PWCHAR)*(DWORD*)((PCHAR)pSystemInformation + 0x3c);
	printf("ProcessID: %d\tprocessName: %ws \n", dwID, pImageName);
	while (true)
	{
		if (pSystemInformation->NextEntryOffset == 0)
			break;

		pSystemInformation = (PSYSTEM_PROCESS_INFORMATION)((PCHAR)pSystemInformation + pSystemInformation->NextEntryOffset);
		dwID = (DWORD)pSystemInformation->UniqueProcessId;
		hHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwID);
		pImageName = (PWCHAR)*(DWORD*)((PCHAR)pSystemInformation + 0x3c);
		printf("ProcessID: %d\tprocessName: %ws \n", dwID, pImageName);
	}
	getchar();
}
