#include "stdafx.h"
#include "DataProcess.h"
#include "testAsio.h"

BaseProcess* BaseProcess::m_process = nullptr;
BaseProcess::BaseProcess()
{
    //mmap_processor[8001] = std::make_shared<BaseProcess>();

}

BaseProcess* BaseProcess::getProcessor(const char *tag)
{
    if(strcmp(tag, "0x6012") == 0)     // ConferenceDistributor  0x6012
    {
        m_process = new ConferenceDistributor();
    }
    else if (strcmp(tag, "0x6020") == 0)     // DeviceManager 0x6020
    {
        m_process = new DeviceManager();
    }
    else if (strcmp(tag, "0x6021") == 0)
    {
        m_process = new UserManager();
    }
    else if (strcmp(tag, "0x6022") == 0)     // AuthManager  0x6022
    {
        m_process = new AuthManager();
    }
    else if (strcmp(tag, "0x6023") == 0)     // ConferenceManager  0x6023
    {
        m_process = new ConferenceManager();
    }
    else if (strcmp(tag, "0x6024") == 0)     // UpgradeManager 0x6024 
    {
        m_process = new UpgradeManager();
    }
    else if (strcmp(tag, "0x6025") == 0)     // ConfigManager  0x6025
    {
        m_process = new ConfigManager();
    }
    return m_process;
}

int BaseProcess::headProcess(void *client, std::queue<std::string>& q_recv)
{
    return 0;
}

int BaseProcess::msgProcess(void *client, const std::string &buff)
{

    return 0;
}
// use DeviceMngHead as default
int BaseProcess::msgGet(const std::string & buff, std::queue<std::string>& q_recv)
{
    if (buff.length() <= sizeof(DeviceMngHead))
        return 0;
    std::string tmp = buff;
    int handleLen = 0;
    while (tmp.length() > sizeof(DeviceMngHead))
    {
    //    DeviceMngHead *confHead = (DeviceMngHead *)(tmp.c_str());
        Overload *overload = (Overload *)(tmp.c_str());
        int len = ntohs(overload->len);
        // judge if length is valid
        if (len > 65536)
        {
            std::cout << "data is too big(len>65536): " << len << std::endl;
            g_mylog.log("data is too big(len>65536): %d", len);
            return buff.length();
        }
        if (tmp.length() < sizeof(DeviceMngHead) + len)
            break;
        // put msg into the queue
        len = sizeof(DeviceMngHead) + len + 4;
        q_recv.emplace(tmp.substr(0, len));
        tmp = tmp.substr(len);
        handleLen += len;
    }
    return handleLen;
}

int ConferenceDistributor::msgGet(const std::string & buff, std::queue<std::string>& q_recv)
{
    if (buff.length() <= sizeof(ConferenceMngHead))
        return 0;
    std::string tmp = buff;
    int handleLen = 0;
    while (tmp.length() > sizeof(ConferenceMngHead))
    {
        //    DeviceMngHead *confHead = (DeviceMngHead *)(tmp.c_str());
        Overload *overload = (Overload *)(tmp.c_str());
        int len = ntohs(overload->len);
        // judge if length is valid
        if (len > 65536)
        {
            std::cout << "data is too big: " << len << std::endl;
            g_mylog.log("data is too big(len>65536): %d", len);
            return buff.length();
        }
        if (tmp.length() < (unsigned int)len)
            break;
        // put msg into the queue
        q_recv.emplace(tmp.substr(0, len));
        tmp = tmp.substr(len);
        handleLen += len;
    }
    return handleLen;
}

