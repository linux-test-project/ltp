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
 *	ns-mcast_receiver.c
 *
 * Description:
 *	This is a multicast UDP datagram receiver
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Apr 19 2006 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

/*
 * Header Files
 */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ns-mcast.h"
#include "ns-traffic.h"

/*
 * Structure Definitions
 */
struct mcast_rcv_info {
	struct addrinfo *mainfo;
	struct group_req *greq;
	struct group_filter *gsf;
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
		"\t-I ifname\tname of listening interface\n"
		"\t-m addr\tmulticast address\n"
		"\t-F mode\tfilter mode\n"
		"\t\t  include : include mode\n"
		"\t\t  exclude : exclude mode\n"
		"\t-s addrs\tcomma separated array of Source Addresses\n"
		"\t-p num\tport number\n"
		"\t-t value\ttimeout [sec]\n"
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
 * Function: parse_options()
 *
 * Description:
 *  This function parse the options
 *
 * Argument:
 *   argc:  the number of argument
 *   argv:  arguments
 *  info_p: pointer to data of multicast receiver information
 *   bg_p:  pointer to the flag of working in backgrond
 *
 * Return value:
 *  None
 */
void
parse_options(int argc, char *argv[], struct mcast_rcv_info *info_p, int *bg_p)
{
	int optc;		/* option */
	unsigned long opt_ul;	/* option value in unsigned long */
	double opt_d;		/* option value in double */
	uint32_t ifindex = 0;	/* interface index where listening multicast */
	sa_family_t family = AF_UNSPEC;	/* protocol family */
	char *maddr;		/* multicast address */
	uint32_t fmode = 0;	/* filter mode */
	char *saddrs;		/* comma separated array of source addresses */
	char *portnum;		/* listen port number in character string */

	maddr = NULL;
	saddrs = NULL;
	portnum = NULL;

	while ((optc = getopt(argc, argv, "f:I:m:F:s:p:t:bdh")) != EOF) {
		switch (optc) {
		case 'f':
			if (optarg[0] == '4')
				family = PF_INET;	/* IPv4 */
			else if (optarg[0] == '6')
				family = PF_INET6;	/* IPv6 */
			else {
				fprintf(stderr,
					"protocol family should be 4 or 6.\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 'I':
			ifindex = if_nametoindex(optarg);
			if (ifindex == 0) {
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

		case 'F':
			if (strncmp(optarg, "exclude", 8) == 0)
				fmode = MCAST_EXCLUDE;
			else if (strncmp(optarg, "include", 8) == 0)
				fmode = MCAST_INCLUDE;
			else {
				fprintf(stderr,
					"specified filter mode is incorrect\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 's':
			saddrs = strdup(optarg);
			if (saddrs == NULL)
				fatal_error("strdup()");
			break;

		case 'p':
			opt_ul = strtoul(optarg, NULL, 0);
			if (opt_ul < PORTNUMMIN || PORTNUMMAX < opt_ul) {
				fprintf(stderr,
					"The range of port is from %u to %u\n",
					PORTNUMMIN, PORTNUMMAX);
				usage(program_name, EXIT_FAILURE);
			}
			portnum = strdup(optarg);
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

	if (ifindex == 0) {
		fprintf(stderr, "specified interface seems incorrect\n");
		usage(program_name, EXIT_FAILURE);
	}

	if (maddr == NULL) {
		fprintf(stderr, "multicast address is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}

	if (portnum == NULL) {
		fprintf(stderr, "listening port number is not specified\n");
		usage(program_name, EXIT_FAILURE);
	}

	info_p->mainfo = get_maddrinfo(family, maddr, portnum);
	info_p->greq = create_group_info(ifindex, info_p->mainfo);
	if (saddrs) {
		if (fmode != MCAST_EXCLUDE && fmode != MCAST_INCLUDE) {
			fprintf(stderr, "filter mode is wrong\n");
			usage(program_name, EXIT_FAILURE);
		}
		info_p->gsf =
		    create_source_filter(ifindex, info_p->mainfo, fmode,
					 saddrs);
	}

	free(maddr);
	free(saddrs);
	free(portnum);
}

/*
 * Function: create_mcast_socket()
 *
 * Description:
 *  This function creates a socket to receive multicast datagrams
 *
 * Argument:
 *  info_p: pointer to data of multicast receiver information
 *
 * Return value:
 *  file descriptor referencing the socket
 */
int create_mcast_socket(struct mcast_rcv_info *info_p)
{
	int sd;			/* socket file descriptor */
	int level;		/* protocol levels */
	int on;			/* flag for setsockopt */

	switch (info_p->mainfo->ai_family) {
	case PF_INET:
		level = IPPROTO_IP;
		break;
	case PF_INET6:
		level = IPPROTO_IPV6;
		break;
	default:
		level = 0;
		fprintf(stderr, "Unknown protocol level %d\n", level);
		exit(EXIT_FAILURE);
		break;
	}

	/* Create a socket */
	sd = socket(info_p->mainfo->ai_family, info_p->mainfo->ai_socktype,
		    info_p->mainfo->ai_protocol);
	if (sd < 0)
		fatal_error("socket()");

	/* Bind to the multicast address */
	if (bind(sd, info_p->mainfo->ai_addr, info_p->mainfo->ai_addrlen) < 0)
		fatal_error("bind()");

	/* Enable to reuse the socket */
	on = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)))
		fatal_error("setsockopt(): failed to set reuse the socket");

	/* Join the multicast group */
	if (setsockopt
	    (sd, level, MCAST_JOIN_GROUP, info_p->greq,
	     sizeof(struct group_req)))
		fatal_error("setsockopt(): failed to join the multicast group");

	/* Apply the source filter */
	if (info_p->gsf) {
		if (setsockopt
		    (sd, level, MCAST_MSFILTER, info_p->gsf,
		     GROUP_FILTER_SIZE(info_p->gsf->gf_numsrc)))
			fatal_error
			    ("setsockopt(): failed to apply the source filter");
	}

	return sd;
}

/*
 * Function: receive_mcast()
 *
 * Description:
 *  This function receives multicast datagarms
 *
 * Argument:
 *  info_p: pointer to data of multicast receiver information
 *
 * Return value:
 *  None
 */
void receive_mcast(struct mcast_rcv_info *info_p)
{
	int sd;
	char *msgbuf;		/* Pointer to the message */
	size_t msgbuf_size;	/* size of msgbuf */
	ssize_t msglen;		/* the length of message */
	socklen_t optlen;	/* size of the result parameter */
	double start_time;	/* start time when receiving datagrams */

	/* Create a socket */
	sd = create_mcast_socket(info_p);

	/* Allocate a buffer to store the message */
	optlen = sizeof(msgbuf_size);
	if (getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &msgbuf_size, &optlen) < 0) {
		perror("getsockopt()");
		close(sd);
		exit(EXIT_FAILURE);
	}
	msgbuf = malloc(msgbuf_size + 1);
	if (msgbuf == NULL) {
		fprintf(stderr, "malloc() is failed.\n");
		close(sd);
		exit(EXIT_FAILURE);
	}

	/* Set singal hander for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigfillset()");

	/* Receive the message */
	start_time = time(NULL);
	for (;;) {
		struct sockaddr_storage addr;
		socklen_t addrlen;

		addrlen = sizeof(addr);
		msglen = recvfrom(sd, msgbuf, msgbuf_size, MSG_DONTWAIT,
				  (struct sockaddr *)&addr, &addrlen);
		if (msglen < 0) {
			if (errno != EAGAIN)
				fatal_error("recvfrom()");
		} else if (debug)
			fprintf(stderr, "received %zd byte message\n", msglen);

		if (info_p->timeout)
			if (info_p->timeout < difftime(time(NULL), start_time))
				break;

		if (catch_sighup)	/* catch SIGHUP */
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
	struct mcast_rcv_info mcast_rcv;
	int background = 0;

	debug = 0;
	program_name = strdup(argv[0]);

	memset(&mcast_rcv, '\0', sizeof(struct mcast_rcv_info));
	parse_options(argc, argv, &mcast_rcv, &background);

	if (background)		/* Work in the background */
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	receive_mcast(&mcast_rcv);

	exit(EXIT_SUCCESS);
}
