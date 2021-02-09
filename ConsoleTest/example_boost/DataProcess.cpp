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

int BaseProcess::headProcess(void *client, const char *buff)
{
    return 0;
}

int BaseProcess::dataProcess(void *client, const char *buff)
{

    return 0;
}

/* ConferenceDistributor 0x6012*/
int ConferenceDistributor::dataProcess(void * client, const char * buff)
{
    return 0;
}


/* DeviceManager 0x6020*/
int DeviceManager::dataProcess(void *client, const char *buff)
{
    /*    Overload *overload = (Overload *)buff;
        overload->tag = ntohs(overload->tag);
        overload->len = ntohs(overload->len);
        std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
        if (overload->tag == 0x6020)
        {
            DeviceMngHead *deviceHead = (DeviceMngHead *)(buff + sizeof(Overload));
            */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buff);
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << std::endl;

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
    std::string msg = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
    switch (deviceHead->cmd)
    {
    case 101:
    {
        msg = "{\"Msg\":\"\",\
                \"count\": 2, \"Data\" : [{\"id\":\"1231324\", \"accloudid\" : \"2\", \"name\" : \"\", \"master\" : \"北京服务器1\", \"sn\" : \"\", \"logicid\" : \"\", \
                \"soft\" : \"1.0\", \"level\" : \"\", \"numpre\" : \"22111\", \"logicpre\" : \"\", \"ippre\" : "", \"ktz\" : \"\", \"ktywsj\" : \"\", \"sheng\" : \
                \"beijing\", \"shi\" : \"beijing\"}]}";
    }
    break;
    case 102:
    {
        msg = "{\"Msg\":\"OK\"}";
    }
    break;
    case 103:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 104:
    {
        msg = "{\"Msg\":\"\", \"Data\":{\"accloudid\":\"10011\", \"name\" : \"zizhi2\", \"cldname\" : \"zz\", \"sn\" : \"\", \"region\" : \"\", \"softver\" : \"1.0.0\",\
                \"glip\" : \"\", \"glport\" : \"\", \"logid\" : \"\", \"mac\" : \"\", \"ether\" : \"\", \"vlan\" : \"\", \"level\" : \"\", \"numfix\" : \"\", \"logfix\" : \"\",\
                \"jfxxdz\" : \"\", \"jgh\" : \"\", \"zbjd\" : \"\".\"zbwd\" : \"\", \"fzr\" : \"\", \"fzrdh\" : "", \"khlxr\" : \"\", \"khlxrdh\" : \"\", \"yysm\" : \"\", \"yyslxr\" \
                : \"\", \"yyslxrdh\" : \"\", \"azr\" : \"\", \"azrdh\" : \"\", \"topo\" : \"\", \"workstate\", \"baksn\" : \"\", \"baklogicid\" : \"\", \"bakmac\" : \"\", \
                \"bakworkstate\" : \"\", \"bakhbtimeout\" : \"\", \"ut\" : \"\"}}";
    }
    break;
    case 105:
    {
        msg = "{\"Msg\":\"OK\"}";
    }
    break;
    case 107:
    {
        msg = "{\"Msg\":\"OK\"}";
    }
    break;
    case 108:
    {
        msg = "{\"Msg\":\"OK\"}";
    }
    break;
    case 109:
    {
        msg = "{\"Msg\":\"OK\"}";
    }
    break;
    case 110:
    {
        msg = "{\"Msg\":\"\",\"count\":\"44\",\"Data\":[{\"devno\":\"22211\",\"v2vno\":\"11111\",\"useflg\":\"1\"}]}";

    }
    break;
    case 111:
    {
        msg = "{\"Msg\":\"\"}";

    }
    break;
    case 112:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"version\":\"1.0.2\"}}";

    }
    break;
    case 114:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 115:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"timeout\":\"7\"}}";
    }
    break;
    case 116:
    {
        msg = "{\"Msg\":\"\",\"count\":\"111\",\"Data\":[{\"id\":\"333\",\"category\":\"\",\"no\",\"\",\"type\":\"\",\"level\":\"\",\"logtime\":\
                \"\",\"cloudid\":\"\",\"devno\":\"\",\"occurtime\":\"\",\"queueid\":\"\",\"queuetype\":\"\",\"flag\":\"\",\"stype\":\"\",\"dstno\":\"\",\
                \"dstsub\":\"\",\"dstch\":\"\",\"srcno\":\"\",\"srcsub\":\"\",\"srcch\":\"\",\"reason\":\"\"}]}";
    }
    break;
    case 117:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 118:
    {
        msg = "{\"Msg\":\"\",\"mptmband\":\"20\",\"mptvband\":\"20\",\"gptmband\":\"30\",\"gptvband\":\"30\"}";
    }
    break;
    case 119:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 390:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"sn\":\"\",\"logicid\":\"\",\"mac\":\"\",\"glip\":\"\",\"glport\":\
                \"\",\"acsip\":\"\",\"acsport\":\"\",\"linkstate\":\"\",\"createid\":\"\",\"createtime\":\"\",\"linkcontent\":\"\"}]}";
    }
    break;
    case 391:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 392:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 393:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 394:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 395:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"mstate\":\"\",\"slavestate\":\"\",\"IOthread\":\"\",\"SQLthread\":\"\"}}";
    }
    break;
    case 396:
    {
        msg = "{\"Msg\":\"\",\"count\":\"3\",\"Data\":{\"id\":\"11212\",\"name\":\"zz3\",\"state\":\"\",\"sn\":\"\",\"logicid\":\"\",\"mac\":\"\",\
                \"glip\":\"\",\"glport\":\"\",\"acsip\":\"\",\"acsport\":\"\",\"svrid\":\"\",\"priority\":\"\",\"createid\":\"\",\"createtime\":\"\"}}";
    }
    break;
    case 397:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 398:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 399:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 120:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"usemode\":\"\",\"secretlevel\":\"\",\"sn\":\"\",\
                \"index\":\"\",\"no\":\"\",\"globalno\":\"\",\"v2vno\":\"\",\"ssxmname\",\"\",\"ssmxbh\":\"\",\"topo\":\"\",\"devicetype\":\"\",\
                \"logicid\":\"\",\"mac0\":\"\",\"mac1\":\"\",\"svrtype\":\"\",\"sheng\":\"\",\"shi\":\"\",\"qx\":\"\"}]}";
    }
    break;
    case 121:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"state\":\"\",\"index\":\"\",\"no\":\
                \"\",\"globalno\":\"\",\"v2vno\":\"\",\"ssxmname\",\"\",\"ssmxbh\":\"\",\"topo\":\"\",\"devicetype\":\"\",\"svrtype\":\"\",\"regioninfo\":\
                \"\",\"secretlevel\":\"\",\"logicid\":\"\",\"mac0\":\"\",\"mac1\":\"\",\"dspvsn\":\"\",\"upgdspvsn\":\"\",\"fgpavsn\":\"\",\"upgfgpavsn\":\
                \"\",\"upgstate\",\"up0\":\"\",\"down0\":\"\",\"up1\":\"\",\"down1\":\"\",\"avaipos\":\"\",\"bakstate\":\"\",\"workstate\":\"\",\"bakworkstate\
                \":\"\"}]}";
    }
    break;
    case 122:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 123:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 124:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"region\":\"\",\"state\":\"\",\"index\":\"\",\
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
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 126:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 127:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 128:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 129:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 130:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 131:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 132:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"id\":\"\",\"name\":\"\"}]}";
    }
    break;
    case 133:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"count\":\"\"}}";
    }
    break;
    case 700:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"gid\":\"\",\"pid\":\"\",\"gradeid\":\"\",\"groupname\":\"\",\"leaderid\":\"\",\"leader\":\"\",\"bz\":\"\"}]}";
    }
    break;
    case 701:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 702:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"gid\":\"\",\"pid\":\"\",\"gradeid\":\"\",\"groupname\":\"\",\"leaderid\":\"\",\"leader\":\"\",\"bz\":\"\"}]}";
    }
    break;
    case 703:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 704:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 705:
    {
        msg = "{\"Msg\":\"\"}";
    }
    break;
    case 706:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"gid\":\"\",\"devid\":\"\",\"devname\":\"\",\"devtype\":\"\",\"devrole\":\"\",\"devno\":\"\",\
                \"devport\":\"\",\"routeflg\":\"\"}]}";
        break;
    }
    case 707:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 708:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 140:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"sn\":\"\",\"no\":\"\",\"globalno\":\"\",\
                \"v2vno\":\"\",\"logid\":\"\",\"ip\":\"\",\"logtype\":\"\",\"mac\":\"\",\"phytype\":\"\",\"sub\":\"\",\"sheng\":\"\",\"shi\":\
                \"\",\"qx\":\"\",\"usemode\":\"\"}]}";
        break;
    }
    case 141:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"sn\":\"\",\"no\":\"\",\"globalno\":\"\",\"v2vno\"\
                :\"\",\"logid\":\"\",\"logtype\":\"\",\"mac\":\"\",\"ip\":\"\",\"phytype\":\"\",\"sub\":\"\",\"sheng\":\"\",\"shi\":\"\",\"qx\":\"\",\
                \"state\":\"\",\"servicetype\":\"\",\"lgcdevnum\":\"\",\"endtime\":\"\",\"softvsn\":\"\",\"upgsoftvsn\":\"\",\"menuvsn\":\"\",\"upgmenuvsn\":\
                \"\",\"kernel\":\"\",\"upgkernel\":\"\",\"filesys\":\"\",\"upgfilesys\":\"\",\"font\":\"\",\"upgfont\":\"\",\"upgstate\":\"\",\"upgvsntm\":\
                \"\",\"up\":\"\",\"down\":\"\",\"lossrate\":\"\",\"usemode\":\"\"}]}";
        break;
    }
    case 142:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 143:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 144:
    {
        msg = "{\"id\":\"\",\"endid\":\"\",\"svrid\":\"\",\"name\":\"\",\"usemode\":\"\",\"sn\":\"\",\"state\":\"\",\"softvsn\":\"\",\"upgsoftvsn\":\
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
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 146:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 147:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 148:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 149:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 150:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 151:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 152:
    {
        msg = "{\"Msg\":\"\",\"version\":\"\",\"type\":\"\"}";
        break;
    }
    case 153:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 154:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 155:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 156:
    {
        msg = "{\"Msg\":\"\",\"config\":\"\"}";
        break;
    }
    case 157:
    {
        msg = "{\"Msg\":\"\",\"version\":\"\",\"state\":\"\",\"ip\":\"\",\"gateway\":\"\",\"mask\":\"\"}";
        break;
    }
    case 158:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 159:
    {
        msg = "{\"Msg\":\"\",\"count\":\"\",\"Data\":[{\"svrid\":\"\",\"svrname\":\"\",\"teamname\":\"\",\"teamtype\":\"\",\"teamid\":\"\",\"acid\":\
                \"\",\"acname\":\"\",\"name\":\"\",\"id\":\"\",\"mac\":\"\",\"sn\":\"\",\"devno\":\"\",\"ktywqf\":\"\",\"devtype\":\"\",\"khmc\":\"\"}]}";
        break;
    }
    case 281:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 282:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 283:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 284:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"devname\":\"\",\"devno\":\"\",\"globaldevno\":\"\",\"devsubno\":\"\"}]}";
        break;
    }
    case 286:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"count\":\"\"}}";
        break;
    }
    case 287:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"name\":\"\",\"sn\":\"\",\"no\":\"\",\"globalno\":\"\",\"v2vno\":\"\",\
                \"logid\":\"\",\"devtype\":\"\",\"mac\":\"\",,\"sheng\":\"\",\"shi\":\"\",\"qx\":\"\"，\"state\":\"\",\"ut\":\"\"}]}";
        break;
    }
    case 288:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 289:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 290:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 291:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 292:
    {
        msg = "{\"Msg\":\"\"，\"Data\":{\"svrcount\":\"\",\"sucscount\":\"\",\"failcount\":\"\"}}";
        break;
    }
    case 293:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"acscmsname\":\"\",\"devname\":\"\",\"gatewayname\":\"\",\"devno\":\"\",\"bridgename\":\"\",\"stgdevno\":\"\",\
                \"ispmr\":\"\",\"isencrypt\":\"\",\"istelnet\":\"\",\"isshow\":\"\",\"islogin\":\"\",\"isshowdevsec\":\"\",\"isshowywsec\":\"\",\"isauxiliary\
                \":\"\",\"secretlevel\":\"\",\"mcname\",\"mcno\":\"\",\"mcsubno\":\"\"}]}";
        break;
    }
    case 294:
    {
        msg = "{\"Msg\":\"\",\"Data\":[{\"name\":\"\",\"devtype\":\"\",\"devno\":\"\",\"authstate\":\"\",\"authtime\":\"\"}]}";
        break;
    }
    case 295:
    {
        msg = "{\"Msg\":\"\",\"Data\":{\"servicetype\":\"\",\"softvsn\":\"\",\"menuvsn\":\"\",\"kernel\":\"\",\"filesys\":\"\",\"font\":\"\",\"model\":\
                \"\",\"upgstate\":\"\",\"upgvsntm\":\"\", \"ip\":\"\",\"gateway\":\"\":\"submask\"：\"\",\"upflow\":\"\",\"downflow\":\"\",\"lossratevalue\":\
                \"\",\"videolossratevalue\":\"\",\"audiolossratevalue\":\"\",\"cpuvalue\":\"\",\"memoryvalue\":\"\",\"mcno\":\"\",\"mcsubno\":\"\"}}";
        break;
    }
    case 296:
    {
        msg = "{\"Msg\":\"\",\"cpu\":\"\",\"cputhreshold\":\"\",\"memory\":\"\",\"memorythreshold\":\"\",\"lossrate\":\"\",\"videolossrate\":\"\",\
                \"audiolossrate\":\"\",\"lossvideo\":\"\",\"lossaudio\":\"\":\"reboot\"：\"\",\"upgradefailed\":\"\",\"cardexception\":\"\",\"reporttvmask\":\
                \"\",\"inputsourceloss\":\"\",\"repeatpkgrate\":\"\"}";
        break;
    }
    case 300:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"subno\":\"\",\"servicetype\":\"\"}]}";
        break;
    }
    case 230:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 231:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"endid\":\"\",\"svrid\":\"\",\"name\":\"\",\"devtypename\":\"\",\"devicetype\":\"\":\"sn\":\
                \"\",\"mac\":\"\",\"svrname\":\"\",\"custmname\":\"\",\"devno\":\"\",\"globaldevno\":\"\",\"v2vno\":\"\",\"state\":\"\",\"teamname\":\"\",\
                \"teamtypename\":\"\",\"softvsn\":\"\",\"upgsoftvsn\":\"\",\"menuvsn\":\"\",\"upgmenuvsn\":\"\",\"kernel\":\"\",\"upgkernel\":\"\",\"filesys\":\
                \"\",\"upgfilesys\":\"\",\"font\":\"\",\"upgfont\":\"\",\"upgstate\":\"\",\"upgvsntm\":\"\",\"remarks\":\"\",\"secretlevel\":\"\"}]}";
        break;
    }
    case 232:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 233:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 501:
    {
        msg = "{\"Msg\":\"\",\"count\":,\"Data\":[{\"id\":\"\",\"name\":\"\",\"fuzeren\":\"\",\"phone\":\"\",\"v2vno\":\"\",\"mac\":\"\",\"createtime\":\"\",\"bz\":\"\"}]}";
        break;
    }
    case 502:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 503:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    case 504:
    {
        msg = "{\"Msg\":\"\"}";
        break;
    }
    }
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    //    write->len = htons(len);
    writeDevHead->len = (short)msg.length();
    writeDevHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    std::string data = std::string(write_buffer_, sizeof(Overload) + len);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);

    //}
    return 0;
}

