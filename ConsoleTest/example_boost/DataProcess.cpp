#include "stdafx.h"
#include "DataProcess.h"
#include "testAsio.h"

BaseProcess* BaseProcess::m_process = nullptr;
BaseProcess::BaseProcess()
{
    //mmap_processor[8001] = std::make_shared<BaseProcess>();

}

BaseProcess* BaseProcess::getProcessor(int port)
{
    switch (port)
    {
    case 8001:     // ConferenceDistributor  0x6012
    {
        m_process = new ConferenceDistributor();
        break;
    }
    case 8002:     // DeviceManager 0x6020
    {
        m_process = new DeviceManager();
        break;
    }
    case 8003:     // UserManager 0x6021
    {
        m_process = new UserManager();
        break;
    }
    case 8004:     // AuthManager  0x6022
    {
        m_process = new AuthManager();
        break;
    }
    case 8005:     // ConferenceManager  0x6023
    {
        m_process = new ConferenceManager();
        break;
    }
    case 8006:     // UpgradeManager 0x6024 
    {
        m_process = new UpgradeManager();
        break;
    }
    case 8007:     // ConfigManager  0x6025
    {
        m_process = new ConfigManager();
        break;
    }
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
        DeviceMngHead *confHead = (DeviceMngHead *)(tmp.c_str());
        int len = ntohs(confHead->len);
        // judge if length is valid
        if (len > 4096)
        {
            std::cout << "data is too big(len>4096): " << len << std::endl;
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
        ConferenceMngHead *confHead = (ConferenceMngHead *)(tmp.c_str());
        int len = ntohs(confHead->len);
        // judge if length is valid
        if (len > 4096)
        {
            std::cout << "data is too big: " << len << std::endl;
            return buff.length();
        }
        if (tmp.length() < sizeof(ConferenceMngHead) + len)
            break;
        // put msg into the queue
        len = sizeof(ConferenceMngHead) + len;
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
    /*   Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6012)
    {
    ConferenceMngHead *confHead = (ConferenceMngHead *)(msg.c_str() + sizeof(Overload));
    */
    ConferenceMngHead *confHead = (ConferenceMngHead *)(msg.c_str());
    confHead->cmdType = ntohs(confHead->cmdType);
    confHead->cmd = ntohs(confHead->cmd);
    confHead->id = ntohll(confHead->id);
    confHead->total = ntohs(confHead->total);
    confHead->index = ntohs(confHead->index);
    confHead->len = ntohs(confHead->len);
    std::cout << "cmd:" << confHead->cmd << " id:" << confHead->id << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //     Overload *write = (Overload *)write_buffer_;
    //     ConferenceMngHead *writeConfHead = (ConferenceMngHead *)(write_buffer_ + sizeof(Overload));
    //     write->tag = htons(overload->tag);
    ConferenceMngHead *writeConfHead = (ConferenceMngHead *)(write_buffer_);
    writeConfHead->version[0] = 0x01;
    writeConfHead->version[1] = 0x02;
    writeConfHead->cmdType = htons(confHead->cmdType);
    writeConfHead->cmd = htons(confHead->cmd);
    writeConfHead->id = htonll(confHead->id);
    writeConfHead->total = htons(confHead->total);
    writeConfHead->index = htons(confHead->index);
    std::string response = "";
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(ConferenceMngHead));
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
    case 0x0602:
    {
        response = "{\
        \"cmd\": 1538,\
        \"userId\": \"XXXXXXXX\",\
        \"result\": 0,\
        \"description\": \"查询成功!\",\
        \"content\": {\
        \"schedule\": {\
        \"id\": \"xxx\",\
        \"name\": \"会议1\",\
        \"creatorId\": \"7cb37f3ef77711e7ad1100f1f5119e46\",\
        \"groupId\": \"7cb57ca3f77711e7ad1100f1f5119e46\",\
        \"createTime\": \"0000-00-00 00:00\",\
        \"startTime\": \"2018-03-13 10:34:15\",\
        \"endTime\": \"2018-03-13 12:32:15\",\
        \"description\": \"\",\
        \"xmcu\": \"\",\
        \"dvr\": \"\",\
        \"status\": 1,\
        \"flag\": 1,\
        \"level\": 2,\
        \"type\": 1,\
        \"pamirMasterNo\":\
    {\
        \"prefix_no_1\":1,\
            \"prefix_no_2\":1,\
            \"prefix_no_3\":1,\
            \"devno\":1\
    },\
        \"planPersonNum\": 22,\
            \"planDevNum\": 11,\
            \"compere\": \"主持人张三\",\
            \"loginName\": \"lwqyuyue\",\
            \"staffLevel\": 2,\
            \"createType\": 1,\
            \"clientType\": 1,\
            \"mainBw\": \"3.0\",\
            \"auxiliaryBw\": \"2.0\",\
            \"masterAddr\": \"1\",\
            \"sfDevAddr\": \"1,6,11,16,21\",\
            \"is_mult_pamir\":0\
            \"is_others_meeting\":0\
}}}";
        break;
    }
    case 0x0101:
    {
        response = "{\
                \"cmd\": 0x0101,\
                \"token\": \"\",\
                \"result\": 0,\
                \"uuid\": \"5021846b3db911eba1e2a4bf01303dd7\",\
                \"description\": \"\",\
                \"content\": {\
                \"dev_no\": {\
                \"devno_1\": 0,\
                \"devno_2\": 0,\
                \"devno_3\": 0,\
                \"devno_4\": 0,\
                \"devno_5\": 0\
        },\
            \"devStatus\": 0,\
                \"meetingId\": \"5021846b3db911eba1e2a4bf01303dd7\",\
                \"result\": 0,\
                \"globle_id_1\": 100,\
                \"globle_id_2\": 321,\
                \"globle_id_3\": 0,\
                \"globle_id_4\": 296\
    }}";
        break;
    }
    case 0x0103:
    {
        response = "{\
                \"cmd\":83,\
                \"token\":\"\",\
                \"result\":0,\
                \"uuid\":\"\",\
                \"description\":\"\",\
                \"content\":{\
                \"dev_no\":{\"devno_1\":0,\"devno_2\":0,\"devno_3\":0,\"devno_4\":0,\"devno_5\":0},\
                \"devStatus\":0,\
                \"meetingId\":\"5e7710c2fd6111eabadbac1f6b6c76c6\",\
                \"result\":0,\
                \"globle_id_1\":-1,\"globle_id_2\":-1,\"globle_id_3\":-1,\"globle_id_4\":-1}}";
        break;
    }
    case 0x0605:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"获取会议列表成功\",\
                \"access_token\": null,\
                \"data\": {\
                \"extra\": {\
                \"totalPage\":77\
                \"businessType\":\"1\"\
        },\
            \"items\": [{\
                \"level\": 5,\
                \"customerUnit\": null,\
                \"createType\": 1,\
                \"operatorStatus\": 1,\
                \"userName\": \"省信访局\",\
                \"customerName\": null,\
                \"svrRegionName\": \"天津市\",\
                \"masterNo\": null,\
                \"customerPosition\": null,\
                \"province\": null,\
                \"name\": \"十二月省信访局视频调度\",\
                \"startTime\": \"2018-12-03 08:59:58\",\
                \"svrRegionId\": \"120000000000\",\
                \"endTime\": \"2018-12-31 18:00:00\",\
                \"devNum\": 67,\
                \"scheduleId\": \"2d2ee1b9ac854de6adeb0b9c55418e71\",\
                \"status\": 4}]}}";
        break;
    }
    case 0x0606:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"获取会议信息成功\",\
                \"access_token\": null,\
                \"data\": {\
                \"extra\": {\
                \"totalPage\":77\
                \"businessType\":\"1\"\
        },\
            \"items\": [{\
                \"level\": 5,\
                \"customerUnit\": null,\
                \"createType\": 1,\
                \"operatorStatus\": 1,\
                \"userName\": \"省信访局\",\
                \"customerName\": null,\
                \"svrRegionName\": \"天津市\",\
                \"masterNo\": null,\
                \"customerPosition\": null,\
                \"province\": null,\
                \"name\": \"十二月省信访局视频调度\",\
                \"startTime\": \"2018-12-03 08:59:58\",\
                \"svrRegionId\": \"120000000000\",\
                \"endTime\": \"2018-12-31 18:00:00\",\
                \"devNum\": 67,\
                \"scheduleId\": \"2d2ee1b9ac854de6adeb0b9c55418e71\",\
                \"status\": 4}]}}";
        break;
    }
    case 0x0312:
    {
        response = "{\
    \"uuid\":\"11919199911\",\
    \"content\":\
    {\
    \"dev_info:\"[\
    {\
        \"dev_no\":\"\",\
        \"devName\":\"\",\
        \"devType\":0,\
        \"hwType\":0\
    }]    }";
        break;
    }
    case 0x0313:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"删除参会方成功\",\
                \"access_token\": null,\
                \"data\":\
            {\
                \"items\":null,\
                    \"extra\": null\
            }}";
        break;
    }
    case 0x0153:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"删除参会方成功\",\
                \"access_token\": null,\
                \"data\":\
            {\
                \"items\":null,\
                    \"extra\": null\
            }}";
        break;
    }
    case 0x0164:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"获取会议设备列表成功\",\
                \"access_token\": null,\
                \"data\":\
            {\
                \"items\":null,\
                    \"extra\": null\
            } }  ";
        break;
    }
    case 0x0111:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"终端重启成功\",\
                \"access_token\": null,\
                \"data\":\
            {\
                \"items\":null,\
                    \"extra\": null\
            }}";
        break;
    }
    case 0x0141:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"设置成功\",\
                \"access_token\": null,\
                \"data\":\
            {\
                \"items\":null,\
                    \"extra\": null\
            } } ";
        break;
    }
    case 0x0140:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"设置成功\",\
                \"access_token\": null,\
                \"data\":\
                {\
                \"items\":null,\
                    \"extra\": null\
                    } } ";
        break;
    }
    case 0x0142:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"设置成功\",\
                \"access_token\": null,\
                \"data\":\
                {\
                \"items\":null,\
                    \"extra\": null\
                    } } ";
        break;
    }
    case 0x0143:
    {
        response = "{\
                \"errcode\": 0,\
                \"errmsg\": \"设置成功\",\
                \"access_token\": null,\
                \"data\":\
                {\
                \"items\":null,\
                    \"extra\": null\
                    } } ";
        break;
    }
    case 0x0105:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0110:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0160:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0123:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0127:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0146:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0112:
    {
        response = "{\
                \"result\":0，\
                \"dev_no\":\"100-101-12675-0-0\"\
                }";
        break;
    }
    case 0x0128:
    {
        response = "{\
                \"result\":0\
                }            ";
        break;
    }
    case 0x0311:
    {
        response = "{\
                \"result\":0\
                }            ";
        break;
    }
    case 0x0106:
    {
        response = "{\
                \"result\":0,\
                \"content\":{\
                \"vir_rec_no\":\"100-101-12675-0-0\",\
                \"rec_svr_no\":\"100-101-12675-0-0\"\
            }}";
        break;
    }
    case 0x0107:
    {
        response = "{\
                \"cmd\": 79,\
                \"token\": \"\",\
                \"result\": 0,\
                \"description\": \"\",\
                \"content\": {\
                \"dev_no\": {\
                \"devno_1\": 100,\
                \"devno_2\": 100,\
                \"devno_3\": 2468,\
                \"devno_4\": 0,\
                \"devno_5\": 0\
            }}}";
        break;
    }
    case 0x0126:
    {
        response = "{\
                \"result\":0\
                }            ";
        break;
    }
    case 0x0134:
    {
        response = "{\
                \"result\":0\
                }            ";
        break;
    }
    }
    if (response.length() == 0)
        return 0;
    len = sizeof(ConferenceMngHead) + response.length();
    //   write->len = htons(len);
    writeConfHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
    //      }
    return 0;
}


