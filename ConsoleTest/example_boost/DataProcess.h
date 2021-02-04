#pragma once
#include "testAsio.h"           //ѭ������ͷ�ļ��ᵼ����������

//// process TCP data
class BaseProcess
{
public:
    BaseProcess() {};
    virtual~BaseProcess() {};

    int headProcess(void *client, const char *buff);   // Conn_ptr 
    virtual int dataProcess(void *client, const char *buff);

};
/* ConferenceManager */
class ConferenceManager :public BaseProcess
{
public:
    virtual int dataProcess(void *client, const char *buff);
};
/* DeviceManager */
class DeviceManager :public BaseProcess
{
public:
    virtual int dataProcess(void *client, const char *buff);
};
/* UserManager */
class UserManager :public BaseProcess
{
public:
    virtual int dataProcess(void *client, const char *buff);
};
/* AuthManager */
class AuthManager :public BaseProcess
{
public:
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

