#include "stdafx.h"
#include "myTunTap.h"
//#include "stdafx.h"

#ifdef _MSC_VER
//#pragma comment(lib, "advapi32.lib")
//#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "IPHlpApi.lib")
#endif

/**
* Garbage collection arena used to keep track of dynamically allocated
* memory.
*
* This structure contains a linked list of \c gc_entry structures.  When
* a block of memory is allocated using the \c gc_malloc() function, the
* allocation is registered in the function's \c gc_arena argument.  All
* the dynamically allocated memory registered in a \c gc_arena can be
* freed using the \c gc_free() function.
*/
struct gc_arena
{
    struct gc_entry *list;      /**< First element of the linked list of
                                *   \c gc_entry structures. */
    struct gc_entry_special *list_special;
};
static void delete_temp_addresses(DWORD index);
void *
#ifdef DMALLOC
gc_malloc_debug(size_t size, bool clear, struct gc_arena *a, const char *file, int line)
#else
gc_malloc(size_t size, bool clear, struct gc_arena *a)
#endif
{
    void *ret;
    if (a)
    {
        struct gc_entry *e;
#ifdef DMALLOC
        e = (struct gc_entry *) openvpn_dmalloc(file, line, size + sizeof(struct gc_entry));
#else
        e = (struct gc_entry *) malloc(size + sizeof(struct gc_entry));
#endif
        check_malloc_return(e);
        ret = (char *)e + sizeof(struct gc_entry);
        e->next = a->list;
        a->list = e;
    }
    else
    {
#ifdef DMALLOC
        ret = openvpn_dmalloc(file, line, size);
#else
        ret = malloc(size);
#endif
        check_malloc_return(ret);
    }
#ifndef ZERO_BUFFER_ON_ALLOC
    if (clear)
#endif
        memset(ret, 0, size);
    return ret;
}

void
x_gc_free(struct gc_arena *a)
{
    struct gc_entry *e;
    e = a->list;
    a->list = NULL;

    while (e != NULL)
    {
        struct gc_entry *next = e->next;
        free(e);
        e = next;
    }
}

/*
* Functions to handle special objects in gc_entries
*/

void
x_gc_freespecial(struct gc_arena *a)
{
    struct gc_entry_special *e;
    e = a->list_special;
    a->list_special = NULL;

    while (e != NULL)
    {
        struct gc_entry_special *next = e->next;
        e->free_fnc(e->addr);
        free(e);
        e = next;
    }
}
static inline bool
gc_defined(struct gc_arena *a)
{
    return a->list != NULL;
}

static inline void
gc_init(struct gc_arena *a)
{
    a->list = NULL;
    a->list_special = NULL;
}

static inline void
gc_detach(struct gc_arena *a)
{
    gc_init(a);
}

static inline struct gc_arena
gc_new(void)
{
    struct gc_arena ret;
    gc_init(&ret);
    return ret;
}

static inline void
gc_free(struct gc_arena *a)
{
    if (a->list)
    {
        x_gc_free(a);
    }
    if (a->list_special)
    {
        x_gc_freespecial(a);
    }
}
static inline void
gc_reset(struct gc_arena *a)
{
    gc_free(a);
}

/*
* Allocate memory to hold a structure
*/

#define ALLOC_OBJ(dptr, type) \
    { \
        check_malloc_return((dptr) = (type *) malloc(sizeof(type))); \
    }

#define ALLOC_OBJ_CLEAR(dptr, type) \
    { \
        ALLOC_OBJ(dptr, type); \
        memset((dptr), 0, sizeof(type)); \
    }

#define ALLOC_ARRAY(dptr, type, n) \
    { \
        check_malloc_return((dptr) = (type *) malloc(array_mult_safe(sizeof(type), (n), 0))); \
    }

#define ALLOC_ARRAY_GC(dptr, type, n, gc) \
    { \
        (dptr) = (type *) gc_malloc(array_mult_safe(sizeof(type), (n), 0), false, (gc)); \
    }

#define ALLOC_ARRAY_CLEAR(dptr, type, n) \
    { \
        ALLOC_ARRAY(dptr, type, n); \
        memset((dptr), 0, (array_mult_safe(sizeof(type), (n), 0))); \
    }

#define ALLOC_ARRAY_CLEAR_GC(dptr, type, n, gc) \
    { \
        (dptr) = (type *) gc_malloc(array_mult_safe(sizeof(type), (n), 0), true, (gc)); \
    }

#define ALLOC_VAR_ARRAY_CLEAR_GC(dptr, type, atype, n, gc)      \
    { \
        (dptr) = (type *) gc_malloc(array_mult_safe(sizeof(atype), (n), sizeof(type)), true, (gc)); \
    }

#define ALLOC_OBJ_GC(dptr, type, gc) \
    { \
        (dptr) = (type *) gc_malloc(sizeof(type), false, (gc)); \
    }

#define ALLOC_OBJ_CLEAR_GC(dptr, type, gc) \
    { \
        (dptr) = (type *) gc_malloc(sizeof(type), true, (gc)); \
    }
#define PACKAGE_NAME "v2vproxy"
void out_of_memory(void)
{
    fprintf(stderr, "v2vproxy" ": Out of Memory\n");
    exit(1);
}
static inline void check_malloc_return(const void *p)
{
    if (!p)
    {
        out_of_memory();
    }
}

static inline bool
buf_write_u8(struct buffer* dest, int data)
{
    uint8_t u8 = (uint8_t)data;
    return buf_write(dest, &u8, sizeof(uint8_t));
}

static inline bool
buf_write_u16(struct buffer* dest, int data)
{
    uint16_t u16 = htons((uint16_t)data);
    return buf_write(dest, &u16, sizeof(uint16_t));
}

static inline bool
buf_write_u32(struct buffer* dest, int data)
{
    uint32_t u32 = htonl((uint32_t)data);
    return buf_write(dest, &u32, sizeof(uint32_t));
}

/*
 * Convert DHCP options from the command line / config file
 * into a raw DHCP-format options string.
 */

static void
write_dhcp_u8(struct buffer* buf, const int type, const int data, bool* error)
{
    if (!buf_safe(buf, 3))
    {
        *error = true;
        printf("write_dhcp_u8: buffer overflow building DHCP options\n");
        return;
    }
    buf_write_u8(buf, type);
    buf_write_u8(buf, 1);
    buf_write_u8(buf, data);
}

