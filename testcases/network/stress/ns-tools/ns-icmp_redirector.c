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
 *	ns-icmp_redirector.c
 *
 * Description:
 *	This is ICMPv4/ICMPv6 redirect message sender.
 *	The host under test assume the host where this utility run is a
 *	gateway. When the utility receives the packet from the host
 *	under test. This utility reply ICMP redirect message.
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Mar 31 2006 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

/*
 * Header Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>

#include "ns-traffic.h"

/*
 * Structure definition
 */
struct redirector_info {
	int sd;
	char *ifname;
	double timeout;
};

struct ip4_gateway_info {
	unsigned char hd_addr[ETH_ALEN];
	unsigned char ip_addr[4];
	unsigned char nexthop[4];
};

struct ip6_gateway_info {
	unsigned char hd_addr[ETH_ALEN];
	struct in6_addr ip_addr;
	struct in6_addr nexthop;
};

/*
 * Gloval variables
 */
char *program_name;		/* program name */
struct sigaction handler;	/* Behavior for a signal */
int catch_sighup;		/* When catch the SIGHUP, set to non-zero */

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
		"\t-I if_name\tInterface where input/output packets\n"
		"\t-t value\ttimeout [sec]\n"
		"\t-b\t\ttimeout [sec]\n"
		"\t-d\t\tdisplay debug informations\n"
		"\t-h\t\tdisplay this usage\n", program_name);
	exit(exit_value);
}

/*
 * Function: set_signal_flag()
 *
 * Description:
 *  This function sets global variables according to a signal
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
 * Function: parse_options()
 *
 * Description:
 *  This function parse the options
 *
 * Argument:
 *     argc:	    number of argument
 *     argv:	    arguments
 *  redirector_p:   pointer to data for the redirector information
 *     bg_p:	    pointer to the flag of working in backgrond
 *
 * Return value:
 *  None
 */
