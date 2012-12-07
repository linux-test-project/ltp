/******************************************************************************/
/*                                                                            */
/*   Copyright (c) International Business Machines  Corp., 2006               */
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
 *	ns-icmpv6_sender.c
 *
 * Description:
 *	This is ICMPv6 message (Echo request / MLDv2 query) sender.
 *	This utility is also able to set illegal information in the IP header
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Mar 15 2006 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

/*
 * Header Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>

#include "ns-mcast.h"
#include "ns-traffic.h"

/*
 * Structure Definitions
 */
struct icmp6_info {
	struct ip6_datagram pkt;
	struct sockaddr_ll saddr_ll;
	struct sockaddr_ll daddr_ll;
	struct in6_addr saddr;
	struct in6_addr daddr;
	unsigned short int pkt_size;
	unsigned short int data_size;
	double timeout;
	struct timespec interval;

	u_int16_t fake_flag;
};

/*
 * Gloval variables
 */
char *program_name;		/* program name */
struct sigaction handler;	/* Behavior for a signal */
int catch_sighup;		/* When catch the SIGHUP, set to non-zero */
struct in6_addr in6addr_allnodes = IN6ADDR_ALLNODES_MULTICAST_INIT;

/*
 * Function: usage()
 *
 * Descripton:
 *  Print the usage of this program. Then, terminate this program with
 *  the specified exit value.
 *
 * Argument:
 *  exit_value:	exit value
 *
 * Return value:
 *  This function does not return.
 */
void usage(char *program_name, int exit_value)
{
	FILE *stream = stdout;	/* stream where the usage is output */

	if (exit_value == EXIT_FAILURE)
		stream = stderr;

	fprintf(stream, "%s [OPTION]\n"
		"\t-I if_name\tInterface name of the source host\n"
		"\t-S ip_addr\tIPv6 address of the source host\n"
		"\t-M mac_addr\tMAC address of the destination host\n"
		"\t-D ip_addr\tIPv6 address of the destination host\n"
		"\t-t value\ttimeout [sec]\n"
		"\t-w value\tinterval [nanosec]\n"
		"\t-o\t\tsend only one ICMPv6 message\n"
		"\t-b\t\twork in the background\n"
		"\t-d\t\tdisplay debug informations\n"
		"\t-h\t\tdisplay this usage\n"
		"\n"
		"\t[options for echo request]\n"
		"\t  -s packetsize\tsize of data (exclude header)\n"
		"\n"
		"\t[options for fake]\n"
		"\t  -i\t\tbreak IPv6 destination address\n"
		"\t  -L\t\tbreak payload length\n"
		"\t  -n\t\tbreak next header\n"
		"\t  -v\t\tbreak IP version\n"
		"\n"
		"\t[options for MLDv2 query]\n"
		"\t  -m\t\tsend MLDv2 query\n"
		"\t  -a addrs\tcomma separated array of Source Addresses\n"
		"\t  -r value\tMax Resp Code\n", program_name);
	exit(exit_value);
}

/*
 * Function: set_signal_flag()
 *
 * Description:
 *  This function sets global variables according to signal
 *
 * Argument:
 *  type: type of signal
 *
 * Return value:
 *  None
 */
void set_signal_flag(int type)
{
	if (debug)
		fprintf(stderr, "Catch signal. type is %d\n", type);

	switch (type) {
	case SIGHUP:
		catch_sighup = 1;
		handler.sa_handler = SIG_IGN;
		if (sigaction(type, &handler, NULL) < 0)
			fatal_error("sigaction()");
		break;

	default:
		fprintf(stderr, "Unexpected signal (%d) is caught\n", type);
		exit(EXIT_FAILURE);
	}
}

/*
 * Function: specify_hw_addr()
 *
 * Description:
 *  This function specifies the hardware address from the interface name
 *
 * Argument:
 *   lladdr_p:	pointer to the sockaddr_ll structure
 *    ifname:	interface name where icmpv6 messages go out
 *
 * Return value:
 *  None
 *
 */
