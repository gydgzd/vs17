#pragma once

#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <mutex>
#include "MsgDefine.h"
#include "DataProcess.h"
#include "Mylog.h"
using namespace boost::asio;
extern Mylog g_mylog;
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
class AsyncClient;
class AsyncConnection;
class AsyncServer;
typedef boost::shared_ptr<AsyncClient> Client_ptr;
typedef boost::shared_ptr<AsyncConnection> Conn_ptr;
typedef boost::shared_ptr<AsyncServer>     Server_ptr;
extern std::mutex  g_mutex_conns;         // mutex of s_conns
                                          //extern boost::asio::io_service service;
extern boost::asio::io_context iocontext;
typedef boost::system::system_error asio_error;
typedef boost::shared_ptr<ip::tcp::socket> Socket_ptr;

class AsyncClient : public boost::enable_shared_from_this<AsyncClient>, boost::noncopyable
{

public:

    static Client_ptr start(std::string ip, int port);
    void stop();
    bool connected() { return m_connected_; }

    size_t is_read_complete(const boost::system::error_code & err, size_t bytes);

    void do_read();
    void do_write(const std::string & msg);

private:
    typedef AsyncClient self_type;
    AsyncClient() : m_sock_(iocontext), m_connected_(true), message_(""), m_strand(iocontext) { };

    void start(ip::tcp::endpoint ep);
    void on_connect(ip::tcp::endpoint ep, const asio_error & err);
    void on_read(const asio_error & err, size_t bytes);
    void on_write(const asio_error & err, size_t bytes);

    std::string mstr_remote_ip;       // remote ip
    std::string mstr_local_ip;        // local ip
    int         m_remote_port;        // remote port
    int         m_local_port;         // local port

    ip::tcp::socket m_sock_;
    enum { max_msg = 65536 };
    char m_read_buffer_[max_msg];
    char m_write_buffer_[max_msg];
    bool m_connected_;
    std::string message_;
    boost::asio::io_context::strand m_strand;
};

class AsyncServer : public boost::enable_shared_from_this<AsyncServer>, boost::noncopyable
{
public:
    typedef AsyncServer self_type;
    typedef std::vector<Conn_ptr> array;
    static array s_conns;
    static std::vector<int> s_ports;

public:
    static Server_ptr start(std::string ip, int listenPort, std::string tag);
    void stop();
    bool started() { std::lock_guard<std::mutex> lock(m_mutex_started); return m_started_; }    // server status
    int msgGet(const std::string &buff, std::queue<std::string>& q_recv);
    int msgProcess(Conn_ptr client, const std::string &buff);           //strategy mode

    void deleteConn(Conn_ptr conn);                                     // remove a connection
private:
    AsyncServer(std::string ip, int listenPort, std::string tag);
    int loadConfig();
    int init();
    int start();
    void on_accept(Conn_ptr conn, const asio_error & err);              // add a connection
    std::string mstr_tag;
    std::string mstr_ip;
    int mn_listenPort;
    bool m_started_;
    std::mutex m_mutex_started;                                         // mutex of m_started
    deadline_timer timer_;
    ip::tcp::acceptor m_acceptor;
    BaseProcess *m_processor;
    boost::asio::io_context::strand m_strand;
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
    void do_write(std::string & msg);
    void do_write(const char* msg, unsigned int size);

    int get_remote_ep(std::string& ip, int& port); // to get remote ip:port
    int get_local_ep(std::string& ip, int& port);  // to get local  ip:port

    size_t is_read_complete(const boost::system::error_code & err, size_t bytes);
    bool connected();
    void close();
private:
    void on_read(const asio_error & err, size_t bytes);
    void on_write(const asio_error & err, size_t bytes);
    bool m_connected;                 // connection status: 0:closed, 1:connected
    std::mutex m_connMutex;           // mutex of m_connected

    enum { max_msg = 65536 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    int m_read_pos;
    int m_write_pos;
    std::queue<std::string> mq_recv;
    std::queue<std::string> mq_send;
   
    std::string mstr_remote_ip;       // remote ip
    std::string mstr_local_ip;        // local ip
    int         m_remote_port;        // remote port
    int         m_local_port;         // local port
    ip::tcp::socket m_sock_;
    boost::asio::io_context::strand m_strand;
    Server_ptr m_pserver;
};