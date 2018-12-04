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
 *	ns-igmp_querier.c
 *
 * Description:
 *	This utiltity sends IGMP queries.
 *	(General Query, Multicast Address Specific Query
 *	 or Multicast Address and Source Specific Query)
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Apr 24 2006 - Created (Mitsuru Chinen)
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
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/igmp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ns-mcast.h"
#include "ns-traffic.h"

/*
 * Structure Definitions
 */
struct igmp_info {
	uint32_t ifindex;
	struct igmpv3_query *query;
	double timeout;
	struct timespec interval;
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
		"\t-I ifname\tname of listening interface\n"
		"\t-m addr\tmulticast address\n"
		"\t-s addrs\tcomma separated array of Source Addresses\n"
		"\t-r value\tMax Resp Code\n"
		"\t-i value\tinterval [nanosec]\n"
		"\t-t value\ttimeout [sec]\n"
		"\t-o\t\tsend only one query\n"
		"\t-b\t\twork in the background\n"
		"\t-d\t\tdisplay debug informations\n"
		"\t-h\t\tdisplay this usage\n", program_name);
	exit(exit_value);
}

/*
 * Function: set_signal_flag()
 *
 * Description:
 *  This function sets global variables accordig to signal
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
 * Function: create_query()
 *
 * Description:
 *  This function create a igmpv3 query information.
 *  This function allocates memory to store the information.
 *
 * Argument:
 *   code:	Max Resp Code
 *   maddr:	multicast address
 *   saddrs:	comma separated array of the source addresses
 *
 * Return value:
 *  pointer to allocated igmpv3_query structure
 */
struct igmpv3_query *create_query(uint8_t code, char *maddr, char *saddrs)
{
	struct igmpv3_query *query;	/* pointer to igmpv3_query structure */
	uint16_t numsrc;	/* number of source address */
	size_t query_size;	/* size of igmpv3_query */
	struct in_addr ip;
	uint32_t idx;
	char *sp, *ep;

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

	/* allocate memory for igmpv3_query structure */
	query_size = MY_IGMPV3_QUERY_SIZE(numsrc);
	query = (struct igmpv3_query *)calloc(1, query_size);
	if (query == NULL)
		fatal_error("calloc()");

	/* substitute paramaters */
	query->type = IGMP_HOST_MEMBERSHIP_QUERY;
	query->code = code;
	query->csum = 0;	/* Calculate later */
	query->resv = 0;
	query->suppress = 0;
	query->qrv = 0;
	query->qqic = 0;
	query->nsrcs = htons(numsrc);

	/* substitute multicast address */
	if (maddr == NULL) {
		query->group = htonl(INADDR_ANY);
	} else {
		if (inet_pton(AF_INET, maddr, &ip) <= 0) {
			fprintf(stderr,
				"multicast address is something wrong\n");
			return NULL;
		}
		query->group = ip.s_addr;
	}

	/* substitute source addresses */
	sp = saddrs;
	for (idx = 0; idx < numsrc; idx++) {
		ep = strchr(sp, ',');
		if (ep != NULL)
			*ep = '\0';
		if (debug)
			fprintf(stderr, "source address[%u]: %s\n", idx, sp);

		if (inet_pton(AF_INET, sp, &ip) <= 0) {
			fprintf(stderr,
				"source address list is something wrong\n");
			return NULL;
		}
		query->srcs[idx] = ip.s_addr;
		sp = ep + 1;
	}

	/* Calculate checksum */
	query->csum = calc_checksum((u_int16_t *) query, query_size);

