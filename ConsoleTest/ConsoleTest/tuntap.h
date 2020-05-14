#pragma once
#ifdef _WIN32
//#include <ws2def.h>
#include <ws2ipdef.h>
//#include <Windows.h>
//#include <WinDef.h>
//#include <winioctl.h>

//#include <setupapi.h>
//#include <cfgmgr32.h>
//#include <atlstr.h>     // DWORD     , MFC use #include <cstringt.h>
//#include<atlconv.h>    // for T2A
//#include <objbase.h>
//#include <NtDDNdis.h>
#include "openvpn_proto.h"
#include "openvpn_msg.h"
#include "openvpn_plugin.h"
#endif

//#include "win32.h"
using namespace std;
#define in_addr_t uint32_t
#define ssize_t SSIZE_T

#define S_IRUSR 0
#define S_IWUSR 0
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

#define SIGHUP    1
#define SIGINT    2
#define SIGUSR1   10
#define SIGUSR2   12
#define SIGTERM   15

typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
//typedef __int8 int8_t;
typedef uint16_t in_port_t;

#define WINTUN_RING_CAPACITY        0x800000
#define WINTUN_RING_TRAILING_BYTES  0x10000
#define WINTUN_MAX_PACKET_SIZE      0xffff
#define WINTUN_PACKET_ALIGN         4

#define TUN_IOCTL_REGISTER_RINGS CTL_CODE(51820U, 0x970U, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

//=============
// TAP IOCTLs
//=============
#define TAP_CONTROL_CODE(request,method) CTL_CODE (FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)

#define TAP_IOCTL_GET_MAC               TAP_CONTROL_CODE (1, METHOD_BUFFERED)
#define TAP_IOCTL_GET_VERSION           TAP_CONTROL_CODE (2, METHOD_BUFFERED)
#define TAP_IOCTL_GET_MTU               TAP_CONTROL_CODE (3, METHOD_BUFFERED)
#define TAP_IOCTL_GET_INFO              TAP_CONTROL_CODE (4, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_POINT_TO_POINT TAP_CONTROL_CODE (5, 0)
#define TAP_IOCTL_SET_MEDIA_STATUS      TAP_CONTROL_CODE (6, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_MASQ      TAP_CONTROL_CODE (7, METHOD_BUFFERED)
#define TAP_IOCTL_GET_LOG_LINE          TAP_CONTROL_CODE (8, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_SET_OPT   TAP_CONTROL_CODE (9, METHOD_BUFFERED)
#define TAP_CONFIG_TUN                  TAP_CONTROL_CODE (10, METHOD_BUFFERED)

static inline ULONG wintun_ring_wrap(ULONG value)
{
    return value & (WINTUN_RING_CAPACITY - 1);
}
static inline ULONG wintun_ring_packet_align(ULONG size)
{
    return (size + (WINTUN_PACKET_ALIGN - 1)) & ~(WINTUN_PACKET_ALIGN - 1);
}

typedef struct _TUN_RING {
    volatile ULONG Head;
    volatile ULONG Tail;
    volatile LONG Alertable;
    UCHAR Data[];
} TUN_RING;

struct TUN_PACKET_HEADER
{
    uint32_t size;
};
typedef struct _TUN_PACKET {
    ULONG Size;
    UCHAR Data[];
} TUN_PACKET;

typedef struct _TUN_REGISTER_RINGS {
    struct {
        ULONG RingSize;
        TUN_RING *Ring;
        HANDLE TailMoved;
    } Send, Receive;
} TUN_REGISTER_RINGS;
/*
typedef struct in6_addr {
union {
UCHAR       Byte[16];
USHORT      Word[8];
} u;
} IN6_ADDR, *PIN6_ADDR, FAR *LPIN6_ADDR;
*/

