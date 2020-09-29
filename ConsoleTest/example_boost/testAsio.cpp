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

boost::asio::io_service myservice;

void run()
{
    myservice.run();
}
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
void client_session(socket_ptr sock) {
    while (true) {
        char data[512];
        size_t len = sock->read_some(buffer(data));
        if (len > 0)
            write(*sock, buffer("ok", 2));
    }
}

int testAsio::test()
{
	boost::asio::deadline_timer timer1(myservice, boost::posix_time::seconds(5));
	timer1.async_wait(handler1);
	boost::asio::deadline_timer timer2(myservice, boost::posix_time::seconds(5));
	timer2.async_wait(handler2);
	boost::thread thread1(run);
	boost::thread thread2(run);
	thread1.join();
	thread2.join();

    // client
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string("172.18.10.129"), 2000);
    ip::tcp::socket sock(service);
    try {
        sock.connect(ep);
    }
    catch (boost::system::system_error e)
    {
        std::cout << "connect error: " << e.code() << " - " << e.what() << std::endl;
    }
    

    // server
    
    io_service service1;
    ip::tcp::endpoint ep1(ip::tcp::v4(), 2001); // listen on 2001
    ip::tcp::acceptor acc(service1, ep1);
    while (true) {
        socket_ptr sock(new ip::tcp::socket(service1));
        acc.accept(*sock);
        boost::thread(boost::bind(client_session, sock));
    }

	return 0;
}
