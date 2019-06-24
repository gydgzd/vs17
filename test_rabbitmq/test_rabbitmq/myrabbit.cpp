
#include "stdafx.h"
#include "myrabbit.h"
#include <assert.h>


MyRabbit::MyRabbit()
{
	mn_channelID = 1;
	mn_isConnected = 0;
	m_rabbitlog.setLogFile("log/rabbitmq.log");
}

MyRabbit::~MyRabbit()
{
	if (mn_isConnected == 1)
		closeRabbit();
}

int MyRabbit::connectRabbit(const char * hostname, int port, const char * username, const char * passwd)
{
	msz_host = hostname;
	mn_port = port;
	msz_user = username;
	msz_passwd = passwd;

	m_conn = amqp_new_connection(); 
	m_socket = amqp_tcp_socket_new(m_conn);
	if (!m_socket) {
		log("creating TCP socket");
	}

	int status = amqp_socket_open(m_socket, msz_host, mn_port);
	if (status) {
		log("opening TCP socket");
	}
	int ret = 0;
	ret = log_on_amqp_error(amqp_login(m_conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, msz_user, msz_passwd), "Logging in");
	if (ret != 0)
		return -1;
	amqp_channel_open(m_conn, mn_channelID);
	ret = log_on_amqp_error(amqp_get_rpc_reply(m_conn), "Opening channel");
	if (ret != 0)
		return -1;
	mn_isConnected = 1;
	return 0;
}

int MyRabbit::reconnectRabbit()
{
	if(mn_isConnected == 1)
		closeRabbit();
	m_conn = amqp_new_connection();
	m_socket = amqp_tcp_socket_new(m_conn);
	if (!m_socket) {
		log("creating TCP socket");
	}

	int status = amqp_socket_open(m_socket, msz_host, mn_port);
	if (status) {
		log("opening TCP socket");
	}
	int ret = 0;
	ret = log_on_amqp_error(amqp_login(m_conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, msz_user, msz_passwd), "Logging in");
	if (ret != 0)
		return -1;
	amqp_channel_open(m_conn, mn_channelID);
	ret = log_on_amqp_error(amqp_get_rpc_reply(m_conn), "Opening channel");
	if (ret != 0)
		return -1;
	mn_isConnected = 1;
	return 0;
}

void MyRabbit::closeRabbit()
{
	log_on_amqp_error(amqp_channel_close(m_conn, mn_channelID, AMQP_REPLY_SUCCESS), "Closing channel");
	log_on_amqp_error(amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS), "Closing connection");
	log_on_error(amqp_destroy_connection(m_conn), "Ending connection");
	mn_isConnected = 0;
}

int MyRabbit::createQueue(char const *exchange, char const *exchangetype, char const * queue, char const *bindingkey)
{
	amqp_bytes_t queuename;
	amqp_exchange_declare(m_conn, mn_channelID, amqp_cstring_bytes(exchange), amqp_cstring_bytes(exchangetype), 0, 1, 0, 0, amqp_empty_table);
	log_on_amqp_error(amqp_get_rpc_reply(m_conn), "Declaring exchange");

	{
		amqp_queue_declare_ok_t *pret = amqp_queue_declare(m_conn, mn_channelID, amqp_cstring_bytes(queue), 0, 1, 0, 0, amqp_empty_table);
		if (pret == nullptr)
			return 1;
		log_on_amqp_error(amqp_get_rpc_reply(m_conn), "Declaring queue");
		queuename = amqp_bytes_malloc_dup(pret->queue);

		amqp_queue_bind(m_conn, mn_channelID, amqp_cstring_bytes(queue), amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey), amqp_empty_table);

		if (queuename.bytes == NULL) {
			std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
			fprintf(stderr, "Out of memory while copying queue name");
			return 1;
		}
		amqp_bytes_free(queuename);
	}
	

	return 0;
}