/**************************************************************************/
/**
* Wrapper structure for dynamically allocated memory.
*
* The actual content stored in a buffer structure starts at the memory
* location buffer.data + buffer.offset, and has a length of buffer.len bytes.
* This, together with the space available before and after the content,
* is represented in the pseudocode below:

* uint8_t *content_start    = buffer.data + buffer.offset;
* uint8_t *content_end      = buffer.data + buffer.offset + buffer.len;
* int      prepend_capacity = buffer.offset;
* int      append_capacity  = buffer.capacity - (buffer.offset + buffer.len);
* @endcode
*/
struct buffer
{
    int capacity;               /**< Size in bytes of memory allocated by malloc(). */
    int offset;                 /**< Offset in bytes of the actual content within the allocated memory. */
    int len;                    /**< Length in bytes of the actual content within the allocated memory. */
    uint8_t *data;              /**< Pointer to the allocated memory. */

#ifdef BUF_INIT_TRACKING
    const char *debug_file;
    int debug_line;
#endif
};
struct overlapped_io {
#define IOSTATE_INITIAL          0
#define IOSTATE_QUEUED           1  /* overlapped I/O has been queued */
#define IOSTATE_IMMEDIATE_RETURN 2  /* I/O function returned immediately without queueing */
    int iostate;
    OVERLAPPED overlapped;
    DWORD size;
    DWORD flags;
    int status;
    bool addr_defined;
    union {
        struct sockaddr_in addr;
        //    struct sockaddr_in6 addr6;
    };
    int addrlen;
    struct buffer buf_init;
    struct buffer buf;
};
struct rw_handle {
    HANDLE read;
    HANDLE write;
};


#if defined(_WIN32) || defined(TARGET_ANDROID)

#define TUN_ADAPTER_INDEX_INVALID ((DWORD)-1)

/* time constants for --ip-win32 adaptive */
#define IPW32_SET_ADAPTIVE_DELAY_WINDOW 300
#define IPW32_SET_ADAPTIVE_TRY_NETSH    20

struct tuntap_options {
    /* --ip-win32 options */
    bool ip_win32_defined;

#define IPW32_SET_MANUAL       0   /* "--ip-win32 manual" */
#define IPW32_SET_NETSH        1   /* "--ip-win32 netsh" */
#define IPW32_SET_IPAPI        2   /* "--ip-win32 ipapi" */
#define IPW32_SET_DHCP_MASQ    3   /* "--ip-win32 dynamic" */
#define IPW32_SET_ADAPTIVE     4   /* "--ip-win32 adaptive" */
#define IPW32_SET_N            5
    int ip_win32_type;

#ifdef _WIN32
    HANDLE msg_channel;
#endif

    /* --ip-win32 dynamic options */
    bool dhcp_masq_custom_offset;
    int dhcp_masq_offset;
    int dhcp_lease_time;

    /* --tap-sleep option */
    int tap_sleep;

    /* --dhcp-option options */

    bool dhcp_options;

    const char *domain;      /* DOMAIN (15) */

    const char *netbios_scope; /* NBS (47) */

    int netbios_node_type;   /* NBT 1,2,4,8 (46) */

#define N_DHCP_ADDR 4        /* Max # of addresses allowed for
                             * DNS, WINS, etc. */

                             /* DNS (6) */
    in_addr_t dns[N_DHCP_ADDR];
    int dns_len;

    /* WINS (44) */
    in_addr_t wins[N_DHCP_ADDR];
    int wins_len;

    /* NTP (42) */
    in_addr_t ntp[N_DHCP_ADDR];
    int ntp_len;

    /* NBDD (45) */
    in_addr_t nbdd[N_DHCP_ADDR];
    int nbdd_len;

    /* DISABLE_NBT (43, Vendor option 001) */
    bool disable_nbt;

    bool dhcp_renew;
    bool dhcp_pre_release;

    bool register_dns;

    struct in6_addr dns6[N_DHCP_ADDR];
    int dns6_len;
};

#elif TARGET_LINUX

struct tuntap_options {
    int txqueuelen;
};

