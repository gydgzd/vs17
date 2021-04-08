#include "stdafx.h"
#include "testAsio.h"

std::mutex g_mutex;
std::mutex  g_mutex_conns;           // mutex of s_conns
Mylog g_mylog;

boost::asio::io_service myservice;
//boost::asio::io_service service;    // deprecated ,replaced by io_context
boost::asio::io_context iocontext;
testAsio::testAsio()
{
}


testAsio::~testAsio()
{
}

// start --> async_connect --> on_connect
void AsyncClient::start(ip::tcp::endpoint ep)
{
    std::cout << "connecting to " << ep.address().to_string() << ":" << int(ep.port()) << std::endl;//  << " / " << ep.protocol()
    g_mylog.log("connecting to %s : %d", ep.address().to_string().c_str(), ep.port());
    m_sock_.async_connect(ep, MEM_FN2(on_connect, ep, _1));
}

Client_ptr AsyncClient::start(ip::tcp::endpoint ep, const std::string &message)
{
    Client_ptr pclient(new AsyncClient());
    pclient->start(ep);
    return pclient;
}

void AsyncClient::stop()
{
    if (!m_started_) return;
    m_started_ = false;
    m_sock_.close();
}

void AsyncClient::on_connect(ip::tcp::endpoint ep, const asio_error & err)
{
    if (!err.code())
    {
        m_sock_.set_option(boost::asio::ip::tcp::no_delay(true));
        std::cout << "connected to " << ep.address().to_string() << ":" << int(ep.port()) << std::endl;
        g_mylog.log("connected to %s : %d", ep.address().to_string().c_str(), ep.port());
    }
    else
    {
        std::cout << "on_connect error to " << ep.address().to_string() << ":" << int(ep.port()) << " - " << err.code().value() << " - " << err.what() << std::endl;
        g_mylog.log("on_connect error to %s:%d : %d-%s", ep.address().to_string().c_str(), ep.port(), err.code().value(), err.what());
        stop();
    }
}

void AsyncClient::on_read(const asio_error & err, size_t bytes)
{
    ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
    ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
    if (!err.code()) {
        if (bytes > 0)
        {
            std::string msg(read_buffer_, bytes);
            std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << " received " << bytes << " bytes - " << std::endl;
            g_mylog.log("%s:%d --> %s:%d received %d bytes.", remote_ep.address().to_string().c_str(), remote_ep.port(), local_ep.address().to_string().c_str(), local_ep.port(), bytes);
        }
        do_read();
    }
    else
    {
        std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << "received error: " << err.code().value() << " - " << err.what() << std::endl;
        g_mylog.log("%s:%d --> %s:%d received error: %d-%s.", remote_ep.address().to_string().c_str(), remote_ep.port(), local_ep.address().to_string().c_str(), local_ep.port(), err.code().value(), err.what());
        stop();
    }
}

size_t AsyncClient::is_read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err)
        return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // 我们一个一个读取直到读到回车，不缓存
    return found ? 0 : 1;
}

void AsyncClient::on_write(const asio_error & err, size_t bytes)
{
    ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
    ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
    if (!err.code()) {
        std::string copy(write_buffer_, bytes);
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " wrote " << bytes << " Bytes" << std::endl;
        g_mylog.log("%s:%d --> %s:%d wrote %d bytes.", local_ep.address().to_string().c_str(), local_ep.port(), remote_ep.address().to_string().c_str(), remote_ep.port(), bytes);
        do_read();
    }
    else
    {
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " write error: " << err.code().value() << " - " << err.what() << std::endl;
        g_mylog.log("%s:%d --> %s:%d write error: %d-%s.", local_ep.address().to_string().c_str(), local_ep.port(), remote_ep.address().to_string().c_str(), remote_ep.port(), err.code().value(),  err.what());
        stop();
    }
}

void AsyncClient::do_read() {
    if (!started())
        return;
    //    async_read(sock_, buffer(read_buffer_), MEM_FN2(is_read_complete, _1, _2), MEM_FN3(on_read, boost::ref(sock_), _1, _2));   // sock_ will be error,use boost::ref(sock_)
    m_sock_.async_read_some(buffer(read_buffer_), m_strand.wrap(MEM_FN2(on_read, _1, _2)));   // MEM_FN3(on_read, boost::ref(sock_), _1, _2)
}

