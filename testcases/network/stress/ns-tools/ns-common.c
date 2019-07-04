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
 *	ns-common.c
 *
 * Description:
 *	Common functions and variables in the ns-tools
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Oct 19 2005 - Created (Mitsuru Chinen)
 *	May  1 2006 - Added functions for broken_ip, route, multicast tests
 *---------------------------------------------------------------------------*/

/*
 * Fixed values
 */
#define PROC_RMEM_MAX	"/proc/sys/net/core/rmem_max"
#define PROC_WMEM_MAX	"/proc/sys/net/core/wmem_max"

/*
 * Standard Header Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>

#include "ns-mcast.h"
#define NS_COMMON 1
#include "ns-traffic.h"

/*
 * Function: fatal_error()
 *
 * Description:
 *  Output an error message then exit the program with EXIT_FAILURE
 *
 * Argument:
 *  errmsg: message printed by perror()
 *
 * Return value:
 *  This function does not return.
 */
void fatal_error(char *errmsg)
{
	perror(errmsg);
	exit(EXIT_FAILURE);
}

/*
 * Function: maximize_sockbuf()
 *
 * Descripton:
 *  This function maximize the send and receive buffer size of a socket
 *
 * Argument:
 *  sd:	target socket descriptor
 *
 * Return value:
 *  None
 */
void maximize_sockbuf(int sd)
{
	size_t idx;
	int level[] = { SO_RCVBUF, SO_SNDBUF };
	char *procfile[] = { PROC_RMEM_MAX, PROC_WMEM_MAX };
	char *bufname[] = { "rcvbuf", "sndbuf" };

	for (idx = 0; idx < (sizeof(level) / sizeof(int)); idx++) {
		FILE *fp;	/* File pointer to a proc file */
		int bufsiz;	/* buffer size of socket */
		unsigned int optlen;	/* size of sd option parameter */

		if ((fp = fopen(procfile[idx], "r")) == NULL) {
			fprintf(stderr, "Failed to open %s\n", procfile[idx]);
			fatal_error("fopen()");
		}
		if ((fscanf(fp, "%d", &bufsiz)) != 1) {
			fprintf(stderr, "Failed to read from %s\n",
				procfile[idx]);
			fatal_error("fscanf()");
		}
		if (setsockopt
		    (sd, SOL_SOCKET, level[idx], &bufsiz, sizeof(int))) {
			fatal_error("setsockopt()");
		}
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close to %s\n",
				procfile[idx]);
			fatal_error("fopen()");
		}

		if (debug) {
			optlen = sizeof(bufsiz);
			if (getsockopt
			    (sd, SOL_SOCKET, level[idx], &bufsiz,
			     &optlen) < 0) {
				fatal_error("getsockopt()");
			}
			fprintf(stderr, "socket %s size is %d\n", bufname[idx],
				bufsiz);
		}
	}
}

/*
 * Function: calc_checksum()
 *
 * Description:
 *  This function calculate the checksum of IPv4 or ICMP
 *
 * Argument:
 *  data: pointer to target data for checksum
 *  size: target data size
 *
 * Return value:
 *  None
 */
u_int16_t calc_checksum(u_int16_t * data, size_t size)
{
	u_int32_t sum;
	u_int16_t *pos;
	size_t rest;

	sum = 0;
	pos = data;
	for (rest = size; rest > 1; rest -= 2)
		sum += *(pos++);

	if (rest > 0)
		sum += (*pos) & 0xff00;

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	sum = ~sum;

	return sum;
}

/*
 * Function: fill_payload()
 *
 * Description:
 *  This function fills the payload
 *
 * Argument:
 *  payload_p: pointer to data of payload
 *    size:    payload size
 *
 * Return value:
 *  None
 */
void fill_payload(unsigned char *payload_p, size_t size)
{
	size_t idx;

	for (idx = 0; idx < size; idx++)
		*(payload_p + idx) = idx % 0x100;
}

/*
 * Function: rand_within()
 *
 * Description:
 *  This function returns a presudo-random integer within specified range
 *
 * Argument:
 *  first: Fisrt value of the range. If negative, assumed 0
 *  last : Last value of the range. If bigger than RAND_MAX, assumed RAND_MAX
 *
 * Return value:
 *  integer value between first to last
 */
int rand_within(int first, int last)
{
	unsigned int num;
	int rand_val;

	first = first < 0 ? 0 : first;
	last = RAND_MAX < (unsigned int)last ? RAND_MAX : last;

	num = last - first + 1U;
	rand_val = rand() / ((RAND_MAX + 1U) / num) + first;

	return rand_val;
}

/*
 * Function: bit_change_seed
 *
 * Description:
 *  This function creates a seed to change 1 bit at random position
 *
 * Argument:
 *  bitsize : bit size of data whose bit would be changed
 *  oversize: This value controls whether a bit is changed or not
 *
 * Return value:
 *  seed of the bit for change.
 */
