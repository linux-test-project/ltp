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
 *	ns-udpsender.c
 *
 * Description:
 *	This is UDP datagram sender (not only unicast but also multicast)
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Mar 17 2006 - Created (Mitsuru Chinen)
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
#include <sys/types.h>

#include "ns-traffic.h"

/*
 * Structure Definitions
 */

struct udp_info {
	int sd;
	int is_multicast;
	sa_family_t family;
	char *ifname;
	char *dst_name;
	char *dst_port;
	struct addrinfo addr_info;
	unsigned char *msg;
	size_t msgsize;
	double timeout;
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
		"\t-f num\ttprotocol family\n"
		"\t\t  4 : IPv4\n"
		"\t\t  6 : IPv6\n"
		"\t-D addr\tIP address of the destination host\n"
		"\t-p num\tport number of the destination host\n"
		"\t-s size\tdata size of UDP payload\n"
		"\t-t value\ttimeout [sec]\n"
		"\t-o\t\tsend only one UDP datagram\n"
		"\t-b\t\twork in the background\n"
		"\t-d\t\tdisplay debug informations\n"
		"\t-h\t\tdisplay this usage\n"
		"\n"
		"\t[options for multicast]\n"
		"\t  -m\t\tsend multicast datagrams\n"
		"\t  -I if_name\tinterface name of the source host\n",
		program_name);
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
 * Function: parse_options()
 *
 * Description:
 *  This function parse the options
 *
 * Argument:
 *   argc:  the number of argument
 *   argv:  arguments
 *  udp_p: pointer to the data of udp datagram
 *   bg_p: pointer to the flag of working in backgrond
 *
 * Return value:
 *  None
 */
