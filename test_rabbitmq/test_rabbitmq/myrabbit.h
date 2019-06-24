#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "utils.h"
#include "Mylog.h"
#ifdef WINVER
   #include <Windows.h>
#endif

#include <string>
#include <mutex>
using namespace std;

#define SUMMARY_EVERY_US 1000000

class MyRabbit
{
public:
	MyRabbit();
	~MyRabbit();

	int connectRabbit( const char * hostname, int port, const char * username, const char * passwd);
	int reconnectRabbit();
	void closeRabbit();

	int createQueue(char const *exchange, char const *exchangetype, char const * queue, char const *bindingkey);
	int sendString(char const *exchange, char const *route_key, char * messagebody);
	int getString(char const *queue_name, char * const buf, int& len);
	Mylog m_rabbitlog;
protected:
	int m_message_count;
	int mn_isConnected;	
	int mn_port;
	const char * msz_host;
	const char * msz_user;
	const char * msz_passwd;
	amqp_socket_t *m_socket;
	amqp_connection_state_t m_conn;
	amqp_channel_t mn_channelID;


};