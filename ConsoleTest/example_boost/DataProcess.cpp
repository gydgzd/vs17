#include "stdafx.h"
#include "DataProcess.h"

int BaseProcess::headProcess(void *client, const char *buff)
{
    return 0;
}

int BaseProcess::dataProcess(void *client, const char *buff)
{
    BaseProcess *bp = new UserManager();
    bp->dataProcess(client, buff);
    return 0;
}


/* ConferenceManager */
int ConferenceManager::dataProcess(void *client, const char *buff)
{

    return 0;
}

/* DeviceManager */
int DeviceManager::dataProcess(void *client, const char *buff)
{

    return 0;
}

/* UserManager */
int UserManager::dataProcess(void *client, const char *buff)
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

        char write_buffer_[4096];
        memset(write_buffer_, 0, 4096);
        Overload *write = (Overload *)write_buffer_;
        DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
        write->tag = htons(overload->tag);
        writeDevHead->begin = 0xffffffff;
        writeDevHead->taskNo = htonl(deviceHead->taskNo);
        writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
        writeDevHead->cmdType = htons(deviceHead->cmdType);
        writeDevHead->cmd = htonl(deviceHead->cmd);
        writeDevHead->ret = htons(0);

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
            std::string data = std::string(write_buffer_, sizeof(Overload) + len);
            (*(Conn_ptr*)client)->do_write(data);
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
            std::string data = std::string(write_buffer_, sizeof(Overload) + len);
            (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
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
            std::string data = std::string(write_buffer_, sizeof(Overload) + len);
            (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
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
            std::string data = std::string(write_buffer_, sizeof(Overload) + len);
            (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
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
            std::string data = std::string(write_buffer_, sizeof(Overload) + len);
            (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
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
            std::string data = std::string(write_buffer_, sizeof(Overload) + len);
            (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
        }
        break;
        }
    }
    return 0;
}

/* AuthManager */
int AuthManager::dataProcess(void *client, const char *buff)
{
    return 0;
}

/* UpgradeManager */
int UpgradeManager::dataProcess(void *client, const char *buff)
{
    return 0;
}

/* ConfigManager */
int ConfigManager::dataProcess(void *client, const char *buff)
{
    return 0;
}