static void
write_dhcp_u32_array(struct buffer* buf, const int type, const uint32_t* data, const unsigned int len, bool* error)
{
    if (len > 0)
    {
        unsigned int i;
        const int size = len * sizeof(uint32_t);

        if (!buf_safe(buf, 2 + size))
        {
            *error = true;
            printf("write_dhcp_u32_array: buffer overflow building DHCP options\n");
            return;
        }
        if (size < 1 || size > 255)
        {
            *error = true;
            printf("write_dhcp_u32_array: size (%d) must be > 0 and <= 255\n", size);
            return;
        }
        buf_write_u8(buf, type);
        buf_write_u8(buf, size);
        for (i = 0; i < len; ++i)
        {
            buf_write_u32(buf, data[i]);
        }
    }
}

static void
write_dhcp_str(struct buffer* buf, const int type, const char* str, bool* error)
{
    const int len = strlen(str);
    if (!buf_safe(buf, 2 + len))
    {
        *error = true;
        printf("write_dhcp_str: buffer overflow building DHCP options\n");
        return;
    }
    if (len < 1 || len > 255)
    {
        *error = true;
        printf("write_dhcp_str: string '%s' must be > 0 bytes and <= 255 bytes\n", str);
        return;
    }
    buf_write_u8(buf, type);
    buf_write_u8(buf, len);
    buf_write(buf, str, len);
}

static bool
build_dhcp_options_string(struct buffer* buf, const struct tuntap_options* o)
{
    bool error = false;
    if (o->domain)
    {
        write_dhcp_str(buf, 15, o->domain, &error);
    }

    if (o->netbios_scope)
    {
        write_dhcp_str(buf, 47, o->netbios_scope, &error);
    }

    if (o->netbios_node_type)
    {
        write_dhcp_u8(buf, 46, o->netbios_node_type, &error);
    }

    write_dhcp_u32_array(buf, 6, (uint32_t*)o->dns, o->dns_len, &error);
    write_dhcp_u32_array(buf, 44, (uint32_t*)o->wins, o->wins_len, &error);
    write_dhcp_u32_array(buf, 42, (uint32_t*)o->ntp, o->ntp_len, &error);
    write_dhcp_u32_array(buf, 45, (uint32_t*)o->nbdd, o->nbdd_len, &error);

    /* the MS DHCP server option 'Disable Netbios-over-TCP/IP
     * is implemented as vendor option 001, value 002.
     * A value of 001 means 'leave NBT alone' which is the default */
    if (o->disable_nbt)
    {
        if (!buf_safe(buf, 8))
        {
            printf( "build_dhcp_options_string: buffer overflow building DHCP options\n");
            return false;
        }
        buf_write_u8(buf, 43);
        buf_write_u8(buf, 6);/* total length field */
        buf_write_u8(buf, 0x001);
        buf_write_u8(buf, 4);/* length of the vendor specified field */
        buf_write_u32(buf, 0x002);
    }
    return !error;
}
struct device_instance_id_interface
{
    const char *net_cfg_instance_id;
    const char *device_interface_list;
    struct device_instance_id_interface *next;
};

myTunTap::myTunTap()
{
    m_index = 0;
    m_WintunHandle = NULL;
    memset(&m_read_overlapped, 0, sizeof(m_read_overlapped));
    m_read_overlapped.Offset = 0;
    m_read_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_read_overlapped.hEvent == NULL)
    {
        printError_Win("Error: overlapped_io_init: CreateEvent m_read_overlapped failed");
    }
    memset(&m_write_overlapped, 0, sizeof(m_write_overlapped));
    m_write_overlapped.Offset = 0;
    /* manual reset event, initially set according to event_state */
    m_write_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_write_overlapped.hEvent == NULL)
    {
        printError_Win("Error: overlapped_io_init: CreateEvent m_write_overlapped failed");
    }
}


myTunTap::~myTunTap()
{
    delete_temp_addresses(m_index);
    if (m_WintunHandle != NULL)
    {
        CloseHandle(m_WintunHandle);
        m_WintunHandle = NULL;
    }
}

int myTunTap::init(const char *devname, const char *ipaddr, const char *netmask, const char *gateway, const int mtu)
{
    strcpy(msz_tun_IP, ipaddr);
    strcpy(msz_tun_NETMASK, netmask);
    strcpy(msz_tun_GATEWAY, gateway);
    mn_MTU = mtu > 0 ? mtu : 1500;

    int ret = 0;
    ret = dev_open(CA2T(devname));
    if (ret == -1)
    {
        char logmsg[256] = "";
        sprintf(logmsg, "ERR: Device open failed: %s", devname);
        printf("%s\n", logmsg);
        g_mylog.logException(logmsg);
        return -1;
    }
    
    dev_config(ipaddr, netmask, gateway);

    // get MAC and friendlyName of TAP
    getLocalMac(CT2A(m_device_guid));   
    // set mtu of tap
    char cmd[256] = "";
    sprintf(cmd, "netsh interface ipv4 set subinterface \"%ls\" mtu=%d store=persistent", m_friendlyName, mn_MTU);
    WinExec(cmd, SW_HIDE);
    g_mylog.logException(cmd);

    return 0;
}
/*
* device_instance_id  可看做路径， 网卡->右键属性->配置->详细信息->设备实例路径
* dev_interface_list (实际不是列表，可看做名称)
* 获得到的devname 要保证有足够空间
*/
int myTunTap::dev_getDevid(TCHAR *devid, TCHAR *devname)
{
    HDEVINFO dev_info_set;           // 设备类型的句柄，后续使用该句柄进行多种查找
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
    TCHAR net_cfg_instance_id_string[] = _T("NetCfgInstanceId");      // 注册表子健
    for (DWORD i = 0;; ++i)
    {

        TCHAR net_cfg_instance_id[256] = {};
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
        // 查询注册表，获取net_cfg_instance_id_string子健键值，值存放在 net_cfg_instance_id 
        status = RegQueryValueEx(dev_key, net_cfg_instance_id_string, NULL, &data_type, (unsigned char *)net_cfg_instance_id, &len);
        if (status != ERROR_SUCCESS)
        {
            printError_Win("RegQueryValueEx");
            goto next;
        }

        // device instance ID：设备实例 ID 是系统提供的设备标识字符串，用于唯一标识系统中的设备, is a path like "ROOT\\NET\\0001"
        len = sizeof(device_instance_id);
        res = SetupDiGetDeviceInstanceId(dev_info_set, &device_info_data, device_instance_id, len, &len);     // device_instance_id
        if (!res)
        {
            printError_Win("SetupDiGetDeviceInstanceId");
            goto next;
        }
        if (0 != wcscmp(device_instance_id, devid))
            goto next;
        //else
        //     printf("Get device: %ls\n", device_instance_id);

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
            free(dev_interface_list);
            goto next;
        }
        else
        {
        //    printf_s("%d. net_cfg_instance_id: %ls\n", i, net_cfg_instance_id);
        //    printf_s("   device_instance_id : %ls\n", device_instance_id);
        //    printf_s("   dev_interface_list: %ls\n", dev_interface_list);
            wcscpy(devname, dev_interface_list);
            wcscpy(m_device_guid, net_cfg_instance_id);
            wcscpy(m_device_path, device_instance_id);
            wcscpy(m_device_name, dev_interface_list);
            // get index
            TCHAR wbuf[256];
            _stprintf_s(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), _T("\\DEVICE\\TCPIP_%ls"), m_device_guid);
            GetAdapterIndex(wbuf, &m_index);

            free(dev_interface_list);
            RegCloseKey(dev_key);
            break;
        }

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

    next:
        RegCloseKey(dev_key);
    }

    SetupDiDestroyDeviceInfoList(dev_info_set);

    return 0;
}

