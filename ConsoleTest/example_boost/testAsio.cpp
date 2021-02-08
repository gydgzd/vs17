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
    m_sock_.async_connect(ep, MEM_FN2(on_connect, ep, _1));
}

Client_ptr AsyncClient::start(ip::tcp::endpoint ep, const std::string &message)
{
    Client_ptr new_(new AsyncClient());
    new_->start(ep);
    return new_;
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
        std::cout << "connected to: " << ep.address().to_string() << ":" << int(ep.port()) << std::endl;
    }
    else
    {
        std::cout << "on_connect error to " << ep.address().to_string() << ":" << int(ep.port()) << " - " << err.code() << " - " << err.what() << std::endl;
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
            std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << " received " << bytes << " bytes - " << msg << std::endl;
        }
        do_read();
    }
    else
    {
        std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << "received error: " << err.code() << " - " << err.what() << std::endl;
        stop();
    }
}

size_t AsyncClient::is_read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err)
        return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // ����һ��һ����ȡֱ�������س���������
    return found ? 0 : 1;
}

void AsyncClient::on_write(const asio_error & err, size_t bytes)
{
    ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
    ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
    if (!err.code()) {
        std::string copy(write_buffer_, bytes);
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " wrote " << bytes << " Bytes" << std::endl;
        do_read();
    }
    else
    {
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " write error: " << err.code() << " - " << err.what() << std::endl;
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
    m_sock_.async_write_some(buffer(write_buffer_, msg.size()), m_strand.wrap(MEM_FN2(on_write, _1, _2)));
}

/******** AsyncConnection **********/
AsyncConnection::AsyncConnection() : m_sock_(iocontext), m_connected(true), m_strand(iocontext)
{

}
AsyncConnection::AsyncConnection(Server_ptr pserver) : m_sock_(iocontext), m_connected(true), m_pserver(pserver), m_strand(iocontext)
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
    std::cout << "connection with " << m_sock_.remote_endpoint().address().to_string() << ":" << m_sock_.remote_endpoint().port() << " closed" << std::endl;
    m_sock_.shutdown(ip::tcp::socket::shutdown_both);
    m_sock_.close();
    m_pserver->deleteConn(shared_from_this());
}


void AsyncConnection::on_read(const asio_error & err, size_t bytes)
{
    ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
    ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
    if (err.code())
    {
        close();
        std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << "received error: " << err.code() << " - " << err.what() << std::endl;
    }
    if (!connected())
        return;
    if (bytes > 0)
    {
        std::string msg(read_buffer_, bytes);
        std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << " received " << bytes << " bytes - " << msg << std::endl;
        m_pserver->msgProcess(shared_from_this(), msg.c_str());
    }
    do_read();
}

void AsyncConnection::do_read() {
    // async_read(client->sock(), buffer(read_buffer_), MEM_FN2(is_read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
    if (!connected())
        return;
    m_sock_.async_read_some(buffer(read_buffer_), m_strand.wrap(MEM_FN2(on_read, _1, _2)));
}

void AsyncConnection::do_write(std::string & msg)
{
    if (!connected())
        return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    m_sock_.async_write_some(boost::asio::const_buffer(msg.c_str(), msg.size()), MEM_FN2(on_write, _1, _2));
    // m_sock_.async_write_some(boost::asio::buffer(msg, msg.size()), MEM_FN2(on_write, _1, _2));

    // boost::asio::buffer(msg, msg.size()) not safe, will cause an error - iterator not dereferencable 
    // m_sock_.async_write_some(boost::asio::const_buffer(msg.c_str(), msg.size()), MEM_FN2(on_write, _1, _2)); work OK
}
void AsyncConnection::do_write(const char* msg, unsigned int size)
{
    if (!connected())
        return;
    m_sock_.async_write_some(boost::asio::buffer(msg, size), MEM_FN2(on_write, _1, _2));
}

