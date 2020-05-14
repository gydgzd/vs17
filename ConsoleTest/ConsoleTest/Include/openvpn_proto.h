#pragma once
#pragma pack(1)
#include <stdint.h>
#include <ws2ipdef.h>
/*
 * Tunnel types
 */
#define DEV_TYPE_UNDEF 0
#define DEV_TYPE_NULL  1
#define DEV_TYPE_TUN   2    /* point-to-point IP tunnel */
#define DEV_TYPE_TAP   3    /* ethernet (802.3) tunnel */

 /* TUN topologies */

#define TOP_UNDEF   0
#define TOP_NET30   1
#define TOP_P2P     2
#define TOP_SUBNET  3

/*
 * IP and Ethernet protocol structs.  For portability,
 * OpenVPN needs its own definitions of these structs, and
 * names have been adjusted to avoid collisions with
 * native structs.
 */

#define OPENVPN_ETH_ALEN 6            /* ethernet address length */
struct openvpn_ethhdr
{
	uint8_t dest[OPENVPN_ETH_ALEN];   /* destination ethernet addr */
	uint8_t source[OPENVPN_ETH_ALEN]; /* source ethernet addr   */

#define OPENVPN_ETH_P_IPV4   0x0800   /* IPv4 protocol */
#define OPENVPN_ETH_P_IPV6   0x86DD   /* IPv6 protocol */
#define OPENVPN_ETH_P_ARP    0x0806   /* ARP protocol */
#define OPENVPN_ETH_P_8021Q  0x8100   /* 802.1Q protocol */
	uint16_t proto;                   /* packet type ID field */
};

struct openvpn_8021qhdr
{
	uint8_t dest[OPENVPN_ETH_ALEN];     /* destination ethernet addr */
	uint8_t source[OPENVPN_ETH_ALEN];   /* source ethernet addr	*/

	uint16_t tpid;                      /* 802.1Q Tag Protocol Identifier */
#define OPENVPN_8021Q_MASK_PCP htons(0xE000) /* mask PCP out of pcp_cfi_vid */
#define OPENVPN_8021Q_MASK_CFI htons(0x1000) /* mask CFI out of pcp_cfi_vid */
#define OPENVPN_8021Q_MASK_VID htons(0x0FFF) /* mask VID out of pcp_cfi_vid */
	uint16_t pcp_cfi_vid;               /* bit fields, see IEEE 802.1Q */
	uint16_t proto;                     /* contained packet type ID field */
};

/*
 * Size difference between a regular Ethernet II header and an Ethernet II
 * header with additional IEEE 802.1Q tagging.
 */
#define SIZE_ETH_TO_8021Q_HDR (sizeof(struct openvpn_8021qhdr) \
                               - sizeof(struct openvpn_ethhdr))


struct openvpn_arp {
#define ARP_MAC_ADDR_TYPE 0x0001
	uint16_t mac_addr_type;     /* 0x0001 */

	uint16_t proto_addr_type;   /* 0x0800 */
	uint8_t mac_addr_size;      /* 0x06 */
	uint8_t proto_addr_size;    /* 0x04 */

#define ARP_REQUEST 0x0001
#define ARP_REPLY   0x0002
	uint16_t arp_command;       /* 0x0001 for ARP request, 0x0002 for ARP reply */

	uint8_t mac_src[OPENVPN_ETH_ALEN];
	uint32_t ip_src;
	uint8_t mac_dest[OPENVPN_ETH_ALEN]; 
	uint32_t ip_dest;
};

struct openvpn_iphdr {
#define OPENVPN_IPH_GET_VER(v) (((v) >> 4) & 0x0F)
#define OPENVPN_IPH_GET_LEN(v) (((v) & 0x0F) << 2)
	uint8_t version_len;

	uint8_t tos;
	uint16_t tot_len;
	uint16_t id;

#define OPENVPN_IP_OFFMASK 0x1fff
	uint16_t frag_off;

	uint8_t ttl;

#define OPENVPN_IPPROTO_IGMP    2  /* IGMP protocol */
#define OPENVPN_IPPROTO_TCP     6  /* TCP protocol */
#define OPENVPN_IPPROTO_UDP    17  /* UDP protocol */
#define OPENVPN_IPPROTO_ICMPV6 58 /* ICMPV6 protocol */
	uint8_t protocol;

	uint16_t check;
	uint32_t saddr;
	uint32_t daddr;
	/*The options start here. */
};

/*
 * IPv6 header
 */
struct openvpn_ipv6hdr {
	uint8_t version_prio;
	uint8_t flow_lbl[3];
	uint16_t payload_len;
	uint8_t nexthdr;
	uint8_t hop_limit;

	struct  in6_addr saddr;
	struct  in6_addr daddr;
};

/*
 * ICMPv6 header
 */
struct openvpn_icmp6hdr {
#define OPENVPN_ICMP6_DESTINATION_UNREACHABLE       1
#define OPENVPN_ND_ROUTER_SOLICIT                 133
#define OPENVPN_ND_ROUTER_ADVERT                  134
#define OPENVPN_ND_NEIGHBOR_SOLICIT               135
#define OPENVPN_ND_NEIGHBOR_ADVERT                136
#define OPENVPN_ND_INVERSE_SOLICIT                141
#define OPENVPN_ND_INVERSE_ADVERT                 142
	uint8_t icmp6_type;
#define OPENVPN_ICMP6_DU_NOROUTE                    0
#define OPENVPN_ICMP6_DU_COMMUNICATION_PROHIBTED    1
	uint8_t icmp6_code;
	uint16_t icmp6_cksum;
	uint8_t icmp6_dataun[4];
};

/*
 * UDP header
 */
struct openvpn_udphdr {
	uint16_t source;
	uint16_t dest;
	uint16_t len;
	uint16_t check;
};

/*
 * TCP header, per RFC 793.
 */
struct openvpn_tcphdr {
	uint16_t source;       /* source port */
	uint16_t dest;         /* destination port */
	uint32_t seq;          /* sequence number */
	uint32_t ack_seq;      /* acknowledgement number */

#define OPENVPN_TCPH_GET_DOFF(d) (((d) & 0xF0) >> 2)
	uint8_t doff_res;

#define OPENVPN_TCPH_FIN_MASK (1<<0)
#define OPENVPN_TCPH_SYN_MASK (1<<1)
#define OPENVPN_TCPH_RST_MASK (1<<2)
#define OPENVPN_TCPH_PSH_MASK (1<<3)
#define OPENVPN_TCPH_ACK_MASK (1<<4)
#define OPENVPN_TCPH_URG_MASK (1<<5)
#define OPENVPN_TCPH_ECE_MASK (1<<6)
#define OPENVPN_TCPH_CWR_MASK (1<<7)
	uint8_t flags;

	uint16_t window;
	uint16_t check;
	uint16_t urg_ptr;
};

#define OPENVPN_TCPOPT_EOL     0
#define OPENVPN_TCPOPT_NOP     1
#define OPENVPN_TCPOPT_MAXSEG  2
#define OPENVPN_TCPOLEN_MAXSEG 4

struct ip_tcp_udp_hdr {
	struct openvpn_iphdr ip;
	union {
		struct openvpn_tcphdr tcp;
		struct openvpn_udphdr udp;
	} u;
};

#pragma pack()