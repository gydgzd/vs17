/*
 * myTunTap.h
 *
 *  Created on: Jan 7, 2020
 *      Author: root
 *      reference:
 *      https://backreference.org/2010/03/26/tuntap-interface-tutorial/
 */

#ifndef MYTUNTAP_H_
#define MYTUNTAP_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>        // open
#include <sys/ioctl.h>    // ioctl
#include <unistd.h>       // close
#include <net/if.h>
#include <net/route.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
//#include <linux/icmp.h>
#include "myicmp.h"

class myTunTap {
public:
    myTunTap();
    virtual ~myTunTap();

    int dev_alloc(char *dev, short flags);
    int dev_read();
    int dev_write();
private:
    int m_fd;
    char m_devName[32];
};

#endif /* MYTUNTAP_H_ */