/* DeviceManager 0x6020*/
int DeviceManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;
    /*    Overload *overload = (Overload *)buff;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6020)
    {
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buff + sizeof(Overload));
    */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str());
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << " taskNo:" << deviceHead->taskNo << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //    Overload *write = (Overload *)write_buffer_;
    //    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    //    write->tag = htons(overload->tag);
    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_);
    writeDevHead->begin = 0xffffffff;
    writeDevHead->taskNo = htonl(deviceHead->taskNo);
    writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
    writeDevHead->cmdType = htons(deviceHead->cmdType);
    writeDevHead->cmd = htonl(deviceHead->cmd);
    writeDevHead->ret = htons(0);
    std::string response = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
    switch (deviceHead->cmd)
    {
    case 101:
    {
        response = "{\"Msg\":\"\",\
            \"count\": 2, \"Data\" : [{\"id\":\"1231324\", \"accloudid\" : \"2\", \"name\" : \"\", \"master\" : \"北京服务器1\", \"sn\" : \"\", \"logicid\" : \"\", \
            \"soft\" : \"1.0\", \"level\" : \"\", \"numpre\" : \"22111\", \"logicpre\" : \"\", \"ippre\" : "", \"ktz\" : \"\", \"ktywsj\" : \"\", \"sheng\" : \
            \"beijing\", \"shi\" : \"beijing\"}]}";
    }
    break;
    case 102:
    {
        response = "{\"Msg\":\"OK\"}";
    }
    break;
    case 103:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 104:
    {
        response = "{\"Msg\":\"\", \"Data\":{\"accloudid\":\"10011\", \"name\" : \"zizhi2\", \"cldname\" : \"zz\", \"sn\" : \"\", \"region\" : \"\", \"softver\" : \"1.0.0\",\
            \"glip\" : \"\", \"glport\" : \"\", \"logid\" : \"\", \"mac\" : \"\", \"ether\" : \"\", \"vlan\" : \"\", \"level\" : \"\", \"numfix\" : \"\", \"logfix\" : \"\",\
            \"jfxxdz\" : \"\", \"jgh\" : \"\", \"zbjd\" : \"\".\"zbwd\" : \"\", \"fzr\" : \"\", \"fzrdh\" : "", \"khlxr\" : \"\", \"khlxrdh\" : \"\", \"yysm\" : \"\", \"yyslxr\" \
            : \"\", \"yyslxrdh\" : \"\", \"azr\" : \"\", \"azrdh\" : \"\", \"topo\" : \"\", \"workstate\", \"baksn\" : \"\", \"baklogicid\" : \"\", \"bakmac\" : \"\", \
            \"bakworkstate\" : \"\", \"bakhbtimeout\" : \"\", \"ut\" : \"\"}}";
    }
    break;
    case 105:
    {
        response = "{\"Msg\":\"OK\"}";
    }
    break;
    case 107:
    {
        response = "{\"Msg\":\"OK\"}";
    }
    break;
    case 108:
    {
        response = "{\"Msg\":\"OK\"}";
    }
    break;
    case 109:
    {
        response = "{\"Msg\":\"OK\"}";
    }
    break;
    case 110:
    {
        response = "{\"Msg\":\"\",\"count\":\"44\",\"Data\":[{\"devno\":\"22211\",\"v2vno\":\"11111\",\"useflg\":\"1\"}]}";

    }
    break;
    case 111:
    {
        response = "{\"Msg\":\"\"}";

    }
    break;
    case 112:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"version\":\"1.0.2\"}}";

    }
    break;
    case 114:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 115:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"timeout\":\"7\"}}";
    }
    break;
    case 116:
    {
        response = "{\"Msg\":\"\",\"count\":\"111\",\"Data\":[{\"id\":\"333\",\"category\":\"\",\"no\",\"\",\"type\":\"\",\"level\":\"\",\"logtime\":\
            \"\",\"cloudid\":\"\",\"devno\":\"\",\"occurtime\":\"\",\"queueid\":\"\",\"queuetype\":\"\",\"flag\":\"\",\"stype\":\"\",\"dstno\":\"\",\
            \"dstsub\":\"\",\"dstch\":\"\",\"srcno\":\"\",\"srcsub\":\"\",\"srcch\":\"\",\"reason\":\"\"}]}";
    }
    break;
    case 117:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 118:
    {
        response = "{\"Msg\":\"\",\"mptmband\":\"20\",\"mptvband\":\"20\",\"gptmband\":\"30\",\"gptvband\":\"30\"}";
    }
    break;
    case 119:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 390:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"sn\":\"\",\"logicid\":\"\",\"mac\":\"\",\"glip\":\"\",\"glport\":\
            \"\",\"acsip\":\"\",\"acsport\":\"\",\"linkstate\":\"\",\"createid\":\"\",\"createtime\":\"\",\"linkcontent\":\"\"}]}";
    }
    break;
    case 391:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 392:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 393:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 394:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 395:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"mstate\":\"\",\"slavestate\":\"\",\"IOthread\":\"\",\"SQLthread\":\"\"}}";
    }
    break;
    case 396:
    {
        response = "{\"Msg\":\"\",\"count\":\"3\",\"Data\":{\"id\":\"11212\",\"name\":\"zz3\",\"state\":\"\",\"sn\":\"\",\"logicid\":\"\",\"mac\":\"\",\
            \"glip\":\"\",\"glport\":\"\",\"acsip\":\"\",\"acsport\":\"\",\"svrid\":\"\",\"priority\":\"\",\"createid\":\"\",\"createtime\":\"\"}}";
    }
    break;
    case 397:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 398:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 399:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 120:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"usemode\":\"\",\"secretlevel\":\"\",\"sn\":\"\",\
            \"index\":\"\",\"no\":\"\",\"globalno\":\"\",\"v2vno\":\"\",\"ssxmname\",\"\",\"ssmxbh\":\"\",\"topo\":\"\",\"devicetype\":\"\",\
            \"logicid\":\"\",\"mac0\":\"\",\"mac1\":\"\",\"svrtype\":\"\",\"sheng\":\"\",\"shi\":\"\",\"qx\":\"\"}]}";
    }
    break;
    case 121:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"state\":\"\",\"index\":\"\",\"no\":\
            \"\",\"globalno\":\"\",\"v2vno\":\"\",\"ssxmname\",\"\",\"ssmxbh\":\"\",\"topo\":\"\",\"devicetype\":\"\",\"svrtype\":\"\",\"regioninfo\":\
            \"\",\"secretlevel\":\"\",\"logicid\":\"\",\"mac0\":\"\",\"mac1\":\"\",\"dspvsn\":\"\",\"upgdspvsn\":\"\",\"fgpavsn\":\"\",\"upgfgpavsn\":\
            \"\",\"upgstate\",\"up0\":\"\",\"down0\":\"\",\"up1\":\"\",\"down1\":\"\",\"avaipos\":\"\",\"bakstate\":\"\",\"workstate\":\"\",\"bakworkstate\
            \":\"\"}]}";
    }
    break;
    case 122:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 123:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 124:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"region\":\"\",\"state\":\"\",\"index\":\"\",\
            \"svrtype\":\"\",\"devno\":\"\",\"topo\":\"\",\"devicetype\":\"\",\"logtype\":\"\",\"phytype\":\"\",\"logid\":\"\",\"subcount\
            \":\"\",\"glbdevno\":\"\",\"v2vno\":\"\",\"ssxmid\":\"\",\"jfxxdz\":\"\",\"jgh\":\"\",\"zbjd\":\"\".\"zbwd\":\"\",\"fzr\":\"\"\
            ,\"fzrdh\":\"\",\"khlxr\":\"\",\"khlxrdh\":\"\",\"yysmc\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\"azr\":\"\",\"azrdh\":\"\",\
            \"network\":\"\",\"usemode\":\"\",\"secretlevel\":\"\",\"ut\":\"\",\"dspvsn\":\"\",\"upgdspvsn\":\"\",\"fpgavsn\":\"\",\"upgfpgavsn\
            \":\"\",\"upgstate\",\"upgtime\":\"\",\"logport\":[{\"port\":\"\",\"mac\":\"\",\"ether\":\"\",\"vlan\":\"\",\"glblogaddr\":\"\"}],\
            \"baksn\":\"\",\"baklogicid\":\"\",\"bakmac0\":\"\",\"bakmac1\":\"\",\"bakstate\":\"\",\"workstate\":\"\",\"bakworkstate\":\"\",\
            \"devtype\":\"\",\"bakdevtype\":\"\"}";
    }
    break;
    case 125:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 126:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 127:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 128:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 129:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 130:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 131:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 132:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"id\":\"\",\"name\":\"\"}]}";
    }
    break;
    case 133:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"count\":\"\"}}";
    }
    break;
    case 700:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"gid\":\"\",\"pid\":\"\",\"gradeid\":\"\",\"groupname\":\"\",\"leaderid\":\"\",\"leader\":\"\",\"bz\":\"\"}]}";
    }
    break;
    case 701:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 702:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"gid\":\"\",\"pid\":\"\",\"gradeid\":\"\",\"groupname\":\"\",\"leaderid\":\"\",\"leader\":\"\",\"bz\":\"\"}]}";
    }
    break;
    case 703:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 704:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 705:
    {
        response = "{\"Msg\":\"\"}";
    }
    break;
    case 706:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"gid\":\"\",\"devid\":\"\",\"devname\":\"\",\"devtype\":\"\",\"devrole\":\"\",\"devno\":\"\",\
            \"devport\":\"\",\"routeflg\":\"\"}]}";
        break;
    }
    case 707:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 708:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 140:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"sn\":\"\",\"no\":\"\",\"globalno\":\"\",\
            \"v2vno\":\"\",\"logid\":\"\",\"ip\":\"\",\"logtype\":\"\",\"mac\":\"\",\"phytype\":\"\",\"sub\":\"\",\"sheng\":\"\",\"shi\":\
            \"\",\"qx\":\"\",\"usemode\":\"\"}]}";
        break;
    }
    case 141:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"sn\":\"\",\"no\":\"\",\"globalno\":\"\",\"v2vno\"\
            :\"\",\"logid\":\"\",\"logtype\":\"\",\"mac\":\"\",\"ip\":\"\",\"phytype\":\"\",\"sub\":\"\",\"sheng\":\"\",\"shi\":\"\",\"qx\":\"\",\
            \"state\":\"\",\"servicetype\":\"\",\"lgcdevnum\":\"\",\"endtime\":\"\",\"softvsn\":\"\",\"upgsoftvsn\":\"\",\"menuvsn\":\"\",\"upgmenuvsn\":\
            \"\",\"kernel\":\"\",\"upgkernel\":\"\",\"filesys\":\"\",\"upgfilesys\":\"\",\"font\":\"\",\"upgfont\":\"\",\"upgstate\":\"\",\"upgvsntm\":\
            \"\",\"up\":\"\",\"down\":\"\",\"lossrate\":\"\",\"usemode\":\"\"}]}";
        break;
    }
    case 142:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 143:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 144:
    {
        response = "{\"id\":\"\",\"endid\":\"\",\"svrid\":\"\",\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"state\":\"\",\"softvsn\":\"\",\"upgsoftvsn\":\
            \"\",\"menuvsn\":\"\",\"upgmenuvsn\":\"\",\"kernel\":\"\",\"upgkernel\":\"\",\"filesys\":\"\",\"upgfilesys\":\"\",\"font\":\"\",\"upgfont\":\
            \"\",\"isallowpmr\":\"\",\"upgstate\":\"\",\"upgvsntm\":\"\",\"bsnstate\":\"\",\"devno\":\"\",\"phytype\":\"\",\"glbdevno\":\"\",\"v2vno\":\
            \"\",\"khmc\":\"\",\"ssxm\":\"\",\"region\":\"\",\"khlxr1\":\"\",\"khlxrdh1\":\"\",\"khlxrzw1\":\"\",\"khlxr2\":\"\",\"khlxrdh2\":\"\",\
            \"khlxrzw2\":\"\",\"khlxr3\":\"\",\"khlxrdh3\":\"\",\"khlxrzw3\":\"\",\"khhy\":\"\",\"khjb\":\"\",\"contract\":\"\",\"hospital\":\"\",\"khbw\
            \":\"\",\"xxdz\":\"\",\"yysmc\":\"\",\"yyslxr\":\"\",\"yyslxrdh\":\"\",\"zbjd\":\"\",\"zbwd\":\"\",\"azr\":\"\",\"azrdh\":\"\",\"fzr1\":\"\",\
            \"fzrdh1\":\"\",\"fzr2\":\"\",\"fzrdh2\":\"\",\"network\":\"\",\"logid\":\"\",\"logtype\":\"\",\"sub\":\"\",\"ktsx\":\"\",\"zzsj\":\"\",\
            \"instlunit\":\"\",\"instlunitinfo\":\"\",\"logport\":[{\"mac\":\"\",\"ether\":\"\",\"vlan\":\"\",\"ip\":\"\"},\"ut\":\"\",\"opener\":\"\",\
            \"opentime\":\"\"]}";
        break;
    }
    case 145:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 146:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 147:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 148:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 149:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 150:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 151:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 152:
    {
        response = "{\"Msg\":\"\",\"version\":\"\",\"type\":\"\"}";
        break;
    }
    case 153:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 154:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 155:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 156:
    {
        response = "{\"Msg\":\"\",\"config\":\"\"}";
        break;
    }
    case 157:
    {
        response = "{\"Msg\":\"\",\"version\":\"\",\"state\":\"\",\"ip\":\"\",\"gateway\":\"\",\"mask\":\"\"}";
        break;
    }
    case 158:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 159:
    {
        response = "{\"Msg\":\"\",\"count\":\"\",\"Data\":[{\"svrid\":\"\",\"svrname\":\"\",\"teamname\":\"\",\"teamtype\":\"\",\"teamid\":\"\",\"acid\":\
            \"\",\"acname\":\"\",\"name\":\"\",\"id\":\"\",\"mac\":\"\",\"sn\":\"\",\"devno\":\"\",\"ktywqf\":\"\",\"devtype\":\"\",\"khmc\":\"\"}]}";
        break;
    }
    case 281:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 282:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 283:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 284:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"devname\":\"\",\"devno\":\"\",\"globaldevno\":\"\",\"devsubno\":\"\"}]}";
        break;
    }
    case 286:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"count\":\"\"}}";
        break;
    }
    case 287:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"sn\":\"\",\"no\":\"\",\"globalno\":\"\",\"v2vno\":\"\",\
            \"logid\":\"\",\"devtype\":\"\",\"mac\":\"\",,\"sheng\":\"\",\"shi\":\"\",\"qx\":\"\"，\"state\":\"\",\"ut\":\"\"}]}";
        break;
    }
    case 288:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 289:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 290:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 291:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 292:
    {
        response = "{\"Msg\":\"\"，\"Data\":{\"svrcount\":\"\",\"sucscount\":\"\",\"failcount\":\"\"}}";
        break;
    }
    case 293:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"acscmsname\":\"\",\"devname\":\"\",\"gatewayname\":\"\",\"devno\":\"\",\"bridgename\":\"\",\"stgdevno\":\"\",\
            \"ispmr\":\"\",\"isencrypt\":\"\",\"istelnet\":\"\",\"isshow\":\"\",\"islogin\":\"\",\"isshowdevsec\":\"\",\"isshowywsec\":\"\",\"isauxiliary\
            \":\"\",\"secretlevel\":\"\",\"mcname\",\"mcno\":\"\",\"mcsubno\":\"\"}]}";
        break;
    }
    case 294:
    {
        response = "{\"Msg\":\"\",\"Data\":[{\"name\":\"\",\"devtype\":\"\",\"devno\":\"\",\"authstate\":\"\",\"authtime\":\"\"}]}";
        break;
    }
    case 295:
    {
        response = "{\"Msg\":\"\",\"Data\":{\"servicetype\":\"\",\"softvsn\":\"\",\"menuvsn\":\"\",\"kernel\":\"\",\"filesys\":\"\",\"font\":\"\",\"model\":\
            \"\",\"upgstate\":\"\",\"upgvsntm\":\"\", \"ip\":\"\",\"gateway\":\"\":\"submask\"：\"\",\"upflow\":\"\",\"downflow\":\"\",\"lossratevalue\":\
            \"\",\"videolossratevalue\":\"\",\"audiolossratevalue\":\"\",\"cpuvalue\":\"\",\"memoryvalue\":\"\",\"mcno\":\"\",\"mcsubno\":\"\"}}";
        break;
    }
    case 296:
    {
        response = "{\"Msg\":\"\",\"cpu\":\"\",\"cputhreshold\":\"\",\"memory\":\"\",\"memorythreshold\":\"\",\"lossrate\":\"\",\"videolossrate\":\"\",\
            \"audiolossrate\":\"\",\"lossvideo\":\"\",\"lossaudio\":\"\":\"reboot\"：\"\",\"upgradefailed\":\"\",\"cardexception\":\"\",\"reporttvmask\":\
            \"\",\"inputsourceloss\":\"\",\"repeatpkgrate\":\"\"}";
        break;
    }
    case 300:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"subno\":\"\",\"servicetype\":\"\"}]}";
        break;
    }
    case 230:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 231:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"svrid\":\"\",\"name\":\"\",\"devtypename\":\"\",\"devicetype\":\"\":\"sn\":\
            \"\",\"mac\":\"\",\"svrname\":\"\",\"custmname\":\"\",\"devno\":\"\",\"globaldevno\":\"\",\"v2vno\":\"\",\"state\":\"\",\"teamname\":\"\",\
            \"teamtypename\":\"\",\"softvsn\":\"\",\"upgsoftvsn\":\"\",\"menuvsn\":\"\",\"upgmenuvsn\":\"\",\"kernel\":\"\",\"upgkernel\":\"\",\"filesys\":\
            \"\",\"upgfilesys\":\"\",\"font\":\"\",\"upgfont\":\"\",\"upgstate\":\"\",\"upgvsntm\":\"\",\"remarks\":\"\",\"secretlevel\":\"\"}]}";
        break;
    }
    case 232:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 233:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 501:
    {
        response = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"fuzeren\":\"\",\"phone\":\"\",\"v2vno\":\"\",\"mac\":\"\",\"createtime\":\"\",\"bz\":\"\"}]}";
        break;
    }
    case 502:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 503:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    case 504:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    }
    if (response.length() == 0)
        return 0;
    len = sizeof(DeviceMngHead) + response.length() + 4;
    //    write->len = htons(len);
    writeDevHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    memcpy(json + response.length(), &endtag, 4);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);

    //}
    return 0;
}