/* UserManager 0x6021 */
int UserManager::dataProcess(void *client, const char *buff)
{
    /*    Overload *overload = (Overload *)buff;
        overload->tag = ntohs(overload->tag);
        overload->len = ntohs(overload->len);
        std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
        if (overload->tag == 0x6021)
        {
            DeviceMngHead *deviceHead = (DeviceMngHead *)(buff + sizeof(Overload));
            */
    DeviceMngHead *deviceHead = (DeviceMngHead *)(buff);
    deviceHead->taskNo = ntohl(deviceHead->taskNo);
    deviceHead->deviceNo = ntohl(deviceHead->deviceNo);
    deviceHead->cmdType = ntohs(deviceHead->cmdType);
    deviceHead->cmd = ntohl(deviceHead->cmd);
    deviceHead->ret = ntohs(0);
    std::cout << "cmd:" << deviceHead->cmd << std::endl;

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
    std::string msg = "";
    int endtag = 0xeeeeeeee;
    int len = 0;
    char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
    switch (deviceHead->cmd)
    {
    case 101:
    {
        msg = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
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
        msg = "{\
                \"ret\":0,\
                \"msg\" : \"用户退出成功\",\
                }  ";
    }
    break;
    case 131:
    {
        msg = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
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
        msg = "{\
                \"ret\": 0,\
                \"msg\" : \"修改成功\"\
                }";
    }
    break;
    case 151:
    {
        msg = "{\
                \"ret\": 0,\
                \"msg\" : \"删除成功\"\
                }";
    }
    break;
    case 161:
    {
        msg = "{ \"access_token\": \"4e29de6b9c3511e9be51a4bf01303dd7\", \"data\" :   { \"loginName\": \"bao\", \"name\" : \"王方军\",\
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
    len = sizeof(DeviceMngHead) + msg.length() + 4;
    //    write->len = htons(len);
    writeDevHead->len = (short)msg.length();
    writeDevHead->len = htons(deviceHead->len);
    memcpy(json, msg.c_str(), msg.length());
    memcpy(json + msg.length(), &endtag, 4);
    std::string data = std::string(write_buffer_, sizeof(Overload) + len);
    (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);

    //}
    return 0;
}

/* AuthManager 0x6022 */
int AuthManager::dataProcess(void *client, const char *buff)
{
    Overload *overload = (Overload *)buff;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6022)
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
        std::string msg = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 1:
        {
            msg = "{\
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
            msg = "{\
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
        len = sizeof(DeviceMngHead) + msg.length() + 4;
        write->len = htons(len);
        writeDevHead->len = (short)msg.length();
        writeDevHead->len = htons(deviceHead->len);
        memcpy(json, msg.c_str(), msg.length());
        memcpy(json + msg.length(), &endtag, 4);
        std::string data = std::string(write_buffer_, sizeof(Overload) + len);
        (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
    }
    return 0;
}

/* ConferenceManager 0x6023 */
int ConferenceManager::dataProcess(void *client, const char *buff)
{
    Overload *overload = (Overload *)buff;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6023)
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
        std::string msg = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 601:
        {
            msg = "{\"Msg\":\"\"}";
            break;
        }
        case 602:
        {
            msg = "{\"Msg\":\"\"}";
            break;
        }
        case 603:
        {
            msg = "{\"Msg\":\"\"}";
            break;
        }
        }
        len = sizeof(DeviceMngHead) + msg.length() + 4;
        write->len = htons(len);
        writeDevHead->len = (short)msg.length();
        writeDevHead->len = htons(deviceHead->len);
        memcpy(json, msg.c_str(), msg.length());
        memcpy(json + msg.length(), &endtag, 4);
        std::string data = std::string(write_buffer_, sizeof(Overload) + len);
        (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);

    }
    return 0;
}