int MyRabbit::sendString(char const *exchange, char const *route_key, char * messagebody) {
	char logmsg[1024] = "";
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.content_encoding = amqp_cstring_bytes("utf-8");
	props.delivery_mode = 2;   /* persistent delivery mode */
	amqp_confirm_select(m_conn, mn_channelID);  //在通道上打开Publish确认

//	die_on_error(amqp_basic_publish(m_conn, mn_channelID, amqp_cstring_bytes(exchange), amqp_cstring_bytes(route_key), 0, 0, &props, amqp_cstring_bytes(messagebody)), "Publishing");
	log_on_error(amqp_basic_publish(m_conn, mn_channelID, amqp_cstring_bytes(exchange), amqp_cstring_bytes(route_key), 0, 0, &props, amqp_cstring_bytes(messagebody)), "Publishing");
	{
		/* Publish消息后需要在当前通道上监听返回的信息，来判断消息是否成功投递
		* 有几种情况要判断
		*/
		amqp_frame_t frame;
		amqp_rpc_reply_t ret;

		if (AMQP_STATUS_OK != amqp_simple_wait_frame(m_conn, &frame)) {
			return -1;
		}

		if (AMQP_FRAME_METHOD == frame.frame_type) {
			amqp_method_t method = frame.payload.method;
			sprintf_s(logmsg, "method.id=%08X,method.name=%s\n", method.id, amqp_method_name(method.id));
		//	std::lock_guard<std::mutex> guard(g_num_mutex);   // 加锁，保证下面语句原子执行
		//	m_rabbitlog.logException(logmsg);
			switch (method.id) {
			/* if we've turned publisher confirms on, and we've published a message
			* here is a message being confirmed
			*/
			case AMQP_BASIC_ACK_METHOD: 
			{
				amqp_basic_ack_t *s;
				s = (amqp_basic_ack_t *)method.decoded;
				sprintf_s(logmsg, "Ack.delivery_tag=%I64d", s->delivery_tag);
				sprintf_s(logmsg, "%s, Ack.multiple=%d\n", logmsg, s->multiple);
			//	std::lock_guard<std::mutex> guard(g_num_mutex);   // 加锁，保证下面语句原子执行
			//	m_rabbitlog.logException(logmsg);    // don't log if it's normal
			}
			break;
		
			/* if we've turned publisher confirms on, and we've published a message
			* here is a message not being confirmed
			*/
			case AMQP_BASIC_NACK_METHOD:	
			{
				amqp_basic_nack_t *s;
				s = (amqp_basic_nack_t *)method.decoded;
				sprintf_s(logmsg, "NAck.delivery_tag=%I64d\n", s->delivery_tag);
				sprintf_s(logmsg, "%s, NAck.multiple=%d\n", logmsg, s->multiple);
				sprintf_s(logmsg, "%s, NAck.requeue=%d\n", logmsg, s->requeue);
				std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
				m_rabbitlog.logException(logmsg);
			}
			break;

			/* if a published message couldn't be routed and the mandatory flag was set
			* this is what would be returned. The message then needs to be read.
			*/			
			case AMQP_BASIC_RETURN_METHOD:
			{
				amqp_message_t message;
				amqp_basic_return_t *s;
				char str[1024];
				s = (amqp_basic_return_t *)method.decoded;
				sprintf_s(logmsg, "Return.reply_code=%d\n", s->reply_code);
				strncpy_s(str, (char *)s->reply_text.bytes, s->reply_text.len);
				str[s->reply_text.len] = 0;
				sprintf_s(logmsg, "%s, Return.reply_text=%s\n", logmsg, str);

				ret = amqp_read_message(m_conn, frame.channel, &message, 0);
				if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
					return 0;
				}
				strncpy_s(str, (char *)message.body.bytes, message.body.len); 
				str[message.body.len] = 0;
				sprintf_s(logmsg, "%s, Return.message=%s\n", logmsg, str);
				std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
				m_rabbitlog.logException(logmsg);
				amqp_destroy_message(&message);
			}
			break;

			case AMQP_CHANNEL_CLOSE_METHOD:
				/* a channel.close method happens when a channel exception occurs, this
				* can happen by publishing to an exchange that doesn't exist for example
				*
				* In this case you would need to open another channel redeclare any queues
				* that were declared auto-delete, and restart any consumers that were attached
				* to the previous channel
				*/
				return 0;

			case AMQP_CONNECTION_CLOSE_METHOD:
				/* a connection.close method happens when a connection exception occurs,
				* this can happen by trying to use a channel that isn't open for example.
				*
				* In this case the whole connection must be restarted.
				*/
				return 0;

			default:
			{	
				std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
				fprintf(stderr, "An unexpected method was received %d\n", frame.payload.method.id);
				return 0;
			}
			} // end of switch
		} // end of if
	} // end of sector

	return 0;
}