#else  /* if defined(_WIN32) || defined(TARGET_ANDROID) */

struct tuntap_options {
    int dummy; /* not used */
};

#endif /* if defined(_WIN32) || defined(TARGET_ANDROID) */

struct tuntap
{
#define TUNNEL_TYPE(tt) ((tt) ? ((tt)->type) : DEV_TYPE_UNDEF)
    int type; /* DEV_TYPE_x as defined in proto.h */

#define TUNNEL_TOPOLOGY(tt) ((tt) ? ((tt)->topology) : TOP_UNDEF)
    int topology; /* one of the TOP_x values */

    bool did_ifconfig_setup;
    bool did_ifconfig_ipv6_setup;

    bool persistent_if;         /* if existed before, keep on program end */

    struct tuntap_options options; /* options set on command line */

    char *actual_name; /* actual name of TUN/TAP dev, usually including unit number */

                       /* number of TX buffers */
    int txqueuelen;

    /* ifconfig parameters */
    in_addr_t local;
    in_addr_t remote_netmask;

    //    struct in6_addr local_ipv6;
    //   struct in6_addr remote_ipv6;
    int netbits_ipv6;

#ifdef _WIN32
    HANDLE hand;
    struct overlapped_io reads;
    struct overlapped_io writes;
    struct rw_handle rw_handle;

    /* used for setting interface address via IP Helper API
    * or DHCP masquerade */
    bool ipapi_context_defined;
    ULONG ipapi_context;
    ULONG ipapi_instance;
    in_addr_t adapter_netmask;

    /* Windows adapter index for TAP-Windows adapter,
    * ~0 if undefined */
    DWORD adapter_index;

    bool wintun; /* true if wintun is used instead of tap-windows6 */
    int standby_iter;

    HANDLE wintun_send_ring_handle;
    HANDLE wintun_receive_ring_handle;
    TUN_RING *wintun_send_ring;
    TUN_RING *wintun_receive_ring;
#else  /* ifdef _WIN32 */
    int fd; /* file descriptor for TUN/TAP dev */
#endif

#ifdef TARGET_SOLARIS
    int ip_fd;
#endif

#ifdef HAVE_NET_IF_UTUN_H
    bool is_utun;
#endif
    /* used for printing status info only */
    unsigned int rwflags_debug;

    /* Some TUN/TAP drivers like to be ioctled for mtu
    * after open */
    int post_open_mtu;
};



/**************************************************************************/
/**
* Garbage collection entry for one dynamically allocated block of memory.
*
* This structure represents one link in the linked list contained in a \c
* gc_arena structure.  Each time the \c gc_malloc() function is called,
* it allocates \c sizeof(gc_entry) + the requested number of bytes.  The
* \c gc_entry is then stored as a header in front of the memory address
* returned to the caller.
*/
struct gc_entry
{
    struct gc_entry *next;      /**< Pointer to the next item in the
                                *   linked list. */
};

/**
* Garbage collection entry for a specially allocated structure that needs
* a custom free function to be freed like struct addrinfo
*
*/
struct gc_entry_special
{
    struct gc_entry_special *next;
    void(*free_fnc)(void *);
    void *addr;
};

static inline void check_malloc_return(const void *p);



#define BPTR(buf)  (buf_bptr(buf))
#define BEND(buf)  (buf_bend(buf))
#define BLAST(buf) (buf_blast(buf))
#define BLEN(buf)  (buf_len(buf))
#define BDEF(buf)  (buf_defined(buf))
#define BSTR(buf)  (buf_str(buf))
#define BCAP(buf)  (buf_forward_capacity(buf))

#if defined(__GNUC__)
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x)      (x)
#define unlikely(x)    (x)
#endif




/*
* Manage lists of buffers
*/
struct buffer_entry
{
    struct buffer buf;
    struct buffer_entry *next;
};

struct buffer_list
{
    struct buffer_entry *head; /* next item to pop/peek */
    struct buffer_entry *tail; /* last item pushed */
    int size;                /* current number of entries */
    int max_size;            /* maximum size list should grow to */
};


