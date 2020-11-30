/*
 * my_zmq_server.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: gyd
 */


//  Hello World server in C
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int my_zmq_server (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://172.18.10.129:5555");
    if (rc != 0)
    {
    	printf("zma_bind failed: %s \n",strerror(errno));
    }
    while (1) {
        char buffer [10];
        zmq_recv (responder, buffer, 10, 0);
        printf ("Received %s\n", buffer);
        sleep (1);          //  Do some 'work'
        zmq_send (responder, "World", 5, 0);
    }
    return 0;
}

