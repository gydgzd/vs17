#include "stdafx.h"
#include "testAsio.h"

std::mutex g_mutex;
testAsio::testAsio()
{
}


testAsio::~testAsio()
{
}

// start --> async_connect --> on_connect
void AsyncClient::start(ip::tcp::endpoint ep)
{
    std::cout << "connecting to "  << ep.address().to_string() << ":" << int(ep.port())<< std::endl;//  << " / " << ep.protocol()
    sock_.async_connect(ep, MEM_FN2(on_connect, ep, _1));
}

AsyncClient::ptr AsyncClient::start(ip::tcp::endpoint ep, const std::string & message)
{
    ptr new_(new AsyncClient(message));
    new_->start(ep);
    return new_;
}

void AsyncClient::stop()
{
    if (!started_) return;
    started_ = false;
    sock_.close();
}

void AsyncClient::on_connect(ip::tcp::endpoint ep, const asio_error & err)
{
    if (!err.code())
    {
        std::cout << "connected to: " << ep.address().to_string() << ":" << int(ep.port()) << std::endl;
        do_write(message_ + "\n");
    }
    else
    {
        std::cout << "on_connect error to " << ep.address().to_string() << ":" << int(ep.port()) << " - " << err.code() << " - " << err.what() << std::endl;
        stop();
    }
}

void AsyncClient::on_read(ip::tcp::socket &sock, const asio_error & err, size_t bytes)
{
    if (!err.code()) {
        std::string copy(read_buffer_, bytes - 1);
        std::cout << "server echoed our " << sock.remote_endpoint().address().to_string() << message_ << ": " << (copy == message_ ? "OK" : "FAIL") << std::endl;
    }
    else
    {
        std::cout << "on_read error: " << sock.remote_endpoint().address().to_string() << err.code() << " - " << err.what() << std::endl;
        stop();
    }
   
}

void AsyncClient::on_write( const asio_error & err, size_t bytes)
{
    if (!err.code()) {
        do_read(sock_);
    }
    else
    {
        std::cout << "on_write error: " << err.code() << " - " << err.what() << std::endl;
        stop();
    }
}

void AsyncClient::do_read(ip::tcp::socket &sock) {
    async_read(sock_, buffer(read_buffer_), MEM_FN2(read_complete,  _1, _2), MEM_FN3(on_read, boost::ref(sock_), _1, _2));   // sock_   will be error
}

void AsyncClient::do_write(const std::string & msg) {
    if (!started()) return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    sock_.async_write_some(buffer(write_buffer_, msg.size()), MEM_FN2(on_write, _1, _2));
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
boost::asio::io_service service;

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

    ip::tcp::endpoint ep(ip::address::from_string("10.1.4.169"), 8000);
    auto shrd_client = AsyncClient::start(ep, "hi");
    service.run();

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
