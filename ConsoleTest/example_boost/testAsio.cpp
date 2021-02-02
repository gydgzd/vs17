#include "stdafx.h"
#include "testAsio.h"

std::mutex g_mutex;
std::mutex  g_mutex_conns;           // mutex of s_conns

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
    sock_.async_connect(ep, MEM_FN2(on_connect, ep, _1));
}

AsyncClient::ptr AsyncClient::start(ip::tcp::endpoint ep, const std::string &message)
{
    ptr new_(new AsyncClient());
    new_->start(ep);
    return new_;
}

void AsyncClient::stop()
{
    if (!m_started_) return;
    m_started_ = false;
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

void AsyncClient::on_read(const asio_error & err, size_t bytes)
{
    if (!err.code()) {
        if (bytes > 0)
        {
            std::string copy(read_buffer_, bytes);
            std::cout << "received:  " << sock_.remote_endpoint().address().to_string() << "  " << copy << std::endl;
        }
        do_read();
    }
    else
    {
        std::cout << "on_read error: " << sock_.remote_endpoint().address().to_string() << err.code() << " - " << err.what() << std::endl;
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
    if (!err.code()) {
        do_read();
    }
    else
    {
        std::cout << "on_write error: " << err.code() << " - " << err.what() << std::endl;
        stop();
    }
}

void AsyncClient::do_read() {
    if (!started())
        return;
    //    async_read(sock_, buffer(read_buffer_), MEM_FN2(is_read_complete, _1, _2), MEM_FN3(on_read, boost::ref(sock_), _1, _2));   // sock_ will be error,use boost::ref(sock_)
    sock_.async_read_some(buffer(read_buffer_), MEM_FN2(on_read, _1, _2));   // MEM_FN3(on_read, boost::ref(sock_), _1, _2)
}

void AsyncClient::do_write(const std::string & msg) {
    if (!started())
        return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    sock_.async_write_some(buffer(write_buffer_, msg.size()), MEM_FN2(on_write, _1, _2));
}

/******** AsyncConnection **********/
AsyncConnection::AsyncConnection() : m_sock_(iocontext), m_connected(true)
{

}
AsyncConnection::AsyncConnection(Server_ptr pserver) : m_sock_(iocontext), m_connected(true), m_pserver(pserver)
{

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
    m_sock_.shutdown(ip::tcp::socket::shutdown_both);
    m_sock_.close();
    m_pserver->deleteConn(shared_from_this());
    std::cout << "connection with " << m_sock_.remote_endpoint().address().to_string() << ":" << m_sock_.remote_endpoint().port() << " closed" << std::endl;
}


void AsyncConnection::on_read(const asio_error & err, size_t bytes) {
    if (err.code())
    {
        close();
        std::cout << "on_read error: " << err.code() << " - " << err.what() << std::endl;
    }
    if (!connected())
        return;
    if (bytes > 0)
    {
        std::string copy(read_buffer_, bytes);
        std::cout << "received:  " << bytes << " - " << m_sock_.remote_endpoint().address().to_string() << ":" << m_sock_.remote_endpoint().port() << copy << std::endl;
        msgProcess(shared_from_this(), read_buffer_);
    }
    do_read();
    std::string msg(read_buffer_, bytes);
    std::cout << read_buffer_ << std::endl;
}

void AsyncConnection::do_read() {
    // async_read(client->sock(), buffer(read_buffer_), MEM_FN2(is_read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
    if (!connected())
        return;
    m_sock_.async_read_some(buffer(read_buffer_), MEM_FN2(on_read, _1, _2));
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
    if (!err.code()) {
        std::string copy(write_buffer_, bytes);
        std::cout << "write to " << m_sock_.remote_endpoint().address().to_string() << ":" << m_sock_.remote_endpoint().port() << " : " << bytes << " Bytes - " << copy << std::endl;
    }
    else
    {
        std::cout << "on_write error: " << err.code() << " - " << err.what() << std::endl;
        close();
    }
}
void AsyncConnection::msgProcess(Conn_ptr client, char * buff)
{
    Overload *overload = (Overload *)buff;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6020)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(buff + sizeof(Overload));
        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd << std::endl;
        memset(write_buffer_, 0, 1024);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);
        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        int len = 0;
        std::string data;
        int endtag = 0xeeeeeeee;
        switch (deviceHead->cmd)
        {
        case 101:
        {
            char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
            char msg[] = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
            \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
            \"phone\" : \"13599999999\",\
            \"role\" : \"安全保密管理员\",\
            \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
            \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
            \
            },\
           \"ret\": 0,\
           \"msg\" : \"登录成功\"}";
            int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
            write->len = htons(len);
            writeDevHead->len = (short)strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->m_sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        case 121:
        {

            char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
            char msg[] = "{\
                \"ret\":0,\
                \"msg\" : \"用户退出成功\",\
                }  ";
            int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
            write->len = htons(len);
            writeDevHead->len = (short)strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->m_sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        case 131:
        {

            char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
            char msg[] = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
            \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
            \"phone\" : \"13599999999\",\
            \"role\" : \"安全保密管理员\",\
            \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
            \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
            \
            },\
           \"ret\": 0,\
           \"msg\" : \"创建成功\"}";
            int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
            write->len = htons(len);
            writeDevHead->len = (short)strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->m_sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        case 141:
        {

            char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
            char msg[] = "{\
                \"ret\": 0,\
                \"msg\" : \"修改成功\"\
                }";
            int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
            write->len = htons(len);
            writeDevHead->len = (short)strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->m_sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        case 151:
        {
            char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
            char msg[] = "{\
                \"ret\": 0,\
                \"msg\" : \"删除成功\"\
                }";
            int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
            write->len = htons(len);
            writeDevHead->len = (short)strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->m_sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        case 161:
        {
            char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
            char msg[] = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
            \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
            \"phone\" : \"13599999999\",\
            \"role\" : \"安全保密管理员\",\
            \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
            \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
            \
            },\
           \"ret\": 0,\
           \"msg\" : \"查询成功\"}";
            int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
            write->len = htons(len);
            writeDevHead->len = (short)strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->m_sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        }

    }
}

std::vector<Conn_ptr> AsyncServer::s_conns;
std::vector<int> AsyncServer::s_ports;
/******** AsyncServer **********/
//ip::tcp::acceptor AsyncServer::m_acceptor = ip::tcp::acceptor(iocontext, ip::tcp::endpoint(ip::tcp::v4(), 8001));// 放在非静态成员变量中，会导致一个端口多个listen, 无法再次收到连接
AsyncServer::AsyncServer(int listenPort) : timer_(iocontext), m_acceptor(iocontext)
{
    mn_listenPort = listenPort;
};
Server_ptr AsyncServer::start(int listenPort) {
    Server_ptr server(new AsyncServer(listenPort));
    int ret = server->init();
    if (ret == -1)
        return nullptr;
    server->start();
    return server;
}
int AsyncServer::init()
{
    try {
        m_acceptor.open(ip::tcp::v4());
        m_acceptor.set_option(socket_base::reuse_address(true));
        m_acceptor.bind(ip::tcp::endpoint(ip::address::from_string("0.0.0.0"), mn_listenPort)); // ip::address::from_string("127.0.0.1")   // ip::tcp::v4()
        m_acceptor.listen(10000);
        std::lock_guard<std::mutex> lock(m_mutex_started);
        m_started_ = true;
    }
    catch (boost::system::system_error &e)
    {
        std::cout << "error: " << e.code() << " - " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
// start--> async_accept --> on_accept -->do_read --> start
int AsyncServer::start()
{
    //m_acceptor = ip::tcp::acceptor(iocontext, ip::tcp::endpoint(ip::tcp::v4(), listen_port));
    Conn_ptr conn(new AsyncConnection(shared_from_this()));
    m_acceptor.async_accept(conn->sock(), boost::bind(&AsyncServer::on_accept, shared_from_this(), conn, _1));
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
    ip::tcp::endpoint remote_ep = conn->sock().remote_endpoint();
    if (err.code())
    {
        std::cout << remote_ep.address().to_string() << " : " << remote_ep.port() << " connecte error, " << err.code() << " - " << err.what() << std::endl;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(g_mutex_conns);
        s_conns.push_back(conn);
    }
    conn->do_read(); //首先，我们等待客户端连接
    std::cout << remote_ep.address().to_string() << " : " << remote_ep.port() << " connected." << "local port:" << conn->sock().local_endpoint().port() << std::endl;
    this->start();
}

void AsyncServer::deleteConn(Conn_ptr conn)
{
    std::lock_guard<std::mutex> lock(g_mutex_conns);
    array::iterator it = std::find(s_conns.begin(), s_conns.end(), conn);
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
    boost::asio::deadline_timer timer1(myservice, boost::posix_time::seconds(6));
    timer1.async_wait(handler1);
    boost::asio::deadline_timer timer2(myservice, boost::posix_time::seconds(6));
    timer2.async_wait(handler2);

    boost::thread thread1(run);
    boost::thread thread2(run);
    thread1.join();
    thread2.join();
    //
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
    auto shrd_client = AsyncClient::start(ep, "");
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6020);
    ////101
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);

    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    char msg[] = "{   \
                \"user\":\"zhangsan1\",\
                \"passwd\" : , \"e10adc3949ba59abbe56e057f20f883e\"\
                }";
    int len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    std::string data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////121
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
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
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg1, strlen(msg1));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////131
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(131);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    char msg2[] = "{\
        \"user\":\"zhangsan1\",\
        \"passwd\" : \"e10adc3949ba59abbe56e057f20f883e\",\
        \"name\" : \"王方军\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"安全保密管理员\",\
        \"dept\" : \"xx部xx办公室\",\
        \"secureLevel\" : \"0\",\
        \"address\" : \"xxx路xx号\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg2) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg2);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg2, strlen(msg2));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////141
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(141);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    char msg3[] = "{\
        \"user\":\"zhangsan1\",\
        \"passwd\" : \"e10adc3949ba59abbe56e057f20f883e\",\
        \"name\" : \"王方军\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"安全保密管理员\",\
        \"dept\" : \"xx部xx办公室\",\
        \"secureLevel\" : \"0\",\
        \"address\" : \"xxx路xx号\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg3) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg3);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg3, strlen(msg3));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////151
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(151);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    char msg4[] = "{\
        \"user\":\"zhangsan1\",\
        \"role\" : \"安全保密管理员\",\
        \"secureLevel\" : \"0\",\
        \"deleteUserId\" : \" awdaw83f15bd42411e8basdwadwad0134505a\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg4) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg4);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg4, strlen(msg4));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////161
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(161);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    char msg5[] = "{\
        \"user\":\"zhangsan1\",\
        \"passwd\" : \"e10adc3949ba59abbe56e057f20f883e\",\
        \"name\" : \"王方军\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"安全保密管理员\",\
        \"dept\" : \"xx部xx办公室\",\
        \"secureLevel\" : \"0\",\
        \"address\" : \"xxx路xx号\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg5) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg5);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg5, strlen(msg5));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);

    iocontext.run();



    return 0;
}
