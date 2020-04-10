#include "stdafx.h"
#include "testAsio.h"


testAsio::testAsio()
{
}


testAsio::~testAsio()
{
}

void handler1(const boost::system::error_code &ec)
{
	for(int i = 0; i < 5; i++)
		std::cout << "handle 1: " << i << "s."<< std::endl;
}

void handler2(const boost::system::error_code &ec)
{
	for (int i = 0; i < 5; i++)
		std::cout << "handle 2: " << i << "s." << std::endl;
}

boost::asio::io_service io_service;

void run()
{
	io_service.run();
}
int testAsio::test()
{

	boost::asio::deadline_timer timer1(io_service, boost::posix_time::seconds(5));
	timer1.async_wait(handler1);
	boost::asio::deadline_timer timer2(io_service, boost::posix_time::seconds(5));
	timer2.async_wait(handler2);
	boost::thread thread1(run);
	boost::thread thread2(run);
	thread1.join();
	thread2.join();
	return 0;
}