void specify_hw_addr(struct sockaddr_ll *lladdr_p, const char *ifname)
{
	int sock_fd;		/* Socket for ioctl() */
	struct ifreq ifinfo;	/* Interface information */

	if ((sock_fd = socket(AF_PACKET, SOCK_DGRAM, 0)) < 0)
		fatal_error("socket()");

	lladdr_p->sll_family = AF_PACKET;	/* Always AF_PACKET */
	lladdr_p->sll_protocol = htons(ETH_P_IPV6);	/* IPv6 */
	lladdr_p->sll_hatype = ARPHRD_ETHER;	/* Header type */
	lladdr_p->sll_pkttype = PACKET_HOST;	/* Packet type */
	lladdr_p->sll_halen = ETH_ALEN;	/* Length of address */

	/* Get the MAC address of the interface at source host */
	get_ifinfo(&ifinfo, sock_fd, ifname, SIOCGIFHWADDR);
	memcpy(lladdr_p->sll_addr, ifinfo.ifr_hwaddr.sa_data, ETH_ALEN);

	/* Get the interface index */
	lladdr_p->sll_ifindex = if_nametoindex(ifname);
	close(sock_fd);
}

/*
 * Function: calc_hd_mcastaddr
 *
 * Description:
 *  This function calculate multicast hardware address from IPv6
 *  multicast address
 *
 * Argument:
 *   lladdr_p:	pointer to the sockaddr_ll structure
 *    addr_p:	pointer to the in6_addr structure
 *
 * Return value:
 *  None
 */
void calc_hd_mcastaddr(struct sockaddr_ll *lladdr_p, struct in6_addr *addr_p)
{
	lladdr_p->sll_family = AF_PACKET;	/* Always AF_PACKET */
	lladdr_p->sll_protocol = htons(ETH_P_IPV6);	/* IPv6 */
	lladdr_p->sll_ifindex = 0;	/* Unspecified here */
	lladdr_p->sll_hatype = ARPHRD_ETHER;	/* Header type */
	lladdr_p->sll_pkttype = PACKET_MULTICAST;	/* Packet type */
	lladdr_p->sll_halen = ETH_ALEN;	/* Length of address */

	lladdr_p->sll_addr[0] = 0x33;
	lladdr_p->sll_addr[1] = 0x33;
	memcpy(&lladdr_p->sll_addr[2], &addr_p->s6_addr[12], ETH_ALEN - 2);
}

/*
 * Function: create_mld_query()
 *
 * Description:
 *  This function create a mldv2 query information.
 *
 * Argument:
 *  info_p:	pointer to data of icmp structure
 *   mrc:	Max Resp Code
 *   saddrs:	comma separated array of the source addresses
 *
 * Return value:
 *  0: Success
 *  1: Fail
 */