size_t AsyncConnection::is_read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err)
        return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // ����һ��һ����ȡֱ�������س���������
    return found ? 0 : 1;
}
void AsyncConnection::on_write(const asio_error & err, size_t bytes)
{
    ip::tcp::endpoint remote_ep = m_sock_.remote_endpoint();
    ip::tcp::endpoint local_ep = m_sock_.local_endpoint();
    if (!err.code()) {
        std::string copy(write_buffer_, bytes);
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " wrote " << bytes << " Bytes" << std::endl;
    }
    else
    {
        std::cout << local_ep.address().to_string() << ":" << local_ep.port() << "-->" << remote_ep.address().to_string() << ":" << remote_ep.port() << " write error: " << err.code() << " - " << err.what() << std::endl;
        close();
    }
}
void AsyncConnection::msgProcess(Conn_ptr client, char * buff)
{

}

std::vector<Conn_ptr> AsyncServer::s_conns;
std::vector<int> AsyncServer::s_ports;
/******** AsyncServer **********/
//ip::tcp::acceptor AsyncServer::m_acceptor = ip::tcp::acceptor(iocontext, ip::tcp::endpoint(ip::tcp::v4(), 8001));// ���ڷǾ�̬��Ա�����У��ᵼ��һ���˿ڶ��listen, �޷��ٴ��յ�����
// BaseProcess * AsyncServer::m_processor = nullptr;
AsyncServer::AsyncServer(int listenPort) : timer_(iocontext), m_acceptor(iocontext), m_strand(iocontext)
{
    mn_listenPort = listenPort;
    m_processor = m_processor->getProcessor(listenPort);
};
Server_ptr AsyncServer::start(int listenPort) {
    Server_ptr server(new AsyncServer(listenPort));
    int ret = server->init();
    if (ret == -1)
        return nullptr;
    server->start();
    std::cout << "Start server on port " << listenPort << std::endl;
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
    ip::tcp::endpoint remote_ep = conn->sock().remote_endpoint();
    ip::tcp::endpoint local_ep = conn->sock().local_endpoint();
    if (err.code())
    {
        std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << " connecte error, " << err.code() << " - " << err.what() << std::endl;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(g_mutex_conns);
        s_conns.push_back(conn);
    }
    conn->do_read(); //���ȣ����ǵȴ��ͻ�������
    std::cout << remote_ep.address().to_string() << ":" << remote_ep.port() << "-->" << local_ep.address().to_string() << ":" << local_ep.port() << " connected." << std::endl;
    this->start();
}

void AsyncServer::msgProcess(Conn_ptr client, const char * buff)
{
    m_processor->dataProcess(&client, buff);
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
    ip::tcp::endpoint ep_user(ip::address::from_string("10.1.4.75"), 33300);
    Client_ptr client_user = AsyncClient::start(ep_user, "");
/*
    ip::tcp::endpoint ep_device(ip::address::from_string("127.0.0.1"), 8002);
    Client_ptr client_device = AsyncClient::start(ep_device, "");

    ip::tcp::endpoint ep_conf(ip::address::from_string("127.0.0.1"), 8003);
    Client_ptr client_conf = AsyncClient::start(ep_conf, "");

    ip::tcp::endpoint ep_auth(ip::address::from_string("127.0.0.1"), 8004);
    Client_ptr client_auth = AsyncClient::start(ep_auth, "");

    ip::tcp::endpoint ep_upgrade(ip::address::from_string("127.0.0.1"), 8005);
    Client_ptr client_upgrade = AsyncClient::start(ep_upgrade, "");

    ip::tcp::endpoint ep_config(ip::address::from_string("127.0.0.1"), 8006);
    Client_ptr client_config = AsyncClient::start(ep_config, "");
    */
    std::thread th_user{ testUserManager, client_user };
    std::thread th_device{ testDeviceManager, client_user };
 /*   std::thread th_conf{ testConferenceManager, client_conf };
    std::thread th_auth{ testAuthManager, client_auth };
    std::thread th_upgrade{ testUpgradeManager, client_upgrade };
    std::thread th_config{ testConfigManager, client_config };
    */
    iocontext.run();

    th_user.join();
    th_device.join();
    return 0;
}