int myTunTap::dev_open(TCHAR *device_instance_id)
{
    char logmsg[256] = "";
    TCHAR dev_interface_list[256] = {};
    dev_getDevid(device_instance_id, dev_interface_list);

    CString kefile_path = "\\\\.\\Global\\";
    CString suffix = ".tap";
    CString filestr = "";
    filestr.Format(L"%s%s%s", kefile_path, m_device_guid, suffix);
//    printf("file is: %ls\n", filestr.GetBuffer());                  // this is the path to open TAP-Windows, device_interface_list is the path to open Wintun

    //DWORD Flags = flag ? (FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED) : FILE_ATTRIBUTE_SYSTEM;//
    HANDLE handle = CreateFile(filestr, GENERIC_READ | GENERIC_WRITE,
        /*FILE_SHARE_READ | FILE_SHARE_WRITE*/NULL,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
        NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        printError_Win("CreateFile failed on TUN device");
        CloseHandle(handle);
        return -1;
    }
    if (m_WintunHandle != NULL)
    {
        CloseHandle(m_WintunHandle);
        m_WintunHandle = NULL;
    }
    m_WintunHandle = handle;
    /* 
    m_WintunHandle = CreateFile(dev_interface_list, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL, OPEN_EXISTING, 0, NULL);
    if (m_WintunHandle == INVALID_HANDLE_VALUE)
    {
    printError_Win("CreateFile failed on TUN device");
    }
    else
    {
    printf("CreateFile succeed, get m_WintunHandle.\n");
    }
   */
    sprintf(logmsg, "INFO: Device open success:%ls", device_instance_id);
    printf("%s\n", logmsg);
    g_mylog.logException(logmsg);
    return 0;
}


/*
* Get adapter info list
*/
const IP_ADAPTER_INFO *
get_adapter_info_list(struct gc_arena *gc)
{
    ULONG size = 0;
    IP_ADAPTER_INFO *pi = NULL;
    DWORD status;

    if ((status = GetAdaptersInfo(NULL, &size)) != ERROR_BUFFER_OVERFLOW)
    {
        myTunTap::printError_Win("GetAdaptersInfo #1 failed ");
    }
    else
    {
        pi = (PIP_ADAPTER_INFO)gc_malloc(size, false, gc);
        if ((status = GetAdaptersInfo(pi, &size)) != NO_ERROR)
        {
            myTunTap::printError_Win("GetAdaptersInfo #2 failed ");
            pi = NULL;
        }
    }
    return pi;
}
/*
* Given an adapter index, return a pointer to the
* IP_ADAPTER_INFO structure for that adapter.
*/
const IP_ADAPTER_INFO *get_adapter(const IP_ADAPTER_INFO *ai, DWORD index)
{
    if (ai && index != TUN_ADAPTER_INDEX_INVALID)
    {
        const IP_ADAPTER_INFO *a;

        /* find index in the linked list */
        for (a = ai; a != NULL; a = a->Next)
        {
            if (a->Index == index)
            {
                return a;
            }
        }
    }
    return NULL;
}
const IP_ADAPTER_INFO *get_adapter_info(DWORD index, struct gc_arena *gc)
{
    return get_adapter(get_adapter_info_list(gc), index);
}
/*
* Delete all temporary address/netmask pairs which were added
* to adapter (given by index) by previous calls to AddIPAddress.
*/
static void
delete_temp_addresses(DWORD index)
{
    struct gc_arena gc = gc_new();
    const IP_ADAPTER_INFO *a = get_adapter_info(index, &gc);

    if (a)
    {
        const IP_ADDR_STRING *ip = &a->IpAddressList;
        while (ip)
        {
            DWORD status;
            const DWORD context = ip->Context;

            if ((status = DeleteIPAddress((ULONG)context)) == NO_ERROR)
            {
                printf("INFO: Successfully deleted previously set dynamic IP/netmask: %s/%s\n",
                    ip->IpAddress.String,
                    ip->IpMask.String);
            }
            else
            {
                const char *empty = "0.0.0.0";
                if (strcmp(ip->IpAddress.String, empty) || strcmp(ip->IpMask.String, empty))
                {
                    printf("ERR: could not delete previously set dynamic IP/netmask: %s/%s (status=%u)\n",
                        ip->IpAddress.String,
                        ip->IpMask.String,
                        (unsigned int)status);
                }
            }
            ip = ip->Next;
        }
    }
    gc_free(&gc);
}
struct buffer
#ifdef DMALLOC
    alloc_buf_gc_debug(size_t size, struct gc_arena *gc, const char *file, int line)
