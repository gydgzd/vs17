#include "NetInfo.h"
#include <atlconv.h>    // for T2A ,USES_CONVERSION
namespace V2Vnms
{


CNetInfo::CNetInfo()
{

}

CNetInfo::~CNetInfo()
{
	
}

void CNetInfo::Append(CDevInfo *pDev)
{
	
}

void CNetInfo::Erase(CDevInfo *pDev)
{
	
}

int CNetInfo::GetDevInfo()
{
	getAdapterInfo();
//	for(int i = 0; i < 10;i++)
		getIOSpeed();
	return 0;
}
/*
use GetAdaptersAddresses - Iphlpapi.lib and iphlpapi.h is needed (Minimum supporte Windows 2000) 
ref:https://docs.microsoft.com/zh-cn/windows/desktop/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
GetAdaptersInfo can be alternative(https://docs.microsoft.com/zh-cn/windows/desktop/api/iphlpapi/nf-iphlpapi-getadaptersinfo)
*/
int CNetInfo::getAdapterInfo()
{
	mv_adapters.clear();
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	unsigned int i = 0;
	setlocale(LC_ALL, "");  // print Chinese
	// Set the flags to pass to GetAdaptersAddresses
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
	// default to unspecified address family (both)
	ULONG family = AF_UNSPEC;
	LPVOID lpMsgBuf = NULL;

	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	ULONG outBufLen = 0;
	ULONG Iterations = 0;

	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
	IP_ADAPTER_PREFIX *pPrefix = NULL;

	family = AF_INET;    // AF_INET  AF_INET6  AF_UNSPEC
	// Allocate a 15 KB buffer to start with.
	outBufLen = WORKING_BUFFER_SIZE;
	do {
		pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
		if (pAddresses == NULL) {
			printf("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
			exit(1);
		}

		dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
		if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
			FREE(pAddresses);
			pAddresses = NULL;
		}
		else {
			break;    // run until succeed one time within MAX_TRIES
		}
		Iterations++;
	} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));
	// If successful, output some information from the data we received
	if (dwRetVal == NO_ERROR) {
		
		pCurrAddresses = pAddresses;
		while (pCurrAddresses) 
		{	
			if (pCurrAddresses->IfType ==IF_TYPE_SOFTWARE_LOOPBACK || pCurrAddresses->OperStatus != 1)   // 状态，1 up, 2 down, 3 testing, 4 unknown等
			{
				pCurrAddresses = pCurrAddresses->Next;
				continue;
			}

			NetAdapter tmpAdapter;
			
		//	printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);
		//	printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);
		//	printf("\tDescription: %wS\n", pCurrAddresses->Description);			
			tmpAdapter.FriendlyName = pCurrAddresses->FriendlyName;
			tmpAdapter.describe = pCurrAddresses->Description;

			pUnicast = pCurrAddresses->FirstUnicastAddress;    // 获取单播地址 (同理可获取多播地址)
			//Unicast IP
			if (pUnicast != NULL) {
				for (i = 0; pUnicast != NULL; i++)
				{
					char ipAddr[64] = "";
					if (AF_INET == pUnicast->Address.lpSockaddr->sa_family)// IPV4 地址，使用 IPV4 转换
						inet_ntop(PF_INET, &((sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr, ipAddr, sizeof(ipAddr));
					else if (AF_INET6 == pUnicast->Address.lpSockaddr->sa_family)// IPV6 地址，使用 IPV6 转换
						inet_ntop(PF_INET6, &((sockaddr_in6*)pUnicast->Address.lpSockaddr)->sin6_addr, ipAddr, sizeof(ipAddr));
					pUnicast->OnLinkPrefixLength;
			//		printf("\tUnicast Addresses: %d  %s \n", i, ipAddr);
					
					NetAddress tmpAddr;
					tmpAddr.sa_family = pUnicast->Address.lpSockaddr->sa_family;
					tmpAddr.ip = ipAddr;
					tmpAddr.prefix = pUnicast->OnLinkPrefixLength;
				//	tmpAddr.netMask = prefixToMask(pUnicast->OnLinkPrefixLength).c_str();
				//	printf("\tMASK: %s\n", prefixToMask(pUnicast->OnLinkPrefixLength).c_str());
					tmpAdapter.address.push_back(tmpAddr); 
	
					pUnicast = pUnicast->Next;
				}
			}
			// DNS 
		/*	pDnServer = pCurrAddresses->FirstDnsServerAddress;
			if (pDnServer) {
				for (i = 0; pDnServer != NULL; i++)
				{
					char ip[64] = "";
					if (AF_INET == pDnServer->Address.lpSockaddr->sa_family)// IPV4 地址，使用 IPV4 转换
						inet_ntop(PF_INET, &((sockaddr_in*)pDnServer->Address.lpSockaddr)->sin_addr, ip, sizeof(ip));
					else if (AF_INET6 == pDnServer->Address.lpSockaddr->sa_family)// IPV6 地址，使用 IPV6 转换
						inet_ntop(PF_INET6, &((sockaddr_in6*)pDnServer->Address.lpSockaddr)->sin6_addr, ip, sizeof(ip));

					printf("\tNumber of DNS Server Addresses: %d %s\n", i, ip);
					pDnServer = pDnServer->Next;
				}
			}*/
			// MAC
			char tmpMAC[64] = "";
			if (pCurrAddresses->PhysicalAddressLength != 0) {
				for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;	i++) {
					if (i == (pCurrAddresses->PhysicalAddressLength - 1))
						sprintf_s(tmpMAC, "%s%.2X", tmpMAC, (int)pCurrAddresses->PhysicalAddress[i]);	
					else
						sprintf_s(tmpMAC, "%s%.2X:", tmpMAC, (int)pCurrAddresses->PhysicalAddress[i]);
				}
				tmpAdapter.MAC = tmpMAC;
		//		printf("\tPhysical address: %s \n", tmpMAC);
			}
			tmpAdapter.MTU = pCurrAddresses->Mtu;

		//	printf("\tIfType: %ld\n", pCurrAddresses->IfType);     // 类型 https://docs.microsoft.com/zh-cn/windows/desktop/api/iptypes/ns-iptypes-_ip_adapter_addresses_lh
		//	printf("\tIpv6IfIndex (IPv6 interface): %u\n", pCurrAddresses->Ipv6IfIndex); // IPv6 不可用时是0
		//	printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);  // 发送bit带宽
		//	printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);    // 接收bit带宽
		//	printf("\n");
			mv_adapters.push_back(tmpAdapter);
			pCurrAddresses = pCurrAddresses->Next;
		}
	}
	else {
		printf("Call to GetAdaptersAddresses failed with error: %d\n",
			dwRetVal);
		if (dwRetVal == ERROR_NO_DATA)
			printf("\tNo addresses were found for the requested parameters\n");
		else {

			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL)) {
				USES_CONVERSION;
				printf("\tError: %s", T2A((LPTSTR)lpMsgBuf));
				LocalFree(lpMsgBuf);
				if (pAddresses)
					FREE(pAddresses);
				return 1;
			}
		}
	}
	if (pAddresses) {
		FREE(pAddresses);
	}
	return 0; 
}