/* ConferenceDistributor 0x6012*/
int ConferenceDistributor::msgProcess(void * client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;
    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag;
    std::cout << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6012)
    {
        ConferenceMngHead *confHead = (ConferenceMngHead *)(msg.c_str() + sizeof(Overload));

    //    confHead->cmdType = ntohs(confHead->cmdType);
        confHead->cmd = ntohs(confHead->cmd);
        confHead->id = ntohll(confHead->id);
        confHead->total = ntohs(confHead->total);
        confHead->index = ntohs(confHead->index);
        confHead->len = ntohs(confHead->len);
        std::cout << "cmd:" << confHead->cmd << " id:" << confHead->id  << std::endl;
        g_mylog.log("cmd: 0x%04x, id: %d", confHead->cmd, confHead->id);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);

        Overload *write = (Overload *)write_buffer_;
        ConferenceMngHead *writeConfHead = (ConferenceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeConfHead->version[0] = 0x01;
        writeConfHead->version[1] = 0x02;
    //    writeConfHead->cmdType = htons(confHead->cmdType);
        writeConfHead->cmd = htons(confHead->cmd);
        writeConfHead->id = htonll(confHead->id);
        writeConfHead->total = htons(confHead->total);
        writeConfHead->index = htons(confHead->index);
        std::string response = "";
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(ConferenceMngHead));
        switch (confHead->cmd)
        {
        case 0x0600:
        {
            response = "{\
                    \"cmd\": 383963584,\
                    \"token\": \"\",\
                    \"result\": 0,\
                    \"uuid\": \"5021846b3db911eba1e2a4bf01303dd7\",\
                    \"description\": \"\",\
                    \"content\": {\
                    \"dev_no\": \"00000-00000-00000-00000-00000\",\
                    \"devStatus\": 0,\
                    \"meetingId\": \"5021846b3db911eba1e2a4bf01303dd7\",\
                    \"result\": 0,\
                    \"globle_id\": \"00100-00321-00000-00296\"\
                    }  }";
            break;
        }
        case 0x0601:
        {
            response = "{\
                    \"cmd\": 124,\
                    \"token\": \"\",\
                    \"result\": 0,\
                    \"uuid\": \"5021846b3db911eba1e2a4bf01303dd7\",\
                    \"description\": \"\",\
                    \"content\": {\
                    \"dev_no\": \"00000-00000-00000-00000-00000\",\
                    \"devStatus\": 0,\
                    \"meetingId\": \"5021846b3db911eba1e2a4bf01303dd7\",\
                    \"result\": 0,\
                    \"globle_id\": \"00100-00321-00000-00296\"\
                    }  }";
            break;
        }
        }
        if (response.length() == 0)
            return 0;
        len = sizeof(ConferenceMngHead) + response.length();
        write->len = htons(len);
        writeConfHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);
    }
    return 0;
}


/* DeviceManager 0x6020*/
int DeviceManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;
    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6020)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));

        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd<< " taskNo:" << deviceHead->taskNo<< std::endl;
        g_mylog.log("cmd: %d, taskNo: %d", deviceHead->cmd, deviceHead->taskNo);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        std::string response = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 101:
        {
            response = "{\"Msg\":\"\",\
                \"count\": 2, \"Data\" : [{\"id\":\"1231324\", \"accloudid\" : \"2\", \"name\" : \"\", \"master\" : \"����������1\", \"sn\" : \"\", \"logicid\" : \"\", \
                \"soft\" : \"1.0\", \"level\" : \"\", \"numpre\" : \"22111\", \"logicpre\" : \"\", \"ippre\" : "", \"ktz\" : \"\", \"ktywsj\" : \"\", \"sheng\" : \
                \"beijing\", \"shi\" : \"beijing\"}]}";
        }
        break;
        case 102:
        {
            response = "{\"Msg\":\"OK\"}";
        }
        break;

        }
        if (response.length() == 0)
            return 0;
        len = sizeof(DeviceMngHead) + response.length() + 4;
        write->len = htons(len);
        writeDevHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        memcpy(json + response.length(), &endtag, 4);
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);

    }
    return 0;
}

/* UserManager 0x6021 */
int UserManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;
    Overload *overload = (Overload *)msg.c_str();;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6021)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));

        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd<< " taskNo:" << deviceHead->taskNo<< std::endl;
        g_mylog.log("cmd: %d, taskNo: %d", deviceHead->cmd, deviceHead->taskNo);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        std::string response = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 101:
        {
            response = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"������\",\
            \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
            \"phone\" : \"13599999999\",\
            \"role\" : \"��ȫ���ܹ���Ա\",\
            \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
            \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
            \
            },\
            \"ret\": 0,\
            \"msg\" : \"��¼�ɹ�\"}";

        }
        break;
        case 121:
        {
            response = "{\
                \"ret\":0,\
                \"msg\" : \"�û��˳��ɹ�\",\
                }  ";
        }
        break;

        }
        if (response.length() == 0)
            return 0;
        len = sizeof(DeviceMngHead) + response.length() + 4;
        write->len = htons(len);
        writeDevHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        memcpy(json + response.length(), &endtag, 4);
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);
    }
    return 0;
}

