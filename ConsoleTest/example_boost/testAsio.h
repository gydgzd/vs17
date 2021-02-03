#pragma once

#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <iostream>
#include <string>
#include <mutex>
#include "MsgDefine.h"
#include "DataProcess.h"
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
extern std::mutex  g_mutex_conns;         // mutex of s_conns
                                          //extern boost::asio::io_service service;
extern boost::asio::io_context iocontext;
typedef boost::system::system_error asio_error;
class AsyncClient : public boost::enable_shared_from_this<AsyncClient>, boost::noncopyable
{

public:
    typedef boost::shared_ptr<AsyncClient> ptr;
    static ptr start(ip::tcp::endpoint ep, const std::string &message);
    void stop();
    bool started() { return m_started_; }

    size_t is_read_complete(const boost::system::error_code & err, size_t bytes);

    void do_read();
    void do_write(const std::string & msg);

private:
    typedef AsyncClient self_type;
    AsyncClient() : sock_(iocontext), m_started_(true), message_(""), m_strand(iocontext){}
    void start(ip::tcp::endpoint ep);
    void on_connect(ip::tcp::endpoint ep, const asio_error & err);
    void on_read(const asio_error & err, size_t bytes);
    void on_write(const asio_error & err, size_t bytes);

    ip::tcp::socket sock_;
    boost::asio::io_context::strand m_strand;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool m_started_;
    std::string message_;
};


class AsyncConnection;
class AsyncServer;
typedef boost::shared_ptr<ip::tcp::socket> Socket_ptr;
typedef boost::shared_ptr<AsyncConnection> Conn_ptr;
typedef boost::shared_ptr<AsyncServer>     Server_ptr;

class AsyncServer : public boost::enable_shared_from_this<AsyncServer>, boost::noncopyable
{
public:
    typedef AsyncServer self_type;
    typedef std::vector<Conn_ptr> array;
    static array s_conns;
    static std::vector<int> s_ports;

public:
    static Server_ptr start(int listenPort);
    void stop();
    bool started() { std::lock_guard<std::mutex> lock(m_mutex_started); return m_started_; }             // server status

    void deleteConn(Conn_ptr conn);                         // remove a connection
private:
    AsyncServer(int listenPort);
    int init();
    int start();
    void on_accept(Conn_ptr conn, const asio_error & err);   // add a connection

    int mn_listenPort;
    boost::asio::io_context::strand m_strand;
    bool m_started_;
    std::mutex m_mutex_started;                              // mutex of m_started
    deadline_timer timer_;
    ip::tcp::acceptor m_acceptor;

};

//split connection from server
class AsyncConnection : public boost::enable_shared_from_this<AsyncConnection>
{
public:
    typedef AsyncConnection self_type;
    AsyncConnection();
    AsyncConnection(Server_ptr pserver);
    ip::tcp::socket & sock() { return m_sock_; }

    void do_read();

    size_t is_read_complete(const boost::system::error_code & err, size_t bytes);
    bool connected();
    void close();
private:
    void on_read(const asio_error & err, size_t bytes);
    void on_write(const asio_error & err, size_t bytes);
    virtual void msgProcess(Conn_ptr client, char *buff); //strategy mode

    bool m_connected;                      // connection status: 0:closed, 1:connected
    std::mutex m_connMutex;                // mutex of m_connected

    ip::tcp::socket m_sock_;
    boost::asio::io_context::strand m_strand;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    Server_ptr m_pserver;
};