#else
    alloc_buf_gc(size_t size, struct gc_arena *gc)
#endif
{
    struct buffer buf;
    if (!buf_size_valid(size))
    {
        printf("buffer size is error %d\n", size);
    }
    buf.capacity = (int)size;
    buf.offset = 0;
    buf.len = 0;
#ifdef DMALLOC
    buf.data = (uint8_t *)gc_malloc_debug(size, false, gc, file, line);
#else
    buf.data = (uint8_t *)gc_malloc(size, false, gc);
#endif
    if (size)
    {
        *buf.data = 0;
    }
    return buf;
}
/*
* Convert an in_addr_t in host byte order
* to an ascii dotted quad.
*/
const char *
print_in_addr_t(in_addr_t addr, unsigned int flags, struct gc_arena *gc)
{
    struct in_addr ia;
    struct buffer out = alloc_buf_gc(64, gc);

    if (addr || !(flags & IA_EMPTY_IF_UNDEF))
    {
        memset(&ia, 0, sizeof(ia));
        ia.s_addr = (flags & IA_NET_ORDER) ? addr : htonl(addr);
        char ip[16] = "";
        inet_ntop(AF_INET, &ia, ip, sizeof(ip));
        sprintf((char *)&out, "%s", ip);
    }
    return BSTR(&out);
}
static void
clear_tuntap(struct tuntap *tuntap)
{
    memset(tuntap, 0, sizeof(*tuntap));
#ifdef _WIN32
    tuntap->hand = NULL;
#else
    tuntap->fd = -1;
#endif
#ifdef TARGET_SOLARIS
    tuntap->ip_fd = -1;
#endif
}
/*
static void
open_null(struct tuntap *tt)
{
tt->actual_name = string_alloc("null", NULL);
}
*/




static void clear_tuntap(struct tuntap *tuntap);
void buf_size_error(const size_t size)
{
    printf("fatal buffer size error, size=%lu\n", (unsigned long)size);
}
static inline bool buf_defined(const struct buffer *buf)
{
    return buf->data != NULL;
}
bool is_dev_type(const char *dev, const char *dev_type, const char *match_type)
{
    if (!dev || !dev_type || !match_type)
    {
        return false;
    }
    if (dev_type)
    {
        return !strcmp(dev_type, match_type);
    }
    else
    {
        return !strncmp(dev, match_type, strlen(match_type));
    }
}

int
dev_type_enum(const char *dev, const char *dev_type)
{
    if (is_dev_type(dev, dev_type, "tun"))
    {
        return DEV_TYPE_TUN;
    }
    else if (is_dev_type(dev, dev_type, "tap"))
    {
        return DEV_TYPE_TAP;
    }
    else if (is_dev_type(dev, dev_type, "null"))
    {
        return DEV_TYPE_NULL;
    }
    else
    {
        return DEV_TYPE_UNDEF;
    }
}

void open_tun(const char *dev, const char *dev_type, const char *dev_node, struct tuntap *tt)

{
    const char *device_guid = NULL;

    /*netcmd_semaphore_lock ();*/

    printf("open_tun\n");

    if (tt->type != DEV_TYPE_TAP && tt->type != DEV_TYPE_TUN)
    {
        printf("Unknown virtual device type: '%s'", dev);
    }
    /*
    tun_open_device(tt, dev_node, &device_guid);

    if (!tt->wintun)
    {
    tuntap_post_open(tt, device_guid);
    }
    */
    /*netcmd_semaphore_release ();*/
}