u_int32_t bit_change_seed(size_t bitsize, size_t oversize)
{
	int rand_val;
	u_int32_t seed;
	rand_val = rand() / ((RAND_MAX + 1U) / (bitsize + oversize));

	seed = (rand_val < bitsize) ? (0x00000001 << rand_val) : 0;

	if (debug)
		fprintf(stderr, "Bit seed is %08x\n", seed);

	return seed;
}

/*
 * Function: eth_pton()
 *
 * Description:
 *  This function convert a string to struct sockaddr_ll (Ethernet)
 *  Note) The ifindex is set to `any'.
 *
 * Argument:
 *   af : AF_INET or AF_INET6
 *   str: Pointer to a string which represents MAC address
 *   ll : pointer to struct sockaddr_ll
 *
 * Return value:
 *    0  : Success
 *    1  : Fail
 */
int eth_pton(int af, const char *str, struct sockaddr_ll *ll)
{
	size_t idx;
	unsigned char *addr_p;
	unsigned int val[ETH_ALEN];

	ll->sll_family = AF_PACKET;	/* Always AF_PACKET */
	if (af == AF_INET)
		ll->sll_protocol = htons(ETH_P_IP);	/* IPv4 */
	else
		ll->sll_protocol = htons(ETH_P_IPV6);	/* IPv6 */
	ll->sll_ifindex = 0;	/* any interface */
	ll->sll_hatype = ARPHRD_ETHER;	/* Header type */
	ll->sll_pkttype = PACKET_OTHERHOST;	/* Packet type */
	ll->sll_halen = ETH_ALEN;	/* Length of address */

	/* Physical layer address */
	if (sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", &val[0], &val[1],
		   &val[2], &val[3], &val[4], &val[5]) != ETH_ALEN) {
		fprintf(stderr, "%s is not a valid MAC address", str);
		return 1;
	}

	addr_p = (unsigned char *)ll->sll_addr;
	for (idx = 0; idx < ETH_ALEN; idx++)
		addr_p[idx] = val[idx];

	return 0;
}

/*
 * Function: get_ifinfo()
 *
 * Description:
 *  This function gets the interface information with ioctl()
 *
 * Argument:
 *    ans   : ifreq structure to store the information
 *  sock_fd : socket file descriptor
 *  ifname  : interface name
 *   query  : ioctl request value
 *
 * Return value:
 *  None
 *
 */
void get_ifinfo(struct ifreq *ans, int sock_fd, const char *ifname, int query)
{
	memset(ans, '\0', sizeof(struct ifreq));
	strncpy(ans->ifr_name, ifname, (IFNAMSIZ - 1));

	if (ioctl(sock_fd, query, ans) < 0)
		fatal_error("ioctl()");
}

/*
 * Function: strtotimespec()
 *
 * Description:
 *  This function converts a string to timespec structure
 *
 * Argument:
 *    str   : nano second value in character representation
 *    ts_p  : pointer to a timespec structure
 *
 * Return value:
 *  0: Success
 *  1: Fail
 */
int strtotimespec(const char *str, struct timespec *ts_p)
{
	size_t len;
	char *sec_str;
	unsigned long sec = 0;
	unsigned long nsec = 0;

	len = strlen(str);
	if (len > 9) {		/* Check the specified value is bigger than 999999999 */
		sec_str = calloc((len - 9 + 1), sizeof(char));
		strncpy(sec_str, str, len - 9);
		sec = strtoul(sec_str, NULL, 0);
		if (sec > 0x7fffffff)
			return 1;
		free(sec_str);
		nsec = strtoul(str + len - 9, NULL, 0);
	} else {
		nsec = strtoul(str, NULL, 0);
	}

	ts_p->tv_sec = sec;
	ts_p->tv_nsec = nsec;

	return 0;
}

/*
 * Function: get_a_lla_byifindex()
 *
 * Description:
 *  This function gets one of the link-local addresses which is specified
 *  by interface index
 *
 * Argument:
 *   lla_p  : pointer to a sockaddr_in6 structure which stores the lla
 *  ifindex : index of the interface
 *
 * Return value:
 *  0: Success
 *  1: Fail
 */
int get_a_lla_byifindex(struct sockaddr_in6 *lla_p, int ifindex)
{
	FILE *fp;
	int ret;
	unsigned int oct[16];
	int ifidx, prefixlen, scope;
	char line[PROC_IFINET6_FILE_LINELENGTH];
	int pos;

	if ((fp = fopen(PROC_IFINET6_FILE, "r")) == NULL) {
		fprintf(stderr, "Faile to open %s\n", PROC_IFINET6_FILE);
		return 1;
	}

	while (fgets(line, PROC_IFINET6_FILE_LINELENGTH, fp) != NULL) {
		ret = sscanf(line,
			     "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x %x %x %x",
			     &oct[0], &oct[1], &oct[2], &oct[3],
			     &oct[4], &oct[5], &oct[6], &oct[7],
			     &oct[8], &oct[9], &oct[10], &oct[11],
			     &oct[12], &oct[13], &oct[14], &oct[15],
			     &ifidx, &prefixlen, &scope);

		if (ret == EOF)
			fatal_error("scanf()");
		else if (ret != 19)
			fatal_error
			    ("The number of input item is less than the expected");

		if (ifidx != ifindex)
			continue;

		if (prefixlen != 64)
			continue;

		if (scope != PROC_IFINET6_LINKLOCAL)
			continue;

		/* Find a link-local address */
		lla_p->sin6_family = AF_INET6;
		lla_p->sin6_port = 0;
		lla_p->sin6_flowinfo = 0;
		lla_p->sin6_scope_id = ifindex;

		for (pos = 0; pos < 16; pos++)
			lla_p->sin6_addr.s6_addr[pos] = oct[pos];

		return 0;
	}

	fprintf(stderr, "No link-local address is found.\n");
	return 1;
}

