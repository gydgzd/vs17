//#include "stdafx.h"
#include "testNDIS.h"

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

struct device_instance_id_interface
{
    const char *net_cfg_instance_id;
    const char *device_interface_list;
    struct device_instance_id_interface *next;
};

testNDIS::testNDIS()
{
    m_index = 0;
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


testNDIS::~testNDIS()
{

}

int testNDIS::init(const char *devname, const char *ipaddr, const char *netmask, const char *gateway)
{
    return 0;
}
/* 
 * device_instance_id  可看做路径， 网卡->右键属性->配置->详细信息->设备实例路径
 * dev_interface_list (实际不是列表，可看做名称)
 * 获得到的devname 要保证有足够空间
 */
int testNDIS::dev_getDevid(TCHAR *devid, TCHAR *devname)
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
        else
            printf("Get device: %ls\n", device_instance_id);
        
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
            printf_s("%d. net_cfg_instance_id: %ls\n", i, net_cfg_instance_id);
            printf_s("   device_instance_id : %ls\n", device_instance_id);
            printf_s("   dev_interface_list: %ls\n", dev_interface_list);
            wcscpy(devname, dev_interface_list);
            wcscpy(m_device_guid, net_cfg_instance_id);
            wcscpy(m_device_path, device_instance_id);
            wcscpy(m_device_name, dev_interface_list);
            // get index
            TCHAR wbuf[256];
            _stprintf_s(wbuf, sizeof(wbuf)/sizeof(wbuf[0]), _T("\\DEVICE\\TCPIP_%ls"), m_device_guid);
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

int testNDIS::dev_open(TCHAR *device_instance_id)
{
    TCHAR dev_interface_list[256] = {};
    dev_getDevid(device_instance_id, dev_interface_list);

    CString kefile_path = "\\\\.\\Global\\";
    CString suffix = ".tap";
    CString filestr = "";
    filestr.Format(L"%s%s%s", kefile_path, m_device_guid, suffix);
    printf("file is: %ls\n", filestr.GetBuffer());                  // this is the path to open TAP-Windows, device_interface_list is the path to open Wintun

    //DWORD Flags = flag ? (FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED) : FILE_ATTRIBUTE_SYSTEM;//
 //   HANDLE handle = CreateFile(filestr, GENERIC_READ | GENERIC_WRITE,
        /*FILE_SHARE_READ | FILE_SHARE_WRITE*/NULL,
 /*       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
        NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        //	ACE_DEBUG((LM_INFO, "open tap device failed!\n"));
        printError_Win("CreateFile failed on TUN device");
        CloseHandle(handle);
        return -1;
    }
    m_WintunHandle = handle;
   */ 
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
        testNDIS::printError_Win("GetAdaptersInfo #1 failed ");
    }
    else
    {
        pi = (PIP_ADAPTER_INFO)gc_malloc(size, false, gc);
        if ((status = GetAdaptersInfo(pi, &size)) != NO_ERROR)
        {
            testNDIS::printError_Win("GetAdaptersInfo #2 failed ");
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
                printf("Successfully deleted previously set dynamic IP/netmask: %s/%s\n",
                    ip->IpAddress.String,
                    ip->IpMask.String);
            }
            else
            {
                const char *empty = "0.0.0.0";
                if (strcmp(ip->IpAddress.String, empty)
                    || strcmp(ip->IpMask.String, empty))
                {
                    printf("NOTE: could not delete previously set dynamic IP/netmask: %s/%s (status=%u)\n",
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

		sprintf((char *)&out, "%s", inet_ntoa(ia));
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

int testNDIS::dev_config(const char *ipaddr, const char *netmask, const char *gateway)
{
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
        printf("TAP - Windows Driver Version %d.%d %s\n", (int)info[0], (int)info[1], (info[2] ? "(DEBUG)" : ""));
    }
    // get mtu
    ULONG mtu;
    bret = DeviceIoControl(m_WintunHandle, TAP_IOCTL_GET_MTU, &mtu, sizeof(mtu), &mtu, sizeof(mtu), &len, NULL);
    if (bret == TRUE)
    {
        tt->post_open_mtu = (int)mtu;
        printf("MTU = %d\n", mtu);
    }
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

 /*   if (m_index != TUN_ADAPTER_INDEX_INVALID)
    {
        DWORD status = -1;

        if (tt->options.msg_channel)
        {
            ack_message_t ack;
            flush_neighbors_message_t msg = {
                .header = {
                msg_flush_neighbors,
                sizeof(flush_neighbors_message_t),
                0
            },
                .family = AF_INET,
                .iface = { .index = index,.name = "" }
            };

            if (send_msg_iservice(tt->options.msg_channel, &msg, sizeof(msg),
                &ack, "TUN"))
            {
                status = ack.error_number;
            }
        }
        else
        {
            status = FlushIpNetTable(index);
        }

        if (status == NO_ERROR)
        {
            msg(M_INFO, "Successful ARP Flush on interface [%lu] %s",
                index,
                device_guid);
        }
        else if (status != -1)
        {
            printf("NOTE: FlushIpNetTable failed on interface [%lu] %s (status=%lu) : %s", index, device_guid, status, strerror_win32(status, &gc));
        }
    }*/
    // set ip address
    DWORD status;
/**/    status = FlushIpNetTable(m_index);
    if (status == NO_ERROR)
    {
        printf("Successful ARP Flush on interface [%lu] %ls\n", m_index, m_device_guid);
    }
    else if (status != -1)
    {
        printf("NOTE: FlushIpNetTable failed on interface [%lu] %ls (status=%lu)\n", m_index, m_device_guid, status);
        printError_Win("FlushIpNetTable error ");
    }
   
    delete_temp_addresses(m_index);

    // add a new IP address
    struct in_addr iaddr, imask;
    memset(&iaddr, 0, sizeof(iaddr));
    memset(&imask, 0, sizeof(imask));
    inet_pton(AF_INET, ipaddr, &iaddr.s_addr);    // inet_addr(localip)
    inet_pton(AF_INET, netmask, &imask.s_addr);    // inet_addr(netmask)
    if ((status = AddIPAddress(iaddr.s_addr,
        imask.s_addr,
        m_index,
        &tt->ipapi_context,
        &tt->ipapi_instance)) == NO_ERROR)
    {
        printf("Succeeded in adding a temporary IP/netmask of %s/%s to interface %ls using the Win32 IP Helper API\n", ipaddr, netmask, m_device_guid);
    }
    else
    {
        printError_Win("AddIPAddress error ");
        printf("ERROR: AddIPAddress %s/%s failed on interface %ls, index=%lu, status=%lu\n", ipaddr, netmask, m_device_guid, m_index, status);
    }
    tt->ipapi_context_defined = true;


/*  ULONG ep[3];  
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
int testNDIS::dev_read(struct tuntap *tt, int maxsize)
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

int testNDIS::dev_write(struct tuntap *tt, struct buffer *buf)
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

inline void testNDIS::read_wintun(struct tuntap * tt, struct buffer * buf)
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

    packet = (TUN_PACKET *) &ring->Data[head];
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

inline int testNDIS::write_wintun(tuntap * tt, buffer * buf)
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

int testNDIS::myread()
{
    BOOL status ;
    DWORD len = 0;
    char buffer[65536] = "";
    char write_buffer[65536] = "";

    openvpn_ethhdr *eth_hdr = (openvpn_ethhdr *)buffer;
    openvpn_iphdr  *ip_hdr = (openvpn_iphdr  *)(buffer + sizeof(openvpn_ethhdr));
    openvpn_arp pkt_arp;
    uint16_t arp_proto = 0x0806;
    ULONG64 dst_mac = 0x010203040506;
    while(true)                     // operation completed immediately?   
    {
        if( status = ReadFile(m_WintunHandle, buffer, 65536, &len, &m_read_overlapped) )
        {
            /* since we got an immediate return, we must signal the event object ourselves */
            if (!SetEvent(m_read_overlapped.hEvent))
            {
                printError_Win("SetEvent - read ");
            }
            printf("WIN32 I/O: TAP Read immediate return [%d,%d]", (int)len, len);
        }
        else
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {//当错误是ERROR_IO_PENDING,那意味着读文件的操作还在进行中
             //等候，直到文件读完
                WaitForSingleObject(m_read_overlapped.hEvent, INFINITE);//
                status = GetOverlappedResult(m_WintunHandle, &m_read_overlapped, &len, FALSE);
                printf("WIN32 I/O: TAP Read immediate return [%d,%d]", (int)len, len); 
            }
            else
            {
                printError_Win("SetEvent - read ");
                return -1;
            }  
        }
        /*    for (unsigned int i = 0; i < len; i++)
        {
        printf("%02X", (unsigned char)buffer[i]);
        if ((i + 1) % 8 == 0)
        printf(" ");
        }
        printf("\n");
        */
        // judge if it is an arp packet
        if (eth_hdr->proto == htons(0x0806) && len < 100)
        {
            printf("This is an arp packet.\n");
            memset(&pkt_arp, 0, sizeof(openvpn_arp));
            memcpy(&pkt_arp, buffer + 14, len - 14);
            if (pkt_arp.arp_command == htons(0x0001))
            {
                pkt_arp.arp_command = htons(0x0002);
                memcpy(pkt_arp.mac_dest, pkt_arp.mac_src, 6);
                char dst_ip[64] = "";
                inet_ntop(AF_INET, &pkt_arp.ip_dest, dst_ip, sizeof(dst_ip));
                //sprintf(dst_ip, "%s", inet_ntoa(*(struct in_addr *)(&pkt_arp.ip_dest)));
                if (0 == strcmp(dst_ip, "10.0.0.7"))
                {
                    pkt_arp.arp_command = htons(0x0002);
                    memcpy(eth_hdr->dest, eth_hdr->source, 6);
                    memcpy(eth_hdr->source, &dst_mac, 6);
                    memcpy(pkt_arp.mac_dest, pkt_arp.mac_src, 6);
                    memcpy(pkt_arp.mac_src, &dst_mac, 6);
                    uint32_t tmp = pkt_arp.ip_dest;
                    pkt_arp.ip_dest = pkt_arp.ip_src;     // 
                    pkt_arp.ip_src = tmp;

                    // write pkt_arp
                    memcpy(write_buffer, eth_hdr, sizeof(openvpn_ethhdr));
                    memcpy(write_buffer + sizeof(openvpn_ethhdr), &pkt_arp, sizeof(openvpn_arp));
                    //      WaitForSingleObject(m_WintunHandle, INFINITE);
                    //      status = GetOverlappedResult(m_WintunHandle, &overlapped, &len, FALSE);
                    BOOL ret = WriteFile(m_WintunHandle, (LPCVOID)&write_buffer, 42, &len, &m_write_overlapped); // sizeof(openvpn_ethhdr) + sizeof(openvpn_arp)
                    if (!ret)
                    {
                        DWORD err = GetLastError();
                        if (err == ERROR_IO_PENDING)
                        {
                            WaitForSingleObject(m_write_overlapped.hEvent, INFINITE);
                            status = GetOverlappedResult(m_WintunHandle, &m_write_overlapped, &len, FALSE);
                        }
                        else
                            printError_Win("WriteFile : ");
                    }
                    else
                    {
                        if (!SetEvent(m_write_overlapped.hEvent))
                        {
                            printError_Win("SetEvent - write ");
                        }
                    }
                    printf("Write rarp to tap: ret = %d, len = %d\n", ret, len);
                }
            }
        }
        Sleep(1);
    }
    return 0;
}

int testNDIS::mywrite()
{
    BOOL status;
    DWORD len = 0;
    char write_buffer[65536] = "";
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = 0;
    /* manual reset event, initially set according to event_state */
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (overlapped.hEvent == NULL)
    {
        printError_Win("Error: overlapped_io_init: CreateEvent failed");
    }

    openvpn_ethhdr *eth_hdr = (openvpn_ethhdr *)write_buffer;
    openvpn_arp *pkt_arp = (openvpn_arp *)(write_buffer + 14);
    uint16_t arp_proto = 0x0806;
    unsigned char dst_mac[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    unsigned char src_mac[8] = { 0x00, 0xff, 0xf6, 0x6c, 0x1e, 0x9b }; 

    while (true)                     // operation completed immediately?   
    {
        memcpy(eth_hdr->dest,   dst_mac, 6);
        memcpy(eth_hdr->source, src_mac, 6);
        eth_hdr->proto = htons(arp_proto);
        pkt_arp->arp_command = htons(0x0002);
        pkt_arp->ip_src = inet_addr("172.18.11.11");
        if (status = WriteFile(m_WintunHandle, write_buffer, sizeof(openvpn_ethhdr) + sizeof(openvpn_arp), &len, &overlapped))
        {
            /* since we got an immediate return, we must signal the event object ourselves */
            if (!SetEvent(overlapped.hEvent))
            {
                printError_Win("SetEvent - write ");
            }
            printf("WIN32 I/O: TAP write immediate return [%d,%d]", (int)len, len);
            for (unsigned int i = 0; i < len; i++)
            {
                printf("%02X", (unsigned char)write_buffer[i]);
                if ((i + 1) % 8 == 0)
                    printf(" ");
            }
            printf("\n");

        }
        else
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {//当错误是ERROR_IO_PENDING,那意味着读文件的操作还在进行中
             //等候，直到文件读完
            //    WaitForSingleObject(m_WintunHandle, INFINITE);
            //    status = GetOverlappedResult(m_WintunHandle, &overlapped, &len, FALSE);
                //上面二条语句完成的功能与下面一条语句的功能等价：
           /*     printf("WIN32 I/O: TAP write immediate return [%d,%d]", (int)len, len);
                for (int i = 0; i < len; i++)
                {
                    printf("%02X", (unsigned char)write_buffer[i]);
                    if ((i + 1) % 8 == 0)
                        printf(" ");
                }
                printf("\n");*/

            }
            else
            {
                printError_Win("SetEvent - write ");
            }
        }
        Sleep(1);
    }
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
