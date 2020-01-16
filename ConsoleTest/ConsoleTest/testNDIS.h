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
#include <Windows.h>
#include <WinDef.h>
#include <winioctl.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <atlstr.h>     // DWORD     , MFC use #include <cstringt.h>
#include<atlconv.h>    // for T2A
//#include <objbase.h>
//#include <NtDDNdis.h>
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
    int dev_alloc(char *dev, short flags);
    int dev_read(char *buffer, unsigned int size);
    int dev_write(char *buffer, unsigned int size);

    static void printError_Win(const char* msg);
private:
    int m_fd;
    char m_devName[32];

    char msz_tun_Name[16];        // TUN/TAP
    char msz_tun_IP[16];          // TUN/TAP
    char msz_tun_NETMASK[16];     // TUN/TAP
    char msz_tun_GATEWAY[16];     // TUN/TAP

};

