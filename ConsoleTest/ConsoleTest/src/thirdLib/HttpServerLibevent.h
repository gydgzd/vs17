/*
 * HttpServerLibevent.h
 *
 *  Created on: Jul 2, 2020
 *      Author: gyd
 */

#ifndef HTTPSERVERLIBEVENT_H_
#define HTTPSERVERLIBEVENT_H_

#include <sys/types.h>
#include <event2/event-config.h>
#include <iostream>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <event2/event.h>

#if defined(_WIN64)
typedef long int ssize_t;
#endif
#if defined(WIN32)
typedef long ssize_t;
#endif

constexpr auto MAX_LINE = 16384;

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);

extern int called;

class HttpServerLibevent {
public:
    HttpServerLibevent();
    virtual ~HttpServerLibevent();


    int testLibevent();


};

#endif /* HTTPSERVERLIBEVENT_H_ */
