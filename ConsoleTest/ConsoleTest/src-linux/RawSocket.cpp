/*
 * RawSocket.cpp
 *
 *  Created on: Jan 3, 2020
 *      Author: gyd
 */

#include "RawSocket.h"

int n = 0;
const char hello[] = "This is my ping.";

//计算校验和
unsigned short checksum(unsigned char *buf,int len)
{
    unsigned int sum=0;
    unsigned short *cbuf;
    cbuf=(unsigned short *)buf;
    while(len>1)
    {
        sum+=*cbuf++;
        len-=2;
    }
    if(len)
        sum+=*(unsigned char *)cbuf;
    sum=(sum>>16)+(sum & 0xffff);
    sum+=(sum>>16);
    return ~sum;
}

RawSocket::RawSocket() {
    // TODO Auto-generated constructor stub
    m_rawSocket = 0;
    memset(m_buff, 0, 4096);
    initSocket();
}

RawSocket::~RawSocket() {
    // TODO Auto-generated destructor stub
}

int RawSocket::initSocket()
{

    if((m_rawSocket = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        printf("socket create error: %d - %s\n", errno, strerror(errno));
    }


    return 0;
}

int RawSocket::sendPkt()
{
    struct iphdr *ip_hdr;   //iphdr为IP头部结构体
    struct icmphdr *icmp_hdr;   //icmphdr为ICMP头部结构体
    int len;
    int i;
    icmp_hdr=(struct icmphdr *)(m_buff);  //字符串指针
    icmp_hdr->type=ICMP_ECHO;    //初始化ICMP消息类型type
    icmp_hdr->code=0;    //初始化消息代码code
    icmp_hdr->un.echo.id=1;   //把进程标识码初始给icmp_id
    icmp_hdr->un.echo.sequence=n++;  //发送的ICMP消息序号赋值给icmp序号
//    gettimeofday((struct timeval *)icmp_hdr->icmp_timestamp,NULL); // 获取当前时间
    icmp_hdr->icmp_timestamp[0] = 0x5E11C643;

    memcpy(icmp_hdr->data, hello, strlen(hello));

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_addr.s_addr =  inet_addr("10.1.24.128");
    dst.sin_family = PF_INET;
    dst.sin_port = ntohs(0);

    len=ICMP_HSIZE+strlen(hello);
    icmp_hdr->checksum=0;    //初始化
    icmp_hdr->checksum=checksum((u8 *)icmp_hdr,len);  //计算校验和
    for(i=0; i < len; i++)
    {
        printf("%02X", (unsigned char)m_buff[i]);
        if((i+1)%8 == 0)
            printf("  ");
    }
    printf("\n");
  //  printf("The send pack checksum is:0x%x\n",icmp_hdr->checksum);
    sendto(m_rawSocket, m_buff, len, 0, (struct sockaddr *)&dst, sizeof (dst)); //经socket传送数据

    return 0;
}
//////////////////////////////////////////
ether_RawSocket::ether_RawSocket() {
    // TODO Auto-generated constructor stub
    m_rawSocket = 0;
    memset(m_buff, 0, 4096);
    initSocket("ens37");
}

ether_RawSocket::~ether_RawSocket() {
    // TODO Auto-generated destructor stub
}

int ether_RawSocket::initSocket(const char *eth_dev)
{
    if((m_rawSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
        printf("socket create error: %d - %s\n", errno, strerror(errno));
    }
    int ret = 0;
    struct ifreq ifr;
    strcpy(ifr.ifr_name, eth_dev);
//  get interface index
    if (ioctl(m_rawSocket, SIOCGIFINDEX, &ifr) == -1)
    {
        printf("fail to get index - %d-%s\n", errno, strerror(errno));
        return -8;
    }
    int index = ifr.ifr_ifindex;
    printf("%s index: %d\n", eth_dev, index);
    struct sockaddr_ll socket_address;
 // 当发送数据包时，指定 sll_family, sll_protocol, sll_ifindex, sll_halen, sll_addr 就足够了,其它字段设置为0；
 // sll_hatype和 sll_pkttype是在接收数据包时使用的；
 // 如果要bind, 只需要使用 sll_protocol和 sll_ifindex；
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    socket_address.sll_ifindex = index;
    socket_address.sll_hatype = ARPHRD_ETHER;
    socket_address.sll_pkttype = PACKET_HOST;
    socket_address.sll_halen = ETH_ALEN;
/*
    socket_address.sll_addr[0] = 0x01;
    socket_address.sll_addr[1] = 0x02;
    socket_address.sll_addr[2] = 0x03;
    socket_address.sll_addr[3] = 0x04;
    socket_address.sll_addr[4] = 0x05;
    socket_address.sll_addr[5] = 0x06;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;
*/
    ret = bind(m_rawSocket, (struct sockaddr*)(&socket_address), sizeof(socket_address));
    if (ret < 0)
    {
        printf("fail to bind socket - %d-%s\n", errno, strerror(errno));
        return -9;
    }

    return 0;
}

int ether_RawSocket::sendPkt()
{
    struct iphdr *ip_hdr;   //iphdr为IP头部结构体
    struct icmphdr *icmp_hdr;   //icmphdr为ICMP头部结构体
    int len;
    int i;
    icmp_hdr=(struct icmphdr *)(m_buff);  //字符串指针
    icmp_hdr->type=ICMP_ECHO;    //初始化ICMP消息类型type
    icmp_hdr->code=0;    //初始化消息代码code
    icmp_hdr->un.echo.id=1;   //把进程标识码初始给icmp_id
    icmp_hdr->un.echo.sequence=n++;  //发送的ICMP消息序号赋值给icmp序号
//    gettimeofday((struct timeval *)icmp_hdr->icmp_timestamp,NULL); // 获取当前时间
    icmp_hdr->icmp_timestamp[0] = 0x5E11C643;

    memcpy(icmp_hdr->data, hello, strlen(hello));

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_addr.s_addr =  inet_addr("10.1.24.128");
    dst.sin_family = PF_INET;
    dst.sin_port = ntohs(0);

    len=ICMP_HSIZE+strlen(hello);
    icmp_hdr->checksum=0;    //初始化
    icmp_hdr->checksum=checksum((u8 *)icmp_hdr,len);  //计算校验和
    for(i=0; i < len; i++)
    {
        printf("%02X", (unsigned char)m_buff[i]);
        if((i+1)%8 == 0)
            printf("  ");
    }
    printf("\n");
  //  printf("The send pack checksum is:0x%x\n",icmp_hdr->checksum);
    sendto(m_rawSocket, m_buff, len, 0, (struct sockaddr *)&dst, sizeof (dst)); //经socket传送数据

    return 0;
}



