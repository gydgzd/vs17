/*
 * my_zmq_client.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: gyd
 */


//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

int my_zmq_client (void)
{
    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
  //  char  addr[] = "tcp://172.18.10.129:5555";
    char  addr[] = "tcp://10.1.24.63:14460";
    int rc = 0;
    rc = zmq_connect (requester, addr);
    assert(rc == 0);
    if(rc != 0)
    	printf("Connect failed.\n");
    printf("ret = %d, errno = %d\n", rc, errno);
    printf("%s \n", strerror(errno));
    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [100];
        printf ("Sending Hello %d…\n", request_nbr);
    //    zmq_send (requester, "Hello", 5, 0);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received World %d\n", request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}

