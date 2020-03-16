#pragma once
#include "DevInfo.h"

#include <string.h>
#include <tchar.h>
#define DIV 1024
// Specify the width of the field in which to print the numbers.
// The asterisk in the format specifier "%*I64d" takes an integer
// argument and uses it to pad and right justify the number.
#define WIDTH 7
namespace V2Vnms
{


class CMemoryInfo : public CDevInfo
{
public:
	CMemoryInfo();
	virtual ~CMemoryInfo();

	virtual void Append(CDevInfo *pDev);
	virtual void Erase(CDevInfo *pDev);	
	virtual int GetDevInfo();
private:
	int getMemoryInfo();
	
private:
	ULONG64 m_memTotal;      // 物理内存
	ULONG64	m_memAvail;
	ULONG64 m_memVirtTotal;  // 虚拟内存
	ULONG64	m_memVirtAvail;
	float   m_memRate;       // memory usage
};
}