int testUserManager(Client_ptr shrd_client)
{
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6021);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    std::string data = "";
    ////101
    /*   */
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0001);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);
    char msg[] = "{   \
                \"user\":\"zhangsan1\",\
                \"passwd\" : \"e10adc3949ba59abbe56e057f20f883e\"\
                }";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
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
    memcpy(json + strlen(msg1), &endtag, 4);
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

    char msg2[] = "{\
        \"user\":\"zhangsan1\",\
        \"passwd\" : \"e10adc3949ba59abbe56e057f20f883e\",\
        \"name\" : \"������\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"��ȫ���ܹ���Ա\",\
        \"dept\" : \"xx��xx�칫��\",\
        \"secureLevel\" : \"0\",\
        \"address\" : \"xxx·xx��\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg2) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg2);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg2, strlen(msg2));
    memcpy(json + strlen(msg2), &endtag, 4);
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
        \"name\" : \"������\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"��ȫ���ܹ���Ա\",\
        \"dept\" : \"xx��xx�칫��\",\
        \"secureLevel\" : \"0\",\
        \"address\" : \"xxx·xx��\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg3) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg3);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg3, strlen(msg3));
    memcpy(json + strlen(msg3), &endtag, 4);
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
        \"role\" : \"��ȫ���ܹ���Ա\",\
        \"secureLevel\" : \"0\",\
        \"deleteUserId\" : \" awdaw83f15bd42411e8basdwadwad0134505a\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg4) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg4);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg4, strlen(msg4));
    memcpy(json + strlen(msg4), &endtag, 4);
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
        \"name\" : \"������\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"��ȫ���ܹ���Ա\",\
        \"dept\" : \"xx��xx�칫��\",\
        \"secureLevel\" : \"0\",\
        \"address\" : \"xxx·xx��\"\
        }";
    len = sizeof(DeviceMngHead) + strlen(msg5) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg5);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg5, strlen(msg5));
    memcpy(json + strlen(msg5), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}
