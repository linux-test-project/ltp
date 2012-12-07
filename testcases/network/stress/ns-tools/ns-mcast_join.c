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
 *	ns-mcast_join.c
 *
 * Description:
 *	This is an assistant tool to join multicast groups
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	May 1 2006 - Created (Mitsuru Chinen)
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

#define ADDR_STR_MAXSIZE    80
#define OPEN_SOCK_MIN	    6

/*
 * Gloval variables
 */
char *program_name;		/* program name */

struct sigaction handler;	/* Behavior for a signal */
volatile int catch_sighup;	/* When catch the SIGHUP, set to non-zero */

sa_family_t family;		/* protocol family */
int level;			/* protocol levels */
uint32_t ifindex;		/* interface index where listening multicast */

size_t num_group;		/* Number of the groups */
char *mcast_prefix;		/* Prefix of the multicast address */
uint32_t fmode;			/* filter mode */
char *saddrs;			/* comma separated array of source addresses */

int is_multi_socket;		/* If non-zero, multi-socket mode */

size_t join_leave_times;	/* If non-zero, join-leave mode */
				/* the value is times of join/leave */
char *mcast_addr;		/* multicast address to join/leave */
struct timespec interval;	/* interval for join-leave mode */

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
		"\t-f num\tprotocol family\n"
		"\t\t  4 : IPv4\n"
		"\t\t  6 : IPv6\n"
		"\t-I ifname\tname of listening interface\n"
		"\t-a addr\tmulticast address for join-leave mode\n"
		"\t-F mode\tfilter mode\n"
		"\t\t  include : include mode\n"
		"\t\t  exclude : exclude mode\n"
		"\t-s addrs\tcomma separated array of Source Addresses\n"
		"\t-d\t\tdisplay debug informations\n"
		"\t-h\t\tdisplay this usage\n"
		"\n"
		"\t[multiple join mode]\n"
		"\t  -n num\tnumber of multicast address\n"
		"\t  -p prefix\tprefix of the multicast address\n"
		"\t  -m\t\tmultiple socket mode\n"
		"\t\t  4 : a.b(.x.y) - x y is defined automatically\n"
		"\t\t  6 : {prefix}::z - z is defined automatically\n"
		"\n"
		"\t[join-leave mode]\n"
		"\t  -l times of join/leave\n"
		"\t  -i nsec\tinterval for join-leave mode\n", program_name);
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
 *
 * Return value:
 *  None
 */
