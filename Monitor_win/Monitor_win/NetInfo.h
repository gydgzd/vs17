#pragma once
#include <winsock2.h>    // 应该放在最前，以免与 winsock.h 冲突
#include <math.h>
#include "DevInfo.h"
#include <netioapi.h>    // GetIfTable2 
#include <iphlpapi.h>
#include <Ws2tcpip.h>       // inet_ntop
#pragma comment( lib, "Ws2_32.lib" )  // inet_ntop
#pragma comment(lib, "IPHLPAPI.lib")
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

namespace V2Vnms
{
 struct NetFlow {
	 // in
	 int          Updated = 0;
	 std::string  MAC;
	 UINT64_DELTA InOctets;
	 UINT64_DELTA InUcastOctets;
//	 UINT64_DELTA InMulticastOctets;
//	 UINT64_DELTA InBroadcastOctets;
	 UINT64_DELTA InUcastPkts;
	 UINT64_DELTA InNUcastPkts;
	 UINT64_DELTA InDiscards;   
	 UINT64_DELTA InErrors;     
	 // out
	 UINT64_DELTA OutOctets;
	 UINT64_DELTA OutUcastOctets;
//	 UINT64_DELTA OutMulticastOctets;
//	 UINT64_DELTA OutBroadcastOctets;
	 UINT64_DELTA OutUcastPkts;
	 UINT64_DELTA OutNUcastPkts;
	 UINT64_DELTA OutDiscards;
	 UINT64_DELTA OutErrors;
 }; // 参考https://docs.microsoft.com/zh-cn/windows/desktop/api/netioapi/ns-netioapi-_mib_if_row2
struct NetAddress
{
	int prefix;
	int sa_family;
	std::string ip;
//	std::string netMask;
	std::string gateway;
};
struct NetAdapter {
	int          MTU;
	std::wstring name;	          // interface name       
	std::wstring FriendlyName;
	std::wstring describe;        // 
	std::string MAC;
	ULONG64 transmitSpeed = 0;    // Kb/s
	ULONG64 recvSpeed = 0;        // Kb/s
	std::vector<NetAddress> address;
};


class CNetInfo : public CDevInfo
{
public:
	CNetInfo();
	virtual ~CNetInfo();
	virtual void Append(CDevInfo *pDev);
	virtual void Erase(CDevInfo *pDev);

	virtual int GetDevInfo();
private:
	int __cdecl getAdapterInfo( );        // get basic information
	int getIOSpeed();
	int getIOStatistic(int isCalc, NetFlow *myNetFlow, int& nMAC);
	std::string prefixToMask(int prefix); // convert prefix into a dotted decimal mask(IPv4)
	std::string getAdapterType(int nType);
private:
	std::vector<NetAdapter> mv_adapters;  //

};
}