/* UpgradeManager 0x6024 */
int UpgradeManager::dataProcess(void *client, const char *buff)
{
    Overload *overload = (Overload *)buff;
    overload->tag = ntohs(overload->tag);
    overload->len = ntohs(overload->len);
    std::cout << "msg tag: " << overload->tag << " len:" << overload->len << std::endl;
    if (overload->tag == 0x6024)
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
        std::string msg = "";
        int endtag = 0xeeeeeeee;
        int len = 0;
        char* json = (char*)(write_buffer_ + sizeof(Overload) + sizeof(DeviceMngHead));
        switch (deviceHead->cmd)
        {
        case 703:
        {
            msg = "{\"ret\":\"\",\"Msg\":\"\"}";
            break;
        }
        case 704:
        {
            msg = "{\"list\":[{\"file_name\":\"\",\
                \"product\":\"\",\
                \"version\":\"\",\
                \"datetime\":\"\"}],\"Msg\":\"\"}";
            break;
        }
        case 705:
        {
            msg = "{\"ret\":\"\",\"Msg\":\"\"}";
            break;
        }
        case 706:
        {
            msg = "{\"ret\":\"\",\"Msg\":\"\"}";
            break;
        }
        case 707:
        {
            msg = "{\"ret\":\"\",\"Msg\":\"\"}";
            break;
        }
        }
        len = sizeof(DeviceMngHead) + msg.length() + 4;
        write->len = htons(len);
        writeDevHead->len = (short)msg.length();
        writeDevHead->len = htons(deviceHead->len);
        memcpy(json, msg.c_str(), msg.length());
        memcpy(json + msg.length(), &endtag, 4);
        std::string data = std::string(write_buffer_, sizeof(Overload) + len);
        (*(Conn_ptr*)client)->do_write(write_buffer_, sizeof(Overload) + len);
    }
    return 0;
}

/* ConfigManager 0x6025 */
int ConfigManager::dataProcess(void *client, const char *buff)
{

    return 0;
}