int create_mld_query(struct icmp6_info *info_p, uint16_t mrc, char *saddrs)
{
	struct ip6_datagram pkt;	/* ICMPv6 packet */
	struct hbh_router_alert *alart_p;	/* pointer to router alart */
	struct my_mldv2_query *query_p;	/* pointer to my_mldv2_query */
	struct pseudo_ip6_datagram pseudo;	/* ICMPv6 pseudo packet for checksum */
	uint16_t numsrc;	/* number of source address */
	unsigned short int ip6_psize;	/* size of IPv6 payload */
	unsigned short int query_size;	/* size of my_mldv2_query */
	struct in6_addr ip6;
	uint32_t idx;
	char *sp, *ep;

	memset(&pkt, '\0', sizeof(struct ip6_datagram));
	alart_p = (struct hbh_router_alert *)&(pkt.payload);
	query_p =
	    (struct my_mldv2_query *)((unsigned char *)alart_p +
				      sizeof(struct hbh_router_alert));

	/* calculate the number of source address */
	if (saddrs == NULL) {
		numsrc = 0;
	} else {
		numsrc = 1;
		for (sp = saddrs; *sp != '\0'; sp++)
			if (*sp == ',')
				numsrc++;
	}
	if (debug)
		fprintf(stderr, "number of source address is %u\n", numsrc);

	query_size = MY_MLDV2_QUERY_SIZE(numsrc);
	ip6_psize = sizeof(struct hbh_router_alert) + query_size;

	/* IPv6 Header */
	pkt.hdr.ip6_vfc = 6 << 4;
	pkt.hdr.ip6_flow |= 0;
	pkt.hdr.ip6_plen = htons(ip6_psize);
	pkt.hdr.ip6_nxt = IPPROTO_HOPOPTS;
	pkt.hdr.ip6_hlim = 1;
	pkt.hdr.ip6_src = info_p->saddr;
	pkt.hdr.ip6_dst = info_p->daddr;

	/* Router Alert Option */
	alart_p->nxthdr = IPPROTO_ICMPV6;
	alart_p->hbh_len = 0;
	alart_p->alart_type = 0x05;	/* router alert */
	alart_p->alart_len = 0x02;	/* data len */
	alart_p->alart_data = htons(0x0000);	/* MLD */
	alart_p->padn_type = 0x01;	/* PadN option */
	alart_p->padn_len = 0x00;	/* 2 Octets */

	/* MLDv2 query */
	query_p->type = MLD_LISTENER_QUERY;
	query_p->code = 0;
	query_p->cksum = 0;	/* Calculate later */
	query_p->maxdelay = htons(mrc);
	query_p->resv = 0;
	query_p->suppress = 0;
	query_p->qrv = 0;
	query_p->qqic = 0;
	query_p->nsrcs = htons(numsrc);

	/* define the multicast address */
	if (memcmp(&info_p->daddr, &in6addr_allnodes, sizeof(struct in6_addr))
	    == 0)
		query_p->addr = in6addr_any;
	else
		query_p->addr = info_p->daddr;

	/* substitute source addresses */
	sp = saddrs;
	for (idx = 0; idx < numsrc; idx++) {
		ep = strchr(sp, ',');
		if (ep != NULL)
			*ep = '\0';
		if (debug)
			fprintf(stderr, "source address[%u]: %s\n", idx, sp);

		if (inet_pton(AF_INET6, sp, &ip6) <= 0) {
			fprintf(stderr,
				"source address list is something wrong\n");
			return 1;
		}
		query_p->srcs[idx] = ip6;
		sp = ep + 1;
	}

	/* ICMPv6 Pseudo packet */
	pseudo.hdr.p_ip6_src = pkt.hdr.ip6_src;
	pseudo.hdr.p_ip6_dst = pkt.hdr.ip6_dst;
	pseudo.hdr.p_ip6_plen = htons(query_size);
	pseudo.hdr.p_ip6_zero1 = 0;
	pseudo.hdr.p_ip6_zero2 = 0;
	pseudo.hdr.p_ip6_nxt = IPPROTO_ICMPV6;
	memcpy(pseudo.payload, query_p, query_size);

	/* Calcualte checksums */
	query_p->cksum = calc_checksum((u_int16_t *) (&pseudo),
				       sizeof(struct pseudo_ip6_hdr) +
				       query_size);

	/* Store the clean packet data */
	info_p->pkt = pkt;
	info_p->pkt_size = sizeof(struct ip6_hdr) + ip6_psize;

	return 0;
}

/*
 * Function: create_echo_request()
 *
 * Description:
 *  This function creates icmpv6 echo request
 *
 * Argument:
 *  info_p: pointer to data of icmp structure
 *
 * Return value:
 *  None
 */