int testDeviceManager(Client_ptr shrd_client)
{
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6020);
    std::string msg = "";
    ////101  zz
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);

    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"name\":\"gg\",\"pageno\":2,\"pagecount\":3}";
    int len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    std::string data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    /// 102
    std::this_thread::sleep_for(chrono::seconds(1));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(100);
    deviceHead->deviceNo = htonl(41);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(102);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"accloudid\":\"3321\",\"name\":\"acss\",\"region\":\"111\",\"glip\":\"22\",\"glport\":\"33\",\"sn\":\"\",\"logid\":\"10086\",\"mac\":\"60:F2:FF:33:24:22\",\
        \"ether\":\"\",\"vlan\":\"2\",\"baksn\":\"\",\"baklogid\":\"\",\"bakmac\":\"60:F2:FF:33:24:21\",\"bakhbtimeout\":\"30\",\"level\":\"3\",\"numfix\":\"00101\",\"logfix\":\
        \"00000\",\"jfxxdz\":\"����\",\"jgh\":\"33\",\"zbjd\":\"\".\"zbwd\":\"\",\"fzr\":\"\",\"fzrdh\":\"\",\"khlxr\":\"\",\"khlxrdh\":\"\",\"yysm\":\"\",\"yyslxr\":\"\",\
        \"yyslxrdh\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"topo\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////103
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(103);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"22111\",\"accloudid\":\"11011\",\"name\":\"zizhi1\",\"sn\":\"\",\"region\":\"\",\"glip\":\"\",\"glport\":\"\",\"logid\":\"\",\"mac\":\"\",\
       \"ether\":\"\",\"vlan\":\"\",\"level\":\"\",\"jfxxdz\":\"\",\"jgh\":\"\",\"zbjd\":\"\".\"zbwd\":\"\",\"fzr\":\"\",\"fzrdh\":\"\",\"khlxr\":\"\",\"khlxrdh\":\"\",\
        \"yysm\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"topo\":\"tree\",\"baksn\":\"\",\"baklogid\":\"\",\"bakmac\":\"\",\"bakhbtimeout\":\"\",\
        \"operid\":\"\",\"ut\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////104
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(104);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"22111\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////105
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(105);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11221\",\"name\":\"zz1\",\"operid\":\"22231\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////107
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(107);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11213\",\"operid\":\"12314121\"}"; 
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////108
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(108);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11213\",\"operid\":\"12314121\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////109
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(109);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"53412\",\"operid\":\"2312312eadwa\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////110
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(110);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"212\",\"custmid\":\"312131\",\"devno\":\"11221\",\"notype\":\"end\",\"operid\":\"33\",\"pageno\":\"3\",\"pagecount\":\"20\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////111
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(111);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"2211\",\"operid\":\"333\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////112
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(112);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"333\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////114
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(114);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"22131\",\"timeout\":\"20\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////115
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(115);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"213123\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////116
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(116);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"213\",\"category\":\"82\",\"devno\":\"11221\",\"begindate\":\"\",\"enddate\":\"\",\"pageno\":\"31\",\"pagecount\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////117
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(117);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11221\",\"operid\":\"333\",\"mptmband\":\"10\",\"mptvband\":\"10\",\"gptmband\":\"100\",\"gptvband\":\"20\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////118
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(118);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11211\",\"operid\":\"2233\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////119
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(119);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11221\",\"gradeid\":\"31213\",\"devnopre\":\"00000\",\"addrpre\":\"12313\",\"operid\":\"333\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////390
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(390);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"name\":\"zz\",\"pageno\":3,\"pagecount\":30}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////391
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(391);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"33\",\"delacid\":\"11211\",\"delacflg\":\"1\",\"addacid\":{\"id\":\"11\",\"name\":\"zz2\",\"sn\":\"\",\"logicid\":\"\",\"mac\":\
        \"\",\"glip\":\"\",\"glport\":\"\",\"acsip\":\"\",\"acsport\",\"devtype\":\"\"}}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////392
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(392);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"33\",\"masteracid\":\"11312\",\"name\":\"zz2\",\"sn\":\"\",\"logicid\":\"\",\"glip\":\"\",\"glport\":\"\",\"acip\":\"\",\"devtype\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////393
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(393);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"211\",\"id\":\"11221\",\"loginuser\":\"cx\",\"loginpwd\":\"123456\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////394
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(394);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"333\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////395
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(395);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"333\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////396
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(396);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"33\",\"pageno\":\"2\",\"pagecount\":\"22\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////397
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(397);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"333\",\"id\":\"14213\",\"name\":\"zz2\",\"sn\":\"\",\"logicid\":\"\",\"mac\":\"\",\"glip\":\"\",\"glport\":\"\",\"acsip\":\"\",\
        \"acsport\":\"\",\"priority\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////398
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(398);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"333\",\"id\":\"11312\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////399
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(399);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"33\",\"id\":\"13121\",\"svrid\":\"00000\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////120 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(120);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11211\",\"name\":\"zza\",\"usemode\":\"\",\"sn\":\"\",\"devno\":\"\",\"logicid\":\"\",\"mac\":\"\",\"pageno\":,\"pagecount\":}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////121 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(122);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(121);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11222\",\"name\":\"wy\",\"usemode\":\"1\",\"sn\":\"\",\"devno\":\"\",\"logicid\":\"\",\"mac\":\"\",\"operid\":\"\",\"pageno\":,\"pagecount\":}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////122 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(122);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"name\":\"\",\"usemode\":\"\",\"ssxmid\":\"\",\"ssxmname\":\"\",\"ssxmbh\":\"\",\"sn\":\"\",\"devtype\":\"\",\"usemode\":\
        \"\",\"secretlevel\":\"\",\"region\":\"\",\"topo\":\"\",\"logid\":\"\",\"subcount\":\"\",\"svrtype\":\"\",\"jfxxdz\":\"\",\"jgh\":\"\",\
        \"zbjd\":\"\".\"zbwd\":\"\",\"fzr\":\"\",\"fzrdh\":\"\",\"khlxr\":\"\",\"khlxrdh\":\"\",\"yysmc\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\
        \"azr\":\"\",\"azrdh\":\"\",\"network\":\"\",\"operid\":\"\",\"logport\":[{\"mac\":\"\",\"ether\":\"\",\"vlan\":\"\"}],\"bakdevinfo\":{\"baksn\"\
        :\"\",\"baklogicid\":\"\",\"bakmac0\":\"\",\"bakmac1\":\"\"}}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////123 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(123);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"name\":\"\",\"usemode\":\"\",\"ssxmid\":\"\",\"ssxmname\":\"\",\"ssxmbh\":\"\",\"region\":\"\",\"topo\":\"\",\"logid\":\
        \"\",\"subcount\":\"\",\"jfxxdz\":\"\",\"jgh\":\"\",\"zbjd\":\"\",\"zbwd\":\"\",\"fzr\":\"\",\"fzrdh\":\"\",\"khlxr\":\"\",\"khlxrdh\":\
        \"\",\"yysmc\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"network\":\"\",\"usemode\":\"\",\"secretlevel\":\"\",\
        \"operid\":\"\",\"ut\":\"\",\"logport\":[{\"port\":\"\",\"mac\":\"\",\"ether\":\"\",\"vlan\":\"\"}],\"bakdevinfo\":{\"baksn\":\"\",\
        \"baklogicid\":\"\",\"bakmac0\":\"\",\"bakmac1\":\"\"}}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////124 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(124);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"12112\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////125 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(125);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"name\":\"wy2\",\"operid\":\"11221\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////126 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(126);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////127 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(127);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////128 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(128);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////130 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(130);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11222\",\"name\":\"zzz\",\"sn\":\"\",\"devno\":\"11213\",\"logicid\":\"\",\"mac\":\"\",\"pageno\":,\"pagecount\":}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////131 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(131);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11\",\"operid\":\"112\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////132 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(132);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"11222\",\"svrtype\":\"master\",\"operid\":\"111\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////133 weiyun
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(133);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"operid\":\"33\",\"id\":\"11211\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////700 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(700);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"groupname\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////701 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(701);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"groupname\": \"\",\"gradeid\":\"\",\"pid\":\"\",\"svrlist\":\"\",\"leaderid\":\"\",\"bz\":\"\" ,\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////702 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(702);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"gid\":\"1\",\"groupname\": \"\",\"bz\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////703 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(703);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"gid\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////704 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(704);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"gid\":\"\",\"adddevlist\": \"\",\"deldevlist\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////705 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(705);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"grouplist\":[{\"gid\":\"\",\"leaderid\": \"\"}],\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////706 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(706);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"gid\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////707 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(707);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devid\":\"\",\"devtype\":\"\".\"opertype\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////708 zf
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(708);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devid\":\"\",\"devtype\":\"\",\"opertype\":\"\",\"srcdevid\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////140 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(140);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"svrid\":\"\",\"name\":\"\",\"sn\":\"\",\"mac\":\"\",\"logicid\":\"\",\"devno\":\"\",\"devtype\":\"\",\"usemode\":\"\",\
        \"teamtype\":\"\",\"teamid\":\"\",\"pageno\":\"\",\"pagecount\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////141 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(141);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"svrid\":\"\",\"gatewayid\":\"\",\"name\":\"\",\"sn\":\"\",\"mac\":\"\",\"logicid\":\"\",\"devno\":\"\",\"devtype\":\
        \"\",\"usemode\":\"\",\"teamtype\":\"\",\"teamid\":\"\",\"ktywqf\":\"\",\"pageno\":\"\",\"pagecount\":\"\",\"opertype\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////142 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(142);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"svrid\":\"\",\"name\":\"\",\"sn\":\"\",\"usemode\":\"\",\"phytype\":\"\",\"khid\":\"\",\"khmc\":\"\",\"ssxm\":\"\",\"isallowpmr\"\
        :\"\",\"khlxr1\":\"\",\"khlxrdh1\":\"\",\"khlxrzw1\":\"\",\"khlxr2\":\"\",\"khlxrdh2\":\"\",\"khlxrzw2\":\"\",\"khlxr3\":\"\",\"khlxrdh3\":\"\",\
        \"khlxrzw3\":\"\",\"khhy\":\"\",\"khjb\":\"\",\"khbw\":\"\",\"region\":\"\",\"xxdz\":\"\",\"yysmc\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\"useflg\":\
        \"\",\"orgid\":\"\",\"contractid\":\"\",\"contract\":\"\",\"hospitalid\":\"\",\"zbjd\":\"\",\"zbwd\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"fzr1\":\"\",\
        \"fzrdh1\":\"\",\"fzr2\":\"\",\"fzrdh2\":\"\",\"network\":\"\",\"usemode\":\"\",\"operid\":\"\",\"loginfo\":[{\"logid\":\"\",\"devno\":\"\",\"nouseflg\
        \":\"\",\"logtype\":\"\",\"sub\":\"\",\"ktsx\":\"\",\"zzsj\":\"\",\"instlunit\":\"\",\"instlunitinfo\":\"\",\"logport\":[{\"mac\":\"\",\"ether\":\"\",\
        \"vlan\":\"\"}]}]}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////143 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(143);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"usemode\":\"\",\"zbjd\":\"\",\"zbwd\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"fzr1\":\"\",\"fzrdh1\":\"\",\
        \"fzr2\":\"\",\"fzrdh2\":\"\",\"network\":\"\",\"isallowpmr\":\"\",\"operid\":\"\",\"ut\":\"\",\"logid\":\"\",\"logtype\":\"\",\"sub\":\"\",\"ktsx\":\
        \"\",\"zzsj\":\"\",\"instlunit\":\"\",\"instlunitinfo\":\"\",logport[{\"mac\":\"\",\"ether\":\"\",\"vlan\":\"\"}]}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////144 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(144);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"id\":\"\",\"endid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////145 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(145);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////146 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(146);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"operid\":\"\",\"cmscntno\":\"\",\"acscmsid\":\"\",\"bridgeno\":\"\",\"ispmr\":\"\",\"isencrypt\":\"\",\"istelnet\":\"\",\"isshow\":\
        \"\",\"islogin\":\"\",\"isshowdevsec\":\"\",\"isshowywsec\":\"\",\"isauxiliary\":\"\",\"secretlevel\":\"\",\"mcname\":\"\",\"mcid\":\"\",\"mcno\":\
        \"\",\"mcsubno\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////147 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(147);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////148 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(148);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////149 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(149);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////150 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(150);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////151 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(151);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////152 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(152);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"11211\",\"operid\":\"33\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////153 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(153);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"endid\":\"\",\"id\":\"\",\"operid\":\"\",\"sn\":\"\",\"mac\":\"\",\"logicid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////154 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(154);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"svrid\":\"\",\"no\":\"\",\"swapsvrid\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////155 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(155);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"Msg\":\"\",\"count\":\"\",\"Data\":[{\"time\":\"\",\"svrname\":\"\",\"name\":\"\",\"sn\":\"\",\"endno\":\"\",\"endtype\":\"\"]}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////156 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(156);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////157 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(157);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////158 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(158);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"acid\":\"\",\"id\":\"\",\"ip\":\"\",\"gateway\":\"\",\"mask\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////159 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(159);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"cnd\":\"\",��operid\":\"\",\"pageno\":\"\",\"pagecount\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////281 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(281);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svrid\":\"\",\"teamtype.\":\"\",\"teamid\":\"\",\"sn\":\"\",\"devtype\":\"\",\"custmid\":\"\",\"operid\":\"\",\"subcount\":\"\",\"instlnuit\":\
        \"\",\"unstlunitinfo\":\"\",\"usemode\":\"\",\"name\":\"\",\"zbwd\":\"\",\"zbjd\":\"\",'azr\":\"\",\"azrdh\":\"\",\"fzr1\":\"\",\"fzrdh1\":\"\",\
        \"network\":\"\",\"ethertype\":\"\",\"vlan\":\"\",\"addtype\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////282 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(282);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svrid\":\"\",\"teamid\":\"\",\"teamtype\":\"\",\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"phytype\":\"\",\"isallowpmr\":\"\",\"khid\":\
        \"\",\"khmc\":\"\",\"ssxm\":\"\",\"khlxr1\":\"\",\"khlxrdh1\":\"\",\"khlxrzw1\":\"\",\"khlxr2\":\"\",\"khlxrdh2\":\"\",\"khlxrzw2\":\"\",\"khlxr3\":\
        \"\",\"khlxrdh3\":\"\",\"khlxrzw3\":\"\",\"khhy\":\"\",\"khjb\":\"\",\"contractid\":\"\",\"contract\":\"\",\"hospitalid\":\"\",\"khbw\":\"\",\"region\
        \":\"\",\"xxdz\":\"\",\"yysmc\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\"useflg\":\"\",\"zbjd\":\"\",\"zbwd\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"fzr1\
        \":\"\",\"fzrdh1\":\"\",\"fzr2\":\"\",\"fzrdh2\":\"\",\"network\":\"\",\"operid\":\"\",\"logid\":\"\",\"devno\":\"\",\"nouseflg\":\"\",\"logtype\":\"\
        \",\"sub\":\"\",\"ktsx\":\"\",\"zzsj\":\"\",\"instlunit\":\"\",\"instlunitinfo\":\"\",\"mac\":\"\",\"ether\":\"\",\"vlan\":\"\",\"probeid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////283 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(283);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"gtwayendno\":\"\",\"gtwaysubno\":\"\",\"soft\":\"\",\"menu\":\"\",\"kernel\":\"\",\"filesystem\":\"\",\"font\":\"\",\"fontmd5\":\"\",\
        \"devtype\":\"\",\"strategy\":\"\",\"time\":\"\",\"endlist\":[{\"devno\":\"\",\"subno\":\"\"}],\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////284 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(284);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svrid\":\"\",\"teamid\":\"\",\"teamtype\":\"\",��operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////286 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(286);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svrid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////287 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(287);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svrid\":\"\",\"teamid\":\"\",\"teamtype\":\"\",\"name\":\"\",\"sn\":\"\",\"mac\":\"\",\"logicid\":\"\",\"devno\":\"\",\"usemode\":\"\",\
        \"pageno\":\"\",\"pagecount\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////289 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(289);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"oldid\":\"\",\"newid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////288 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(288);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"cmscntno\":\"\",\"acscmsid\":\"\",\"bridgeno\":\"\",\"ispmr\":\"\",\"isencrypt\":\"\",\"istelnet\":\"\",\"isshow\":\"\",\"mcname\":\"\",\
        \"mcid\":\"\",\"mcno\":\"\",\"mcsubno\":\"\",\"islogin\":\"\",\"isshowdevsec\":\"\",\"isshowywsec\":\"\",\"isauxiliary\":\"\",\"secretlevel\":\
        \"\",\"devlist\":[{\"devno\":\"\",\"\":\"subno\"}],\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////289 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(289);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"oldid\":\"\",\"newid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////290 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(289);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"olddevno\":\"\",\"operid\":\"\",\"devno\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////291 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(291);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svrid\":\"\",\"sn\":\"\",\"devtype\":\"\",\"custmid\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////292 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(292);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devtype\":\"\",\"count\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////293 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(293);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devnolist\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////294 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(294);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////295 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(295);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devno\":\"\",\"state\":\"\",\"devtype\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////295 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(295);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devno\":\"\",\"state\":\"\",\"devtype\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////296 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(296);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"devno\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////300 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(300);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////230 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(230);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"devno\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////231 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(231);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"svridlist\":\"\",\"teamtype\":\"\",\"teamname\":\"\",\"name\":\"\",\"sn\":\"\",\"mac\":\"\",\"devno\":\"\",\"devtypelist\":\"\",\"state\":\"\",\
        \"remarks\":\"\",\"secretlevel\":\"\",\"regionid\":\"\",\"operid\":\"\",\"pageno\":\"\",\"pagecount\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////232 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(232);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"endid\":\"\",\"remarks\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////233 zd
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(233);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"endid\":\"\",\"operflg\":\"\",\"oldpwd\":\"\",\"newpwd\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////501 zjgl
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(501);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"name\":\"\",\"pageno\":,\"pagecount\":}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////502 zjgl
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(502);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"name\":\"\",\"xmbh\":\"\",\"bz\":\"\",\"name\":\"\",\"fuzeren\":\"\",\"phone\":\"\",\"v2vno\":\"\",\"mac\":\"\",\"opertype\":\"\",\"createtime\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////503 zjgl
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(503);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"name\":\"\",\"bz\":\"\",\"name\":\"\",\"fuzeren\":\"\",\"phone\":\"\",\"v2vno\":\"\",\"mac\":\"\",\"operid\":\"\",\"ut\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ///////504 zjgl
    std::this_thread::sleep_for(chrono::seconds(2));
    deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(332);
    deviceHead->deviceNo = htonl(22211);
    deviceHead->cmdType = htons(0x0002);
    deviceHead->cmd = htonl(504);
    deviceHead->ret = htons(0);

    json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    msg = "{\"id\":\"\",\"name\":\"\",\"operobjtype\":\"\",\"operid\":\"\"}";
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    overload->len = htons(len);
    deviceHead->len = (short)msg.length();
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;

}
int testAuthManager(Client_ptr shrd_client)
{
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6022);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    std::string data = "";
    ////01
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0010);
    deviceHead->cmd = htonl(1);
    deviceHead->ret = htons(0);
    char msg[] = "{\
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
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////02
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0010);
    deviceHead->cmd = htonl(2);
    deviceHead->ret = htons(0);
    char msg[] = "{\
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
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}
int testConferenceManager(Client_ptr shrd_client)
{
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6023);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    std::string data = "";
    ////601
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0003);
    deviceHead->cmd = htonl(601);
    deviceHead->ret = htons(0);
    char msg[] = "{\"startTime\":\"\",\
        \"endTime\":\"\",\
        \"creatorId\":\"\",\
        \"reqId \":\"\",\
        \"status\":\"\",\
        \"pageNum\":\"\",\
        \"pageSize\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////602
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0003);
    deviceHead->cmd = htonl(602);
    deviceHead->ret = htons(0);
    char msg[] = "{\"startTime\":\"\",\
        \"endTime\":\"\",\
        \"creatorId\":\"\",\
        \"reqId\":\"\",\
        \"status\":\"2\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////603
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0003);
    deviceHead->cmd = htonl(603);
    deviceHead->ret = htons(0);
    char msg[] = "{\"startTime\":\"\",\
        \"endTime\":\"\",\
        \"creatorId\":\"\",\
        \"reqId\":\"\",}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}

