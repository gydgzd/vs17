#include "stdafx.h"
#include "TestWinPcap.h"


TestWinPcap::TestWinPcap()
{
}


TestWinPcap::~TestWinPcap()
{
}
/*生成数据包*/
void gen_packet(unsigned char *buf, int len)
{
	int i = 0;

	//设置目标MAC地址为01:01:01:01:01:01
	for (i = 0; i < 6; i++)
	{
		buf[i] = 0x01;
	}
	//设置源MAC地址为02:02:02:02:02:02
	for (i = 6; i < 12; i++)
	{
		buf[i] = 0x02;
	}

	//设置协议标识为0xc0xd，无任何实际意义
	buf[12] = 0xc;
	buf[13] = 0xd;

	//填充数据包的内容
	for (i = 14; i < len; i++)
	{
		buf[i] = i - 14;
	}
}

int TestWinPcap::process()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i = 0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	int ret = -1;

	/* 获取本机网络设备列表 */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* 打印网络设备列表 */
	for (d = alldevs; d; d = d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
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
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

	/* 打开设备 */
	if ((adhandle = pcap_open(d->name,          // 设备名
		65536,            // 65535保证能捕获到数据链路层上每个数据包的全部内容
		PCAP_OPENFLAG_PROMISCUOUS,    // 混杂模式
		1000,             // 读取超时时间
		NULL,             // 远程机器验证
		errbuf            // 错误缓冲池
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* 释放设备列表 */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/*在选中的设备接口上发送数据*/
	printf("\nsending on %s...\n", d->description);

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
