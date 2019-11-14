#include "stdafx.h"
#include "TestWinPcap.h"


TestWinPcap::TestWinPcap()
{
}


TestWinPcap::~TestWinPcap()
{
}
/*�������ݰ�*/
void gen_packet(unsigned char *buf, int len)
{
	int i = 0;

	//����Ŀ��MAC��ַΪ01:01:01:01:01:01
	for (i = 0; i < 6; i++)
	{
		buf[i] = 0x01;
	}
	//����ԴMAC��ַΪ02:02:02:02:02:02
	for (i = 6; i < 12; i++)
	{
		buf[i] = 0x02;
	}

	//����Э���ʶΪ0xc0xd�����κ�ʵ������
	buf[12] = 0xc;
	buf[13] = 0xd;

	//������ݰ�������
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

	/* ��ȡ���������豸�б� */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* ��ӡ�����豸�б� */
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
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

	/* ���豸 */
	if ((adhandle = pcap_open(d->name,          // �豸��
		65536,            // 65535��֤�ܲ���������·����ÿ�����ݰ���ȫ������
		PCAP_OPENFLAG_PROMISCUOUS,    // ����ģʽ
		1000,             // ��ȡ��ʱʱ��
		NULL,             // Զ�̻�����֤
		errbuf            // ���󻺳��
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* �ͷ��豸�б� */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/*��ѡ�е��豸�ӿ��Ϸ�������*/
	printf("\nsending on %s...\n", d->description);

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