void AsyncClient::do_write(const std::string & msg) {
    if (!started())
        return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
//    m_sock_.async_write_some(buffer(write_buffer_, msg.size()), m_strand.wrap(MEM_FN2(on_write, _1, _2)));
    try {
        m_sock_.send(buffer(write_buffer_, msg.size()));
        do_read();
    }
    catch (boost::system::system_error &err)
    {
        ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
        ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << "send error: " << err.code().value() << " - " << err.what() << std::endl;
        g_mylog.log("%s:%d --> %s:%d send error: %d-%s.", local_ep.address().to_string().c_str(), local_ep.port(), remote_ep.address().to_string().c_str(), remote_ep.port(), err.code().value(),  err.what());
    }
    
}

/******** AsyncConnection **********/
AsyncConnection::AsyncConnection() : m_connected(true), m_sock_(iocontext), m_strand(iocontext)
{
    m_read_pos = 0;
    m_write_pos = 0;
}
AsyncConnection::AsyncConnection(Server_ptr pserver) : m_connected(true), m_sock_(iocontext), m_pserver(pserver), m_strand(iocontext)
{
    m_read_pos = 0;
    m_write_pos = 0;
}

bool AsyncConnection::connected()
{
    std::lock_guard<std::mutex> lock(m_connMutex);
    return m_connected;
}

void AsyncConnection::close()
{
    {
        std::lock_guard<std::mutex> lock(m_connMutex);
        m_connected = false;
    }
    std::cout << "connection with " << m_sock_.remote_endpoint().address().to_string() << ":" << m_sock_.remote_endpoint().port() << " closed" << std::endl;
    g_mylog.log("connection with %s:%d closed", m_sock_.remote_endpoint().address().to_string().c_str(), m_sock_.remote_endpoint().port());
    m_sock_.shutdown(ip::tcp::socket::shutdown_both);
    m_sock_.close();
    m_pserver->deleteConn(shared_from_this());
}


void AsyncConnection::on_read(const asio_error & err, size_t bytes)
{
    if (err.code())
    {
        close();
        std::cout << m_remote_ip << ":" << m_remote_port << "-->" << m_local_ip << ":" << m_local_port << "received error: " << err.code() << " - " << err.what() << std::endl;
    }
    if (!connected())
        return;
    if (bytes > 0)
    {
        m_read_pos += bytes;
        std::string msg(read_buffer_, m_read_pos);
        std::cout << m_remote_ip << ":" << m_remote_port << "-->" << m_local_ip << ":" << m_local_port << " received " << bytes << " bytes - " << std::endl;
        int ret = m_pserver->msgGet(msg, mq_recv);
        if (ret > 0)
        {
            memmove(read_buffer_, read_buffer_ + ret, max_msg - ret);
            m_read_pos -= ret;
        }
        while (!mq_recv.empty())
        {
            std::string tmp = mq_recv.front();
            mq_recv.pop();
            m_pserver->msgProcess(shared_from_this(), tmp);
        }
    }
    do_read();
}

void AsyncConnection::do_read() {
    // async_read(client->sock(), buffer(read_buffer_), MEM_FN2(is_read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
    if (!connected())
        return;
    m_sock_.async_read_some(buffer(read_buffer_ + m_read_pos, max_msg - m_read_pos), m_strand.wrap(MEM_FN2(on_read, _1, _2)));
}

void AsyncConnection::do_write(std::string & msg)
{
    if (!connected())
        return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    //    m_sock_.async_write_some(boost::asio::const_buffer(msg.c_str(), msg.size()), MEM_FN2(on_write, _1, _2));
    m_sock_.async_write_some(boost::asio::buffer(msg.c_str(), msg.size()), MEM_FN2(on_write, _1, _2));

    // boost::asio::buffer(msg, msg.size()) not safe, will cause an error - iterator not dereferencable 
    // m_sock_.async_write_some(boost::asio::const_buffer(msg.c_str(), msg.size()), MEM_FN2(on_write, _1, _2)); work OK
}
void AsyncConnection::do_write(const char* msg, unsigned int size)
{
    if (!connected())
        return;
    m_sock_.async_write_some(boost::asio::buffer(msg, size), MEM_FN2(on_write, _1, _2));
}

