/******************************************************************************/
/*                                                                            */
/*   Copyright (c) International Business Machines  Corp., 2005               */
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
 *	ns-udpclient.c
 *
 * Description:
 *	This is UDP traffic client.
 *	Send UDP datagram to a server, then receive datagram from it
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Oct 19 2005 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

#include "ns-traffic.h"

/*
 * Fixed value
 */
#define MESSAGE_LEN 1000	/* The length of message */
#define RECVFROM_TIMEOUT  1	/* Timeout length of recvfrom() */

/*
 * Gloval variables
 */
struct sigaction handler;	/* Behavior for a signal */
int catch_sigalrm;		/* When catch the SIGALRM, set to non-zero */
int catch_sighup;		/* When catch the SIGHUP, set to non-zero */

/*
 * Standard Header Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

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
		"\t-S\tname or IP address of the server\n"
		"\t-f\tprotocol family\n"
		"\t\t  4 : IPv4\n"
		"\t\t  6 : IPv6\n"
		"\t-p\tport number\n"
		"\t-b\twork in the background\n"
		"\t-d\tdisplay debug informations\n"
		"\t-h\tdisplay this usage\n", program_name);
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

	case SIGALRM:
		catch_sigalrm = 1;
		break;
	default:
		fprintf(stderr, "Unexpected signal (%d) is caught\n", type);
		exit(EXIT_FAILURE);
	}
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	char *program_name = argv[0];
	int optc;		/* option */

	sa_family_t family;	/* protocol family */
	char *server_name;	/* Name (or IP address) of the server */
	char *portnum;		/* port number in string representation */

	int sock_fd;		/* socket descriptor to access */
	int on;			/* on/off at an socket option */

	struct addrinfo hints;	/* hints for getaddrinfo() */
	struct addrinfo *res;	/* pointer to addrinfo structure */
	int err;		/* return value of getaddrinfo */

	char *message;		/* Pointer to the message */
	char *recvbuf = NULL;	/* Pointer to the message */

	int background = 0;	/* work in the background if non-zero */

	debug = 0;

	/* Initilalize the client information */
	family = PF_UNSPEC;
	server_name = NULL;
	portnum = NULL;

	/* Retrieve the options */
	while ((optc = getopt(argc, argv, "S:f:p:bdh")) != EOF) {
		switch (optc) {
		case 'S':
			server_name = strdup(optarg);
			if (server_name == NULL) {
				fprintf(stderr, "strdup() failed.");
				exit(EXIT_FAILURE);
			}
			break;

		case 'f':
			if (strncmp(optarg, "4", 1) == 0)
				family = PF_INET;	/* IPv4 */
			else if (strncmp(optarg, "6", 1) == 0)
				family = PF_INET6;	/* IPv6 */
			else {
				fprintf(stderr,
					"protocol family should be 4 or 6.\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 'p':
			{
				unsigned long int tmp;
				tmp = strtoul(optarg, NULL, 0);
				if (tmp < PORTNUMMIN || PORTNUMMAX < tmp) {
					fprintf(stderr,
						"The range of port is from %u to %u\n",
						PORTNUMMIN, PORTNUMMAX);
					usage(program_name, EXIT_FAILURE);
				}
				portnum = strdup(optarg);
			}
			break;

		case 'b':
			background = 1;
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

	/* Check the family is specified. */
	if (family == PF_UNSPEC) {
		fprintf(stderr, "protocol family isn't specified.\n");
		usage(program_name, EXIT_FAILURE);
	}

	/* Check the server name is specified. */
	if (server_name == NULL) {
		fprintf(stderr, "server name isn't specified.\n");
		usage(program_name, EXIT_FAILURE);
	}

	/* Check the port number is specified. */
	if (portnum == NULL) {
		fprintf(stderr, "port number isn't specified.\n");
		usage(program_name, EXIT_FAILURE);
	}

	/* If -b option is specified, work as a daemon */
	if (background)
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	/* Set a signal handler against SIGALRM */
	handler.sa_handler = set_signal_flag;
	handler.sa_flags = 0;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	if (sigaction(SIGALRM, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* At first, SIGHUP are Ignored. */
	handler.sa_handler = SIG_IGN;
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Set the hints to addrinfo() */
	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	err = getaddrinfo(server_name, portnum, &hints, &res);
	if (err) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (res->ai_next) {
		fprintf(stderr, "getaddrinfo(): multiple address is found.");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock_fd < 0)
		fatal_error("socket()");

	/* Enable to reuse the socket */
	on = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)))
		fatal_error("setsockopt()");

	/* Create a message */
	message = malloc(MESSAGE_LEN);
	if (debug) {
		strncpy(message, "Hello!", MESSAGE_LEN);
		message[MESSAGE_LEN - 1] = '\0';
	}

	/* Prepare the buffer to store the received message */
	recvbuf = malloc(MESSAGE_LEN + 1);
	if (recvbuf == NULL) {
		fprintf(stderr, "malloc() is failed.\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Loop for access to the server
	 */
	handler.sa_handler = set_signal_flag;
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");
	for (;;) {
		int recvlen;	/* lenght of recevied message */
		struct sockaddr_storage from_addr;	/* address of a client */
		socklen_t from_addr_len;	/* length of `client_addr' */

		/* Send the message to the server */
		if (sendto(sock_fd, message, MESSAGE_LEN, 0,
			   res->ai_addr, res->ai_addrlen) != MESSAGE_LEN) {
			if (catch_sighup)
				break;
			else
				fatal_error("sendto()");
		}

		/* Receive the response from the server */
		from_addr_len = sizeof(from_addr);
		alarm(RECVFROM_TIMEOUT);
		if ((recvlen = recvfrom(sock_fd, recvbuf, MESSAGE_LEN, 0,
					(struct sockaddr *)&from_addr,
					&from_addr_len)) < 0) {
			if (errno == EINTR) {
				if (catch_sighup) {
					break;
				} else if (catch_sigalrm) {
					if (debug)
						fprintf(stderr,
							"recvfrom() is timeout\n");
					continue;
				}
			}
			fatal_error("recvfrom()");
		}
		alarm(0);
		recvbuf[recvlen] = '\0';
		if (debug)
			fprintf(stderr, "Message is %s\n", recvbuf);

		/* Catch sighup */
		if (catch_sighup)
			break;
	}

	exit(EXIT_SUCCESS);
}
