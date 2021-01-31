#pragma once

#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <iostream>
#include <string>
#include "MsgDefine.h"
using namespace boost::asio;

class testAsio
{
public:
    testAsio();
    virtual ~testAsio();


    int test();
};

#define MEM_FN(x)         boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)      boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)    boost::bind(&self_type::x, shared_from_this(),y,z)
#define MEM_FN3(x,w,y,z)  boost::bind(&self_type::x, shared_from_this(),w,y,z)
//extern boost::asio::io_service service;
extern boost::asio::io_context iocontext;
typedef boost::system::system_error asio_error;
class AsyncClient : public boost::enable_shared_from_this<AsyncClient>, boost::noncopyable
{

public:
    typedef boost::shared_ptr<AsyncClient> ptr;
    static ptr start(ip::tcp::endpoint ep, const std::string &message);
    void stop();
    bool started() { return started_; }

    size_t read_complete(const boost::system::error_code & err, size_t bytes);

    void do_read();
    void do_write(const std::string & msg);

private:
    typedef AsyncClient self_type;
    AsyncClient() : sock_(iocontext), started_(true), message_("") {}
    void start(ip::tcp::endpoint ep);
    void on_connect(ip::tcp::endpoint ep, const asio_error & err);
    void on_read(const asio_error & err, size_t bytes);
    void on_write(const asio_error & err, size_t bytes);

    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    std::string message_;
};



class AsyncServer : public boost::enable_shared_from_this<AsyncServer>, boost::noncopyable
{
public:
    typedef boost::shared_ptr<AsyncServer> client_ptr;
    typedef std::vector<client_ptr> array;
    static array clients;
    typedef AsyncServer self_type;
    typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
public:
    static client_ptr start();
    void stop();
    bool started() const { return started_; }


    void do_read(client_ptr client);
    ip::tcp::socket & sock() { return sock_; }

    size_t is_read_complete(const boost::system::error_code & err, size_t bytes);

private:
    AsyncServer() : sock_(iocontext), timer_(iocontext) {};
    void start(int listen_port);
    void on_accept(client_ptr client, const asio_error & err);
    void on_read(client_ptr client, const asio_error & err, size_t bytes);
    void on_write(const asio_error & err, size_t bytes);

    ip::tcp::socket sock_;

    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;

    deadline_timer timer_;

    static ip::tcp::acceptor m_acceptor;
    void msgProcess(client_ptr client, char *buff);
};
