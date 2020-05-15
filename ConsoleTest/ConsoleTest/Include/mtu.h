/*
 *  OpenVPN -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2018 OpenVPN Inc <sales@openvpn.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MTU_H
#define MTU_H

//#include "buffer.h"

/*
 *
 * Packet maninipulation routes such as encrypt, decrypt, compress, decompress
 * are passed a frame buffer that looks like this:
 *
 *    [extra_frame bytes] [mtu bytes] [extra_frame_bytes] [compression overflow bytes]
 *                         ^
 *                   Pointer passed to function points here so that routine
 *                   can make use of extra_frame bytes before pointer
 *                   to prepend headers, etc.
 *
 *    extra_frame bytes is large enough for all encryption related overhead.
 *
 *    mtu bytes will be the MTU size set in the ifconfig statement that configures
 *      the TUN or TAP device such as:
 *
 *      ifconfig $1 10.1.0.2 pointopoint 10.1.0.1 mtu 1450
 *
 *    Compression overflow bytes is the worst-case size expansion that would be
 *    expected if we tried to compress mtu + extra_frame bytes of uncompressible data.
 */

/*
 * Standard ethernet MTU
 */
#define ETHERNET_MTU       1500

/*
 * It is a fatal error if mtu is less than
 * this value for tun device.
 */
#define TUN_MTU_MIN        100

/*
 * Default MTU of network over which tunnel data will pass by TCP/UDP.
 */
#define LINK_MTU_DEFAULT   1500

/*
 * Default MTU of tunnel device.
 */
#define TUN_MTU_DEFAULT    1500

/*
 * MTU Defaults for TAP devices
 */
#define TAP_MTU_EXTRA_DEFAULT  32

/*
 * Default MSSFIX value, used for reducing TCP MTU size
 */
#define MSSFIX_DEFAULT     1450

/*
 * Alignment of payload data such as IP packet or
 * ethernet frame.
 */
#define PAYLOAD_ALIGN 4


/**************************************************************************/
/**
 * Packet geometry parameters.
 */
struct frame {
    int link_mtu;               /**< Maximum packet size to be sent over
                                 *   the external network interface. */

    int link_mtu_dynamic;       /**< Dynamic MTU value for the external
                                 *   network interface. */

    int extra_frame;            /**< Maximum number of bytes that all
                                 *   processing steps together could add.
                                 *   @code
                                 *   frame.link_mtu = "socket MTU" - extra_frame;
                                 *   @endcode
                                 */

    int extra_buffer;           /**< Maximum number of bytes that
                                 *   processing steps could expand the
                                 *   internal work buffer.
                                 *
                                 *   This is used by the \link compression
                                 *   Data Channel Compression
                                 *   module\endlink to give enough working
                                 *   space for worst-case expansion of
                                 *   incompressible content. */

    int extra_tun;              /**< Maximum number of bytes in excess of
                                 *   the tun/tap MTU that might be read
                                 *   from or written to the virtual
                                 *   tun/tap network interface. */

    int extra_link;             /**< Maximum number of bytes in excess of
                                 *   external network interface's MTU that
                                 *   might be read from or written to it. */

    /*
     * Alignment control
     */
#define FRAME_HEADROOM_MARKER_DECRYPT     (1<<0)
#define FRAME_HEADROOM_MARKER_FRAGMENT    (1<<1)
#define FRAME_HEADROOM_MARKER_READ_LINK   (1<<2)
#define FRAME_HEADROOM_MARKER_READ_STREAM (1<<3)
    unsigned int align_flags;
    int align_adjust;
};

/* Forward declarations, to prevent includes */
struct options;

/* Routines which read struct frame should use the macros below */

/*
 * Overhead added to packet payload due to encapsulation
 */
#define EXTRA_FRAME(f)           ((f)->extra_frame)

/*
 * Delta between tun payload size and final TCP/UDP datagram size
 * (not including extra_link additions)
 */
#define TUN_LINK_DELTA(f)        ((f)->extra_frame + (f)->extra_tun)

/*
 * This is the size to "ifconfig" the tun or tap device.
 */
#define TUN_MTU_SIZE(f)          ((f)->link_mtu - TUN_LINK_DELTA(f))
#define TUN_MTU_SIZE_DYNAMIC(f)  ((f)->link_mtu_dynamic - TUN_LINK_DELTA(f))

/*
 * This is the maximum packet size that we need to be able to
 * read from or write to a tun or tap device.  For example,
 * a tap device ifconfiged to an MTU of 1200 might actually want
 * to return a packet size of 1214 on a read().
 */
#define PAYLOAD_SIZE(f)          ((f)->link_mtu - (f)->extra_frame)
#define PAYLOAD_SIZE_DYNAMIC(f)  ((f)->link_mtu_dynamic - (f)->extra_frame)

/*
 * Max size of a payload packet after encryption, compression, etc.
 * overhead is added.
 */
#define EXPANDED_SIZE(f)         ((f)->link_mtu)
#define EXPANDED_SIZE_DYNAMIC(f) ((f)->link_mtu_dynamic)
#define EXPANDED_SIZE_MIN(f)     (TUN_MTU_MIN + TUN_LINK_DELTA(f))

/*
 * These values are used as maximum size constraints
 * on read() or write() from TUN/TAP device or TCP/UDP port.
 */
#define MAX_RW_SIZE_TUN(f)       (PAYLOAD_SIZE(f))
#define MAX_RW_SIZE_LINK(f)      (EXPANDED_SIZE(f) + (f)->extra_link)

/*
 * Control buffer headroom allocations to allow for efficient prepending.
 */
#define FRAME_HEADROOM_BASE(f)     (TUN_LINK_DELTA(f) + (f)->extra_buffer + (f)->extra_link)
#define FRAME_HEADROOM(f)          frame_headroom(f, 0)
#define FRAME_HEADROOM_ADJ(f, fm)  frame_headroom(f, fm)

/*
 * Max size of a buffer used to build a packet for output to
 * the TCP/UDP port.
 */
#define BUF_SIZE(f)              (TUN_MTU_SIZE(f) + FRAME_HEADROOM_BASE(f) * 2)


#endif /* ifndef MTU_H */