int AsyncConnection::get_remote_ep(std::string& ip, int& port)
{
    ip::tcp::endpoint remote_ep;
    try {
        remote_ep = m_sock_.remote_endpoint();
    }
    catch (boost::system::system_error &e)
    {
        std::cout << "get_remote_ep error: " << e.code() << " - " << e.what() << std::endl;
        return -1;
    }
    ip = remote_ep.address().to_string();
    m_remote_ip = ip;
    port = remote_ep.port();
    m_remote_port = port;
    return 0;
}

int  AsyncConnection::get_local_ep(std::string& ip, int& port)
{
    ip::tcp::endpoint local_ep;
    try {
        local_ep = m_sock_.local_endpoint();
    }
    catch (boost::system::system_error &e)
    {
        std::cout << "get_local_ep error: " << e.code() << " - " << e.what() << std::endl;
        return -1;
    }
    ip = local_ep.address().to_string();
    m_local_ip = ip;
    port = local_ep.port();
    m_local_port = port;
    return 0;
}

size_t AsyncConnection::is_read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err)
        return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // 我们一个一个读取直到读到回车，不缓存
    return found ? 0 : 1;
}
void AsyncConnection::on_write(const asio_error & err, size_t bytes)
{
    ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
    ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
    if (!err.code()) {
        std::string copy(write_buffer_, bytes);
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " wrote " << bytes << " Bytes" << std::endl;
        g_mylog.log("%s:%d --> %s:%d wrote %d bytes.", local_ep.address().to_string().c_str(), local_ep.port(), remote_ep.address().to_string().c_str(), remote_ep.port(), bytes);
    }
    else
    {
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " write error: " << err.code().value() << " - " << err.what() << std::endl;
        g_mylog.log("%s:%d --> %s:%d  write error: %d-%s.", local_ep.address().to_string().c_str(), local_ep.port(), remote_ep.address().to_string().c_str(), remote_ep.port(), err.code().value(), err.what());
        close();
    }
}

std::vector<Conn_ptr> AsyncServer::s_conns;
std::vector<int> AsyncServer::s_ports;
/******** AsyncServer **********/
//ip::tcp::acceptor AsyncServer::m_acceptor = ip::tcp::acceptor(iocontext, ip::tcp::endpoint(ip::tcp::v4(), 8001));// 放在非静态成员变量中，会导致一个端口多个listen, 无法再次收到连接
// BaseProcess * AsyncServer::m_processor = nullptr;
AsyncServer::AsyncServer(std::string ip, int listenPort, std::string tag) : timer_(iocontext), m_acceptor(iocontext), m_strand(iocontext)
{
    mstr_ip = ip;
    mn_listenPort = listenPort;
    mstr_tag = tag;

    m_processor = m_processor->getProcessor(tag.c_str());
};

int AsyncServer::loadConfig()
{
    // read from file
    std::ifstream infile;
    char filename[] = "ServerConfig.cfg";
    infile.open(filename, std::ios_base::in);
    char logmsg[1024] = "";
    if (!infile)
    {
        sprintf(logmsg, "ERR: Load config from %s failed:(%d) %s", filename, errno, strerror(errno));
        g_mylog.logException(logmsg);
        return -1;
    }
    std::string linebuf;
    std::string param, value;
    std::map<std::string, std::string> map_config;
    while (getline(infile, linebuf))   // !infile.eof()
    {
        if (linebuf.substr(0, 1) == "#" || linebuf.substr(0, 1) == "["
            || linebuf.substr(0, 1) == "\r" || linebuf.substr(0, 1) == "\0" || linebuf.substr(0, 1) == ";")
            continue;
        // remove ' '
        for (unsigned int i = 0; i < linebuf.size(); )
        {
            if (linebuf.at(i) == ' ' || linebuf.at(i) == '\t')
                linebuf.erase(i, 1);
            else
                i++;
        }
        // 
        std::size_t pos = linebuf.find('=');
        if (pos != std::string::npos)
        {
            param = linebuf.substr(0, pos);
            value = linebuf.substr(pos + 1);
            if (param.find("0x") != std::string::npos)
            {
                std::size_t pos = linebuf.find(':');
                std::string ip = value.substr(0, pos);
                int port = atoi(value.substr(pos + 1).c_str());
            }
        }
    }
    infile.close();

    g_mylog.logException("INFO: *********************** BEGIN*********************** ");
    g_mylog.logException("INFO: Load config from file succeed.");
    return 0;
}

