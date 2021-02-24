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
    unsigned int begin;         // ��ʼ��ʶ 
    int taskNo;                 // ������
    int deviceNo;               // �豸���
    short cmdType;              // ��������
    int cmd;                    // ����
    short ret;                  // ����ֵ
    int len;                  // ���ݳ���
    DeviceMngHead()
    {
        memset(this, 0, sizeof(*this));
        begin = htonl(0xFFFFFFFF);
    }
};

struct ConferenceMngHead
{
    unsigned char version[4];        //�汾��
    unsigned short cmdType;          //��Ϣ����
    unsigned short cmd;              //��Ϣ����
    unsigned int len;                //��Ϣ����
    unsigned long long id;           //��Ϣ��
    unsigned short total;            //��Ϣ�ܰ�
    unsigned short index;            //������
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