/* AuthManager 0x6022 */
int AuthManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6022)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));

        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd<< " taskNo:" << deviceHead->taskNo<< std::endl;
        g_mylog.log("cmd: %d, taskNo: %d", deviceHead->cmd, deviceHead->taskNo);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        std::string response = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 1:
        {
            response = "{\
            \"msg\": \"\",\
            \"result\": 0,\
            \"data\":[{\
            \"time\": \"2020-12-21 14:52:11\",\
            \"moudle_type\": \"1\",\
            \"user_id\": \" shenjiyuan\",\
            \"event_type\": \"101\",	\
            \"object_id\": \"zhangsan\",\
            \"object_type\": \"user\",\
            \"content\": \"\"\
    }, {}] } ";
            break;
        }
        case 2:
        {
            response = "{\
            \"msg\": \"\",\
            \"result\": 0,\
            \"data\":[{\
            \"time\": \"2020-12-21 14:52:11\",\
            \"moudle_type\": \"1\",\
            \"user_id\": \" shenjiyuan\",\
            \"event_type\": \"101\",	\
            \"object_id\": \"zhangsan\",\
            \"object_type\": \"user\",\
            \"content\": \"\"}, {}]} ";
            break;
        }
        }
        if (response.length() == 0)
            return 0;
        len = sizeof(DeviceMngHead) + response.length() + 4;
        write->len = htons(len);
        writeDevHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        memcpy(json + response.length(), &endtag, 4);
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);
    }
    return 0;
}

/* ConferenceManager 0x6023 */
int ConferenceManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6023)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));

        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd<< " taskNo:" << deviceHead->taskNo<< std::endl;
        g_mylog.log("cmd: %d, taskNo: %d", deviceHead->cmd, deviceHead->taskNo);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        std::string response = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 601:
        {
            response = "{\"Msg\":\"\"}";
            break;
        }
        case 602:
        {
            response = "{\"Msg\":\"\"}";
            break;
        }

        }
        if (response.length() == 0)
            return 0;
        len = sizeof(DeviceMngHead) + response.length() + 4;
        write->len = htons(len);
        writeDevHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        memcpy(json + response.length(), &endtag, 4);
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);
    }
    return 0;
}

/* UpgradeManager 0x6024 */
int UpgradeManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6024)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));

        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd<< " taskNo:" << deviceHead->taskNo<< std::endl;
        g_mylog.log("cmd: %d, taskNo: %d", deviceHead->cmd, deviceHead->taskNo);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        std::string response = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 703:
        {
            response = "{\"ret\":\"\",\"Msg\":\"\"}";
            break;
        }
        case 704:
        {
            response = "{\"list\":[{\"file_name\":\"\",\
                \"product\":\"\",\
                \"version\":\"\",\
                \"datetime\":\"\"}],\"Msg\":\"\"}";
            break;
        }

        }
        if (response.length() == 0)
            return 0;
        len = sizeof(DeviceMngHead) + response.length() + 4;
        write->len = htons(len);
        writeDevHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        memcpy(json + response.length(), &endtag, 4);
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);
    }
    return 0;
}

/* ConfigManager 0x6025 */
int ConfigManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6025)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));

        deviceHead->taskNo = ntohl(deviceHead->taskNo);
        deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
        deviceHead->cmdType = ntohs(deviceHead->cmdType);
        deviceHead->cmd = ntohl(deviceHead->cmd);
        deviceHead->ret = ntohs(0);
        std::cout << "cmd:" << deviceHead->cmd<< " taskNo:" << deviceHead->taskNo << std::endl;
        g_mylog.log("cmd: %d, taskNo: %d", deviceHead->cmd, deviceHead->taskNo);
        char write_buffer_[65536];
        memset(write_buffer_, 0, 65536);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);

        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);
        std::string response = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
            case 101:
            {
                response = "{\"ret\":\"\",\"Msg\":\"\"}";
                break;
            }
        }
        if (response.length() == 0)
            return 0;
        len = sizeof(DeviceMngHead) + response.length() + 4;
        write->len = htons(len);
        writeDevHead->len = htons((short)response.length());
        memcpy(json, response.c_str(), response.length());
        memcpy(json + response.length(), &endtag, 4);
        (*(Conn_ptr*)client)->do_write(write_buffer_, len);
    }
    return 0;
}

