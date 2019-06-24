/*
* ***** BEGIN LICENSE BLOCK *****
* Version: MIT
*
* Portions created by Alan Antonuk are Copyright (c) 2012-2013
* Alan Antonuk. All Rights Reserved.
*
* Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
* All Rights Reserved.
*
* Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
* VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy,
* modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ***** END LICENSE BLOCK *****
*/
#include "stdafx.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "utils.h"

#include <string>
using namespace std;

#define SUMMARY_EVERY_US 1000000

static void send_batch(amqp_connection_state_t conn, char const *queue_name, int rate_limit, int message_count, char * messagebody) {
	uint64_t start_time = now_microseconds();
	int i;
	int sent = 0;
	int previous_sent = 0;
	uint64_t previous_report_time = start_time;
	uint64_t next_summary_time = start_time + SUMMARY_EVERY_US;

	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */

	for (i = 0; i < message_count; i++) {
		uint64_t now = now_microseconds();

		die_on_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(""), amqp_cstring_bytes(queue_name), 0, 0, &props, amqp_cstring_bytes(messagebody)), "Publishing");
		sent++;
		if (now > next_summary_time) {
			int countOverInterval = sent - previous_sent;
			double intervalRate = countOverInterval / ((now - previous_report_time) / 1000000.0);
			printf("%d ms: Sent %d - %d since last report (%d Hz)\n", (int)(now - start_time) / 1000, sent, countOverInterval, (int)intervalRate);

			previous_sent = sent;
			previous_report_time = now;
			next_summary_time += SUMMARY_EVERY_US;
		}

		while (((i * 1000000.0) / (now - start_time)) > rate_limit) {
			microsleep(2000);
			now = now_microseconds();
		}
	}

	{
		uint64_t stop_time = now_microseconds();
		int total_delta = (int)(stop_time - start_time);

		printf("PRODUCER - Message count: %d\n", message_count);
		printf("Total time, milliseconds: %d\n", total_delta / 1000);
		printf("Overall messages-per-second: %g\n",
			(message_count / (total_delta / 1000000.0)));
	}
}

int amqp_producer(char const *hostname, int port, int rate_limit, int message_count) {

	char *queue = "task";
	char *exchange = "";
	char *bindingkey = "task";
	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;
	conn = amqp_new_connection();
	socket = amqp_tcp_socket_new(conn);
	if (!socket) {
		die("creating TCP socket");
	}

	int status = amqp_socket_open(socket, hostname, port);
	if (status) {
		die("opening TCP socket");
	}

	die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "cnki", "cnki"), "Logging in");
	amqp_channel_open(conn, 1);
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

	amqp_queue_bind(conn, 1, amqp_cstring_bytes(queue), amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey), amqp_empty_table);  
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Unbinding");

	char msg[256] = "Nice to see U.";
	send_batch(conn, "test", rate_limit, message_count, msg);

	die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
	die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
	die_on_error(amqp_destroy_connection(conn), "Ending connection");
	return 0;
}
