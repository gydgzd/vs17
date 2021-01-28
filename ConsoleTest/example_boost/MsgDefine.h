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
    unsigned int begin;
    int taskNo;
    int deviceNo;
    short cmdType;
    int cmd;
    short ret;
    short len;
    DeviceMngHead()
    {
        memset(this, 0, sizeof(*this));
        begin = htonl(0xFFFFFFFF);
    }
};

struct ConferenceMngHead
{
    int   version;
    short cmdType;
    short cmd;
    int   len;
    double seqNo;
    short totalNum;
    short currNum;

};
#pragma pack(pop)