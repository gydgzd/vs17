#pragma once
#include <map>
//           //循环引用头文件会导致声明问题,移到 .cpp中

//// process TCP data
class BaseProcess
{
public:
    BaseProcess();
    virtual~BaseProcess() {};
    static BaseProcess* getProcessor(int port);         // 根据port决定选择哪个子类方法
    int headProcess(void *client, const char *buff);    // Conn_ptr 
    virtual int dataProcess(void *client, const char *buff);

private:
    static BaseProcess* m_process;
};

/* ConferenceManager */
class ConferenceManager :public BaseProcess
{
public:
    ConferenceManager() {};
    virtual int dataProcess(void *client, const char *buff);
};
/* DeviceManager */
class DeviceManager :public BaseProcess
{
public:
    DeviceManager() {};
    virtual int dataProcess(void *client, const char *buff);
};
/* UserManager */
class UserManager :public BaseProcess
{
public:
    UserManager() {};
    virtual int dataProcess(void *client, const char *buff);
};
/* AuthManager */
class AuthManager :public BaseProcess
{
public:
    AuthManager() {};
    virtual int dataProcess(void *client, const char *buff);
};

/* UpgradeManager */
class UpgradeManager :public BaseProcess
{
public:
    virtual int dataProcess(void *client, const char *buff);
};

class ConfigManager :public BaseProcess
{
public:
    virtual int dataProcess(void *client, const char *buff);
};

