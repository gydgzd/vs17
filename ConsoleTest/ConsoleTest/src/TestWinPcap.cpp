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
/*�������ݰ�*/
void gen_packet(unsigned char *buff, int len)
{
	int i = 0;

	//����Ŀ��MAC��ַ7c:11:cd:06:71:00

	buff[0] = 0x7c;
	buff[1] = 0x11;
	buff[2] = 0xcd;
	buff[3] = 0x06;
	buff[4] = 0x71;
	buff[5] = 0x00;


	//����ԴMAC��ַ
	buff[6] = 0xac;
	buff[7] = 0xb5;
	buff[8] = 0x7d;
	buff[9] = 0x2c;
	buff[10] = 0x31;
	buff[11] = 0x72;

	//����Э���ʶΪ0xc0xd�����κ�ʵ������
	buff[12] = 0xc;
	buff[13] = 0xd;

	//������ݰ�������
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

	/* ��ȡ���������豸�б� */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)  // add  "rpcap://" before name
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	char mac[32] = "";
	/* ��ӡ�����豸�б� */
	for (dev = alldevs; dev; dev = dev->next)
	{
		// remove "rpcap://" before name
		strcpy(dev->name, dev->name + strlen("rpcap://"));
		if (MyPcap_getMac(dev->name, mac) < 0)    // 
		{
			printf("[virtualcon::pcap_open_adapter] get mac failed!\n");//��ȡ��ǰ�豸��mac��ַ	
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

	/*ѡ�������豸�ӿ�*/
	printf("Enter the interface number (1-%d):", i);
	scanf("%d", &inum);

	if (inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* ��ת��ѡ�е������� */
	for (dev = alldevs, i = 0; i < inum - 1; dev = dev->next, i++);

	/* ���豸 */
	if ((adhandle = pcap_open(dev->name,          // �豸��
		65536,            // 65535��֤�ܲ���������·����ÿ�����ݰ���ȫ������
		PCAP_OPENFLAG_PROMISCUOUS,    // ����ģʽ
		1000,             // ��ȡ��ʱʱ��
		NULL,             // Զ�̻�����֤
		errbuf            // ���󻺳��
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", dev->name);
		/* �ͷ��豸�б� */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/*��ѡ�е��豸�ӿ��Ϸ�������*/
	printf("\nsending on %s...\n", dev->description);

	/* �������ݰ�*/
	//�������ݱ�
	int packetlen = 100;
	unsigned char *buf = (unsigned char *)malloc(packetlen);
	memset(buf, 0x0, packetlen);
	gen_packet(buf, packetlen); //������ɵ����ݰ�������Ϊpacketlen
	//��ʼ���ݰ�����
	if ((ret = pcap_sendpacket(adhandle, buf, packetlen)) == -1)
	{
		printf("����ʧ��\n");
		free(buf);
		pcap_close(adhandle);
		pcap_freealldevs(alldevs);
		return -1;
	}

	/*�ͷ���Դ*/
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