void create_echo_request(struct icmp6_info *info_p)
{
	struct ip6_datagram pkt;	/* ICMPv6 packet */
	struct icmp6_segment *echoreq_p;	/* Echo request header and payload */
	struct pseudo_ip6_datagram pseudo;	/* ICMPv6 pseudo packet for checksum */
	unsigned short int ip6_psize;	/* payload size */

	ip6_psize = sizeof(struct icmp6_hdr)	/* ICMP header */
	    +info_p->data_size;	/* ICMP payload */
	memset(&pkt, '\0', sizeof(struct ip6_datagram));
	echoreq_p = (struct icmp6_segment *)&(pkt.payload);

	/* IPv6 Header */
	pkt.hdr.ip6_vfc = 6 << 4;
	pkt.hdr.ip6_flow |= 0;
	pkt.hdr.ip6_plen = htons(ip6_psize);
	pkt.hdr.ip6_nxt = IPPROTO_ICMPV6;
	pkt.hdr.ip6_hlim = IPV6_DEFAULT_HOPLIMIT;
	pkt.hdr.ip6_src = info_p->saddr;
	pkt.hdr.ip6_dst = info_p->daddr;

	/* Echo Request Header */
	echoreq_p->hdr.icmp6_type = ICMP6_ECHO_REQUEST;
	echoreq_p->hdr.icmp6_code = 0;
	echoreq_p->hdr.icmp6_cksum = 0;	/* Calculate later */
	echoreq_p->hdr.icmp6_id = htons(ICMP_ECHO_ID);
	echoreq_p->hdr.icmp6_seq = htons(1);

	/* Echo Request Payload */
	fill_payload(echoreq_p->data, info_p->data_size);

	/* ICMPv6 Pseudo packet */
	pseudo.hdr.p_ip6_src = pkt.hdr.ip6_src;
	pseudo.hdr.p_ip6_dst = pkt.hdr.ip6_dst;
	pseudo.hdr.p_ip6_plen = htons(ip6_psize);
	pseudo.hdr.p_ip6_zero1 = 0;
	pseudo.hdr.p_ip6_zero2 = 0;
	pseudo.hdr.p_ip6_nxt = IPPROTO_ICMPV6;
	memcpy(pseudo.payload, echoreq_p, ip6_psize);

	/* Calcualte checksums */
	echoreq_p->hdr.icmp6_cksum = calc_checksum((u_int16_t *) (&pseudo),
						   sizeof(struct pseudo_ip6_hdr)
						   + ip6_psize);

	/* Store the clean packet data */
	info_p->pkt = pkt;
	info_p->pkt_size = sizeof(struct ip6_hdr) + ip6_psize;
}

/*
 * Function: parse_options()
 *
 * Description:
 *  This function parse the options
 *
 * Argument:
 *   argc:  the number of argument
 *   argv:  arguments
 *  info_p: pointer to data of icmp data to modify
 *   bg_p:  pointer to the flag of working in backgrond
 *
 * Return value:
 *  None
 */
