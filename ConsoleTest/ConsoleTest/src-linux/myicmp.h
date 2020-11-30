/*
 * myicmp.h
 *
 *  Created on: Jan 7, 2020
 *      Author: root
 */

#ifndef MYICMP_H_
#define MYICMP_H_

#define ICMP_HSIZE sizeof(struct icmphdr)
#define ICMP_ECHOREPLY 0 //Echo应答
#define ICMP_ECHO      8 //Echo请求
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
struct icmphdr
{
    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    union
    {
        struct
        {
            unsigned short id;
            unsigned short sequence;
        }echo;
        unsigned int gateway;
        struct
        {
            unsigned short unused;
            unsigned short mtu;
        }frag; //pmtu发现
    }un;
    unsigned int  icmp_timestamp[2];//时间戳
    //ICMP数据占位符
    unsigned char data[0];
#define icmp_id un.echo.id
#define icmp_seq un.echo.sequence
};

//计算校验和
extern unsigned short checksum(unsigned char *buf,int len);


#endif /* MYICMP_H_ */