int myTunTap::dev_config(const char *ipaddr, const char *netmask, const char *gateway)
{
    char logmsg[256] = "";
    struct tuntap *tt;

    ALLOC_OBJ(tt, struct tuntap);
    clear_tuntap(tt);
    tt->type = dev_type_enum("tun111", "tun");
    //	tt->options = *options;

    // tap version
    ULONG info[3];
    DWORD len = 0;
    memset(info, 0, sizeof(info));
    BOOL bret = DeviceIoControl(m_WintunHandle, TAP_IOCTL_GET_VERSION, info, sizeof(info), info, sizeof(info), &len, NULL);
    if (bret == TRUE)
    {
        sprintf(logmsg, "INFO: Windows Driver Version %d.%d %s", (int)info[0], (int)info[1], (info[2] ? "(DEBUG)" : ""));
        printf("%s\n", logmsg);
        g_mylog.logException(logmsg);
    }
    // get mtu (invalid if use netsh modified the MTU)
 /*   ULONG mtu;
    bret = DeviceIoControl(m_WintunHandle, TAP_IOCTL_GET_MTU, &mtu, sizeof(mtu), &mtu, sizeof(mtu), &len, NULL);
    if (bret == TRUE)
    {
        tt->post_open_mtu = (int)mtu;
        sprintf(logmsg, "INFO: MTU = %d", mtu);
        printf("%s\n", logmsg);
        g_mylog.logException(logmsg);
    }*/
    // set on
    BOOL tun_status = FALSE;
    bret = DeviceIoControl(m_WintunHandle, TAP_IOCTL_SET_MEDIA_STATUS, &tun_status, sizeof(tun_status), &tun_status, sizeof(tun_status), &len, NULL);
    if (bret == FALSE)
    {
        printError_Win("IOCTL set media status off failed!\n");
        //    CloseHandle(m_WintunHandle);
        //    return -1;
    }

    tun_status = TRUE;
    bret = DeviceIoControl(m_WintunHandle, TAP_IOCTL_SET_MEDIA_STATUS, &tun_status, sizeof(tun_status), &tun_status, sizeof(tun_status), &len, NULL);
    if (bret == FALSE)
    {
        printError_Win("IOCTL set media status on failed!\n");
        //    CloseHandle(m_WintunHandle);
        //    return -1;
    }

    // flush arp
    DWORD status;
    /**/    status = FlushIpNetTable(m_index);
    if (status == NO_ERROR)
    {
        sprintf(logmsg, "INFO: Successful ARP Flush on interface [%lu] %ls", m_index, m_device_guid);
        g_mylog.logException(logmsg);
    }
    else if (status != -1)
    {
        sprintf(logmsg, "ERR: FlushIpNetTable failed on interface [%lu] %ls (status=%lu)", m_index, m_device_guid, status);
        g_mylog.logException(logmsg);
        printError_Win("FlushIpNetTable error ");
    }

    delete_temp_addresses(m_index);
    
    struct in_addr iaddr, imask, igateway;
    memset(&iaddr,    0, sizeof(iaddr));
    memset(&imask,    0, sizeof(imask));
    memset(&igateway, 0, sizeof(igateway));
    inet_pton(AF_INET, ipaddr,  &iaddr.s_addr);       // inet_addr(localip)
    inet_pton(AF_INET, netmask, &imask.s_addr);       // inet_addr(netmask)
    inet_pton(AF_INET, gateway, &igateway.s_addr);    // inet_addr(gateway)
    // set dhcp 
 /*   uint32_t ep[4];
    in_addr_t addr;
    ep[0] = htonl(iaddr.s_addr);
    ep[1] = htonl(imask.s_addr);
    ep[2] = htonl(igateway.s_addr);
    ep[3] = (uint32_t)65536;
    if (!DeviceIoControl(m_WintunHandle, TAP_IOCTL_CONFIG_DHCP_MASQ,
        ep, sizeof(ep),
        ep, sizeof(ep), &len, NULL))
    {
        printError_Win("ERROR: The TAP-Windows driver rejected a DeviceIoControl call to set TAP_WIN_IOCTL_CONFIG_DHCP_MASQ mode");
    }
    struct buffer buf ;
    buf.capacity = 256;
    buf.offset = 0;
    buf.len = 0;
    buf.data = (uint8_t*)calloc(1, buf.capacity);

    if (build_dhcp_options_string(&buf, &tt->options))
    {
        if (!DeviceIoControl(m_WintunHandle, TAP_IOCTL_CONFIG_DHCP_SET_OPT,
            BPTR(&buf), BLEN(&buf),
            BPTR(&buf), BLEN(&buf), &len, NULL))
        {
            printError_Win("ERROR: The TAP-Windows driver rejected a TAP_WIN_IOCTL_CONFIG_DHCP_SET_OPT DeviceIoControl call\n");
        }
    }
    else
    {
        printf("DHCP option string not set due to error\n");
    }
    */
    // add a new IP address

    if ((status = AddIPAddress(iaddr.s_addr,
        imask.s_addr,
        m_index,
        &tt->ipapi_context,
        &tt->ipapi_instance)) == NO_ERROR)
    {
        sprintf(logmsg, "INFO: Succeeded in adding a temporary IP/netmask of %s/%s to interface %ls", ipaddr, netmask, m_device_guid);
        printf("%s\n", logmsg);
        g_mylog.logException(logmsg);
    }
    else
    {
        printError_Win("AddIPAddress error ");
        sprintf(logmsg, "ERR: AddIPAddress %s/%s failed on interface %ls, index=%lu, status=%lu", ipaddr, netmask, m_device_guid, m_index, status);
        printf("%s\n", logmsg);
        g_mylog.logException(logmsg);
    }
    tt->ipapi_context_defined = true;
    

    /*
    ULONG ep[3];
    ep[0] = htonl(inet_addr(ipaddr));
    ep[1] = htonl(inet_addr(netmask));

    bret = DeviceIoControl(m_WintunHandle, TAP_IOCTL_CONFIG_POINT_TO_POINT, ep, 12, ep, 12, &len, NULL);
    if (bret == FALSE)
    {
    printError_Win("IOCTL set tun point to point failed!\n");
    //    return -1;
    }

    ep[0] = htonl(inet_addr(ipaddr));
    ep[1] = htonl(inet_addr(ipaddr) & inet_addr(netmask));
    ep[2] = htonl(inet_addr(netmask));

    bret = DeviceIoControl(m_WintunHandle, TAP_CONFIG_TUN, ep, sizeof(ep), ep, sizeof(ep), &len, NULL);
    if (bret == FALSE)
    {
    printError_Win("IOCTL set tun failed!\n");
    //   CloseHandle(m_WintunHandle);
    //    return -1;
    }
    */


    return 0;
}
// DeviceIoControl
int myTunTap::dev_read(struct tuntap *tt, int maxsize)
{
    if (tt->reads.iostate == IOSTATE_INITIAL)
    {
        DWORD len;
        BOOL status;
        int err;

        /* reset buf to its initial state */
        tt->reads.buf = tt->reads.buf_init;

        len = maxsize ? maxsize : BLEN(&tt->reads.buf);
        if (len > (unsigned long)BLEN(&tt->reads.buf))
        {
            printf("len is too small");
            return -1;
        }

        /* the overlapped read will signal this event on I/O completion */
        if (!ResetEvent(tt->reads.overlapped.hEvent))
        {
            printError_Win("ResetEvent - read ");
        }

        status = ReadFile(tt->hand, BPTR(&tt->reads.buf), len, &tt->reads.size, &tt->reads.overlapped);
        if (status) /* operation completed immediately? */
        {
            /* since we got an immediate return, we must signal the event object ourselves */
            if (!SetEvent(tt->reads.overlapped.hEvent))
            {
                printError_Win("SetEvent - read ");
            }

            tt->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
            tt->reads.status = 0;

            printf("WIN32 I/O: TAP Read immediate return [%d,%d]", (int)len, (int)tt->reads.size);
        }
        else
        {
            err = GetLastError();
            if (err == ERROR_IO_PENDING) /* operation queued? */
            {
                tt->reads.iostate = IOSTATE_QUEUED;
                tt->reads.status = err;
                printf("WIN32 I/O: TAP Read queued [%d]", (int)len);
            }
            else /* error occurred */
            {
                if (!SetEvent(tt->reads.overlapped.hEvent))
                {
                    printError_Win("SetEvent - read ");
                }
                tt->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
                tt->reads.status = err;
                printError_Win("WIN32 I/O: TAP Read error");
            }
        }
    }
    return tt->reads.iostate;
}

