/******************************************************************************/
/*                                                                            */
/*   Copyright (c) International Business Machines  Corp., 2005, 2006         */
/*                                                                            */
/*   This program is free software;  you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by     */
/*   the Free Software Foundation; either version 2 of the License, or        */
/*   (at your option) any later version.                                      */
/*                                                                            */
/*   This program is distributed in the hope that it will be useful,          */
/*   but WITHOUT ANY WARRANTY;  without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                */
/*   the GNU General Public License for more details.                         */
/*                                                                            */
/*   You should have received a copy of the GNU General Public License        */
/*   along with this program;  if not, write to the Free Software             */
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA  */
/*                                                                            */
/******************************************************************************/


/*
 * File:
 *	ns-traffic.h
 *
 * Description:
 *	Header file for TCP/UDP traffic utilities
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Oct 19 2005 - Created (Mitsuru Chinen)
 *	May  1 2006 - Added functions for broken_ip, route, multicast tests
 *---------------------------------------------------------------------------*/

#ifndef _NS_TRAFFIC_H
#define _NS_TRAFFIC_H 1

/*
 * Gloval variables
 */
#ifdef NS_COMMON
#  define EXTERN
#else
#  define EXTERN extern
#endif
EXTERN int debug;		/* If nonzero, output debug information. */

/*
 * Include headers
 */
#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>


/*
 * Fixed Values
 */
#define PORTNUMMIN		IPPORT_RESERVED + 1
#define PORTNUMMAX		0xFFFF
#define ETH_DATA_MAXSIZE	1500
#define IPV4_PACKET_ID		0xBEEF
#define IPV4_DEFAULT_TTL	64
#define IPV4_DEFAULT_FLAG	0x4000
#define IPV4_PAYLOAD_MAXSIZE	1480
#define IPV6_DEFAULT_HOPLIMIT	255
#define IPV6_PAYLOAD_MAXSIZE	1460
#define ICMP_ECHO_ID		0xCAFE
#define ICMPV4_DATA_MAXSIZE	1472
#define ICMPV6_DATA_MAXSIZE	1452
#define RDOPT_MAXSIZE		1412

#define FAKE_VERSION	0x01
#define FAKE_IHL	0x02
#define FAKE_TOT_LEN	0x04
#define FAKE_PLEN	0x04
#define FAKE_FRAGMENT	0x08
#define FAKE_PROTOCOL	0x10
#define FAKE_NXT	0x10
#define FAKE_CHECK	0x20
#define FAKE_DADDR	0x40

#define PROC_IFINET6_FILE		"/proc/net/if_inet6"
#define PROC_IFINET6_FILE_LINELENGTH	64
#define PROC_IFINET6_LINKLOCAL		0x20


/*
 * Structure definition
 */
struct eth_frame {
    struct ethhdr hdr;
    unsigned char data[ETH_DATA_MAXSIZE];
};

struct arp_datagram {
    struct arphdr hdr;
    unsigned char ar_sha[ETH_ALEN];
    unsigned char ar_sip[4];
    unsigned char ar_tha[ETH_ALEN];
    unsigned char ar_tip[4];
};

struct ip4_datagram {
    struct iphdr hdr;
    unsigned char payload[IPV4_PAYLOAD_MAXSIZE];
};

struct icmp4_segment {
    struct icmphdr hdr;
    unsigned char data[ICMPV4_DATA_MAXSIZE];
};

struct ip6_datagram {
    struct ip6_hdr hdr;
    unsigned char payload[IPV6_PAYLOAD_MAXSIZE];
};

struct pseudo_ip6_hdr {
    struct in6_addr p_ip6_src;
    struct in6_addr p_ip6_dst;
    u_int32_t p_ip6_plen;
    u_int16_t p_ip6_zero1;
    u_int8_t p_ip6_zero2;
    u_int8_t p_ip6_nxt;
};

struct pseudo_ip6_datagram {
    struct pseudo_ip6_hdr hdr;
    unsigned char payload[IPV6_PAYLOAD_MAXSIZE];
};

struct icmp6_segment {
    struct icmp6_hdr hdr;
    unsigned char data[ICMPV6_DATA_MAXSIZE];
};

struct neighbor_sol {
    struct nd_neighbor_solicit defs;
    struct nd_opt_hdr sla_opt;
    unsigned char src_laddr[ETH_ALEN];
};

struct neighbor_adv {
    struct nd_neighbor_advert defs;
    struct nd_opt_hdr tla_opt;
    unsigned char tgt_laddr[ETH_ALEN];
};

struct neighbor_redirect {
    struct nd_redirect defs;
    struct nd_opt_rd_hdr rdopt_hdr;
    unsigned rdopt_data[RDOPT_MAXSIZE];
};


struct hbh_router_alert {
    uint8_t  nxthdr;
    uint8_t  hbh_len;
    uint8_t  alart_type;
    uint8_t  alart_len;
    uint16_t alart_data;
    uint8_t  padn_type;
    uint8_t  padn_len;
    uint8_t  padn_data[0];
};


#if !defined(__LITTLE_ENDIAN_BITFIELD) || !defined(__BIG_ENDIAN_BITFIELD)
#  include <endian.h>
#endif
struct my_mldv2_query {
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint16_t maxdelay;
    uint16_t reserved;
    struct in6_addr addr;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t qrv:3;
    uint8_t suppress:1;
    uint8_t resv:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t resv:4;
    uint8_t suppress:1;
    uint8_t qrv:3;
#else
#  error "Failed to detect endian"
#endif
    uint8_t qqic;
    uint16_t nsrcs;
    struct in6_addr srcs[0];
};


/*
 * Macros
 */
#define MY_IGMPV3_QUERY_SIZE(numsrc) \
		(sizeof(struct igmpv3_query) + (numsrc) * sizeof(uint32_t))

#define MY_MLDV2_QUERY_SIZE(numsrc) \
		(sizeof(struct my_mldv2_query) \
		 + (numsrc) * sizeof(struct in6_addr))

#ifdef MLD_MAX_HOST_REPORT_DELAY
#  define MY_MLD_MAX_HOST_REPORT_DELAY	MLD_MAX_HOST_REPORT_DELAY
#else
#  define MY_MLD_MAX_HOST_REPORT_DELAY	1000
#endif

#define IN6ADDR_ALLNODES_MULTICAST_INIT \
	    { { { 0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

/*
 * Functions in ns-common.c
 */
void fatal_error(char *errmsg);
void maximize_sockbuf(int sd);
u_int16_t calc_checksum(u_int16_t *data, size_t size);
void fill_payload(unsigned char *payload_p, size_t size);
int rand_within(int first, int last);
u_int32_t bit_change_seed(size_t bitsize, size_t oversize);
int eth_pton(int af, const char *str, struct sockaddr_ll *ll);
void get_ifinfo(struct ifreq *ans, int sock_fd, const char *ifname, int query);
int strtotimespec(const char *str, struct timespec *ts_p);
int get_a_lla_byifindex(struct sockaddr_in6 *lla_p, int ifindex);
struct addrinfo *get_maddrinfo(sa_family_t family, const char *maddr, const char *portnum);
struct group_req *create_group_info(uint32_t ifindex, struct addrinfo *mainfo_p);
struct group_filter *create_source_filter(uint32_t ifindex, struct addrinfo *mainfo_p, uint32_t fmode, char *saddrs);

#endif
