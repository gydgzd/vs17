#pragma once
/*
https://www.wintun.net/
https://git.zx2c4.com/wintun/about/
https://github.com/OpenVPN/openvpn/blob/master/src/openvpn/tun.c
https://github.com/OpenVPN/Windows-driver-samples
https://docs.microsoft.com/zh-cn/windows-hardware/drivers/download-the-wdk
tap-windows6(NDIS6)编译：https://blog.csdn.net/weixin_42057209/article/details/94981569
https://www.npmjs.com/package/makensis
在注册表项找到各种设备类GUID:
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class            // 该项中找到的无效
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
*/

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
//
#include <Winsock2.h>
#include <WS2tcpip.h>  // inet_ntop
//#include <ws2def.h>  // should use #include <Winsock2.h>
#include <WinDef.h>    // _WIN32
#include <setupapi.h>
//#include <ws2ipdef.h>
#include <tchar.h>
#include <Windows.h>  // have #include <winsock.h>
#include <winioctl.h>
//

#include <cfgmgr32.h>
#include <atlstr.h>     // DWORD     , MFC use #include <cstringt.h>
//#include<atlconv.h>    // for T2A
//#include <objbase.h>
//#include <NtDDNdis.h>
/**/
#include <IPTypes.h>
#include <IPHlpApi.h>
#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include "tuntap.h"
#include "Mylog.h"
#endif
using namespace std;

extern Mylog g_mylog;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MAX_TRIES 3

const static GUID GUID_DEVCLASS_NET = { 0x4d36e972L, 0xe325, 0x11ce,{ 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };
const static GUID GUID_DEVINTERFACE_NET = { 0xcac88484, 0x7515, 0x4c03,{ 0x82, 0xe6, 0x71, 0xa8, 0x7a, 0xba, 0xc3, 0x61 } };

struct ProxyInfo
{
    std::string os;
    std::string tap_mac;
    std::string v2v_mac;
    std::string virtual_number;
    std::string status;        // on/off
    int srcPort;               // local port
    int dstPort;
    int hdr;
    int connect;               // connected: 1; not connected: 0
    int isSameAC;              // same: 1; not same:0
    int isCalled;              // be called: 1; caller: 0;               prepare
    int isRecovered;
    int retryCount;
    ProxyInfo()
    {
        srcPort = 0;
        dstPort = 0;
        hdr     = 0;
        connect = 0;
        isSameAC = -1;
        isCalled = -1;
        isRecovered = 1;
        retryCount = 0;
    }
};

class myTunTap
{
public:
    myTunTap();
    virtual ~myTunTap();

    int init(const char *devname, const char *ipaddr, const char *netmask, const char *gateway, const int mtu);
    int dev_getDevid(TCHAR *devid, TCHAR *devname);
    int dev_open(TCHAR *devid);
    int dev_config(const char *ipaddr, const char *netmask, const char *gateway);
    int dev_read(struct tuntap *tt, int maxsize);
    int dev_write(struct tuntap *tt, struct buffer *buf);

    static inline void
        read_wintun(struct tuntap *tt, struct buffer* buf);
    static inline int
        write_wintun(struct tuntap *tt, struct buffer *buf);

    int myread(char *buffer, unsigned int size);

    int mywrite(char *buffer, unsigned int size);
    static std::string myTunTap::Utf8ToGbk(const char *src_str);
    static void printError_Win(const char* msg);
    int getLocalMac(char* device_guid);
    char * returnTunMAC();
    
    
    unsigned short checksum(unsigned char *buf, int len);
    int process();
    TCHAR m_device_guid[256];     // net_cfg_instance_id (Adapter name)
private:
    int m_fd;
    DWORD m_index;
    char m_devName[32];

    char msz_tun_Name[16];        // TUN/TAP
    char msz_tun_IP[16];          // TUN/TAP
    char msz_tun_NETMASK[16];     // TUN/TAP
    char msz_tun_GATEWAY[16];     // TUN/TAP
    char msz_tun_MAC[32];         // TUN/TAP
    int  mn_MTU;

    TCHAR m_device_path[256];     // device_instance_id
    TCHAR m_device_name[256];     // dev_interface_list
    TCHAR m_friendlyName[256];    // friendly name
    HANDLE m_WintunHandle;

    OVERLAPPED m_read_overlapped;
    OVERLAPPED m_write_overlapped;
    //mylog m_mylog;
};