/* UserManager 0x6021 */
int UserManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;
    /*    Overload *overload = (Overload *)data;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6021)
    {
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buff + sizeof(Overload));
    */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str());
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << " taskNo:" << deviceHead->taskNo << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //     Overload *write = (Overload *)write_buffer_;
    //     DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    //     write->tag = htons(overload->tag);
    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_);
    writeDevHead->begin = 0xffffffff;
    writeDevHead->taskNo = htonl(deviceHead->taskNo);
    writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
    writeDevHead->cmdType = htons(deviceHead->cmdType);
    writeDevHead->cmd = htonl(deviceHead->cmd);
    writeDevHead->ret = htons(0);
    std::string response = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
    switch (deviceHead->cmd)
    {
    case 101:
    {
        response = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"安全保密管理员\",\
        \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
        \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
        \
        },\
        \"ret\": 0,\
        \"msg\" : \"登录成功\"}";

    }
    break;
    case 121:
    {
        response = "{\
            \"ret\":0,\
            \"msg\" : \"用户退出成功\",\
            }  ";
    }
    break;
    case 131:
    {
        response = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"安全保密管理员\",\
        \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
        \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
        \
        },\
        \"ret\": 0,\
        \"msg\" : \"创建成功\"}";
    }
    break;
    case 141:
    {
        response = "{\
            \"ret\": 0,\
            \"msg\" : \"修改成功\"\
            }";
    }
    break;
    case 151:
    {
        response = "{\
            \"ret\": 0,\
            \"msg\" : \"删除成功\"\
            }";
    }
    break;
    case 161:
    {
        response = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
        \"permission\" : \"1,1,1,1,1,0,0,0,0\",\
        \"phone\" : \"13599999999\",\
        \"role\" : \"安全保密管理员\",\
        \"roleId\" : \"005020d2abc511e6802eb82a72db6d4d\",\
        \"userid\" : \"d783f15bd42411e8b8b5a4bf0134505a\"\
        \
        },\
        \"ret\": 0,\
        \"msg\" : \"查询成功\"}";
    }
    break;
    }
    if (response.length() == 0)
        return 0;
    len = sizeof(DeviceMngHead) + response.length() + 4;
    //    write->len = htons(len);

    writeDevHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    memcpy(json + response.length(), &endtag, 4);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);

    //}
    return 0;
}

