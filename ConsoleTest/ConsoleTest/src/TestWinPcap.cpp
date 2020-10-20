#include "stdafx.h"
#include "TestWinPcap.h"

TestWinPcap::TestWinPcap()
{
}


TestWinPcap::~TestWinPcap()
{
}
int TestWinPcap::MyPcap_init()
{
	return 0;
}
/*
in:  char *if_name
out: char *mac
*/
int TestWinPcap::MyPcap_getMac(char *if_name, char *mac)
{
	if (if_name == NULL)
	{
		printf("if_name is NULL.\n");
		return -1;
	}

	LPADAPTER	lpAdapter = 0;
//	DWORD		dwErrorCode;
	PPACKET_OID_DATA  OidData;
	int         status;
	lpAdapter = PacketOpenAdapter(if_name);
	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
	{
		printError_Win("Unable to open the adapter");
		return -1;
	}

	// Allocate a buffer to get the MAC adress

	OidData = (PPACKET_OID_DATA)malloc(6 + sizeof(PACKET_OID_DATA));
	if (OidData == NULL)
	{
		printf("error allocating memory!\n");
		PacketCloseAdapter(lpAdapter);
		return -1;
	}

	// Retrieve the adapter MAC querying the NIC driver

	OidData->Oid = OID_802_3_CURRENT_ADDRESS;
	OidData->Length = 6;
	ZeroMemory(OidData->Data, 6);

	status = PacketRequest(lpAdapter, FALSE, OidData);
	if (status)
	{
	/*	printf("The MAC address of the adapter is %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
			(OidData->Data)[0],
			(OidData->Data)[1],
			(OidData->Data)[2],
			(OidData->Data)[3],
			(OidData->Data)[4],
			(OidData->Data)[5]);
			*/
		sprintf(mac, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", 
			(OidData->Data)[0],
			(OidData->Data)[1],
			(OidData->Data)[2],
			(OidData->Data)[3],
			(OidData->Data)[4],
			(OidData->Data)[5]);

	}
	else
	{
		printf("error retrieving the MAC address of the adapter!\n");
	}

	free(OidData);
	PacketCloseAdapter(lpAdapter);
	return (0);
}
/*生成数据包*/
void gen_packet(unsigned char *buff, int len)
{
	int i = 0;

	//设置目标MAC地址7c:11:cd:06:71:00

	buff[0] = 0x7c;
	buff[1] = 0x11;
	buff[2] = 0xcd;
	buff[3] = 0x06;
	buff[4] = 0x71;
	buff[5] = 0x00;


	//设置源MAC地址
	buff[6] = 0xac;
	buff[7] = 0xb5;
	buff[8] = 0x7d;
	buff[9] = 0x2c;
	buff[10] = 0x31;
	buff[11] = 0x72;

	//设置协议标识为0xc0xd，无任何实际意义
	buff[12] = 0xc;
	buff[13] = 0xd;

	//填充数据包的内容
	for (i = 14; i < len; i++)
	{
		buff[i] = i - 14;
	}
}

int TestWinPcap::process()
{
	pcap_if_t *alldevs;
	pcap_if_t *dev;
	int inum;
	int i = 0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	int ret = -1;

	/* 获取本机网络设备列表 */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)  // add  "rpcap://" before name
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	char mac[32] = "";
	/* 打印网络设备列表 */
	for (dev = alldevs; dev; dev = dev->next)
	{
		// remove "rpcap://" before name
		strcpy(dev->name, dev->name + strlen("rpcap://"));
		if (MyPcap_getMac(dev->name, mac) < 0)    // 
		{
			printf("[virtualcon::pcap_open_adapter] get mac failed!\n");//获取当前设备的mac地址	
		}
		printf("%d. %s %s", ++i, dev->name, mac);
		if (dev->description)
			printf(" (%s)\n", dev->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}

	/*选择网络设备接口*/
	printf("Enter the interface number (1-%d):", i);
	scanf("%d", &inum);

	if (inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* 释放设备列表 */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* 跳转到选中的适配器 */
	for (dev = alldevs, i = 0; i < inum - 1; dev = dev->next, i++);

	/* 打开设备 */
	if ((adhandle = pcap_open(dev->name,          // 设备名
		65536,            // 65535保证能捕获到数据链路层上每个数据包的全部内容
		PCAP_OPENFLAG_PROMISCUOUS,    // 混杂模式
		1000,             // 读取超时时间
		NULL,             // 远程机器验证
		errbuf            // 错误缓冲池
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", dev->name);
		/* 释放设备列表 */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/*在选中的设备接口上发送数据*/
	printf("\nsending on %s...\n", dev->description);

	/* 发送数据包*/
	//生成数据报
	int packetlen = 100;
	unsigned char *buf = (unsigned char *)malloc(packetlen);
	memset(buf, 0x0, packetlen);
	gen_packet(buf, packetlen); //获得生成的数据包，长度为packetlen
	//开始数据包发送
	if ((ret = pcap_sendpacket(adhandle, buf, packetlen)) == -1)
	{
		printf("发送失败\n");
		free(buf);
		pcap_close(adhandle);
		pcap_freealldevs(alldevs);
		return -1;
	}

	/*释放资源*/
	free(buf);
	pcap_close(adhandle);
	pcap_freealldevs(alldevs);

	return 0;
}

void TestWinPcap::printError_Win(char * msg)
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
	do { *p-- = 0; } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

	// Display the message
	USES_CONVERSION;
	printf("ERROR: %s failed with error %d - %s", msg, eNum, T2A((LPCTSTR)lpMsgBuf));
	// LOG(ERROR) << msg << " failed with error " << eNum << " - " << CDevInfo::AnsiToUtf8((LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}