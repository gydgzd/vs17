/*
 * myTunTap.cpp
 *
 *  Created on: Jan 7, 2020
 *      Author: root
 *      # ip link set proxy_tun up
 *      # ip addr add 10.0.0.1/24 dev proxy_tun
 */

#include "myTunTap.h"

myTunTap::myTunTap() {
    // TODO Auto-generated constructor stub
    m_fd = 0;
    strcpy(m_devName, "proxy_tun");
    m_fd = dev_alloc(m_devName, IFF_TUN | IFF_NO_PI);
    if(m_fd < 0){
        perror("Allocating interface");
        exit(1);
    }
}

myTunTap::~myTunTap() {
    // TODO Auto-generated destructor stub
}


/* Arguments taken by the function:
 * char *dev: the name of an interface (or '\0'). MUST have enough
 *             space to hold the interface name if '\0' is passed
 * int flags: interface flags (eg, IFF_TUN etc.)
 */
int myTunTap::dev_alloc(char *dev, short flags)
{
    struct ifreq ifr;
    int fd, err;
    char clonedev[16] = "/dev/net/tun";

    if( (fd = open(clonedev, O_RDWR)) < 0 ) {
        return fd;
     }
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags |= flags;   // IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI

    if (*dev) {
        // if a device name was specified, put it in the structure; otherwise,
        // the kernel will try to allocate the "next" device of the specified type
        strncpy(ifr.ifr_name, dev, strlen(dev));
    }
    /* try to create the device */
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 )
    {
        printf("create interface error: %d - %s\n", errno, strerror(errno));
        close(fd);
        return err;
    }
//    strcpy(dev, ifr.ifr_name);
    // system("ip link set proxy_tun up");
    int sd = 0;
    if((sd = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("Error create socket :%d - %s\n", errno, strerror(errno));
        return -1;
    }
    if ((err = ioctl(sd, SIOCGIFFLAGS, &ifr))<0)
    {
        printf("get status error: %d - %s\n", errno, strerror(errno));
        close(sd);
        close(fd);
        return-1;
    }
    if(ifr.ifr_flags & IFF_RUNNING)
    {
        printf("interface status: up\n");
    }else
    {
        printf("interface status: down\n");
        ifr.ifr_flags |= IFF_UP;
        if(ioctl(sd, SIOCSIFFLAGS, &ifr) < 0)
        {
            printf("%s set status to up error: %d - %s\n",ifr.ifr_name, errno, strerror(errno));
            close(sd);
            close(fd);
            return -1;
        }
        else
            printf("%s set status to up success.\n", ifr.ifr_name);
    }

    //system("ip addr add 10.0.0.1/24 dev proxy_tun");
    char ip[16] = "10.0.0.2";
    struct sockaddr_in addr;
    addr.sin_addr.s_addr =  inet_addr(ip);//
    addr.sin_family = PF_INET;
    addr.sin_port = ntohs(0);
    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));
    if(ioctl(sd, SIOCSIFADDR, &ifr) < 0)
    {
        printf("%s SIOCSIFADDR error :%d - %s\n",ifr.ifr_name, errno, strerror(errno));
        close(sd);
        close(fd);
        return -1;
    }
    else
        printf("%s set ip to %s success\n",ifr.ifr_name, ip);
    addr.sin_addr.s_addr =  inet_addr("255.255.255.0");//
    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));
    if(ioctl(sd, SIOCSIFNETMASK, &ifr) < 0)
    {
        printf("%s set SIOCSIFNETMASK error :%d - %s\n",ifr.ifr_name, errno, strerror(errno));
        close(sd);
        close(fd);
        return -1;
    }
    else
        printf("%s set mask to 255.255.255.0 success\n",ifr.ifr_name);
    // set gateway(route)
    struct rtentry rt;
    addr.sin_addr.s_addr =  inet_addr("10.0.0.1");//
    memcpy ( &rt.rt_gateway, &addr, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(sd, SIOCADDRT, &rt)<0)
    {
        printf("%s set SIOCADDRT error :%d - %s\n",ifr.ifr_name, errno, strerror(errno));
        close(sd);
        close(fd);
        return -1;
    }
    else
        printf("%s set gateway to 10.0.0.1 success\n",ifr.ifr_name);

    close(sd);
    return fd;
}

