#pragma once
//// data:  Overload + DeviceMngHead + json + 0xEEEEEEEE
////   or   Overload + ConferenceMngHead + json
struct Overload
{
    unsigned short tag;
    unsigned short len;
};

#pragma pack(push,1)
struct DeviceMngHead
{
    unsigned int begin;         // 开始标识 
    int taskNo;                 // 任务编号
    int deviceNo;               // 设备编号
    short cmdType;              // 命令类型
    int cmd;                    // 命令
    short ret;                  // 返回值
    int len;                  // 数据长度
    DeviceMngHead()
    {
        memset(this, 0, sizeof(*this));
        begin = htonl(0xFFFFFFFF);
    }
};

struct ConferenceMngHead
{
    unsigned char version[4];        //版本号
    unsigned short cmdType;          //消息类型
    unsigned short cmd;              //消息信令
    unsigned int len;                //消息长度
    unsigned long long id;           //消息号
    unsigned short total;            //消息总包
    unsigned short index;            //包索引
    ConferenceMngHead()
    {
        memset(this, 0, sizeof(*this));
        version[0] = 0x01;
        version[1] = 0x02;
        version[2] = 0x08;
        version[3] = 0x05;
    }
};
#pragma pack(pop)