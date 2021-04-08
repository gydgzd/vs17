#pragma once
#include <map>
#include <queue>
//           //循环引用头文件会导致声明问题,移到 .cpp中

//// process TCP data
class BaseProcess
{
public:
    BaseProcess();
    virtual~BaseProcess() {};
    static BaseProcess* getProcessor(const char *tag);         // 根据tag决定选择哪个子类方法
    int headProcess(void *client, std::queue<std::string>& q_recv);    // Conn_ptr 
    virtual int msgProcess(void *client, const std::string &msg);
    //judge if there is a msg in read_buffer, and put it into q_recv;
    // return the size of data that handled
    virtual int msgGet(const std::string &buff, std::queue<std::string>& q_recv);

private:
    static BaseProcess* m_process;
};
/* ConferenceDistributor */
class ConferenceDistributor :public BaseProcess
{
public:
    ConferenceDistributor() {};
    virtual int msgGet(const std::string &buff, std::queue<std::string>& q_recv);
    virtual int msgProcess(void *client, const std::string &msg);
};
/* ConferenceManager */
class ConferenceManager :public BaseProcess
{
public:
    ConferenceManager() {};
    virtual int msgProcess(void *client, const std::string &msg);
};
/* DeviceManager */
class DeviceManager :public BaseProcess
{
public:
    DeviceManager() {};
    virtual int msgProcess(void *client, const std::string &msg);
};
/* UserManager */
class UserManager :public BaseProcess
{
public:
    UserManager() {};
    virtual int msgProcess(void *client, const std::string &msg);
};
/* AuthManager */
class AuthManager :public BaseProcess
{
public:
    AuthManager() {};
    virtual int msgProcess(void *client, const std::string &msg);
};

/* UpgradeManager */
class UpgradeManager :public BaseProcess
{
public:
    virtual int msgProcess(void *client, const std::string &msg);
};

class ConfigManager :public BaseProcess
{
public:
    virtual int msgProcess(void *client, const std::string &msg);
};