Server_ptr AsyncServer::start(std::string ip, int listenPort, std::string tag) {
    Server_ptr server(new AsyncServer(ip, listenPort, tag));

    int ret = server->init();
    if (ret == -1)
        return nullptr;
    server->start();
    std::cout << "Start server on port " << listenPort << std::endl;
    g_mylog.log("Start server on port %d.", listenPort);
    return server;
}
int AsyncServer::init()
{
    try {
        m_acceptor.open(ip::tcp::v4());
        m_acceptor.set_option(socket_base::reuse_address(true));
        m_acceptor.bind(ip::tcp::endpoint(ip::address::from_string(mstr_ip), mn_listenPort)); // ip::address::from_string("127.0.0.1")   // ip::tcp::v4()
        m_acceptor.listen(10000);
        std::lock_guard<std::mutex> lock(m_mutex_started);
        m_started_ = true;
    }
    catch (boost::system::system_error &e)
    {
        std::cout << "Server init error: " << e.code().value() << " - " << e.what() << std::endl;
        g_mylog.log("Server init error: %d-%s", e.code().value(), e.what());
        return -1;
    }
    return 0;
}
// start--> async_accept --> on_accept -->do_read --> start
int AsyncServer::start()
{
    //m_acceptor = ip::tcp::acceptor(iocontext, ip::tcp::endpoint(ip::tcp::v4(), listen_port));
    Conn_ptr conn(new AsyncConnection(shared_from_this()));
    m_acceptor.async_accept(conn->sock(), m_strand.wrap(boost::bind(&AsyncServer::on_accept, shared_from_this(), conn, _1)));
    return 0;
}

void AsyncServer::stop() {
    std::lock_guard<std::mutex> lock(m_mutex_started);
    if (!m_started_)
        return;
    m_started_ = false;
}

void AsyncServer::on_accept(Conn_ptr conn, const asio_error & err)
{
    std::string remote_ip;
    std::string local_ip;
    int remote_port = 0;
    int local_port = 0;

    int ret = conn->get_remote_ep(remote_ip, remote_port);
    if (ret != 0)
    {
        std::cout << "get_remote_ep error!" << std::endl;
        return;
    }
    ret = conn->get_local_ep(local_ip, local_port);
    if (ret != 0)
    {
        std::cout << "get_local_ep error!" << std::endl;
        return;
    }
    if (err.code())
    {
        std::cout << remote_ip << ":" << remote_port << "-->" << local_ip << ":" << local_port << " connecte error, " << err.code() << " - " << err.what() << std::endl;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(g_mutex_conns);
        s_conns.push_back(conn);
    }
    conn->do_read(); //首先，我们等待客户端连接
    std::cout << remote_ip << ":" << remote_port << "-->" << local_ip << ":" << local_port << " connected." << std::endl;
    g_mylog.log("%s:%d --> %s:%d  connected.", remote_ip.c_str(), remote_port, local_ip.c_str(), local_port);

    this->start();
}

int AsyncServer::msgGet(const std::string & buff, std::queue<std::string>& q_recv)
{
    return m_processor->msgGet(buff, q_recv);
}

int AsyncServer::msgProcess(Conn_ptr client, const std::string &buff)
{
    return m_processor->msgProcess(&client, buff);
}

void AsyncServer::deleteConn(Conn_ptr conn)
{
    std::lock_guard<std::mutex> lock(g_mutex_conns);
    array::iterator it = std::find(s_conns.begin(), s_conns.end(), conn);
    if (it != s_conns.end())
        s_conns.erase(it);
}







void handler1(const boost::system::error_code &ec)
{
    for (int i = 0; i < 5; i++)
        std::cout << "handle 1: " << i << "s." << std::endl;
}

void handler2(const boost::system::error_code &ec)
{
    for (int i = 0; i < 5; i++)
        std::cout << "handle 2: " << i << "s." << std::endl;
}


