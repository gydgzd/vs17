//#include "stdafx.h"
#include "testNDIS.h"

#ifdef _MSC_VER
//#pragma comment(lib, "advapi32.lib")
//#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "setupapi.lib")
#endif

struct device_instance_id_interface
{
    const char *net_cfg_instance_id;
    const char *device_interface_list;
    struct device_instance_id_interface *next;
};

testNDIS::testNDIS()
{

}


testNDIS::~testNDIS()
{

}

int testNDIS::init(const char *devname, const char *ipaddr, const char *netmask, const char *gateway)
{
    return 0;
}
/**/
int testNDIS::dev_alloc(char *dev, short flags)
{
    HDEVINFO dev_info_set;           // 设备类型的句柄，后续使用该句柄进行多种查找
    DWORD err;
    struct device_instance_id_interface *first = NULL;
    struct device_instance_id_interface *last = NULL;

    dev_info_set = SetupDiGetClassDevsEx(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT, NULL, NULL, NULL);
    if (dev_info_set == INVALID_HANDLE_VALUE)
    {
        printError_Win("SetupDiGetClassDevsEx");
    }

    SP_DEVINFO_DATA device_info_data;
    BOOL res;
    HKEY dev_key;
    for (DWORD i = 0;; ++i)
    {
        TCHAR net_cfg_instance_id_string[] = _T("NetCfgInstanceId");
        char net_cfg_instance_id[256];
        TCHAR device_instance_id[256] = {};
        DWORD len;
        DWORD data_type;
        LONG status;
        ULONG dev_interface_list_size;
        CONFIGRET cr;
        TCHAR *dev_interface_list = NULL;

        ZeroMemory(&device_info_data, sizeof(SP_DEVINFO_DATA));
        device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
        res = SetupDiEnumDeviceInfo(dev_info_set, i, &device_info_data);   // 枚举设备信息,  可以直接用SetupDiEnumDeviceInterfaces
        if (!res)
        {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
            {
                break;
            }
            else
            {
                continue;
            }
        }
        // 打开注册表项，获取特定设备的配置信息
        dev_key = SetupDiOpenDevRegKey(dev_info_set, &device_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_QUERY_VALUE);
        if (dev_key == INVALID_HANDLE_VALUE)
        {
            printError_Win("SetupDiOpenDevRegKey");
            continue;
        }
        len = sizeof(net_cfg_instance_id);
        data_type = REG_SZ;
        status = RegQueryValueEx(dev_key, net_cfg_instance_id_string, NULL, &data_type, (unsigned char *)net_cfg_instance_id, &len);
        if (status != ERROR_SUCCESS)
        {
            printError_Win("RegQueryValueEx");
            goto next;
        }

        len = sizeof(device_instance_id);
        res = SetupDiGetDeviceInstanceId(dev_info_set, &device_info_data, device_instance_id, len, &len);
        if (!res)
        {
            printError_Win("SetupDiGetDeviceInstanceId");
            goto next;
        }

        cr = CM_Get_Device_Interface_List_Size(&dev_interface_list_size, (LPGUID)& GUID_DEVINTERFACE_NET, device_instance_id, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (cr != CR_SUCCESS)
        {
            printError_Win("CM_Get_Device_Interface_List_Size");
            goto next;//return FALSE;
        }

        dev_interface_list = (TCHAR *)calloc(sizeof(*dev_interface_list), dev_interface_list_size);

        cr = CM_Get_Device_Interface_List((LPGUID)& GUID_DEVINTERFACE_NET, device_instance_id, dev_interface_list, dev_interface_list_size, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
        if (cr != CR_SUCCESS)
        {
            printError_Win("CM_Get_Device_Interface_List");
            goto next;
        }
        printf_s("interface name: %ls\n", dev_interface_list);
        
        free(dev_interface_list);
        /*
        struct device_instance_id_interface* dev_if;
        dev_if = (device_instance_id_interface*)calloc(sizeof(device_instance_id_interface), dev_interface_list_size);
        dev_if->net_cfg_instance_id = (char *)calloc(strlen(net_cfg_instance_id) + 1, dev_interface_list_size);
        dev_if->device_interface_list = (char *)calloc(wcslen(dev_interface_list) + 1, dev_interface_list_size);
        // link into return list 
        if (!first)
        {
            first = dev_if;
        }
        if (last)
        {
            last->next = dev_if;
        }
        last = dev_if;
*/
        HANDLE WintunHandle = CreateFile(dev_interface_list, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL, OPEN_EXISTING, 0, NULL);
    next:
        RegCloseKey(dev_key);
    }

    SetupDiDestroyDeviceInfoList(dev_info_set);

    return 0;
}

void testNDIS::printError_Win(const char * msg)
{
    DWORD eNum;
    TCHAR sysMsg[256] = _T("");
    TCHAR* p;
    LPVOID lpMsgBuf;
    eNum = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, eNum,
        0, //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf, 0, NULL);

    // Trim the end of the line and terminate it with a null
    p = sysMsg;

    while ((*p > 31) || (*p == 9))
        ++p;
    do { *p-- = 0; } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

    // Display the message
    USES_CONVERSION;
    printf("ERROR: %s failed with error %d - %s", msg, eNum, T2A((LPCTSTR)lpMsgBuf));
 //   LOG(ERROR) << msg << " failed with error " << eNum << " - " << CDevInfo::AnsiToUtf8((LPCTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);

}
