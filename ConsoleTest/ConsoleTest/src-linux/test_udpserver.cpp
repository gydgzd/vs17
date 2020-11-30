/*
 * test_udpserver.cpp
 *
 *  Created on: May 10, 2019
 *      Author: gyd
 */


#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>      // for close , usleep
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>   //inet_pton
//#include <netdb.h>
#include <fcntl.h>       // fcntl
#include <queue>
#include "Mylog.h"

int test_udpserver()
{
    unsigned int sin_len;
    char message[256] = "";

    int socketfd;
    struct sockaddr_in sin;
    printf("Waiting for data form sender \n");

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin.sin_port=htons(1234);
    sin_len=sizeof(sin);

    socketfd=socket(AF_INET,SOCK_DGRAM,0);
    bind(socketfd,(struct sockaddr *)&sin,sizeof(sin));

    while(1)
    {
        recvfrom(socketfd, message, sizeof(message), 0, (struct sockaddr *)&sin, &sin_len);
        printf("Response from server:%s\n",message);
        if(strncmp(message,"stop",4) == 0)//接受到的消息为 “stop”
        {

            printf("Sender has told me to end the connection\n");
            break;
        }
    }

    close(socketfd);

    return (EXIT_SUCCESS);
}

