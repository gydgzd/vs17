#pragma once
#include "testAsio.h"           //循环引用头文件会导致声明问题
//// process TCP data
class BaseProcess
{
public:
    BaseProcess() {};
//    virtual~BaseProcess() {};

    int headProcess();
 //   virtual int dataProcess() = 0;//Conn_ptr client, char *buff

};
/* ConferenceManager */
class ConferenceManager :public BaseProcess
{
public:
    virtual int dataProcess();
};
/* DeviceManager */
class DeviceManager :public BaseProcess
{
public:
    virtual int dataProcess();
};
/* UserManager */
class UserManager :public BaseProcess
{
public:
    virtual int dataProcess();
};
/* AuthManager */
class AuthManager :public BaseProcess
{
public:
    virtual int dataProcess();
};

/* UpgradeManager */
class UpgradeManager :public BaseProcess
{
public:
    virtual int dataProcess();
};

class ConfigManager :public BaseProcess
{
public:
    virtual int dataProcess();
};

