#include "stdafx.h"
#include "testAsio.h"

std::mutex g_mutex;
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
    std::lock_guard <std::mutex> lock(g_mutex);
    myservice.run();
}
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
void client_session(socket_ptr sock) {
    register size_t len = 0;
    while (true) {
        char data[512];
        try {
            len = sock->read_some(buffer(data));
        }
        catch (boost::system::system_error &e)
        {
            std::cout << "read_some error: " << e.code() << " - " << e.what() << std::endl;
        }
        if (len > 0)
            write(*sock, buffer("ok", 2));
    }
}

int testAsio::test()
{
	boost::asio::deadline_timer timer1(myservice, boost::posix_time::seconds(2));
	timer1.async_wait(handler1);
	boost::asio::deadline_timer timer2(myservice, boost::posix_time::seconds(2));
	timer2.async_wait(handler2);
	boost::thread thread1(run);
	boost::thread thread2(run);
	thread1.join();
	thread2.join();
    /*
    // client
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string("172.18.10.129"), 2000);
    ip::tcp::socket sock(service);
    try {
        sock.connect(ep);     // async_connect
    }
    catch (boost::system::system_error e)
    {
        std::cout << "connect error: " << e.code() << " - " << e.what() << std::endl;
    }
    */

    // server
    
    io_service service1;
    ip::tcp::endpoint ep1(ip::tcp::v4(), 2001); // listen on 2001

    ip::tcp::acceptor acc(service1, ep1);
    while (true) {
        socket_ptr sock(new ip::tcp::socket(service1));
    //    if(!sock.get()->is_open())
    //        sock.get()->open(ip::tcp::v4());
        try {
            // reuse addr ip::tcp::socket::reuse_address ra(true);
            sock.get()->set_option(ip::tcp::socket::reuse_address(true));

            ip::tcp::socket::receive_buffer_size rbs;
            sock.get()->get_option(rbs);
            std::cout << rbs.value() << std::endl;
        }
        catch (boost::system::system_error &e)
        {
            std::cout << "set_option error: " << e.code() << " - " << e.what() << std::endl;
        }
        
        try {
            acc.accept(*sock);     // async_accept
            boost::thread(&client_session, sock);  // boost::bind(client_session, sock)
        }
        catch (boost::system::system_error &e)
        {
            std::cout << "set_option error: " << e.code() << " - " << e.what() << std::endl;
        }
        
    }

	return 0;
}