int myTunTap::dev_read()
{

    int nread = 0;
    unsigned char buffer[4096];
    /* Now read data coming from the kernel */
    while(1)
    {
        memset(buffer, 0, 4096);
        /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
        nread = read(m_fd, buffer,sizeof(buffer));
        if(nread < 0)
        {
            perror("Reading from interface");
            close(m_fd);
            exit(1);
        }

        /* Do whatever with the data */
        printf("Read %d bytes from device %s\n", nread, m_devName);
        for(int n = 0; n < nread; n++)
        {
            printf("%02x", buffer[n]);
            if( (n+1)%8 == 0)
                printf("  ");
        }
        printf("\n");
    }
    return 0;
}

int myTunTap::dev_write()
{
    const char hello[] = "This is my ping.";
    unsigned char buffer[4096];
    unsigned char rdBuffer[4096];
    struct ethhdr *eth_hdr;
    struct iphdr *ip_hdr;   //iphdr为IP头部结构体
    struct icmphdr *icmp_hdr;   //icmphdr为ICMP头部结构体
    int len;
    int i;
 //   eth_hdr = (struct ethhdr *)buffer;
    ip_hdr = (struct iphdr *)(buffer );        //+ sizeof(ethhdr)
    icmp_hdr=(struct icmphdr *)(buffer + sizeof(struct iphdr));   //+ sizeof(ethhdr)
    // Fabricate the IP header or we can use the
    // standard header structures but assign our own values.
 //   unsigned char dstMac[6] = {0x00, 0x0c, 0x29, 0xce, 0xb7, 0xb6};
 //   unsigned char srcMac[6] = {0xcc, 0xd3, 0x1e, 0x78, 0x69, 0xfd};
 //   memcpy(eth_hdr->h_dest, &dstMac, 6);
 //   memcpy(eth_hdr->h_source, &srcMac, 6);
 //   eth_hdr->h_proto = 0x0008;
    ip_hdr->ihl = 5;
    ip_hdr->version = 4;//报头长度，4*32=128bit=16B
    ip_hdr->tos = 0; // 服务类型
    ip_hdr->tot_len = htons(((sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(hello))));
    //ip->id = htons(54321);//可以不写
    ip_hdr->ttl = 64; // hops生存周期
    ip_hdr->protocol = IPPROTO_ICMP; // UDP
    ip_hdr->check = 0;
    // Source IP address, can use spoofed address here!!!
    ip_hdr->saddr = inet_addr("10.0.0.11");
    // The destination IP address
    ip_hdr->daddr = inet_addr("10.0.0.2");

    icmp_hdr->type=8;    //初始化ICMP消息类型type
    icmp_hdr->code=0;    //初始化消息代码code
    icmp_hdr->un.echo.id=1;   //把进程标识码初始给icmp_id
    icmp_hdr->un.echo.sequence=1;  //发送的ICMP消息序号赋值给icmp序号
//    gettimeofday((struct timeval *)icmp_hdr->icmp_timestamp,NULL); // 获取当前时间
    icmp_hdr->icmp_timestamp[0] = 0x5E11C643;

    memcpy(icmp_hdr->data, hello, strlen(hello));

    len=14 + 20 + ICMP_HSIZE+strlen(hello);
    icmp_hdr->checksum=0;    //初始化
    icmp_hdr->checksum=checksum((unsigned char *)icmp_hdr,ICMP_HSIZE+strlen(hello));  //计算校验和
    for(i=0; i < len; i++)
    {
        printf("%02X", (unsigned char)buffer[i]);
        if((i+1)%8 == 0)
            printf("  ");
    }
    printf("\n");
  //  printf("The send pack checksum is:0x%x\n",icmp_hdr->checksum);
    int nwrite = 0;
    int nread = 0;
    while(true)
    {
        if((nwrite=write(m_fd, buffer, len)) < 0)
        {
            perror("Writing data");
            return -1;
        }
        printf("write:");
        for(i=0; i < len; i++)
        {
            printf("%02X", (unsigned char)buffer[i]);
            if((i+1)%8 == 0)
                printf("  ");
        }
        printf("\n");
        //
        memset(rdBuffer, 0, 4096);
        if((nread=read(m_fd, rdBuffer, len)) < 0)
        {
            perror("Writing data");
            return -1;
        }
        printf("read:");
        for(i=0; i < len; i++)
        {
            printf("%02X", (unsigned char)rdBuffer[i]);
            if((i+1)%8 == 0)
                printf("  ");
        }
        printf("\n");

        sleep(1);
    }
    return 0;
}