int myTunTap::dev_write(struct tuntap *tt, struct buffer *buf)
{
    if (tt->writes.iostate == IOSTATE_INITIAL)
    {
        BOOL status;
        int err;

        /* make a private copy of buf */
        tt->writes.buf = tt->writes.buf_init;
        tt->writes.buf.len = 0;
        if (!buf_copy(&tt->writes.buf, buf))
        {
            printf("buf_copy error:");
        }

        /* the overlapped write will signal this event on I/O completion */
        if (!ResetEvent(tt->writes.overlapped.hEvent))
        {
            printError_Win("ResetEvent - write ");
        }

        status = WriteFile(tt->hand, BPTR(&tt->writes.buf), BLEN(&tt->writes.buf), &tt->writes.size, &tt->writes.overlapped);
        if (status) /* operation completed immediately? */
        {
            tt->writes.iostate = IOSTATE_IMMEDIATE_RETURN;

            /* since we got an immediate return, we must signal the event object ourselves */
            if (!SetEvent(tt->writes.overlapped.hEvent))
            {
                printError_Win("SetEvent - write ");
            }
            tt->writes.status = 0;

            printf("WIN32 I/O: TAP Write immediate return [%d,%d]", BLEN(&tt->writes.buf), (int)tt->writes.size);
        }
        else
        {
            err = GetLastError();
            if (err == ERROR_IO_PENDING) /* operation queued? */
            {
                tt->writes.iostate = IOSTATE_QUEUED;
                tt->writes.status = err;
                printf("WIN32 I/O: TAP Write queued [%d]",
                    BLEN(&tt->writes.buf));
            }
            else /* error occurred */
            {
                if (SetEvent(tt->writes.overlapped.hEvent))
                {
                    printError_Win("SetEvent - write ");
                }
                tt->writes.iostate = IOSTATE_IMMEDIATE_RETURN;
                tt->writes.status = err;
                printError_Win("WIN32 I/O: TAP Write error");

            }
        }
    }
    return tt->writes.iostate;
}

inline void myTunTap::read_wintun(struct tuntap * tt, struct buffer * buf)
{
    TUN_RING *ring = tt->wintun_send_ring;
    ULONG head = ring->Head;
    ULONG tail = ring->Tail;
    ULONG content_len;
    TUN_PACKET *packet;
    ULONG aligned_packet_size;

    *buf = tt->reads.buf_init;
    buf->len = 0;

    if ((head >= WINTUN_RING_CAPACITY) || (tail >= WINTUN_RING_CAPACITY))
    {
        printf("Wintun: ring capacity exceeded");
        buf->len = -1;
        return;
    }

    if (head == tail)
    {
        /* nothing to read */
        return;
    }

    content_len = wintun_ring_wrap(tail - head);
    if (content_len < sizeof(struct TUN_PACKET_HEADER))
    {
        printf("Wintun: incomplete packet header in send ring");
        buf->len = -1;
        return;
    }

    packet = (TUN_PACKET *)&ring->Data[head];
    if (packet->Size > WINTUN_MAX_PACKET_SIZE)
    {
        printf("Wintun: packet too big in send ring");
        buf->len = -1;
        return;
    }

    aligned_packet_size = wintun_ring_packet_align(sizeof(struct TUN_PACKET_HEADER) + packet->Size);
    if (aligned_packet_size > content_len)
    {
        printf("Wintun: incomplete packet in send ring");
        buf->len = -1;
        return;
    }

    buf_write(buf, packet->Data, packet->Size);

    head = wintun_ring_wrap(head + aligned_packet_size);
    ring->Head = head;
}

inline int myTunTap::write_wintun(tuntap * tt, buffer * buf)
{
    TUN_RING *ring = tt->wintun_receive_ring;
    ULONG head = ring->Head;
    ULONG tail = ring->Tail;
    ULONG aligned_packet_size;
    ULONG buf_space;
    TUN_PACKET *packet;

    /* wintun marks ring as corrupted (overcapacity) if it receives invalid IP packet */
    if (!is_ip_packet_valid(buf))
    {
        printf("write_wintun(): drop invalid IP packet");
        return 0;
    }

    if ((head >= WINTUN_RING_CAPACITY) || (tail >= WINTUN_RING_CAPACITY))
    {
        printf("write_wintun(): head/tail value is over capacity");
        return -1;
    }

    aligned_packet_size = wintun_ring_packet_align(sizeof(struct TUN_PACKET_HEADER) + BLEN(buf));
    buf_space = wintun_ring_wrap(head - tail - WINTUN_PACKET_ALIGN);
    if (aligned_packet_size > buf_space)
    {
        printf("write_wintun(): ring is full");
        return 0;
    }

    /* copy packet size and data into ring */
    packet = (TUN_PACKET*)&ring->Data[tail];
    packet->Size = BLEN(buf);
    memcpy(packet->Data, BPTR(buf), BLEN(buf));

    /* move ring tail */
    ring->Tail = wintun_ring_wrap(tail + aligned_packet_size);
    if (ring->Alertable != 0)
    {
        SetEvent(tt->rw_handle.write);
    }

    return BLEN(buf);
}