#define BUF_SIZE_MAX 1000000
void buf_size_error(const size_t size);
static inline bool buf_defined(const struct buffer *buf);



static inline bool buf_valid(const struct buffer *buf)
{
    return likely(buf->data != NULL) && likely(buf->len >= 0);
}

static inline bool buf_size_valid(const size_t size)
{
    return likely(size < BUF_SIZE_MAX);
}
static inline bool buf_safe(const struct buffer *buf, int len)
{
    return buf_valid(buf) && buf_size_valid(len)
        && buf->offset + buf->len + len <= buf->capacity;
}
static inline uint8_t * buf_bptr(const struct buffer *buf)
{
    if (buf_valid(buf))
    {
        return buf->data + buf->offset;
    }
    else
    {
        return NULL;
    }
}
static int buf_len(const struct buffer *buf)
{
    if (buf_valid(buf))
    {
        return buf->len;
    }
    else
    {
        return 0;
    }
}
static inline uint8_t * buf_write_alloc(struct buffer *buf, int size)
{
    uint8_t *ret;
    if (!buf_safe(buf, size))
    {
        return NULL;
    }
    ret = BPTR(buf) + buf->len;
    buf->len += size;
    return ret;
}
static inline bool buf_write(struct buffer *dest, const void *src, int size)
{
    uint8_t *cp = buf_write_alloc(dest, size);
    if (!cp)
    {
        return false;
    }
    memcpy(cp, src, size);
    return true;
}
static inline bool buf_copy(struct buffer *dest, const struct buffer *src)
{
    return buf_write(dest, BPTR(src), BLEN(src));
}

static inline uint8_t * buf_read_alloc(struct buffer *buf, int size)
{
    uint8_t *ret;
    if (size < 0 || buf->len < size)
    {
        return NULL;
    }
    ret = BPTR(buf);
    buf->offset += size;
    buf->len -= size;
    return ret;
}
static inline char * buf_str(const struct buffer *buf)
{
	return (char *)buf_bptr(buf);
}

static inline void buf_reset(struct buffer *buf)
{
	buf->capacity = 0;
	buf->offset = 0;
	buf->len = 0;
	buf->data = NULL;
}

static inline void buf_reset_len(struct buffer *buf)
{
	buf->len = 0;
	buf->offset = 0;
}

static inline bool buf_init_dowork(struct buffer *buf, int offset)
{
	if (offset < 0 || offset > buf->capacity || buf->data == NULL)
	{
		return false;
	}
	buf->len = 0;
	buf->offset = offset;
	return true;
}

static inline void buf_set_write(struct buffer *buf, uint8_t *data, int size)
{
	if (!buf_size_valid(size))
	{
		buf_size_error(size);
	}
	buf->len = 0;
	buf->offset = 0;
	buf->capacity = size;
	buf->data = data;
	if (size > 0 && data)
	{
		*data = 0;
	}
}

static inline void buf_set_read(struct buffer *buf, const uint8_t *data, int size)
{
	if (!buf_size_valid(size))
	{
		buf_size_error(size);
	}
	buf->len = buf->capacity = size;
	buf->offset = 0;
	buf->data = (uint8_t *)data;
}

static inline bool is_ip_packet_valid(const struct buffer *buf)
{
    const struct openvpn_iphdr* ih = (const struct openvpn_iphdr *)BPTR(buf);

    if ((((ih->version_len) >> 4) & 0x0F) == 4)
    {
        if (BLEN(buf) < sizeof(struct openvpn_iphdr))
        {
            return false;
        }
    }
    else if ((((ih->version_len) >> 4) & 0x0F) == 6)
    {
        if (BLEN(buf) < sizeof(struct openvpn_ipv6hdr))
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

#define IA_EMPTY_IF_UNDEF (1<<0)
#define IA_NET_ORDER      (1<<1)