void parse_options(int argc, char *argv[], struct udp_info *udp_p, int *bg_p)
{
	int optc;		/* option */
	unsigned long opt_ul;	/* option value in unsigned long */
	double opt_d;		/* option value in double */
	int is_specified_family = 0;
	int is_specified_daddr = 0;
	int is_specified_port = 0;

	while ((optc = getopt(argc, argv, "f:D:p:s:t:obdhmI:")) != EOF) {
		switch (optc) {
		case 'f':
			if (optarg[0] == '4')
				udp_p->family = PF_INET;	/* IPv4 */
			else if (optarg[0] == '6')
				udp_p->family = PF_INET6;	/* IPv6 */
			else {
				fprintf(stderr,
					"protocol family should be 4 or 6.\n");
				usage(program_name, EXIT_FAILURE);
			}
			is_specified_family = 1;
			break;

		case 'D':
			udp_p->dst_name = strdup(optarg);
			if (udp_p->dst_name == NULL)
				fatal_error("strdup() failed.");
			is_specified_daddr = 1;
			break;

		case 'p':
			opt_ul = strtoul(optarg, NULL, 0);
			if (opt_ul < 1 || PORTNUMMAX < opt_ul) {
				fprintf(stderr,
					"The range of port is from %u to %u\n",
					1, PORTNUMMAX);
				usage(program_name, EXIT_FAILURE);
			}
			udp_p->dst_port = strdup(optarg);
			is_specified_port = 1;
			break;

		case 's':
			opt_ul = strtoul(optarg, NULL, 0);
			udp_p->msgsize = opt_ul;
			break;

		case 't':
			opt_d = strtod(optarg, NULL);
			if (opt_d < 0.0) {
				fprintf(stderr,
					"Timeout should be positive value\n");
				usage(program_name, EXIT_FAILURE);
			}
			udp_p->timeout = opt_d;
			break;

		case 'o':
			udp_p->timeout = -1.0;
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

			/* Options for multicast */
		case 'm':
			udp_p->is_multicast = 1;
			break;

		case 'I':
			udp_p->ifname = strdup(optarg);
			if (udp_p->dst_name == NULL)
				fatal_error("strdup() failed.");
			break;

		default:
			usage(program_name, EXIT_FAILURE);
		}
	}

	if (!is_specified_family) {
		fprintf(stderr, "protocol family is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}

	if (!is_specified_daddr) {
		fprintf(stderr, "desttinaion IP address is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}

	if (!is_specified_port) {
		fprintf(stderr, "destination port is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}

	if (udp_p->is_multicast) {
		if (udp_p->ifname == NULL) {
			fprintf(stderr,
				"interface name is not specified with multicast\n");
			usage(program_name, EXIT_FAILURE);
		}
	}
}

/*
 * Function: create_udp_datagram()
 *
 * Description:
 *  This function creates udp datagram
 *
 * Argument:
 *  udp_p: pointer to data of udp data structure
 *
 * Return value:
 *  None
 */
void create_udp_datagram(struct udp_info *udp_p)
{
	struct addrinfo hints;	/* hints for getaddrinfo() */
	struct addrinfo *res;	/* pointer to addrinfo structure */
	struct ifreq ifinfo;	/* Interface information */
	int err;		/* return value of getaddrinfo */
	int on;			/* variable for socket option */

	/* Set the hints to addrinfo() */
	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = udp_p->family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	/* Get the address information */
	err = getaddrinfo(udp_p->dst_name, udp_p->dst_port, &hints, &res);
	if (err) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (res->ai_next) {
		fprintf(stderr, "getaddrinfo(): multiple address is found.");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	udp_p->sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (udp_p->sd < 0)
		fatal_error("socket()");

	/* Enable to reuse the socket */
	on = 1;
	if (setsockopt(udp_p->sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)))
		fatal_error("setsockopt()");

	/* In multicast case, specify the interface for outgoing datagrams */
	if (udp_p->is_multicast) {
		struct ip_mreqn mcast_req, *req_p = &mcast_req;
		int ifindex, *id_p = &ifindex;

		get_ifinfo(&ifinfo, udp_p->sd, udp_p->ifname, SIOCGIFINDEX);
		ifindex = ifinfo.ifr_ifindex;

		switch (udp_p->family) {
		case PF_INET:	/* IPv4 */
			req_p->imr_multiaddr =
			    ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
			req_p->imr_address.s_addr = htonl(INADDR_ANY);
			req_p->imr_ifindex = ifindex;
			if (setsockopt(udp_p->sd, IPPROTO_IP, IP_MULTICAST_IF,
				       req_p, sizeof(struct ip_mreqn))) {
				fatal_error("setsockopt()");
			}
			break;

		case PF_INET6:	/* IPv6 */
			if (setsockopt
			    (udp_p->sd, IPPROTO_IPV6, IPV6_MULTICAST_IF, id_p,
			     sizeof(int))) {
				fatal_error("setsockopt()");
			}
			break;
		}
	}

	/* Make the payload */
	udp_p->msg = malloc(udp_p->msgsize);
	if (udp_p->msg == NULL) {
		fatal_error("malloc()");
		exit(EXIT_FAILURE);
	}
	fill_payload(udp_p->msg, udp_p->msgsize);

	/* Store addrinfo */
	memcpy(&(udp_p->addr_info), res, sizeof(struct addrinfo));
	freeaddrinfo(res);
}

/*
 * Function: send_udp_datagram()
 *
 * Description:
 *  This function sends udp datagram
 *
 * Argument:
 *  udp_p: pointer to the udp data structure
 *
 * Return value:
 *  None
 */
void send_udp_datagram(struct udp_info *udp_p)
{
	int retval;
	double start_time;

	/* Set singal hander for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/*
	 * loop for sending packets
	 */
	start_time = time(NULL);

	for (;;) {
		retval = sendto(udp_p->sd, udp_p->msg, udp_p->msgsize, 0,
				udp_p->addr_info.ai_addr,
				udp_p->addr_info.ai_addrlen);
		if (retval != udp_p->msgsize) {
			if (catch_sighup)
				break;
			else
				fatal_error("sendto()");
		}

		/* Check timeout:
		   If timeout value is negative only send one datagram */
		if (udp_p->timeout)
			if (udp_p->timeout < difftime(time(NULL), start_time))
				break;

		if (catch_sighup)	/* catch SIGHUP */
			break;
	}

	/* Close the socket */
	close(udp_p->sd);
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	struct udp_info udp_data;
	int background = 0;

	debug = 0;
	program_name = strdup(argv[0]);

	memset(&udp_data, '\0', sizeof(struct udp_info));
	parse_options(argc, argv, &udp_data, &background);

	create_udp_datagram(&udp_data);

	if (background)		/* Work in the background */
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	send_udp_datagram(&udp_data);

	exit(EXIT_SUCCESS);
}
