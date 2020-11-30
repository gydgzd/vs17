/*
 * socketDef.h
 *
 *  Created on: May 7, 2019
 *      Author: gyd
 */
#ifndef SOCKETDEF_H_
#define SOCKETDEF_H_

#include <stdio.h>

using namespace std;
typedef unsigned char BYTE;
#define MSGHEAD_LENGTH 8
#define MAXLENGTH 1024*64
struct MSGBODY
{
    int type;              // 0:int, 1:string, 2: byte(hex)
    int length;            // length of msg
    BYTE msg[MAXLENGTH];
    MSGBODY()
    {
        memset(this, 0, sizeof(MSGBODY));
        type = 0;
        length = 0;
    }
    MSGBODY(const MSGBODY & msgbody)
    {
        memset(this, 0, sizeof(MSGBODY));
        type = msgbody.type;
        length = msgbody.length;
        memcpy(msg,msgbody.msg,length);
    }
};
/*
 * use to form a string clientIP:clientPort--> serverIP:serverPort
 */
struct CONNECTION
{
    int  socket_fd ;       //
    int  status ;          // 0: closed; 1:connected
    int  clientPort ;      // test if ok
    int  serverPort ;
    char clientIP[64] ;
    char serverIP[64] ;
    CONNECTION()
    {
        memset(this, 0, sizeof(CONNECTION));
        socket_fd = 0;
        status = 0;
        clientPort = 0;
        serverPort = 0;
    }
};


#endif /* SOCKETDEF_H_ */