void run()
{
    //    std::lock_guard <std::mutex> lock(g_mutex);
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
int testConferenceDistributor(Client_ptr shrd_client);
int testUserManager(Client_ptr shrd_client);
int testDeviceManager(Client_ptr shrd_client);
int testAuthManager(Client_ptr shrd_client);
int testConferenceManager(Client_ptr shrd_client);
int testUpgradeManager(Client_ptr shrd_client);
int testConfigManager(Client_ptr shrd_client);

int testAsio::test()
{
    boost::asio::io_service::strand strand(myservice);
    boost::asio::deadline_timer timer1(myservice, boost::posix_time::seconds(3));
    timer1.async_wait(strand.wrap(handler1));
    boost::asio::deadline_timer timer2(myservice, boost::posix_time::seconds(3));
    timer2.async_wait(strand.wrap(handler2));

    boost::thread thread1(run);
    boost::thread thread2(run);
    thread1.join();
    thread2.join();
    //10.1.4.75:33300
    ip::tcp::endpoint ep_dis(ip::address::from_string("172.18.10.129"), 8001);
    Client_ptr client_dis = AsyncClient::start(ep_dis, "");
    std::thread th_dis   { testConferenceDistributor, client_dis };
    /*
    ip::tcp::endpoint ep_device(ip::address::from_string("172.18.10.129"), 8002);
    Client_ptr client_device = AsyncClient::start(ep_device, "");
    std::thread th_device{ testDeviceManager, client_device };

    ip::tcp::endpoint ep_user(ip::address::from_string("172.18.10.129"), 8003);
    Client_ptr client_user = AsyncClient::start(ep_user, "");
    std::thread th_user{ testUserManager, client_user };

    ip::tcp::endpoint ep_auth(ip::address::from_string("172.18.10.129"), 8004);
    Client_ptr client_auth = AsyncClient::start(ep_auth, "");
    std::thread th_auth{ testAuthManager, client_auth };

    ip::tcp::endpoint ep_confe(ip::address::from_string("172.18.10.129"), 8005);
    Client_ptr client_confe = AsyncClient::start(ep_confe, "");
    std::thread th_confe{ testConferenceManager, client_confe };

    ip::tcp::endpoint ep_upgrade(ip::address::from_string("172.18.10.129"), 8006);
    Client_ptr client_upgrade = AsyncClient::start(ep_upgrade, "");
    std::thread th_upgrade{ testUpgradeManager, client_upgrade };

    ip::tcp::endpoint ep_config(ip::address::from_string("172.18.10.129"), 8007);
    Client_ptr client_config = AsyncClient::start(ep_config, "");
    std::thread th_config{ testConfigManager, client_config };
    */

    iocontext.run();

    th_dis.join();
 /*   th_device.join();
    th_user.join();
    th_auth.join();
    th_confe.join();
    th_upgrade.join();
    th_config.join();
    */
    return 0;
}
// 0x6012
int testConferenceDistributor(Client_ptr shrd_client)
{
    char buffer[4096] = {};
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6012);
    ConferenceMngHead *confHead = (ConferenceMngHead *)(buffer + sizeof(Overload));//
    confHead->version[0] = 0x01;
    confHead->version[1] = 0x02;
    confHead->version[2] = 0x08;
    confHead->version[3] = 0x05;
    int id = 0;
    char* json = (char*)(buffer  + sizeof(ConferenceMngHead))+ sizeof(Overload);//
    int len = 0;
    std::string data = "";
    std::string msg = "";
    ////0x0600
    confHead->cmd = htons(0x0600);
    confHead->id = htonll(id++);
    confHead->total = htons(1);
    confHead->index = htons(1);
    msg = "{\
        \"userId\": \"7dd21363732911ea8362a4bf01303dd7\",\
        \"uuid\": \"5021846b3db911eba1e2a4bf01303dd7\",\
        \"content\": {\
        \"is_auto\": 1,\
        \"meet_type\": 0,\
        \"is_encry\": 0,\
        \"meet_name\": \"创建一个一周的会\",\
        \"brige_master_ip\": \"\",\
        \"master_no\": \"100-321-2750-0-0\",\
        \"area_number\": 11,\
        \"devs\": [\
    {\
        \"dev_no\": \"00100-00321-01900-00000-00000\", \
            \"devName\": \"1900\",\
            \"devType\": 5,\
            \"hwType\": 0\
    },\
    {\
        \"dev_no\": \"00100-00321-02750-00000-00000\",\
        \"devName\": \"2750\",\
            \"devType\": 5,\
            \"hwType\": 0\
    }\
    ],\
    \"config_list\": [\
        {\
            \"output_mode_dvi\": 1,\
                \"display_source_dvi\": 0,\
                \"output_mode_hdmi\": 4,\
                \"display_source_hdmi\": 4501\
        }\
    ]\
}}";
    len = sizeof(ConferenceMngHead) + msg.length();
    overload->len = htons(len + sizeof(Overload));
    confHead->len = htonl(len);
    memcpy(json, msg.c_str(), msg.length());
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    std::cout << "write id:" << id - 1 << std::endl;
    ///////0x0601
    //std::this_thread::sleep_for(chrono::seconds(2));
    //confHead->cmdType = htons(0x0100);
    confHead->cmd = htons(0x0601);
    confHead->id = htonll(id++);
    confHead->total = htons(1);
    confHead->index = htons(1);
    msg = "{\
        \"userId\": \"7dd21363732911ea8362a4bf01303dd7\",\
        \"uuid\": \"5021846b3db911eba1e2a4bf01303dd7\",\
        \"content\": {\
        \"is_auto\": 1,\
        \"meet_type\": 0,\
        \"is_encry\": 0,\
        \"meet_name\": \"创建一个一周的\",\
        \"brige_master_ip\": \"\",\
        \"master_no\": \"100-321-2750-0-0\",\
        \"area_number\": 11,\
        \"devs\": [\
    {\
        \"dev_no\": \"00100-00321-01900-00000-00000\", \
            \"devName\": \"1900\",\
            \"devType\": 5,\
            \"hwType\": 0\
    },\
    {\
        \"dev_no\": \"00100-00321-02750-00000-00000\",\
        \"devName\": \"2750\",\
            \"devType\": 5,\
            \"hwType\": 0\
    }\
    ],\
    \"config_list\": [\
        {\
            \"output_mode_dvi\": 1,\
                \"display_source_dvi\": 0,\
                \"output_mode_hdmi\": 4,\
                \"display_source_hdmi\": 4501\
        }\
    ]\
}}";
    len = sizeof(ConferenceMngHead) + msg.length();
    overload->len = htons(len + sizeof(Overload));
    confHead->len = htonl(len);
    memcpy(json, msg.c_str(), msg.length());
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    std::cout << "write id:" << id - 1 << std::endl;
  
    std::this_thread::sleep_for(chrono::seconds(3));
    return 0;
}
// 0x6020
int testDeviceManager(Client_ptr shrd_client)
{
    char buffer[4096] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6020);
    std::string msg = "";
    int taskNo = 0;
    ////101  zz
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);

    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"name\":\"gg\",\"pageno\":2,\"pagecount\":3}";
    int len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    std::string data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    /// 102
    std::this_thread::sleep_for(chrono::seconds(1));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(41);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(102);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"accloudid\":\"3321\",\"name\":\"acss\",\"region\":\"111\",\"glip\":\"22\",\"glport\":\"33\",\"sn\":\"\",\"logid\":\"10086\",\"mac\":\"60:F2:FF:33:24:22\",\
        \"ether\":\"\",\"vlan\":\"2\",\"baksn\":\"\",\"baklogid\":\"\",\"bakmac\":\"60:F2:FF:33:24:21\",\"bakhbtimeout\":\"30\",\"level\":\"3\",\"numfix\":\"00101\",\"logfix\":\
        \"00000\",\"jfxxdz\":\"北京\",\"jgh\":\"33\",\"zbjd\":\"\".\"zbwd\":\"\",\"fzr\":\"\",\"fzrdh\":\"\",\"khlxr\":\"\",\"khlxrdh\":\"\",\"yysm\":\"\",\"yyslxr\":\"\",\
        \"yyslxrdh\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"topo\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);

    return 0;

}
// 0x6021
int testUserManager(Client_ptr shrd_client)
{
    char buffer[4096] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6021);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    int taskNo = 0;
    std::string data = "";
    std::string msg = "";
    ////101
    /*   */
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);
    msg = "{   \
                \"user\":\"zhangsan1\",\
                \"passwd\" : \"e10adc3949ba59abbe56e057f20f883e\"\
                }";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////121
    //std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(121);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    char msg1[] = "{\
        \"loginName\":\"zhangsan1\",\
        \"access_token\" : \"e10adc3949ba59abbe56e057f20f883e\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg1) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg1);
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg1, strlen(msg1));
    memcpy(json + strlen(msg1), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    
    return 0;
}
// 0x6022
int testAuthManager(Client_ptr shrd_client)
{
    char buffer[4096] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6022);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    int taskNo = 0;
    std::string msg = "";
    std::string data = "";
    ////01
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0010);
    deviceHead->cmd = htonl(1);
    deviceHead->ret = htons(0);
    msg = "{\
        \"user_id\": \"shenjiyuan\",\
        \"query\": {\
        \"begin_time\": \"2020-12-14 14:52:11\",\
        \"end_time\": \"2020-12-21 14:52:11\",\
        \"user_id\": \"zhangsan\",\
        \"moudle_type\": \"1\",\
        \"user_id\": \"shenjiyuan\",\
        \"event_type\": \"101\",	\
        \"object_id\": \"\",\
        \"object_type\": \"\",\
        \"content\": \"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////02
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0010);
    deviceHead->cmd = htonl(2);
    deviceHead->ret = htons(0);
    msg = "{\
        \"user_id\": \"shenjiyuan\",\
        \"query\": {\
        \"begin_time\": \"2020-12-14 14:52:11\",\
        \"end_time\": \"2020-12-21 14:52:11\",\
        \"moudle_type\": \"1\",\
        \"user_id\": \"shenjiyuan\",\
        \"event_type\": \"101\",	\
        \"object_id\": \"\",\
        \"object_type\": \"\",\
        \"content\": \"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}
