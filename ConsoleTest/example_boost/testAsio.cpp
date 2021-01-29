#include "stdafx.h"
#include "testAsio.h"

std::mutex g_mutex;


boost::asio::io_service myservice;
boost::asio::io_service service;
ip::tcp::acceptor AsyncServer::m_acceptor = ip::tcp::acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
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
        if (bytes > 0)
        {
            std::string copy(read_buffer_, bytes);
            std::cout << "received:  " << sock.remote_endpoint().address().to_string() << "  " << copy << std::endl;
        }
        do_read(sock_);
    }
    else
    {
        std::cout << "on_read error: " << sock.remote_endpoint().address().to_string() << err.code() << " - " << err.what() << std::endl;
        stop();
    }

}

size_t AsyncClient::read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err)
        return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // 我们一个一个读取直到读到回车，不缓存
    return found ? 0 : 1;
}

void AsyncClient::on_write(const asio_error & err, size_t bytes)
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
    //    async_read(sock_, buffer(read_buffer_), MEM_FN2(read_complete, _1, _2), MEM_FN3(on_read, boost::ref(sock_), _1, _2));   // sock_ will be error,use boost::ref(sock_)
    sock.async_read_some(buffer(read_buffer_), MEM_FN3(on_read, boost::ref(sock_), _1, _2));
}

void AsyncClient::do_write(const std::string & msg) {
    if (!started())
        return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    sock_.async_write_some(buffer(write_buffer_, msg.size()), MEM_FN2(on_write, _1, _2));
}

/******** AsyncServer **********/

AsyncServer::client_ptr AsyncServer::start() {
    client_ptr client(new AsyncServer);
    client->start(8001);

    return client;
}

void AsyncServer::start(int listen_port)
{
    m_acceptor = ip::tcp::acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), listen_port));
    m_acceptor.async_accept(sock_, boost::bind(&AsyncServer::on_accept, shared_from_this(), shared_from_this(), _1));
}

void AsyncServer::stop() {
    if (!started_)
        return;
    started_ = false;
    sock_.close();
    client_ptr self = shared_from_this();
    array::iterator it = std::find(clients.begin(), clients.end(), self);
    clients.erase(it);
}

void AsyncServer::on_accept(client_ptr client, const asio_error & err)
{
    if (err.code())
    {
        std::cout << "on_accept error: " << err.code() << " - " << err.what() << std::endl;
        return;
    }
    ip::tcp::endpoint ep = client->sock_.remote_endpoint();
    std::cout << ep.address().to_string() << " : " << ep.port() << " connected." << std::endl;
    started_ = true;
    clients.push_back(shared_from_this());
    //last_ping = boost::posix_time::microsec_clock::local_time();
    do_read(client); //首先，我们等待客户端连接

    client->start();
}

void AsyncServer::on_read(client_ptr client, const asio_error & err, size_t bytes) {
    if (err.code())
    {
        stop();
        std::cout << "on_read error: " << err.code() << " - " << err.what() << std::endl;
    }
    if (!started())
        return;
    if (bytes > 0)
    {
        std::string copy(read_buffer_, bytes);
        std::cout << "received:  " << bytes << " - " << client->sock_.remote_endpoint().address().to_string() << "  " << copy << std::endl;
        msgProcess(client, read_buffer_);
    }
    do_read(client);
    std::string msg(read_buffer_, bytes);
    std::cout << read_buffer_ << std::endl;
}

void AsyncServer::do_read(client_ptr client) {
    //    async_read(client->sock(), buffer(read_buffer_), MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
    client->sock().async_read_some(buffer(read_buffer_), MEM_FN3(on_read, client, _1, _2));
}

size_t AsyncServer::is_read_complete(const boost::system::error_code & err, size_t bytes) {
    if (err)
        return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // 我们一个一个读取直到读到回车，不缓存
    return found ? 0 : 1;
}
void AsyncServer::on_write(const asio_error & err, size_t bytes)
{
    if (!err.code()) {
        std::string copy(write_buffer_, bytes);
        std::cout << "write: " << bytes << " - " << copy << std::endl;
    }
    else
    {
        std::cout << "on_write error: " << err.code() << " - " << err.what() << std::endl;
        stop();
    }
}
void AsyncServer::msgProcess(client_ptr client, char * buff)
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
            writeDevHead->len = strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
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
            writeDevHead->len = strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
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
            writeDevHead->len = strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
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
            writeDevHead->len = strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
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
            writeDevHead->len = strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
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
            writeDevHead->len = strlen(msg);
            writeDevHead->len = htons(deviceHead->len);
            memcpy(json, msg, strlen(msg));
            memcpy(json + strlen(msg), &endtag, 4);
            data = std::string(write_buffer_, sizeof(Overload) + len);
            client->sock_.async_write_some(boost::asio::buffer(write_buffer_, data.size()), MEM_FN2(on_write, _1, _2));
        }
        break;
        }

    }
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
    deviceHead->len = strlen(msg);
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
    deviceHead->len = strlen(msg1);
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
    deviceHead->len = strlen(msg2);
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
    deviceHead->len = strlen(msg3);
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
    deviceHead->len = strlen(msg4);
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
    deviceHead->len = strlen(msg5);
    deviceHead->len = htons(deviceHead->len);
    memcpy(json, msg5, strlen(msg5));
    memcpy(json + strlen(msg), &endtag, 4);
    data = std::string(buffer, sizeof(Overload) + len);
    shrd_client->do_write(data);

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