int myTunTap::myread(char *buffer, unsigned int size)
{
    BOOL status;
    DWORD len = 0;
    ethhdr *eth_hdr = (ethhdr *)buffer;
    iphdr  *ip_hdr = (iphdr  *)(buffer + sizeof(ethhdr));
    uint16_t arp_proto = 0x0806;
    unsigned char dstMAC[32] = { 0x00, 0xff, 0x93, 0xb6, 0xf7, 0x50 };
    unsigned char srcMAC[32] ;

    if (status = ReadFile(m_WintunHandle, buffer, 65536, &len, &m_read_overlapped))
    {
        /* since we got an immediate return, we must signal the event object ourselves */
        if (!SetEvent(m_read_overlapped.hEvent))
        {
            printError_Win("SetEvent - read ");
        }
    }
    else
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {//当错误是ERROR_IO_PENDING,那意味着读文件的操作还在进行中
            //等候，直到文件读完
            WaitForSingleObject(m_read_overlapped.hEvent, INFINITE);//
            status = GetOverlappedResult(m_WintunHandle, &m_read_overlapped, &len, FALSE);
        }
        else
        {
            printError_Win("SetEvent - read ");
            return -1;
        }
    }

    // judge if it is an arp packet
    if (eth_hdr->proto == htons(0x0806) && len < 100)
    {
    //    printf("This is an arp packet.\n");
        openvpn_arp pkt_arp;
        char write_buffer[256] = "";
        DWORD write_len = 0;
        memset(&pkt_arp, 0, sizeof(openvpn_arp));
        memcpy(&pkt_arp, buffer + 14, len - 14);
        if (pkt_arp.arp_command == htons(0x0001))
        {
            pkt_arp.arp_command = htons(0x0002);
        //    memcpy(pkt_arp.mac_dest, pkt_arp.mac_src, 6);
            char dst_ip[64] = "";
            inet_ntop(AF_INET, &pkt_arp.ip_dest, dst_ip, sizeof(dst_ip));

            
            sscanf(msz_tun_MAC, "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned int *)srcMAC, (unsigned int *)(srcMAC + 1), (unsigned int *)(srcMAC + 2), (unsigned int *)(srcMAC + 3), (unsigned int *)(srcMAC + 4), (unsigned int *)(srcMAC + 5));
            
            memcpy(eth_hdr->dest, &srcMAC, 6);   //TAP_MAC
            memcpy(eth_hdr->source, &dstMAC, 6);
            memcpy(pkt_arp.mac_dest, pkt_arp.mac_src, 6);
            memcpy(pkt_arp.mac_src, &dstMAC, 6);
            uint32_t tmp = pkt_arp.ip_dest;
            pkt_arp.ip_dest = pkt_arp.ip_src;     // 
            pkt_arp.ip_src = tmp;
            
            // write pkt_arp
            memcpy(write_buffer, eth_hdr, sizeof(ethhdr));
            memcpy(write_buffer + sizeof(ethhdr), &pkt_arp, sizeof(openvpn_arp));
           
            BOOL ret = WriteFile(m_WintunHandle, (LPCVOID)&write_buffer, 42, &write_len, &m_write_overlapped); // sizeof(ethhdr) + sizeof(openvpn_arp)
            if (!ret)
            {
                DWORD err = GetLastError();
                if (err == ERROR_IO_PENDING)
                {
                    WaitForSingleObject(m_write_overlapped.hEvent, INFINITE);
                    status = GetOverlappedResult(m_WintunHandle, &m_write_overlapped, &write_len, FALSE);
                }
                else
                    printError_Win("WriteFile : ");
                //	SetEvent(writeOverlapped.hEvent);
            }
            else
            {
                if (!SetEvent(m_write_overlapped.hEvent))
                {
                    printError_Win("SetEvent - write ");
                }
            }
        }
    }
    return (int)len;
}