void
parse_options(int argc, char *argv[], struct redirector_info *redirector_p,
	      int *bg_p)
{
	int optc;		/* option */
	double opt_d;		/* option value in double */

	while ((optc = getopt(argc, argv, "I:N:t:bdh")) != EOF) {
		switch (optc) {
		case 'I':
			redirector_p->ifname = strdup(optarg);
			if (redirector_p->ifname == NULL)
				fatal_error("strdup() failed.");
			break;

		case 't':
			opt_d = strtod(optarg, NULL);
			if (opt_d < 0.0) {
				fprintf(stderr,
					"Timeout should be positive value\n");
				usage(program_name, EXIT_FAILURE);
			}
			redirector_p->timeout = opt_d;
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

		default:
			usage(program_name, EXIT_FAILURE);
		}
	}

	if (redirector_p->ifname == NULL) {
		fprintf(stderr, "Interface name is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}
}

/*
 * Function: open_socket()
 *
 * Description:
 *  This function opens a socket for capture/sending
 *
 * Argument:
 *  ifname: interface name
 *
 * Return value:
 *  socket file descriptor for receiving packets
 */
int open_socket(const char *ifname)
{
	int sd;			/* Socket to packets */
	struct ifreq ifinfo;	/* Interface information */
	struct sockaddr_ll lla;	/* Link-local address info for receiving */

	/* Create a socket for capture */
	if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
		fatal_error("socket()");

	/* Make a socket into non-blocking mode */
	if (fcntl(sd, F_SETFL, O_NONBLOCK) < 0)
		fatal_error("fcntl()");

	/* Get the logical interface number */
	get_ifinfo(&ifinfo, sd, ifname, SIOCGIFINDEX);

	/* Bind to the interface */
	memset(&lla, '\0', sizeof(struct sockaddr_ll));
	lla.sll_family = PF_PACKET;
	lla.sll_protocol = htons(ETH_P_ALL);
	lla.sll_ifindex = ifinfo.ifr_ifindex;
	if (bind(sd, (struct sockaddr *)&lla, sizeof(struct sockaddr_ll)) < 0)
		fatal_error("bind()");

	/* Change into the promiscuous mode */
	get_ifinfo(&ifinfo, sd, ifname, SIOCGIFFLAGS);
	ifinfo.ifr_flags = ifinfo.ifr_flags | IFF_PROMISC;
	if (ioctl(sd, SIOCSIFFLAGS, &ifinfo) < 0)
		fatal_error("ioctl()");
	if (debug)
		fprintf(stderr, "%s is changed into promiscuous mode\n",
			ifname);

	if (debug)
		fprintf(stderr, "Packet receiving socket is %d\n", sd);
	return sd;
}

/*
 * Function: return_arp_reply()
 *
 * Description:
 *  This function returns arp reply message to arp request message.
 *  And it updates the IPv4 gateway information.
 *
 * Argument:
 *      sd     : socket to send arp reply message
 *   rcveth_p  : pointer to ether frame data
 *   gateway_p : pointer to IPv4 gateway information
 *
 * Return value:
 *  None
 */
void
return_arp_reply(int sd, struct eth_frame *rcveth_p,
		 struct ip4_gateway_info *gateway_p)
{
	int retval;
	struct arp_datagram *rcvarp_p;	/* ARP part of receiving frame */
	unsigned char new_hd_addr[ETH_ALEN];	/* New MAC address */
	unsigned char new_nexthop[4];	/* New next hop */
	size_t sndeth_size;	/* Size of sending frame */
	struct eth_frame sndeth;	/* sending frame */
	struct arp_datagram *sndarp_p;	/* ARP part of sending frame */

	rcvarp_p = (struct arp_datagram *)&(rcveth_p->data);

	/* If arp message is not arp request, do nothing */
	if (debug)
		fprintf(stderr, "ARP OP code is %02x\n",
			ntohs(rcvarp_p->hdr.ar_op));
	if (rcvarp_p->hdr.ar_op != htons(ARPOP_REQUEST))
		return;

	/* Update the gateway information */
	memset(new_hd_addr, '\0', ETH_ALEN);	/* MAC address */
	for (;;) {
		new_hd_addr[3] = rand_within(0, 254);
		new_hd_addr[4] = rand_within(0, 254);
		new_hd_addr[5] = rand_within(1, 254);
		if (memcmp(gateway_p->hd_addr, new_hd_addr, ETH_ALEN)) {
			memcpy(gateway_p->hd_addr, new_hd_addr, ETH_ALEN);
			break;
		}
	}

	memcpy(gateway_p->ip_addr, rcvarp_p->ar_tip, 4);	/* IP address */

	for (;;) {		/* next hop */
		memcpy(new_nexthop, gateway_p->ip_addr, 4);
		new_nexthop[3] = rand_within(1, 254);
		if (memcmp(gateway_p->nexthop, new_nexthop, 4)) {
			memcpy(gateway_p->nexthop, new_nexthop, 4);
			break;
		}
	}

	/* Build a frame to send */
	memset(&sndeth, '\0', sizeof(struct eth_frame));
	sndarp_p = (struct arp_datagram *)&(sndeth.data);
	sndeth_size = sizeof(struct ethhdr) + sizeof(struct arp_datagram);

	/* Ether */
	memcpy(sndeth.hdr.h_dest, rcveth_p->hdr.h_source, ETH_ALEN);
	memcpy(sndeth.hdr.h_source, gateway_p->hd_addr, ETH_ALEN);
	sndeth.hdr.h_proto = htons(ETH_P_ARP);

	/* Arp */
	sndarp_p->hdr.ar_hrd = htons(ARPHRD_ETHER);
	sndarp_p->hdr.ar_pro = htons(ETH_P_IP);
	sndarp_p->hdr.ar_hln = ETH_ALEN;
	sndarp_p->hdr.ar_pln = 4;
	sndarp_p->hdr.ar_op = htons(ARPOP_REPLY);
	memcpy(sndarp_p->ar_sha, gateway_p->hd_addr, ETH_ALEN);
	memcpy(sndarp_p->ar_sip, gateway_p->ip_addr, 4);
	memcpy(sndarp_p->ar_tha, rcvarp_p->ar_sha, ETH_ALEN);
	memcpy(sndarp_p->ar_tip, rcvarp_p->ar_sip, 4);

	/* Send ARP reply */
	retval = write(sd, &sndeth, sndeth_size);
	if (retval != sndeth_size)
		fatal_error("write()");
}

/*
 * Function: return_icmp4_redirect()
 *
 * Description:
 *  This function returns icmp redirect message
 *
 * Argument:
 *      sd     : socket to send arp reply message
 *   rcveth_p  : pointer to ether frame data
 *  rcveth_size: size of received ehter frame
 *    new_gw_p : pointer to new IPv4 gateway information
 *
 * Return value:
 *  None
 */
void
return_icmp4_redirect(int sd, struct eth_frame *rcveth_p, size_t rcveth_size,
		      struct ip4_gateway_info *new_gw_p)
{
	static struct ip4_gateway_info *gw_p;	/* pointor to gateway */

	int retval;
	struct ip4_datagram *rcvip_p;	/* IPv4 part of receiving frame */
	size_t sndeth_size;	/* Size of sending frame */
	struct eth_frame sndeth;	/* sending frame */
	struct ip4_datagram *sndip_p;	/* IPv4 part of sending frame */
	struct icmp4_segment *sndicmp_p;	/* ICMPv4 part of sending frame */
	size_t icmp4_datasize;	/* Size of sending ICMPv4 */

	/* If MAC address in received frame is changed, update the gateway info */
	if (memcmp(rcveth_p->hdr.h_dest, new_gw_p->hd_addr, ETH_ALEN) == 0) {
		if (gw_p == NULL)
			if ((gw_p = malloc(sizeof(struct ip4_gateway_info))) == NULL)
				fatal_error("malloc()");
		*gw_p = *new_gw_p;
	} else if (gw_p == NULL
		   || memcmp(rcveth_p->hdr.h_dest, gw_p->hd_addr, ETH_ALEN))
		return;

	rcvip_p = (struct ip4_datagram *)&(rcveth_p->data);

	/* Build a frame to send */
	sndeth_size = sizeof(struct ethhdr)	/* Ether header */
	    +sizeof(struct iphdr)	/* IPv4 header */
	    +sizeof(struct icmphdr)	/* ICMPv4 header */
	    +rcveth_size - sizeof(struct ethhdr);	/* ICMPv4 payload */
	sndeth_size = (sndeth_size < ETH_DATA_MAXSIZE)
	    ? sndeth_size : ETH_DATA_MAXSIZE;
	memset(&sndeth, '\0', sizeof(struct eth_frame));
	sndip_p = (struct ip4_datagram *)&(sndeth.data);
	sndicmp_p = (struct icmp4_segment *)&(sndip_p->payload);

	/* Ether */
	memcpy(sndeth.hdr.h_dest, rcveth_p->hdr.h_source, ETH_ALEN);
	memcpy(sndeth.hdr.h_source, gw_p->hd_addr, ETH_ALEN);
	sndeth.hdr.h_proto = htons(ETH_P_IP);

	/* IP */
	sndip_p->hdr.version = 4;
	sndip_p->hdr.ihl = sizeof(struct iphdr) / 4;
	sndip_p->hdr.tos = 0;
	sndip_p->hdr.tot_len = htons(sndeth_size - sizeof(struct ethhdr));
	sndip_p->hdr.id = htons(IPV4_PACKET_ID);
	sndip_p->hdr.frag_off = htons(IPV4_DEFAULT_FLAG);
	sndip_p->hdr.ttl = IPV4_DEFAULT_TTL;
	sndip_p->hdr.protocol = IPPROTO_ICMP;
	sndip_p->hdr.check = 0;	/* Calculate later */
	memcpy((unsigned char *)&sndip_p->hdr.saddr, gw_p->ip_addr, 4);
	sndip_p->hdr.daddr = rcvip_p->hdr.saddr;
	sndip_p->hdr.check = calc_checksum((u_int16_t *) & (sndip_p->hdr),
					   sizeof(struct iphdr));

	/* ICMP */
	sndicmp_p->hdr.type = ICMP_REDIRECT;
	sndicmp_p->hdr.code = ICMP_REDIR_HOST;
	sndicmp_p->hdr.checksum = 0;	/* Calculate later */
	memcpy((unsigned char *)&(sndicmp_p->hdr.un.gateway), gw_p->nexthop, 4);

	/* ICMP payload */
	icmp4_datasize = rcveth_size - sizeof(struct ethhdr);
	icmp4_datasize =
	    (icmp4_datasize <
	     ICMPV4_DATA_MAXSIZE) ? icmp4_datasize : ICMPV4_DATA_MAXSIZE;
	memcpy(sndicmp_p->data, rcvip_p, icmp4_datasize);

	/* Calculate ICMP checksum */
	sndicmp_p->hdr.checksum = calc_checksum((u_int16_t *) sndicmp_p,
						sizeof(struct icmphdr) +
						icmp4_datasize);

	/* Send ICMP redirect */
	retval = write(sd, &sndeth, sndeth_size);
	if (retval != sndeth_size)
		fatal_error("write()");
}

/*
 * Function: return_neigh_adv()
 *
 * Description:
 *  This function returns neighbor advertisement message
 *  And this updates the gateway information.
 *
 * Argument:
 *      sd     : socket to send arp reply message
 *   rcveth_p  : pointer to ether frame data
 *  current_eth: current MAC address
 *   gateway_p : pointer to IPv6 gateway information
 *
 * Return value:
 *  None
 */
void
return_neigh_adv(int sd, struct eth_frame *rcveth_p,
		 struct ip6_gateway_info *gateway_p)
{
	int retval;
	struct ip6_datagram *rcvip6_p;	/* IPv6 part of receiving frame */
	struct neighbor_sol *rcvns_p;	/* NS part of receiving frame */
	unsigned char new_hd_addr[ETH_ALEN];	/* new MAC address */
	struct in6_addr new_nexthop;	/* new next hop */
	size_t sndeth_size;	/* size of sending frame */
	struct eth_frame sndeth;	/* sending frame */
	struct ip6_datagram *sndip6_p;	/* IPv6 part of sending frame */
	struct pseudo_ip6_datagram p_ip6;	/* pseudo IP header */
	struct neighbor_adv *sndna_p;	/* NA part of sending frame */

	rcvip6_p = (struct ip6_datagram *)&(rcveth_p->data);
	rcvns_p = (struct neighbor_sol *)&(rcvip6_p->payload);

	/* If NS is DAD NS, do nothing */
	if (memcmp
	    (&(rcvip6_p->hdr.ip6_src), &in6addr_any,
	     sizeof(struct in6_addr)) == 0) {
		if (debug) {
			fprintf(stderr, "Received NS is a DAD NS\n");
			return;
		}
	}

	/* Update the gateway information */
	memset(new_hd_addr, '\0', ETH_ALEN);	/* MAC address */
	for (;;) {
		new_hd_addr[3] = rand_within(0, 254);
		new_hd_addr[4] = rand_within(0, 254);
		new_hd_addr[5] = rand_within(1, 254);
		if (memcmp(gateway_p->hd_addr, new_hd_addr, ETH_ALEN)) {
			memcpy(gateway_p->hd_addr, new_hd_addr, ETH_ALEN);
			break;
		}
	}

	gateway_p->ip_addr = rcvns_p->defs.nd_ns_target;	/* IP address */

	for (;;) {		/* next hop */
		memset(&new_nexthop, '\0', sizeof(struct in6_addr));
		new_nexthop.s6_addr[0] = 0xfe;
		new_nexthop.s6_addr[1] = 0x80;
		new_nexthop.s6_addr[15] = rand_within(1, 254);
		if (memcmp
		    (&(gateway_p->nexthop), &new_nexthop,
		     sizeof(struct in6_addr))) {
			gateway_p->nexthop = new_nexthop;
			break;
		}
	}

	/* Build a frame to send */
	sndeth_size = sizeof(struct ethhdr) + sizeof(struct ip6_hdr)
	    + sizeof(struct neighbor_adv);
	memset(&sndeth, '\0', sizeof(struct eth_frame));
	sndip6_p = (struct ip6_datagram *)&(sndeth.data);
	sndna_p = (struct neighbor_adv *)&(sndip6_p->payload);

	/* Ether */
	memcpy(sndeth.hdr.h_dest, rcvns_p->src_laddr, ETH_ALEN);
	memcpy(sndeth.hdr.h_source, gateway_p->hd_addr, ETH_ALEN);
	sndeth.hdr.h_proto = htons(ETH_P_IPV6);

	/* IPv6 */
	sndip6_p->hdr.ip6_vfc = 6 << 4;
	sndip6_p->hdr.ip6_flow |= 0;
	sndip6_p->hdr.ip6_plen = htons(sizeof(struct neighbor_adv));
	sndip6_p->hdr.ip6_nxt = IPPROTO_ICMPV6;
	sndip6_p->hdr.ip6_hlim = 255;
	sndip6_p->hdr.ip6_src = gateway_p->ip_addr;
	sndip6_p->hdr.ip6_dst = rcvip6_p->hdr.ip6_src;

	/* Neighbor Advertisement */
	sndna_p->defs.nd_na_type = ND_NEIGHBOR_ADVERT;
	sndna_p->defs.nd_na_code = 0;
	sndna_p->defs.nd_na_cksum = 0;	/* Calculate later */
	sndna_p->defs.nd_na_target = gateway_p->ip_addr;
	sndna_p->defs.nd_na_flags_reserved
	    = ND_NA_FLAG_ROUTER | ND_NA_FLAG_SOLICITED | ND_NA_FLAG_OVERRIDE;
	sndna_p->tla_opt.nd_opt_type = ND_OPT_TARGET_LINKADDR;
	sndna_p->tla_opt.nd_opt_len = 1;
	memcpy(sndna_p->tgt_laddr, &(gateway_p->hd_addr), ETH_ALEN);

	/* Pseudo IPv6 datagram for checksum calculation */
	memset(&p_ip6, '\0', sizeof(struct pseudo_ip6_datagram));
	p_ip6.hdr.p_ip6_src = sndip6_p->hdr.ip6_src;
	p_ip6.hdr.p_ip6_dst = sndip6_p->hdr.ip6_dst;
	p_ip6.hdr.p_ip6_plen = sndip6_p->hdr.ip6_plen;
	p_ip6.hdr.p_ip6_zero1 = 0;
	p_ip6.hdr.p_ip6_zero2 = 0;
	p_ip6.hdr.p_ip6_nxt = sndip6_p->hdr.ip6_nxt;
	memcpy(p_ip6.payload, sndna_p, sizeof(struct neighbor_adv));

	/* Calculate checksum */
	sndna_p->defs.nd_na_cksum = calc_checksum((u_int16_t *) (&p_ip6),
						  sizeof(struct pseudo_ip6_hdr)
						  +
						  sizeof(struct neighbor_adv));

	/* Send Neighbor Advertisement reply */
	retval = write(sd, &sndeth, sndeth_size);
	if (retval != sndeth_size)
		fatal_error("write()");
}

/*
 * Function: return_icmp6_redirect()
 *
 * Description:
 *  This function returns an ICMPv6 redirect message
 *
 * Argument:
 *      sd     : socket to send arp reply message
 *   rcveth_p  : pointer to ether frame data
 *  rcveth_size: size of received ehter frame
 *   new_gw_p  : pointer to new IPv4 gateway information
 *
 * Return value:
 *  None
 */
void
return_icmp6_redirect(int sd, struct eth_frame *rcveth_p, size_t rcveth_size,
		      struct ip6_gateway_info *new_gw_p)
{
	static struct ip6_gateway_info *gw_p = NULL;	/* pointor to gateway */

	int retval;
	struct ip6_datagram *rcvip6_p;	/* IPv6 part of receiving frame */
	struct eth_frame sndeth;	/* sending frame */
	size_t sndeth_size;	/* size of sending frame */
	struct ip6_datagram *sndip6_p;	/* IPv6 part of sending frame */
	size_t ip6_payload_size;	/* payload size of IPv6 part */
	struct pseudo_ip6_datagram p_ip6;	/* pseudo header for checksum */
	struct neighbor_redirect *sndrd_p;	/* ICMPv6 part of sending frame */
	size_t redirect_optsize;	/* Option size of ICMPv6 */

	rcvip6_p = (struct ip6_datagram *)&(rcveth_p->data);

	/* If MAC address in received frame is changed, update the gateway info */
	if (memcmp(rcveth_p->hdr.h_dest, new_gw_p->hd_addr, ETH_ALEN) == 0) {
		if (gw_p == NULL)
			if ((gw_p = malloc(sizeof(struct in6_addr))) == NULL)
				fatal_error("malloc()");
		*gw_p = *new_gw_p;
	} else if (gw_p == NULL
		   || memcmp(rcveth_p->hdr.h_dest, gw_p->hd_addr, ETH_ALEN))
		return;

	/* Build a frame to send */
	memset(&sndeth, '\0', sizeof(struct eth_frame));
	sndip6_p = (struct ip6_datagram *)&(sndeth.data);
	sndrd_p = (struct neighbor_redirect *)&(sndip6_p->payload);
	redirect_optsize = sizeof(struct nd_opt_rd_hdr)
	    + rcveth_size - sizeof(struct ethhdr);
	redirect_optsize = (redirect_optsize < RDOPT_MAXSIZE)
	    ? redirect_optsize : RDOPT_MAXSIZE;
	ip6_payload_size = sizeof(struct nd_redirect) + redirect_optsize;
	sndeth_size = sizeof(struct ethhdr) + sizeof(struct ip6_hdr)
	    + ip6_payload_size;

	/* Ether */
	memcpy(sndeth.hdr.h_dest, rcveth_p->hdr.h_source, ETH_ALEN);
	memcpy(sndeth.hdr.h_source, gw_p->hd_addr, ETH_ALEN);
	sndeth.hdr.h_proto = htons(ETH_P_IPV6);

	/* IPv6 */
	sndip6_p->hdr.ip6_vfc = 6 << 4;
	sndip6_p->hdr.ip6_flow |= 0;
	sndip6_p->hdr.ip6_plen = htons(ip6_payload_size);
	sndip6_p->hdr.ip6_nxt = IPPROTO_ICMPV6;
	sndip6_p->hdr.ip6_hlim = 255;
	sndip6_p->hdr.ip6_src = gw_p->ip_addr;
	sndip6_p->hdr.ip6_dst = rcvip6_p->hdr.ip6_src;

	/* Rediret Message */
	sndrd_p->defs.nd_rd_type = ND_REDIRECT;
	sndrd_p->defs.nd_rd_code = 0;
	sndrd_p->defs.nd_rd_cksum = 0;	/* Calculate later */
	sndrd_p->defs.nd_rd_reserved = 0;
	sndrd_p->defs.nd_rd_target = gw_p->nexthop;
	sndrd_p->defs.nd_rd_dst = rcvip6_p->hdr.ip6_dst;;
	sndrd_p->rdopt_hdr.nd_opt_rh_type = ND_OPT_REDIRECTED_HEADER;
	sndrd_p->rdopt_hdr.nd_opt_rh_len = redirect_optsize / 8;
	memcpy(sndrd_p->rdopt_data, rcvip6_p, redirect_optsize);

	/* Pseudo IPv6 datagram for checksum calculation */
	memset(&p_ip6, '\0', sizeof(struct pseudo_ip6_datagram));
	p_ip6.hdr.p_ip6_src = sndip6_p->hdr.ip6_src;
	p_ip6.hdr.p_ip6_dst = sndip6_p->hdr.ip6_dst;
	p_ip6.hdr.p_ip6_plen = sndip6_p->hdr.ip6_plen;
	p_ip6.hdr.p_ip6_zero1 = 0;
	p_ip6.hdr.p_ip6_zero2 = 0;
	p_ip6.hdr.p_ip6_nxt = sndip6_p->hdr.ip6_nxt;
	memcpy(p_ip6.payload, sndrd_p, ip6_payload_size);

	/* Calculate checksum */
	sndrd_p->defs.nd_rd_cksum = calc_checksum((u_int16_t *) (&p_ip6),
						  sizeof(struct pseudo_ip6_hdr)
						  + ip6_payload_size);

	/* Send ICMPv6 redirct message */
	retval = write(sd, &sndeth, sndeth_size);
	if (retval != sndeth_size)
		fatal_error("write()");
}

/*
 * Function: analyze_ip6_datagram()
 *
 * Description:
 *  This function analyze captured IPv6 datagram
 *
 * Argument:
 *       sd      : socket to send arp reply message
 *    rcveth_p   : pointer to ether frame data
 *   rcveth_size : size of received ehter frame
 *    gateway_p  : pointer to IPv6 gateway information
 *
 * Return value:
 *  None
 */
void
analyze_ip6_datagram(int sd, struct eth_frame *rcveth_p, size_t rcveth_size,
		     struct ip6_gateway_info *gateway_p)
{
	struct ip6_datagram *rcvip6_p;	/* IPv6 Part of receiving frame */
	struct icmp6_segment *rcvicmp6_p;	/* ICMPv6 Part of receiving frame */
	uint8_t nxt_hdr;	/* Next header of IPv6 */
	uint8_t icmp6_type;	/* Type of ICMPv6 */

	rcvip6_p = (struct ip6_datagram *)&(rcveth_p->data);
	rcvicmp6_p = (struct icmp6_segment *)&(rcvip6_p->payload);

	nxt_hdr = rcvip6_p->hdr.ip6_nxt;
	switch (nxt_hdr) {
	case IPPROTO_ICMPV6:
		icmp6_type = rcvicmp6_p->hdr.icmp6_type;
		switch (icmp6_type) {
		case ND_NEIGHBOR_SOLICIT:
			if (debug)
				fprintf(stderr, "Received ICMP NS\n");
			return_neigh_adv(sd, rcveth_p, gateway_p);
			break;

		case ICMP6_ECHO_REQUEST:
			if (debug)
				fprintf(stderr, "Received ICMP Echo Request\n");
			return_icmp6_redirect(sd, rcveth_p, rcveth_size,
					      gateway_p);
			break;
		}
		break;

	case IPPROTO_UDP:
		if (debug)
			fprintf(stderr, "Received UDP message\n");
		return_icmp6_redirect(sd, rcveth_p, rcveth_size, gateway_p);
		break;
	}
}

/*
 * Function: capture_frames()
 *
 * Description:
 *  This function captures frames
 *
 * Argument:
 *  redirector_p: pointer to data for the redirector information
 *
 * Return value:
 *  socket file descriptor for receiving packets
 */
void capture_frames(struct redirector_info *redirector_p)
{
	struct ip4_gateway_info ip4_gateway;	/* IPv4 gateway information */
	struct ip6_gateway_info ip6_gateway;	/* IPv6 gateway information */
	struct eth_frame frame;	/* captured frame data */
	ssize_t frame_size;	/* captured frame size */
	double start_time;	/* capture starting time */
	int sd = redirector_p->sd;	/* socket fd for capture */

	/* Initialize gateway information */
	memset(&ip4_gateway, '\0', sizeof(struct ip4_gateway_info));
	memset(&ip6_gateway, '\0', sizeof(struct ip6_gateway_info));

	/* Set singal hander for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigfillset()");

	/*
	 * loop for capture
	 */
	start_time = time(NULL);

	for (;;) {
		frame_size = read(sd, (void *)(&frame), sizeof(frame));
		if (frame_size < 0) {
			if (errno != EAGAIN)
				fatal_error("read()");
		} else {
			switch (ntohs(frame.hdr.h_proto)) {
			case ETH_P_ARP:
				if (debug)
					fprintf(stderr, "Get ARP packet\n");
				return_arp_reply(sd, &frame, &ip4_gateway);
				break;

			case ETH_P_IP:
				if (debug)
					fprintf(stderr, "Get IPv4 packet\n");
				return_icmp4_redirect(sd, &frame, frame_size,
						      &ip4_gateway);
				break;

			case ETH_P_IPV6:
				if (debug)
					fprintf(stderr, "Get IPv6 packet\n");
				analyze_ip6_datagram(sd, &frame, frame_size,
						     &ip6_gateway);
				break;
			}
		}

		if (redirector_p->timeout)
			if (redirector_p->timeout <
			    difftime(time(NULL), start_time))
				break;

		if (catch_sighup)	/* catch SIGHUP */
			break;
	}
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	struct redirector_info redirector;
	int background = 0;

	debug = 0;
	program_name = strdup(argv[0]);
	srand(getpid());

	memset(&redirector, '\0', sizeof(struct redirector_info));
	parse_options(argc, argv, &redirector, &background);

	redirector.sd = open_socket(redirector.ifname);

	if (background)		/* Work in the background */
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	capture_frames(&redirector);

	close(redirector.sd);
	exit(EXIT_SUCCESS);
}