// 0x6023
int testConferenceManager(Client_ptr shrd_client)
{
    char buffer[4096] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6023);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    int taskNo = 0;
    std::string data = "";
    std::string msg = "";
    ////601
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0003);
    deviceHead->cmd = htonl(601);
    deviceHead->ret = htons(0);
    msg = "{\"startTime\":\"\",\
        \"endTime\":\"\",\
        \"creatorId\":\"\",\
        \"reqId \":\"\",\
        \"status\":\"\",\
        \"pageNum\":\"\",\
        \"pageSize\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////602
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0003);
    deviceHead->cmd = htonl(602);
    deviceHead->ret = htons(0);
    msg = "{\"startTime\":\"\",\
        \"endTime\":\"\",\
        \"creatorId\":\"\",\
        \"reqId\":\"\",\
        \"status\":\"2\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    
    return 0;
}
// 0x6024
int testUpgradeManager(Client_ptr shrd_client)
{
    char buffer[4096] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6024);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    int taskNo = 0;
    std::string data = "";
    std::string msg = "";
    ////703
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(703);
    deviceHead->ret = htons(0);
    msg = "{\"file_name\":\"\",\
        \"path\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\",\
        \"datetime\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////704
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(704);
    deviceHead->ret = htons(0);
    msg = "{\
        \"product\":\"\",\
        \"version\":\"\",\
        \"datetime\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    
    return 0;
}
// 0x6025
int testConfigManager(Client_ptr shrd_client)
{
    char buffer[6553] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6025);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    int taskNo = 0;
    std::string data = "";
    std::string msg = "";
    ////101
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(taskNo++);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0005);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);
    msg = "{\"file_name\":\"\",\
        \"path\":\"E:\\work\\CS\\CMSServer\\BoostAsio\\Debug\",\
        \"product\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"version\":\"1.0.21.3\",\
        \"reqId\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"path\":\"D:\\work\\CS\\CMSServer\\BoostAsio\\Debug\",\
        \"product\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"version\":\"1.0.21.31.0.21.31.0.21.31.0.21.31.0.21.31.0.21.31.0.21.3\",\
        \"reqId\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"path\":\"C:\\work\\CS\\CMSServer\\BoostAsio\\Debug\",\
        \"product\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"version\":\"1.0.21.31.0.21.31.0.21.31.0.21.1.0.21.31.0.21.31.0.21.31.0.21.331.0.21.31.0.21.3\",\
        \"reqId\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"path\":\"E:\\work\\CS\\CMSServer\\BoostAsio\\Debug\",\
        \"product\":\"qiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanliqiushichanpingxianhuiyiguanli\",\
        \"version\":\"1.0.21.3\",\
        \"datetime\":\"2020-2010-2020:20202020202022020-2010-2020:20202020202022020-2010-2020:20202020202022020-2010-2020:2020202020202\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htonl(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}