/* AuthManager 0x6022 */
int AuthManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    /*    Overload *overload = (Overload *)data.c_str();
        overload->tag = ntohs(overload->tag);
        overload->len = ntohs(overload->len);
        std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
        if (overload->tag == 0x6022)
        {
            DeviceMngHead *deviceHead = (DeviceMngHead *)(data.c_str() + sizeof(Overload));
            */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str());
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << " taskNo:" << deviceHead->taskNo << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //      Overload *write = (Overload *)write_buffer_;
    //      DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    //      write->tag = htons(overload->tag);
    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    writeDevHead->begin = 0xffffffff;
    writeDevHead->taskNo = htonl(deviceHead->taskNo);
    writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
    writeDevHead->cmdType = htons(deviceHead->cmdType);
    writeDevHead->cmd = htonl(deviceHead->cmd);
    writeDevHead->ret = htons(0);
    std::string response = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
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
    //     write->len = htons(len);
    writeDevHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    memcpy(json + response.length(), &endtag, 4);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);

    return 0;
}

/* ConferenceManager 0x6023 */
int ConferenceManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;
    /*
    Overload *overload = (Overload *)msg.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6023)
    {
        DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str() + sizeof(Overload));
        */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str());
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << " taskNo:" << deviceHead->taskNo << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //    Overload *write = (Overload *)write_buffer_;
    //    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    //    write->tag = htons(overload->tag);
    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_);
    writeDevHead->begin = 0xffffffff;
    writeDevHead->taskNo = htonl(deviceHead->taskNo);
    writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
    writeDevHead->cmdType = htons(deviceHead->cmdType);
    writeDevHead->cmd = htonl(deviceHead->cmd);
    writeDevHead->ret = htons(0);
    std::string response = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
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
    case 603:
    {
        response = "{\"Msg\":\"\"}";
        break;
    }
    }
    if (response.length() == 0)
        return 0;
    len = sizeof(DeviceMngHead) + response.length() + 4;
    //    write->len = htons(len);
    writeDevHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    memcpy(json + response.length(), &endtag, 4);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
    //}
    return 0;
}