int MyRabbit::getString(char const * queue_name, char * const  buf, int& len)
{
	int prefetch_count = 1;                              // take only 1 message per consume
	amqp_basic_qos(m_conn, mn_channelID, 0, prefetch_count, 0);      
	amqp_basic_consume(m_conn, mn_channelID, amqp_cstring_bytes(queue_name), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
	log_on_amqp_error(amqp_get_rpc_reply(m_conn), "Consuming");

	{
		amqp_frame_t frame;
		int result;
		amqp_basic_deliver_t *d;
		amqp_basic_properties_t *p;
		size_t body_target;
		size_t body_received;
		int nCount = 0;
		while (nCount < prefetch_count) {
			amqp_maybe_release_buffers(m_conn);
			result = amqp_simple_wait_frame(m_conn, &frame);
			printf("Result %d\n", result);
			if (result < 0)
				break;

			printf("Frame type %d, channel %d\n", frame.frame_type, frame.channel);
			if (frame.frame_type != AMQP_FRAME_METHOD)
				continue;

			printf("Method %s\n", amqp_method_name(frame.payload.method.id));
			if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
				continue;

			d = (amqp_basic_deliver_t *)frame.payload.method.decoded;
			printf("Delivery %u, exchange %.*s routingkey %.*s\n", (unsigned)d->delivery_tag,
				(int)d->exchange.len, (char *)d->exchange.bytes,
				(int)d->routing_key.len, (char *)d->routing_key.bytes);

			result = amqp_simple_wait_frame(m_conn, &frame);
			if (result < 0)
				break;

			if (frame.frame_type != AMQP_FRAME_HEADER) {
				fprintf(stderr, "Expected header!");
				abort();
			}
			p = (amqp_basic_properties_t *)frame.payload.properties.decoded;
			if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
				printf("Content-type: %.*s\n",
					(int)p->content_type.len, (char *)p->content_type.bytes);
			}

			body_target = frame.payload.properties.body_size;
			body_received = 0;

			int sleep_seconds = 0;
			while (body_received < body_target) {
				result = amqp_simple_wait_frame(m_conn, &frame);
				if (result < 0)
					break;

				if (frame.frame_type != AMQP_FRAME_BODY) {
					fprintf(stderr, "Expected body!");
					abort();
				}

				body_received += frame.payload.body_fragment.len;
				assert(body_received <= body_target);

			/*	int i;
				for (i = 0; i<frame.payload.body_fragment.len; i++)
				{
					printf("%c", *((char*)frame.payload.body_fragment.bytes + i));
					if (*((char*)frame.payload.body_fragment.bytes + i) == '.')
						sleep_seconds++;
				}
				*/
				
				memcpy_s(buf + len, frame.payload.body_fragment.len + 1, (char*)frame.payload.body_fragment.bytes, frame.payload.body_fragment.len);
				len += frame.payload.body_fragment.len;
				printf("\n");
			}
			buf[len] = '\0';
			if (body_received != body_target) {
				/* Can only happen when amqp_simple_wait_frame returns <= 0 */
				/* We break here to close the connection */
				break;
			}
			/* do something */
#ifdef WINVER
			Sleep(sleep_seconds*1000);
#endif
#ifdef __LINUX
			sleep(sleep_seconds);
#endif // __LINUX

			amqp_basic_ack(m_conn, mn_channelID, d->delivery_tag, 0);
			nCount++;
		}
	}

	return 0;
}