void parse_options(int argc, char *argv[], struct icmp6_info *info_p, int *bg_p)
{
	int optc;		/* option */
	unsigned long opt_ul;	/* option value in unsigned long */
	double opt_d;		/* option value in double */
	struct in6_addr opt_addr;	/* option value in struct in_addr */
	struct sockaddr_ll opt_addr_ll;	/* option value in struct sockaddr_ll */
	char *ifname;		/* interface name where datagrams go out */
	int is_mld_query;	/* set to non-zero if sending MLDv2 query */
	char *mld_saddrs;	/* comma separated array of source addresses */
	uint16_t max_resp;	/* Max Resp Code */
	int is_specified_daddr_ll = 0;
	int is_specified_saddr = 0;
	int is_specified_daddr = 0;

	ifname = NULL;
	is_mld_query = 0;
	mld_saddrs = NULL;
	max_resp = MY_MLD_MAX_HOST_REPORT_DELAY;

	while ((optc =
		getopt(argc, argv, "I:S:M:D:t:w:obdhs:iLnvma:r:")) != EOF) {
		switch (optc) {
		case 'I':
			if (if_nametoindex(optarg) == 0) {
				fprintf(stderr,
					"specified interface is incorrect\n");
				usage(program_name, EXIT_FAILURE);
			}
			ifname = strdup(optarg);
			if (ifname == NULL)
				fatal_error("strdup() failed.");
			break;

		case 'S':
			if (inet_pton(AF_INET6, optarg, &opt_addr) <= 0) {
				fprintf(stderr, "Source address is wrong\n");
				usage(program_name, EXIT_FAILURE);
			}
			info_p->saddr = opt_addr;
			is_specified_saddr = 1;
			break;

		case 'M':
			if (eth_pton(AF_INET6, optarg, &opt_addr_ll)) {
				fprintf(stderr,
					"Destination MAC address is wrong\n");
				usage(program_name, EXIT_FAILURE);
			}
			info_p->daddr_ll = opt_addr_ll;
			is_specified_daddr_ll = 1;
			break;

		case 'D':
			if (inet_pton(AF_INET6, optarg, &opt_addr) <= 0) {
				fprintf(stderr,
					"Destination address is wrong\n");
				usage(program_name, EXIT_FAILURE);
			}
			info_p->daddr = opt_addr;
			is_specified_daddr = 1;
			break;

		case 't':
			opt_d = strtod(optarg, NULL);
			if (opt_d < 0.0) {
				fprintf(stderr,
					"Timeout should be positive value\n");
				usage(program_name, EXIT_FAILURE);
			}
			info_p->timeout = opt_d;
			break;

		case 'w':
			if (strtotimespec(optarg, &info_p->interval)) {
				fprintf(stderr,
					"Interval is something wrong\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 'o':
			info_p->timeout = -1.0;
			break;

		case 'b':
			*bg_p = 1;
			break;

		case 'd':
			debug = 1;
			break;

		case 'h':
			usage(program_name, EXIT_SUCCESS);
			break;

			/* Options for echo request */
		case 's':
			opt_ul = strtoul(optarg, NULL, 0);
			if (opt_ul > ICMPV6_DATA_MAXSIZE) {
				fprintf(stderr,
					"Data size sholud be less than %d\n",
					ICMPV6_DATA_MAXSIZE + 1);
				usage(program_name, EXIT_FAILURE);
			}
			info_p->data_size = opt_ul;
			break;

			/* Options for fake */
		case 'i':
			info_p->fake_flag |= FAKE_DADDR;
			break;

		case 'L':
			info_p->fake_flag |= FAKE_PLEN;
			break;

		case 'n':
			info_p->fake_flag |= FAKE_NXT;
			break;

		case 'v':
			info_p->fake_flag |= FAKE_VERSION;
			break;

			/* Options for MLDv2 query */
		case 'm':
			is_mld_query = 1;
			break;

		case 'a':
			mld_saddrs = strdup(optarg);
			if (mld_saddrs == NULL)
				fatal_error("strdup()");
			break;

		case 'r':
			opt_ul = strtoul(optarg, NULL, 0);
			if (opt_ul > 0xFFFF) {
				fprintf(stderr,
					"Max Resp Code should be less than 65536\n");
				usage(program_name, EXIT_FAILURE);
			}
			max_resp = opt_ul;
			break;

		default:
			usage(program_name, EXIT_FAILURE);
		}
	}

	if (ifname == NULL) {
		fprintf(stderr, "Outgoing interface is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}
	specify_hw_addr(&info_p->saddr_ll, ifname);

	if (!is_specified_saddr) {
		fprintf(stderr, "Source IP address is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}

	if (is_mld_query) {	/* MLDv2 query */
		if (info_p->fake_flag) {
			fprintf(stderr,
				"It is not permitted to break MLDv2 query\n");
			usage(program_name, EXIT_FAILURE);
		}

		if (!is_specified_daddr)
			info_p->daddr = in6addr_allnodes;

		calc_hd_mcastaddr(&info_p->daddr_ll, &info_p->daddr);
		if (create_mld_query(info_p, max_resp, mld_saddrs))
			exit(EXIT_FAILURE);
	} else {		/* echo request */
		if (info_p->fake_flag)
			srand(getpid());

		if (!is_specified_daddr_ll) {
			fprintf(stderr,
				"Destination MAC address is not specified\n");
			usage(program_name, EXIT_FAILURE);
		}

		if (!is_specified_daddr) {
			fprintf(stderr,
				"Destination IP address is not specified\n");
			usage(program_name, EXIT_FAILURE);
		}

		create_echo_request(info_p);
	}
	info_p->daddr_ll.sll_ifindex = if_nametoindex(ifname);
}

/*
 * Function: thrust_fakes()
 *
 * Description:
 *  This function thrust fake information to the icmp packet
 *
 * Argument:
 *     pkt   : Payload of the Ethernet frame (Namely, IPv6 packet)
 *  fake_flag: Flag which represents what information would be faked
 *
 * Return value:
 *  None
 */
void thrust_fakes(struct ip6_datagram *pkt, u_int16_t fake_flag)
{
	int rand_val;
	size_t bitsize;
	u_int32_t seed;

	if (debug)
		fprintf(stderr, "fake_flag = %2x\n", fake_flag);

	if (fake_flag & FAKE_VERSION) {	/* version */
		bitsize = 4;
		seed = bit_change_seed(bitsize, 1);
		pkt->hdr.ip6_vfc ^= (seed << 4);
	}

	if (fake_flag & FAKE_PLEN) {	/* total length */
		bitsize = sizeof(pkt->hdr.ip6_plen) * 8;
		seed = bit_change_seed(bitsize, bitsize / 8);
		pkt->hdr.ip6_plen ^= seed;
	}

	if (fake_flag & FAKE_NXT) {	/* next header */
		rand_val = rand() / ((RAND_MAX + 1U) / 5);
		switch (rand_val) {
		case 1:
		case 2:
			if (debug)
				fprintf(stderr, "Bit reverse\n");
			bitsize = sizeof(pkt->hdr.ip6_nxt) * 8;
			seed = bit_change_seed(bitsize, 0);
			pkt->hdr.ip6_nxt ^= seed;
			break;

		case 3:
		case 4:
			if (debug)
				fprintf(stderr, "Unknown Protocol\n");
			if (rand_val) {
				int number;
				int counter;
				for (counter = 0; counter <= 0xff; counter++) {
					number =
					    rand() / ((RAND_MAX + 1U) / 0x100);
					if (getprotobynumber(number) == NULL) {
						pkt->hdr.ip6_nxt = number;
						break;
					}
				}
			}
			break;

		default:
			if (debug)
				fprintf(stderr, "Do nothing\n");
			break;
		}
	}

	if (fake_flag & FAKE_DADDR) {	/* destination address */
		rand_val = rand() / ((RAND_MAX + 1U) / 4);
		bitsize = sizeof(pkt->hdr.ip6_dst.s6_addr32[rand_val]) * 8;
		seed = bit_change_seed(bitsize, bitsize / 8);
		pkt->hdr.ip6_dst.s6_addr32[rand_val] ^= seed;
	}
}

/*
 * Function: send_packet()
 *
 * Description:
 *  This function sends icmpv6 packet
 *
 * Argument:
 *  info_p: pointer to data of icmp structure
 *
 * Return value:
 *  None
 */
void send_packets(struct icmp6_info *info_p)
{
	int sock_fd;
	int retval;
	struct ip6_datagram pkt;
	double start_time;

	/* Open a socket */
	sock_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IPV6));
	if (sock_fd < 0)
		fatal_error("socket()");

	/* Bind the socket to the physical address */
	retval = bind(sock_fd, (struct sockaddr *)&(info_p->saddr_ll),
		      sizeof(struct sockaddr_ll));
	if (retval < 0)
		fatal_error("bind()");

	/* Set singal hander for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigfillset()");

	/*
	 * loop for sending packets
	 */
	pkt = info_p->pkt;
	start_time = time(NULL);

	for (;;) {
		if (info_p->fake_flag) {
			pkt = info_p->pkt;
			thrust_fakes(&pkt, info_p->fake_flag);
		}

		retval = sendto(sock_fd, &pkt, info_p->pkt_size, 0,
				(struct sockaddr *)&(info_p->daddr_ll),
				sizeof(struct sockaddr_ll));
		if (retval < 0)
			fatal_error("sendto()");

		/* Check timeout:
		   If timeout value is negative only send one datagram */
		if (info_p->timeout)
			if (info_p->timeout < difftime(time(NULL), start_time))
				break;

		/* Wait in specified interval */
		nanosleep(&info_p->interval, NULL);

		if (catch_sighup)	/* catch SIGHUP */
			break;
	}

	/* Close the socket */
	close(sock_fd);
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	struct icmp6_info icmp6_data;
	int background = 0;

	debug = 0;
	program_name = strdup(argv[0]);

	memset(&icmp6_data, '\0', sizeof(struct icmp6_info));
	parse_options(argc, argv, &icmp6_data, &background);

	if (background)		/* Work in the background */
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	send_packets(&icmp6_data);

	exit(EXIT_SUCCESS);
}