/* UpgradeManager 0x6024 */
int UpgradeManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    /*     Overload *overload = (Overload *)data.c_str();
        overload->tag = ntohs(overload->tag);
        overload->len = ntohs(overload->len);
        std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
        if (overload->tag == 0x6024)
        {
            DeviceMngHead *deviceHead = (DeviceMngHead *)(data.c_str() + sizeof(Overload));
            */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str());
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << " taskNo:" << deviceHead->taskNo << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //      Overload *write = (Overload *)write_buffer_;
    //      DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    //      write->tag = htons(overload->tag);
    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_);
    writeDevHead->begin = 0xffffffff;
    writeDevHead->taskNo = htonl(deviceHead->taskNo);
    writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
    writeDevHead->cmdType = htons(deviceHead->cmdType);
    writeDevHead->cmd = htonl(deviceHead->cmd);
    writeDevHead->ret = htons(0);
    std::string response = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
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
    case 705:
    {
        response = "{\"ret\":\"\",\"Msg\":\"\"}";
        break;
    }
    case 706:
    {
        response = "{\"ret\":\"\",\"Msg\":\"\"}";
        break;
    }
    case 707:
    {
        response = "{\"ret\":\"\",\"Msg\":\"\"}";
        break;
    }
    }
    if (response.length() == 0)
        return 0;
    len = sizeof(DeviceMngHead) + response.length() + 4;
    //     write->len = htons(len);
    writeDevHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    memcpy(json + response.length(), &endtag, 4);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
    //    }

    return 0;
}