/*
 * Function: get_maddrinfo()
 *
 * Description:
 *  This function translates multicast address informantion into the addrinfo
 *  structure
 *
 * Argument:
 *   family:    protocol family
 *   maddr:     multicast address in character string
 *   portnum:   port number in character string
 *
 * Return value:
 *  pointer to the addrinfo which stores the multicast address information
 */
struct addrinfo *get_maddrinfo(sa_family_t family, const char *maddr,
			       const char *portnum)
{
	struct addrinfo hints;	/* hints for getaddrinfo() */
	struct addrinfo *res;	/* pointer to addrinfo structure */
	int err;		/* return value of getaddrinfo */

	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags |= AI_NUMERICHOST;

	err = getaddrinfo(maddr, portnum, &hints, &res);
	if (err) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (res->ai_next) {
		fprintf(stderr, "getaddrinfo(): multiple address is found.");
		exit(EXIT_FAILURE);
	}

	return res;
}

/*
 * Function: create_group_info()
 *
 * Description:
 *  This function create a group information to join the group
 *  This function calls malloc to store the information
 *
 * Argument:
 *   ifindex:   interface index
 *   mainfo_p:  pointer to addrinfo structure for multicast address
 *
 * Return value:
 *  pointer to allocated group_filter structure
 */
struct group_req *create_group_info(uint32_t ifindex, struct addrinfo *mainfo_p)
{
	struct group_req *greq;

	/* allocate memory for group_filter */
	greq = (struct group_req *)calloc(1, sizeof(struct group_req));
	if (greq == NULL)
		fatal_error("calloc()");

	/* substitute informations */
	greq->gr_interface = ifindex;
	memcpy(&greq->gr_group, mainfo_p->ai_addr, mainfo_p->ai_addrlen);

	return greq;
}

/*
 * Function: create_source_filter()
 *
 * Description:
 *  This function create a source filter.
 *  This function calls malloc to store the source filter.
 *
 * Argument:
 *   ifindex:   interface index
 *   mainfo_p:  pointer to addrinfo structure for multicast address
 *   fmode:     filter mode
 *   saddrs:    comma separated array of the source addresses
 *
 * Return value:
 *  pointer to allocated group_filter structure
 */
struct group_filter *create_source_filter(uint32_t ifindex,
					  struct addrinfo *mainfo_p,
					  uint32_t fmode, char *saddrs)
{
	struct group_filter *gsf;	/* pointer to group_filter structure */
	uint32_t numsrc;	/* number of source address */
	struct addrinfo hints;	/* hints for getaddrinfo() */
	struct addrinfo *res;	/* pointer to addrinfo structure */
	int err;		/* return value of getaddrinfo */
	uint32_t idx;
	char *sp, *ep;

	/* calculate the number of source address */
	numsrc = 1;
	for (sp = saddrs; *sp != '\0'; sp++)
		if (*sp == ',')
			numsrc++;

	if (debug)
		fprintf(stderr, "number of source address is %u\n", numsrc);

	/* allocate memory for group_filter */
	gsf = (struct group_filter *)calloc(1, GROUP_FILTER_SIZE(numsrc));
	if (gsf == NULL)
		fatal_error("calloc()");

	/* substitute interface index, multicast address, filter mode */
	gsf->gf_interface = ifindex;
	memcpy(&gsf->gf_group, mainfo_p->ai_addr, mainfo_p->ai_addrlen);
	gsf->gf_fmode = fmode;
	gsf->gf_numsrc = numsrc;

	/* extract source address aray and substitute the addersses */
	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = mainfo_p->ai_family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags |= AI_NUMERICHOST;

	/* extract source address aray and substitute the addersses */
	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = mainfo_p->ai_family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags |= AI_NUMERICHOST;

	sp = saddrs;
	for (idx = 0; idx < numsrc; idx++) {
		ep = strchr(sp, ',');
		if (ep != NULL)
			*ep = '\0';
		if (debug)
			fprintf(stderr, "source address[%u]: %s\n", idx, sp);

		err = getaddrinfo(sp, NULL, &hints, &res);
		if (err) {
			fprintf(stderr, "getaddrinfo(): %s\n",
				gai_strerror(err));
			exit(EXIT_FAILURE);
		}

		memcpy(&gsf->gf_slist[idx], res->ai_addr, res->ai_addrlen);
		freeaddrinfo(res);
		sp = ep + 1;
	}

	return gsf;
}
