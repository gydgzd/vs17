#pragma once
/*
 https://www.wintun.net/
 https://git.zx2c4.com/wintun/about/
 https://github.com/OpenVPN/openvpn/blob/master/src/openvpn/tun.c
 https://github.com/OpenVPN/Windows-driver-samples
 https://docs.microsoft.com/zh-cn/windows-hardware/drivers/download-the-wdk
 tap-windows6(NDIS6)���룺https://blog.csdn.net/weixin_42057209/article/details/94981569
                          https://www.npmjs.com/package/makensis
 ��ע������ҵ������豸��GUID: 
       HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class            // �������ҵ�����Ч
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

#include "tuntap.h"


#endif

using namespace std;


const static GUID GUID_DEVCLASS_NET = { 0x4d36e972L, 0xe325, 0x11ce,{ 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };
const static GUID GUID_DEVINTERFACE_NET = { 0xcac88484, 0x7515, 0x4c03,{ 0x82, 0xe6, 0x71, 0xa8, 0x7a, 0xba, 0xc3, 0x61 } };

class testNDIS
{
public:
    testNDIS();
    virtual ~testNDIS();

    int init(const char *devname, const char *ipaddr, const char *netmask, const char *gateway);
    int dev_getDevid(TCHAR *devid, TCHAR *devname);
    int dev_open(TCHAR *devid);
    int dev_config(const char *ipaddr, const char *netmask, const char *gateway);
    int dev_read(struct tuntap *tt, int maxsize);
    int dev_write(struct tuntap *tt, struct buffer *buf);

    static inline void
        read_wintun(struct tuntap *tt, struct buffer* buf);
    static inline int
        write_wintun(struct tuntap *tt, struct buffer *buf);

    int myread();

    int mywrite();

    static void printError_Win(const char* msg);
private:
    int m_fd;
    DWORD m_index;
    char m_devName[32];

    char msz_tun_Name[16];        // TUN/TAP
    char msz_tun_IP[16];          // TUN/TAP
    char msz_tun_NETMASK[16];     // TUN/TAP
    char msz_tun_GATEWAY[16];     // TUN/TAP

    TCHAR m_device_guid[256];     // net_cfg_instance_id
    TCHAR m_device_path[256];     // device_instance_id
    TCHAR m_device_name[256];     // dev_interface_list

    HANDLE m_WintunHandle;

    OVERLAPPED m_read_overlapped;
    OVERLAPPED m_write_overlapped;
};