/* ConfigManager 0x6025 */
int ConfigManager::msgProcess(void *client, const std::string &msg)
{
    if (msg.length() == 0)
        return 0;

    /*     Overload *overload = (Overload *)data.c_str();
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6025)
    {
    DeviceMngHead *deviceHead = (DeviceMngHead *)(data.c_str() + sizeof(Overload));
    */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(msg.c_str());
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << " taskNo:" << deviceHead->taskNo << " taskNo:" << deviceHead->taskNo << std::endl;

    char write_buffer_[4096];
    memset(write_buffer_, 0, 4096);
    //      Overload *write = (Overload *)write_buffer_;
    //      DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_ + sizeof(Overload));
    //      write->tag = htons(overload->tag);
    DeviceMngHead *writeDevHead = (DeviceMngHead *)(write_buffer_);
    writeDevHead->begin = 0xffffffff;
    writeDevHead->taskNo = htonl(deviceHead->taskNo);
    writeDevHead->deviceNo = htonl(deviceHead->deviceNo);
    writeDevHead->cmdType = htons(deviceHead->cmdType);
    writeDevHead->cmd = htonl(deviceHead->cmd);
    writeDevHead->ret = htons(0);
    std::string response = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
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
    //     write->len = htons(len);
    writeDevHead->len = htons((short)response.length());
    memcpy(json, response.c_str(), response.length());
    memcpy(json + response.length(), &endtag, 4);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
    //    }

    return 0;
}