	return query;
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
 *  info_p: pointer to data of querier information
 *   bg_p:  pointer to the flag of working in backgrond
 *
 * Return value:
 *  None
 */
void parse_options(int argc, char *argv[], struct igmp_info *info_p, int *bg_p)
{
	int optc;		/* option */
	unsigned long opt_ul;	/* option value in unsigned long */
	double opt_d;		/* option value in double */
	uint8_t max_resp;	/* Max Resp Code */
	char *maddr;		/* multicast address */
	char *saddrs;		/* comma separated array of source addresses */

	max_resp = IGMP_MAX_HOST_REPORT_DELAY;
	maddr = NULL;
	saddrs = NULL;

	while ((optc = getopt(argc, argv, "I:m:s:r:t:i:obdh")) != EOF) {
		switch (optc) {
		case 'I':
			info_p->ifindex = if_nametoindex(optarg);
			if (info_p->ifindex == 0) {
				fprintf(stderr,
					"specified interface is incorrect\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 'm':
			maddr = strdup(optarg);
			if (maddr == NULL)
				fatal_error("strdup()");
			break;

		case 's':
			saddrs = strdup(optarg);
			if (saddrs == NULL)
				fatal_error("strdup()");
			break;

		case 'r':
			opt_ul = strtoul(optarg, NULL, 0);
			if (opt_ul > 255) {
				fprintf(stderr,
					"Max Resp Code should be less then 256\n");
				usage(program_name, EXIT_FAILURE);
			}
			max_resp = opt_ul;
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

		case 'i':
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

		default:
			usage(program_name, EXIT_FAILURE);
		}
	}

	if (info_p->ifindex == 0) {
		fprintf(stderr, "specified interface seems incorrect\n");
		usage(program_name, EXIT_FAILURE);
	}

	if ((info_p->query = create_query(max_resp, maddr, saddrs)) == NULL)
		usage(program_name, EXIT_FAILURE);

	free(maddr);
	free(saddrs);
}

/*
 * Function: create_socket()
 *
 * Description:
 *  This function creates a socket to send
 *
 * Argument:
 *  info_p: pointer to data of igmp query information
 *
 * Return value:
 *  file descriptor referencing the socket
 */
int create_socket(struct igmp_info *info_p)
{
	int sd;			/* socket file descriptor */
	int on;
	unsigned char opt[4] = { 0x94, 0x04, 0x00, 0x00 };	/* Router Alert */
	struct ip_mreqn mcast_req, *req_p = &mcast_req;

	/* Create a socket */
	sd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
	if (sd < 0)
		fatal_error("socket()");

	/* Enable to reuse the socket */
	on = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)))
		fatal_error("setsockopt(): enable to reuse the socket");

	/* Add router alert option */
	if (setsockopt(sd, IPPROTO_IP, IP_OPTIONS, opt, sizeof(opt)))
		fatal_error("setsockopt(): socket options");

	/* Specify the interface for outgoing datagrams */
	req_p->imr_multiaddr.s_addr = info_p->query->group;
	req_p->imr_address.s_addr = htonl(INADDR_ANY);
	req_p->imr_ifindex = info_p->ifindex;
	if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF,
		       req_p, sizeof(struct ip_mreqn))) {
		fatal_error("setsockopt(): specify the interface");
	}

	return sd;
}

/*
 * Function: send_query()
 *
 * Description:
 *  This function sends IGMP query
 *
 * Argument:
 *  info_p: pointer to data of igmp query information
 *
 * Return value:
 *  None
 */
void send_query(struct igmp_info *info_p)
{
	int sd;
	int retval;
	double start_time;
	struct sockaddr_in to;
	size_t query_size;

	/* Set singal hander for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Specify multicast address to send */
	to.sin_family = AF_INET;
	to.sin_port = IPPROTO_IGMP;
	if (info_p->query->group == htonl(INADDR_ANY))
		to.sin_addr.s_addr = IGMP_ALL_HOSTS;
	else
		to.sin_addr.s_addr = info_p->query->group;

	/* Create a socket */
	sd = create_socket(info_p);

	/* loop for sending queries */
	start_time = time(NULL);
	query_size = MY_IGMPV3_QUERY_SIZE(ntohs(info_p->query->nsrcs));
	if (debug)
		fprintf(stderr, "query size is %zu\n", query_size);

	for (;;) {
		retval = sendto(sd, info_p->query, query_size, 0,
				(struct sockaddr *)&to,
				sizeof(struct sockaddr_in));
		if (retval != query_size) {
			if (errno == ENOBUFS) {
				sleep(1);
				continue;
			}
			
			if (catch_sighup)
				break;
			else
				fatal_error("sendto()");
		}

		/* Check timeout:
		   If timeout value is negative only send one datagram */
		if (info_p->timeout)
			if (info_p->timeout < difftime(time(NULL), start_time))
				break;

		/* Wait in specified interval */
		nanosleep(&info_p->interval, NULL);

		/* catch SIGHUP */
		if (catch_sighup)
			break;

	}

	close(sd);
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	struct igmp_info mcast_rcv;
	int background = 0;

	debug = 0;
	program_name = strdup(argv[0]);

	memset(&mcast_rcv, '\0', sizeof(struct igmp_info));
	parse_options(argc, argv, &mcast_rcv, &background);

	if (background)		/* Work in the background */
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	send_query(&mcast_rcv);

	exit(EXIT_SUCCESS);
}