int myTunTap::mywrite(char *buffer, unsigned int size)
{
    BOOL status;
    DWORD len = 0;

    if (status = WriteFile(m_WintunHandle, buffer, size, &len, &m_write_overlapped))
    {
        /* since we got an immediate return, we must signal the event object ourselves */
        if (!SetEvent(m_write_overlapped.hEvent))
        {
            printError_Win("SetEvent - write ");
        }
    }
    else
    {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING)
        {
            WaitForSingleObject(m_write_overlapped.hEvent, INFINITE);
            status = GetOverlappedResult(m_WintunHandle, &m_write_overlapped, &len, FALSE);
        }
        else
        {
            char logmsg[256] = "";
            sprintf(logmsg, "WriteFile : %d", len);
            printError_Win(logmsg);
            return -1;
        }
    }
    /*
    printf("WIN32 I/O: TAP write immediate return [%d,%d]", (int)len, len);
    for (unsigned int i = 0; i < len; i++)
    {
        printf("%02X", (unsigned char)buffer[i]);
        if ((i + 1) % 8 == 0)
            printf(" ");
    }
    printf("\n");
    Sleep(1);
    */
    return len;
}
std::string myTunTap::Utf8ToGbk(const char *src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    std::string strTemp(szGBK);
    if (wszGBK)
        delete[] wszGBK;
    if (szGBK)
        delete[] szGBK;
    return strTemp;
}
void myTunTap::printError_Win(const char * msg)
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
    char logmsg[256] = "";
    sprintf(logmsg, "ERR: %s failed with error %d - %s", msg, eNum, T2A((LPCTSTR)lpMsgBuf));
    g_mylog.logException(logmsg);
    //   LOG(ERROR) << msg << " failed with error " << eNum << " - " << CDevInfo::AnsiToUtf8((LPCTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);

}
// get the MAC by device_guid
int myTunTap::getLocalMac(char* device_guid)
{
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i = 0;
    setlocale(LC_ALL, "");  // print Chinese

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;             // Set the flags to pass to GetAdaptersAddresses

    ULONG family = AF_UNSPEC;                          // default to unspecified address family (both)
    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
    IP_ADAPTER_PREFIX *pPrefix = NULL;

    family = AF_INET;                      // AF_INET  AF_INET6  AF_UNSPEC

    outBufLen = 65536;        // Allocate a 64 KB buffer to start with.
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
        if (pAddresses == NULL) {
            printf("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
            exit(1);
        }

        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        }
        else {
            break;    // run until succeed one time within MAX_TRIES
        }
        Iterations++;
    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));
    // If successful, output some information from the data we received
    if (dwRetVal == NO_ERROR)
    {
        pCurrAddresses = pAddresses;
        while (pCurrAddresses)
        {
            if (pCurrAddresses->IfType == IF_TYPE_SOFTWARE_LOOPBACK )   // 状态，1 up, 2 down, 3 testing, 4 unknown等|| pCurrAddresses->OperStatus != 1
            {
                pCurrAddresses = pCurrAddresses->Next;
                continue;
            }

        //    printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);  //{2788335C-885A-42AC-967E-7A689B367741}
        //    printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);
        //    printf("\tDescription: %wS\n", pCurrAddresses->Description); //TAP-Windows Adapter V9
            std::wstring FriendlyName = pCurrAddresses->FriendlyName;
            std::wstring describe = pCurrAddresses->Description;

            pUnicast = pCurrAddresses->FirstUnicastAddress;    // 获取单播地址 (同理可获取多播地址)
                                                               //Unicast IP
            if (pUnicast != NULL) {
                for (i = 0; pUnicast != NULL; i++)
                {
                    char ipAddr[64] = "";
                    if (AF_INET == pUnicast->Address.lpSockaddr->sa_family)// IPV4 地址，使用 IPV4 转换
                        inet_ntop(PF_INET, &((sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr, ipAddr, sizeof(ipAddr));
                    else if (AF_INET6 == pUnicast->Address.lpSockaddr->sa_family)// IPV6 地址，使用 IPV6 转换
                        inet_ntop(PF_INET6, &((sockaddr_in6*)pUnicast->Address.lpSockaddr)->sin6_addr, ipAddr, sizeof(ipAddr));

                    int sa_family = pUnicast->Address.lpSockaddr->sa_family;
                    string ip = ipAddr;
                    int prefix = pUnicast->OnLinkPrefixLength;
                    //	printf("\tUnicast Addresses: %d  %s \n", i, ipAddr);
                    //	printf("\tMASK: %s\n", prefixToMask(pUnicast->OnLinkPrefixLength).c_str());

                    pUnicast = pUnicast->Next;
                }
            }
            // DNS 
            /*	pDnServer = pCurrAddresses->FirstDnsServerAddress;
            if (pDnServer) {
                for (i = 0; pDnServer != NULL; i++)
                {
                    char ip[64] = "";
                    if (AF_INET == pDnServer->Address.lpSockaddr->sa_family)// IPV4 地址，使用 IPV4 转换
                    inet_ntop(PF_INET, &((sockaddr_in*)pDnServer->Address.lpSockaddr)->sin_addr, ip, sizeof(ip));
                    else if (AF_INET6 == pDnServer->Address.lpSockaddr->sa_family)// IPV6 地址，使用 IPV6 转换
                    inet_ntop(PF_INET6, &((sockaddr_in6*)pDnServer->Address.lpSockaddr)->sin6_addr, ip, sizeof(ip));

                    printf("\tNumber of DNS Server Addresses: %d %s\n", i, ip);
                    pDnServer = pDnServer->Next;
                }
            }
            //	printf("\tIfType: %ld\n", pCurrAddresses->IfType);     // 类型 https://docs.microsoft.com/zh-cn/windows/desktop/api/iptypes/ns-iptypes-_ip_adapter_addresses_lh
            //	printf("\tIpv6IfIndex (IPv6 interface): %u\n", pCurrAddresses->Ipv6IfIndex); // IPv6 不可用时是0
            //	printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);  // 发送bit带宽
            //	printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);    // 接收bit带宽
            //	printf("\n");}
            */
            // MAC
            char tmpMAC[64] = "";
            if (pCurrAddresses->PhysicalAddressLength != 0) {
                for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength; i++) {
                    if (i == (pCurrAddresses->PhysicalAddressLength - 1))
                        sprintf_s(tmpMAC, "%s%.2X", tmpMAC, (int)pCurrAddresses->PhysicalAddress[i]);
                    else
                        sprintf_s(tmpMAC, "%s%.2X:", tmpMAC, (int)pCurrAddresses->PhysicalAddress[i]);
                } 
            }

            if (0 == strcmp(device_guid, pCurrAddresses->AdapterName))
            {
                strcpy(msz_tun_MAC, tmpMAC);
                _stprintf_s(m_friendlyName, 256, _T("%ls"), pCurrAddresses->FriendlyName);
                char logmsg[256] = "";
                sprintf(logmsg, "INFO: Physical address of tap device: %s %ls\n", tmpMAC, m_friendlyName);
                g_mylog.logException(logmsg);
                break;
            }
            else
            {
                pCurrAddresses = pCurrAddresses->Next;
            }
        }
    }
    else {
        printf("Call to GetAdaptersAddresses failed with error: %d\n",
            dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            printf("\tNo addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL))
            {
                USES_CONVERSION;
                printf("\tError: %s", T2A((LPCTSTR)lpMsgBuf));
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    FREE(pAddresses);
                return 1;
            }
        }
    }
    if (pAddresses) {
        FREE(pAddresses);
    }
    return 0;
}
char * myTunTap::returnTunMAC()
{
    if (strlen(msz_tun_MAC) != 0 && msz_tun_MAC != NULL)
        return msz_tun_MAC;
    else
        return nullptr;
}

//计算校验和
unsigned short myTunTap::checksum(unsigned char *buf, int len)
{
    unsigned int sum = 0;
    unsigned short *cbuf;

    cbuf = (unsigned short *)buf;

    while (len>1)
    {
        sum += *cbuf++;
        len -= 2;
    }

    if (len)
        sum += *(unsigned char *)cbuf;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}

int myTunTap::process()
{
    std::unique_ptr<char> buffer_ptr(new char[65536]);
    char *m_buffer = buffer_ptr.get();
    char *posBegin = m_buffer;
    char *posEnd = m_buffer;

    ethhdr *eth_hdr = (ethhdr *)m_buffer;
    iphdr  *ip_hdr = (iphdr  *)(m_buffer + sizeof(ethhdr));

    uint16_t arp_proto = 0x0806;
    unsigned char dstMAC[32] = { };
    unsigned char srcMAC[32] = { 0x00, 0xff, 0x93, 0xb6, 0xf7, 0x50 };
    char logmsg[256] = "";
    int nRead = 0;
    while (TRUE)
    {
        memset(m_buffer, 0, 65536);
        nRead = myread(m_buffer, 65536);
        if (nRead == 0)
        {
            //    m_mylog.logException("WAR: dev_read nothing.");
            Sleep(1);
            continue;
        }
        else if (nRead < 0)
        {
            // check status of the tun device
            Sleep(5000);
            continue;
        }
        posEnd = m_buffer + nRead;
        int iphdr_size = 0;
        iphdr_size = ((ip_hdr->version_len) & 0x0F) << 2;
        //    struct tcphdr *tcp = ((struct tcphdr *) (nf_packet + (iph->ihl << 2)));
        struct icmphdr *icmph = ((struct icmphdr *) (m_buffer + sizeof(ethhdr) + iphdr_size));
        if (ip_hdr->protocol == IPPROTO_ICMP || ip_hdr->protocol == IPPROTO_TCP || ip_hdr->protocol == IPPROTO_UDP)
        {
            printf("read one packet\n");
        }
    } // end of while
    return 0;
}