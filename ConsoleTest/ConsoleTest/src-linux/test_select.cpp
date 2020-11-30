/*
 * test_select.cpp
 *
 *  Created on: Jan 5, 2019
 *      Author: gyd
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>       // fcntl
#include <netinet/in.h>
#include <arpa/inet.h>   // inet_pton
#include "socketDef.h"
#include "Mylog.h"
using namespace std;
#define DEFAULT_PORT 3401
#define MAXLINE 4096
void thread_fun(int socket_fd);
int myrecv(int fd, struct sockaddr_in *client_addr, char *buffer );//
int getCONN(int fd, struct sockaddr_in *client_addr, CONNECTION & client);
int logMsg(const MSGBODY *pMsg, const char *logHead, int isRecv);
int socket_server()
{
    int socket_fd;
    struct sockaddr_in servaddr;
    // initialize
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        printf("Create socket error: %s (errno: %d). Exit.\n", strerror(errno), errno);
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //
    servaddr.sin_port = htons(DEFAULT_PORT);       //
    //
    int optval = 1;
    if(-1 == setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
    {
        printf( "ERROR: Reuse addr error: %s (errno: %d). \nExit.", strerror(errno), errno);
        exit(-1);
        //return -1;
    }
    // bind
    if( bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("Bind error: %s (errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    if( listen(socket_fd, 10) == -1)
    {
        printf("Listen error: %s (errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    std::thread th1{thread_fun, socket_fd};
    th1.join();
    close(socket_fd);
    return 0;
}
// ensure that socket_fd is valid before use this function
void thread_fun(int socket_fd)
{
    vector<int> v_fds;
    v_fds.push_back(socket_fd);
    int conns[4096] = {};
    int z = 0;
    struct timeval tv;  // = {5, 0};
    /*设置超时时间*/
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&client_addr, 0, client_len);

    CONNECTION client;
    char logmsg[512] = "";
    Mylog mylog;
    fd_set read_fds, test_fds;
    FD_ZERO(&read_fds);                    //把可读文件描述符的集合清空
    FD_ZERO(&test_fds);
    FD_SET(socket_fd, &read_fds);          //把监听的文件描述符加入到集合中
    int retval;
    while(true)
    {
        test_fds = read_fds;
        int nread = 0;
        retval = select(FD_SETSIZE, &test_fds, NULL, NULL, &tv);
        if(retval == 0)
            continue;
        if(retval < 0)
        {
            printf("select error: %d - %s\n", errno, strerror(errno));
            exit(-1);
        }
        for(unsigned int idx = 0; idx < v_fds.size(); ) //扫描所有的文件描述符
        {
            if(FD_ISSET(v_fds[idx],&test_fds))     //找到相关文件描述符
            {
                if(v_fds[idx] == socket_fd)        //判断是否为服务器套接字，是则表示为客户请求连接。
                {
                    memset(&client, 0, sizeof(client));

                    client.socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
                    if( client.socket_fd < 0 ) {
                        sprintf(logmsg, "Accept error: %s (errno: %d)\n", strerror(errno), errno);
                        exit(1);
                    }
                    getCONN(client.socket_fd, &client_addr, client);
                    sprintf(logmsg, "INFO: %s:%d --> %s:%d connected", client.clientIP, client.clientPort, client.serverIP, client.serverPort);
                    mylog.logException(logmsg);
                    //set nonlocking mode
                    int flags;
                    if( (flags = fcntl(client.socket_fd, F_GETFL, 0)) < 0)
                    {
                        sprintf(logmsg, "ERROR: %s:%d --> %s:%d: fcntl error: %d--%s. Give up the connection.",client.clientIP, client.clientPort, client.serverIP, client.serverPort, errno, strerror(errno) );
                        mylog.logException(logmsg);
                        continue;
                    }
                    fcntl(client.socket_fd, F_SETFL, flags | O_NONBLOCK);
                    FD_SET(client.socket_fd, &read_fds);

                    //把连接保存到临时数组中;
                    v_fds.push_back(client.socket_fd);

                }
                else
                {
                    ioctl(v_fds[idx], FIONREAD, &nread);  //取得数据量交给nread
                    if(nread == 0)                //客户数据请求完毕，关闭套接字，从集合中清除相应描述符
                    {
                        close(v_fds[idx]);
                        FD_CLR(v_fds[idx], &read_fds);     //去掉关闭的fd
                        printf("removing client on fd %d\n", v_fds[idx]);
                        auto iter = v_fds.begin();
                        for( ; iter != v_fds.end(); )
                        {
                            if(*iter == v_fds[idx])
                            {
                                v_fds.erase(iter);
                                continue;
                            }
                            else
                                iter++;
                        }
                    }
                    /*处理客户数据请求*/
                    else
                    {
                        char buffer[1024];
                        memset(buffer, 0 ,sizeof(buffer));
                        myrecv(v_fds[idx], &client_addr, buffer);
                        if(strcmp(buffer, "exit\n") == 0) break;
                        printf("%s\n", buffer);

                        MSGBODY msg;
                        msg.type = 1;
                        sprintf((char *)msg.msg, "hello");
                        msg.length = strlen((char *)msg.msg);
                        for(int i=0; i<z; i++)
                        {
                            int ret = send(conns[i], &msg, sizeof(msg.type) + sizeof(msg.length)+ msg.length, 0);
                            if( ret < 0 )
                            {
                                printf("ERROR: send msg error: %s(errno: %d)\n", strerror(errno), errno);
                            }else
                            {
                                printf("send %s\n", (char *)msg.msg);
                            }
                        }
                    }
                }
            } // end of if(FD_ISSET(fd,&test_fds))
            idx++;
        } // end of for
    } // end of while
}