int testUpgradeManager(Client_ptr shrd_client)
{
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6024);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    std::string data = "";
    ////703
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(703);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"path\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\",\
        \"datetime\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////704
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(704);
    deviceHead->ret = htons(0);
    char msg[] = "{\
        \"product\":\"\",\
        \"version\":\"\",\
        \"datetime\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////705
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(705);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////706
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(706);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////707
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(707);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}
int testConfigManager(Client_ptr shrd_client)
{
    char buffer[1024] = {};
    int endtag = 0xeeeeeeee;
    Overload *overload = (Overload *)buffer;
    overload->tag = htons(0x6025);
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buffer + sizeof(Overload));
    char* json = (char*)(buffer + sizeof(Overload) + sizeof(DeviceMngHead));
    int len = 0;
    std::string data = "";
    ////101
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0005);
    deviceHead->cmd = htonl(101);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"path\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\",\
        \"datetime\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////704
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(704);
    deviceHead->ret = htons(0);
    char msg[] = "{\
        \"product\":\"\",\
        \"version\":\"\",\
        \"datetime\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////705
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(705);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////706
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(706);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    ////707
    deviceHead->begin = 0xffffffff;
    deviceHead->taskNo = htonl(123);
    deviceHead->deviceNo = htonl(456);
    deviceHead->cmdType = htons(0x0004);
    deviceHead->cmd = htonl(707);
    deviceHead->ret = htons(0);
    char msg[] = "{\"file_name\":\"\",\
        \"product\":\"\",\
        \"version\":\"\",\
        \"reqId\":\"\"}";
    len = sizeof(DeviceMngHead) + strlen(msg) + 4;
    overload->len = htons(len);
    deviceHead->len = (short)strlen(msg);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg, strlen(msg));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);
    return 0;
}