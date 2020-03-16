// test_SimpleAmqpClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <iostream>

using namespace AmqpClient;


int main()
{
	AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create("10.1.0.192");

	std::string consumer_tag = channel->BasicConsume("test", "");
	Envelope::ptr_t envelope = channel->BasicConsumeMessage(consumer_tag);
	// Alternatively:
//	Envelope::ptr_t envelope;
//	channel->BasicConsumeMessage(consumer_tag, envelope, 10); // 10 ms timeout
															  // To ack:
	channel->BasicAck(envelope);
	// To cancel:
	channel->BasicCancel(consumer_tag);


    return 0;
}