/*
 * recv thread function
 *
 */
int myrecv(int fd, struct sockaddr_in *client_addr, char *buffer )
{
    CONNECTION client;
    getCONN(fd, client_addr, client);
    char logmsg[512] = "";
    char logHead[64] = "";
    sprintf(logHead, "%s:%d --> %s:%d ", client.clientIP, client.clientPort, client.serverIP, client.serverPort);
    Mylog mylog;
    int length = 0;
    MSGBODY recvMsg;
    int err = 0;
    while(true)
    {
        recvMsg.length = 0;
        memset(&recvMsg, 0, sizeof(recvMsg));
        // recv head, to get the length of msg
        length = recv(client.socket_fd, &recvMsg, MSGHEAD_LENGTH, 0);
        if(length == -1)     // recv error
        {
            err = errno;
            if(err != 11)   // data isnot ready when errno = 11, log other error
            {
                sprintf(logmsg, "ERROR: %s recv error: %d--%s",logHead, errno, strerror(errno) );
                mylog.logException(logmsg);
                if(err == 9)
                {
                    close(client.socket_fd);
                    client.status = 0;
                    mylog.logException("ERROR: recv exit.");
                    return -1;
                }
            }
            //sleep(1);
            return 0;
        }
        else                              // recv success
        {
            if( length == 0 )
            {
                close(client.socket_fd);
                client.status = 0;
                sprintf(logmsg, "INFO: %s: The client exited. Recv thread exit.", logHead);
                mylog.logException(logmsg);
                return 0;
            }
        }
        // recv msg head to get length, then get msg by length
        if(0 != recvMsg.length)
        {
            printf("type = %d, recvLen = %d,\n", recvMsg.type, recvMsg.length);
            length = recv(client.socket_fd, recvMsg.msg, recvMsg.length, 0);
            if(length == -1)     // recv
            {
                err = errno;
                if(err != 11) // data isnot ready when errno = 11, log other error
                {
                    sprintf(logmsg, "ERROR: %s recv msg error: %d--%s",logHead, errno, strerror(errno) );
                    mylog.logException(logmsg);
                }
                if(err == 9)
                {
                    close(client.socket_fd);
                    client.status = 0;
                    mylog.logException("ERROR: recv exit.");
                    return -1;
                }
                //sleep(1);
                usleep(10000);  // 10ms
                length = 0;  // set it back to 0
                continue;
            }
            else                     // recv success
            {

            /*    int ret = 0;
                ret = msgCheck(&recvBuf);
                if(ret == 0)  // valid msg
                {
                //  printf("sizeof SendQueue: %lu, RecvQueue: %lu\n", mp_msgQueueSend->size(), mp_msgQueueRecv->size());

                }
                else
                {
                    mylog.logException("INFO: msg invalid.");
                }*/
                if( length == 0 )
                {
                    close(client.socket_fd);
                    client.status = 0;
                    sprintf(logmsg, "INFO: %s: The client exited. Recv thread exit.", logHead);
                    mylog.logException(logmsg);
                    return -1;
                }else
                {
                    logMsg(&recvMsg, logHead, 1);
                }

            }// end if,  recv finished
        }
    }
    return 0;
}

int getCONN(int fd, struct sockaddr_in *client_addr, CONNECTION & client)
{
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    memset(&server_addr, 0, server_len);

    // get server address
    getsockname(fd, (struct sockaddr *)&server_addr, &server_len);
    inet_ntop(AF_INET,(void *)&server_addr.sin_addr, client.serverIP, 64 );
    client.serverPort = ntohs(server_addr.sin_port);

    // get client address
    inet_ntop(AF_INET,(void *)&client_addr->sin_addr, client.clientIP, 64 );
    client.clientPort = ntohs(client_addr->sin_port);
    client.socket_fd = fd;
    client.status = 1;
    return 0;
}

int logMsg(const MSGBODY *pMsg, const char *logHead, int isRecv)
{
    Mylog mylog;
    if(pMsg == NULL)
        return -1;
    char logmsg[256] = "";
    char direction[32] = "";
    if(1 == isRecv)
        sprintf(direction, "received");
    else
        sprintf(direction, "send");
    if(2 == pMsg->type)             //  hex
    {
        try
        {
            char *p_hexLog = new char[pMsg->length*3 + 128];    // include the logHead
            memset(p_hexLog, 0, pMsg->length*3 + 128);
            sprintf(p_hexLog, "INFO: %s %s: ", logHead, direction);
            int len = strlen(p_hexLog);
            for(int i=0; i<pMsg->length; i++)
                sprintf(p_hexLog+len+3*i, "%02x ", (unsigned char)pMsg->msg[i]);
            mylog.logException(p_hexLog);
            delete[] p_hexLog;
        }catch(bad_alloc& bad)
        {
            sprintf(logmsg,"ERROR: Failed to alloc mem when log hex: %s", bad.what());
            mylog.logException(logmsg);
        }
    }
    else if(1 == pMsg->type)
    {
        char logmsg[pMsg->length + 128];
        memset(logmsg, 0, pMsg->length + 128);
        sprintf(logmsg, "INFO: %s %s: %s", logHead, direction, pMsg->msg);
        mylog.logException(logmsg);
    }
    else if(0 == pMsg->type)   // int
    {
        char logmsg[pMsg->length + 128];
        memset(logmsg, 0, pMsg->length + 128);
        sprintf(logmsg, "INFO: %s %s: %d", logHead, direction, *(int *)pMsg->msg);
        mylog.logException(logmsg);
    }
    return 0;
}