void parse_options(int argc, char *argv[])
{
	int optc;		/* option */
	unsigned long opt_ul;	/* option value in unsigned long */

	while ((optc = getopt(argc, argv, "f:I:p:F:s:n:ml:i:a:dh")) != EOF) {
		switch (optc) {
		case 'f':
			if (optarg[0] == '4') {
				family = PF_INET;	/* IPv4 */
				level = IPPROTO_IP;
			} else if (optarg[0] == '6') {
				family = PF_INET6;	/* IPv6 */
				level = IPPROTO_IPV6;
			} else {
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

		case 'p':
			mcast_prefix = strdup(optarg);
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

		case 'l':
			join_leave_times = strtoul(optarg, NULL, 0);
			break;

		case 'i':
			if (strtotimespec(optarg, &interval)) {
				fprintf(stderr,
					"Interval is something wrong\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 'a':
			mcast_addr = strdup(optarg);
			break;

		case 's':
			saddrs = strdup(optarg);
			if (saddrs == NULL)
				fatal_error("strdup()");
			break;

		case 'n':
			opt_ul = strtoul(optarg, NULL, 0);
			if (opt_ul > 255 * 254) {
				fprintf(stderr,
					"The number of group shoud be less than %u\n",
					255 * 254);
				usage(program_name, EXIT_FAILURE);
			}
			num_group = opt_ul;
			break;

		case 'm':
			is_multi_socket = 1;
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

	if (saddrs) {
		if (fmode != MCAST_EXCLUDE && fmode != MCAST_INCLUDE) {
			fprintf(stderr, "filter mode is wrong\n");
			usage(program_name, EXIT_FAILURE);
		}
	}
}

/*
 * Function: join_group()
 *
 * Description:
 *  This function make sockets to join the groups
 *
 * Return value:
 *  None
 */
void join_group(void)
{
	int sd;			/* socket file descriptor */
	int *sock_array;	/* socket descriptor array */
	size_t num_sock;	/* number of the socket */
	char maddr[ADDR_STR_MAXSIZE];	/* multicast address in string */
	int idx;
	struct addrinfo *maddr_info;
	struct group_req *grp_info;
	struct group_filter *gsf;

	if (!is_multi_socket)
		num_sock = 1;
	else
		num_sock = num_group;

	/* Allocate socket array */
	sock_array = calloc(num_sock, sizeof(int));
	if (sock_array == NULL)
		fatal_error("calloc()");

	for (idx = 0; idx < num_sock; idx++) {
		sock_array[idx] = socket(family, SOCK_DGRAM, IPPROTO_UDP);

		if (sock_array[idx] < 0) {
			if (idx < OPEN_SOCK_MIN)
				fatal_error("socket()");
			else {
				int j;	/* Closed some sockets for daemon() */
				for (j = 0; j < OPEN_SOCK_MIN; j++)
					close(sock_array[idx - 1 - j]);
				num_group = idx - j - 1;
				break;
			}
		}

		if (!is_multi_socket)
			maximize_sockbuf(sock_array[idx]);
	}

	sd = sock_array[0];
	if (mcast_addr) {
		strncpy(maddr, mcast_addr, ADDR_STR_MAXSIZE);
		if (debug)
			fprintf(stderr, "multicast address is %s\n", maddr);
	}

	for (idx = 0; idx < num_group; idx++) {
		if (is_multi_socket)
			sd = sock_array[idx];

		if (debug)
			fprintf(stderr, "socket: %d\n", sd);

		if (mcast_prefix) {
			switch (family) {
			case PF_INET:
				{
					unsigned int x, y;
					x = idx / 254;
					y = idx % 254 + 1;
					sprintf(maddr, "%s.%d.%d", mcast_prefix,
						x, y);
				}
				break;

			case PF_INET6:
				sprintf(maddr, "%s:%x", mcast_prefix, idx + 1);
				break;
			}

			if (debug)
				fprintf(stderr, "multicast address is %s\n",
					maddr);
		}

		maddr_info = get_maddrinfo(family, maddr, NULL);

		grp_info = create_group_info(ifindex, maddr_info);
		if (setsockopt(sd, level, MCAST_JOIN_GROUP, grp_info,
			       sizeof(struct group_req)) == -1) {
			if (idx == 0)
				fatal_error("setsockopt(): Join no group");
			else {
				num_group--;
				free(grp_info);
				freeaddrinfo(maddr_info);
				break;
			}
			free(grp_info);
		}

		if (saddrs) {
			gsf =
			    create_source_filter(ifindex, maddr_info, fmode,
						 saddrs);
			if (setsockopt
			    (sd, level, MCAST_MSFILTER, gsf,
			     GROUP_FILTER_SIZE(gsf->gf_numsrc)) == -1) {
				if (idx == 0)
					fatal_error
					    ("setsockopt(): Add no group filter");
				else {
					num_group--;
					free(gsf);
					freeaddrinfo(maddr_info);
					break;
				}
				free(gsf);
			}
		}

		freeaddrinfo(maddr_info);
	}

	fprintf(stdout, "%zu groups\n", num_group);
	fflush(stdout);

	/* Become a daemon for the next step in shell script */
	if (daemon(0, 0) < 0)
		fatal_error("daemon()");

	/* Waiting for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigfillset()");

	for (;;)
		if (catch_sighup)
			break;
}

/*
 * Function: join_leave_group()
 *
 * Description:
 *  This function make sockets to join the groups then leave it
 *
 * Return value:
 *  None
 */
void join_leave_group(void)
{
	int sd;			/* socket file descriptor */
	struct addrinfo *maddr_info;
	struct group_req *grp_info;
	struct group_filter *gsf;
	size_t cnt;

	sd = socket(family, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0)
		fatal_error("socket()");

	maddr_info = get_maddrinfo(family, mcast_addr, NULL);
	grp_info = create_group_info(ifindex, maddr_info);
	if (saddrs)
		gsf = create_source_filter(ifindex, maddr_info, fmode, saddrs);
	else
		gsf = NULL;

	/* Waiting for SIGHUP */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigfillset()");

	for (cnt = 0; cnt < join_leave_times; cnt++) {
		/* Join */
		if (setsockopt(sd, level, MCAST_JOIN_GROUP, grp_info,
			       sizeof(struct group_req)) == -1)
			fatal_error("setsockopt(): Failed to join a group");

		if (gsf)
			if (setsockopt(sd, level, MCAST_MSFILTER, gsf,
				       GROUP_FILTER_SIZE(gsf->gf_numsrc)) == -1)
				fatal_error
				    ("setsockopt(): Failed to add a group filter");

		nanosleep(&interval, NULL);

		/* Leave */
		if (setsockopt(sd, level, MCAST_LEAVE_GROUP, grp_info,
			       sizeof(struct group_req)) == -1)
			fatal_error("setsockopt(): Failed to leave a group");

		nanosleep(&interval, NULL);

		if (catch_sighup)
			break;
	}

	free(grp_info);
	if (gsf)
		free(gsf);
	freeaddrinfo(maddr_info);
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	debug = 0;
	program_name = strdup(argv[0]);

	parse_options(argc, argv);

	if (!join_leave_times) {
		if (mcast_prefix == NULL && mcast_addr == NULL) {
			fprintf(stderr, "multicast address is not specified\n");
			usage(program_name, EXIT_FAILURE);
		}
		join_group();
	} else {
		if (mcast_addr == NULL) {
			fprintf(stderr, "multicast address is not specified\n");
			usage(program_name, EXIT_FAILURE);
		}
		join_leave_group();
	}

	exit(EXIT_SUCCESS);
}