int CNetInfo::getIOSpeed()
{
	NetFlow *myNetFlow = new NetFlow[mv_adapters.size()];
	int nMAC = 0;                       // records myNetFlow point to
	getIOStatistic(0, myNetFlow, nMAC); // getIOStatistic may get different number of adapters with mv_adapters
	Sleep(1000);
	getIOStatistic(1, myNetFlow, nMAC);
	setlocale(LC_ALL, "");  // print Chinese
	// 根据MAC地址把之前获得的基本信息和速率匹配
	printf("FriendlyName\t\t\tMAC\t\t    InSpeed(Kbit/s)\tOutSpeed(Kbit/s)\tIP\n");
	for (auto adapters : mv_adapters)
	{
		for (unsigned int n = 0; n < mv_adapters.size(); n++)
			if (adapters.MAC == myNetFlow[n].MAC)
			{
				adapters.recvSpeed = (myNetFlow[n].InOctets.Delta * 8)/KBYTES;
				adapters.transmitSpeed = (myNetFlow[n].OutOctets.Delta * 8)/KBYTES;
				break;
			}
		// 
		printf("%30S %12s\t", adapters.FriendlyName.c_str(), adapters.MAC.c_str());
		printf("%12llu\t\t%12llu\t%s\n", adapters.recvSpeed, adapters.transmitSpeed, adapters.address[0].ip.c_str());//, adapters.address[1].ip.c_str()
	}
	delete[] myNetFlow;
	return 0;
}
/*
use GetIfTable2, GetIfTable use 32bit to storage the data, will get error in 10Gbit/s
GetIfTable2 will get rows with duplicate mac ,need to make a filter
*/
int CNetInfo::getIOStatistic(int isCalc, NetFlow * myNetFlow, int& nMAC)
{
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	MIB_IF_TABLE2 *pIfTable2;	
	MIB_IF_ROW2 *pIfRow;

	// Allocate memory for our pointer.
	pIfTable2 = (MIB_IF_TABLE2 *)MALLOC(sizeof(MIB_IF_TABLE2));
	if (pIfTable2 == NULL) {
		printf("Error allocating memory needed to call GetIfTable2\n");
		return 1;
	}
	// Make an initial call to GetIfTable to get the necessary size into dwSize
	dwSize = sizeof(MIB_IF_TABLE2);
	if (GetIfTable2(&pIfTable2) != NO_ERROR) {
		printf("GetIfTable2 failed first with error: %d\n", dwRetVal);
		if (pIfTable2 != NULL)
		{
			FREE(pIfTable2);
			pIfTable2 = NULL;
		}
		return -1;
	}
	// Make a second call to GetIfTable to get the actual data we want.
	if ((dwRetVal = GetIfTable2(&pIfTable2)) != NO_ERROR) {
		printf("GetIfTable2 failed second with error: %d\n", dwRetVal);
		if (pIfTable2 != NULL) 
		{	
			FREE(pIfTable2);
			pIfTable2 = NULL;
		}
		return -2;
	}
	unsigned int i, j;

	for (i = 0; i < pIfTable2->NumEntries; i++) 
	{
		pIfRow = (MIB_IF_ROW2 *)& pIfTable2->Table[i];
		// OperStatus 参考https://docs.microsoft.com/zh-cn/windows/desktop/api/ifmib/ns-ifmib-_mib_ifrow
		if (pIfRow->OperStatus != IfOperStatusUp || pIfRow->AdminStatus == MIB_IF_ADMIN_STATUS_DOWN || pIfRow->PhysicalAddressLength == 0)
			continue; 
		// get MAC
		char tmpMAC[64] = "";
		for (j = 0; j < pIfRow->PhysicalAddressLength; j++) 
		{
			if (j == (pIfRow->PhysicalAddressLength - 1))
				sprintf_s(tmpMAC, "%s%.2X", tmpMAC, (int)pIfRow->PhysicalAddress[j]);
			else
				sprintf_s(tmpMAC, "%s%.2X:", tmpMAC, (int)pIfRow->PhysicalAddress[j]);
		}
		std::string strMac = tmpMAC;
		// make a filter
		int found_dup = 0;
		unsigned int n = 0;
		for (n = 0; n < nMAC; n++)
			if (strMac == myNetFlow[n].MAC)
			{
				found_dup = 1; 
				break;
			}
		if (0 == isCalc)
		{
			if (found_dup == 1)
				continue;
		}
		if (1 == isCalc)
		{
			if (myNetFlow[n].Updated == 2)
				continue;
		}
		myNetFlow[n].MAC = strMac;
		// in
		UpdateDelta(&myNetFlow[n].InOctets,pIfRow->InOctets);
		UpdateDelta(&myNetFlow[n].InUcastOctets, pIfRow->InUcastOctets);
		UpdateDelta(&myNetFlow[n].InUcastPkts, pIfRow->InUcastPkts);
		UpdateDelta(&myNetFlow[n].InNUcastPkts, pIfRow->InNUcastPkts);
		UpdateDelta(&myNetFlow[n].InDiscards, pIfRow->InDiscards);
		UpdateDelta(&myNetFlow[n].InErrors, pIfRow->InErrors);

		UpdateDelta(&myNetFlow[n].OutOctets, pIfRow->OutOctets);
		UpdateDelta(&myNetFlow[n].OutUcastOctets, pIfRow->OutUcastOctets);
		UpdateDelta(&myNetFlow[n].OutUcastPkts, pIfRow->OutUcastPkts);
		UpdateDelta(&myNetFlow[n].OutNUcastPkts, pIfRow->OutNUcastPkts);
		UpdateDelta(&myNetFlow[n].OutDiscards, pIfRow->OutDiscards);
		UpdateDelta(&myNetFlow[n].OutErrors, pIfRow->OutErrors);
		myNetFlow[n].Updated++;
		if (0 == isCalc) // in first call, count number od record  
		{
			nMAC++;
			if (nMAC == mv_adapters.size())
				break;
		}
	
/*		printf("\tInOctets[%d]:\t %llu\t", i, pIfRow->InOctets);        // in octets
		printf("\tInOctets[%d]:\t %llu\t", i, pIfRow->InUcastOctets);   //
		printf("\tInOctets[%d]:\t %llu\t", i, pIfRow->InUcastPkts);     // unicast packets
		printf("\tInOctets[%d]:\t %llu\t", i, pIfRow->InNUcastPkts);    // non-unicast packets, Broadcast and multicast packets are included.
		printf("\tInOctets[%d]:\t %llu\t", i, pIfRow->InDiscards);      // incoming packets discarded ,even though they did not have errors.
		printf("\tInOctets[%d]:\t %llu\t", i, pIfRow->InErrors);        // incoming packets discarded, because of errors.
//		printf("\tInOctets[%d]:\t %llu\n", i, pIfRow->InUnknownProtos); // incoming packets discarded, because the protocol was unknown
		// out
		printf("\tOutOctets[%d]:\t %llu\n", i, pIfRow->OutOctets);    
		printf("\tOutOctets[%d]:\t %llu\n", i, pIfRow->OutUcastOctets);
		printf("\tOutOctets[%d]:\t %llu\n", i, pIfRow->OutUcastPkts); 
		printf("\tOutOctets[%d]:\t %llu\n", i, pIfRow->OutNUcastPkts);
		printf("\tOutOctets[%d]:\t %llu\n", i, pIfRow->OutDiscards);
		printf("\tOutOctets[%d]:\t %llu\n", i, pIfRow->OutErrors);*/
	}
	if (pIfTable2 != NULL) {
		FREE(pIfTable2);
		pIfTable2 = NULL;
	}
	return 0;
}
// convert prefix into a dotted decimal mask(IPv4)
std::string CNetInfo::prefixToMask(int prefix)
{
	std::string netmask = "";
	char tmp[8] = "";
	int i = 0;
	for (i = 0; i < prefix / 8; i++)
	{
		netmask += "255";
		if(i != prefix / 8 -1 && 0 != (prefix % 8))
			netmask += ".";
	}
	if (0 != (prefix % 8))
	{
		int n = pow(2, 8 - (prefix % 8));
		sprintf_s(tmp, ".%d", 256 - n);
		netmask += std::string(tmp);
		i++;
	}
	while(i++ < 4 )	
			netmask += ".0";
	return netmask;
}

std::string CNetInfo::getAdapterType(int nType)
{
	std::string strType ;
	switch (nType) {
	case IF_TYPE_OTHER:
		strType = "Other\n";
		break;
	case IF_TYPE_ETHERNET_CSMACD:
		strType = "Ethernet\n";
		break;
	case IF_TYPE_ISO88025_TOKENRING:
		strType = "Token Ring\n";
		break;
	case IF_TYPE_PPP:
		strType = "PPP\n";
		break;
	case IF_TYPE_SOFTWARE_LOOPBACK:
		strType = "Software Lookback\n";
		break;
	case IF_TYPE_ATM:
		strType = "ATM\n";
		break;
	case IF_TYPE_IEEE80211:
		strType = "IEEE 802.11 Wireless\n";
		break;
	case IF_TYPE_TUNNEL:
		strType = "Tunnel type encapsulation\n";
		break;
	case IF_TYPE_IEEE1394:
		strType = "IEEE 1394 Firewire\n";
		break;
	default:
		strType = "Unknown type";
		break;
	}
	return strType;
}

}