/*
 * HttpServerLibevent.h
 *
 *  Created on: Jul 2, 2020
 *      Author: gyd
 */

#ifndef HTTPSERVERLIBEVENT_H_
#define HTTPSERVERLIBEVENT_H_

#include <netinet/in.h>      // For sockaddr_in
#include <sys/socket.h>      // For socket functions
#include <fcntl.h>           // For fcntl

#include <event2/event.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);


class HttpServerLibevent {
public:
    HttpServerLibevent();
    virtual ~HttpServerLibevent();


    int testLibevent();


};

#endif /* HTTPSERVERLIBEVENT_H_ */
