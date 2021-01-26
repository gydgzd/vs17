#pragma once

#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <iostream>
#include <string>
using namespace boost::asio;

class testAsio
{
public:
	testAsio();
	virtual ~testAsio();


	int test();
};

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)
#define MEM_FN3(x,w,y,z)  boost::bind(&self_type::x, shared_from_this(),w,y,z)
extern boost::asio::io_service service;

class AsyncClient : public boost::enable_shared_from_this<AsyncClient>, boost::noncopyable
{

public:
    typedef boost::system::system_error asio_error;
    typedef boost::shared_ptr<AsyncClient> ptr;
    static ptr start(ip::tcp::endpoint ep, const std::string &message);
    void stop();
    bool started() { return started_; }
    void on_connect(ip::tcp::endpoint ep, const asio_error & err);

    void on_read(ip::tcp::socket &sock, const asio_error & err, size_t bytes);
    void on_write(  const asio_error & err, size_t bytes);
    void do_read(ip::tcp::socket &sock);
    void do_write(const std::string & msg);
    size_t read_complete(  const boost::system::error_code & err, size_t bytes) {
        // 和TCP客户端中的类似
        std::cout << "read_complete" << std::endl;
        return 0;
    }
private:
    AsyncClient(const std::string & message) : sock_(service), started_(true), message_(message) {}
    typedef AsyncClient self_type;
    
    void start(ip::tcp::endpoint ep);
    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    std::string message_;
};
